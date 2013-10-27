using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;

namespace NANDORway
{
	public partial class Main : Form
	{
		public class StatusEventArgs
		{
			public StatusEventArgs(string status) { Status = status; }
			public string Status { get; private set; }
		}
		public class ProgressEventArgs
		{
			public ProgressEventArgs(string status) { Status = status; }
			public string Status { get; private set; }
		}

		public event StatusEventHandler StatusEvent;
		public event ProgressEventHandler ProgressEvent;

		public delegate void StatusEventHandler(object sender, StatusEventArgs e);
		public delegate void ProgressEventHandler(object sender, ProgressEventArgs e);

		public void RaiseStatusEvent(string status)
		{
			if (StatusEvent != null)
				StatusEvent(this, new StatusEventArgs(status));
		}

		public void RaiseProgressEvent(string status)
		{
			if (ProgressEvent != null)
				ProgressEvent(this, new ProgressEventArgs(status));
		}

		private Boards.Teensy _boardTeensy;
		private Flash.NOR _flashNOR;
		private Flash.NAND _flashNAND;

		public Main()
		{
			InitializeComponent();
			Initialize();
		}

		private void Initialize()
		{
			SetStatus("Disconnected!");
			SetStatus2("");

			textBox1.Text = @"d:\ps3\test.bin";

			this.StatusEvent += new StatusEventHandler(Main_StatusEvent);
			this.ProgressEvent += new ProgressEventHandler(Main_ProgressEvent);

			cmbNORFlashMode.DataSource = Enum.GetValues(typeof(Flash.NOR.ProgrammingModes));
			cmbNORFlashMode.SelectedIndex = 0;

			cmbNANDid.DataSource = Enum.GetValues(typeof(Flash.NAND.NANDSelect));
			cmbNANDid.SelectedIndex = 0;

			_boardTeensy = new Boards.Teensy(this);
			_flashNOR = new Flash.NOR(_boardTeensy);
			_flashNAND = new Flash.NAND(_boardTeensy);
		}

		void Main_ProgressEvent(object sender, Main.ProgressEventArgs e)
		{
			SetStatus2(e.Status);
		}

		void Main_StatusEvent(object sender, Main.StatusEventArgs e)
		{
			SetStatus(e.Status);
		}

		private void SetStatus(string status)
		{
			toolStripStatusLabel1.Text = status;
		}

		private void SetStatus2(string status)
		{
			toolStripStatusLabel2.Text = status;
		}

		private void btnPing_Click(object sender, EventArgs e)
		{
			if (_flashNOR.Ping())
			{
				System.Media.SystemSounds.Beep.Play();
				SetStatus2("Ping okay!");
			}
			else
			{
				System.Media.SystemSounds.Exclamation.Play();
				SetStatus2("Ping failed!");
			}
		}

		private void btnNORTristate_Click(object sender, EventArgs e)
		{
		}

		private void btnSelectFile_Click(object sender, EventArgs e)
		{
			SaveFileDialog dlgSave = new SaveFileDialog();

			dlgSave.Title = "Specify Destination Filename";
			dlgSave.Filter = "Bin files (*.bin)|*.bin|All files (*.*)|*.*";
			dlgSave.FilterIndex = 1;
			dlgSave.OverwritePrompt = true;

			DialogResult r = dlgSave.ShowDialog();
			if (r == System.Windows.Forms.DialogResult.OK)
				textBox1.Text = dlgSave.FileName;
		}

