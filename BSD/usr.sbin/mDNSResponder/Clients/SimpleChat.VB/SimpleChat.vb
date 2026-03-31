'
' Copyright (c) 2010 Apple Inc. All rights reserved.
'
' Licensed under the Apache License, Version 2.0 (the "License");
' you may not use this file except in compliance with the License.
' You may obtain a copy of the License at
' 
'     http://www.apache.org/licenses/LICENSE-2.0
' 
' Unless required by applicable law or agreed to in writing, software
' distributed under the License is distributed on an "AS IS" BASIS,
' WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
' See the License for the specific language governing permissions and
' limitations under the License.
'

Imports System.Net
Imports System.Net.Sockets
Imports System.Data
Imports System.Text

Public Class SimpleChat
	'
	' Associate Bonjour events with event handlers
	'
    Public WithEvents MyEventManager As New Bonjour.DNSSDEventManager
    Private m_service As New Bonjour.DNSSDService
    Private m_registrar As Bonjour.DNSSDService
    Private m_browser As Bonjour.DNSSDService
    Private m_resolver As Bonjour.DNSSDService
    Private m_socket As New Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp)
    Private m_port As Integer
    Private m_buffer(1024 * 32) As Byte
    Private m_async As IAsyncResult
    Public Delegate Sub SocketDelegate(ByVal msg As String)
    Private m_socketDelegate As SocketDelegate
    Private m_name As String

    Public Sub New()
        MyBase.New()

        'This call is required by the Windows Form Designer.
        InitializeComponent()

        Button1.Enabled = False

        m_socketDelegate = New SocketDelegate(AddressOf MessageReceived)

        Dim endPoint As New IPEndPoint(IPAddress.Any, 0)
        m_socket.Bind(endPoint)
        endPoint = m_socket.LocalEndPoint
        m_port = endPoint.Port

        Dim txtRecord As Bonjour.TXTRecord
        m_async = m_socket.BeginReceive(m_buffer, 0, m_buffer.Length, SocketFlags.Partial, New AsyncCallback(AddressOf OnReceive), Me)
        m_registrar = m_service.Register(0, 0, Environment.UserName, "_p2pchat._udp", vbNullString, vbNullString, m_port, txtRecord, MyEventManager)
    End Sub

	'
	' Called when Bonjour core finished registering a service successfully
	'
    Public Sub MyEventManager_ServiceRegistered(ByVal registrar As Bonjour.DNSSDService, ByVal flags As Bonjour.DNSSDFlags, ByVal name As String, ByVal regType As String, ByVal domain As String) Handles MyEventManager.ServiceRegistered
        m_name = name
        m_browser = m_service.Browse(0, 0, regType, vbNullString, MyEventManager)
    End Sub

	'
	' Called when a service is found
	'
    Public Sub MyEventManager_ServiceFound(ByVal browser As Bonjour.DNSSDService, ByVal flags As Bonjour.DNSSDFlags, ByVal ifIndex As UInteger, ByVal serviceName As String, ByVal regtype As String, ByVal domain As String) Handles MyEventManager.ServiceFound
        If (serviceName <> m_name) Then
            Dim peer As PeerData = New PeerData
            peer.InterfaceIndex = ifIndex
            peer.Name = serviceName
            peer.Type = regtype
            peer.Domain = domain
            ComboBox1.Items.Add(peer)
            ComboBox1.SelectedIndex = 0
        End If
    End Sub

	'
	' Called when a service is lost
	'
    Public Sub MyEventManager_ServiceLost(ByVal browser As Bonjour.DNSSDService, ByVal flags As Bonjour.DNSSDFlags, ByVal ifIndex As UInteger, ByVal serviceName As String, ByVal regtype As String, ByVal domain As String) Handles MyEventManager.ServiceLost
        ComboBox1.Items.Remove(serviceName)
    End Sub

	'
	' Called when a service is resolved
	'
    Public Sub MyEventManager_ServiceResolved(ByVal resolver As Bonjour.DNSSDService, ByVal flags As Bonjour.DNSSDFlags, ByVal ifIndex As UInteger, ByVal fullname As String, ByVal hostname As String, ByVal port As UShort, ByVal record As Bonjour.TXTRecord) Handles MyEventManager.ServiceResolved
        m_resolver.Stop()
        Dim peer As PeerData = ComboBox1.SelectedItem
        peer.Port = port
        m_resolver = m_service.QueryRecord(0, ifIndex, hostname, Bonjour.DNSSDRRType.kDNSSDType_A, Bonjour.DNSSDRRClass.kDNSSDClass_IN, MyEventManager)
    End Sub

    Public Sub MyEventManager_QueryAnswered(ByVal resolver As Bonjour.DNSSDService, ByVal flags As Bonjour.DNSSDFlags, ByVal ifIndex As UInteger, ByVal fullName As String, ByVal rrtype As Bonjour.DNSSDRRType, ByVal rrclass As Bonjour.DNSSDRRClass, ByVal rdata As Object, ByVal ttl As UInteger) Handles MyEventManager.QueryRecordAnswered
        m_resolver.Stop()
        Dim peer As PeerData = ComboBox1.SelectedItem
        Dim bits As UInteger = BitConverter.ToUInt32(rdata, 0)
        Dim address As IPAddress = New System.Net.IPAddress(bits)
        peer.Address = address
    End Sub

    Public Sub MyEventManager_OperationFailed(ByVal registrar As Bonjour.DNSSDService, ByVal errorCode As Bonjour.DNSSDError) Handles MyEventManager.OperationFailed
        MessageBox.Show("Operation failed error code: " + errorCode)
    End Sub

    Private Sub Button1_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button1.Click
        Dim peer As PeerData = ComboBox1.SelectedItem
        Dim message As String = m_name + ": " + TextBox2.Text
        Dim bytes As Byte() = Encoding.UTF8.GetBytes(message)
        Dim endPoint As IPEndPoint = New IPEndPoint(peer.Address, peer.Port)
        m_socket.SendTo(bytes, 0, bytes.Length, 0, endPoint)
        TextBox1.AppendText(TextBox2.Text + Environment.NewLine)
        TextBox2.Text = ""
    End Sub

    Private Sub ComboBox1_SelectedIndexChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles ComboBox1.SelectedIndexChanged
        Dim peer As PeerData = ComboBox1.SelectedItem
        m_resolver = m_service.Resolve(0, peer.InterfaceIndex, peer.Name, peer.Type, peer.Domain, MyEventManager)
    End Sub
    Private Sub TextBox2_TextChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles TextBox2.TextChanged
        Dim peer As PeerData = ComboBox1.SelectedItem
        If ((peer.Address IsNot Nothing) And TextBox2.Text.Length > 0) Then
            Button1.Enabled = True
        Else
            Button1.Enabled = False
        End If
    End Sub
    Public Sub MessageReceived(ByVal msg As System.String)
        TextBox1.AppendText(msg)
    End Sub
    Private Sub OnReceive(ByVal ar As IAsyncResult)
        Dim bytesReceived As Integer = m_socket.EndReceive(ar)
        If (bytesReceived > 0) Then
            Dim msg As String = Encoding.UTF8.GetString(m_buffer, 0, bytesReceived)
            Me.Invoke(m_socketDelegate, msg)
        End If
        m_async = m_socket.BeginReceive(m_buffer, 0, m_buffer.Length, SocketFlags.Partial, New AsyncCallback(AddressOf OnReceive), Me)
    End Sub
End Class

Public Class PeerData
    Public InterfaceIndex As UInteger
    Public Name As String
    Public Type As String
    Public Domain As String
    Public Address As IPAddress
    Public Port As UShort

    Overrides Function ToString() As String
        Return Name
    End Function
End Class
