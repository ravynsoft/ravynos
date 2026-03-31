/*
 * Copyright (c) 2004-2009 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 *
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#if !defined(DISABLE_SEARCH_PLUGIN) || !defined(DISABLE_MEMBERSHIP_CACHE)

#include "Mbrd_HashTable.h"
#include "Mbrd_UserGroup.h"
#include <strings.h>
#include <DirectoryServiceCore/CLog.h>
#include <uuid/uuid.h>
#include <syslog.h>
#include <membershipPriv.h>

// we have to use external struct because UserGroup records can be part of multiple rbtrees
struct ug_rb_node {
	struct rb_node		rbn;
	long				index;	// index from keyoffset
	struct UserGroup	*ug;
};

extern void ConvertSIDToString( char* string, ntsid_t* sid );

#define RBNODE_TO_USERGROUP(n)			(((struct ug_rb_node *) n)->ug)
#define USERGROUP_TO_KEY(ug,keyoff)		((void *) (((uintptr_t) ug) + keyoff))

static int rbt_compare_id_nodes( const struct rb_node *n1, const struct rb_node *n2 )
{
	const id_t id1 = RBNODE_TO_USERGROUP(n1)->fID;
	const id_t id2 = RBNODE_TO_USERGROUP(n2)->fID;
	
    if ( id1 < id2 ) {
        return -1;
    }
	
    if ( id1 > id2 ) {
        return 1;
    }
    return 0;
}

static int rbt_compare_id_key( const struct rb_node *n, const void *v )
{
	const id_t id1 = RBNODE_TO_USERGROUP(n)->fID;
	const id_t id2 = *((id_t *) v);
	
    if ( id1 < id2 ) {
        return -1;
    }
	
    if ( id1 > id2 ) {
        return 1;
    }
    return 0;
}

static int rbt_compare_name_nodes( const struct rb_node *n1, const struct rb_node *n2 )
{
	const char *name1 = RBNODE_TO_USERGROUP(n1)->fName;
	const char *name2 = RBNODE_TO_USERGROUP(n2)->fName;
	
	assert(name1);
	assert(name2);
	return strcmp( name1, name2 );
}

static int rbt_compare_name_key( const struct rb_node *n, const void *v )
{
	const char *name1 = RBNODE_TO_USERGROUP(n)->fName;
	const char *name2 = (const char *) v;
	
	assert(name1);
	assert(name2);
	return strcmp( name1, name2 );
}

static int rbt_compare_guid_nodes( const struct rb_node *n1, const struct rb_node *n2 )
{
	return uuid_compare( RBNODE_TO_USERGROUP(n1)->fGUID, RBNODE_TO_USERGROUP(n2)->fGUID );
}

static int rbt_compare_guid_key( const struct rb_node *n, const void *v )
{
	return uuid_compare( RBNODE_TO_USERGROUP(n)->fGUID, v );
}

static int rbt_compare_sid_nodes( const struct rb_node *n1, const struct rb_node *n2 )
{
	return bcmp( &RBNODE_TO_USERGROUP(n1)->fSID, &RBNODE_TO_USERGROUP(n2)->fSID, sizeof(ntsid_t) );
}

static int rbt_compare_sid_key( const struct rb_node *n, const void *v )
{
	return bcmp( &RBNODE_TO_USERGROUP(n)->fSID, v, sizeof(ntsid_t) );
}

static int rbt_compare_kerberos_nodes( const struct rb_node *n1, const struct rb_node *n2 )
{
	const char *kerberos1 = RBNODE_TO_USERGROUP(n1)->fKerberos[((struct ug_rb_node *)n1)->index];
	const char *kerberos2 = RBNODE_TO_USERGROUP(n2)->fKerberos[((struct ug_rb_node *)n2)->index];
	
	return strcmp( kerberos1, kerberos2 );
}

static int rbt_compare_kerberos_key( const struct rb_node *n, const void *v )
{
	const char *kerberos1 = RBNODE_TO_USERGROUP(n)->fKerberos[((struct ug_rb_node *)n)->index];
	const char *kerberos2 = (const char *) v;
	
	return strcmp( kerberos1, kerberos2 );
}

static int rbt_compare_x509dn_nodes( const struct rb_node *n1, const struct rb_node *n2 )
{
	const char *x509dn1 = RBNODE_TO_USERGROUP(n1)->fX509DN[((struct ug_rb_node *)n1)->index];
	const char *x509dn2 = RBNODE_TO_USERGROUP(n2)->fX509DN[((struct ug_rb_node *)n2)->index];
	
	return strcmp( x509dn1, x509dn2 );
}

static int rbt_compare_x509dn_key( const struct rb_node *n, const void *v )
{
	const char *x509dn1 = RBNODE_TO_USERGROUP(n)->fX509DN[((struct ug_rb_node *)n)->index];
	const char *x509dn2 = (const char *) v;
	
	return strcmp( x509dn1, x509dn2 );
}

static bool __IsReservedGroup( UserGroup *item )
{
	bool bReturn = false;
	
	if ( item == NULL || (item->fRecordType & kUGRecordTypeGroup) == 0 ) {
		return false;
	}
	
	// first look for group 80 or 20
	if ( (item->fFlags & kUGFlagHasID) != 0 && (item->fID == 20 || item->fID == 80) ) {
		item->fFlags |= kUGFlagReservedID;
		bReturn = true;
	}
	
	// first look for group name "staff" or "admin"
	if ( (item->fFlags & kUGFlagHasName) != 0 && (strcmp(item->fName, "staff") == 0 || strcmp(item->fName, "admin") == 0) ) {
		item->fFlags |= kUGFlagReservedName;
		bReturn = true;
	}
	
	// first look for builtin SID prefixes (S-1-0, S-1-1, S-1-2, S-1-3, S-1-5-1 through S-1-5-20, and S-1-5-32)
	if ( (item->fFlags & kUGFlagHasSID) != 0 && item->fSID.sid_kind == 1 ) {
		int64_t	sid_authority = ((int64_t) item->fSID.sid_authority[0]) << 40 | ((int64_t) item->fSID.sid_authority[4]) << 32 | 
		((int64_t) item->fSID.sid_authority[2]) << 24 | ((int64_t) item->fSID.sid_authority[2]) << 16 | 
		((int64_t) item->fSID.sid_authority[4]) << 8  | ((int64_t) item->fSID.sid_authority[5]);
		
		switch ( sid_authority ) {
			case 0:
				// NULL authority
			case 1:
				// world authority
			case 2:
				// Local authority
			case 3:
				// Creator authority
				item->fFlags |= kUGFlagReservedSID;
				bReturn = true;
				break;
			case 5:
				if ( item->fSID.sid_authcount > 0 ) {
					uint32_t authority = item->fSID.sid_authorities[0];
					if ( authority == 32 || (authority > 0 && authority < 21) ) {
						item->fFlags |= kUGFlagReservedSID;
						bReturn = true;
					}
				}
				break;
		}
	}
	
	return bReturn;
}

static bool __IsBuiltinGroup( UserGroup *item )
{
	bool bReturn = false;
	
	if ( (item->fFlags & kUGFlagIsBuiltin) != 0 ) {
		return true;
	}
	
	// if not a group, nothing to do
	if ( (item->fRecordType & kUGRecordTypeGroup) == 0 || (item->fFlags & kUGFlagBuiltinChecked) != 0 ) {
		return false;
	}
	
	/* if a group is a builtin group, then it is also considered authoritative for searches related to it */
	if ( __IsReservedGroup(item) == true && (item->fFlags & kUGFlagLocalAccount) != 0 ) {
		if ( (item->fFlags & (kUGFlagHasID | kUGFlagReservedID)) == (kUGFlagHasID | kUGFlagReservedID) &&
			 (item->fFoundBy & kUGFoundByID) == 0 )
		{
			DbgLog( kLogDebug, "mbr_mig - Membership - builtin group ID %d, setting authority for ID", item->fID );
			item->fFoundBy |= kUGFoundByID;
		}
		
		if ( (item->fFlags & (kUGFlagHasName | kUGFlagReservedName)) == (kUGFlagHasName | kUGFlagReservedName) &&
			 (item->fFoundBy & kUGFoundByName) == 0 )
		{
			DbgLog( kLogDebug, "mbr_mig - Membership - builtin group name '%s', setting authority for Name", item->fName );
			item->fFoundBy |= kUGFoundByName;
		}
		
		if ( (item->fFlags & (kUGFlagHasSID | kUGFlagReservedSID)) == (kUGFlagHasSID | kUGFlagReservedSID) &&
			 (item->fFoundBy & kUGFoundBySID) == 0 )
		{
			char	tempResult[MBR_MAX_SID_STRING_SIZE];
			
			ConvertSIDToString( tempResult, &item->fSID );
			DbgLog( kLogDebug, "mbr_mig - Membership - builtin group SID %s, setting authority for SID", tempResult );
			item->fFoundBy |= kUGFoundBySID;
		}
		
		item->fFlags |= kUGFlagIsBuiltin;
		bReturn = true;
	}
	
	// set the flag we have checked builtin status
	item->fFlags |= kUGFlagBuiltinChecked;

	return bReturn;
}

