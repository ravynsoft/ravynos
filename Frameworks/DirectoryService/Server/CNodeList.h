/*
 * Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

/*!
 * @header CNodeList
 */

#ifndef __CNodeList_h__
#define __CNodeList_h__	1

#include <map>			//STL map class
#include <string>		//STL string class

#include "DirServicesTypes.h"
#include "PrivateTypes.h"
#include "DSMutexSemaphore.h"
#include "DSEventSemaphore.h"
#include "PluginData.h"

class	CServerPlugin;

using namespace std;

typedef struct sDSNode
{
	tDataList		*fDataListPtr;
	CServerPlugin	*fPlugInPtr;
	UInt32			fPlugInToken;
	UInt32			fDirNodeTypes;
} sDSNode;

// Typedefs --------------------------------------------------------------------

typedef map<string, sDSNode*>	DSNodeMap;
typedef DSNodeMap::iterator		DSNodeMapI;

// Classes ---------------------------------------------------------------------

class DSNode {

public:
					DSNode			( void );
	virtual		   ~DSNode			( void );

protected:

private:
   	char			   *fNodeName;
	tDataList		   *fDataListPtr;
	CServerPlugin	   *fPlugInPtr;
	UInt32				fPlugInToken;
	eDirNodeType		fType;
	DSMutexSemaphore	fMutex;

};

class CNodeList {

public:

typedef struct sTreeNode
{
   	char			*fNodeName;
	tDataList		*fDataListPtr;
	CServerPlugin	*fPlugInPtr;
	UInt32			fPlugInToken;
	eDirNodeType	fType;
   	sTreeNode		*left;
   	sTreeNode		*right;
} sTreeNode;

enum {
	kBuffFull		= -128,
	kBuffTooSmall	= -129,
	kDupNodeErr		= -130
};

public:
					CNodeList			( void );
	virtual		   ~CNodeList			( void );

	void			RegisterAll			( void );
	UInt32			GetNodeCount		( void );
	UInt32			GetNodeChangeToken 	( void );
	void			CountNodes			( UInt32 *outCount );
	bool		 	IsPresent			( const char *inStr, eDirNodeType inType );
	bool		 	GetPluginHandle		( const char *inStr, CServerPlugin **outPlugInPtr );
	CServerPlugin*	GetPluginPtr		( sTreeNode* nodePtr );
	
	SInt32			GetNodes			( char *inStr, tDirPatternMatch inMatch, tDataBuffer *inBuff );

	SInt32		   	AddNode				( const char *inStr, tDataList *inListPtr, eDirNodeType inType, CServerPlugin *inPlugInPtr, UInt32 inToken );
	bool			DeleteNode			( char *inStr );

	SInt32		   	BuildNodeListBuff	( sGetDirNodeList *inData );

	const char*		GetDelimiter		( void ) { return( "/" ); };

	void			WaitForLocalNode		( void );
	void			WaitForCacheNode		( void );
	void			WaitForConfigureNode	( void );
	void			WaitForBSDNode			( void );
	void			WaitForAuthenticationSearchNode	( void );
	void			Lock					( void) { fMutex.WaitLock(); }
	void			Unlock					( void) { fMutex.SignalLock(); }

protected:
	// Protected member functions
	SInt32		DeleteTree				( sTreeNode **inTree );	// called by the destructor
	bool		DeleteNodeFromTree		( char *inStr, sTreeNode  *inTree );

	SInt32		AddNodePathToTDataBuff	( tDataList *inPtr, tDataBuffer *inBuff );

private:
	// Private member functions
	void		Count					( sTreeNode *inTree, UInt32 *outCount );
	SInt32		DoGetNode				( sTreeNode *inTree, char *inStr, tDirPatternMatch inMatch, tDataBuffer *inBuff, sTreeNode **outNodePtr );
	void		Register				( sTreeNode *inTree );
	SInt32		CompareCString			( const char *inStr_1, const char *inStr_2 );

	SInt32		DoBuildNodeListBuff		( sTreeNode *inTree, tDataBuffer *outData, UInt32 *outCount );

	SInt32	   	AddLocalNode					( const char *inStr, tDataList *inListPtr, eDirNodeType inType, CServerPlugin *inPlugInPtr, UInt32 inToken );
	SInt32	   	AddCacheNode					( const char *inStr, tDataList *inListPtr, eDirNodeType inType, CServerPlugin *inPlugInPtr, UInt32 inToken );
	SInt32	   	AddNodeToTree					( sTreeNode **inTree, const char *inStr, tDataList *inListPtr, eDirNodeType inType, CServerPlugin *inPlugInPtr, UInt32 inToken );
	SInt32	   	AddAuthenticationSearchNode		( const char *inStr, tDataList *inListPtr, eDirNodeType inType, CServerPlugin *inPlugInPtr, UInt32 inToken );
	SInt32	   	AddContactsSearchNode			( const char *inStr, tDataList *inListPtr, eDirNodeType inType, CServerPlugin *inPlugInPtr, UInt32 inToken );
	SInt32	   	AddNetworkSearchNode			( const char *inStr, tDataList *inListPtr, eDirNodeType inType, CServerPlugin *inPlugInPtr, UInt32 inToken );
	SInt32	   	AddConfigureNode				( const char *inStr, tDataList *inListPtr, eDirNodeType inType, CServerPlugin *inPlugInPtr, UInt32 inToken );
	SInt32		AddBSDNode						( const char *inNodeName, tDataList *inListPtr, eDirNodeType inType, CServerPlugin *inPlugInPtr, UInt32	inToken );

	bool	 	GetLocalNode					( CServerPlugin **outPlugInPtr );
	bool	 	GetLocalNode					( tDataList **outListPtr );

	bool	 	GetCacheNode					( CServerPlugin **outPlugInPtr );
	bool	 	GetCacheNode					( tDataList **outListPtr );

	bool	 	GetAuthenticationSearchNode		( CServerPlugin **outPlugInPtr );
	bool	 	GetAuthenticationSearchNode		( tDataList **outListPtr );

	bool	 	GetContactsSearchNode			( CServerPlugin **outPlugInPtr );
	bool	 	GetContactsSearchNode			( tDataList **outListPtr );

	bool	 	GetNetworkSearchNode			( CServerPlugin **outPlugInPtr );
	bool	 	GetNetworkSearchNode			( tDataList **outListPtr );

	void		WaitForContactsSearchNode		( void );
	void		WaitForNetworkSearchNode		( void );

	// Private data members
	sTreeNode		   *fTreePtr;
	sTreeNode		   *fLocalNode;
	sTreeNode		   *fCacheNode;
	sTreeNode		   *fConfigureNode;
	sTreeNode		   *fAuthenticationSearchNode;
	sTreeNode		   *fContactsSearchNode;
	sTreeNode		   *fNetworkSearchNode;
	sTreeNode		   *fLocalHostedNodes;
	sTreeNode		   *fDefaultNetworkNodes;
	sTreeNode		   *fBSDNode;
	UInt32				fCount;
	UInt32				fNodeChangeToken;

	DSMutexSemaphore		fMutex;
	DSEventSemaphore		fWaitForAuthenticationSN;
	DSEventSemaphore		fWaitForContactsSN;
	DSEventSemaphore		fWaitForNetworkSN;
	DSEventSemaphore		fWaitForLN;
	DSEventSemaphore		fWaitForCacheN;
	DSEventSemaphore		fWaitForConfigureN;
	DSEventSemaphore		fWaitForBSDN;
};

#endif
