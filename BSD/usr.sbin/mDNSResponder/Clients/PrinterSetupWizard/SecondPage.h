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

#pragma once

#include "PrinterSetupWizardSheet.h"
#include "CommonServices.h"
#include "UtilTypes.h"
#include "afxcmn.h"
#include "dns_sd.h"
#include "afxwin.h"
#include <map>

using namespace PrinterSetupWizard;

// CSecondPage dialog

class CSecondPage : public CPropertyPage
{
DECLARE_DYNAMIC(CSecondPage)

public:
CSecondPage();
virtual ~CSecondPage();

// Dialog Data
enum { IDD = IDD_SECOND_PAGE };

protected:

void         InitBrowseList();
virtual void DoDataExchange(CDataExchange* pDX);        // DDX/DDV support
afx_msg BOOL OnSetCursor(CWnd * pWnd, UINT nHitTest, UINT message);
virtual BOOL OnSetActive();
virtual BOOL OnKillActive();

DECLARE_MESSAGE_MAP()

public:

HTREEITEM m_emptyListItem;
bool m_selectOkay;
CTreeCtrl m_browseList;
bool m_initialized;
bool m_waiting;

afx_msg void    OnTvnSelchangedBrowseList(NMHDR *pNMHDR, LRESULT *pResult);
afx_msg void    OnNmClickBrowseList(NMHDR * pNMHDR, LRESULT * pResult);
afx_msg void    OnTvnKeyDownBrowseList(NMHDR * pNMHDR, LRESULT * pResult );

OSStatus
OnAddPrinter(
    Printer     *   printer,
    bool moreComing);

OSStatus
OnRemovePrinter(
    Printer     *   printer,
    bool moreComing);

void
OnResolveService( Service * service );

private:

void
LoadTextAndDisableWindow( CString & text );

void
SetPrinterInformationState( BOOL state );

std::string m_selectedName;


private:



CStatic m_printerInformation;

CStatic m_descriptionLabel;

CStatic m_descriptionField;

CStatic m_locationLabel;

CStatic m_locationField;


bool m_gotChoice;
};
