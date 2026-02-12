#pragma once

#include "Resource.h"
#include "afxwin.h"

// CAbout dialog

class CAbout : public CDialog
{
DECLARE_DYNAMIC(CAbout)

public:
CAbout(CWnd* pParent = NULL);       // standard constructor
virtual ~CAbout();

// Dialog Data
enum { IDD = IDD_ABOUT };

protected:
virtual void DoDataExchange(CDataExchange* pDX);        // DDX/DDV support
virtual HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
virtual BOOL    OnInitDialog();
DECLARE_MESSAGE_MAP()
public:
CStatic m_componentCtrl;
CStatic m_legalCtrl;
CBrush m_bkBrush;
};
