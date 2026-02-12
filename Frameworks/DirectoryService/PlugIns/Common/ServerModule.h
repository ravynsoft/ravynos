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
 * @header ServerModule
 */

#ifndef __ServerModule_h__
#define __ServerModule_h__	1

#include <CoreFoundation/CFPlugIn.h>
#if COREFOUNDATION_CFPLUGINCOM_SEPARATE
#include <CoreFoundation/CFPlugInCOM.h>
#endif

#include <DirectoryServiceCore/PluginData.h>


//-----------------------------------------------------------------------------
//	* ServerModule Definitions
//-----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
namespace DSServerPlugin {
#endif

/**** Typedefs, enums, and constants. ****/
// The following constants are COM UUID's used to identify the plug-in classes
// in each bundle's Info.plist. See CFBundle / CFPlugIn docs for details.

#define kModuleTypeUUID			(CFUUIDGetConstantUUIDWithBytes (NULL, \
									0x69, 0x7B, 0x5D, 0x6C, 0x87, 0xA1, 0x12, 0x26, \
									0x89, 0xCA, 0x00, 0x05, 0x02, 0xC1, 0xC7, 0x36 ))

#define kModuleInterfaceUUID		(CFUUIDGetConstantUUIDWithBytes (NULL, \
			0x1A, 0xE9, 0x66, 0x90, /* - */ \
			0x62, 0xCF, /* - */ 0x12, 0x26, /* - */ 0xB4, 0x5C, /* - */ \
			0x00, 0x05, 0x02, 0x07, 0xF7, 0xFD))

// kPlugin and kPluginInterface are used to identify the plug-in type in
// each bundle's Info.plist. See CFBundle / CFPlugIn docs for details.
#define kPluginVersionStr		CFSTR( "CFBundleVersion" )
#define kPluginShortVersionStr		CFSTR( "CFBundleShortVersionString" )
#define kPluginConfigAvailStr	CFSTR( "CFBundleConfigAvail" )
#define kPluginConfigFileStr	CFSTR( "CFBundleConfigFile" )
#define kPluginNameStr			CFSTR( "CFPluginNameString" )
#define kPluginLazyNodesToRegStr	CFSTR( "DSNodesToRegister" )
#define kPluginOKToLoadLazilyStr	CFSTR( "DSOKToLoadLazily" )
#define kPluginNodeToRegisterType	CFSTR( "DSNodeToRegisterType" )
#define kPluginNodeToRegisterPath	CFSTR( "DSNodeToRegisterPath" )

typedef struct tagSvrLibFtbl
{
	SInt32			(*registerNode)		( const UInt32 inToken, tDataList *inNode, eDirNodeType inNodeType );
	SInt32			(*unregisterNode)	( const UInt32 inToken, tDataList *inNode );
	void			(*debugLog)			( const char *inFormat, va_list inArgs );
	void			(*debugLogWithType)	( const UInt32 inSignature, const UInt32 inLogType,  const char *inFormat, va_list inArgs );
	FourCharCode	fSignature;
} SvrLibFtbl;


//-----------------------------------------------------------------------------
//	* Plugin Module Function Table representation for CFPlugin
//-----------------------------------------------------------------------------
// Function table for the com.apple.DSServer.ModuleInterface

typedef struct tagModuleInterfaceFtbl
{
	/**** Required COM header info. ****/
    IUNKNOWN_C_GUTS;

	/**** Instance methods. ****/
	SInt32	(*validate)			( void *thisp, const char *inVersionStr, const UInt32 inSignature );
	SInt32	(*initialize)		( void *thisp );
	SInt32	(*configure)		( void *thisp );
	SInt32	(*processRequest)	( void *thisp, void *inData );
	SInt32	(*setPluginState)	( void *thisp, const UInt32 inState );
	SInt32	(*periodicTask)		( void *thisp );
	SInt32	(*shutdown)			( void *thisp );
	void	(*linkLibFtbl)		( void *thisp, SvrLibFtbl *inLinkBack );
} ModuleFtbl;


//-----------------------------------------------------------------------------
//	* Plugin Module Session Function Table representation for CFPlugin
//-----------------------------------------------------------------------------
// Function table for the com.apple.DSServer.ModuleInterface

typedef struct tagModuleSessionInterfaceFtbl
{
	/**** Required COM header info. ****/
    IUNKNOWN_C_GUTS;

	/**** Instance methods. ****/
	SInt32	(*receiveFromClient)	( void *thisp, const UInt8 *inData, UInt32 inLength );
} ModuleSessionFtbl;


#ifdef __cplusplus
}	// namespace DSServerPlugin
}	// extern "C"
#endif

#endif	// __ServerModule_h__