		private void btnNORDump_Click(object sender, EventArgs e)
		{
			const uint BLOCK = 0x10000;
			System.IO.FileStream fs = null;

			_flashNOR.NORInformationRefresh();
			SetStatus(_flashNOR.NORInformation.ManufacturerName + "/" + _flashNOR.NORInformation.DeviceName);

			//if (_flashNOR.NORInformation.ChipType == Flash.NOR.NORInfo.ChipTypes.Unknown)
			//{
			//   SetStatus2("Unknown flash type. Dump aborted..");
			//   return;
			//}

			try
			{
				Stopwatch sw = Stopwatch.StartNew();

				SetStatus2("Dumping: 0 KB / 16384 KB");
				
				fs = new System.IO.FileStream(textBox1.Text, System.IO.FileMode.Create);
				
				_flashNOR.RxBytes = 0;
				for (uint i = 0; i < 0x800000; i += BLOCK)
				{
					fs.Write(_flashNOR.ReadSector(i, 0x20000), 0, 0x20000);
					SetStatus2(string.Format("Dumping: {0} KB / 16384 KB", (i + BLOCK) / 512));
					System.Windows.Forms.Application.DoEvents();
				}

				sw.Stop();
				TimeSpan ts = sw.Elapsed;
				//TimeSpan ts2 = sw.Elapsed;
				//for (int i = 0; i < 15; i++)
				//   ts = ts.Add(ts2);

				SetStatus2(string.Format("Dumping done. [{0:00}:{1:00}:{2:00} ({3:F2} KB/s)]", ts.Hours, ts.Minutes, ts.Seconds, 0x1000000 / ts.TotalMilliseconds * 1000 / 1024));
			}
			catch (Exception ex)
			{
				Debug.WriteLine("Error.");
				SetStatus2("Dumping error.");
			}
			finally
			{
				if (fs != null)
					fs.Close();
			}
		}

		private void btnNORFlash_Click(object sender, EventArgs e)
		{
			/*
				n.checkchip()
				print
				data = open(sys.argv[3],"rb").read()
				addr = 0
				if len(sys.argv) == 5:
					addr = int(sys.argv[4],16)
				if (n.MF_ID == 0xEC) and (n.DEVICE_ID == 0x7e0601):
					print "Buffered programming mode not supported for Samsung K8Q2815UQB!"
					print "Programming in unlock bypass mode (writewordubm)..."
					if sys.argv[2] == "write":
						n.writerange(addr, data, False, True, True)
					else:
						n.writerange(addr, data, False, True, False)
				else:
					if sys.argv[2] == "write":
						n.writerange(addr, data, False, False, True)
					else:
						n.writerange(addr, data, False, False, False)
				print "Done. [%s]"%(datetime.timedelta(seconds=time.time() - tStart))
				tStart = time.time()
				n.verify(addr, data)
				print "Done. [%s]"%(datetime.timedelta(seconds=time.time() - tStart))
			*/
			const uint BLOCK = 0x20000;
			System.IO.FileStream fs = null;

			_flashNOR.NORInformationRefresh();
			SetStatus(_flashNOR.NORInformation.ManufacturerName + "/" + _flashNOR.NORInformation.DeviceName);

			if (_flashNOR.NORInformation.ChipType == Flash.NOR.NORInfo.ChipTypes.Unknown)
			{
				SetStatus2("Unknown flash type. Write aborted..");
				return;
			}

			try
			{
				Stopwatch sw = Stopwatch.StartNew();

				SetStatus2("Writing: 0 KB / 16384 KB");

				fs = new System.IO.FileStream(textBox1.Text, System.IO.FileMode.Open);

				bool success = false;
				byte[] data = new byte[BLOCK];

				for (uint i = 0; i < 0x1000000; i += BLOCK)
				//for (uint i = 0; i < 0x20000; i += BLOCK)
				{
					fs.Read(data, 0, (int)BLOCK);
					success = _flashNOR.WriteRange(i, data, (Flash.NOR.ProgrammingModes)cmbNORFlashMode.SelectedValue, false);
					if (!success) break;

					SetStatus2(string.Format("Writing: {0} KB / 16384 KB", (i + BLOCK) / 1024));
					System.Windows.Forms.Application.DoEvents();
				}

				sw.Stop();
				TimeSpan ts = sw.Elapsed;

				if (success)
					SetStatus2(string.Format("Writing done. [{0:00}:{1:00}:{2:00} ({3:F2} KB/s)]", ts.Hours, ts.Minutes, ts.Seconds, 0x1000000 / ts.TotalMilliseconds * 1000 / 1024));
				else
					SetStatus2("Writing failed!");
			}
			catch (Exception ex)
			{
				Debug.WriteLine("Error.");
				SetStatus2("Write error.");
			}
			finally
			{
				if (fs != null)
					fs.Close();
			}
		}

