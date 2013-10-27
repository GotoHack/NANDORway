using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;

namespace NANDORway.Flash
{
	class NOR : FlashBase
	{
		private const int WRITE_RETRIES = 5;

		public NOR(Boards.Teensy board)
			: base(board)
		{
			NORInformation = new NORInfo();
		}

		public enum ProgrammingModes
		{
			WordMode = 0,
			UnlockBypassMode = 1,
			WriteBufferMode = 2
		}

		public struct NORInfo
		{
			public NORInfo(byte[] norID)
			{
				Debug.Assert(norID.Length == 8);

				ManufacturerCode = norID[1];
				DeviceCode = ((uint)norID[3] << 16) | ((uint)norID[5] << 8) | (uint)norID[7];
			}

			public enum ChipTypes
			{
				Unknown = 0,
				Spansion_S29GL128 = 1,
				Samsung_K8P2716UZC = 2,
				Samsung_K8Q2815UQB = 3,
				Macronix_MX29GL128 = 4
			}

			public readonly byte ManufacturerCode;
			public readonly uint DeviceCode;

			public ChipTypes ChipType
			{
				get
				{
					switch (ManufacturerCode)
					{
						case 0x01:
							if (DeviceCode == 0x7e2101) return ChipTypes.Spansion_S29GL128;
							break;
						case 0xc2:
							if (DeviceCode == 0x7e2101) return ChipTypes.Macronix_MX29GL128;
							break;
						case 0xec:
							if (DeviceCode == 0x7e6660) return ChipTypes.Samsung_K8P2716UZC;
							if (DeviceCode == 0x7e0601) return ChipTypes.Samsung_K8Q2815UQB;
							break;
						default:
							return ChipTypes.Unknown;
					}
					return ChipTypes.Unknown;
				}
			}

			public string ManufacturerName
			{
				get
				{
					switch (ChipType)
					{
						case ChipTypes.Spansion_S29GL128:
							return "Spansion";
						case ChipTypes.Macronix_MX29GL128:
							return "Macronix";
						case ChipTypes.Samsung_K8P2716UZC:
						case ChipTypes.Samsung_K8Q2815UQB:
							return "Samsung";
						default:
							return "Unknown";
					}
				}
			}

			public string DeviceName
			{
				get
				{
					switch (ChipType)
					{
						case ChipTypes.Spansion_S29GL128:
							return "S29GL128";
						case ChipTypes.Macronix_MX29GL128:
							return "MX29GL128";
						case ChipTypes.Samsung_K8P2716UZC:
							return "K8P2716UZC";
						case ChipTypes.Samsung_K8Q2815UQB:
							return "K8Q2815UQB";
						default:
							return "Unknown";
					}
				}
			}
		}

		public NORInfo NORInformation { get; private set; }

		private void Address(uint address)
		{
			Debug.Assert(address <= 0x7FFFFF);
			SendCommand(FlashCommand.CMD_NOR_ADDRESS_SET);
			Write(new byte[] { (byte)(address >> 16), (byte)((address >> 8) & 0xff), (byte)(address & 0xff) });
		}

		private void WriteWord(ushort data, bool incrementAddress)
		{
			if (incrementAddress)
				SendCommand(FlashCommand.CMD_NOR_ADDRESS_INCREMENT_ENABLE);
			else
				SendCommand(FlashCommand.CMD_NOR_ADDRESS_INCREMENT_DISABLE);

			SendCommand(FlashCommand.CMD_NOR_WRITE_WORD);
			Write(new byte[] { (byte)((data >> 8) & 0xff), (byte)(data & 0xff) });
		}

		private void WriteAt(uint address, ushort data)
		{
			Address(address);
			WriteWord(data, false);
		}

		private byte[] GetNOR_ID()
		{
			SendCommand(FlashCommand.CMD_NOR_ID);
			return Read(8);
		}

