/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
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
 * @header CAuditUtils
 */

#include "CAuditUtils.h"
#include "SharedConsts.h"
#include "PluginData.h"
#include "DirServices.h"
#include "DirServicesUtils.h"
#include "DSUtils.h"

/* list of attributes that are audited */
#if USE_BSM_AUDIT
static const char *sAuditAttrTable[] = 
{
	/* attributes */
	kDS1AttrSMBRID,
	kDS1AttrSMBGroupRID,
	kDS1AttrSMBSID,
	kDS1AttrSMBPrimaryGroupSID,
	kDS1AttrDistinguishedName,
	kDS1AttrFirstName,
	kDS1AttrMiddleName,
	kDS1AttrLastName,
	kDS1AttrPassword,
	kDS1AttrPasswordPlus,
	kDS1AttrAuthenticationHint,
	kDS1AttrUniqueID,
	kDS1AttrPrimaryGroupID,
	kDS1AttrGeneratedUID,
	kDS1AttrRealUserID,
	kDSNAttrGroupMembership,
	kDS1AttrAuthCredential,
	kDSNAttrKDCAuthKey,
	kDSNAttrRecordName,
	kDSNAttrSetPasswdMethod,
	kDSNAttrGroup,
	kDSNAttrMember,
	kDSNAttrNetGroups,
	kDSNAttrNickName,
	kDSNAttrNamePrefix,
	kDSNAttrNameSuffix,
	kDSNAttrComputers,
	kDSNAttrAuthenticationAuthority,
	
	/* end */
	NULL
};

static const char *sAuditMethodTable[ kAuditAuthMethodConsts ] = 
{
	/* changes password */
	kDSStdAuthSetPasswd,
	kDSStdAuthChangePasswd,
	kDSStdAuthSetPasswdAsRoot,
	kDSStdAuth2WayRandomChangePasswd,
	kDSStdAuthWriteSecureHash,
	kDSStdAuthSetWorkstationPasswd,
	kDSStdAuthSetLMHash,
	kDSStdAuthSetNTHash,
	
	/* changes something other than the password */
	kDSStdAuthSetPolicyAsRoot,
	kDSStdAuthNewUser,
	kDSStdAuthNewUserWithPolicy,
	kDSStdAuthSetPolicy,
	kDSStdAuthSetGlobalPolicy,
	kDSStdAuthSetUserName,
	kDSStdAuthSetUserData,
	kDSStdAuthDeleteUser,
	
	/* auth-only */
	kDSStdAuthClearText,
	kDSStdAuthAPOP,
	kDSStdAuth2WayRandom,
	kDSStdAuthNodeNativeClearTextOK,
	kDSStdAuthNodeNativeNoClearText,
	kDSStdAuthSMB_NT_Key,
	kDSStdAuthSMB_LM_Key,
	kDSStdAuthCRAM_MD5,
	kDSStdAuthDIGEST_MD5,
	kDSStdAuthSecureHash,
	kDSStdAuthMSCHAP2,
	kDSStdAuthMSCHAP1,
	kDSStdAuthCHAP,
	kDSStdAuthWithAuthorizationRef,
	kDSStdAuthCrypt,
	kDSStdAuthNTLMv2,
	
	/* end */
	NULL
};

// "New user [<domain>:<shortname>]",														// AUE_create_user
#define	kAuditCtlStrNewUser	"New user [<%s>:<%s>]"
	
// (In the following, if the short name is changing, use the old shortname following "Modify user.")
	
// "Modify user <shortname> <UID|GID|SHORTNAME|LONGNAME>: old = <oldval>, new = <newval>",  // AUE_modify_user
#define	kAuditCtlStrModifyUser1	"Modify user <%s>: attribute = <%s>, value = <%s>"
#define	kAuditCtlStrModifyUser2	"Modify user <%s>: new attribute <%s> = <%s>"
	
// "Modify password for user <shortname>",													// AUE_modify_password
#define	kAuditCtlStrModifyPassword	"Modify password for user <%s>"
	
// "Delete user [<uid>, <gid>, <shortname>, <longname>]",									// AUE_delete_user
#define	kAuditCtlStrDeleteUser	"Delete user [<%lu>, <%lu>, <%s>, <%s>]"
	
