/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 1997-2004 Apple Computer, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Net;
using System.Net.Sockets;
using System.Data;
using System.Text;
using Bonjour;

namespace SimpleChat.NET
{
	/// <summary>
	/// Summary description for Form1.
	/// </summary>
	/// 

	public class SimpleChat : System.Windows.Forms.Form
	{
		private System.Windows.Forms.ComboBox   comboBox1;
		private System.Windows.Forms.TextBox    textBox2;
		private System.Windows.Forms.Button     button1;
		private System.Windows.Forms.Label      label1;

        private Bonjour.DNSSDEventManager       m_eventManager = null;
        private Bonjour.DNSSDService            m_service = null;
        private Bonjour.DNSSDService            m_registrar = null;
        private Bonjour.DNSSDService            m_browser = null;
        private Bonjour.DNSSDService            m_resolver = null;
		private String					        m_name;
        private Socket                          m_socket = null;
        private const int                       BUFFER_SIZE = 1024;
        public byte[]                           m_buffer = new byte[BUFFER_SIZE];
        public bool                             m_complete = false;
        public StringBuilder                    m_sb = new StringBuilder();

        delegate void                           ReadMessageCallback(String data);

        ReadMessageCallback                     m_readMessageCallback;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;
		private System.Windows.Forms.RichTextBox richTextBox1;

		// ServiceRegistered
		//
		// Called by DNSServices core as a result of Register()
		// call
		//
        public void
        ServiceRegistered
                    (
                    DNSSDService service,
                    DNSSDFlags flags,
                    String name,
                    String regType,
                    String domain
                    )
        {
            m_name = name;

			//
			// Try to start browsing for other instances of this service
			//
            try
            {
                m_browser = m_service.Browse(0, 0, "_p2pchat._udp", null, m_eventManager);
            }
            catch
            {
                MessageBox.Show("Browse Failed", "Error");
                Application.Exit();
            }
        }

		//
		// ServiceFound
		//
		// Called by DNSServices core as a result of a Browse call
		//
		public void
        ServiceFound
				    (
				    DNSSDService    sref,
				    DNSSDFlags  	flags,
				    uint			ifIndex,
                    String          serviceName,
                    String          regType,
                    String          domain
				    )
		{
            if (serviceName != m_name)
            {
                PeerData peer = new PeerData();

                peer.InterfaceIndex = ifIndex;
                peer.Name = serviceName;
                peer.Type = regType;
                peer.Domain = domain;
                peer.Address = null;

                comboBox1.Items.Add(peer);

                if (comboBox1.Items.Count == 1)
                {
                    comboBox1.SelectedIndex = 0;
                }
            }
		}

        //
        // ServiceLost
        //
        // Called by DNSServices core as a result of a Browse call
        //
        public void
        ServiceLost
                    (
                    DNSSDService sref,
                    DNSSDFlags flags,
                    uint ifIndex,
                    String serviceName,
                    String regType,
                    String domain
                    )
        {
            PeerData peer = new PeerData();

            peer.InterfaceIndex = ifIndex;
            peer.Name = serviceName;
            peer.Type = regType;
            peer.Domain = domain;
            peer.Address = null;

            comboBox1.Items.Remove(peer);
        }

		//
		// ServiceResolved
		//
		// Called by DNSServices core as a result of DNSService.Resolve()
		// call
		//
        public void
        ServiceResolved
                    (
                    DNSSDService sref,
                    DNSSDFlags flags,
                    uint ifIndex,
                    String fullName,
                    String hostName,
                    ushort port,
                    TXTRecord txtRecord
                    )
		{
            m_resolver.Stop();
            m_resolver = null;

            PeerData peer = (PeerData)comboBox1.SelectedItem;

            peer.Port = port;

			//
			// Query for the IP address associated with "hostName"
			//
            try
            {
                m_resolver = m_service.QueryRecord(0, ifIndex, hostName, DNSSDRRType.kDNSSDType_A, DNSSDRRClass.kDNSSDClass_IN, m_eventManager );
            }
            catch
            {
                MessageBox.Show("QueryRecord Failed", "Error");
                Application.Exit();
            }
		}

		//
		// QueryAnswered
		//
		// Called by DNSServices core as a result of DNSService.QueryRecord()
		// call
		//
		public void
		QueryAnswered
			(
            DNSSDService    service, 
            DNSSDFlags      flags,
            uint            ifIndex,
            String          fullName,
            DNSSDRRType     rrtype,
            DNSSDRRClass    rrclass,
            Object          rdata,
            uint            ttl
            )
        {
			//
			// Stop the resolve to reduce the burden on the network
			//
            m_resolver.Stop();
            m_resolver = null;

            PeerData peer = (PeerData) comboBox1.SelectedItem;
			uint bits = BitConverter.ToUInt32( (Byte[])rdata, 0);
			System.Net.IPAddress address = new System.Net.IPAddress(bits);

            peer.Address = address;
		}

