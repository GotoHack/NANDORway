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
            SendCommand(FlashCommand.CMD_PING);
            var data = Read(2);

            return ByteArraysEqual(data, new byte[] { 0x42, 0xbd });
        }

        protected void SendCommand(FlashCommand command) {
            Write((byte) command);
        }

        public byte[] SpeedTestRead() {
            SendCommand(FlashCommand.CMD_SPEEDTEST_READ);
            return Read(0x20000);
        }

        public void SpeedTestWrite(byte[] data) {
            const uint sectorSIZE = 0x20000; // 128KB blocks
            const uint blockSIZE = 0x8000; // 32KB blocks

            Debug.Assert(data.Length == sectorSIZE);

            var tempData = new byte[blockSIZE];
            for(uint i = 0; i < sectorSIZE; i += blockSIZE) {
                Buffer.BlockCopy(data, (int) i, tempData, 0, (int) blockSIZE);
                SendCommand(FlashCommand.CMD_SPEEDTEST_WRITE);
                Flush();
                WriteBlock(tempData);

                // read write status byte
                // 'K' = okay, 'T' = timeout error when writing, 'R' = Teensy receive buffer timeout
                var res = Read();
                if(res != 75)
                    Debug.WriteLine(string.Format("Error: {0:H}", res));
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
            SendCommand(FlashCommand.CMD_BOOTLOADER);
            Flush();
        }

        #region Nested type: FlashCommand

        protected enum FlashCommand {
            CMD_PING = 0,
            CMD_BOOTLOADER,
            CMD_SPEEDTEST_READ,
            CMD_SPEEDTEST_WRITE,
            CMD_IO_LOCK,
            CMD_IO_RELEASE,
            CMD_NOR_ID,
            CMD_NOR_RESET,
            CMD_NOR_ERASE_SECTOR,
            CMD_NOR_ERASE_CHIP,
            CMD_NOR_ADDRESS_SET,
            CMD_NOR_ADDRESS_INCREMENT,
            CMD_NOR_ADDRESS_INCREMENT_ENABLE,
            CMD_NOR_ADDRESS_INCREMENT_DISABLE,
            CMD_NOR_2ND_DIE_ENABLE,
            CMD_NOR_2ND_DIE_DISABLE,
            CMD_NOR_READ_WORD,
            CMD_NOR_READ_BLOCK_4KB,
            CMD_NOR_READ_BLOCK_8KB,
            CMD_NOR_READ_BLOCK_64KB,
            CMD_NOR_READ_BLOCK_128KB,
            CMD_NOR_WRITE_WORD,
            CMD_NOR_WRITE_BLOCK_WORD,
            CMD_NOR_WRITE_BLOCK_UBM,
            CMD_NOR_WRITE_BLOCK_WBP,
            CMD_NAND_ID,
            CMD_NAND_ID_SET,
            CMD_NAND_BLOCK_SET,
            CMD_NAND_READ_BLOCK_ECC,
            CMD_NAND_READ_BLOCK_RAW,
        }

        #endregion
    }
}