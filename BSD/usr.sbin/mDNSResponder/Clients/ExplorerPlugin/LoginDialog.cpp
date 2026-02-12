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

#include	<assert.h>
#include	<stdlib.h>

#include	"stdafx.h"

#include	"LoginDialog.h"

// MFC Debugging

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//===========================================================================================================================
//	Message Map
//===========================================================================================================================

BEGIN_MESSAGE_MAP( LoginDialog, CDialog )
END_MESSAGE_MAP()

//===========================================================================================================================
//	LoginDialog
//===========================================================================================================================

LoginDialog::LoginDialog( CWnd *inParent )
	: CDialog( LoginDialog::IDD, inParent )
{
	//
}

//===========================================================================================================================
//	OnInitDialog
//===========================================================================================================================

BOOL	LoginDialog::OnInitDialog( void )
{
	CDialog::OnInitDialog();
	return( TRUE );
}

//===========================================================================================================================
//	DoDataExchange
//===========================================================================================================================

void	LoginDialog::DoDataExchange( CDataExchange *inDX )
{
	CDialog::DoDataExchange( inDX );
}

//===========================================================================================================================
//	OnOK
//===========================================================================================================================

void	LoginDialog::OnOK( void )
{
	const CWnd *		control;
		
	// Username
	
	control = GetDlgItem( IDC_LOGIN_USERNAME_TEXT );
	assert( control );
	if( control )
	{
		control->GetWindowText( mUsername );
	}
	
	// Password
	
	control = GetDlgItem( IDC_LOGIN_PASSWORD_TEXT );
	assert( control );
	if( control )
	{
		control->GetWindowText( mPassword );
	}
	
	CDialog::OnOK();
}

//===========================================================================================================================
//	GetLogin
//===========================================================================================================================

BOOL	LoginDialog::GetLogin( CString &outUsername, CString &outPassword )
{
	if( DoModal() == IDOK )
	{
		outUsername = mUsername;
		outPassword = mPassword;
		return( TRUE );
	}
	return( FALSE );
}