static bool __HashTable_Add( HashTable* hash, UserGroup* item, bool replaceExisting )
{
	bool	bSuccess	= false;
	long	iIndex		= 0;
	
	if ( hash == NULL || item == NULL ) return false;

	do
	{
		void *key = USERGROUP_TO_KEY( item, hash->fKeyOffset );
		if ( hash->fHashType == eKerberosHash || hash->fHashType == eX509DNHash ) {
			// if this is a kerberos hash or X509 hash, then it is really an array
			key = ((void **) key)[iIndex];
			if ( key == NULL ) {
				goto bail;
			}
		} else if (hash->fHashType == eNameHash) {
			key = ((void **)key)[0];
		}

		
		bool bBuiltin = __IsBuiltinGroup( item );
		struct rb_node *node = rb_tree_find_node( &hash->fRBtree, key );
		if ( node != NULL )
		{
			UserGroup *entry = RBNODE_TO_USERGROUP( node );
			
			// if entry is in hash, nothing to do
			if ( entry == item ) goto bail;
			
			// if they are both negative entries or they are both matching entries
			if ( (item->fFlags & kUGFlagNotFound) != 0 && (entry->fFlags & kUGFlagNotFound) != 0 )
			{
				goto bail;
			}
			else if ( item->fNode != NULL && entry->fNode != NULL && strcmp(item->fNode, entry->fNode) == 0 &&
					  item->fName != NULL && entry->fName != NULL && strcmp(item->fName, entry->fName) == 0 )
			{
				goto bail;
			}
			
			// if it's a builtin item, ensure it's added to the hash replacing any existing
			if ( bBuiltin == true ) {
				bool forceReplace = false;
				switch ( hash->fHashType )
				{
					case eIDHash:
						if ( (item->fFlags & kUGFlagReservedID) != 0 ) {
							forceReplace = true;
						}
						break;
						
					case eNameHash:
						if ( (item->fFlags & kUGFlagReservedName) != 0 ) {
							forceReplace = true;
						}
						break;
						
					case eSIDHash:
						if ( (item->fFlags & kUGFlagReservedSID) != 0 ) {
							forceReplace = true;
						}
						break;
				}
				
				if ( forceReplace == true ) {
					DbgLog( kLogInfo, "mbr_mig - Membership - RBtree add - builtin group ID (forcing replace because it is local)" );
					replaceExisting = true;
				}
				else {
					DbgLog( kLogInfo, "mbr_mig - Membership - RBtree add - builtin group ID (not allowing replacement of existing entry)" );
					goto bail;
				}
			}
			
			// if item is found, but the old was not found, just replace the entry
			if ( (item->fFlags & kUGFlagNotFound) == 0 && (entry->fFlags & kUGFlagNotFound) != 0 ) {
				replaceExisting = true;
			}
			
			if ( replaceExisting == true )
			{
				DbgLog( kLogDebug, "mbr_mig - Membership - RBtree add - %s - replacing existing entry %s (%X) - node %X", 
						hash->fName ?: "", entry->fName ?: "", entry, node );
				
				rb_tree_remove_node( &hash->fRBtree, node );
				DSFree( node );
				hash->fNumEntries--;
				
				// release it if owner and entry are not the same (don't self release)
				if ( entry != hash->fOwner ) {
					UserGroup_Release( entry );
					entry = NULL;
				}
			}
			// if not a reserved ID, we log a message, workaround for LDAP and AD conflicts
			else if ( (item->fFlags & (kUGFlagReservedID | kUGFlagReservedSID | kUGFlagReservedName)) == 0 )
			{
				uuid_string_t	entryUUID, itemUUID;
				char			entrySID[MBR_MAX_SID_STRING_SIZE];
				char			itemSID[MBR_MAX_SID_STRING_SIZE];
				
				ConvertSIDToString( entrySID, &entry->fSID );
				ConvertSIDToString( itemSID, &item->fSID );
				
				uuid_unparse_upper( entry->fGUID, entryUUID );
				uuid_unparse_upper( item->fGUID, itemUUID );
				
				DbgLog( kLogError, "Misconfiguration detected in hash '%s':", hash->fName );
				DbgLog( kLogError, "%s '%s' (%s) - ID %d - UUID %s - SID %s", 
						UserGroup_GetRecordTypeString(item), item->fName, item->fNode, item->fID, itemUUID, itemSID );
				DbgLog( kLogError, "%s '%s' (%s) - ID %d - UUID %s - SID %s", 
						UserGroup_GetRecordTypeString(entry), entry->fName, entry->fNode, entry->fID, entryUUID, entrySID );
				syslog( LOG_NOTICE, "Misconfiguration detected in hash '%s' - see /Library/Logs/DirectoryService/DirectoryService.error.log for details",
						hash->fName );
			}
		}
		
		struct ug_rb_node *ugnode = (struct ug_rb_node *) calloc( 1, sizeof(struct ug_rb_node) );
		assert( ugnode != NULL );
		
		ugnode->ug = item;
		ugnode->index = iIndex;
		if ( rb_tree_insert_node(&hash->fRBtree, (struct rb_node *) ugnode) == true ) {
			
			// retain it if owner and item are not the same (don't self retain)
			if ( item != hash->fOwner ) {
				(void) UserGroup_Retain( item );
			}
			hash->fNumEntries++;
			DbgLog( kLogDebug, "mbr_mig - Membership - RBtree add - %s - adding entry %s (%X) - node %X", 
					hash->fName, item->fName ? : "", item, ugnode );
			bSuccess = true;
		}
		else {
			// free the node if we didn't add it, should never happen since we searched beforehand, but safety
			DSFree( ugnode );
		}
		
		if ( bSuccess == true && 
			 (hash->fHashType == eKerberosHash || hash->fHashType == eX509DNHash) && 
			 ++iIndex < kMaxAltIdentities )
		{
			continue;
		}
		
		// just break the loop here since all other types don't have an array
		break;
		
	} while ( 1 );
	
bail:
	
	return bSuccess;
}
		
