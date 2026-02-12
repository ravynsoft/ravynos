// About.cpp : implementation file
//

#include "stdafx.h"
#include "PrinterSetupWizardApp.h"
#include "About.h"


// CAbout dialog

IMPLEMENT_DYNAMIC(CAbout, CDialog)
CAbout::CAbout(CWnd* pParent /*=NULL*/)
	: CDialog(CAbout::IDD, pParent)
{
}

CAbout::~CAbout()
{
}

void CAbout::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAbout, CDialog)
END_MESSAGE_MAP()


// CAbout message handlers