// "Add group [<groupname>]",																// AUE_create_group
#define	kAuditCtlStrCreateGroup	"Add group [<%s>]"
	
// "Delete group [<gid>, <groupname>]",														// AUE_delete_group
#define	kAuditCtlStrDeleteGroup	"Delete group [<%lu>, <%s>]"
	
// (In the following, if the name is changing, use the old name following "Modify group.")
	
// "Modify group <groupname> <GID|NAME>: old = <oldval>, new = <newval>",					// AUE_modify_group (membership)
#define	kAuditCtlStrModifyGroupMembership1	"Modify group <%s> <%lu>: old = <%s>, new = <%s>"
#define	kAuditCtlStrModifyGroupMembership2	"Modify group <%s>: attribute = <%s>, value = <%s>"
	
// "Add user <shortname> to group <groupname>",												// AUE_add_to_group
#define	kAuditCtlStrAddToGroup	"Add user <%s> to group <%s>"
	
// "Removed user <shortname> from group <groupname>"										// AUE_remove_from_group
#define	kAuditCtlStrRemoveFromGroup	"Removed user <%s> from group <%s>"

// "Modify group <groupname> <GID|NAME>: old = <oldval>, new = <newval>",					// AUE_modify_group (non-membership-attribute)
#define	kAuditCtlStrModifyGroupAttribute	"Modify group <%s>: attribute = <%s>, value = <%s>"
	
// "Authentication for user <shortname>",													// 
#define	kAuditCtlStrAuthenticateUser	"Authentication for user <%s>"

#endif

//------------------------------------------------------------------------------------
//	* AuditForThisEvent
//
//  RETURNS: BSM event code
//  Caller must free <outTextStr>
//------------------------------------------------------------------------------------

