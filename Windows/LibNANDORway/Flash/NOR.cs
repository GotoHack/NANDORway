namespace LibNANDORway.Flash {
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using LibNANDORway.Boards;

    public sealed class NOR : FlashBase {
        #region ProgrammingModes enum

        public enum ProgrammingModes {
            WordMode = 0,
            UnlockBypassMode = 1,
            WriteBufferMode = 2
        }

        #endregion

        private const int WriteRetries = 5;

        public NOR(IBoard board) : base(board) {
            NORInformation = new NORInfo();
        }

        public NORInfo NORInformation { get; private set; }

        private void SetAddress(uint address) {
            Debug.Assert(address <= 0x7FFFFF);
            SendCommand(FlashCommand.NORAddressSet);
            Write(new[] { (byte) (address >> 16), (byte) (address >> 8 & 0xff), (byte) (address & 0xff) });
        }

        private void WriteWord(ushort data, bool incrementAddress) {
            SendCommand(incrementAddress ? FlashCommand.NORAddressIncrementEnable : FlashCommand.NORAddressIncrementDisable);

            SendCommand(FlashCommand.NORWriteWord);
            Write(new[] { (byte) (data >> 8 & 0xff), (byte) (data & 0xff) });
        }

        private void WriteAt(uint address, ushort data) {
            SetAddress(address);
            WriteWord(data, false);
        }

        private byte[] GetNORID() {
            SendCommand(FlashCommand.NORID);
            return Read(8);
        }

        public void NORInformationRefresh() {
            Ping();
            SendCommand(FlashCommand.IoLock);
            SendCommand(FlashCommand.NORReset);

            NORInformation = new NORInfo(GetNORID());
        }

        public byte[] ReadSector(uint offset, uint blockSize) {
            Debug.Assert((offset & blockSize / 2 - 1) == 0);

            SetAddress(offset);

            switch(blockSize) {
                case 0x1000:
                    SendCommand(FlashCommand.NORReadBlock4Kb);
                    break;
                case 0x2000:
                    SendCommand(FlashCommand.NORReadBlock8Kb);
                    break;
                case 0x10000:
                    SendCommand(FlashCommand.NORReadBlock64Kb);
                    break;
                case 0x20000:
                    SendCommand(FlashCommand.NORReadBlock128Kb);
                    break;
                default:
                    throw new ArgumentOutOfRangeException("blockSize");
            }

            return Read(blockSize);
        }

        public void EraseSector(uint offset) {
            SendCommand(NORInformation.ChipType == NORInfo.ChipTypes.SamsungK8Q2815Uqb && offset >= 0x400000 ? FlashCommand.NOR2NdDieEnable : FlashCommand.NOR2NdDieDisable);

            SetAddress(offset);
            SendCommand(FlashCommand.NOREraseSector);
            Debug.Assert(Ping());
        }

        public void EraseChip() {
            SendCommand(FlashCommand.NORReset);

            SendCommand(FlashCommand.NOR2NdDieDisable);
            SendCommand(FlashCommand.NOREraseChip);
            Ping();

            if(NORInformation.ChipType != NORInfo.ChipTypes.SamsungK8Q2815Uqb)
                return;
            SendCommand(FlashCommand.NOR2NdDieEnable);
            SendCommand(FlashCommand.NOREraseChip);
            Ping();
        }

        private uint GetSectorSize(uint address) {
            const uint s8Kb = 0x2000;
            const uint s64Kb = 0x10000;
            const uint s128Kb = 0x20000;

            //sector/block sizes for Samsung K8Q2815UQB
            if(NORInformation.ChipType == NORInfo.ChipTypes.SamsungK8Q2815Uqb) {
                if(address < 0x10000)
                    return s8Kb;
                if(address >= 0x10000 && address < 0x7f0000)
                    return s64Kb;
                if(address >= 0x7f0000 && address < 0x810000)
                    return s8Kb;
                if(address >= 0x810000 && address < 0xff0000)
                    return s64Kb;
                return s8Kb;
            }

            //sector/block size for all other NORs
            return s128Kb;
        }

        private bool Program(uint address, byte[] data, ProgrammingModes prgMode) {
            uint blockSIZE;

            var sSize = GetSectorSize(address * 2);
            Debug.Assert(data.Length == sSize);
            Debug.Assert((address & sSize / 2 - 1) == 0);

            SendCommand(NORInformation.ChipType == NORInfo.ChipTypes.SamsungK8Q2815Uqb && address >= 0x400000 ? FlashCommand.NOR2NdDieEnable : FlashCommand.NOR2NdDieDisable);

            FlashCommand flashCommand;
            switch(prgMode) {
                case ProgrammingModes.WordMode:
                    blockSIZE = 0x2000; // 8KB blocks
                    flashCommand = FlashCommand.NORWriteBlockWord;
                    break;
                case ProgrammingModes.UnlockBypassMode:
                    blockSIZE = 0x2000; // 8KB blocks
                    flashCommand = FlashCommand.NORWriteBlockUbm;
                    break;
                case ProgrammingModes.WriteBufferMode:
                    blockSIZE = 0x8000; // 32KB blocks
                    flashCommand = FlashCommand.NORWriteBlockWbp;
                    break;
                default:
                    return false;
            }

            var oData = ReadSector(address, sSize);
            if(ByteArraysEqual(oData, data))
                return true;

            if(!ByteArrayHasSameValues(oData, 0xff))
                EraseSector(address);

            for(uint i = 0; i < sSize; i += blockSIZE) {
                var retries = WriteRetries;
                while(retries != 0) {
                    SetAddress(address + (i / 2));
                    SendCommand(flashCommand);
                    Flush();
                    WriteBlock(ByteArrayCopy(data, i, blockSIZE));

                    // read write status byte
                    // 'K' = okay, 'T' = timeout error when writing, 'R' = Teensy receive buffer timeout, 'V' = Verification error
                    var res = Read();
                    if(res == 75)
                        break;

                    SendCommand(FlashCommand.NORReset);
                    Ping();

                    retries -= 1;
                    Debug.WriteLine(string.Format("({0}. Retry) {1}", WriteRetries - retries, res == 84 ? "RY/BY timeout error while writing!" : string.Format("Received unknown error! (Got 0x{0:X})", res)));
                } //end while

                if(retries != 0)
                    continue;
                Debug.WriteLine("Flashing failed");
                return false;
            } //end for
            return true;
        }

        public bool WriteRange(uint address, byte[] data, ProgrammingModes prgMode, bool doVerify) {
            if(data == null || data.Length == 0)
                return false;

            var dataLength = data.Length;
            var start = address;

            var sSize = GetSectorSize(address);
            var dataBlock = new byte[sSize];

            while(dataLength >= sSize) {
                Buffer.BlockCopy(data, (int) (address - start), dataBlock, 0, (int) sSize);
                if(!Program(address / 2, dataBlock, prgMode))
                    return false;
                address += sSize;
                dataLength -= (int) sSize;
                sSize = GetSectorSize(address);
                if(dataBlock.Length != sSize)
                    Array.Resize(ref dataBlock, (int) sSize);
            }
            return doVerify && Verify(start, data) || !doVerify;
        }

        public bool Verify(uint address, byte[] data) {
            const uint sectorSIZE = 0x20000; // 128KB blocks
            var start = address;

            LibMain.UpdateStatus("Verifying... sector) 0x{0:X}", address);

            var tempData = new byte[sectorSIZE];

            for(uint i = 0; i < data.Length; i += sectorSIZE) {
                address = start + i;
                var oData = ReadSector(address / 2, sectorSIZE);
                Debug.WriteLine(string.Format("{0} KB / {1} KB", (i + sectorSIZE) / 1024, data.Length / 1024));

                Buffer.BlockCopy(data, (int) i, tempData, 0, (int) sectorSIZE);
                if(ByteArraysEqual(oData, tempData))
                    continue;

                LibMain.UpdateStatus("Verification failed! Please repeat write command!");

                return false;
            }
            return true;
        }

        #region Nested type: NORInfo

        public struct NORInfo {
            #region ChipTypes enum

            public enum ChipTypes {
                Unknown = 0,
                SpansionS29Gl128 = 1,
                SamsungK8P2716Uzc = 2,
                SamsungK8Q2815Uqb = 3,
                MacronixMx29Gl128 = 4
            }

            #endregion

            public readonly uint DeviceCode;
            public readonly byte ManufacturerCode;

            internal NORInfo(IList<byte> norID) {
                Debug.Assert(norID.Count == 8);

                ManufacturerCode = norID[1];
                DeviceCode = (uint) norID[3] << 16 | (uint) norID[5] << 8 | norID[7];
            }

            public ChipTypes ChipType {
                get {
                    switch(ManufacturerCode) {
                        case 0x01:
                            if(DeviceCode == 0x7e2101)
                                return ChipTypes.SpansionS29Gl128;
                            break;
                        case 0xc2:
                            if(DeviceCode == 0x7e2101)
                                return ChipTypes.MacronixMx29Gl128;
                            break;
                        case 0xec:
                            if(DeviceCode == 0x7e6660)
                                return ChipTypes.SamsungK8P2716Uzc;
                            if(DeviceCode == 0x7e0601)
                                return ChipTypes.SamsungK8Q2815Uqb;
                            break;
                        default:
                            return ChipTypes.Unknown;
                    }
                    return ChipTypes.Unknown;
                }
            }

            public string ManufacturerName {
                get {
                    switch(ChipType) {
                        case ChipTypes.SpansionS29Gl128:
                            return "Spansion";
                        case ChipTypes.MacronixMx29Gl128:
                            return "Macronix";
                        case ChipTypes.SamsungK8P2716Uzc:
                        case ChipTypes.SamsungK8Q2815Uqb:
                            return "Samsung";
                        default:
                            return "Unknown";
                    }
                }
            }

            public string DeviceName {
                get {
                    switch(ChipType) {
                        case ChipTypes.SpansionS29Gl128:
                            return "S29GL128";
                        case ChipTypes.MacronixMx29Gl128:
                            return "MX29GL128";
                        case ChipTypes.SamsungK8P2716Uzc:
                            return "K8P2716UZC";
                        case ChipTypes.SamsungK8Q2815Uqb:
                            return "K8Q2815UQB";
                        default:
                            return "Unknown";
                    }
                }
            }
        }

        #endregion
    }
}