		private void btnNORid_Click(object sender, EventArgs e)
		{
			_flashNOR.NORInformationRefresh();
			SetStatus(_flashNOR.NORInformation.ManufacturerName + "/" + _flashNOR.NORInformation.DeviceName);
		}

		private void btnNOREraseChip_Click(object sender, EventArgs e)
		{
			_flashNOR.NORInformationRefresh();
			SetStatus(_flashNOR.NORInformation.ManufacturerName + "/" + _flashNOR.NORInformation.DeviceName);

			if (_flashNOR.NORInformation.ChipType == Flash.NOR.NORInfo.ChipTypes.Unknown)
			{
				SetStatus2("Unknown flash type. Erase aborted..");
				return;
			}

			Stopwatch sw = Stopwatch.StartNew();
			SetStatus2("Erasing NOR...");

			_flashNOR.EraseChip();

			sw.Stop();
			TimeSpan ts = sw.Elapsed;

			SetStatus2(string.Format("Erasechip done. [{0:00}:{1:00}:{2:00}]", ts.Hours, ts.Minutes, ts.Seconds));
		}

		private void btnSpeedTestRead_Click(object sender, EventArgs e)
		{
			const uint SIZE = 0x1000000;
			const uint BLOCK = 0x20000;

			System.IO.FileStream fs = null;
			SetStatus("SpeedTestRead");

			try
			{
				Stopwatch sw = Stopwatch.StartNew();

				SetStatus2("Dumping: 0 KB / 16384 KB");

				fs = new System.IO.FileStream(textBox1.Text, System.IO.FileMode.Create);

				_flashNOR.RxBytes = 0;
				for (uint i = 0; i < SIZE; i += BLOCK)
				{
					fs.Write(_flashNOR.SpeedTestRead(), 0, (int)BLOCK);
					SetStatus2(string.Format("Dumping: {0} KB / 16384 KB", i / 1024));
					System.Windows.Forms.Application.DoEvents();
				}

				sw.Stop();
				TimeSpan ts = sw.Elapsed;

				SetStatus2(string.Format("Dumping done. [{0:00}:{1:00}:{2:00} ({3:F2} KB/s)]", ts.Hours, ts.Minutes, ts.Seconds, SIZE / ts.TotalMilliseconds * 1000 / 1024));
			}
			catch (Exception ex)
			{
				Debug.WriteLine("Error.");
				SetStatus2("SpeedTest error.");
			}
			finally
			{
				if (fs != null)
					fs.Close();
			}
		}

		
		private void btnSpeedTestWrite_Click(object sender, EventArgs e)
		{
			const uint SIZE = 0x1000000;
			const uint BLOCK = 0x20000;

			byte[] data = new byte[BLOCK];
			SetStatus("SpeedTestWrite");

			try
			{
				Stopwatch sw = Stopwatch.StartNew();

				SetStatus2(string.Format("Writing: 0 KB / 16384 KB"));

				_flashNOR.TxBytes = 0;
				for (uint i = 0; i < SIZE; i += BLOCK)
				{
					SetStatus2(string.Format("Writing: {0} KB / 16384 KB", i / 1024));
					System.Windows.Forms.Application.DoEvents();
					_flashNOR.SpeedTestWrite(data);
				}

				sw.Stop();
				TimeSpan ts = sw.Elapsed;

				SetStatus2(string.Format("SpeedTest done. [{0:00}:{1:00}:{2:00}:{3:0000} ({4:F2} KB/s)]", ts.Hours, ts.Minutes, ts.Seconds, ts.Milliseconds, SIZE / ts.TotalMilliseconds * 1000 / 1024));
			}
			catch (Exception ex)
			{
				Debug.WriteLine("Error.");
				SetStatus2("SpeedTest error.");
			}
		}