UInt32 AuditForThisEvent( UInt32 inType, void *inData, char **outTextStr )
{
	UInt32				eventCode					= 0;
#if USE_BSM_AUDIT
	tDataNodePtr		recType						= NULL;
	tRecordReference	recRef						= 0;
	bool				typeIsAudited				= false;
	bool				attrIsAudited				= false;
	tDirStatus			siResult					= eDSNoErr;
	char				*recTypeStr					= NULL;
	char				*recNameStr					= NULL;
	const char			*recNameToUseStr			= NULL;
	tDataNodePtr		pAttrType					= NULL;
	tDataNodePtr		pAttrValue					= NULL;
	char				*attrValueNameStr			= NULL;
	tDataNodePtr		pAuthMethod					= NULL;
	tDataNodePtr		pTempAttrValue				= NULL;
	tDataBufferPtr		authBuffer					= NULL;
	AuditTypeHint		hint						= kATHChange;
	char				textStr[256]				= {0};
	int					idx							= 0;	
	
	if ( outTextStr != NULL )
		*outTextStr = NULL;
	
	if ( au_get_state() == AUDIT_OFF )
		return 0;
	
	switch ( inType )
	{
		case kCreateRecord:
		case kCreateRecordAndOpen:
			if ( inData != NULL )
				recType = ((sCreateRecord *)inData)->fInRecType;
			if ( recType != NULL )
			{
				recNameToUseStr = ((sCreateRecord *)inData)->fInRecName ? ((sCreateRecord *)inData)->fInRecName->fBufferData : kAuditUnknownNameStr;
				
				if ( strcmp( recType->fBufferData, kDSStdRecordTypeUsers ) == 0 )
				{
					eventCode = AUE_create_user;
					snprintf( textStr, sizeof(textStr), kAuditCtlStrNewUser, "", recNameToUseStr );
				}
				else
				if ( strcmp( recType->fBufferData, kDSStdRecordTypeGroups ) == 0 )
				{
					eventCode = AUE_create_group;
					snprintf( textStr, sizeof(textStr), kAuditCtlStrCreateGroup, recNameToUseStr );
				}
			}
			break;
			
		case kAddAttribute:
			if ( inData != NULL )
			{
				recRef = ((sAddAttribute *)inData)->fInRecRef;
				pAttrType = ((sAddAttribute *)inData)->fInNewAttr;
				pAttrValue = ((sAddAttribute *)inData)->fInFirstAttrValue;
				hint = kATHAdd;
				AuditUserOrGroupRecord( recRef, &recNameStr, &recTypeStr, &eventCode );
			}
			break;
						
		case kRemoveAttribute:
			if ( inData != NULL )
			{
				recRef = ((sRemoveAttribute *)inData)->fInRecRef;
				pAttrType = ((sRemoveAttribute *)inData)->fInAttribute;
				hint = kATHRemove;
				AuditUserOrGroupRecord( recRef, &recNameStr, &recTypeStr, &eventCode );
			}
			break;
			
		case kAddAttributeValue:
			if ( inData != NULL )
			{
				recRef = ((sAddAttributeValue *)inData)->fInRecRef;
				pAttrType = ((sAddAttributeValue *)inData)->fInAttrType;
				pAttrValue = ((sAddAttributeValue *)inData)->fInAttrValue;
				hint = kATHAdd;
				AuditUserOrGroupRecord( recRef, &recNameStr, &recTypeStr, &eventCode );
			}
			break;
			
		case kRemoveAttributeValue:
			if ( inData != NULL )
			{
				recRef = ((sRemoveAttributeValue *)inData)->fInRecRef;
				pAttrType = ((sRemoveAttributeValue *)inData)->fInAttrType;
				hint = kATHRemove;
				AuditUserOrGroupRecord( recRef, &recNameStr, &recTypeStr, &eventCode );
			}
			break;
			
		case kSetAttributeValue:
			if ( inData != NULL )
			{
				recRef = ((sSetAttributeValue *)inData)->fInRecRef;
				pAttrType = ((sSetAttributeValue *)inData)->fInAttrType;
				if ( ((sSetAttributeValue *)inData)->fInAttrValueEntry != NULL )
					pAttrValue = &(((sSetAttributeValue *)inData)->fInAttrValueEntry->fAttributeValueData);
				AuditUserOrGroupRecord( recRef, &recNameStr, &recTypeStr, &eventCode );
			}
			break;
		
		case kSetAttributeValues:
			if ( inData != NULL )
			{
				recRef = ((sSetAttributeValues *)inData)->fInRecRef;
				pAttrType = ((sSetAttributeValues *)inData)->fInAttrType;
				if ( ((sSetAttributeValues *)inData)->fInAttrValueList != NULL ) {
					if (dsDataListGetNodeAllocPriv(((sSetAttributeValues *)inData)->fInAttrValueList, 1, &pTempAttrValue) == eDSNoErr) {
						pAttrValue = pTempAttrValue;
					}
				}
				AuditUserOrGroupRecord( recRef, &recNameStr, &recTypeStr, &eventCode );
			}
			break;
		
		case kDeleteRecord:
			if ( inData != NULL )
			{
				recRef = ((sDeleteRecord *)inData)->fInRecRef;
				siResult = AuditGetRecordRefInfo( recRef, &recNameStr, &recTypeStr );
				if ( siResult == eDSNoErr )
				{
					recNameToUseStr = (recNameStr != NULL) ? recNameStr : kAuditUnknownNameStr;
					if ( strcmp( (const char *)recTypeStr, kDSStdRecordTypeUsers ) == 0 )
					{
						eventCode = AUE_delete_user;
						snprintf( textStr, sizeof(textStr), kAuditCtlStrDeleteUser, 99L, 99L, recNameToUseStr, "" );
					}
					else
					if ( strcmp( (const char *)recTypeStr, kDSStdRecordTypeGroups ) == 0 )
					{
						eventCode = AUE_delete_group;
						snprintf( textStr, sizeof(textStr), kAuditCtlStrDeleteGroup, 99L, recNameToUseStr );
					}
				}
			}
			break;
			
		case kSetRecordName:
			if ( inData != NULL )
			{
				recRef = ((sSetRecordName *)inData)->fInRecRef; 
				AuditUserOrGroupRecord( recRef, &recNameStr, &recTypeStr, &eventCode );
			}
			break;
			
		case kSetRecordType:
			if ( inData != NULL )
			{
				recRef = ((sSetRecordType *)inData)->fInRecRef;	
				AuditUserOrGroupRecord( recRef, &recNameStr, &recTypeStr, &eventCode );
			}
			break;
		
		case kDoDirNodeAuth:
			pAuthMethod = ((sDoDirNodeAuth *)inData)->fInAuthMethod;
			authBuffer = ((sDoDirNodeAuth *)inData)->fInAuthStepData;
			eventCode = AUE_modify_user;
			typeIsAudited = true;
			break;
			
		case kDoDirNodeAuthOnRecordType:
			pAuthMethod = ((sDoDirNodeAuthOnRecordType *)inData)->fInAuthMethod;
			authBuffer = ((sDoDirNodeAuthOnRecordType *)inData)->fInAuthStepData;
			eventCode = AUE_modify_user;
			typeIsAudited = true;
			break;
		
		case kCheckUserNameAndPassword:
			//pAuthMethod => kDSStdAuthNodeNativeClearTextOK
			pAuthMethod = nil;
			recNameStr = strdup((char *)inData);
			eventCode = AUE_modify_user;
			typeIsAudited = true;
			break;
			
		default:
			typeIsAudited = false;
	}
	
	if ( eventCode > 0 )
	{
		typeIsAudited = true;
		if ( pAttrType != NULL )
		{
			for ( idx = 0; sAuditAttrTable[idx] != NULL; idx++ )
			{
				if ( strcmp( pAttrType->fBufferData, sAuditAttrTable[idx] ) == 0 )
				{
					attrIsAudited = true;
					break;
				}
			}
			
			if ( attrIsAudited )
			{
				attrValueNameStr = (pAttrValue != NULL) ? dsCStrFromCharacters(pAttrValue->fBufferData, pAttrValue->fBufferLength) : strdup("");
				
				if ( eventCode == AUE_modify_user )
				{
					recNameToUseStr = (recNameStr != NULL) ? recNameStr : kAuditUnknownNameStr;
					
					// we handle the authentication authority with special checking
					if (strcmp(pAttrType->fBufferData, kDSNAttrAuthenticationAuthority) == 0) {
						bool hasDisableTag = (strcasestr(attrValueNameStr, kDSTagAuthAuthorityDisabledUser) != NULL);
						
						switch( hint ) {
							case kATHAdd:
							case kATHChange:
								if (hasDisableTag == true) {
									snprintf(textStr, sizeof(textStr), "%s disabled by adding ';DisabledUser;'", recNameToUseStr);
									eventCode = AUE_disable_user;
									break;
								}
								else if (dsIsRecordDisabledInternal(recRef) == true) {
									snprintf(textStr, sizeof(textStr), "%s enabled by removing ';DisabledUser;'", recNameToUseStr);
									eventCode = AUE_enable_user;
								}
								
								break;
								
							case kATHRemove:
								if (hasDisableTag == false) {
									snprintf(textStr, sizeof(textStr), "%s enabled by removing ';DisabledUser;'", recNameToUseStr);
									eventCode = AUE_enable_user;
								}
								break;
						}
					}
					else {
						switch( hint )
						{
							case kATHAdd:
								snprintf( textStr, sizeof(textStr), kAuditCtlStrModifyUser2, recNameToUseStr, pAttrType->fBufferData, attrValueNameStr );
								break;
								
							default:
								snprintf( textStr, sizeof(textStr), kAuditCtlStrModifyUser1, recNameToUseStr, pAttrType->fBufferData, attrValueNameStr );
								break;
						}
					}
				}
				else
				// special-case for special attribute
				if ( eventCode == AUE_modify_group )
				{
					recNameToUseStr = (recNameStr != NULL) ? recNameStr : kAuditUnknownNameStr;
					
					if ( strcmp( pAttrType->fBufferData, kDSNAttrGroupMembership ) == 0 )
					{
						switch( hint )
						{
							case kATHChange:
								//eventCode = AUE_modify_group;
								snprintf( textStr, sizeof(textStr), kAuditCtlStrModifyGroupMembership2, recNameToUseStr, attrValueNameStr, "" );
								break;
							
							case kATHAdd:
								eventCode = AUE_add_to_group;
								if ( attrValueNameStr == NULL || *attrValueNameStr == '\0' )
									snprintf( textStr, sizeof(textStr), kAuditCtlStrModifyGroupAttribute, recNameToUseStr, pAttrType->fBufferData, "" );
								else
									snprintf( textStr, sizeof(textStr), kAuditCtlStrAddToGroup, attrValueNameStr, recNameToUseStr );
								break;
							
							case kATHRemove:
								eventCode = AUE_remove_from_group;
								snprintf( textStr, sizeof(textStr), kAuditCtlStrRemoveFromGroup, attrValueNameStr, recNameToUseStr );
								break;
						}
					}
					else
					{
						snprintf( textStr, sizeof(textStr), kAuditCtlStrModifyGroupAttribute, recNameToUseStr, pAttrType->fBufferData, attrValueNameStr );
					}
				}

				free(attrValueNameStr);
			} //if ( attrIsAudited )
			else
			{
				eventCode = 0;
			}
		} //if ( pAttrType != NULL )
		
		else if ( pAuthMethod != NULL && attrIsAudited == false )
		{
			for ( idx = 0; sAuditMethodTable[idx] != NULL; idx++ )
			{
				if ( strcmp( pAuthMethod->fBufferData, sAuditMethodTable[idx] ) == 0 )
				{
					attrIsAudited = true;
					break;
				}
			}
			
			if ( attrIsAudited )
			{
				AuditGetNameFromAuthBuffer( pAuthMethod, authBuffer, &recNameStr );
				
				// change code for auth-only
				if ( eventCode == AUE_modify_user )
				{
					recNameToUseStr = (recNameStr != NULL) ? recNameStr : kAuditUnknownNameStr;
					
					if ( idx > kAuditAuthChangeConsts )
					{
						eventCode = AUE_auth_user;
						snprintf( textStr, sizeof(textStr), kAuditCtlStrAuthenticateUser, recNameToUseStr );
					}
					else
					if ( idx < kAuditAuthPasswordChangeConsts )
					{
						eventCode = AUE_modify_password;
						snprintf( textStr, sizeof(textStr), kAuditCtlStrModifyPassword, recNameToUseStr );
					}
				}
			}
			else
			{
				eventCode = 0;
			}
		}
		
		else if ( pAuthMethod == NULL && attrIsAudited == false && inType == kCheckUserNameAndPassword )
		{
			for ( idx = 0; sAuditMethodTable[idx] != NULL; idx++ )
			{
				if ( strcmp( kDSStdAuthNodeNativeClearTextOK, sAuditMethodTable[idx] ) == 0 )
				{
					attrIsAudited = true;
					break;
				}
			}
			
			if ( attrIsAudited )
			{
				// change code for auth-only
				if ( eventCode == AUE_modify_user )
				{
					recNameToUseStr = (recNameStr != NULL) ? recNameStr : kAuditUnknownNameStr;
					
					eventCode = AUE_auth_user;
					snprintf( textStr, sizeof(textStr), kAuditCtlStrAuthenticateUser, recNameToUseStr );
				}
			}
			else
			{
				eventCode = 0;
			}
		}
		
		if ( outTextStr != NULL && textStr[0] != '\0' ) {
			*outTextStr = strdup( textStr );
		}
		else {
			// no string, no eventCode
			eventCode = 0;
		}
	}
	
	if (pTempAttrValue != NULL) {
		dsDataNodeDeAllocate(0, pTempAttrValue);
		pTempAttrValue = NULL;
	}
	
	DSFree( recTypeStr );
	DSFree( recNameStr );
#endif
	
	return eventCode;
}


