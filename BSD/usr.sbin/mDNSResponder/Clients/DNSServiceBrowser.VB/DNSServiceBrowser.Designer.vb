<Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()> _
Partial Class DNSServiceBrowser
    Inherits System.Windows.Forms.Form

    'Form overrides dispose to clean up the component list.
    <System.Diagnostics.DebuggerNonUserCode()> _
    Protected Overrides Sub Dispose(ByVal disposing As Boolean)
        Try
            If disposing AndAlso components IsNot Nothing Then
                components.Dispose()
            End If
        Finally
            MyBase.Dispose(disposing)
        End Try
    End Sub

    'Required by the Windows Form Designer
    Private components As System.ComponentModel.IContainer

    'NOTE: The following procedure is required by the Windows Form Designer
    'It can be modified using the Windows Form Designer.  
    'Do not modify it using the code editor.
    <System.Diagnostics.DebuggerStepThrough()> _
    Private Sub InitializeComponent()
        Me.ComboBox1 = New System.Windows.Forms.ComboBox
        Me.ServiceNames = New System.Windows.Forms.ListBox
        Me.Label1 = New System.Windows.Forms.Label
        Me.Label2 = New System.Windows.Forms.Label
        Me.Label3 = New System.Windows.Forms.Label
        Me.Label4 = New System.Windows.Forms.Label
        Me.Label5 = New System.Windows.Forms.Label
        Me.NameField = New System.Windows.Forms.TextBox
        Me.PortField = New System.Windows.Forms.TextBox
        Me.HostField = New System.Windows.Forms.TextBox
        Me.DomainField = New System.Windows.Forms.TextBox
        Me.TypeField = New System.Windows.Forms.TextBox
        Me.TextRecord = New System.Windows.Forms.ListBox
        Me.SuspendLayout()
        '
        'ComboBox1
        '
        Me.ComboBox1.FormattingEnabled = True
        Me.ComboBox1.Items.AddRange(New Object() {"_accessone._tcp", "_accountedge._tcp", "_actionitems._tcp", "_addressbook._tcp", "_aecoretech._tcp", "_afpovertcp._tcp", "_airport._tcp", "_animobserver._tcp", "_animolmd._tcp", "_apple-sasl._tcp", "_aquamon._tcp", "_async._tcp", "_auth._tcp", "_beep._tcp", "_bfagent._tcp", "_bootps._udp", "_bousg._tcp", "_bsqdea._tcp", "_cheat._tcp", "_chess._tcp", "_clipboard._tcp", "_collection._tcp", "_contactserver._tcp", "_cvspserver._tcp", "_cytv._tcp", "_daap._tcp", "_difi._tcp", "_distcc._tcp", "_dossier._tcp", "_dpap._tcp", "_earphoria._tcp", "_ebms._tcp", "_ebreg._tcp", "_ecbyesfsgksc._tcp", "_eheap._tcp", "_embrace._tcp", "_eppc._tcp", "_eventserver._tcp", "_exec._tcp", "_facespan._tcp", "_faxstfx._tcp", "_fish._tcp", "_fjork._tcp", "_fmpro-internal._tcp", "_ftp._tcp", "_ftpcroco._tcp", "_gbs-smp._tcp", "_gbs-stp._tcp", "_grillezvous._tcp", "_h323._tcp", "_hotwayd._tcp", "_http._tcp", "_hydra._tcp", "_ica-networking._tcp", "_ichalkboard._tcp", "_ichat._tcp", "_iconquer._tcp", "_ifolder._tcp", "_ilynx._tcp", "_imap._tcp", "_imidi._tcp", "_ipbroadcaster._tcp", "_ipp._tcp", "_ishare._tcp", "_isparx._tcp", "_ispq-vc._tcp", "_isticky._tcp", "_istorm._tcp", "_iwork._tcp", "_lan2p._tcp", "_ldap._tcp", "_liaison._tcp", "_login._tcp", "_lontalk._tcp", "_lonworks._tcp", "_macfoh-remote._tcp", "_macminder._tcp", "_moneyworks._tcp", "_mp3sushi._tcp", "_mttp._tcp", "_ncbroadcast._tcp", "_ncdirect._tcp", "_ncsyncserver._tcp", "_net-assistant._tcp", "_newton-dock._tcp", "_nfs._udp", "_nssocketport._tcp", "_odabsharing._tcp", "_omni-bookmark._tcp", "_openbase._tcp", "_p2pchat._udp", "_pdl-datastream._tcp", "_poch._tcp", "_pop3._tcp", "_postgresql._tcp", "_presence._tcp", "_printer._tcp", "_ptp._tcp", "_quinn._tcp", "_raop._tcp", "_rce._tcp", "_realplayfavs._tcp", "_rfb._tcp", "_riousbprint._tcp", "_rtsp._tcp", "_safarimenu._tcp", "_sallingclicker._tcp", "_scone._tcp", "_sdsharing._tcp", "_see._tcp", "_seeCard._tcp", "_serendipd._tcp", "_servermgr._tcp", "_sge-exec._tcp", "_sge-qmaster._tcp", "_shell._tcp", "_shout._tcp", "_shoutcast._tcp", "_soap._tcp", "_spike._tcp", "_spincrisis._tcp", "_spl-itunes._tcp", "_spr-itunes._tcp", "_ssh._tcp", "_ssscreenshare._tcp", "_stickynotes._tcp", "_strateges._tcp", "_sxqdea._tcp", "_sybase-tds._tcp", "_teamlist._tcp", "_teleport._udp", "_telnet._tcp", "_tftp._udp", "_ticonnectmgr._tcp", "_tinavigator._tcp", "_tryst._tcp", "_upnp._tcp", "_utest._tcp", "_vue4rendercow._tcp", "_webdav._tcp", "_whamb._tcp", "_wired._tcp", "_workgroup._tcp", "_workstation._tcp", "_wormhole._tcp", "_ws._tcp", "_xserveraid._tcp", "_xsync._tcp", "_xtshapro._tcp"})
        Me.ComboBox1.Location = New System.Drawing.Point(13, 13)
        Me.ComboBox1.Name = "ComboBox1"
        Me.ComboBox1.Size = New System.Drawing.Size(252, 21)
        Me.ComboBox1.Sorted = True
        Me.ComboBox1.TabIndex = 0
        '
        'ServiceNames
        '
        Me.ServiceNames.Anchor = CType((((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Bottom) _
                    Or System.Windows.Forms.AnchorStyles.Left) _
                    Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.ServiceNames.FormattingEnabled = True
        Me.ServiceNames.Location = New System.Drawing.Point(13, 41)
        Me.ServiceNames.Name = "ServiceNames"
        Me.ServiceNames.Size = New System.Drawing.Size(662, 251)
        Me.ServiceNames.Sorted = True
        Me.ServiceNames.TabIndex = 1
        '
        'Label1
        '
        Me.Label1.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Left), System.Windows.Forms.AnchorStyles)
        Me.Label1.AutoSize = True
        Me.Label1.Location = New System.Drawing.Point(16, 311)
        Me.Label1.Name = "Label1"
        Me.Label1.Size = New System.Drawing.Size(38, 13)
        Me.Label1.TabIndex = 2
        Me.Label1.Text = "Name:"
        '
        'Label2
        '
        Me.Label2.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Left), System.Windows.Forms.AnchorStyles)
        Me.Label2.AutoSize = True
        Me.Label2.Location = New System.Drawing.Point(16, 439)
        Me.Label2.Name = "Label2"
        Me.Label2.Size = New System.Drawing.Size(29, 13)
        Me.Label2.TabIndex = 3
        Me.Label2.Text = "Port:"
        '
        'Label3
        '
        Me.Label3.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Left), System.Windows.Forms.AnchorStyles)
        Me.Label3.AutoSize = True
        Me.Label3.Location = New System.Drawing.Point(16, 407)
        Me.Label3.Name = "Label3"
        Me.Label3.Size = New System.Drawing.Size(32, 13)
        Me.Label3.TabIndex = 4
        Me.Label3.Text = "Host:"
        '
        'Label4
        '
        Me.Label4.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Left), System.Windows.Forms.AnchorStyles)
        Me.Label4.AutoSize = True
        Me.Label4.Location = New System.Drawing.Point(16, 374)
        Me.Label4.Name = "Label4"
        Me.Label4.Size = New System.Drawing.Size(46, 13)
        Me.Label4.TabIndex = 5
        Me.Label4.Text = "Domain:"
        '
        'Label5
        '
        Me.Label5.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Left), System.Windows.Forms.AnchorStyles)
        Me.Label5.AutoSize = True
        Me.Label5.Location = New System.Drawing.Point(16, 342)
        Me.Label5.Name = "Label5"
        Me.Label5.Size = New System.Drawing.Size(34, 13)
        Me.Label5.TabIndex = 6
        Me.Label5.Text = "Type:"
        '
        'NameField
        '
        Me.NameField.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Left), System.Windows.Forms.AnchorStyles)
        Me.NameField.Location = New System.Drawing.Point(69, 309)
        Me.NameField.Name = "NameField"
        Me.NameField.ReadOnly = True
        Me.NameField.Size = New System.Drawing.Size(195, 20)
        Me.NameField.TabIndex = 7
        '
        'PortField
        '
        Me.PortField.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Left), System.Windows.Forms.AnchorStyles)
        Me.PortField.Location = New System.Drawing.Point(69, 436)
        Me.PortField.Name = "PortField"
        Me.PortField.ReadOnly = True
        Me.PortField.Size = New System.Drawing.Size(195, 20)
        Me.PortField.TabIndex = 8
        '
        'HostField
        '
        Me.HostField.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Left), System.Windows.Forms.AnchorStyles)
        Me.HostField.Location = New System.Drawing.Point(69, 403)
        Me.HostField.Name = "HostField"
        Me.HostField.ReadOnly = True
        Me.HostField.Size = New System.Drawing.Size(195, 20)
        Me.HostField.TabIndex = 9
        '
        'DomainField
        '
        Me.DomainField.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Left), System.Windows.Forms.AnchorStyles)
        Me.DomainField.Location = New System.Drawing.Point(69, 371)
        Me.DomainField.Name = "DomainField"
        Me.DomainField.ReadOnly = True
        Me.DomainField.Size = New System.Drawing.Size(195, 20)
        Me.DomainField.TabIndex = 10
        '
        'TypeField
        '
        Me.TypeField.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Left), System.Windows.Forms.AnchorStyles)
        Me.TypeField.Location = New System.Drawing.Point(69, 340)
        Me.TypeField.Name = "TypeField"
        Me.TypeField.ReadOnly = True
        Me.TypeField.Size = New System.Drawing.Size(195, 20)
        Me.TypeField.TabIndex = 11
        '
        'TextRecord
        '
        Me.TextRecord.Anchor = CType(((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Left) _
                    Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.TextRecord.FormattingEnabled = True
        Me.TextRecord.Location = New System.Drawing.Point(278, 309)
        Me.TextRecord.Name = "TextRecord"
        Me.TextRecord.SelectionMode = System.Windows.Forms.SelectionMode.None
        Me.TextRecord.Size = New System.Drawing.Size(397, 147)
        Me.TextRecord.TabIndex = 12
        '
        'Form1
        '
        Me.AutoScaleDimensions = New System.Drawing.SizeF(6.0!, 13.0!)
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.ClientSize = New System.Drawing.Size(690, 480)
        Me.Controls.Add(Me.TextRecord)
        Me.Controls.Add(Me.TypeField)
        Me.Controls.Add(Me.DomainField)
        Me.Controls.Add(Me.HostField)
        Me.Controls.Add(Me.PortField)
        Me.Controls.Add(Me.NameField)
        Me.Controls.Add(Me.Label5)
        Me.Controls.Add(Me.Label4)
        Me.Controls.Add(Me.Label3)
        Me.Controls.Add(Me.Label2)
        Me.Controls.Add(Me.Label1)
        Me.Controls.Add(Me.ServiceNames)
        Me.Controls.Add(Me.ComboBox1)
        Me.Name = "Form1"
        Me.Text = "Bonjour Browser"
        Me.ResumeLayout(False)
        Me.PerformLayout()

    End Sub
    Friend WithEvents ComboBox1 As System.Windows.Forms.ComboBox
    Friend WithEvents ServiceNames As System.Windows.Forms.ListBox
    Friend WithEvents Label1 As System.Windows.Forms.Label
    Friend WithEvents Label2 As System.Windows.Forms.Label
    Friend WithEvents Label3 As System.Windows.Forms.Label
    Friend WithEvents Label4 As System.Windows.Forms.Label
    Friend WithEvents Label5 As System.Windows.Forms.Label
    Friend WithEvents NameField As System.Windows.Forms.TextBox
    Friend WithEvents PortField As System.Windows.Forms.TextBox
    Friend WithEvents HostField As System.Windows.Forms.TextBox
    Friend WithEvents DomainField As System.Windows.Forms.TextBox
    Friend WithEvents TypeField As System.Windows.Forms.TextBox
    Friend WithEvents TextRecord As System.Windows.Forms.ListBox

End Class
