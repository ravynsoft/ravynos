#pragma once


// CAbout dialog

class CAbout : public CDialog
{
DECLARE_DYNAMIC(CAbout)

public:
CAbout(CWnd* pParent = NULL);       // standard constructor
virtual ~CAbout();

// Dialog Data
enum { IDD = IDD_DIALOG1 };

protected:
virtual void DoDataExchange(CDataExchange* pDX);        // DDX/DDV support

DECLARE_MESSAGE_MAP()
};
