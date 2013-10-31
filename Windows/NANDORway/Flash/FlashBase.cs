using System;
using System.Diagnostics;

namespace NANDORway.Flash
{
	public class FlashBase
	{
		public FlashBase(Boards.Teensy board)
		{
			_boardTeensy = board;
		}

		private Boards.Teensy _boardTeensy;
		private Boards.UBW32 _boardUBW32;

		public int TxBytes { get; set; }
		public int RxBytes { get; set; }

		public enum FlashCommand
		{
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

		protected byte Read()
		{
			return _boardTeensy.Read();
		}

		protected byte[] Read(uint size)
		{
			return _boardTeensy.Read(size);
		}

		protected void Write(byte data)
		{
			_boardTeensy.Write(data);
		}

		protected void Write(byte[] data)
		{
			_boardTeensy.Write(data);
		}

		protected void WriteBlock(byte[] data)
		{
			_boardTeensy.WriteBlock(data);
		}

		protected void Flush()
		{
			_boardTeensy.Flush();
		}

		public bool Ping()
		{
			SendCommand(FlashCommand.CMD_PING);
			byte[] data = Read(2);

			if (ByteArraysEqual(data, new byte[] { 0x42, 0xbd }))
				return true;

			return false;
		}

		public void SendCommand(FlashCommand command)
		{
			Write((byte)command);
		}

		public byte[] SpeedTestRead()
		{
			SendCommand(FlashCommand.CMD_SPEEDTEST_READ);
			return Read(0x20000);
		}

		public void SpeedTestWrite(byte[] data)
		{
			const uint SECTOR_SIZE = 0x20000; // 128KB blocks
			const uint BLOCK_SIZE = 0x8000; // 32KB blocks

			Debug.Assert(data.Length == SECTOR_SIZE);

			byte[] tempData = new byte[BLOCK_SIZE];
			for (uint i = 0; i < SECTOR_SIZE; i += BLOCK_SIZE)
			{
				Buffer.BlockCopy(data, (int)i, tempData, 0, (int)BLOCK_SIZE);
				SendCommand(FlashCommand.CMD_SPEEDTEST_WRITE);
				Flush();
				WriteBlock(tempData);

				// read write status byte
				// 'K' = okay, 'T' = timeout error when writing, 'R' = Teensy receive buffer timeout
				byte res = Read();
				if (res != 75)
					Debug.WriteLine(string.Format("Error: {0:H}", res));
			}
		}

		protected static void ByteArrayMemSet(byte[] a, byte value, int length)
		{
			if (length > a.Length) length = a.Length;

			for (int i = 0; i < length; ++i)
				a[i] = value;
		}

		public static bool ByteArraysEqual(byte[] a1, byte[] a2)
		{
			if (a1.Length == a2.Length)
			{
				for (int i = 0; i < a1.Length; i++)
				{
					if (a1[i] != a2[i])
					{
						return false;
					}
				}
				return true;
			}
			return false;
		}

		public static bool ByteArrayHasSameValues(byte[] data, byte value)
		{
			for (int i = 0; i < data.Length; i++)
				if (data[i] != value)
					return false;

			if (data.Length == 0) return false;
			return true;
		}

		protected static byte[] ByteArrayCopy(byte[] data, uint startIndex, uint length)
		{
			byte[] tempData = new byte[length];
			Buffer.BlockCopy(data, (int)startIndex, tempData, 0, (int)length);
			return tempData;
		}


	}
}
