using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;
using LibUsbDotNet;
using LibUsbDotNet.LibUsb;
using LibUsbDotNet.Main;
using LibUsbDotNet.DeviceNotify;

namespace NANDORway.Boards
{
	public class Teensy
	{
		private const int VID = 0xB00B;
		private const int PID = 0xBABE;

		private const int EP_IN = 0x81;
		private const int EP_OUT = 0x02;

		private const int MIN_PACKET_SIZE = 64;
		private const int MAX_PACKET_SIZE = 32768; //4096;
		private const int RX_TIMEOUT = 120000;
		private const int TX_TIMEOUT = 120000;

		private Main _parent;
		private System.IO.MemoryStream _buffer;

		private UsbDevice _usbDevice;
		private UsbEndpointReader _epReader;
		private UsbEndpointWriter _epWriter;
		private IDeviceNotifier _devNotifier;

		public static UsbDeviceFinder _usbFinder = new UsbDeviceFinder(VID, PID);

		public Teensy(Main parent)
		{
			_parent = parent;
			_buffer = null;
			_devNotifier = DeviceNotifier.OpenDeviceNotifier();
			_devNotifier.OnDeviceNotify += new EventHandler<DeviceNotifyEventArgs>(_devNotifier_OnDeviceNotify);
		}

		void _devNotifier_OnDeviceNotify(object sender, DeviceNotifyEventArgs e)
		{
			if (e.Device.IdVendor != VID || e.Device.IdProduct != PID) return;

			switch (e.EventType)
			{
				case EventType.DeviceArrival:
					_parent.RaiseStatusEvent("Connected!");
					break;
				case EventType.DeviceRemoveComplete:
					_parent.RaiseStatusEvent("Disconnected!");
					CloseDevice();
					break;
				default:
					break;
			}
		}

		public void WriteBlock(byte[] data)
		{
			OpenDevice();

			if (data == null || data.Length <= 0) return;

			ErrorCode eCode;
			int tLength;

			if ((eCode = _epWriter.Write(data, TX_TIMEOUT, out tLength)) != ErrorCode.None)
			{
				Debug.Assert(false, "Write error! " + eCode.ToString());
			}
		}

		public void Write(byte[] data)
		{
			if (_buffer == null) _buffer = new System.IO.MemoryStream();
			_buffer.Write(data, 0, data.Length);
		}

		public void Write(byte data)
		{
			Write(new System.Byte[1] { data });
		}

		public byte[] Read(uint size)
		{
			Flush();

			int index = 0;
			byte[] tempBuffer = new byte[size];

			int packetSize = MIN_PACKET_SIZE;
			if (size >= MAX_PACKET_SIZE) packetSize = MAX_PACKET_SIZE;

			byte[] packetBuffer = new byte[packetSize];

			int tLength;
			ErrorCode eCode;

			while (index < size)
			{
				if (size - index < packetSize)
				{
					packetSize = MIN_PACKET_SIZE;
					Array.Resize(ref packetBuffer, packetSize);
				}

				if ((eCode = _epReader.Read(packetBuffer, RX_TIMEOUT, out tLength)) == ErrorCode.None)
				{
					if (tLength > 0)
						if (size - index >= packetSize)
							Buffer.BlockCopy(packetBuffer, 0, tempBuffer, index, packetSize);
						else
							Buffer.BlockCopy(packetBuffer, 0, tempBuffer, index, (int)size - index);
					else
						Debug.Assert(false, "Nothing received!");
				}
				else
				{
					Debug.Assert(false, "Read error! " + eCode.ToString());
				}

				if (size - index >= packetSize)
					index += packetSize;
				else
					index += (int)size - index;

				//if (packetSize == MAX_PACKET_SIZE) _parent.RxBytes += packetSize;
				//if (_parent.RxBytes % (1024 * 128) == 0)
				//{
				//   _parent.RaiseProgressEvent(string.Format("Dumping: {0} KB / 16384 KB", _parent.RxBytes / 1024));
				//   System.Windows.Forms.Application.DoEvents();
				//}
			}

			return tempBuffer;
		}

