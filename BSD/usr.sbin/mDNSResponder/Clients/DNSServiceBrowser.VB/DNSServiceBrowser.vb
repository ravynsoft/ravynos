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

Public Class DNSServiceBrowser
    Public WithEvents MyEventManager As New Bonjour.DNSSDEventManager

    Private m_service As New Bonjour.DNSSDService
    Private m_browser As Bonjour.DNSSDService
    Private m_resolver As Bonjour.DNSSDService

    Public Sub New()
        MyBase.New()

        'This call is required by the Windows Form Designer.
        InitializeComponent()

        ComboBox1.SelectedIndex = 0
    End Sub

	'Called when a service is found as a result of a browse operation
    Public Sub MyEventManager_ServiceFound(ByVal browser As Bonjour.DNSSDService, ByVal flags As Bonjour.DNSSDFlags, ByVal ifIndex As UInteger, ByVal serviceName As String, ByVal regtype As String, ByVal domain As String) Handles MyEventManager.ServiceFound
        Dim index As Integer
        index = ServiceNames.Items.IndexOf(serviceName)
		'
		' A simple reference counting scheme is implemented so that the same service
		' does not show up more than once in the browse list.  This can happen
		' if the machine has more than one network interface.
		'
		' If we have not seen this service before, then it is added to the browse list
		' Otherwise it's reference count is bumped up.
		'
        If index = -1 Then
            Dim browseData As New BrowseData
            browseData.ServiceName = serviceName
            browseData.RegType = regtype
            browseData.Domain = domain
            browseData.Refs = 1
            ServiceNames.Items.Add(browseData)
        Else
            Dim browseData As BrowseData
            browseData = ServiceNames.Items([index])
            browseData.Refs += 1
        End If
    End Sub

    Public Sub MyEventManager_ServiceLost(ByVal browser As Bonjour.DNSSDService, ByVal flags As Bonjour.DNSSDFlags, ByVal ifIndex As UInteger, ByVal serviceName As String, ByVal regtype As String, ByVal domain As String) Handles MyEventManager.ServiceLost
        Dim index As Integer
		'
		' See the above about reference counting
		'
        index = ServiceNames.Items.IndexOf(serviceName)
        If index <> -1 Then
            Dim browseData As BrowseData
            browseData = ServiceNames.Items([index])
            browseData.Refs -= 1
            If browseData.Refs = 0 Then
                ServiceNames.Items.Remove(serviceName)
            End If
        End If
    End Sub

    Public Sub MyEventManager_ServiceResolved(ByVal resolver As Bonjour.DNSSDService, ByVal flags As Bonjour.DNSSDFlags, ByVal ifIndex As UInteger, ByVal fullname As String, ByVal hostname As String, ByVal port As UShort, ByVal record As Bonjour.TXTRecord) Handles MyEventManager.ServiceResolved
		'
		' Once the service is resolved, the resolve operation is stopped. This reduces the burdne on the network
		'
        m_resolver.Stop()
        m_resolver = Nothing
        Dim browseData As BrowseData = ServiceNames.Items.Item(ServiceNames.SelectedIndex)
        NameField.Text = browseData.ServiceName
        TypeField.Text = browseData.RegType
        DomainField.Text = browseData.Domain
        HostField.Text = hostname
        PortField.Text = port

		'
		' The values found in the text record are assumed to be human readable strings.
		'
        If record IsNot Nothing Then
            For i As UInteger = 0 To record.GetCount() - 1
                Dim key As String = record.GetKeyAtIndex(i)
                If key.Length() > 0 Then
                    TextRecord.Items.Add(key + "=" + System.Text.Encoding.ASCII.GetString(record.GetValueAtIndex(i)))
                End If
            Next i
        End If
    End Sub

    Private Sub ClearServiceInfo()
        TextRecord.Items.Clear()
        NameField.Text = ""
        TypeField.Text = ""
        DomainField.Text = ""
        HostField.Text = ""
        PortField.Text = ""
    End Sub

    Private Sub ServiceNames_SelectedIndexChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles ServiceNames.SelectedIndexChanged
        If m_resolver IsNot Nothing Then
            m_resolver.Stop()
        End If
        Me.ClearServiceInfo()
        Dim browseData As BrowseData = ServiceNames.Items.Item(ServiceNames.SelectedIndex)

		'
		' Selecting a service instance starts a new resolve operation
		'
        m_resolver = m_service.Resolve(0, 0, browseData.ServiceName, browseData.RegType, browseData.Domain, MyEventManager)
    End Sub

    Private Sub ComboBox1_SelectedIndexChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles ComboBox1.SelectedIndexChanged
        If m_browser IsNot Nothing Then
            m_browser.Stop()
        End If

        ServiceNames.Items.Clear()
        Me.ClearServiceInfo()

		'
		' Selecting a service type start a new browse operation
		'

        m_browser = m_service.Browse(0, 0, ComboBox1.Items.Item(ComboBox1.SelectedIndex), "", MyEventManager)
    End Sub

    Private Sub ListBox2_SelectedIndexChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles TextRecord.SelectedIndexChanged

    End Sub
End Class
Public Class BrowseData
    Private m_serviceName As String
    Private m_regType As String
    Private m_domain As String
    Private m_refs As Integer

    Property ServiceName() As String
        Get
            Return m_serviceName
        End Get
        Set(ByVal Value As String)
            m_serviceName = Value
        End Set
    End Property

    Property RegType() As String
        Get
            Return m_regType
        End Get
        Set(ByVal Value As String)
            m_regType = Value
        End Set
    End Property

    Property Domain() As String
        Get
            Return m_domain
        End Get
        Set(ByVal Value As String)
            m_domain = Value
        End Set
    End Property

    Property Refs As Integer
        Get
            Return m_refs
        End Get
        Set(ByVal Value As Integer)
            m_refs = Value
        End Set
    End Property

    Public Overrides Function Equals(obj As Object) As Boolean
        If obj Is Nothing Then
            Return False
        Else
            Return m_serviceName.Equals(obj.ToString)
        End If
    End Function

    Public Overrides Function ToString() As String
        Return m_serviceName
    End Function

End Class