		private void btnNANDid_Click(object sender, EventArgs e)
		{
			_flashNAND.NANDInformationRefresh((byte)(Flash.NAND.NANDSelect)cmbNANDid.SelectedValue);

			Debug.WriteLine("------------------------------");
			Debug.WriteLine("MakerCode: 0x{0:X2}", _flashNAND.NANDInformation.ManufacturerCode);
			Debug.WriteLine("DeviceCode: 0x{0:X2}", _flashNAND.NANDInformation.DeviceCode);
			Debug.WriteLine("PageSize: {0} bytes", _flashNAND.NANDInformation.PageSize);
			Debug.WriteLine("SpareSize: {0} bytes", _flashNAND.NANDInformation.SpareSize);
			Debug.WriteLine("BlockSize: {0} bytes", _flashNAND.NANDInformation.BlockSize);
			Debug.WriteLine("NumberOfBlocks: {0}", _flashNAND.NANDInformation.NumberOfBlocks);
			Debug.WriteLine("NumberOfPlanes: {0}", _flashNAND.NANDInformation.NumberOfPlanes);
			Debug.WriteLine("PlaneSize: {0} bytes", _flashNAND.NANDInformation.PlaneSize);
			Debug.WriteLine("FlashSize: {0} MB", _flashNAND.NANDInformation.NumberOfBlocks * _flashNAND.NANDInformation.BlockSize / 1048576);
			Debug.WriteLine("------------------------------");
		}

		private void btnNANDDump_Click(object sender, EventArgs e)
		{
			System.IO.FileStream fs = null;

			//_flashNAND.NANDInformationRefresh((byte)(Flash.NAND.NANDSelect)cmbNANDid.SelectedValue);

			//uint BLOCK_SIZE = 131072;
			uint BLOCK_SIZE = 135168;

			try
			{
				Stopwatch sw = Stopwatch.StartNew();

				fs = new System.IO.FileStream(textBox1.Text, System.IO.FileMode.Create);

				_flashNAND.RxBytes = 0;
				for (uint i = 0; i < 1024; i++)
				//for (uint i = 0; i < 4; i++)
				//for (uint i = 32; i < 33; i++)
				{
					fs.Write(_flashNAND.ReadBlock(i, BLOCK_SIZE), 0, (int)BLOCK_SIZE);
					SetStatus2(string.Format("Dumping: {0} KB / {1} KB", (i + 1) * (BLOCK_SIZE / 1024), BLOCK_SIZE));
					System.Windows.Forms.Application.DoEvents();
				}

				sw.Stop();
				TimeSpan ts = sw.Elapsed;
				//TimeSpan ts2 = sw.Elapsed;
				//for (int i = 0; i < 256; i++)
				//   ts = ts.Add(ts2);

				SetStatus2(string.Format("Dumping done. [{0:00}:{1:00}:{2:00} ({3:F2} KB/s)]", ts.Hours, ts.Minutes, ts.Seconds, (BLOCK_SIZE * 1024) / ts.TotalMilliseconds * 1000 / 1024));
			}
			catch (Exception ex)
			{
				Debug.WriteLine("Error.");
				SetStatus2("Dumping error.");
			}
			finally
			{
				if (fs != null)
					fs.Close();
			}

		}

