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

#include "stdafx.h"
#include "PrinterSetupWizardApp.h"
#include "PrinterSetupWizardSheet.h"
#include "CommonServices.h"
#include "DebugServices.h"
#include "WinServices.h"
#include "About.h"
#include "tcpxcv.h"
#include <winspool.h>
#include <string>
#include <shlwapi.h>

// unreachable code
#pragma warning(disable:4702)


#if( !TARGET_OS_WINDOWS_CE )
#	include	<mswsock.h>
#	include	<process.h>
#endif


#if defined( UNICODE ) || defined( _UNICODE )
#	define GetEnv	_wgetenv
#else
#	define GetEnv	getenv
#endif

static TCHAR*
g_printerDriverFiles[] =		// Printer driver files
{
	TEXT( "ps5ui.dll" ),
	TEXT( "pscript.hlp" ),
	TEXT( "pscript.ntf" ),
	TEXT( "pscript5.dll" ),
	TEXT( "cups6.ini" ),
	TEXT( "cupsui6.dll" ),
	TEXT( "cupsps6.dll" )
};


// Private Messages

#define WM_SOCKET_EVENT		( WM_USER + 0x100 )
#define WM_PROCESS_EVENT	( WM_USER + 0x101 )


static BOOL
Is64BitWindows()
{
#if defined(_WIN64)
	return TRUE;  // 64-bit programs run only on Win64
#else
	typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS)( HANDLE, PBOOL );
	LPFN_ISWOW64PROCESS fnIsWow64Process;
	BOOL bIsWow64 = FALSE;

    fnIsWow64Process = ( LPFN_ISWOW64PROCESS ) GetProcAddress( GetModuleHandle( TEXT( "kernel32" ) ), "IsWow64Process" );
  
    if ( fnIsWow64Process != NULL )
    {
		BOOL ok;

        ok = fnIsWow64Process( GetCurrentProcess(), &bIsWow64 );

		if ( !ok )
		{
			bIsWow64 = FALSE;
		}
	}

	return bIsWow64;
#endif
}


// CPrinterSetupWizardSheet
CPrinterSetupWizardSheet * CPrinterSetupWizardSheet::m_self;

IMPLEMENT_DYNAMIC(CPrinterSetupWizardSheet, CPropertySheet)
CPrinterSetupWizardSheet::CPrinterSetupWizardSheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage),
	m_selectedPrinter(NULL),
	m_driverThreadExitCode( 0 ),
	m_driverThreadFinished( false ),
	m_pdlBrowser( NULL ),
	m_ippBrowser( NULL ),
	m_lprBrowser( NULL ),
	m_lastPage( NULL )
{
	m_arrow		=	LoadCursor(0, IDC_ARROW);
	m_wait		=	LoadCursor(0, IDC_APPSTARTING);
	m_active	=	m_arrow;
	m_self		=	this;
	
	Init();

	LoadPrinterNames();
}


CPrinterSetupWizardSheet::~CPrinterSetupWizardSheet()
{
	Printer * printer;

	while ( m_printers.size() > 0 )
	{
		printer = m_printers.front();
		m_printers.pop_front();

		delete printer;
	}

	m_self = NULL;
}


// ------------------------------------------------------
// SetSelectedPrinter
//
// Manages setting a printer as the printer to install.  Stops
// any pending resolves.  
//	
void
CPrinterSetupWizardSheet::SetSelectedPrinter(Printer * printer)
{
	check( !printer || ( printer != m_selectedPrinter ) );

	m_selectedPrinter = printer;
}


OSStatus
CPrinterSetupWizardSheet::LoadPrinterNames()
{
	PBYTE		buffer	=	NULL;
	OSStatus	err		= 0;

	//
	// rdar://problem/3701926 - Printer can't be installed twice
	//
	// First thing we want to do is make sure the printer isn't already installed.
	// If the printer name is found, we'll try and rename it until we
	// find a unique name
	//
	DWORD dwNeeded = 0, dwNumPrinters = 0;

	BOOL ok = EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 4, NULL, 0, &dwNeeded, &dwNumPrinters);
	err = translate_errno( ok, errno_compat(), kUnknownErr );

	if ((err == ERROR_INSUFFICIENT_BUFFER) && (dwNeeded > 0))
	{
		try
		{
			buffer = new unsigned char[dwNeeded];
		}
		catch (...)
		{
			buffer = NULL;
		}
	
		require_action( buffer, exit, kNoMemoryErr );
		ok = EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 4, buffer, dwNeeded, &dwNeeded, &dwNumPrinters);
		err = translate_errno( ok, errno_compat(), kUnknownErr );
		require_noerr( err, exit );

		for (DWORD index = 0; index < dwNumPrinters; index++)
		{
			PRINTER_INFO_4 * lppi4 = (PRINTER_INFO_4*) (buffer + index * sizeof(PRINTER_INFO_4));

			m_printerNames.push_back( lppi4->pPrinterName );
		}
	}

exit:

	if (buffer != NULL)
	{
		delete [] buffer;
	}

	return err;
}



// ------------------------------------------------------
// InstallPrinter
//
// Installs a printer with Windows.
//
// Note: this works one of two ways, depending on whether
// there are drivers already installed for this printer.
// If there are, then we can just create a port with XcvData,
// and then call AddPrinter.  If not, we use the printui.dll
// to install the printer. Actually installing drivers that
// are not currently installed is painful, and it's much
// easier and less error prone to just let printui.dll do
// the hard work for us.
//	