//------------------------------------------------------------------------------------
//	* AuditUserOrGroupRecord
//
//  RETURNS: DS status
//------------------------------------------------------------------------------------

tDirStatus AuditUserOrGroupRecord( tRecordReference inRecRef, char **outRecNameStr, char **outRecTypeStr, UInt32 *outEventCode )
{
	tDirStatus			siResult			= eDSNoErr;
	
	if ( outRecNameStr == NULL || outRecTypeStr == NULL || outEventCode == NULL )
		return eParameterError;
	
	siResult = AuditGetRecordRefInfo( inRecRef, outRecNameStr, outRecTypeStr );
	if ( siResult == eDSNoErr )
	{
		if ( strcmp( *outRecTypeStr, kDSStdRecordTypeUsers ) == 0 )
		{
			*outEventCode = AUE_modify_user;
		}
		else
		if ( strcmp( *outRecTypeStr, kDSStdRecordTypeGroups ) == 0 )
		{
			*outEventCode = AUE_modify_group;
		}
	}
	
	return siResult;
}
		
		
//------------------------------------------------------------------------------------
//	* AuditGetRecordRefInfo
//
//  RETURNS: DS status
//------------------------------------------------------------------------------------

tDirStatus AuditGetRecordRefInfo( tRecordReference inRecRef, char **outRecNameStr, char **outRecTypeStr )
{
	tDirStatus siResult = eDSNoErr;
	tRecordEntryPtr recInfoPtr = NULL;
	
	siResult = dsGetRecordReferenceInfoInternal( inRecRef, &recInfoPtr );
	if ( siResult == eDSNoErr )
	{
		siResult = dsGetRecordTypeFromEntry( recInfoPtr, outRecTypeStr );
		if ( siResult == eDSNoErr )
		{
			siResult = dsGetRecordNameFromEntry( recInfoPtr, outRecNameStr );
			if ( siResult != eDSNoErr && *outRecTypeStr != NULL )
			{
				free( *outRecTypeStr );
				*outRecTypeStr = NULL;
			}
		}
		
		dsDeallocRecordEntry( 0, recInfoPtr );
	}
	
	return siResult;
}