		private void btnCalcECC_Click(object sender, EventArgs e)
		{
			System.IO.FileStream fs0 = null;
			System.IO.FileStream fs1 = null;
			System.IO.TextWriter outfile = null;

			try
			{
				fs0 = new System.IO.FileStream(@"D:\PS3\dump_orig\nand1.bin", System.IO.FileMode.Open);
				fs1 = new System.IO.FileStream(@"D:\PS3\dump_orig\nand1.bin.new.bin", System.IO.FileMode.Open);
				outfile = new System.IO.StreamWriter(@"D:\PS3\dump_orig\nand1.txt");

				byte[] data0 = new byte[2112];
				byte[] data1 = new byte[2112];
				byte[] ecc0 = new byte[16];
				byte[] ecc1 = new byte[16];

				string s0 = string.Empty;
				string s1 = string.Empty;

				while (fs0.Position < fs0.Length)
				{
					fs0.Read(data0, 0, data0.Length);
					fs1.Read(data1, 0, data1.Length);
					
					int i = 0;
					for (int k = 2112 - 64 + 8 - 1; k < 2112 - 1; k += 14)
					{
						Array.Copy(data0, k + 11, ecc0, i, 4);
						Array.Copy(data1, k + 11, ecc1, i, 4);

						i += 4;
					}
					
					if (Flash.FlashBase.ByteArraysEqual(ecc0, ecc1))
					{
						s0 = string.Format("0x{0:x} (orig): ", fs0.Position - 0x40);
						s1 = string.Format("0x{0:x} (ptch): ", fs0.Position - 0x40);
						for (i = 0; i < ecc0.Length; i += 4)
						{
							s0 += string.Format("{0:x2} {1:x2} {2:x2} {3:x2} : ", ecc0[i], ecc0[i + 1], ecc0[i + 2], ecc0[i + 3]);
							s1 += string.Format("{0:x2} {1:x2} {2:x2} {3:x2} : ", ecc1[i], ecc1[i + 1], ecc1[i + 2], ecc1[i + 3]);
						}

						s0 = s0.Substring(0, s0.Length - 3);
						s1 = s1.Substring(0, s1.Length - 3);
						outfile.WriteLine(s0);
						outfile.WriteLine(s1);
						//Debug.WriteLine(s0);
						//Debug.WriteLine(s1);
					}
				}

		
				
				//nand_calculate_ecc_kw(data, ecc);

				//Debug.WriteLine("-----------");
				//for (int i = 0; i < 10; i++)
				//{
				//   Debug.WriteLine("0x{0:X2}", ecc[i]);
				//}
				//Debug.WriteLine("-----------");
			}
			catch (Exception ex)
			{
			}
			finally
			{
				if (fs0 != null)
					fs0.Close();
				if (fs1 != null)
					fs1.Close();
				if (outfile != null)
					outfile.Close();
			}
		}

		private void cmdGetOOB_Click(object sender, EventArgs e)
		{
			System.IO.FileStream fs0 = null;
			System.IO.TextReader blockfile = null;
			System.IO.TextWriter outfile = null;

			try
			{
				fs0 = new System.IO.FileStream(@"D:\PS3\dump_orig\nand1.bin", System.IO.FileMode.Open, System.IO.FileAccess.Read, System.IO.FileShare.ReadWrite);
				blockfile = new System.IO.StreamReader(@"D:\PS3\dump_orig\nand1_blocks.txt");
				outfile = new System.IO.StreamWriter(@"D:\PS3\dump_orig\nand1_oob.txt");

				byte[] oob = new byte[6];

				string s0 = null;
				string s1 = string.Empty;

				s0 = blockfile.ReadLine();
				do
				{
					fs0.Seek(Convert.ToInt64(s0) * 0x21000 + 0x802, System.IO.SeekOrigin.Begin);
					fs0.Read(oob, 0, oob.Length);
					outfile.WriteLine("{0:X2} {1:X2} {2:X2} {3:X2} {4:X2} {5:X2}", oob[0], oob[1], oob[2], oob[3], oob[4], oob[5]);
					s0 = blockfile.ReadLine();
				} while (s0 != null);

			}
			catch (Exception ex)
			{
				Debug.Assert(false, ex.Message);
			}
			finally
			{
				if (fs0 != null)
					fs0.Close();
				if (blockfile != null)
					blockfile.Close();
				if (outfile != null)
					outfile.Close();
			}

		}

		private void cmdEmptyBlocks_Click(object sender, EventArgs e)
		{
			System.IO.FileStream fs0 = null;
			System.IO.TextReader blockfile = null;
			System.IO.TextWriter outfile = null;

			try
			{
				fs0 = new System.IO.FileStream(@"D:\PS3\Dev\Firmware\Dumps\nand\guerrierodipace\2.bin", System.IO.FileMode.Open, System.IO.FileAccess.Read, System.IO.FileShare.ReadWrite);
				//blockfile = new System.IO.StreamReader(@"D:\PS3\dump_orig\nand1_blocks.txt");
				//outfile = new System.IO.StreamWriter(@"D:\PS3\dump_orig\nand1_oob.txt");

				byte[] block = new byte[0x21000];
				byte[] page = new byte[0x800];
				byte[] spare = new byte[0x40 - 8];
				byte[] ecc = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
								 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
								 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
								 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00 };

				while (fs0.Position < fs0.Length)
				{
					fs0.Read(block, 0, block.Length);

					for (int i = 0; i < block.Length; i += (page.Length + spare.Length + 8))
					{
						Buffer.BlockCopy(block, i, page, 0, page.Length);
						Buffer.BlockCopy(block, i + page.Length + 8, spare, 0, spare.Length);

						if (Flash.FlashBase.ByteArrayHasSameValues(page, 0xFF))
						{
							if (!Flash.FlashBase.ByteArraysEqual(spare, ecc))
							{
								Debug.WriteLine(string.Format("Block at 0x{0:X8} is all 0xFF!", fs0.Position - block.Length));
								break;
							}
						}
					}
				}
			}
			catch (Exception ex)
			{
				Debug.Assert(false, ex.Message);
			}
			finally
			{
				if (fs0 != null)
					fs0.Close();
				if (blockfile != null)
					blockfile.Close();
				if (outfile != null)
					outfile.Close();
			}

		}










