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

#ifndef __EXPLORER_BAR_WINDOW__
#define __EXPLORER_BAR_WINDOW__

#pragma once

#include    "afxtempl.h"

#include    "dns_sd.h"
#include    <list>

//===========================================================================================================================
//	Structures
//===========================================================================================================================

// Forward Declarations

struct  ServiceHandlerEntry;
class ExplorerBarWindow;

// ServiceInfo

struct  ServiceInfo
{
    CString displayName;
    char *                      name;
    char *                      type;
    char *                      domain;
    uint32_t ifi;
    HTREEITEM item;
    ServiceHandlerEntry *       handler;
    DWORD refs;

    ServiceInfo( void )
    {
        item    = NULL;
        type    = NULL;
        domain  = NULL;
        handler = NULL;
    }

    ~ServiceInfo( void )
    {
        if( name )
        {
            free( name );
        }
        if( type )
        {
            free( type );
        }
        if( domain )
        {
            free( domain );
        }
    }
};

typedef CArray < ServiceInfo *, ServiceInfo * >     ServiceInfoArray;

// TextRecord

struct  TextRecord
{
    uint8_t *       mData;
    uint16_t mSize;

    TextRecord( void )
    {
        mData = NULL;
        mSize = 0;
    }

    ~TextRecord( void )
    {
        if( mData )
        {
            free( mData );
        }
    }

    void    GetData( void *outData, uint16_t *outSize )
    {
        if( outData )
        {
            *( (void **) outData ) = mData;
        }
        if( outSize )
        {
            *outSize = mSize;
        }
    }

    OSStatus    SetData( const void *inData, uint16_t inSize )
    {
        OSStatus err;
        uint8_t *       newData;

        newData = (uint8_t *) malloc( inSize );
        require_action( newData, exit, err = kNoMemoryErr );
        memcpy( newData, inData, inSize );

        if( mData )
        {
            free( mData );
        }
        mData = newData;
        mSize = inSize;
        err  = kNoErr;

exit:
        return( err );
    }
};

// ResolveInfo

struct  ResolveInfo
{
    CString host;
    uint16_t port;
    uint32_t ifi;
    TextRecord txt;
    ServiceHandlerEntry *       handler;
};

// ServiceHandlerEntry

struct  ServiceHandlerEntry
{
    const char *            type;
    const char *            urlScheme;
    DNSServiceRef ref;
    ServiceInfoArray array;
    ExplorerBarWindow *     obj;
    bool needsLogin;

    ServiceHandlerEntry( void )
    {
        type        = NULL;
        urlScheme   = NULL;
        ref         = NULL;
        obj         = NULL;
        needsLogin  = false;
    }

    ~ServiceHandlerEntry( void )
    {
        int i;
        int n;

        n = (int) array.GetSize();
        for( i = 0; i < n; ++i )
        {
            delete array[ i ];
        }
    }
};

typedef CArray < ServiceHandlerEntry *, ServiceHandlerEntry * >     ServiceHandlerArray;

//===========================================================================================================================
//	ExplorerBarWindow
//===========================================================================================================================

class ExplorerBar;      // Forward Declaration

class ExplorerBarWindow : public CWnd
{
protected:

ExplorerBar *           mOwner;
CTreeCtrl mTree;

ServiceHandlerArray mServiceHandlers;
DNSServiceRef mResolveServiceRef;

public:

ExplorerBarWindow( void );
virtual ~ExplorerBarWindow( void );

protected:

// General

afx_msg int     OnCreate( LPCREATESTRUCT inCreateStruct );
afx_msg void    OnDestroy( void );
afx_msg void    OnSize( UINT inType, int inX, int inY );
afx_msg void    OnDoubleClick( NMHDR *inNMHDR, LRESULT *outResult );
afx_msg LRESULT OnServiceEvent( WPARAM inWParam, LPARAM inLParam );

// Browsing

static void DNSSD_API
BrowseCallBack(
    DNSServiceRef inRef,
    DNSServiceFlags inFlags,
    uint32_t inInterfaceIndex,
    DNSServiceErrorType inErrorCode,
    const char *            inName,
    const char *            inType,
    const char *            inDomain,
    void *                  inContext );
LONG OnServiceAdd( ServiceInfo * service );
LONG OnServiceRemove( ServiceInfo * service );

// Resolving

OSStatus    StartResolve( ServiceInfo *inService );
void        StopResolve( void );


void        Stop( DNSServiceRef ref );


static void DNSSD_API
ResolveCallBack(
    DNSServiceRef inRef,
    DNSServiceFlags inFlags,
    uint32_t inInterfaceIndex,
    DNSServiceErrorType inErrorCode,
    const char *            inFullName,
    const char *            inHostName,
    uint16_t inPort,
    uint16_t inTXTSize,
    const char *            inTXT,
    void *                  inContext );
LONG OnResolve( ResolveInfo * resolve );

// Accessors

public:

ExplorerBar *   GetOwner( void ) const { return( mOwner ); }
void            SetOwner( ExplorerBar *inOwner )    { mOwner = inOwner; }

DECLARE_MESSAGE_MAP()
private:

typedef std::list< DNSServiceRef >  ServiceRefList;

HTREEITEM m_about;
ServiceRefList m_serviceRefs;
CImageList m_imageList;
};

#endif  // __EXPLORER_BAR_WINDOW__
