namespace LibNANDORway.Flash {
    using System;
    using System.Diagnostics;
    using LibNANDORway.Boards;

    public class FlashBase {
        private readonly IBoard _board;

        protected FlashBase(IBoard board)
        {
            _board = board;
        }

        public int TxBytes { get; set; }
        public int RxBytes { get; set; }

        protected byte Read() {
            return _board.Read();
        }

        protected byte[] Read(uint size) {
            return _board.Read(size);
        }

        protected void Write(byte data) {
            _board.Write(data);
        }

        protected void Write(byte[] data) {
            _board.Write(data);
        }

        protected void WriteBlock(byte[] data) {
            _board.WriteBlock(data);
        }

        protected void Flush() {
            _board.Flush();
        }

        public bool Ping() {
            SendCommand(FlashCommand.Ping);
            var data = Read(2);

            return data.Length == 2 && data[0] == 0x42 && data[1] == 0xBD;
        }

        public Version GetVersion() {
            SendCommand(FlashCommand.Getversion);
            var ret = Read(3);
            return new Version(ret[0], ret[1], ret[2]);
        }

        protected void SendCommand(FlashCommand command) {
            Write((byte) command);
        }

        public byte[] SpeedTestRead() {
            SendCommand(FlashCommand.SpeedtestRead);
            return Read(0x20000);
        }

        public void SpeedTestWrite(byte[] data) {
            const uint sectorSIZE = 0x20000; // 128KB blocks
            const uint blockSIZE = 0x8000; // 32KB blocks

            Debug.Assert(data.Length == sectorSIZE);

            var tempData = new byte[blockSIZE];
            for(uint i = 0; i < sectorSIZE; i += blockSIZE) {
                Buffer.BlockCopy(data, (int) i, tempData, 0, (int) blockSIZE);
                SendCommand(FlashCommand.SpeedtestWrite);
                Flush();
                WriteBlock(tempData);

                // read write status byte
                // 'K' = okay, 'T' = timeout error when writing, 'R' = Teensy receive buffer timeout
                var res = (FlashResponse)Read();
                if(res != FlashResponse.Success)
                    Debug.WriteLine(string.Format("Error: {0}", res));
            }
        }

        protected static void ByteArrayMemSet(byte[] a, byte value, int length) {
            if(length > a.Length)
                length = a.Length;

            for(var i = 0; i < length; ++i)
                a[i] = value;
        }

        public static bool ByteArraysEqual(byte[] a1, byte[] a2) {
            if(a1.Length == a2.Length) {
                for(var i = 0; i < a1.Length; i++) {
                    if(a1[i] == a2[i])
                        continue;
                    return false;
                }
                return true;
            }
            return false;
        }

        public static bool ByteArrayHasSameValues(byte[] data, byte value) {
            for(var i = 0; i < data.Length; i++) {
                if(data[i] != value)
                    return false;
            }

            return data.Length != 0;
        }

        protected static byte[] ByteArrayCopy(byte[] data, uint startIndex, uint length) {
            var tempData = new byte[length];
            Buffer.BlockCopy(data, (int) startIndex, tempData, 0, (int) length);
            return tempData;
        }

        public void EnterBootLoader()
        {
            SendCommand(FlashCommand.Bootloader);
            Flush();
        }

        #region Nested type: FlashCommand

        protected enum FlashCommand {
            Getversion = 0,
            Ping,
            Bootloader,
            SpeedtestRead,
            SpeedtestWrite,
            IoLock,
            IoRelease,
            NORID,
            NORReset,
            NOREraseSector,
            NOREraseChip,
            NORAddressSet,
            NORAddressIncrement,
            NORAddressIncrementEnable,
            NORAddressIncrementDisable,
            NOR2NdDieEnable,
            NOR2NdDieDisable,
            NORReadWord,
            NORReadBlock4Kb,
            NORReadBlock8Kb,
            NORReadBlock64Kb,
            NORReadBlock128Kb,
            NORWriteWord,
            NORWriteBlockWord,
            NORWriteBlockUbm,
            NORWriteBlockWbp,
            NANDID,
            NANDIDSet,
            NANDBlockSet,
            NANDReadBlockECC,
            NANDReadBlockRaw,
        }

        #endregion

        protected enum FlashResponse : byte {
            Success = 0,
            Error,
            Timeout
        }
    }
}