		/*****************************************************************************
		 * Arithmetic in GF(2^10) ("F") modulo x^10 + x^3 + 1.
		 *
		 * For multiplication, a discrete log/exponent table is used, with
		 * primitive element x (F is a primitive field, so x is primitive).
		 */
		const int MODPOLY = 0x409;		/* x^10 + x^3 + 1 in binary */

		/*
		 * Maps an integer a [0..1022] to a polynomial b = gf_exp[a] in
		 * GF(2^10) mod x^10 + x^3 + 1 such that b = x ^ a.  There's two
		 * identical copies of this array back-to-back so that we can save
		 * the mod 1023 operation when doing a GF multiplication.
		 */
		UInt16[] gf_exp = new UInt16[1023 + 1023];

		/*
		 * Maps a polynomial b in GF(2^10) mod x^10 + x^3 + 1 to an index
		 * a = gf_log[b] in [0..1022] such that b = x ^ a.
		 */
		UInt16[] gf_log = new UInt16[1024];

		void gf_build_log_exp_table()
		{
			int i;
			int p_i;

			/*
			 * p_i = x ^ i
			 *
			 * Initialise to 1 for i = 0.
			 */
			p_i = 1;

			for (i = 0; i < 1023; i++) {
				gf_exp[i] = (UInt16)p_i;
				gf_exp[i + 1023] = (UInt16)p_i;
				gf_log[p_i] = (UInt16)i;

				/*
				 * p_i = p_i * x
				 */
				p_i <<= 1;
				if ((p_i & (1 << 10)) != 0)
					p_i ^= MODPOLY;
			}
		}


