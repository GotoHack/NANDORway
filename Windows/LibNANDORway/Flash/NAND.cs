namespace LibNANDORway.Flash
{
    using System.Collections.Generic;
    using System.Diagnostics;

    public sealed class NAND : FlashBase
	{
        public NAND(Boards.IBoard board) : base(board)
		{
			NANDInformation = new NANDInfo();
		}

		public enum NANDSelect
		{
			NAND0 = 0,
			NAND1 = 1,
			DualNAND = 2
		}

		public struct NANDInfo
		{
		    internal NANDInfo(IList<byte> nandID)
			{
				Debug.Assert(nandID.Count == 0x18);

				ManufacturerCode = nandID[0];
				DeviceCode = nandID[1];
				PageSize = (uint) (nandID[2] << 24 | nandID[3] << 16 | nandID[4] << 8 | nandID[5]);
				SpareSize = nandID[6];
				BlockSize = (uint) (nandID[7] << 24 | nandID[8] << 16 | nandID[9] << 8 | nandID[10]);
				NumberOfBlocks = (uint) (nandID[11] << 24 | nandID[12] << 16 | nandID[13] << 8 | nandID[14]);
				NumberOfPlanes = nandID[15];
				PlaneSize = (ulong) (nandID[16] << 56 | nandID[17] << 48 | nandID[18] << 40 | nandID[19] << 32 | nandID[20] << 24 | nandID[21] << 16 | nandID[22] << 8 | nandID[23]);
			}

			public readonly byte ManufacturerCode;
			public readonly byte DeviceCode;
			public readonly uint PageSize;
			public readonly byte SpareSize;
			public readonly uint BlockSize;
			public readonly uint NumberOfBlocks;
			public readonly byte NumberOfPlanes;
			public readonly ulong PlaneSize;
		}

		public NANDInfo NANDInformation { get; private set; }

		private byte[] GetNANDID(byte nandID)
		{
			SendCommand(FlashCommand.NANDIDSet);
			Write(nandID);
			SendCommand(FlashCommand.NANDID);
			return Read(0x18);
		}

		public void NANDInformationRefresh(byte nandID)
		{
			Ping();
			SendCommand(FlashCommand.IoLock);

			NANDInformation = new NANDInfo(GetNANDID(nandID));
		}

		public byte[] ReadBlock(uint block, uint size)
		{
			SendCommand(FlashCommand.NANDBlockSet);
			Write(new[] { (byte)(block >> 8), (byte)(block & 0xFF) });
			SendCommand(FlashCommand.NANDReadBlockRaw);

			return Read(size);
		}


	}
}
