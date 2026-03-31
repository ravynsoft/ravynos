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
#include "FourthPage.h"

#if !defined( PBS_MARQUEE )
#	define PBS_MARQUEE  0x08
#endif

#if !defined( PBM_SETMARQUEE )
#	define PBM_SETMARQUEE WM_USER + 10
#endif



// CFourthPage dialog

IMPLEMENT_DYNAMIC(CFourthPage, CPropertyPage)
CFourthPage::CFourthPage()
	: CPropertyPage(CFourthPage::IDD),
		m_initialized(false)
{
	CString fontName;

	m_psp.dwFlags &= ~(PSP_HASHELP);
	m_psp.dwFlags |= PSP_DEFAULT|PSP_HIDEHEADER;

	fontName.LoadString(IDS_LARGE_FONT);

	// create the large font
	m_largeFont.CreateFont(-16, 0, 0, 0, 
		FW_BOLD, FALSE, FALSE, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, 
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, fontName);
}

CFourthPage::~CFourthPage()
{
}

void CFourthPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_GOODBYE, m_goodbye);
	DDX_Control(pDX, IDC_PRINTER_NAME, m_printerNameCtrl);
	DDX_Control(pDX, IDC_PRINTER_MANUFACTURER, m_printerManufacturerCtrl);
	DDX_Control(pDX, IDC_PRINTER_MODEL, m_printerModelCtrl);
	DDX_Control(pDX, IDC_PRINTER_PROTOCOL, m_printerProtocolCtrl);
	DDX_Control(pDX, IDC_PRINTER_DEFAULT, m_printerDefault);
}


BEGIN_MESSAGE_MAP(CFourthPage, CPropertyPage)
END_MESSAGE_MAP()


// CFourthPage message handlers
OSStatus 
CFourthPage::OnInitPage()
{
	CWnd * window; 
	OSStatus err = kNoErr;

	window = GetDlgItem( IDC_INSTALLING );
	require_action( window, exit, err = kUnknownErr );
	window->ShowWindow( SW_HIDE );

	window = GetDlgItem( IDC_PROGRESS );
	require_action( window, exit, err = kUnknownErr );
	SetWindowLong( *window, GWL_STYLE, GetWindowLong( *window, GWL_STYLE ) | PBS_MARQUEE );
	SetWindowLongPtr( *window, GWL_STYLE, GetWindowLongPtr( *window, GWL_STYLE ) | PBS_MARQUEE );
	window->SendMessage( ( UINT ) PBM_SETMARQUEE, ( WPARAM ) FALSE,( LPARAM ) 35 );
	window->ShowWindow( SW_HIDE );

exit:

	return err;
}


BOOL
CFourthPage::OnSetActive()
{
	CPrinterSetupWizardSheet	*	psheet;
	CString							goodbyeText;
	Printer						*	printer;
	CString							defaultText;

	psheet = reinterpret_cast<CPrinterSetupWizardSheet*>(GetParent());
	require_quiet( psheet, exit );
   
	printer = psheet->GetSelectedPrinter();
	require_quiet( psheet, exit );
	
	psheet->SetWizardButtons(PSWIZB_BACK|PSWIZB_FINISH);

	if (m_initialized == false)
	{
		m_initialized = true;
		OnInitPage();
	}

	m_goodbye.SetFont(&m_largeFont);

	goodbyeText.LoadString(IDS_GOODBYE);
	m_goodbye.SetWindowText(goodbyeText);

	m_printerNameCtrl.SetWindowText( printer->actualName );
	m_printerManufacturerCtrl.SetWindowText ( printer->manufacturer );
	m_printerModelCtrl.SetWindowText ( printer->displayModelName );

	Service * service = printer->services.front();
	require_quiet( service, exit );
	m_printerProtocolCtrl.SetWindowText ( service->protocol );

	if (printer->deflt)
	{
		defaultText.LoadString(IDS_YES);
	}
	else
	{
		defaultText.LoadString(IDS_NO);
	}

	m_printerDefault.SetWindowText ( defaultText );

exit:

	return CPropertyPage::OnSetActive();
}


BOOL
CFourthPage::OnKillActive()
{
	CPrinterSetupWizardSheet * psheet;

	psheet = reinterpret_cast<CPrinterSetupWizardSheet*>(GetParent());
	require_quiet( psheet, exit );   
   
	psheet->SetLastPage(this);

exit:

	return CPropertyPage::OnKillActive();
}


BOOL
CFourthPage::StartActivityIndicator()
{
	CWnd * window; 
	BOOL ok = TRUE;

	window = GetDlgItem( IDC_COMPLETE1 );
	require_action( window, exit, ok = FALSE );
	window->ShowWindow( SW_HIDE );

	window = GetDlgItem( IDC_COMPLETE2 );
	require_action( window, exit, ok = FALSE );
	window->ShowWindow( SW_HIDE );

	window = GetDlgItem( IDC_INSTALLING );
	require_action( window, exit, ok = FALSE );
	window->ShowWindow( SW_SHOW );

	window = GetDlgItem( IDC_PROGRESS );
	require_action( window, exit, ok = FALSE );
	window->SendMessage( ( UINT ) PBM_SETMARQUEE, ( WPARAM ) TRUE,( LPARAM ) 50 );
	window->ShowWindow( SW_SHOW );

exit:

	return ok;
}


BOOL
CFourthPage::StopActivityIndicator()
{
	CWnd * window; 
	BOOL ok = TRUE;

	window = GetDlgItem( IDC_INSTALLING );
	require_action( window, exit, ok = FALSE );
	window->ShowWindow( SW_HIDE );

	window = GetDlgItem( IDC_PROGRESS );
	require_action( window, exit, ok = FALSE );
	window->SendMessage( ( UINT ) PBM_SETMARQUEE, ( WPARAM ) FALSE,( LPARAM ) 35 );
	window->ShowWindow( SW_HIDE );

exit:

	return ok;
}