		/*****************************************************************************
		 * Reed-Solomon code
		 *
		 * This implements a (1023,1015) Reed-Solomon ECC code over GF(2^10)
		 * mod x^10 + x^3 + 1, shortened to (520,512).  The ECC data consists
		 * of 8 10-bit symbols, or 10 8-bit bytes.
		 *
		 * Given 512 bytes of data, computes 10 bytes of ECC.
		 *
		 * This is done by converting the 512 bytes to 512 10-bit symbols
		 * (elements of F), interpreting those symbols as a polynomial in F[X]
		 * by taking symbol 0 as the coefficient of X^8 and symbol 511 as the
		 * coefficient of X^519, and calculating the residue of that polynomial
		 * divided by the generator polynomial, which gives us the 8 ECC symbols
		 * as the remainder.  Finally, we convert the 8 10-bit ECC symbols to 10
		 * 8-bit bytes.
		 *
		 * The generator polynomial is hardcoded, as that is faster, but it
		 * can be computed by taking the primitive element a = x (in F), and
		 * constructing a polynomial in F[X] with roots a, a^2, a^3, ..., a^8
		 * by multiplying the minimal polynomials for those roots (which are
		 * just 'x - a^i' for each i).
		 *
		 * Note: due to unfortunate circumstances, the bootrom in the Kirkwood SOC
		 * expects the ECC to be computed backward, i.e. from the last byte down
		 * to the first one.
		 */
		int tables_initialized = 0;
		int nand_calculate_ecc_kw(byte[] data, byte[] ecc)
		{
			uint r7, r6, r5, r4, r3, r2, r1, r0;
			int i;


			if (tables_initialized == 0) {
				gf_build_log_exp_table();
				tables_initialized = 1;
			}

			/*
			 * Load bytes 504..511 of the data into r.
			 */
			r0 = data[504];
			r1 = data[505];
			r2 = data[506];
			r3 = data[507];
			r4 = data[508];
			r5 = data[509];
			r6 = data[510];
			r7 = data[511];

			/*
			 * Shift bytes 503..0 (in that order) into r0, followed
			 * by eight zero bytes, while reducing the polynomial by the
			 * generator polynomial in every step.
			 */
			for (i = 503; i >= -8; i--) {
				uint d;

				d = 0;
				if (i >= 0)
					d = data[i];

				if (r7 > 0) {
					UInt16 gflog = (UInt16)(gf_log[r7]);

					r7 = r6 ^ gf_exp[0x21c + gflog];
					r6 = r5 ^ gf_exp[0x181 + gflog];
					r5 = r4 ^ gf_exp[0x18e + gflog];
					r4 = r3 ^ gf_exp[0x25f + gflog];
					r3 = r2 ^ gf_exp[0x197 + gflog];
					r2 = r1 ^ gf_exp[0x193 + gflog];
					r1 = r0 ^ gf_exp[0x237 + gflog];
					r0 = d ^ gf_exp[0x024 + gflog];
				} else {
					r7 = r6;
					r6 = r5;
					r5 = r4;
					r4 = r3;
					r3 = r2;
					r2 = r1;
					r1 = r0;
					r0 = d;
				}
			}

			ecc[0] = (byte)r0;
			ecc[1] = (byte)((r0 >> 8) | (r1 << 2));
			ecc[2] = (byte)((r1 >> 6) | (r2 << 4));
			ecc[3] = (byte)((r2 >> 4) | (r3 << 6));
			ecc[4] = (byte)(r3 >> 2);
			ecc[5] = (byte)r4;
			ecc[6] = (byte)((r4 >> 8) | (r5 << 2));
			ecc[7] = (byte)((r5 >> 6) | (r6 << 4));
			ecc[8] = (byte)((r6 >> 4) | (r7 << 6));
			ecc[9] = (byte)(r7 >> 2);

			return 0;
		}

	}
}