HashTable* HashTable_Create( const char *name, void *owner, eHashType hashType )
{
	// we don't use calloc due to static objects need clearing
	HashTable* result = (HashTable*) malloc( sizeof(HashTable) );
	assert( result != NULL );
	
	HashTable_Initialize( result, name, owner, hashType );
	result->fRefCount = 1;

	return result;
}

void HashTable_Initialize( HashTable *hash, const char *name, void *owner, eHashType hashType )
{
	static const struct rb_tree_ops id_rbt_ops = {
		.rbto_compare_nodes = rbt_compare_id_nodes,
		.rbto_compare_key   = rbt_compare_id_key
	};
	static const struct rb_tree_ops guid_rbt_ops = {
		.rbto_compare_nodes = rbt_compare_guid_nodes,
		.rbto_compare_key   = rbt_compare_guid_key
	};
	static const struct rb_tree_ops sid_rbt_ops = {
		.rbto_compare_nodes = rbt_compare_sid_nodes,
		.rbto_compare_key   = rbt_compare_sid_key
	};
	static const struct rb_tree_ops name_rbt_ops = {
		.rbto_compare_nodes = rbt_compare_name_nodes,
		.rbto_compare_key   = rbt_compare_name_key
	};
	static const struct rb_tree_ops kerberos_rbt_ops = {
		.rbto_compare_nodes = rbt_compare_kerberos_nodes,
		.rbto_compare_key   = rbt_compare_kerberos_key
	};
	static const struct rb_tree_ops x509dn_rbt_ops = {
		.rbto_compare_nodes = rbt_compare_x509dn_nodes,
		.rbto_compare_key   = rbt_compare_x509dn_key
	};
	
	bzero( hash, sizeof(HashTable) );
	
	hash->fRefCount = INT32_MAX;
	hash->fOwner = owner; // this is used to ensure a hashtable doesn't retain it's owner
	hash->fName = (name ? : "no name");
	hash->fQueue = dispatch_queue_create( hash->fName, NULL );
	hash->fHashType = hashType;
	
	switch( hashType )
	{
		case eIDHash:
			rb_tree_init( &hash->fRBtree, &id_rbt_ops );
			hash->fKeyOffset = __offsetof(struct UserGroup, fID);
			break;
			
		case eGUIDHash:
			rb_tree_init( &hash->fRBtree, &guid_rbt_ops );
			hash->fKeyOffset = __offsetof(struct UserGroup, fGUID);
			break;
			
		case eSIDHash:
			rb_tree_init( &hash->fRBtree, &sid_rbt_ops );
			hash->fKeyOffset = __offsetof(struct UserGroup, fSID);
			break;
			
		case eNameHash:
			rb_tree_init( &hash->fRBtree, &name_rbt_ops );
			hash->fKeyOffset = __offsetof(struct UserGroup, fName);
			break;
			
		case eKerberosHash:
			rb_tree_init( &hash->fRBtree, &kerberos_rbt_ops );
			hash->fKeyOffset = __offsetof(struct UserGroup, fKerberos);
			break;
			
		case eX509DNHash:
			rb_tree_init( &hash->fRBtree, &x509dn_rbt_ops );
			hash->fKeyOffset = __offsetof(struct UserGroup, fX509DN);
			break;
	}
}