//------------------------------------------------------------------------------------
//	* AuditGetNameFromAuthBuffer
//
//  RETURNS: DS status
//------------------------------------------------------------------------------------

tDirStatus AuditGetNameFromAuthBuffer( tDataNodePtr inAuthMethod, tDataBufferPtr inAuthBuffer, char **outUserNameStr )
{
	tDirStatus siResult = eDSNoErr;
	tDataListPtr dataList = NULL;
    tDataNodePtr dataNode = NULL;
	UInt32 len = 0;
	
	if ( outUserNameStr == NULL )
		return eParameterError;
	*outUserNameStr = NULL;
	
	if ( strcmp( inAuthMethod->fBufferData, kDSStdAuth2WayRandom ) == 0 )
	{
		*outUserNameStr = (char *) malloc( inAuthBuffer->fBufferLength + 1 );
		if ( *outUserNameStr == NULL )
			return eMemoryError;
		
		strlcpy( *outUserNameStr, inAuthBuffer->fBufferData, inAuthBuffer->fBufferLength + 1 );
	}
	else
	if ( strncmp( inAuthMethod->fBufferData, kDSStdAuthMethodPrefix, sizeof(kDSStdAuthMethodPrefix)-1 ) == 0 )
	{
		// name is the first item in the buffer
		dataList = dsAuthBufferGetDataListAllocPriv( inAuthBuffer );
		if ( dataList == NULL )
		{
			siResult = eDSInvalidBuffFormat;
		}
		else
		{
			if ( dsDataListGetNodeCountPriv(dataList) < 1 )
			{
				siResult = eDSInvalidBuffFormat;
			}
			else
			{
				siResult = dsDataListGetNodeAllocPriv( dataList, 1, &dataNode );
				if ( siResult == eDSNoErr )
				{
					if ( dataNode->fBufferLength > 34 && strncmp( dataNode->fBufferData, "0x", 2 ) == 0 )
						len = 34;
					else
						len = dataNode->fBufferLength;
						
					*outUserNameStr = (char *) calloc( len + 1, 1 );
					if ( *outUserNameStr == NULL )
					{
						siResult = eMemoryError;
					}
					else
					{
						memcpy( *outUserNameStr, dataNode->fBufferData, len );
					}
				}
			}
		}
        
		if ( dataNode != NULL ) {
			dsDataBufferDeallocatePriv( dataNode );
			dataNode = NULL;
		}
		
		if ( dataList != NULL ) {
			(void)dsDataListDeallocatePriv( dataList );
			free( dataList );
		}
	}
	
	return siResult;
}