		public void NORInformationRefresh()
		{
			Ping();
			SendCommand(FlashCommand.CMD_IO_LOCK);
			SendCommand(FlashCommand.CMD_NOR_RESET);

			NORInformation = new NORInfo(GetNOR_ID());
		}

		public void EnterBootLoader()
		{
			SendCommand(FlashCommand.CMD_BOOTLOADER);
			Flush();
		}

		public byte[] ReadSector(uint offset, uint blockSize)
		{
			Debug.Assert((offset & (blockSize / 2 - 1)) == 0);

			Address(offset);

			switch (blockSize)
			{
				case 0x1000:
					SendCommand(FlashCommand.CMD_NOR_READ_BLOCK_4KB);
					break;
				case 0x2000:
					SendCommand(FlashCommand.CMD_NOR_READ_BLOCK_8KB);
					break;
				case 0x10000:
					SendCommand(FlashCommand.CMD_NOR_READ_BLOCK_64KB);
					break;
				case 0x20000:
					SendCommand(FlashCommand.CMD_NOR_READ_BLOCK_128KB);
					break;
				default:
					return new byte[] { };
			}

			return Read(blockSize);
		}

		public void EraseSector(uint offset)
		{
			if ((NORInformation.ChipType == NORInfo.ChipTypes.Samsung_K8Q2815UQB) && (offset >= 0x400000))
				SendCommand(FlashCommand.CMD_NOR_2ND_DIE_ENABLE);
			else
				SendCommand(FlashCommand.CMD_NOR_2ND_DIE_DISABLE);

			Address(offset);
			SendCommand(FlashCommand.CMD_NOR_ERASE_SECTOR);
			bool res = Ping();
			Debug.Assert(res);
		}

		public void EraseChip()
		{
			SendCommand(FlashCommand.CMD_NOR_RESET);

			SendCommand(FlashCommand.CMD_NOR_2ND_DIE_DISABLE);
			SendCommand(FlashCommand.CMD_NOR_ERASE_CHIP);
			Ping();

			if (NORInformation.ChipType == NORInfo.ChipTypes.Samsung_K8Q2815UQB)
			{
				SendCommand(FlashCommand.CMD_NOR_2ND_DIE_ENABLE);
				SendCommand(FlashCommand.CMD_NOR_ERASE_CHIP);
				Ping();
			}
		}

		private uint GetSectorSize(uint address)
		{
			const uint s8kb = 0x2000;
			const uint s64kb = 0x10000;
			const uint s128kb = 0x20000;

			//sector/block sizes for Samsung K8Q2815UQB
			if (NORInformation.ChipType == NORInfo.ChipTypes.Samsung_K8Q2815UQB)
			{
				if (address < 0x8000 * 2)
					return s8kb;
				else if ((address >= 0x8000 * 2) && (address < 0x3f8000 * 2))
					return s64kb;
				else if ((address >= 0x3f8000 * 2) && (address < 0x408000 * 2))
					return s8kb;
				else if ((address >= 0x408000 * 2) && (address < 0x7f8000 * 2))
					return s64kb;
				else
					return s8kb;
			}
		
			//sector/block size for all other NORs
			return s128kb;
		}

