// About.cpp : implementation file
//

#include "stdafx.h"
#include "ExplorerPlugin.h"
#include "About.h"
#include "WinVersRes.h"
#include <DebugServices.h>


// CAbout dialog

IMPLEMENT_DYNAMIC(CAbout, CDialog)
CAbout::CAbout(CWnd* pParent /*=NULL*/)
	: CDialog(CAbout::IDD, pParent)
{
	// Initialize brush with the desired background color
	m_bkBrush.CreateSolidBrush(RGB(255, 255, 255));
}

CAbout::~CAbout()
{
}

void CAbout::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMPONENT, m_componentCtrl);
	DDX_Control(pDX, IDC_LEGAL, m_legalCtrl);
}


BEGIN_MESSAGE_MAP(CAbout, CDialog)
ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CAbout message handlers
BOOL
CAbout::OnInitDialog()
{
	BOOL b = CDialog::OnInitDialog();

	CStatic * control = (CStatic*) GetDlgItem( IDC_ABOUT_BACKGROUND );
	check( control );

	if ( control )
	{
		control->SetBitmap( ::LoadBitmap( GetNonLocalizedResources(), MAKEINTRESOURCE( IDB_ABOUT ) ) );
	}

	control = ( CStatic* ) GetDlgItem( IDC_COMPONENT_VERSION );
	check( control );

	if ( control )
	{
		control->SetWindowText( TEXT( MASTER_PROD_VERS_STR2 ) );
	}

	return b;
}


HBRUSH CAbout::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{ 
	switch (nCtlColor)
	{
		case CTLCOLOR_STATIC:
	
			if ( pWnd->GetDlgCtrlID() == IDC_COMPONENT )
			{
				pDC->SetTextColor(RGB(64, 64, 64));
			}
			else
			{
				pDC->SetTextColor(RGB(0, 0, 0));
			}

			pDC->SetBkColor(RGB(255, 255, 255));
			return (HBRUSH)(m_bkBrush.GetSafeHandle());

		case CTLCOLOR_DLG:
	
			return (HBRUSH)(m_bkBrush.GetSafeHandle());

		default:
	
			return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	}
}
