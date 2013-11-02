namespace LibNANDORway.Boards {
    public interface IBoard {

        void WriteBlock(byte[] data);

        void Write(byte[] data);

        void Write(byte data);

        byte[] Read(uint size);

        byte Read();

        void Flush();

        void CloseDevice();
    }
}