void HashTable_Release( HashTable* hash )
{
	if ( dsReleaseObject(hash, &hash->fRefCount, false) == true )
	{
		HashTable_FreeContents( hash );
		free( hash );
	}
}

void HashTable_FreeContents( HashTable *hash )
{
	HashTable_Reset( hash );
	dispatch_release( hash->fQueue );
	hash->fQueue = NULL;
}

void HashTable_Reset( HashTable* hash )
{
	dispatch_sync( hash->fQueue, 
				   ^(void) {
					   struct rb_tree *tree = &hash->fRBtree;
					   struct rb_node *node = RB_TREE_MIN( tree );
						
					   // we have to delete after we iterate forward
					   while ( node != NULL ) {
						   struct rb_node	*delNode	= node;
						   struct UserGroup *entry		= RBNODE_TO_USERGROUP( delNode );
							
						   node = rb_tree_iterate( tree, node, RB_DIR_RIGHT );

						   // now let's remove the node
						   rb_tree_remove_node( tree, delNode );
						   DSFree( delNode );
						   
						   // release it if owner and entry are not the same (don't self release)
						   if ( entry != hash->fOwner ) {
							   UserGroup_Release( entry );
							   entry = NULL;
						   }
					   };
						
					   hash->fNumEntries = 0;
				   } );
}