		private bool Program(uint address, byte[] data, ProgrammingModes prgMode)
		{
			uint BLOCK_SIZE;
			
			uint sSize = GetSectorSize(address * 2);
			Debug.Assert(data.Length == sSize);
			Debug.Assert((address & (sSize / 2 - 1)) == 0);

			if ((NORInformation.ChipType == NORInfo.ChipTypes.Samsung_K8Q2815UQB) && (address >= 0x400000))
				SendCommand(FlashCommand.CMD_NOR_2ND_DIE_ENABLE);
			else
				SendCommand(FlashCommand.CMD_NOR_2ND_DIE_DISABLE);

			FlashCommand flashCommand;
			switch (prgMode)
			{
				case ProgrammingModes.WordMode:
					BLOCK_SIZE = 0x2000; // 8KB blocks
					flashCommand = FlashCommand.CMD_NOR_WRITE_BLOCK_WORD;
					break;
				case ProgrammingModes.UnlockBypassMode:
					BLOCK_SIZE = 0x2000; // 8KB blocks
					flashCommand = FlashCommand.CMD_NOR_WRITE_BLOCK_UBM;
					break;
				case ProgrammingModes.WriteBufferMode:
					BLOCK_SIZE = 0x8000; // 32KB blocks
					flashCommand = FlashCommand.CMD_NOR_WRITE_BLOCK_WBP;
					break;
				default:
					return false;
			}	

			byte[] oData;
			oData = ReadSector(address, sSize);
			if (ByteArraysEqual(oData, data)) return true;
				
			if (!ByteArrayHasSameValues(oData, 0xff))
				EraseSector(address);

			for (uint i = 0; i < sSize; i += BLOCK_SIZE)
			{
				int retries = WRITE_RETRIES;
				while (retries != 0)
				{
					Address(address+(i/2));
					SendCommand(flashCommand);
					Flush();
					WriteBlock(ByteArrayCopy(data, i, BLOCK_SIZE));

					// read write status byte
					// 'K' = okay, 'T' = timeout error when writing, 'R' = Teensy receive buffer timeout, 'V' = Verification error
					byte res = Read();
					string error_msg = "";
					if (res != 75) //'K'
					{
						if (res == 84) //'T'
							error_msg = "RY/BY timeout error while writing!";
						else
							error_msg = string.Format("Received unknown error! (Got 0x{0:X})", res);

						SendCommand(FlashCommand.CMD_NOR_RESET);
						Ping();

						retries -= 1;
						Debug.WriteLine("({0}. Retry) {1}", WRITE_RETRIES - retries, error_msg);
					}
					else
						break;
				} //end while

				if (retries == 0)
				{
					Debug.WriteLine("Flashing failed");
					return false;
				}
			} //end for
			return true;
		}

		public bool WriteRange(uint address, byte[] data, ProgrammingModes prgMode, bool doVerify)
		{
			if (data == null || data.Length == 0)
				return false;

			int dataLength = data.Length;
			uint start = address;

			//if (doVerify) SendCommand(FlashCommand.CMD_VERIFY_ENABLE);
			//else SendCommand(FlashCommand.CMD_VERIFY_DISABLE);

			uint sSize = GetSectorSize(address);
			byte[] dataBlock = new byte[sSize];

			while (dataLength >= sSize)
			{
				Buffer.BlockCopy(data, (int)(address - start), dataBlock, 0, (int)sSize);
				if (!Program(address / 2, dataBlock, prgMode)) return false;
				address += sSize;
				dataLength -= (int)sSize;
				sSize = GetSectorSize(address);
				if (dataBlock.Length != sSize)
					Array.Resize(ref dataBlock, (int)sSize);
			}
			return true;
		}

		public void Verify(uint address, byte[] data)
		{
			const uint SECTOR_SIZE = 0x20000; // 128KB blocks
			uint start = address;

			Debug.WriteLine("");
			Debug.WriteLine("Verifying...");

			byte[] oData;
			byte[] tempData = new byte[SECTOR_SIZE];

			for (uint i = 0; i < data.Length; i += SECTOR_SIZE)
			{
				address = start + i;
				oData = ReadSector(address / 2, SECTOR_SIZE);
				Debug.WriteLine("{0} KB / {1} KB", (i + SECTOR_SIZE) / 1024, data.Length / 1024);

				Buffer.BlockCopy(data, (int)i, tempData, 0, (int)SECTOR_SIZE);
				if (ByteArraysEqual(oData, tempData))
					continue;

				Debug.WriteLine("");
				Debug.WriteLine("Verification failed! Please repeat write command!");

				//self.close()
				//sys.exit(1)
				break;
			}

			Debug.WriteLine("");
		}

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
