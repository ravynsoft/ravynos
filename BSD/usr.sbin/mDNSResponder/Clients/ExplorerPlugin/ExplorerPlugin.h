/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2003-2004 Apple Computer, Inc. All rights reserved.
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

//===========================================================================================================================
//	Globals
//===========================================================================================================================

// {9999A076-A9E2-4c99-8A2B-632FC9429223}
DEFINE_GUID(CLSID_ExplorerBar,
            0x9999a076, 0xa9e2, 0x4c99, 0x8a, 0x2b, 0x63, 0x2f, 0xc9, 0x42, 0x92, 0x23);

extern HINSTANCE gInstance;
extern int gDLLRefCount;
extern HINSTANCE        GetNonLocalizedResources();
extern HINSTANCE        GetLocalizedResources();


class CExplorerPluginApp : public CWinApp
{
public:

CExplorerPluginApp();
virtual ~CExplorerPluginApp();

protected:

virtual BOOL    InitInstance();
virtual int     ExitInstance();

DECLARE_DYNAMIC(CExplorerPluginApp);
};