int HashTable_ResetMemberships( HashTable *hash )
{
	__block int offlineCount = 0;
	
	dispatch_sync( hash->fQueue, 
				   ^(void) {
					   if ( hash->fNumEntries > 0 )
					   {
						   struct rb_tree *tree = &hash->fRBtree;
						   struct rb_node *node = RB_TREE_MIN( tree );
						   
						   while ( node != NULL ) {
							   struct rb_node	*delNode	= node;
							   struct UserGroup *entry		= RBNODE_TO_USERGROUP( delNode );
							   
							   // we have to delete after we iterate forward
							   node = rb_tree_iterate( tree, node, RB_DIR_RIGHT );

							   // check the node to be deleted
							   if ( entry->fNode == NULL || entry->fNodeAvailable == true ) {
								   
								   // safe to remove
								   rb_tree_remove_node( tree, delNode );
								   DSFree( delNode );
								   hash->fNumEntries--;
								   
								   // release it if owner and entry are not the same (don't self release)
								   if ( entry != hash->fOwner ) {
									   UserGroup_Release( entry );
									   entry = NULL;
								   }
							   }
							   else {
								   offlineCount++;
								   DbgLog( kLogInfo, "mbr_mig - Membership - RBtree membership reset - %s - %s (%d) - node %s - offline", 
										   hash->fName, entry->fName ? :"", entry->fID, entry->fNode ? : "no node" );
							   }
						   };
					   }
				    } );
	
	return offlineCount;
}