/*
	n = NORFlasher(sys.argv[1])
	print "Pinging..."
	n.ping()

	print "Set SB to tristate"
	print
	n.setports(1)
	n.reset = 0
	#n.trist = 1
	n.printstate()
	print
	print "Resetting NOR..."
	n.reset = 1
	n.udelay(40)
	n.reset = 0
	n.udelay(40)
	n.ping()
	print "Ready."

	tStart = time.time()
	if len(sys.argv) == 4 and sys.argv[2] == "dump":
		BLOCK = 0x10000
		print
		print "Dumping NOR..."
		fo = open(sys.argv[3],"wb")
		for offset in range(0, 0x800000, BLOCK):
			fo.write(n.readsector(offset, 0x20000))
			print "\r%d KB / 16384 KB"%((offset+BLOCK)/512),
			sys.stdout.flush()
		print
		print "Done. [%s]"%(datetime.timedelta(seconds=time.time() - tStart))
	elif len(sys.argv) == 4 and sys.argv[2] == "erase":
		n.checkchip()
		print
		addr = int(sys.argv[3], 16)
		if addr & 0x1:
			print "Address must be even!"
			n.closedevice()
			sys.exit(1)
		print "Erasing sector/block at address %06x..."%addr,
		sys.stdout.flush()
		n.erasesector(addr/2)
		print "Done. [%s]"%(datetime.timedelta(seconds=time.time() - tStart))
	elif len(sys.argv) == 3 and sys.argv[2] == "erasechip":
		n.checkchip()
		print
		print "Erasing chip, might take a while... (1-3 minutes)"
		n.erasechip()
		print
		print "Done. [%s]"%(datetime.timedelta(seconds=time.time() - tStart))
	elif len(sys.argv) in (4,5) and (sys.argv[2] == "write" or sys.argv[2] == "vwrite"):
		n.checkchip()
		print
		data = open(sys.argv[3],"rb").read()
		addr = 0
		if len(sys.argv) == 5:
			addr = int(sys.argv[4],16)
		if (n.MF_ID == 0xEC) and (n.DEVICE_ID == 0x7e0601):
			print "Buffered programming mode not supported for Samsung K8Q2815UQB!"
			print "Programming in unlock bypass mode (writewordubm)..."
			if sys.argv[2] == "write":
				n.writerange(addr, data, False, True, True)
			else:
				n.writerange(addr, data, False, True, False)
		else:
			if sys.argv[2] == "write":
				n.writerange(addr, data, False, False, True)
			else:
				n.writerange(addr, data, False, False, False)
		print "Done. [%s]"%(datetime.timedelta(seconds=time.time() - tStart))
		tStart = time.time()
		n.verify(addr, data)
		print "Done. [%s]"%(datetime.timedelta(seconds=time.time() - tStart))
	elif len(sys.argv) in (4,5) and (sys.argv[2] == "writeword" or sys.argv[2] == "vwriteword"):
		n.checkchip()
		print
		data = open(sys.argv[3],"rb").read()
		addr = 0
		if len(sys.argv) == 5:
			addr = int(sys.argv[4],16)
		if sys.argv[2] == "writeword":
			n.writerange(addr, data, True, False, True)
		else:
			n.writerange(addr, data, True, False, False)
		print "Done. [%s]"%(datetime.timedelta(seconds=time.time() - tStart))
		tStart = time.time()
		n.verify(addr, data)
		print "Done. [%s]"%(datetime.timedelta(seconds=time.time() - tStart))
	elif len(sys.argv) in (4,5) and (sys.argv[2] == "writewordubm" or sys.argv[2] == "vwritewordubm"):
		n.checkchip()
		print
		data = open(sys.argv[3],"rb").read()
		addr = 0
		if len(sys.argv) == 5:
			addr = int(sys.argv[4],16)
		if sys.argv[2] == "writewordubm":
			n.writerange(addr, data, False, True, True)
		else:
			n.writerange(addr, data, False, True, False)
		print "Done. [%s]"%(datetime.timedelta(seconds=time.time() - tStart))
		tStart = time.time()
		n.verify(addr, data)
		print "Done. [%s]"%(datetime.timedelta(seconds=time.time() - tStart))
	elif len(sys.argv) in (4,5) and sys.argv[2] == "verify":
		data = open(sys.argv[3],"rb").read()
		addr = 0
		if len(sys.argv) == 5:
			addr = int(sys.argv[4],16)
		n.verify(addr, data)
		print "Done. [%s]"%(datetime.timedelta(seconds=time.time() - tStart))
	elif len(sys.argv) == 3 and sys.argv[2] == "release":
		print
		#n.trist = 0
		n.setports(0)
		print "NOR Released"
	elif len(sys.argv) == 3 and sys.argv[2] == "bootloader":
		print
		print "Entering Teensy's bootloader mode... Goodbye!"
		n.bootloader()
		n.closedevice()
		sys.exit(0)
	#elif len(sys.argv) == 4 and sys.argv[2] == "speedtest_read":
	#	BLOCK = 0x10000
	#	print
	#	print "Measuring read performance..."
	#	fo = open(sys.argv[3],"wb")
	#	for offset in range(0, 0x800000, BLOCK):
	#		fo.write(n.speedtest_read())
	#		print "\r%d KB / 16384 KB"%((offset+BLOCK)/512),
	#		sys.stdout.flush()
	#	print
	#	print "Done. [%s]"%(datetime.timedelta(seconds=time.time() - tStart))
	#elif len(sys.argv) in (4,5) and sys.argv[2] == "speedtest_write":
	#	print
	#	print "Measuring write performance..."
	#	data = open(sys.argv[3],"rb").read()
	#	addr = 0
	#	if len(sys.argv) == 5:
	#		addr = int(sys.argv[4],16)
	#	n.speedtest_write(addr, data)
	#	print "Done. [%s]"%(datetime.timedelta(seconds=time.time() - tStart))

	n.ping()
	n.closedevice()
*/