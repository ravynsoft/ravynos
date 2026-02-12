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
 * @header CRefTable
 * References for the API calls.
 */

#ifndef __CRefTable_h__
#define	__CRefTable_h__		1

#include "DirServicesTypes.h"
#include "CObject.h"
#include <dispatch/dispatch.h>
#include <map>
#include <vector>
#include <string>
#include <netinet/in.h>

using namespace std;

class CServerPlugin;

/*
 * Logic is:
 *     socket/mach_port_t -> tDirReference[] -> tNodeReference[] -> tRecordReference[]
 *                                                                  -> tAttributeListRef[]
 *                                                                  -> tAttributeValueListRef[]
 *
 * References here always use 0x00000000 bits
 *                              XX			= Reference type (eRefType)
 *                                XX        = Reference subtype (eSubType)
 *                                  XXXX	= Reference index (circular)
 *
 * Indexes are not recycled until we loop back around.  Simplifies debugging so refs aren't
 * re-used immediately after they are freed.  Most references are freed anyway, so there should
 * always be free slots.
 */

enum eRefType {
	eRefTypeDir					= 0x01,
	eRefTypeDirNode				= 0x02,
	eRefTypeRecord				= 0x03,
	eRefTypeAttributeList		= 0x04,
	eRefTypeAttributeValueList	= 0x05,
};

typedef SInt32 RefDeallocateProc ( UInt32 inRefNum, eRefType inRefType, CServerPlugin *inPluginPtr );

enum eSubType {
	eSubTypeDefault				= 0x00,
	eSubTypeTCP					= 0xC0, // used by older clients, only for reference
	eSubTypeCSBP				= 0x30, // used by older clients, only for reference
};

#define kRefTypeMask		0xff000000
#define kSubTypeMask		0x00ff0000
#define kIndexMask			0x0000ffff

struct sRefEntry;
struct sClientEntry;
class CRefTable;

// create our map types for ease
typedef map<UInt32, sRefEntry *>					tRefToEntry;
typedef map<UInt32, sRefEntry *>::iterator			tRefToEntryI;

typedef map<UInt32, sClientEntry *>					tRefToClientEntry;
typedef map<UInt32, sClientEntry *>::iterator		tRefToClientEntryI;

typedef map<mach_port_t, sClientEntry *>			tMachPortToClientEntry;
typedef map<mach_port_t, sClientEntry *>::iterator	tMachPortToClientEntryI;

typedef map<int, sClientEntry *>					tPortToClientEntry;
typedef map<int, sClientEntry *>::iterator			tPortToClientEntryI;

struct sRefEntry : public CObject<sRefEntry>
{
	UInt32				fRefNum;
	UInt32				fParentID;
	CServerPlugin		*fPlugin;
	char				*fNodeName;	// only retained for an OpenDirNode call inside the daemon for record type restrictions support
	tRefToEntry			fSubRefs;
	CRefTable			*fRefTable;

public:
			sRefEntry( void );
	
protected:
	virtual	~sRefEntry( void );
};

union uClientID
{
	union {
		sockaddr_in			ipV4;
		sockaddr_in6		ipV6;
		sockaddr_storage	storage;
	} fAddress;					// proxy clients
	pid_t			fPID;		// mach clients
};

union uPortInfo
{
	int				fSocket;	// proxy clients
	mach_port_t		fMachPort;	// mach clients
};

struct sClientEntry : public CObject<sClientEntry>
{
	int32_t				fFlags;
	tRefToEntry			fSubRefs;
	uClientID			clientID;
	uPortInfo			portInfo;
	CRefTable			*fRefTable;
	
public:
			sClientEntry( void );
	void	ClearChildRefs( void );
	
protected:
	virtual	~sClientEntry( void );
};

#define kClientTypeMach	0x00000001
#define kClientTypeTCP	0x00000002

struct tClientDetails
{
	int32_t				flags;
	uClientID			clientID;
	uPortInfo			portInfo;
	vector<UInt32>		refs;
};

inline eRefType	GetRefType( UInt32 inRefNum ) { return (eRefType) ((inRefNum & kRefTypeMask) >> 24); }
int GetClientIPString( sockaddr *address, char *clientIP, size_t client_size );

//------------------------------------------------------------------------------------
//	* CRefTable
//------------------------------------------------------------------------------------

class CRefTable {
public:
	// this queue is used externally to schedule deallocation callbacks so they are done in an orderly fashion
	dispatch_queue_t	fCleanupQueue;
	
					CRefTable			( RefDeallocateProc *deallocProc );
					~CRefTable			( void );

	tDirStatus		CreateReference		( UInt32 *outRef, eRefType inType, CServerPlugin *inPlugin, UInt32 inParentID, pid_t inPID, mach_port_t inMachPort, 
										  sockaddr *inAddress = NULL, int inSocket = 0, const char *inNodeName = NULL );
	
	tDirStatus		RemoveReference		( UInt32 inRef, eRefType inRefType, mach_port_t inMachPort, int inSocket );
	void			RemoveReference		( UInt32 inRef );
	
	tDirStatus		VerifyReference		( UInt32 inRef, eRefType inRefType, CServerPlugin **outPlugin, mach_port_t inMachPort, int inSocket );

	char *			CopyNodeRefName		( tDirNodeReference inDirNodeRef );
	tDirStatus		SetNodePluginPtr	( tDirNodeReference inNodeRef, CServerPlugin *inPlugin );
	
	CServerPlugin	*GetPluginForRef	( UInt32 inRef );

	void			CleanRefsForSocket	( int inSocket );
	void			CleanRefsForMachRefs( mach_port_t inMachPort );
	void			CleanRefsForPlugin	( CServerPlugin *inPlugin, dispatch_block_t completeBlock );

	vector<string>			*GetClientPIDListStrings	( void );
	vector<tClientDetails>	*GetClientDetails			( mach_port_t inMachPort = -1, int inSocket = -1 );
	
	inline dispatch_queue_t		GetQueue(void)			{ return fQueue; }
	inline RefDeallocateProc	*GetDeallocProc(void)	{ return fDeallocProc; }

private:
	sRefEntry		*GetRefEntry		( UInt32 inRef );
	static void		RemoveReference		( void *inContext );

private:
	RefDeallocateProc		*fDeallocProc;
	
	tMachPortToClientEntry	fMachPortToClientEntry;
	tPortToClientEntry		fPortToClientEntry;
	
	tRefToEntry				fRefToEntry;
	tRefToClientEntry		fRefToClientEntry;
	
	uint16_t				fNextIndex;	// this is the circular ref value
	
	dispatch_queue_t		fQueue;
};

#endif