		public byte Read()
		{
			return Read(1)[0];
		}

		public void Flush()
		{
			OpenDevice();

			if (_buffer == null || _buffer.Length <= 0) return;
			
			ErrorCode eCode;
			int tLength;
			int bytesRead = 0;
			int packetBufferStartIndex = 2;

			int packetSize = MIN_PACKET_SIZE;
			if (_buffer.Length >= MAX_PACKET_SIZE) packetSize = MAX_PACKET_SIZE;

			byte[] packetBuffer = new byte[packetSize];

			Debug.Assert(_buffer.Length <= ushort.MaxValue);

			packetBuffer[0] = (byte)(_buffer.Length >> 8);
			packetBuffer[1] = (byte)(_buffer.Length & 0xff);

			_buffer.Position = 0;

			while (_buffer.Length > _buffer.Position)
			{
				if (packetBuffer.Length > _buffer.Length - _buffer.Position)
				{
					Array.Resize(ref packetBuffer, MIN_PACKET_SIZE);
				}

				bytesRead = _buffer.Read(packetBuffer, packetBufferStartIndex, packetBuffer.Length - packetBufferStartIndex);
				packetBufferStartIndex = 0;

				if ((eCode = _epWriter.Write(packetBuffer, TX_TIMEOUT, out tLength)) != ErrorCode.None)
				{
					Debug.Assert(false, "Write error! " + eCode.ToString());
					break;
				}

				//if (packetBuffer.Length == PACKET_SIZE) _parent.TxBytes += packetBuffer.Length;
				//if (_parent.TxBytes % (1024 * 128) == 0)
				//{
				//   _parent.RaiseProgressEvent(string.Format("Writing: {0} KB / 16384 KB", _parent.TxBytes / 1024));
				//   System.Windows.Forms.Application.DoEvents();
				//}
			}

			_buffer.Close();
			_buffer = null;
		}

		private void OpenDevice()
		{
			if (_usbDevice != null && _usbDevice.IsOpen) return;

			// Find and open the usb device.
			_usbDevice = UsbDevice.OpenUsbDevice(_usbFinder);

			// If the device is open and ready
			Debug.Assert(_usbDevice != null);

			// If this is a "whole" usb device (libusb-win32, linux libusb)
			// it will have an IUsbDevice interface. If not (WinUSB) the 
			// variable will be null indicating this is an interface of a 
			// device.
			IUsbDevice wholeUsbDevice = _usbDevice as IUsbDevice;
			if (!ReferenceEquals(wholeUsbDevice, null))
			{
				// This is a "whole" USB device. Before it can be used, 
				// the desired configuration and interface must be selected.

				// Select config #1
				wholeUsbDevice.SetConfiguration(1);

				// Claim interface #0.
				wholeUsbDevice.ClaimInterface(0);
			}

			// open read endpoint 1.
			_epReader = _usbDevice.OpenEndpointReader((ReadEndpointID)EP_IN);
			_epReader.Flush();

			// open write endpoint 1.
			_epWriter = _usbDevice.OpenEndpointWriter((WriteEndpointID)EP_OUT);
		}

		public void CloseDevice()
		{
			if (_usbDevice != null)
			{
				if (_usbDevice.IsOpen)
				{
					if (_epReader != null)
					{
						_epReader.Dispose();
						_epReader = null;
					}
					if (_epWriter != null)
					{
						_epWriter.Abort();
						_epWriter.Dispose();
						_epWriter = null;
					}

					// If this is a "whole" usb device (libusb-win32, linux libusb)
					// it will have an IUsbDevice interface. If not (WinUSB) the 
					// variable will be null indicating this is an interface of a 
					// device.
					IUsbDevice wholeUsbDevice = _usbDevice as IUsbDevice;
					if (!ReferenceEquals(wholeUsbDevice, null))
					{
						// Release interface #0.
						wholeUsbDevice.ReleaseInterface(0);
					}

					_usbDevice.Close();
					_usbDevice = null;
				}
			}
		}
	}
}