        public void
        OperationFailed
                    (
                    DNSSDService service,
                    DNSSDError error
                    )
        {
            MessageBox.Show("Operation returned an error code " + error, "Error");
        }

        //
        // OnReadMessage
        //
        // Called when there is data to be read on a socket
        //
        // This is called (indirectly) from OnReadSocket()
        //
        private void
        OnReadMessage
                (
                String msg
                )
        {
            int rgb = 0;

            for (int i = 0; i < msg.Length && msg[i] != ':'; i++)
            {
                rgb = rgb ^ ((int)msg[i] << (i % 3 + 2) * 8);

            }

            Color color = Color.FromArgb(rgb & 0x007F7FFF);

            richTextBox1.SelectionColor = color;
            richTextBox1.AppendText(msg + Environment.NewLine);
        }

		//
		// OnReadSocket
		//
		// Called by the .NET core when there is data to be read on a socket
		//
		// This is called from a worker thread by the .NET core
		//
		private void
		OnReadSocket
				(
				IAsyncResult ar
				)
		{
			try
			{
				int read = m_socket.EndReceive(ar);

				if (read > 0)
				{
					String msg = Encoding.UTF8.GetString(m_buffer, 0, read);
					Invoke(m_readMessageCallback, new Object[]{msg});
				}

				m_socket.BeginReceive(m_buffer, 0, BUFFER_SIZE, 0, new AsyncCallback(OnReadSocket), this);
			}
			catch
			{
			}
		}


		public SimpleChat()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

            try
            {
                m_service = new DNSSDService();
            }
            catch
            {
                MessageBox.Show("Bonjour Service is not available", "Error");
                Application.Exit();
            }

			//
			// Associate event handlers with all the Bonjour events that the app is interested in.
			//
            m_eventManager = new DNSSDEventManager();
            m_eventManager.ServiceRegistered += new _IDNSSDEvents_ServiceRegisteredEventHandler(this.ServiceRegistered);
            m_eventManager.ServiceFound += new _IDNSSDEvents_ServiceFoundEventHandler(this.ServiceFound);
            m_eventManager.ServiceLost += new _IDNSSDEvents_ServiceLostEventHandler(this.ServiceLost);
            m_eventManager.ServiceResolved += new _IDNSSDEvents_ServiceResolvedEventHandler(this.ServiceResolved);
            m_eventManager.QueryRecordAnswered += new _IDNSSDEvents_QueryRecordAnsweredEventHandler(this.QueryAnswered);
            m_eventManager.OperationFailed += new _IDNSSDEvents_OperationFailedEventHandler(this.OperationFailed);

			//
			// Socket read handler
			//
			m_readMessageCallback = new ReadMessageCallback(OnReadMessage);

			this.Load += new System.EventHandler(this.Form1_Load);

