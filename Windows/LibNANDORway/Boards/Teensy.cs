namespace LibNANDORway.Boards {
    using System;
    using System.Diagnostics;
    using System.IO;
    using LibUsbDotNet;
    using LibUsbDotNet.DeviceNotify;
    using LibUsbDotNet.Main;

    public sealed class Teensy : IBoard {
        private readonly int _vid;
        private readonly int _pid;
        private const int EpIn = 0x81;
        private const int EpOut = 0x02;

        private const int MinPacketSIZE = 0x40;
        private const int MaxPacketSIZE = 0x8000; //4096;
        private const int RxTimeout = 0x1D4C0;
        private const int TxTimeout = 0x1D4C0;
        private readonly UsbDeviceFinder _usbFinder;
        private readonly IDeviceNotifier _devNotifier;

        private MemoryStream _buffer;

        private UsbEndpointReader _epReader;
        private UsbEndpointWriter _epWriter;
        private UsbDevice _usbDevice;

        public Teensy(int pid, int vid) {
            _vid = vid;
            _pid = pid;
            _usbFinder = new UsbDeviceFinder(_vid, _pid);
            _buffer = null;
            _devNotifier = DeviceNotifier.OpenDeviceNotifier();
            _devNotifier.OnDeviceNotify += OnDeviceNotify;
        }

        ~Teensy() {
            CloseDevice();
        }

        private void OnDeviceNotify(object sender, DeviceNotifyEventArgs e) {
            if(e.Device.IdVendor != _vid || e.Device.IdProduct != _pid)
                return; //Ignore this one!

            switch(e.EventType) {
                case EventType.DeviceArrival:
                    LibMain.OnConnectedChanged(true); // Notify we found device
                    break;
                case EventType.DeviceRemoveComplete:
                    CloseDevice();
                    LibMain.OnConnectedChanged(false); // Notify we lost the device
                    break;
            }
        }

        public void WriteBlock(byte[] data) {
            OpenDevice();

            if(data == null || data.Length <= 0)
                return;

            int tLength;
            var eCode = _epWriter.Write(data, TxTimeout, out tLength);
            Debug.Assert(eCode != ErrorCode.None, string.Format("Write error! {0}", eCode));
        }

        public void Write(byte[] data) {
            if(_buffer == null)
                _buffer = new MemoryStream();
            _buffer.Write(data, 0, data.Length);
        }

        public void Write(byte data) {
            Write(new[] { data });
        }

        public byte[] Read(uint size) {
            Flush();

            var index = 0;
            var tempBuffer = new byte[size];

            var packetSize = size >= MaxPacketSIZE ? MaxPacketSIZE : MinPacketSIZE;

            var packetBuffer = new byte[packetSize];

            while(index < size) {
                if(size - index < packetSize) {
                    packetSize = MinPacketSIZE;
                    Array.Resize(ref packetBuffer, packetSize);
                }

                int tLength;
                var eCode = _epReader.Read(packetBuffer, RxTimeout, out tLength);
                Debug.Assert(eCode == ErrorCode.None, string.Format("Read error! {0}", eCode));
                Debug.Assert(tLength > 0, "Nothing received!");
                Buffer.BlockCopy(packetBuffer, 0, tempBuffer, index, size - index >= packetSize ? packetSize : (int) size - index);
                index += size - index >= packetSize ? packetSize : (int) size - index;
            }

            return tempBuffer;
        }

        public byte Read() {
            return Read(1)[0];
        }

        public void Flush() {
            OpenDevice();

            if(_buffer == null || _buffer.Length <= 0)
                return;

            var packetBufferStartIndex = 2;

            var packetSize = _buffer.Length >= MaxPacketSIZE ? MaxPacketSIZE : MinPacketSIZE;

            var packetBuffer = new byte[packetSize];

            Debug.Assert(_buffer.Length <= ushort.MaxValue);

            packetBuffer[0] = (byte) (_buffer.Length >> 8);
            packetBuffer[1] = (byte) (_buffer.Length & 0xff);

            _buffer.Position = 0;

            while(_buffer.Length > _buffer.Position) {
                if(packetBuffer.Length > _buffer.Length - _buffer.Position)
                    Array.Resize(ref packetBuffer, MinPacketSIZE);

                _buffer.Read(packetBuffer, packetBufferStartIndex, packetBuffer.Length - packetBufferStartIndex);
                packetBufferStartIndex = 0;
                int tLength;

                var eCode = _epWriter.Write(packetBuffer, TxTimeout, out tLength);
                Debug.Assert(eCode == ErrorCode.None, string.Format("Write error! {0}", eCode));

                //if (packetBuffer.Length == PACKET_SIZE) _parent.TxBytes += packetBuffer.Length;
                //if (_parent.TxBytes % (1024 * 128) == 0)
                //{
                //   _parent.RaiseProgressEvent(string.Format("Writing: {0} KB / 16384 KB", _parent.TxBytes / 1024));
                //}
            }

            _buffer.Close();
            _buffer = null;
        }

        private void OpenDevice() {
            if(_usbDevice != null && _usbDevice.IsOpen)
                return;

            // Find and open the usb device.
            _usbDevice = UsbDevice.OpenUsbDevice(_usbFinder);

            // If the device is open and ready
            Debug.Assert(_usbDevice != null, string.Format("No device with PID: {0:X4} & VID: {1:X4} Found!", _pid, _vid));

            // If this is a "whole" usb device (libusb-win32, linux libusb)
            // it will have an IUsbDevice interface. If not (WinUSB) the 
            // variable will be null indicating this is an interface of a 
            // device.
            var wholeUsbDevice = _usbDevice as IUsbDevice;
            if(!ReferenceEquals(wholeUsbDevice, null)) {
                // This is a "whole" USB device. Before it can be used, 
                // the desired configuration and interface must be selected.

                // Select config #1
                wholeUsbDevice.SetConfiguration(1);

                // Claim interface #0.
                wholeUsbDevice.ClaimInterface(0);
            }

            // open read endpoint 1.
            _epReader = _usbDevice.OpenEndpointReader((ReadEndpointID) EpIn);
            _epReader.Flush();

            // open write endpoint 1.
            _epWriter = _usbDevice.OpenEndpointWriter((WriteEndpointID) EpOut);
            LibMain.OnConnectedChanged(true);
        }

        public void CloseDevice() {
            if(_usbDevice == null || !_usbDevice.IsOpen)
                return;
            if(_epReader != null) {
                _epReader.Dispose();
                _epReader = null;
            }
            if(_epWriter != null) {
                _epWriter.Abort();
                _epWriter.Dispose();
                _epWriter = null;
            }

            // If this is a "whole" usb device (libusb-win32, linux libusb)
            // it will have an IUsbDevice interface. If not (WinUSB) the 
            // variable will be null indicating this is an interface of a 
            // device.
            var wholeUsbDevice = _usbDevice as IUsbDevice;
            if(!ReferenceEquals(wholeUsbDevice, null))
                wholeUsbDevice.ReleaseInterface(0); // Release interface #0.

            _usbDevice.Close();
            _usbDevice = null;
        }
    }
}