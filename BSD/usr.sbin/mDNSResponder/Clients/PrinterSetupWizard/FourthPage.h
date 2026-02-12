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
#include "afxwin.h"


// CFourthPage dialog

class CFourthPage : public CPropertyPage
{
DECLARE_DYNAMIC(CFourthPage)

public:
CFourthPage();
virtual ~CFourthPage();

// Dialog Data
enum { IDD = IDD_FOURTH_PAGE };

virtual BOOL OnSetActive();
virtual BOOL OnKillActive();

BOOL StartActivityIndicator();
BOOL StopActivityIndicator();

protected:
virtual void DoDataExchange(CDataExchange* pDX);        // DDX/DDV support

DECLARE_MESSAGE_MAP()

private:

OSStatus    OnInitPage();
CFont m_largeFont;
bool m_initialized;


public:
CStatic m_goodbye;
private:
CStatic m_printerNameCtrl;
CStatic m_printerManufacturerCtrl;
CStatic m_printerModelCtrl;
CStatic m_printerProtocolCtrl;
CStatic m_printerDefault;
};
