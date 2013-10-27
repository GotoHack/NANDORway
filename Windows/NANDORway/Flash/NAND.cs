using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;

namespace NANDORway.Flash
{
	class NAND : FlashBase
	{
		public NAND(Boards.Teensy board)
			: base(board)
		{
			NANDInformation = new NANDInfo();
		}

		public enum NANDSelect
		{
			NAND_0 = 0,
			NAND_1 = 1,
			DUAL_NAND = 2
		}

		public struct NANDInfo
		{
			public NANDInfo(byte[] nandID)
			{
				Debug.Assert(nandID.Length == 24);

				ManufacturerCode = nandID[0];
				DeviceCode = nandID[1];
				PageSize = ((uint)nandID[2] << 24) | ((uint)nandID[3] << 16) | ((uint)nandID[4] << 8) | (uint)nandID[5];
				SpareSize = nandID[6];
				BlockSize = ((uint)nandID[7] << 24) | ((uint)nandID[8] << 16) | ((uint)nandID[9] << 8) | (uint)nandID[10];
				NumberOfBlocks = ((uint)nandID[11] << 24) | ((uint)nandID[12] << 16) | ((uint)nandID[13] << 8) | (uint)nandID[14];
				NumberOfPlanes = nandID[15];
				PlaneSize = ((uint)nandID[16] << 56) | ((uint)nandID[17] << 48) | ((uint)nandID[18] << 40) | ((uint)nandID[19] << 32) | ((uint)nandID[20] << 24) | ((uint)nandID[21] << 16) | ((uint)nandID[22] << 8) | (uint)nandID[23];
			}

			//public enum ChipTypes
			//{
			//   Unknown = 0,
			//   Spansion_S29GL128 = 1,
			//   Samsung_K8P2716UZC = 2,
			//   Samsung_K8Q2815UQB = 3,
			//   Macronix_MX29GL128 = 4
			//}

			public readonly byte ManufacturerCode;
			public readonly byte DeviceCode;
			public readonly uint PageSize;
			public readonly byte SpareSize;
			public readonly uint BlockSize;
			public readonly uint NumberOfBlocks;
			public readonly byte NumberOfPlanes;
			public readonly ulong PlaneSize;

			//public ChipTypes ChipType
			//{
			//   get
			//   {
			//      switch (ManufacturerId)
			//      {
			//         case 0x01:
			//            if (DeviceId == 0x7e2101) return ChipTypes.Spansion_S29GL128;
			//            break;
			//         case 0xc2:
			//            if (DeviceId == 0x7e2101) return ChipTypes.Macronix_MX29GL128;
			//            break;
			//         case 0xec:
			//            if (DeviceId == 0x7e6660) return ChipTypes.Samsung_K8P2716UZC;
			//            if (DeviceId == 0x7e0601) return ChipTypes.Samsung_K8Q2815UQB;
			//            break;
			//         default:
			//            return ChipTypes.Unknown;
			//      }
			//      return ChipTypes.Unknown;
			//   }
			//}

			//public string ManufacturerName
			//{
			//   get
			//   {
			//      switch (ChipType)
			//      {
			//         case ChipTypes.Spansion_S29GL128:
			//            return "Spansion";
			//         case ChipTypes.Macronix_MX29GL128:
			//            return "Macronix";
			//         case ChipTypes.Samsung_K8P2716UZC:
			//         case ChipTypes.Samsung_K8Q2815UQB:
			//            return "Samsung";
			//         default:
			//            return "Unknown";
			//      }
			//   }
			//}

			//public string DeviceName
			//{
			//   get
			//   {
			//      switch (ChipType)
			//      {
			//         case ChipTypes.Spansion_S29GL128:
			//            return "S29GL128";
			//         case ChipTypes.Macronix_MX29GL128:
			//            return "MX29GL128";
			//         case ChipTypes.Samsung_K8P2716UZC:
			//            return "K8P2716UZC";
			//         case ChipTypes.Samsung_K8Q2815UQB:
			//            return "K8Q2815UQB";
			//         default:
			//            return "Unknown";
			//      }
			//   }
			//}
		}

		public NANDInfo NANDInformation { get; private set; }

		private byte[] GetNAND_ID(byte nandID)
		{
			SendCommand(FlashCommand.CMD_NAND_ID_SET);
			Write(nandID);
			SendCommand(FlashCommand.CMD_NAND_ID);
			return Read(24);
		}

		public void NANDInformationRefresh(byte nandID)
		{
			Ping();
			SendCommand(FlashCommand.CMD_IO_LOCK);

			NANDInformation = new NANDInfo(GetNAND_ID(nandID));
		}

		public byte[] ReadBlock(uint block, uint size)
		{
			SendCommand(FlashCommand.CMD_NAND_BLOCK_SET);
			Write(new byte[] { (byte)(block >> 8), (byte)(block & 0xFF) });
			SendCommand(FlashCommand.CMD_NAND_READ_BLOCK_RAW);

			return Read(size);
		}


	}
}