			this.AcceptButton = button1;
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void
		Dispose( bool disposing )
		{
			if( disposing )
			{
				if (components != null) 
				{
					components.Dispose();
				}

				if (m_registrar != null)
				{
					m_registrar.Stop();
				}

				if (m_browser != null)
				{
					m_browser.Stop();
				}

                if (m_resolver != null)
                {
                    m_resolver.Stop();
                }

                m_eventManager.ServiceFound -= new _IDNSSDEvents_ServiceFoundEventHandler(this.ServiceFound);
                m_eventManager.ServiceLost -= new _IDNSSDEvents_ServiceLostEventHandler(this.ServiceLost);
                m_eventManager.ServiceResolved -= new _IDNSSDEvents_ServiceResolvedEventHandler(this.ServiceResolved);
                m_eventManager.QueryRecordAnswered -= new _IDNSSDEvents_QueryRecordAnsweredEventHandler(this.QueryAnswered);
                m_eventManager.OperationFailed -= new _IDNSSDEvents_OperationFailedEventHandler(this.OperationFailed);
			}
			base.Dispose( disposing );
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.comboBox1 = new System.Windows.Forms.ComboBox();
			this.textBox2 = new System.Windows.Forms.TextBox();
			this.button1 = new System.Windows.Forms.Button();
			this.label1 = new System.Windows.Forms.Label();
			this.richTextBox1 = new System.Windows.Forms.RichTextBox();
			this.SuspendLayout();
			// 
			// comboBox1
			// 
			this.comboBox1.Anchor = ((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.comboBox1.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.comboBox1.Location = new System.Drawing.Point(59, 208);
			this.comboBox1.Name = "comboBox1";
			this.comboBox1.Size = new System.Drawing.Size(224, 21);
			this.comboBox1.Sorted = true;
			this.comboBox1.TabIndex = 5;
			this.comboBox1.SelectedIndexChanged += new System.EventHandler(this.comboBox1_SelectedIndexChanged);
			// 
			// textBox2
			// 
			this.textBox2.Anchor = ((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.textBox2.Location = new System.Drawing.Point(8, 248);
			this.textBox2.Name = "textBox2";
			this.textBox2.ScrollBars = System.Windows.Forms.ScrollBars.Horizontal;
			this.textBox2.Size = new System.Drawing.Size(192, 20);
			this.textBox2.TabIndex = 2;
			this.textBox2.Text = "";
			this.textBox2.TextChanged += new System.EventHandler(this.textBox2_TextChanged);
			// 
			// button1
			// 
			this.button1.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right);
			this.button1.Enabled = false;
			this.button1.Location = new System.Drawing.Point(208, 248);
			this.button1.Name = "button1";
			this.button1.TabIndex = 3;
			this.button1.Text = "Send";
			this.button1.Click += new System.EventHandler(this.button1_Click);
			// 
			// label1
			// 
			this.label1.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left);
			this.label1.Location = new System.Drawing.Point(8, 210);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(48, 16);
			this.label1.TabIndex = 4;
			this.label1.Text = "Talk To:";
			// 
			// richTextBox1
			// 
			this.richTextBox1.Anchor = (((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
				| System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.richTextBox1.Location = new System.Drawing.Point(8, 8);
			this.richTextBox1.Name = "richTextBox1";
			this.richTextBox1.ReadOnly = true;
			this.richTextBox1.Size = new System.Drawing.Size(272, 184);
			this.richTextBox1.TabIndex = 1;
			this.richTextBox1.Text = "";
			// 
			// Form1
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(292, 273);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.richTextBox1,
																		  this.label1,
																		  this.button1,
																		  this.textBox2,
																		  this.comboBox1});
			this.Name = "Form1";
			this.Text = "SimpleChat.NET";
			this.ResumeLayout(false);

		}
		#endregion

		private void Form1_Load(object sender, EventArgs e) 
		{
			IPEndPoint localEP = new IPEndPoint(System.Net.IPAddress.Any, 0);
			
			//
			// create the socket and bind to INADDR_ANY
			//
			m_socket	= new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
			m_socket.Bind(localEP);
			localEP = (IPEndPoint) m_socket.LocalEndPoint;

			//
			// start asynchronous read
			//
			m_socket.BeginReceive(m_buffer, 0, BUFFER_SIZE, 0, new AsyncCallback(this.OnReadSocket), this);   

			try
			{
				//
				// start the register and browse operations
				//
				m_registrar	=	m_service.Register( 0, 0, System.Environment.UserName, "_p2pchat._udp", null, null, ( ushort ) localEP.Port, null, m_eventManager );	
			}
			catch
			{
				MessageBox.Show("Bonjour service is not available", "Error");
				Application.Exit();
			}
		}

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main() 
		{
			Application.Run(new SimpleChat());
		}

		//
		// send the message to a peer
		//
		private void button1_Click(object sender, System.EventArgs e)
		{
			PeerData peer = (PeerData) comboBox1.SelectedItem;

			String message = m_name + ": " + textBox2.Text;

			Byte[] bytes = Encoding.UTF8.GetBytes(message);

            IPEndPoint endPoint = new IPEndPoint( peer.Address, peer.Port );



            m_socket.SendTo(bytes, endPoint);

			richTextBox1.SelectionColor = Color.Black;

			richTextBox1.AppendText(textBox2.Text + "\n");

			textBox2.Text = "";
		}

		//
		// called when typing in message box
		//
		private void textBox2_TextChanged(object sender, System.EventArgs e)
		{
			PeerData peer = (PeerData) comboBox1.SelectedItem;
            button1.Enabled = ((peer.Address != null) && (textBox2.Text.Length > 0));
		}

		//
		// called when peer target changes
		//
		/// <summary>
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void comboBox1_SelectedIndexChanged(object sender, System.EventArgs e)
		{
			PeerData peer = (PeerData) comboBox1.SelectedItem;

			try
			{
				m_resolver = m_service.Resolve(0, peer.InterfaceIndex, peer.Name, peer.Type, peer.Domain, m_eventManager);
			}
			catch
			{
				MessageBox.Show("Unable to Resolve service", "Error");
				Application.Exit();
			}
		}
	}

    //
    // PeerData
    //
    // Holds onto the information associated with a peer on the network
    //
    public class PeerData
    {
        public uint InterfaceIndex;
        public String Name;
        public String Type;
        public String Domain;
        public IPAddress Address;
        public int Port;

        public override String
        ToString()
        {
            return Name;
        }

        public override bool
        Equals(object other)
        {
            bool result = false;

            if (other != null)
            {
                if ((object)this == other)
                {
                    result = true;
                }
                else if (other is PeerData)
                {
                    PeerData otherPeerData = (PeerData)other;

                    result = (this.Name == otherPeerData.Name);
                }
            }

            return result;
        }


        public override int
        GetHashCode()
        {
            return Name.GetHashCode();
        }
    };
}