/*
	class NORFlasher(TeensySerial):
		def closedevice(self):
			self.close()
		
		def getargs(self):
			args = ""
			for arg in sys.argv:
				if " " in arg:
					args = args + '"' + arg + '"' + " "
				else:
					args = args + arg + " "
			if len(args) > 0:
				args = args[0:len(args)-1]
			return args

		def checkprotection(self):
			self.writeat(0x555, 0xaa)
			self.writeat(0x2aa, 0x55)
			self.writeat(0x555, 0xc0)

			print
			print "Checking sector protection..."
			for offset in range(0, 0x800000, 0x10000):
				self.addr(offset)
				self.delay(10)
				self.write("\x14")
				self.readbyte()
				val = self.readbyte()
				if ((val & 1) == 0):
					print "Sector at 0x%06x is protected!"%offset
					self.DEVICE_PROTECTED = 1

			if (self.DEVICE_PROTECTED == 0):
				print "No protected sectors found!"

			self.writeat(0x0, 0x90)
			self.writeat(0x0, 0x0)
			self.delay(10)

			self.reset = 1
			self.udelay(40)
			self.reset = 0
			self.udelay(40)
			self.ping()

		def checkchip(self):
			if (self.MF_ID == 0):
				print
				print "Unknown chip manufacturer! Exiting..."
				self.close()
				sys.exit(1)
			if (self.DEVICE_ID == 0):
				print
				print "Unknown device id! Exiting..."
				self.close()
				sys.exit(1)
			if (self.DEVICE_PROTECTED == 1):
				print
				print "Device has protected sectors! Command not supported! Exiting..."
				self.close()
				sys.exit(1)

		def printstate(self):
			state = self.state()
			self.MF_ID = self.manufacturer_id()
			self.DEVICE_ID = self.device_id()

			if self.MF_ID == 0x01:
				print "NOR chip manufacturer: Spansion (0x%02x)"%self.MF_ID
				if self.DEVICE_ID == 0x7e2101:
					print "NOR chip type: S29GL128 (0x%06x)"%self.DEVICE_ID
				else:
					print "NOR chip type: unknown (0x%06x)"%self.DEVICE_ID
					self.DEVICE_ID = 0
			elif self.MF_ID == 0xEC:
				print "NOR chip manufacturer: Samsung (0x%02x)"%self.MF_ID
				if self.DEVICE_ID =K8P2716UZC = 0x7e6660:
					print "NOR chip type: K8P2716UZC (0x%06x)"%self.DEVICE_ID
				elif self.DEVICE_ID == K8Q2815UQB = 0x7e0601:
					print "NOR chip type: K8Q2815UQB (0x%06x)"%self.DEVICE_ID
				else:
					print "NOR chip type: unknown (0x%06x)"%self.DEVICE_ID
					self.DEVICE_ID = 0
			elif self.MF_ID == 0xC2:
				print "NOR chip manufacturer: Macronix (0x%02x)"%self.MF_ID
				if self.DEVICE_ID == MX29GL128 = 0x7e2101:
					print "NOR chip type: MX29GL128 (0x%06x)"%self.DEVICE_ID
				else:
					print "NOR chip type: unknown (0x%06x)"%self.DEVICE_ID
					self.DEVICE_ID = 0
			else:
				print "NOR chip manufacturer: unknown (0x%02x)"%self.MF_ID
				print "NOR chip type: unknown (0x%06x)"%self.DEVICE_ID
				self.MF_ID = 0
				self.DEVICE_ID = 0
		
			if (self.MF_ID != 0xEC) and (self.MF_ID != 0x0) and (self.DEVICE_ID != 0x7e0601) and (self.DEVICE_ID != 0):
				self.checkprotection()

			print
			#print "{0:15} {1}".format("STATUS_TRIST_N:", bool(state & STATUS_TRIST_N))
			#print "{0:15} {1}".format("STATUS_RESET_N:", bool(state & STATUS_RESET_N))
			#print "{0:15} {1}".format("STATUS_READY:", bool(state & STATUS_READY))
			#print "{0:15} {1}".format("STATUS_CE_N:", bool(state & STATUS_CE_N))
			#print "{0:15} {1}".format("STATUS_WE_N:", bool(state & STATUS_WE_N))
			#print "{0:15} {1}".format("STATUS_OE_N:", bool(state & STATUS_OE_N))
			print "{0:15} {1}".format("STATUS_TRIST_N:", "HIGH" if (state & STATUS_TRIST_N) else "LOW")
			print "{0:15} {1}".format("STATUS_RESET_N:", "HIGH" if (state & STATUS_RESET_N) else "LOW")
			print "{0:15} {1}".format("STATUS_READY:", "HIGH" if (state & STATUS_READY) else "LOW")
			print "{0:15} {1}".format("STATUS_CE_N:", "HIGH" if (state & STATUS_CE_N) else "LOW")
			print "{0:15} {1}".format("STATUS_WE_N:", "HIGH" if (state & STATUS_WE_N) else "LOW")
			print "{0:15} {1}".format("STATUS_OE_N:", "HIGH" if (state & STATUS_OE_N) else "LOW")

		#def _s_trist(self, v):
		#	self.write(0x06 | bool(v))
		#def _g_trist(self):
		#	return not (self.state() & STATUS_TRIST_N)
		#trist = property(_g_trist, _s_trist)

		*/