OSStatus
CPrinterSetupWizardSheet::InstallPrinter(Printer * printer)
{
	Logger		log;
	CUPSLibrary	cupsLib;
	Service	*	service		= NULL;
	BOOL		ok;
	OSStatus	err = 0;

	service = printer->services.front();
	check( service );

	if ( printer->isCUPSPrinter && cupsLib.IsInstalled() )
	{
		err = InstallPrinterCUPS( printer, service, cupsLib );
		require_noerr( err, exit );
	}
	else
	{
		//
		// if the driver isn't installed, then install it
		//

		if ( !printer->driverInstalled )
		{
			DWORD		dwResult;
			HANDLE		hThread;
			unsigned	threadID;

			m_driverThreadFinished = false;
		
			//
			// create the thread
			//
			hThread = (HANDLE) _beginthreadex_compat( NULL, 0, InstallDriverThread, printer, 0, &threadID );
			err = translate_errno( hThread, (OSStatus) GetLastError(), kUnknownErr );
			require_noerr_with_log( log, "_beginthreadex_compat()", err, exit );
				
			//
			// go modal
			//
			while (!m_driverThreadFinished)
			{
				MSG msg;
		
				GetMessage( &msg, m_hWnd, 0, 0 );
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			//
			// Wait until child process exits.
			//
			dwResult = WaitForSingleObject( hThread, INFINITE );
			err = translate_errno( dwResult == WAIT_OBJECT_0, errno_compat(), err = kUnknownErr );
			require_noerr_with_log( log, "WaitForSingleObject()", err, exit );

			//
			// check the return value of thread
			//
			require_noerr_with_log( log, "thread exit code", m_driverThreadExitCode, exit );

			//
			// now we know that the driver was successfully installed
			//
			printer->driverInstalled = true;
		}

		if ( service->type == kPDLServiceType )
		{
			err = InstallPrinterPort( printer, service, PROTOCOL_RAWTCP_TYPE, log );
			require_noerr_with_log( log, "InstallPrinterPort()", err, exit );
			err = InstallPrinterPDLAndLPR( printer, service, log );
			require_noerr_with_log( log, "InstallPrinterPDLAndLPR()", err, exit );
		}
		else if ( service->type == kLPRServiceType )
		{
			err = InstallPrinterPort( printer, service, PROTOCOL_LPR_TYPE, log );
			require_noerr_with_log( log, "InstallPrinterPort()", err, exit );
			err = InstallPrinterPDLAndLPR( printer, service, log );
			require_noerr_with_log( log, "InstallPrinterPDLAndLPR()", err, exit );
		}
		else if ( service->type == kIPPServiceType )
		{
			// There's no need to install a printer port for IPP printers, because
			// the call to AddPrinter() will do that for us.

			err = InstallPrinterIPP( printer, service, log );
			require_noerr_with_log( log, "InstallPrinterIPP()", err, exit );
		}
		else
		{
			require_action_with_log( log, ( service->type == kPDLServiceType ) || ( service->type == kLPRServiceType ) || ( service->type == kIPPServiceType ), exit, err = kUnknownErr );
		}
	}

	printer->installed = true;

	//
	// if the user specified a default printer, set it
	//
	if (printer->deflt)
	{
		ok = SetDefaultPrinter( printer->actualName );
		err = translate_errno( ok, errno_compat(), err = kUnknownErr );
		require_noerr_with_log( log, "SetDefaultPrinter()", err, exit );
	}

exit:

	return err;
}


OSStatus
CPrinterSetupWizardSheet::InstallPrinterPort( Printer * printer, Service * service, DWORD protocol, Logger & log )
{
	PRINTER_DEFAULTS	printerDefaults =	{ NULL,  NULL, SERVER_ACCESS_ADMINISTER };
	PORT_DATA_1			portData;
	DWORD				dwStatus;
	DWORD				cbInputData		=	100;
	PBYTE				pOutputData		=	NULL;
	DWORD				cbOutputNeeded	=	0;
	HANDLE				hXcv			=	NULL;
	Queue			*	q;
	BOOL				ok;
	OSStatus			err;

	ZeroMemory(&portData, sizeof(PORT_DATA_1));

	require_action_with_log( log, wcslen(printer->portName) < sizeof_array(portData.sztPortName), exit, err = kSizeErr );
	wcscpy_s(portData.sztPortName, printer->portName);

	q = service->queues.front();
	check( q );

	ok = OpenPrinter(L",XcvMonitor Standard TCP/IP Port", &hXcv, &printerDefaults);
	err = translate_errno( ok, errno_compat(), kUnknownErr );
	require_noerr_with_log( log, "OpenPrinter()", err, exit );

	//
	// BUGBUG: MSDN said this is not required, but my experience shows it is required
	//
	try
	{
		pOutputData = new BYTE[cbInputData];
	}
	catch (...)
	{
		pOutputData = NULL;
	}

	require_action_with_log( log, pOutputData, exit, err = kNoMemoryErr );
    	
	portData.dwPortNumber	=	service->portNumber;
	portData.dwVersion		=	1;
	portData.dwDoubleSpool	=	1;
    	
	portData.dwProtocol	= protocol;
	portData.cbSize		= sizeof PORT_DATA_1;
	portData.dwReserved	= 0L;

	require_action_with_log( log, wcslen(q->name) < sizeof_array(portData.sztQueue), exit, err = kSizeErr );
	wcscpy_s(portData.sztQueue, q->name);

	require_action_with_log( log, wcslen( service->hostname ) < sizeof_array(portData.sztHostAddress), exit, err = kSizeErr );
	wcscpy_s( portData.sztHostAddress, service->hostname );

	ok = XcvData(hXcv, L"AddPort", (PBYTE) &portData, sizeof(PORT_DATA_1), pOutputData, cbInputData,  &cbOutputNeeded, &dwStatus);
	err = translate_errno( ok, errno_compat(), kUnknownErr );
	require_noerr_with_log( log, "XcvData()", err, exit );

exit:

	if (hXcv != NULL)
	{
		ClosePrinter(hXcv);
	}

	if (pOutputData != NULL)
	{
		delete [] pOutputData;
	}

	return err;
}


OSStatus
CPrinterSetupWizardSheet::InstallPrinterPDLAndLPR(Printer * printer, Service * service, Logger & log )
{
	PRINTER_INFO_2		pInfo;
	HANDLE				hPrinter = NULL;
	Queue			*	q;
	OSStatus			err;

	check(printer != NULL);
	check(printer->installed == false);

	q = service->queues.front();
	check( q );

	//
	// add the printer
	//
	ZeroMemory(&pInfo, sizeof(pInfo));
		
	pInfo.pPrinterName			=	printer->actualName.GetBuffer();
	pInfo.pServerName			=	NULL;
	pInfo.pShareName			=	NULL;
	pInfo.pPortName				=	printer->portName.GetBuffer();
	pInfo.pDriverName			=	printer->modelName.GetBuffer();
	pInfo.pComment				=	printer->displayModelName.GetBuffer();
	pInfo.pLocation				=	q->location.GetBuffer();
	pInfo.pDevMode				=	NULL;
	pInfo.pDevMode				=	NULL;
	pInfo.pSepFile				=	L"";
	pInfo.pPrintProcessor		=	L"winprint";
	pInfo.pDatatype				=	L"RAW";
	pInfo.pParameters			=	L"";
	pInfo.pSecurityDescriptor	=	NULL;
	pInfo.Attributes			=	PRINTER_ATTRIBUTE_QUEUED;
	pInfo.Priority				=	0;
	pInfo.DefaultPriority		=	0;
	pInfo.StartTime				=	0;
	pInfo.UntilTime				=	0;

	hPrinter = AddPrinter(NULL, 2, (LPBYTE) &pInfo);
	err = translate_errno( hPrinter, errno_compat(), kUnknownErr );
	require_noerr_with_log( log, "AddPrinter()", err, exit );

exit:

	if (hPrinter != NULL)
	{
		ClosePrinter(hPrinter);
	}

	return err;
}


OSStatus
CPrinterSetupWizardSheet::InstallPrinterIPP(Printer * printer, Service * service, Logger & log)
{
	DEBUG_UNUSED( service );

	Queue		*	q		 = service->SelectedQueue();
	HANDLE			hPrinter = NULL;
	PRINTER_INFO_2	pInfo;
	OSStatus		err;

	check( q );
	
	//
	// add the printer
	//
	ZeroMemory(&pInfo, sizeof(PRINTER_INFO_2));
	
	pInfo.pPrinterName		= printer->actualName.GetBuffer();
	pInfo.pPortName			= printer->portName.GetBuffer();
	pInfo.pDriverName		= printer->modelName.GetBuffer();
	pInfo.pPrintProcessor	= L"winprint";
	pInfo.pLocation			= q->location.GetBuffer();
	pInfo.pComment			= printer->displayModelName.GetBuffer();
	pInfo.Attributes		= PRINTER_ATTRIBUTE_NETWORK | PRINTER_ATTRIBUTE_LOCAL;
	
	hPrinter = AddPrinter(NULL, 2, (LPBYTE)&pInfo);
	err = translate_errno( hPrinter, errno_compat(), kUnknownErr );
	require_noerr_with_log( log, "AddPrinter()", err, exit );

exit:

	if ( hPrinter != NULL )
	{
		ClosePrinter(hPrinter);
	}

	return err;
}


OSStatus
CPrinterSetupWizardSheet::InstallPrinterCUPS(Printer * printer, Service * service, CUPSLibrary & cupsLib )
{
	OSStatus err = kNoErr;

	check( printer );
	check( service );
	check( cupsLib.IsInstalled() );

	err = InstallPrinterCUPS( printer, service, cupsLib, TEXT( "Windows NT x86" ) );
	require_noerr( err, exit );

	if ( Is64BitWindows() )
	{
		err = InstallPrinterCUPS( printer, service, cupsLib, TEXT( "Windows x64" ) );
		require_noerr( err, exit );
	}

exit:

	return err;
}


OSStatus
CPrinterSetupWizardSheet::InstallPrinterCUPS(Printer * printer, Service * service, CUPSLibrary & cupsLib, TCHAR * env )
{
	
	Queue		*	q;
	CString			ppdfile;				// PPD file for printer drivers
	TCHAR			driverdir[1024];		// Directory for driver files
	DWORD			needed;					// Bytes needed
	DRIVER_INFO_3	driverinfo;				// Driver information
	PRINTER_INFO_2	printerinfo;			// Printer information
	HANDLE			printerHandle = NULL;	// Handle to printer
	CString			filename;				// Driver filename
	CString			dependentFiles;			// List of dependent files
	CString			portName;				// Port Name
	int				bytes;					// Bytes copied
	TCHAR			datadir[ MAX_PATH ];	// Driver files location
	CFile			in;						// Input file
	CFile			out;					// Output file
	void		*	http;					// Connection to server
	char			buffer[4096];			// Copy/error buffer
	CString			platform;
	char			hostname[ 1024 ];
	CString			dest;
	char			destANSI[ 1024 ];
	int				i;
	DWORD			num;
	OSStatus		err	= 0;
	BOOL			ok;

	check( printer );
	check( service );
	check( cupsLib.IsInstalled() );
	check( env );

	// What do we do here for multiple queues?
	q = service->queues.front();
	require_action( q != NULL, exit, err = kUnknownErr );

	num = GetModuleFileName( NULL, datadir, MAX_PATH );
	err = translate_errno( num > 0, GetLastError(), kUnknownErr );
	require_noerr( err, exit );
	ok = PathRemoveFileSpec( datadir );
	require_action( ok, exit, err = kUnknownErr );

	ok = GetPrinterDriverDirectory(NULL, env, 1, ( LPBYTE ) driverdir, sizeof( driverdir ), &needed );
	err = translate_errno( ok, GetLastError(), kUnknownErr );
	require_noerr( err, exit );

	platform = env;
	platform = platform.Right( 3 );

	// Append the supported banner pages to the PPD file...
	err = StringObjectToUTF8String( service->hostname, hostname, sizeof( hostname ) );
	require_noerr( err, exit );
	http = cupsLib.httpConnectEncrypt( hostname, service->portNumber, cupsLib.cupsEncryption() );
	err = translate_errno( http != NULL, errno, kUnknownErr );
	require_noerr( err, exit );

	if ( ( service->portNumber == 443 ) || ( cupsLib.cupsEncryption() >= HTTP_ENCRYPT_REQUIRED ) )
	{
		// This forces the use the https: URLs below...
		cupsLib.cupsSetEncryption( HTTP_ENCRYPT_ALWAYS );
	}

	// Strip the leading "printers/" or "classes/" from the beginning
	// of the name

	dest = q->name;
	dest.Replace( TEXT( "printers/" ), TEXT( "" ) );
	dest.Replace( TEXT( "classes/" ), TEXT( "" ) );

	err = StringObjectToUTF8String( dest, destANSI, sizeof( destANSI ) );
	require_noerr( err, exit );

	// Get the PPD file...
	for ( i = 0; i < 10; i++ )
	{
		char ppdfileANSI[ 1024 ];

		if ( cupsLib.cupsAdminCreateWindowsPPD( http, destANSI, ppdfileANSI, sizeof( ppdfileANSI ) ) )
		{
			err = UTF8StringToStringObject( ppdfileANSI, ppdfile );
			require_noerr( err, exit );
			break;
		}
	}

	err = translate_errno( i < 10, errno, kUnknownErr );
	require_noerr( err, exit );

	// Copy the PPD file to the Windows driver directory...
	filename.Format( TEXT( "%s/%s.ppd" ), driverdir, dest );

	ok = in.Open( ppdfile, CFile::modeRead | CFile::typeBinary );
	translate_errno( ok, GetLastError(), kUnknownErr );
	require_noerr( err, exit );

	ok = out.Open( filename, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary );
	translate_errno( ok, GetLastError(), kUnknownErr );
	require_noerr( err, exit );

	while ( ( bytes = in.Read( buffer, sizeof(buffer) ) ) > 0 )
	{
		out.Write(buffer, bytes );
	}

	in.Close();
	out.Close();

	// Cleanup temp file...
	CFile::Remove( ppdfile );

	// Copy the driver files to the driver directory...
	for ( i = 0; i < ( sizeof( g_printerDriverFiles ) / sizeof( g_printerDriverFiles[0] ) ); i++ )
	{
		filename.Format( TEXT( "%s/drivers/%s/%s" ), datadir, platform, g_printerDriverFiles[i]);
	
		ok = in.Open(filename, CFile::modeRead | CFile::typeBinary );
		err = translate_errno( ok, GetLastError(), kUnknownErr );
		require_noerr( err, exit );

		filename.Format( TEXT( "%s/%s" ), driverdir, g_printerDriverFiles[i] );
		ok = out.Open(filename, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary );
		err = translate_errno( ok, errno, kUnknownErr );

		while ( ( bytes = in.Read(buffer, sizeof( buffer ) ) ) > 0 )
		{
			out.Write( buffer, bytes );
		}

		in.Close();
		out.Close();
	}

	// Do the Windows system calls needed to add the printer driver...
	filename.Format( TEXT( "%s.ppd" ), dest);
	dependentFiles.Format( TEXT( "pscript5.dll%c" ) TEXT( "%s.ppd%c" ) TEXT( "ps5ui.dll%c" ) TEXT( "pscript.hlp%c" ) TEXT( "pscript.ntf%c" ) TEXT( "cups6.ini%c" ) TEXT( "cupsps6.dll%c" ) TEXT( "cupsui6.dll%c" ), 0, dest, 0, 0, 0, 0, 0, 0, 0);

	driverinfo.cVersion         = 3;
	driverinfo.pName            = printer->actualName.GetBuffer();
	driverinfo.pEnvironment     = env;
	driverinfo.pDriverPath      = TEXT( "pscript5.dll" );
	driverinfo.pDataFile        = filename.GetBuffer();
	driverinfo.pConfigFile      = TEXT( "ps5ui.dll" );
	driverinfo.pHelpFile        = TEXT( "pscript.hlp" );
	driverinfo.pDependentFiles  = dependentFiles.GetBuffer();
	driverinfo.pMonitorName     = NULL;
	driverinfo.pDefaultDataType = TEXT( "raw" );

	ok = AddPrinterDriverEx(NULL, 3, (LPBYTE) &driverinfo, APD_COPY_ALL_FILES );
	err = translate_errno( ok, GetLastError(), kUnknownErr );
	require_noerr( err, exit );

	// See if the printer has already been added?
	if ( OpenPrinter( printer->actualName.GetBuffer(), &printerHandle, NULL ) )
    {
		// Printer already exists, so we are done now...
		goto exit;
    }

    // Add the printer using the HTTP/IPP port...
	portName.Format( TEXT( "%s://%s:%d/printers/%s" ), cupsLib.cupsEncryption() == HTTP_ENCRYPT_ALWAYS ? TEXT( "https" ) : TEXT( "http" ), service->hostname.GetBuffer(), service->portNumber, dest );

    memset(&printerinfo, 0, sizeof(printerinfo));
    printerinfo.pPrinterName	= printer->actualName.GetBuffer();
    printerinfo.pPortName		= portName.GetBuffer();
    printerinfo.pDriverName		= printer->actualName.GetBuffer();
    printerinfo.Attributes		= PRINTER_ATTRIBUTE_NETWORK | PRINTER_ATTRIBUTE_LOCAL;
	printerinfo.pComment		= q->description.GetBuffer();
	printerinfo.pLocation		= q->location.GetBuffer();
	printerinfo.pPrintProcessor = TEXT( "winprint" );

    printerHandle = AddPrinter( NULL, 2, (LPBYTE) &printerinfo );
	err = translate_errno( printerHandle, GetLastError(), kUnknownErr );
	require_noerr( err, exit );

exit:

	if ( printerHandle != NULL )
	{
		ClosePrinter( printerHandle );
		printerHandle = NULL;
	}

	return err;
}

BEGIN_MESSAGE_MAP(CPrinterSetupWizardSheet, CPropertySheet)
ON_MESSAGE( WM_SOCKET_EVENT, OnSocketEvent )
ON_MESSAGE( WM_PROCESS_EVENT, OnProcessEvent )
ON_WM_SETCURSOR()
ON_WM_TIMER()
END_MESSAGE_MAP()


// ------------------------------------------------------
// OnCommand
//
// Traps when the user hits Finish  
//	
BOOL CPrinterSetupWizardSheet::OnCommand(WPARAM wParam, LPARAM lParam)
{
	//
	// Check if this is OK
	//
	if (wParam == ID_WIZFINISH)              // If OK is hit...
	{
		OnOK();
	}
 
	return CPropertySheet::OnCommand(wParam, lParam);
}


// ------------------------------------------------------
// OnInitDialog
//
// Initializes this Dialog object.
//	
BOOL CPrinterSetupWizardSheet::OnInitDialog()
{
	OSStatus err;

	CPropertySheet::OnInitDialog();

	err = StartBrowse();
	require_noerr( err, exit );

exit:

	if ( err )
	{
		StopBrowse();

		if ( err == kDNSServiceErr_Firewall )
		{
			CString text, caption;

			text.LoadString( IDS_FIREWALL );
			caption.LoadString( IDS_FIREWALL_CAPTION );

			MessageBox(text, caption, MB_OK|MB_ICONEXCLAMATION);
		}
		else
		{
			CString text, caption;

			text.LoadString( IDS_NO_MDNSRESPONDER_SERVICE_TEXT );
			caption.LoadString( IDS_ERROR_CAPTION );

			MessageBox(text, caption, MB_OK|MB_ICONEXCLAMATION);

			_exit( 0 );
		}
	}

	return TRUE;
}


// ------------------------------------------------------
// OnSetCursor
//
// This is called when Windows wants to know what cursor
// to display.  So we tell it.  
//	
BOOL
CPrinterSetupWizardSheet::OnSetCursor(CWnd * pWnd, UINT nHitTest, UINT message)
{
	DEBUG_UNUSED(pWnd);
	DEBUG_UNUSED(nHitTest);
	DEBUG_UNUSED(message);

	SetCursor(m_active);
	return TRUE;
}


// ------------------------------------------------------
// OnContextMenu
//
// This is not fully implemented yet.  
//	

void
CPrinterSetupWizardSheet::OnContextMenu(CWnd * pWnd, CPoint pos)
{
	DEBUG_UNUSED(pWnd);
	DEBUG_UNUSED(pos);

	CAbout dlg;

	dlg.DoModal();
}


// ------------------------------------------------------
// OnOK
//
// This is called when the user hits the "Finish" button  
//	
void
CPrinterSetupWizardSheet::OnOK()
{
	CWnd * window;
	OSStatus err;

	check ( m_selectedPrinter != NULL );

	SetWizardButtons( PSWIZB_DISABLEDFINISH );

	window = GetDlgItem( IDCANCEL );
 
	if ( window )
	{
		window->EnableWindow( FALSE );
	}

	m_pgFourth.StartActivityIndicator();
	
	err = InstallPrinter( m_selectedPrinter );

	m_pgFourth.StopActivityIndicator();

	if ( err != kNoErr )
	{
		CString caption;
		CString message;

		caption.LoadString(IDS_INSTALL_ERROR_CAPTION);
		caption.AppendFormat( TEXT( " (%d)" ), err );
		message.LoadString(IDS_INSTALL_ERROR_MESSAGE);
		MessageBox(message, caption, MB_OK|MB_ICONEXCLAMATION);
	}

	StopBrowse();
}


// CPrinterSetupWizardSheet message handlers

void CPrinterSetupWizardSheet::Init(void)
{
	AddPage(&m_pgSecond);
	AddPage(&m_pgThird);
	AddPage(&m_pgFourth);

	m_psh.dwFlags &= (~PSH_HASHELP);

	m_psh.dwFlags |= PSH_WIZARD97|PSH_WATERMARK|PSH_HEADER;
	m_psh.pszbmWatermark = MAKEINTRESOURCE(IDB_WATERMARK);
	m_psh.pszbmHeader = MAKEINTRESOURCE(IDB_BANNER_ICON);

	m_psh.hInstance = GetNonLocalizedResources();

	SetWizardMode();
}


LRESULT
CPrinterSetupWizardSheet::OnSocketEvent(WPARAM inWParam, LPARAM inLParam)
{
	if (WSAGETSELECTERROR(inLParam) && !(HIWORD(inLParam)))
    {
		dlog( kDebugLevelError, "OnServiceEvent: window error\n" );
    }
    else
    {
		SOCKET sock = (SOCKET) inWParam;

		// iterate thru list
		ServiceRefList::iterator begin = m_serviceRefList.begin();
		ServiceRefList::iterator end   = m_serviceRefList.end();

		while (begin != end)
		{
			DNSServiceRef ref = *begin++;

			check(ref != NULL);

			if ((SOCKET) DNSServiceRefSockFD(ref) == sock)
			{
				DNSServiceProcessResult(ref);
				break;
			}
		}
	}

	return ( 0 );
}


LRESULT
CPrinterSetupWizardSheet::OnProcessEvent(WPARAM inWParam, LPARAM inLParam)
{
	DEBUG_UNUSED(inLParam);

	m_driverThreadExitCode	=	(DWORD) inWParam;
	m_driverThreadFinished	=	true;

	return 0;
}


unsigned WINAPI
CPrinterSetupWizardSheet::InstallDriverThread( LPVOID inParam )
{	
	Printer			*	printer = (Printer*) inParam;
	DWORD				exitCode = 0;
	DWORD				dwResult;
	OSStatus			err;
	STARTUPINFO			si;
	PROCESS_INFORMATION pi;
	BOOL				ok;

	check( printer );
	check( m_self );

	//
	// because we're calling endthreadex(), C++ objects won't be cleaned up
	// correctly.  we'll nest the CString 'command' inside a block so
	// that it's destructor will be invoked.
	//
	{
		CString command;

		ZeroMemory( &si, sizeof(si) );
		si.cb = sizeof(si);
		ZeroMemory( &pi, sizeof(pi) );

		command.Format(L"rundll32.exe printui.dll,PrintUIEntry /ia /m \"%s\" /f \"%s\"", (LPCTSTR) printer->modelName, (LPCTSTR) printer->infFileName );

		ok = CreateProcess(NULL, command.GetBuffer(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
		err = translate_errno( ok, errno_compat(), kUnknownErr );
		require_noerr( err, exit );

		dwResult = WaitForSingleObject( pi.hProcess, INFINITE );
		translate_errno( dwResult == WAIT_OBJECT_0, errno_compat(), err = kUnknownErr );
		require_noerr( err, exit );

		ok = GetExitCodeProcess( pi.hProcess, &exitCode );
		err = translate_errno( ok, errno_compat(), kUnknownErr );
		require_noerr( err, exit );
	}

exit:

	//
	// Close process and thread handles. 
	//
	if ( pi.hProcess )
	{
		CloseHandle( pi.hProcess );
	}

	if ( pi.hThread )
	{
		CloseHandle( pi.hThread );
	}

	//
	// alert the main thread
	//
	m_self->PostMessage( WM_PROCESS_EVENT, err, exitCode );

	_endthreadex_compat( 0 );

	return 0;
}


void DNSSD_API
CPrinterSetupWizardSheet::OnBrowse(
							DNSServiceRef 			inRef,
							DNSServiceFlags 		inFlags,
							uint32_t 				inInterfaceIndex,
							DNSServiceErrorType 	inErrorCode,
							const char *			inName,	
							const char *			inType,	
							const char *			inDomain,	
							void *					inContext )
{
	DEBUG_UNUSED(inRef);

	CPrinterSetupWizardSheet	*	self;
	bool							moreComing = (bool) (inFlags & kDNSServiceFlagsMoreComing);
	CPropertyPage				*	active;
	Printer						*	printer = NULL;
	Service						*	service = NULL;
	OSStatus						err = kNoErr;

	require_noerr( inErrorCode, exit );

	self = reinterpret_cast <CPrinterSetupWizardSheet*>( inContext );
	require_quiet( self, exit );

	active = self->GetActivePage();
	require_quiet( active, exit );

	// Have we seen this printer before?

	printer = self->Lookup( inName );

	if ( printer )
	{
		service = printer->LookupService( inType );
	}

	if ( inFlags & kDNSServiceFlagsAdd )
	{
		BOOL newPrinter = FALSE;

		if ( !printer )
		{
			printer = self->OnAddPrinter( inInterfaceIndex, inName, inType, inDomain, moreComing );
			require_action( printer, exit, err = kUnknownErr );
			newPrinter = TRUE;
		}
		
		// If we're looking at the browse list on page 2, then we need to call
		// CPage2::OnAddPrinter() regardless of whether we've seen the printer
		// or not because the moreComing flag might have changed from a previous
		// call. If we only call CPage2::OnAddPrinter() when there's a new printer,
		// we might not correctly update our UI, so if we've seen the printer before,
		// call OnAddPrinter with a NULL parameter.

		if ( self->GetActivePage() == &self->m_pgSecond )
		{
			self->m_pgSecond.OnAddPrinter( newPrinter ? printer : NULL, moreComing );
		}

		if ( !service )
		{
			err = self->OnAddService( printer, inInterfaceIndex, inName, inType, inDomain );
			require_noerr( err, exit );
		}
		else
		{
			service->refs++;
		}
	}
	else if ( printer )
	{
		check( service );

		err = self->OnRemoveService( service );
		require_noerr( err, exit );

		if ( printer->services.size() == 0 )
		{
			err = self->OnRemovePrinter( printer, moreComing );
			require_noerr( err, exit );
		}
	}

exit:
	
	return;
}


void DNSSD_API
CPrinterSetupWizardSheet::OnResolve(
								DNSServiceRef			inRef,
								DNSServiceFlags			inFlags,
								uint32_t				inInterfaceIndex,
								DNSServiceErrorType		inErrorCode,
								const char *			inFullName,	
								const char *			inHostName, 
								uint16_t 				inPort,
								uint16_t 				inTXTSize,
								const char *			inTXT,
								void *					inContext )
{
	DEBUG_UNUSED(inFullName);
	DEBUG_UNUSED(inInterfaceIndex);
	DEBUG_UNUSED(inFlags);
	DEBUG_UNUSED(inRef);

	CPrinterSetupWizardSheet	*	self;
	Service						*	service;
	Queue						*	q;
	int								idx;
	OSStatus						err;

	require_noerr( inErrorCode, exit );

	service = reinterpret_cast<Service*>( inContext );
	require_quiet( service, exit);

	check( service->refs != 0 );

	self = service->printer->window;
	require_quiet( self, exit );

	err = self->StopOperation( service->serviceRef );
	require_noerr( err, exit );
	
	//
	// hold on to the hostname...
	//
	err = UTF8StringToStringObject( inHostName, service->hostname );
	require_noerr( err, exit );

	//
	// <rdar://problem/3739200> remove the trailing dot on hostname
	//
	idx = service->hostname.ReverseFind('.');

	if ((idx > 1) && ((service->hostname.GetLength() - 1) == idx))
	{
		service->hostname.Delete(idx, 1);
	}

	//
	// hold on to the port
	//
	service->portNumber = ntohs(inPort);

	if ( service->qtotal == 1 )
	{	
		//
		// create a new queue
		//
		try
		{
			q = new Queue;
		}
		catch (...)
		{
			q = NULL;
		}

		require_action( q, exit, err = E_OUTOFMEMORY );

		//
		// parse the text record.
		//

		err = self->ParseTextRecord( service, q, inTXTSize, inTXT );
		require_noerr( err, exit );

		service->queues.push_back( q );

		//
		// we've completely resolved this service
		//

		self->OnResolveService( service );
	}
	else
	{
		//
		// if qtotal is more than 1, then we need to get additional
		// text records.  if not, then this service is considered
		// resolved
		//

		err = DNSServiceQueryRecord(&service->serviceRef, 0, inInterfaceIndex, inFullName, kDNSServiceType_TXT, kDNSServiceClass_IN, OnQuery, (void*) service );
		require_noerr( err, exit );

		err = self->StartOperation( service->serviceRef );
		require_noerr( err, exit );
	}

exit:

	return;
}


void DNSSD_API
CPrinterSetupWizardSheet::OnQuery(
							DNSServiceRef		inRef, 
							DNSServiceFlags		inFlags, 
							uint32_t			inInterfaceIndex, 
							DNSServiceErrorType inErrorCode,
							const char		*	inFullName, 
							uint16_t			inRRType, 
							uint16_t			inRRClass, 
							uint16_t			inRDLen, 
							const void		*	inRData, 
							uint32_t			inTTL, 
							void			*	inContext)
{
	DEBUG_UNUSED( inTTL );
	DEBUG_UNUSED( inRRClass );
	DEBUG_UNUSED( inRRType );
	DEBUG_UNUSED( inFullName );
	DEBUG_UNUSED( inInterfaceIndex );
	DEBUG_UNUSED( inRef );

	Service						*	service = NULL;
	Queue						*	q;
	CPrinterSetupWizardSheet	*	self;
	OSStatus						err = kNoErr;

	require_noerr( inErrorCode, exit );

	service = reinterpret_cast<Service*>( inContext );
	require_quiet( service, exit);

	self = service->printer->window;
	require_quiet( self, exit );

	if ( ( inFlags & kDNSServiceFlagsAdd ) && ( inRDLen > 0 ) && ( inRData != NULL ) )
	{
		const char * inTXT = ( const char * ) inRData;

		//
		// create a new queue
		//
		try
		{
			q = new Queue;
		}
		catch (...)
		{
			q = NULL;
		}

		require_action( q, exit, err = E_OUTOFMEMORY );

		err = service->printer->window->ParseTextRecord( service, q, inRDLen, inTXT );
		require_noerr( err, exit );

		//
		// add this queue
		//

		service->queues.push_back( q );

		if ( service->queues.size() == service->qtotal )
		{
			//
			// else if moreComing is not set, then we're going
			// to assume that we're done
			//

			self->StopOperation( service->serviceRef );

			//
			// sort the queues
			//

			service->queues.sort( OrderQueueFunc );

			//
			// we've completely resolved this service
			//

			self->OnResolveService( service );
		}
	}

exit:

	if ( err && service && ( service->serviceRef != NULL ) )
	{
		service->printer->window->StopOperation( service->serviceRef );
	}

	return;
}


Printer*
CPrinterSetupWizardSheet::OnAddPrinter(
								uint32_t 		inInterfaceIndex,
								const char *	inName,	
								const char *	inType,	
								const char *	inDomain,
								bool			moreComing)
{
	Printer	*	printer = NULL;
	DWORD		printerNameCount;
	OSStatus	err;

	DEBUG_UNUSED( inInterfaceIndex );
	DEBUG_UNUSED( inType );
	DEBUG_UNUSED( inDomain );
	DEBUG_UNUSED( moreComing );

	try
	{
		printer = new Printer;
	}
	catch (...)
	{
		printer = NULL;
	}

	require_action( printer, exit, err = E_OUTOFMEMORY );

	printer->window		=	this;
	printer->name		=	inName;
	
	err = UTF8StringToStringObject(inName, printer->displayName);
	check_noerr( err );
	printer->actualName	=	printer->displayName;
	printer->installed	=	false;
	printer->deflt		=	false;
	printer->resolving	=	0;

	// Compare this name against printers that are already installed
	// to avoid name clashes.  Rename as necessary
	// to come up with a unique name.

	printerNameCount = 2;

	for (;;)
	{
		CPrinterSetupWizardSheet::PrinterNames::iterator it;

		// <rdar://problem/4141221> Don't use find to do comparisons because we need to
		// do a case insensitive string comparison

		for ( it = m_printerNames.begin(); it != m_printerNames.end(); it++ )
		{
			if ( (*it).CompareNoCase( printer->actualName ) == 0 )
			{
				break;
			}
		}

		if (it != m_printerNames.end())
		{
			printer->actualName.Format(L"%s (%d)", printer->displayName, printerNameCount);
		}
		else
		{
			break;
		}

		printerNameCount++;
	}

	m_printers.push_back( printer );

exit:

	return printer;
}


OSStatus
CPrinterSetupWizardSheet::OnAddService(
								Printer		*	printer,
								uint32_t 		inInterfaceIndex,
								const char	*	inName,	
								const char	*	inType,	
								const char	*	inDomain)
{
	Service	*	service = NULL;
	OSStatus	err     = kNoErr;

	DEBUG_UNUSED( inName );
	DEBUG_UNUSED( inDomain );

	try
	{
		service = new Service;
	}
	catch (...)
	{
		service = NULL;
	}

	require_action( service, exit, err = E_OUTOFMEMORY );
	
	service->printer	=	printer;
	service->ifi		=	inInterfaceIndex;
	service->type		=	inType;
	service->domain		=	inDomain;
	service->qtotal		=	1;
	service->refs		=	1;
	service->serviceRef	=	NULL;

	printer->services.push_back( service );

	//
	// if the printer is selected, then we'll want to start a
	// resolve on this guy
	//

	if ( printer == m_selectedPrinter )
	{
		StartResolve( service );
	}

exit:

	return err;
}


OSStatus
CPrinterSetupWizardSheet::OnRemovePrinter( Printer * printer, bool moreComing )
{
	CPropertyPage	*	active	= GetActivePage();
	OSStatus			err		= kNoErr;

	if ( active == &m_pgSecond )
	{
		m_pgSecond.OnRemovePrinter( printer, moreComing );
	}

	m_printers.remove( printer );

	if ( m_selectedPrinter == printer )
	{
		m_selectedPrinter = NULL;

		if ( ( active == &m_pgThird ) || ( active == &m_pgFourth ) )
		{
			CString caption;
			CString message;

			caption.LoadString( IDS_ERROR_CAPTION );
			message.LoadString( IDS_PRINTER_UNAVAILABLE );

			MessageBox(message, caption, MB_OK|MB_ICONEXCLAMATION);

			SetActivePage( &m_pgSecond );
		}
	}

	delete printer;

	return err;
}


OSStatus
CPrinterSetupWizardSheet::OnRemoveService( Service * service )
{
	OSStatus err = kNoErr;

	if ( service && ( --service->refs == 0 ) )
	{
		if ( service->serviceRef != NULL )
		{
			err = StopResolve( service );
			require_noerr( err, exit );
		}

		service->printer->services.remove( service );

		delete service;
	}

exit:

	return err;	
}


void
CPrinterSetupWizardSheet::OnResolveService( Service * service )
{
	// Make sure that the active page is page 2

	require_quiet( GetActivePage() == &m_pgSecond, exit );

	if ( !--service->printer->resolving )
	{
		// sort the services now.  we want the service that
		// has the highest priority queue to be first in
		// the list.

		service->printer->services.sort( OrderServiceFunc );

		// Now we can hit next

		SetWizardButtons( PSWIZB_BACK|PSWIZB_NEXT );
	
		// Reset the cursor	
		
		m_active = m_arrow;

		// And tell page 2 about it

		m_pgSecond.OnResolveService( service );
	}		

exit:

	return;
}


OSStatus
CPrinterSetupWizardSheet::StartBrowse()
{
	OSStatus err;

	//
	// setup the DNS-SD browsing
	//
	err = DNSServiceBrowse( &m_pdlBrowser, 0, 0, kPDLServiceType, NULL, OnBrowse, this );
	require_noerr( err, exit );

	err = StartOperation( m_pdlBrowser );
	require_noerr( err, exit );

	err = DNSServiceBrowse( &m_lprBrowser, 0, 0, kLPRServiceType, NULL, OnBrowse, this );
	require_noerr( err, exit );

	err = StartOperation( m_lprBrowser );
	require_noerr( err, exit );

	err = DNSServiceBrowse( &m_ippBrowser, 0, 0, kIPPServiceType, NULL, OnBrowse, this );
	require_noerr( err, exit );

	err = StartOperation( m_ippBrowser );
	require_noerr( err, exit );

exit:

	return err;
}


OSStatus
CPrinterSetupWizardSheet::StopBrowse()
{
	OSStatus err;

	err = StopOperation( m_pdlBrowser );
	require_noerr( err, exit );

	err = StopOperation( m_lprBrowser );
	require_noerr( err, exit );

	err = StopOperation( m_ippBrowser );
	require_noerr( err, exit );

	while ( m_printers.size() > 0 )
	{
		Printer * printer = m_printers.front();

		m_printers.pop_front();

		if ( printer->resolving )
		{
			StopResolve( printer );
		}

		delete printer;
	}

exit:

	return err;
}


OSStatus
CPrinterSetupWizardSheet::StartResolve( Printer * printer )
{
	OSStatus			err = kNoErr;
	Services::iterator	it;

	check( printer );

	for ( it = printer->services.begin(); it != printer->services.end(); it++ )
	{
		if ( (*it)->serviceRef == NULL )
		{
			err = StartResolve( *it );
			require_noerr( err, exit );
		}
	}

	m_selectedPrinter = printer;

exit:

	return err;
}


OSStatus
CPrinterSetupWizardSheet::StartResolve( Service * service )
{
	OSStatus err = kNoErr;

	check( service->serviceRef == NULL );

	//
	// clean out any queues that were collected during a previous
	// resolve
	//

	service->EmptyQueues();

	//
	// now start the new resolve
	//

	err = DNSServiceResolve( &service->serviceRef, 0, 0, service->printer->name.c_str(), service->type.c_str(), service->domain.c_str(), (DNSServiceResolveReply) OnResolve, service );
	require_noerr( err, exit );

	err = StartOperation( service->serviceRef );
	require_noerr( err, exit );

	//
	// If we're not currently resolving, then disable the next button
	// and set the cursor to hourglass
	//

	if ( !service->printer->resolving )
	{
		SetWizardButtons( PSWIZB_BACK );

		m_active = m_wait;
		SetCursor(m_active);
	}

	service->printer->resolving++;

exit:

	return err;
}


OSStatus
CPrinterSetupWizardSheet::StopResolve(Printer * printer)
{
	OSStatus err = kNoErr;

	check( printer );

	Services::iterator it;

	for ( it = printer->services.begin(); it != printer->services.end(); it++ )
	{
		if ( (*it)->serviceRef )
		{
			err = StopResolve( *it );
			require_noerr( err, exit );
		}
	}

exit:

	return err;
}


OSStatus
CPrinterSetupWizardSheet::StopResolve( Service * service )
{
	OSStatus err;

	check( service->serviceRef );

	err = StopOperation( service->serviceRef );
	require_noerr( err, exit );

	service->printer->resolving--;

exit:

	return err;
}


OSStatus
CPrinterSetupWizardSheet::StartOperation( DNSServiceRef ref )
{
	OSStatus err;

	err = WSAAsyncSelect((SOCKET) DNSServiceRefSockFD(ref), m_hWnd, WM_SOCKET_EVENT, FD_READ|FD_CLOSE);
	require_noerr( err, exit );

	m_serviceRefList.push_back( ref );

exit:

	return err;
}


OSStatus
CPrinterSetupWizardSheet::StopOperation( DNSServiceRef & ref )
{
	OSStatus err = kNoErr;

	if ( ref )
	{
		m_serviceRefList.remove( ref );

		if ( IsWindow( m_hWnd ) )
		{
			err = WSAAsyncSelect((SOCKET) DNSServiceRefSockFD( ref ), m_hWnd, 0, 0 );
			require_noerr( err, exit );
		}

		DNSServiceRefDeallocate( ref );
		ref = NULL;
	}

exit:

	return err;
}


OSStatus
CPrinterSetupWizardSheet::ParseTextRecord( Service * service, Queue * q, uint16_t inTXTSize, const char * inTXT )
{
	check( service );
	check( q );

	// <rdar://problem/3946587> Use TXTRecord APIs declared in dns_sd.h
	
	bool			qtotalDefined	= false;
	const void	*	val;
	char			buf[256];
	uint8_t			len;
	OSStatus		err				= kNoErr;

	// <rdar://problem/3987680> Default to queue "lp"

	q->name = L"lp";

	// <rdar://problem/4003710> Default pdl key to be "application/postscript"

	q->pdl = L"application/postscript";

	if ( ( val = TXTRecordGetValuePtr( inTXTSize, inTXT, "rp", &len ) ) != NULL )
	{
		// Stringize val ( doesn't have trailing '\0' yet )

		memcpy( buf, val, len );
		buf[len] = '\0';

		err = UTF8StringToStringObject( buf, q->name );
		require_noerr( err, exit );
	}
	
	if ( ( val = TXTRecordGetValuePtr( inTXTSize, inTXT, "pdl", &len ) ) != NULL )
	{
		// Stringize val ( doesn't have trailing '\0' yet )

		memcpy( buf, val, len );
		buf[len] = '\0';

		err = UTF8StringToStringObject( buf, q->pdl );
		require_noerr( err, exit );
	}
	
	if ( ( ( val = TXTRecordGetValuePtr( inTXTSize, inTXT, "usb_mfg", &len ) ) != NULL ) ||
	     ( ( val = TXTRecordGetValuePtr( inTXTSize, inTXT, "usb_manufacturer", &len ) ) != NULL ) )
	{
		// Stringize val ( doesn't have trailing '\0' yet )

		memcpy( buf, val, len );
		buf[len] = '\0';

		err = UTF8StringToStringObject( buf, q->usb_MFG );
		require_noerr( err, exit );
	}
	
	if ( ( ( val = TXTRecordGetValuePtr( inTXTSize, inTXT, "usb_mdl", &len ) ) != NULL ) ||
	     ( ( val = TXTRecordGetValuePtr( inTXTSize, inTXT, "usb_model", &len ) ) != NULL ) )
	{
		// Stringize val ( doesn't have trailing '\0' yet )

		memcpy( buf, val, len );
		buf[len] = '\0';

		err = UTF8StringToStringObject( buf, q->usb_MDL );
		require_noerr( err, exit );
	}

	if ( ( val = TXTRecordGetValuePtr( inTXTSize, inTXT, "ty", &len ) ) != NULL )
	{
		// Stringize val ( doesn't have trailing '\0' yet )

		memcpy( buf, val, len );
		buf[len] = '\0';

		err = UTF8StringToStringObject( buf, q->description );
		require_noerr( err, exit );
	}
		
	if ( ( val = TXTRecordGetValuePtr( inTXTSize, inTXT, "product", &len ) ) != NULL )
	{
		// Stringize val ( doesn't have trailing '\0' yet )

		memcpy( buf, val, len );
		buf[len] = '\0';

		err = UTF8StringToStringObject( buf, q->product );
		require_noerr( err, exit );
	}

	if ( ( val = TXTRecordGetValuePtr( inTXTSize, inTXT, "note", &len ) ) != NULL )
	{
		// Stringize val ( doesn't have trailing '\0' yet )

		memcpy( buf, val, len );
		buf[len] = '\0';

		err = UTF8StringToStringObject( buf, q->location );
		require_noerr( err, exit );
	}

	if ( ( val = TXTRecordGetValuePtr( inTXTSize, inTXT, "qtotal", &len ) ) != NULL )
	{
		// Stringize val ( doesn't have trailing '\0' yet )

		memcpy( buf, val, len );
		buf[len] = '\0';

		service->qtotal = (unsigned short) atoi( buf );
		qtotalDefined = true;
	}

	if ( ( val = TXTRecordGetValuePtr( inTXTSize, inTXT, "priority", &len ) ) != NULL )
	{
		// Stringize val ( doesn't have trailing '\0' yet )

		memcpy( buf, val, len );
		buf[len] = '\0';

		q->priority = atoi( buf );
	}

	// <rdar://problem/4124524> Was this printer discovered via OS X Printer Sharing?

	if ( TXTRecordContainsKey( inTXTSize, inTXT, "printer-state" ) || TXTRecordContainsKey( inTXTSize, inTXT, "printer-type" ) )
	{
		service->printer->isCUPSPrinter = true;
	}

exit:

	// The following code is to fix a problem with older HP 
	// printers that don't include "qtotal" in their text
	// record.  We'll check to see if the q->name is "TEXT"
	// and if so, we're going to modify it to be "lp" so
	// that we don't use the wrong queue

	if ( !err && !qtotalDefined && ( q->name == L"TEXT" ) )
	{
		q->name = "lp";
	}

	return err;
}


Printer*
CPrinterSetupWizardSheet::Lookup(const char * inName)
{
	check( inName );

	Printer			*	printer = NULL;
	Printers::iterator	it;

	for ( it = m_printers.begin(); it != m_printers.end(); it++ )
	{
		if ( (*it)->name == inName )
		{
			printer = *it;
			break;
		}
	}

	return printer;
}


bool
CPrinterSetupWizardSheet::OrderServiceFunc( const Service * a, const Service * b )
{
	Queue * q1, * q2;

	q1 = (a->queues.size() > 0) ? a->queues.front() : NULL;

	q2 = (b->queues.size() > 0) ? b->queues.front() : NULL;

	if ( !q1 && !q2 )
	{
		return true;
	}
	else if ( q1 && !q2 )
	{
		return true;
	}
	else if ( !q1 && q2 )
	{
		return false;
	}
	else if ( q1->priority < q2->priority )
	{
		return true;
	}
	else if ( q1->priority > q2->priority )
	{
		return false;
	}
	else if ( ( a->type == kPDLServiceType ) || ( ( a->type == kLPRServiceType ) && ( b->type == kIPPServiceType ) ) )
	{
		return true;
	}
	else
	{
		return false;
	}
}


bool
CPrinterSetupWizardSheet::OrderQueueFunc( const Queue * q1, const Queue * q2 )
{
	return ( q1->priority <= q2->priority ) ? true : false;
}



