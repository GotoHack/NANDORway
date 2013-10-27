namespace NANDORway
{
	partial class Main
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose(bool disposing)
		{
			if (disposing && (components != null))
			{
				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.btnPing = new System.Windows.Forms.Button();
			this.statusStrip1 = new System.Windows.Forms.StatusStrip();
			this.toolStripStatusLabel1 = new System.Windows.Forms.ToolStripStatusLabel();
			this.toolStripStatusLabel2 = new System.Windows.Forms.ToolStripStatusLabel();
			this.btnNORDump = new System.Windows.Forms.Button();
			this.btnNORFlash = new System.Windows.Forms.Button();
			this.btnNORid = new System.Windows.Forms.Button();
			this.btnNOREraseChip = new System.Windows.Forms.Button();
			this.btnNORTristate = new System.Windows.Forms.Button();
			this.textBox1 = new System.Windows.Forms.TextBox();
			this.btnSelectFile = new System.Windows.Forms.Button();
			this.btnSpeedTestRead = new System.Windows.Forms.Button();
			this.btnSpeedTestWrite = new System.Windows.Forms.Button();
			this.tabControl1 = new System.Windows.Forms.TabControl();
			this.tabPage1 = new System.Windows.Forms.TabPage();
			this.cmbNORFlashMode = new System.Windows.Forms.ComboBox();
			this.tabPage2 = new System.Windows.Forms.TabPage();
			this.cmdEmptyBlocks = new System.Windows.Forms.Button();
			this.cmdGetOOB = new System.Windows.Forms.Button();
			this.btnCalcECC = new System.Windows.Forms.Button();
			this.cmbNANDid = new System.Windows.Forms.ComboBox();
			this.btnNANDFlash = new System.Windows.Forms.Button();
			this.btnNANDDump = new System.Windows.Forms.Button();
			this.btnNANDid = new System.Windows.Forms.Button();
			this.btnNANDTristate = new System.Windows.Forms.Button();
			this.btnNANDEraseChip = new System.Windows.Forms.Button();
			this.listView1 = new System.Windows.Forms.ListView();
			this.statusStrip1.SuspendLayout();
			this.tabControl1.SuspendLayout();
			this.tabPage1.SuspendLayout();
			this.tabPage2.SuspendLayout();
			this.SuspendLayout();
			// 
			// btnPing
			// 
			this.btnPing.Location = new System.Drawing.Point(16, 299);
			this.btnPing.Name = "btnPing";
			this.btnPing.Size = new System.Drawing.Size(104, 28);
			this.btnPing.TabIndex = 3;
			this.btnPing.Text = "Ping";
			this.btnPing.UseVisualStyleBackColor = true;
			this.btnPing.Click += new System.EventHandler(this.btnPing_Click);
			// 
			// statusStrip1
			// 
			this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripStatusLabel1,
            this.toolStripStatusLabel2});
			this.statusStrip1.Location = new System.Drawing.Point(0, 440);
			this.statusStrip1.Name = "statusStrip1";
			this.statusStrip1.Size = new System.Drawing.Size(474, 22);
			this.statusStrip1.TabIndex = 4;
			this.statusStrip1.Text = "statusStrip1";
			// 
			// toolStripStatusLabel1
			// 
			this.toolStripStatusLabel1.Name = "toolStripStatusLabel1";
			this.toolStripStatusLabel1.Size = new System.Drawing.Size(118, 17);
			this.toolStripStatusLabel1.Text = "toolStripStatusLabel1";
			// 
			// toolStripStatusLabel2
			// 
			this.toolStripStatusLabel2.Name = "toolStripStatusLabel2";
			this.toolStripStatusLabel2.Size = new System.Drawing.Size(118, 17);
			this.toolStripStatusLabel2.Text = "toolStripStatusLabel2";
			// 
			// btnNORDump
			// 
			this.btnNORDump.Location = new System.Drawing.Point(6, 74);
			this.btnNORDump.Name = "btnNORDump";
			this.btnNORDump.Size = new System.Drawing.Size(104, 28);
			this.btnNORDump.TabIndex = 5;
			this.btnNORDump.Text = "Dump";
			this.btnNORDump.UseVisualStyleBackColor = true;
			this.btnNORDump.Click += new System.EventHandler(this.btnNORDump_Click);
			// 
			// btnNORFlash
			// 
			this.btnNORFlash.Location = new System.Drawing.Point(6, 187);
			this.btnNORFlash.Name = "btnNORFlash";
			this.btnNORFlash.Size = new System.Drawing.Size(104, 28);
			this.btnNORFlash.TabIndex = 6;
			this.btnNORFlash.Text = "Flash";
			this.btnNORFlash.UseVisualStyleBackColor = true;
			this.btnNORFlash.Click += new System.EventHandler(this.btnNORFlash_Click);
			// 
			// btnNORid
			// 
			this.btnNORid.Location = new System.Drawing.Point(6, 6);
			this.btnNORid.Name = "btnNORid";
			this.btnNORid.Size = new System.Drawing.Size(104, 28);
			this.btnNORid.TabIndex = 7;
			this.btnNORid.Text = "Read ID";
			this.btnNORid.UseVisualStyleBackColor = true;
			this.btnNORid.Click += new System.EventHandler(this.btnNORid_Click);
			// 
			// btnNOREraseChip
			// 
			this.btnNOREraseChip.Location = new System.Drawing.Point(6, 221);
			this.btnNOREraseChip.Name = "btnNOREraseChip";
			this.btnNOREraseChip.Size = new System.Drawing.Size(104, 28);
			this.btnNOREraseChip.TabIndex = 9;
			this.btnNOREraseChip.Text = "Erase Chip";
			this.btnNOREraseChip.UseVisualStyleBackColor = true;
			this.btnNOREraseChip.Click += new System.EventHandler(this.btnNOREraseChip_Click);
			// 
			// btnNORTristate
			// 
			this.btnNORTristate.Location = new System.Drawing.Point(6, 40);
			this.btnNORTristate.Name = "btnNORTristate";
			this.btnNORTristate.Size = new System.Drawing.Size(104, 28);
			this.btnNORTristate.TabIndex = 10;
			this.btnNORTristate.Text = "Lock / Unlock";
			this.btnNORTristate.UseVisualStyleBackColor = true;
			this.btnNORTristate.Click += new System.EventHandler(this.btnNORTristate_Click);
			// 
			// textBox1
			// 
			this.textBox1.Location = new System.Drawing.Point(146, 82);
			this.textBox1.Name = "textBox1";
			this.textBox1.Size = new System.Drawing.Size(254, 20);
			this.textBox1.TabIndex = 11;
			// 
			// btnSelectFile
			// 
			this.btnSelectFile.Location = new System.Drawing.Point(406, 82);
			this.btnSelectFile.Name = "btnSelectFile";
			this.btnSelectFile.Size = new System.Drawing.Size(25, 20);
			this.btnSelectFile.TabIndex = 12;
			this.btnSelectFile.Text = "...";
			this.btnSelectFile.UseVisualStyleBackColor = true;
			this.btnSelectFile.Click += new System.EventHandler(this.btnSelectFile_Click);
			// 
			// btnSpeedTestRead
			// 
			this.btnSpeedTestRead.Location = new System.Drawing.Point(126, 299);
			this.btnSpeedTestRead.Name = "btnSpeedTestRead";
			this.btnSpeedTestRead.Size = new System.Drawing.Size(104, 28);
			this.btnSpeedTestRead.TabIndex = 13;
			this.btnSpeedTestRead.Text = "ReadSpeedTest";
			this.btnSpeedTestRead.UseVisualStyleBackColor = true;
			this.btnSpeedTestRead.Click += new System.EventHandler(this.btnSpeedTestRead_Click);
			// 
			// btnSpeedTestWrite
			// 
			this.btnSpeedTestWrite.Location = new System.Drawing.Point(236, 299);
			this.btnSpeedTestWrite.Name = "btnSpeedTestWrite";
			this.btnSpeedTestWrite.Size = new System.Drawing.Size(104, 28);
			this.btnSpeedTestWrite.TabIndex = 14;
			this.btnSpeedTestWrite.Text = "WriteSpeedTest";
			this.btnSpeedTestWrite.UseVisualStyleBackColor = true;
			this.btnSpeedTestWrite.Click += new System.EventHandler(this.btnSpeedTestWrite_Click);
			// 
			// tabControl1
			// 
			this.tabControl1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.tabControl1.Controls.Add(this.tabPage1);
			this.tabControl1.Controls.Add(this.tabPage2);
			this.tabControl1.Location = new System.Drawing.Point(12, 12);
			this.tabControl1.Name = "tabControl1";
			this.tabControl1.SelectedIndex = 0;
			this.tabControl1.Size = new System.Drawing.Size(450, 281);
			this.tabControl1.TabIndex = 15;
			// 
			// tabPage1
			// 
			this.tabPage1.Controls.Add(this.cmbNORFlashMode);
			this.tabPage1.Controls.Add(this.btnNORFlash);
			this.tabPage1.Controls.Add(this.btnNORDump);
			this.tabPage1.Controls.Add(this.btnNORid);
			this.tabPage1.Controls.Add(this.btnNORTristate);
			this.tabPage1.Controls.Add(this.btnSelectFile);
			this.tabPage1.Controls.Add(this.textBox1);
			this.tabPage1.Controls.Add(this.btnNOREraseChip);
			this.tabPage1.Location = new System.Drawing.Point(4, 22);
			this.tabPage1.Name = "tabPage1";
			this.tabPage1.Padding = new System.Windows.Forms.Padding(3);
			this.tabPage1.Size = new System.Drawing.Size(442, 255);
			this.tabPage1.TabIndex = 0;
			this.tabPage1.Text = "NOR";
			this.tabPage1.UseVisualStyleBackColor = true;
			// 
			// cmbNORFlashMode
			// 
			this.cmbNORFlashMode.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.cmbNORFlashMode.FormattingEnabled = true;
			this.cmbNORFlashMode.Location = new System.Drawing.Point(146, 187);
			this.cmbNORFlashMode.Name = "cmbNORFlashMode";
			this.cmbNORFlashMode.Size = new System.Drawing.Size(285, 21);
			this.cmbNORFlashMode.TabIndex = 13;
			// 
			// tabPage2
			// 
			this.tabPage2.Controls.Add(this.cmdEmptyBlocks);
			this.tabPage2.Controls.Add(this.cmdGetOOB);
			this.tabPage2.Controls.Add(this.btnCalcECC);
			this.tabPage2.Controls.Add(this.cmbNANDid);
			this.tabPage2.Controls.Add(this.btnNANDFlash);
			this.tabPage2.Controls.Add(this.btnNANDDump);
			this.tabPage2.Controls.Add(this.btnNANDid);
			this.tabPage2.Controls.Add(this.btnNANDTristate);
			this.tabPage2.Controls.Add(this.btnNANDEraseChip);
			this.tabPage2.Location = new System.Drawing.Point(4, 22);
			this.tabPage2.Name = "tabPage2";
			this.tabPage2.Padding = new System.Windows.Forms.Padding(3);
			this.tabPage2.Size = new System.Drawing.Size(442, 255);
			this.tabPage2.TabIndex = 1;
			this.tabPage2.Text = "NAND";
			this.tabPage2.UseVisualStyleBackColor = true;
			// 
			// cmdEmptyBlocks
			// 
			this.cmdEmptyBlocks.Location = new System.Drawing.Point(169, 181);
			this.cmdEmptyBlocks.Name = "cmdEmptyBlocks";
			this.cmdEmptyBlocks.Size = new System.Drawing.Size(104, 28);
			this.cmdEmptyBlocks.TabIndex = 19;
			this.cmdEmptyBlocks.Text = "Empty Blocks";
			this.cmdEmptyBlocks.UseVisualStyleBackColor = true;
			this.cmdEmptyBlocks.Click += new System.EventHandler(this.cmdEmptyBlocks_Click);
			// 
			// cmdGetOOB
			// 
			this.cmdGetOOB.Location = new System.Drawing.Point(169, 147);
			this.cmdGetOOB.Name = "cmdGetOOB";
			this.cmdGetOOB.Size = new System.Drawing.Size(104, 28);
			this.cmdGetOOB.TabIndex = 18;
			this.cmdGetOOB.Text = "Get OOB";
			this.cmdGetOOB.UseVisualStyleBackColor = true;
			this.cmdGetOOB.Click += new System.EventHandler(this.cmdGetOOB_Click);
			// 
			// btnCalcECC
			// 
			this.btnCalcECC.Location = new System.Drawing.Point(169, 113);
			this.btnCalcECC.Name = "btnCalcECC";
			this.btnCalcECC.Size = new System.Drawing.Size(104, 28);
			this.btnCalcECC.TabIndex = 17;
			this.btnCalcECC.Text = "Calc ECC";
			this.btnCalcECC.UseVisualStyleBackColor = true;
			this.btnCalcECC.Click += new System.EventHandler(this.btnCalcECC_Click);
			// 
			// cmbNANDid
			// 
			this.cmbNANDid.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.cmbNANDid.FormattingEnabled = true;
			this.cmbNANDid.Location = new System.Drawing.Point(151, 11);
			this.cmbNANDid.Name = "cmbNANDid";
			this.cmbNANDid.Size = new System.Drawing.Size(285, 21);
			this.cmbNANDid.TabIndex = 16;
			// 
			// btnNANDFlash
			// 
			this.btnNANDFlash.Location = new System.Drawing.Point(6, 187);
			this.btnNANDFlash.Name = "btnNANDFlash";
			this.btnNANDFlash.Size = new System.Drawing.Size(104, 28);
			this.btnNANDFlash.TabIndex = 12;
			this.btnNANDFlash.Text = "Flash";
			this.btnNANDFlash.UseVisualStyleBackColor = true;
			// 
			// btnNANDDump
			// 
			this.btnNANDDump.Location = new System.Drawing.Point(6, 74);
			this.btnNANDDump.Name = "btnNANDDump";
			this.btnNANDDump.Size = new System.Drawing.Size(104, 28);
			this.btnNANDDump.TabIndex = 11;
			this.btnNANDDump.Text = "Dump";
			this.btnNANDDump.UseVisualStyleBackColor = true;
			this.btnNANDDump.Click += new System.EventHandler(this.btnNANDDump_Click);
			// 
			// btnNANDid
			// 
			this.btnNANDid.Location = new System.Drawing.Point(6, 6);
			this.btnNANDid.Name = "btnNANDid";
			this.btnNANDid.Size = new System.Drawing.Size(104, 28);
			this.btnNANDid.TabIndex = 13;
			this.btnNANDid.Text = "Read ID";
			this.btnNANDid.UseVisualStyleBackColor = true;
			this.btnNANDid.Click += new System.EventHandler(this.btnNANDid_Click);
			// 
			// btnNANDTristate
			// 
			this.btnNANDTristate.Location = new System.Drawing.Point(6, 40);
			this.btnNANDTristate.Name = "btnNANDTristate";
			this.btnNANDTristate.Size = new System.Drawing.Size(104, 28);
			this.btnNANDTristate.TabIndex = 15;
			this.btnNANDTristate.Text = "Lock / Unlock";
			this.btnNANDTristate.UseVisualStyleBackColor = true;
			// 
			// btnNANDEraseChip
			// 
			this.btnNANDEraseChip.Location = new System.Drawing.Point(6, 221);
			this.btnNANDEraseChip.Name = "btnNANDEraseChip";
			this.btnNANDEraseChip.Size = new System.Drawing.Size(104, 28);
			this.btnNANDEraseChip.TabIndex = 14;
			this.btnNANDEraseChip.Text = "Erase Chip";
			this.btnNANDEraseChip.UseVisualStyleBackColor = true;
			// 
			// listView1
			// 
			this.listView1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.listView1.Location = new System.Drawing.Point(12, 333);
			this.listView1.Name = "listView1";
			this.listView1.Size = new System.Drawing.Size(450, 104);
			this.listView1.TabIndex = 16;
			this.listView1.UseCompatibleStateImageBehavior = false;
			// 
			// Main
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(474, 462);
			this.Controls.Add(this.listView1);
			this.Controls.Add(this.tabControl1);
			this.Controls.Add(this.btnSpeedTestWrite);
			this.Controls.Add(this.btnSpeedTestRead);
			this.Controls.Add(this.statusStrip1);
			this.Controls.Add(this.btnPing);
			this.MinimumSize = new System.Drawing.Size(490, 500);
			this.Name = "Main";
			this.Text = "NANDORway";
			this.statusStrip1.ResumeLayout(false);
			this.statusStrip1.PerformLayout();
			this.tabControl1.ResumeLayout(false);
			this.tabPage1.ResumeLayout(false);
			this.tabPage1.PerformLayout();
			this.tabPage2.ResumeLayout(false);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.Button btnPing;
		private System.Windows.Forms.StatusStrip statusStrip1;
		private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel1;
		private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel2;
		private System.Windows.Forms.Button btnNORDump;
		private System.Windows.Forms.Button btnNORFlash;
		private System.Windows.Forms.Button btnNORid;
		private System.Windows.Forms.Button btnNOREraseChip;
		private System.Windows.Forms.Button btnNORTristate;
		private System.Windows.Forms.TextBox textBox1;
		private System.Windows.Forms.Button btnSelectFile;
		private System.Windows.Forms.Button btnSpeedTestRead;
		private System.Windows.Forms.Button btnSpeedTestWrite;
		private System.Windows.Forms.TabControl tabControl1;
		private System.Windows.Forms.TabPage tabPage1;
		private System.Windows.Forms.TabPage tabPage2;
		private System.Windows.Forms.ListView listView1;
		private System.Windows.Forms.ComboBox cmbNORFlashMode;
		private System.Windows.Forms.Button btnNANDFlash;
		private System.Windows.Forms.Button btnNANDDump;
		private System.Windows.Forms.Button btnNANDid;
		private System.Windows.Forms.Button btnNANDTristate;
		private System.Windows.Forms.Button btnNANDEraseChip;
		private System.Windows.Forms.ComboBox cmbNANDid;
		private System.Windows.Forms.Button btnCalcECC;
		private System.Windows.Forms.Button cmdGetOOB;
		private System.Windows.Forms.Button cmdEmptyBlocks;
	}
}