bool HashTable_Add( HashTable* hash, UserGroup* item, bool replaceExisting )
{
	if ( hash == NULL || item == NULL ) return false;
	
	__block bool bSuccess;
	
	dispatch_sync( hash->fQueue, 
				   ^(void) {
					   bSuccess = __HashTable_Add( hash, item, replaceExisting );
				   } );
	
	return bSuccess;
}

UserGroup* HashTable_GetAndRetain( HashTable* hash, const void* data )
{
	__block UserGroup	*entry	= NULL;
	
	if ( hash == NULL ) return NULL;
	
	dispatch_sync( hash->fQueue,
				   ^(void) {
					   struct rb_node *node = rb_tree_find_node( &hash->fRBtree, data );
					   if ( node != NULL ) {
						   entry = UserGroup_Retain( RBNODE_TO_USERGROUP(node) );
					   }
				   } );

	return entry;
}

void HashTable_Remove( HashTable* hash, UserGroup* item )
{
	if ( hash == NULL ) return;
	
	dispatch_sync( hash->fQueue,
				   ^(void) {
					   void *key = USERGROUP_TO_KEY(item, hash->fKeyOffset);
					   if (hash->fHashType == eNameHash) {
						   key = ((void **)key)[0];
					   } else {
						   assert(hash->fHashType != eKerberosHash);
						   assert(hash->fHashType != eX509DNHash);
					   }

					   struct rb_node *node = rb_tree_find_node( &hash->fRBtree, key );
					   if ( node != NULL ) {
						   UserGroup *tempItem = RBNODE_TO_USERGROUP( node );
						
						   // we only remove the exact entry because there could be conflicted authoritative entries
						   if ( tempItem == item ) {
							   
							   // safe to remove
							   rb_tree_remove_node( &hash->fRBtree, node );
							   DSFree( node );
							   
							   // release it if owner and entry are not the same (don't self release)
							   if ( tempItem != hash->fOwner ) {
								   UserGroup_Release( tempItem );
								   tempItem = NULL;
							   }
							   hash->fNumEntries--;
						   }
					   }
				   } );
}

void HashTable_Merge( HashTable* destination, HashTable* source )
{
	UserGroup **tempArray = NULL;
	
	int count = HashTable_CreateItemArray( source, &tempArray );
	if ( count > 0 )
	{
		DbgLog( kLogInfo, "mbr_mig - Membership - RBtree merge - %s - merging %X into %s (%X)", 
			    destination->fName, source, destination->fName, destination );
		
		dispatch_sync( destination->fQueue,
					   ^(void) {
						   int i;
							
						   for ( i = 0; i < count; i++ ) {
							   __HashTable_Add( destination, tempArray[i], true );
							   UserGroup_Release( tempArray[i] );
							   tempArray[i] = NULL;
						   }
							
						   free( tempArray );
					   } );
	}
}

int HashTable_CreateItemArray( HashTable *hash, UserGroup*** itemArray )
{
	__block UserGroup	**tempArray;
	__block int			numResults = 0;
	
	if ( hash == NULL || itemArray == NULL ) return 0;
	
	dispatch_sync( hash->fQueue,
				   ^(void) {
					   long numEntries = hash->fNumEntries;
					   
					   if ( numEntries > 0 )
					   {
						   (*itemArray) = tempArray = (UserGroup **) calloc( numEntries, sizeof(UserGroup *) );
						   assert( tempArray != NULL );
						   
						   struct rb_node *node;
						   struct rb_tree *rbtree = &hash->fRBtree;
						   
						   RB_TREE_FOREACH( node, rbtree ) {
							   struct UserGroup *ug = RBNODE_TO_USERGROUP( node );
							   tempArray[numResults++] = UserGroup_Retain( ug );
							   
							   // this should never happen, but a safety
							   assert( numResults <= numEntries );
						   }
					   }
				   } );
	
	return numResults;
}

#endif // DISABLE_SEARCH_PLUGIN
