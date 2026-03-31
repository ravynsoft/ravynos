/*
 * Copyright (c) 2009 Apple Inc. All rights reserved.
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
 * @header DirServicesConst
 * @discussion This header contains well known record, attribute and
 * authentication type constants plus others.
 * The attribute and record constants are generally used with the
 * API calls dsDataNodeAllocateString() and dsBuildListFromStrings()
 * to create proper data type arguments for the search methods in the
 * Directory Services API.
 * The auth constants are used with dsDataNodeAllocateString().
 */

#ifndef __DirServicesConst_h__
#define	__DirServicesConst_h__	1

/*!
 * @functiongroup DirectoryService Constants
 */

/*!
 * @defined kDSStdMachPortName
 * @discussion Registered name used with mach_init for DirectoryService daemon.
 */
#define		kDSStdMachPortName	"com.apple.DirectoryService"

/*!
 * @defined kDSStdMachDebugPortName
 * @discussion Registered name used with mach_init for DirectoryService debug daemon.
 */
#define		kDSStdMachDebugPortName	"com.apple.DirectoryServiceDebug"

/*!
 * @defined kDSStdMachLocalPortName
 * @discussion Registered name used with mach_init for DirectoryService local only daemon.
 */
#define		kDSStdMachLocalPortName	"com.apple.DirectoryService.localonly"

/*!
 * @defined kDSStdMachDSLookupPortName
 * @discussion Registered name used with mach_init for DirectoryService Lookup MIG server for the DirectoryService daemon.
 */
#define		kDSStdMachDSLookupPortName	"com.apple.system.DirectoryService.libinfo_v1"

#pragma mark -
#pragma mark Meta Record Type Constants
#pragma mark -

/*!
 * @functiongroup DirectoryService Meta Record Type Constants
 */
 
/*!
 * @defined kDSRecordsAll
 * @discussion Used to indicate that all records should be returned of a certain
 * type as opposed to an actual pattern match.
 */
#define		kDSRecordsAll							"dsRecordsAll"

/*!
 * @defined kDSRecordsStandardAll
 * @discussion Retained only for backward compatibility.
 */
#define		kDSRecordsStandardAll					"dsRecordsStandardAll"

/*!
 * @defined kDSRecordsNativeAll
 * @discussion Retained only for backward compatibility.
 */
#define		kDSRecordsNativeAll						"dsRecordsNativeAll"

/*!
 * @defined kDSStdRecordTypePrefix
 * @discussion Used as the prefix for all standard record types.
 */
#define		kDSStdRecordTypePrefix					"dsRecTypeStandard:"

/*!
 * @defined kDSNativeRecordTypePrefix
 * @discussion Prefix used to identify a native record type.
 */
#define		kDSNativeRecordTypePrefix				"dsRecTypeNative:"

/*!
 * @defined kDSStdRecordTypeAll
 * @discussion Used to convey that all record types need to be searched over.
 */
#define		kDSStdRecordTypeAll						"dsRecTypeStandard:All"

/*!
 * @defined kDSStdUserNamesMeta
 * @discussion Retained only for backward compatibility.
 */
#define		kDSStdUserNamesMeta						"dsRecTypeStandard:MetaUserNames"

/*!
 * @defined kDSStdRecordTypePlugins
 * @discussion Identifies records that represent specific DS plugin data.
 */
#define		kDSStdRecordTypePlugins					"dsRecTypeStandard:Plugins"

/*!
 * @defined kDSStdRecordTypeRefTableEntries
 * @discussion Identifies records that represent a DS reference table entry.
 */
#define		kDSStdRecordTypeRefTableEntries			"dsRecTypeStandard:RefTableEntries"

/*!
 * @defined kDSStdRecordTypeRecordTypes
 * @discussion Identifies records that represent each possible record type.
 */
#define		kDSStdRecordTypeRecordTypes				"dsRecTypeStandard:RecordTypes"

/*!
 * @defined kDSStdRecordTypeAttributeTypes
 * @discussion Identifies records that represent each possible attribute type.
 */
#define		kDSStdRecordTypeAttributeTypes			"dsRecTypeStandard:AttributeTypes"

#pragma mark -
#pragma mark Specific Record Type Constants
#pragma mark -

/*!
 * @functiongroup DirectoryService Specific Record Type Constants
 */
 
/*!
 * @defined kDSStdRecordTypeAccessControls
 * @discussion Record type that contains directory access control directives.
 */
#define		kDSStdRecordTypeAccessControls			"dsRecTypeStandard:AccessControls"

/*!
 * @defined kDSStdRecordTypeAFPServer
 * @discussion Record type of AFP server records.
 */
#define		kDSStdRecordTypeAFPServer				"dsRecTypeStandard:AFPServer"

/*!
 * @defined kDSStdRecordTypeAFPUserAliases
 * @discussion Record type of AFP user aliases used exclusively by AFP processes.
 */
#define		kDSStdRecordTypeAFPUserAliases			"dsRecTypeStandard:AFPUserAliases"

/*!
 * @defined kDSStdRecordTypeAliases
 * @discussion Used to represent alias records.
 */
#define		kDSStdRecordTypeAliases					"dsRecTypeStandard:Aliases"

/*!
 * @defined kDSStdRecordTypeAugments
 * @discussion Used to store augmented record data.
 */
#define		kDSStdRecordTypeAugments				"dsRecTypeStandard:Augments"

/*!
 * @defined kDSStdRecordTypeAutomount
 * @discussion Used to store automount record data.
 */
#define		kDSStdRecordTypeAutomount				"dsRecTypeStandard:Automount"

/*!
 * @defined kDSStdRecordTypeAutomountMap
 * @discussion Used to store automountMap record data.
 */
#define		kDSStdRecordTypeAutomountMap			"dsRecTypeStandard:AutomountMap"

/*!
 * @defined kDSStdRecordTypeAutoServerSetup
 * @discussion Used to discover automated server setup information.
 */
#define		kDSStdRecordTypeAutoServerSetup			"dsRecTypeStandard:AutoServerSetup"

/*!
 * @defined kDSStdRecordTypeBootp
 * @discussion Record in the local node for storing bootp info.
 */
#define		kDSStdRecordTypeBootp					"dsRecTypeStandard:Bootp"

/*!
 * @defined kDSStdRecordTypeCertificateAuthority
 * @discussion Record type that contains certificate authority information.
 */
#define		kDSStdRecordTypeCertificateAuthorities  "dsRecTypeStandard:CertificateAuthorities"

/*!
 * @defined kDSStdRecordTypeComputerLists
 * @discussion Identifies computer list records.
 */
#define		kDSStdRecordTypeComputerLists			"dsRecTypeStandard:ComputerLists"

/*!
 * @defined kDSStdRecordTypeComputerGroups
 * @discussion Identifies computer group records.
 */
#define		kDSStdRecordTypeComputerGroups			"dsRecTypeStandard:ComputerGroups"

/*!
 * @defined kDSStdRecordTypeComputers
 * @discussion Identifies computer records.
 */
#define		kDSStdRecordTypeComputers				"dsRecTypeStandard:Computers"

/*!
 * @defined kDSStdRecordTypeConfig
 * @discussion Identifies config records.
 */
#define		kDSStdRecordTypeConfig					"dsRecTypeStandard:Config"

/*!
 * @defined kDSStdRecordTypeEthernets
 * @discussion Record in the local node for storing ethernets.
 */
#define		kDSStdRecordTypeEthernets				"dsRecTypeStandard:Ethernets"

/*!
 * @defined kDSStdRecordTypeFileMakerServers
 * @discussion FileMaker servers record type. Describes available FileMaker servers, 
 *  used for service discovery.
 */
#define		kDSStdRecordTypeFileMakerServers		"dsRecTypeStandard:FileMakerServers"

/*!
 * @defined kDSStdRecordTypeFTPServer
 * @discussion Identifies ftp server records.
 */
#define		kDSStdRecordTypeFTPServer				"dsRecTypeStandard:FTPServer"

/*!
 * @defined kDSStdRecordTypeGroupAliases
 * @discussion No longer supported in Mac OS X 10.4 or later.
 */
#define		kDSStdRecordTypeGroupAliases	"dsRecTypeStandard:GroupAliases"

/*!
 * @defined kDSStdRecordTypeGroups
 * @discussion Identifies group records.
 */
#define		kDSStdRecordTypeGroups					"dsRecTypeStandard:Groups"

/*!
 * @defined kDSStdRecordTypeHostServices
 * @discussion Record in the local node for storing host services.
 */
#define		kDSStdRecordTypeHostServices			"dsRecTypeStandard:HostServices"

/*!
 * @defined kDSStdRecordTypeHosts
 * @discussion Identifies host records.
 */
#define		kDSStdRecordTypeHosts					"dsRecTypeStandard:Hosts"

/*!
 * @defined kDSStdRecordTypeLDAPServer
 * @discussion Identifies LDAP server records.
 */
#define		kDSStdRecordTypeLDAPServer				"dsRecTypeStandard:LDAPServer"

/*!
 * @defined kDSStdRecordTypeLocations
 * @discussion Location record type.
 */
#define		kDSStdRecordTypeLocations				"dsRecTypeStandard:Locations"

/*!
 * @defined kDSStdRecordTypeMachines
 * @discussion Identifies machine records.
 */
#define		kDSStdRecordTypeMachines				"dsRecTypeStandard:Machines"

/*!
 * @defined kDSStdRecordTypeMeta
 * @discussion Identifies meta records.
 */
#define		kDSStdRecordTypeMeta					"dsRecTypeStandard:AppleMetaRecord"

/*!
 * @defined kDSStdRecordTypeMounts
 * @discussion Identifies mount records.
 */
#define		kDSStdRecordTypeMounts					"dsRecTypeStandard:Mounts"

/*!
 * @defined kDSStdRecordTypMounts
 * @discussion Supported only for backward compatibility to kDSStdRecordTypeMounts.
 */
#define		kDSStdRecordTypMounts					"dsRecTypeStandard:Mounts"

/*!
 * @defined kDSStdRecordTypeNeighborhoods
 * @discussion Neighborhood record type. Describes a list of computers and other
 *  neighborhoods, used for network browsing.
 */
#define		kDSStdRecordTypeNeighborhoods			"dsRecTypeStandard:Neighborhoods"

/*!
 * @defined kDSStdRecordTypeNFS
 * @discussion Identifies NFS records.
 */
#define		kDSStdRecordTypeNFS						"dsRecTypeStandard:NFS"

/*!
 * @defined kDSStdRecordTypeNetDomains
 * @discussion Record in the local node for storing net domains.
 */
#define		kDSStdRecordTypeNetDomains				"dsRecTypeStandard:NetDomains"

/*!
 * @defined kDSStdRecordTypeNetGroups
 * @discussion Record in the local node for storing net groups.
 */
#define		kDSStdRecordTypeNetGroups				"dsRecTypeStandard:NetGroups"

/*!
 * @defined kDSStdRecordTypeNetworks
 * @discussion Identifies network records.
 */
#define		kDSStdRecordTypeNetworks				"dsRecTypeStandard:Networks"

/*!
 * @defined kDSStdRecordTypePasswordServer
 * @discussion Used to discover password servers via Bonjour.
 */
#define		kDSStdRecordTypePasswordServer			"dsRecTypeStandard:PasswordServer"

/*!
 * @defined kDSStdRecordTypePeople
 * @discussion Record type that contains "People" records used for contact information.
 */
#define		kDSStdRecordTypePeople					"dsRecTypeStandard:People"

/*!
 * @defined kDSStdRecordTypePresetComputers
 * @discussion The computer record type used for presets in record creation.
 */
#define		kDSStdRecordTypePresetComputers			"dsRecTypeStandard:PresetComputers"

/*!
 * @defined kDSStdRecordTypePresetComputerGroups
 * @discussion The computer group record type used for presets in record creation.
 */
#define		kDSStdRecordTypePresetComputerGroups	"dsRecTypeStandard:PresetComputerGroups"

/*!
 * @defined kDSStdRecordTypePresetComputerLists
 * @discussion The computer list record type used for presets in record creation.
 */
#define		kDSStdRecordTypePresetComputerLists		"dsRecTypeStandard:PresetComputerLists"

/*!
 * @defined kDSStdRecordTypePresetGroups
 * @discussion The group record type used for presets in record creation.
 */
#define		kDSStdRecordTypePresetGroups			"dsRecTypeStandard:PresetGroups"

/*!
 * @defined kDSStdRecordTypePresetUsers
 * @discussion The user record type used for presets in record creation.
 */
#define		kDSStdRecordTypePresetUsers				"dsRecTypeStandard:PresetUsers"

/*!
 * @defined kDSStdRecordTypePrintService
 * @discussion Identifies print service records.
 */
#define		kDSStdRecordTypePrintService			"dsRecTypeStandard:PrintService"

/*!
 * @defined kDSStdRecordTypePrintServiceUser
 * @discussion Record in the local node for storing quota usage for a user.
 */
#define		kDSStdRecordTypePrintServiceUser		"dsRecTypeStandard:PrintServiceUser"

/*!
 * @defined kDSStdRecordTypePrinters
 * @discussion Identifies printer records.
 */
#define		kDSStdRecordTypePrinters				"dsRecTypeStandard:Printers"

/*!
 * @defined kDSStdRecordTypeProtocols
 * @discussion Identifies protocol records.
 */
#define		kDSStdRecordTypeProtocols				"dsRecTypeStandard:Protocols"

/*!
 * @defined kDSStdRecordTypProtocols
 * @discussion Supported only for backward compatibility to kDSStdRecordTypeProtocols.
 */
#define		kDSStdRecordTypProtocols				"dsRecTypeStandard:Protocols"

/*!
 * @defined kDSStdRecordTypeQTSServer
 * @discussion Identifies quicktime streaming server records.
 */
#define		kDSStdRecordTypeQTSServer				"dsRecTypeStandard:QTSServer"

/*!
 * @defined kDSStdRecordTypeResources
 * @discussion Identifies resources used in group services.
 */
#define		kDSStdRecordTypeResources				"dsRecTypeStandard:Resources"

/*!
 * @defined kDSStdRecordTypeRPC
 * @discussion Identifies remote procedure call records.
 */
#define		kDSStdRecordTypeRPC						"dsRecTypeStandard:RPC"

/*!
 * @defined kDSStdRecordTypRPC
 * @discussion Supported only for backward compatibility to kDSStdRecordTypeRPC.
 */
#define		kDSStdRecordTypRPC						"dsRecTypeStandard:RPC"

/*!
 * @defined kDSStdRecordTypeSMBServer
 * @discussion Identifies SMB server records.
 */
#define		kDSStdRecordTypeSMBServer				"dsRecTypeStandard:SMBServer"

/*!
 * @defined kDSStdRecordTypeServer
 * @discussion Identifies generic server records.
 */
#define		kDSStdRecordTypeServer					"dsRecTypeStandard:Server"

/*!
 * @defined kDSStdRecordTypeServices
 * @discussion Identifies directory based service records.
 */
#define		kDSStdRecordTypeServices				"dsRecTypeStandard:Services"

/*!
 * @defined kDSStdRecordTypeSharePoints
 * @discussion Share point record type.
 */
#define		kDSStdRecordTypeSharePoints				"dsRecTypeStandard:SharePoints"

/*!
 * @defined kDSStdRecordTypeUserAliases
 * @discussion No longer supported in Mac OS X 10.4 or later.
 */
#define		kDSStdRecordTypeUserAliases				"dsRecTypeStandard:UserAliases"

/*!
 * @defined kDSStdRecordTypeUsers
 * @discussion Identifies user records.
 */
#define		kDSStdRecordTypeUsers					"dsRecTypeStandard:Users"

/*!
 * @defined kDSStdRecordTypeWebServer
 * @discussion Identifies web server records.
 */
#define		kDSStdRecordTypeWebServer				"dsRecTypeStandard:WebServer"

#pragma mark -
#pragma mark Meta Attribute Type Constants
#pragma mark -

/*!
 * @functiongroup DirectoryService Meta Attribute Type Constants
 */
 
/*!
 * @defined kDSAttributesAll
 * @discussion Used in requesting all attribute types in a search.
 */
#define		kDSAttributesAll						"dsAttributesAll"

/*!
 * @defined kDSAttributesStandardAll
 * @discussion Used in requesting all standard attribute types in a search.
 */
#define		kDSAttributesStandardAll				"dsAttributesStandardAll"

/*!
 * @defined kDSAttributesNativeAll
 * @discussion Used in requesting all native attribute types in a search.
 */
#define		kDSAttributesNativeAll					"dsAttributesNativeAll"

/*!
 * @defined kDSStdAttrTypePrefix
 * @discussion Prefix used to identify all standard attribute types.
 */
#define		kDSStdAttrTypePrefix					"dsAttrTypeStandard:"

/*!
 * @defined kDSNativeAttrTypePrefix
 * @discussion Prefix used to identify directory native attribute types.
 */
#define		kDSNativeAttrTypePrefix					"dsAttrTypeNative:"

/*!
 * @defined kDSAttrNone
 * @discussion Retained for backward compatibility.
 */
#define		kDSAttrNone								"dsNone"

#pragma mark -
#pragma mark Specific Attribute Type Constants
#pragma mark -

/*!
 * @functiongroup DirectoryService Specific Attribute Type Constants
 * As a guideline for the attribute types the following legend is used:
 *
 *		eDS1xxxxxx  Single Valued Attribute
 *
 *		eDSNxxxxxx  Multi-Valued Attribute
 *
 *	NOTE #1: Access controls may prevent any particular client from reading/writting
 *			various attribute types.  In addition some attribute types may not be stored at
 *			all and could also represent "real-time" data generated by the directory node
 *			plug-in.
 *
 *	NOTE #2: Attributes in the model are available for records and directory nodes.
 */
 
#pragma mark -
#pragma mark Single Valued Specific Attribute Type Constants
#pragma mark -

/*!
 * @functiongroup DirectoryService Single Valued Specific Attribute Type Constants
 */

/*!
 *	@defined kDS1AttrAdminLimits
 *	@discussion XML plist indicating what an admin user can edit.
 *		Found in kDSStdRecordTypeUsers records.
 */
#define		kDS1AttrAdminLimits						"dsAttrTypeStandard:AdminLimits"

/*!
 * @defined kDS1AttrAliasData
 * @discussion Used to identify alias data.
 */
#define		kDS1AttrAliasData						"dsAttrTypeStandard:AppleAliasData"

/*!
 * @defined kDS1AttrAlternateDatastoreLocation
 * @discussion Unix path used for determining where a user's email is stored.
 */
#define		kDS1AttrAlternateDatastoreLocation		"dsAttrTypeStandard:AlternateDatastoreLocation"

/*!
 * @defined kDS1AttrAuthenticationHint
 * @discussion Used to identify the authentication hint phrase.
 */
#define		kDS1AttrAuthenticationHint				"dsAttrTypeStandard:AuthenticationHint"

/*!
 * @defined kDSNAttrAttributeTypes
 * @discussion Used to indicated recommended attribute types for a record type in the Config node.
 */
#define		kDSNAttrAttributeTypes                  "dsAttrTypeStandard:AttributeTypes"

/*!
 * @defined kDS1AttrAuthorityRevocationList
 * @discussion Attribute containing the binary of the authority revocation list.
 * A certificate revocation list that defines certificate authority certificates
 * which are no longer trusted.  No user certificates are included in this list.
 * Usually found in kDSStdRecordTypeCertificateAuthority records.
 */
#define		kDS1AttrAuthorityRevocationList			"dsAttrTypeStandard:AuthorityRevocationList"

/*!
 * @defined kDS1AttrBirthday
 * @discussion Single-valued attribute that defines the user's birthday.
 * Format is x.208 standard YYYYMMDDHHMMSSZ which we will require as GMT time.
 */
#define		kDS1AttrBirthday						"dsAttrTypeStandard:Birthday"


/*!
 * @defined kDS1AttrBootFile
 * @discussion Attribute type in host or machine records for the name of the 
 *		kernel that this machine will use by default when NetBooting.
 */
#define		kDS1AttrBootFile						"dsAttrTypeStandard:BootFile"

/*!
 * @defined kDS1AttrCACertificate
 * @discussion Attribute containing the binary of the certificate of a
 * certificate authority. Its corresponding private key is used to sign certificates.
 * Usually found in kDSStdRecordTypeCertificateAuthority records.
 */
#define		kDS1AttrCACertificate					"dsAttrTypeStandard:CACertificate"

/*!
 * @defined kDS1AttrCapabilities
 * @discussion Used with directory nodes so that clients can "discover" the
 * API capabilities for this Directory Node.
 */
#define		kDS1AttrCapabilities					"dsAttrTypeStandard:Capabilities"

/*!
 * @defined kDS1AttrCapacity
 * @discussion Attribute type for the capacity of a resource. 
 *	 found in resource records (kDSStdRecordTypeResources). 
 *	Example: 50
 */
#define		kDS1AttrCapacity					"dsAttrTypeStandard:Capacity"

/*!
 *	@defined kDS1AttrCategory
 *	@discussion The category of an item used for browsing
 */
#define		kDS1AttrCategory						"dsAttrTypeStandard:Category"

/*!
 * @defined kDS1AttrCertificateRevocationList
 * @discussion Attribute containing the binary of the certificate revocation list.
 * This is a list of certificates which are no longer trusted.
 * Usually found in kDSStdRecordTypeCertificateAuthority records.
 */
#define		kDS1AttrCertificateRevocationList		"dsAttrTypeStandard:CertificateRevocationList"

/*!
 * @defined kDS1AttrChange
 * @discussion Retained for backward compatibility.
 */
#define		kDS1AttrChange							"dsAttrTypeStandard:Change"

/*!
 * @defined kDS1AttrComment
 * @discussion Attribute used for unformatted comment.
 */
#define		kDS1AttrComment							"dsAttrTypeStandard:Comment"

/*!
 * @defined kDS1AttrContactGUID
 * @discussion Attribute type for the contact GUID of a group. 
 *	 found in group records (kDSStdRecordTypeGroups). 
 */
#define		kDS1AttrContactGUID					"dsAttrTypeStandard:ContactGUID"

/*!
 * @defined kDS1AttrContactPerson
 * @discussion Attribute type for the contact person of the machine. 
 *		Found in host or machine records.
 */
#define		kDS1AttrContactPerson					"dsAttrTypeStandard:ContactPerson"

/*!
 * @defined kDS1AttrCreationTimestamp
 * @discussion Attribute showing date/time of record creation.
 * Format is x.208 standard YYYYMMDDHHMMSSZ which we will require as GMT time.
 */
#define		kDS1AttrCreationTimestamp				"dsAttrTypeStandard:CreationTimestamp"

/*!
 * @defined kDS1AttrCrossCertificatePair
 * @discussion Attribute containing the binary of a pair of certificates which 
 * verify each other.  Both certificates have the same level of authority.
 * Usually found in kDSStdRecordTypeCertificateAuthority records.
 */
#define		kDS1AttrCrossCertificatePair			"dsAttrTypeStandard:CrossCertificatePair"

/*!
 * @defined kDS1AttrDataStamp
 * @discussion checksum/meta data
 */
#define		kDS1AttrDataStamp						"dsAttrTypeStandard:DataStamp"

/*!
 * @defined kDS1AttrDistinguishedName
 * @discussion Users distinguished or real name
 */
#define		kDS1AttrDistinguishedName				"dsAttrTypeStandard:RealName"

/*!
 * @defined kDS1AttrDNSDomain
 * @discussion DNS Resolver domain attribute.
 */
#define		kDS1AttrDNSDomain						"dsAttrTypeStandard:DNSDomain"

/*!
 * @defined kDS1AttrDNSNameServer
 * @discussion DNS Resolver nameserver attribute.
 */
#define		kDS1AttrDNSNameServer					"dsAttrTypeStandard:DNSNameServer"

/*!
 * @defined kDS1AttrENetAddress
 * @discussion Single-valued attribute for hardware Ethernet address (MAC address).
 *		Found in machine records (kDSStdRecordTypeMachines) and computer records
 *		(kDSStdRecordTypeComputers).
 */
#define		kDS1AttrENetAddress						"dsAttrTypeStandard:ENetAddress"

/*!
 * @defined kDS1AttrExpire
 * @discussion Used for expiration date or time depending on association.
 */
#define		kDS1AttrExpire							"dsAttrTypeStandard:Expire"

/*!
 * @defined kDS1AttrFirstName
 * @discussion Used for first name of user or person record.
 */
#define		kDS1AttrFirstName						"dsAttrTypeStandard:FirstName"

/*!
 * @defined kDS1AttrGeneratedUID
 * @discussion Used for 36 character (128 bit) unique ID. Usually found in user, 
 * group, and computer records. An example value is "A579E95E-CDFE-4EBC-B7E7-F2158562170F".
 * The standard format contains 32 hex characters and four hyphen characters.
 */
#define		kDS1AttrGeneratedUID					"dsAttrTypeStandard:GeneratedUID"

/*!
 *	@defined kDS1AttrHomeDirectoryQuota
 *	@discussion Represents the allowed usage for a user's home directory in bytes.
 *		Found in user records (kDSStdRecordTypeUsers).
 */
#define		kDS1AttrHomeDirectoryQuota				"dsAttrTypeStandard:HomeDirectoryQuota"

/*!
 * @defined kDS1AttrHomeDirectorySoftQuota
 * @discussion Used to define home directory size limit in bytes when user is notified
 * that the hard limit is approaching.
 */
#define		kDS1AttrHomeDirectorySoftQuota			"dsAttrTypeStandard:HomeDirectorySoftQuota"

/*!
 *	@defined kDS1AttrHomeLocOwner
 *	@discussion Represents the owner of a workgroup's shared home directory.
 *		Typically found in kDSStdRecordTypeGroups records.
 */
#define		kDS1AttrHomeLocOwner					"dsAttrTypeStandard:HomeLocOwner"

/*!
 *	@defined kDS1StandardAttrHomeLocOwner
 *	@discussion Retained for backward compatibility.
 */
#define		kDS1StandardAttrHomeLocOwner			kDS1AttrHomeLocOwner

/*!
 * @defined kDS1AttrInternetAlias
 * @discussion Used to track internet alias.
 */
#define		kDS1AttrInternetAlias					"dsAttrTypeStandard:InetAlias"

/*!
 * @defined kDS1AttrKDCConfigData
 * @discussion Contents of the kdc.conf file.
 */
#define		kDS1AttrKDCConfigData					"dsAttrTypeStandard:KDCConfigData"

/*!
 * @defined kDS1AttrLastName
 * @discussion Used for the last name of user or person record.
 */
#define		kDS1AttrLastName						"dsAttrTypeStandard:LastName"

/*!
 * @defined kDS1AttrLDAPSearchBaseSuffix
 * @discussion Search base suffix for a LDAP server.
 */
#define		kDS1AttrLDAPSearchBaseSuffix					"dsAttrTypeStandard:LDAPSearchBaseSuffix"

/*!
 * @defined kDS1AttrLocation
 * @discussion Represents the location a service is available from (usually domain name).
 *     Typically found in service record types including kDSStdRecordTypeAFPServer,
 *     kDSStdRecordTypeLDAPServer, and kDSStdRecordTypeWebServer.
 */
#define		kDS1AttrLocation						"dsAttrTypeStandard:Location"

/*!
 * @defined kDS1AttrMapGUID
 * @discussion Represents the GUID for a record's map.
 */
#define		kDS1AttrMapGUID						"dsAttrTypeStandard:MapGUID"

/*!
 * @defined kDS1AttrMCXFlags
 * @discussion Used by MCX.
 */
#define		kDS1AttrMCXFlags						"dsAttrTypeStandard:MCXFlags"

/*!
 * @defined kDS1AttrMCXSettings
 * @discussion Used by MCX.
 */
#define		kDS1AttrMCXSettings						"dsAttrTypeStandard:MCXSettings"

/*!
 * @defined kDS1AttrMailAttribute
 * @discussion Holds the mail account config data.
 */
#define		kDS1AttrMailAttribute					"dsAttrTypeStandard:MailAttribute"

/*!
 * @defined kDS1AttrMetaAutomountMap
 * @discussion Used to query for kDSStdRecordTypeAutomount entries associated with a specific 
 * kDSStdRecordTypeAutomountMap.
 */
#define		kDS1AttrMetaAutomountMap				"dsAttrTypeStandard:MetaAutomountMap"

/*!
 * @defined kDS1AttrMiddleName
 * @discussion Used for the middle name of user or person record.
 */
#define		kDS1AttrMiddleName						"dsAttrTypeStandard:MiddleName"

/*!
 * @defined kDS1AttrModificationTimestamp
 * @discussion Attribute showing date/time of record modification.
 * Format is x.208 standard YYYYMMDDHHMMSSZ which we will require as GMT time.
 */
#define		kDS1AttrModificationTimestamp			"dsAttrTypeStandard:ModificationTimestamp"

/*!
 * @defined kDSNAttrNeighborhoodAlias
 * @discussion Attribute type in Neighborhood records describing sub-neighborhood records.
 */
#define		kDSNAttrNeighborhoodAlias				"dsAttrTypeStandard:NeighborhoodAlias"

/*!
 * @defined kDS1AttrNeighborhoodType
 * @discussion Attribute type in Neighborhood records describing their function.
 */
#define		kDS1AttrNeighborhoodType				"dsAttrTypeStandard:NeighborhoodType"

/*!
 * @defined kDS1AttrNetworkView
 * @discussion The name of the managed network view a computer should use for browsing.
 */
#define		kDS1AttrNetworkView						"dsAttrTypeStandard:NetworkView"

/*!
 * @defined kDS1AttrNFSHomeDirectory
 * @discussion Defines a user's home directory mount point on the local machine.
 */
#define		kDS1AttrNFSHomeDirectory				"dsAttrTypeStandard:NFSHomeDirectory"

/*!
 * @defined kDS1AttrNote
 * @discussion Note attribute. Commonly used in printer records.
 */
#define		kDS1AttrNote							"dsAttrTypeStandard:Note"

/*!
 * @defined kDS1AttrOwner
 * @discussion Attribute type for the owner of a record. 
 *		Typically the value is a LDAP distinguished name.
 */
#define		kDS1AttrOwner							"dsAttrTypeStandard:Owner"

/*!
 * @defined kDS1AttrOwnerGUID
 * @discussion Attribute type for the owner GUID of a group. 
 *	 found in group records (kDSStdRecordTypeGroups). 
 */
#define		kDS1AttrOwnerGUID					"dsAttrTypeStandard:OwnerGUID"

/*!
 * @defined kDS1AttrPassword
 * @discussion Holds the password or credential value.
 */
#define		kDS1AttrPassword						"dsAttrTypeStandard:Password"

/*!
 * @defined kDS1AttrPasswordPlus
 * @discussion Holds marker data to indicate possible authentication redirection.
 */
#define		kDS1AttrPasswordPlus					"dsAttrTypeStandard:PasswordPlus"

/*!
 * @defined kDS1AttrPasswordPolicyOptions
 * @discussion Collection of password policy options in single attribute.
 * Used in user presets record.
 */
#define		kDS1AttrPasswordPolicyOptions			"dsAttrTypeStandard:PasswordPolicyOptions"

/*!
 * @defined kDS1AttrPasswordServerList
 * @discussion Represents the attribute for storing the password server's replication information.
 */
#define		kDS1AttrPasswordServerList				"dsAttrTypeStandard:PasswordServerList"

/*!
 *	@defined kDS1AttrPasswordServerLocation
 *	@discussion Specifies the IP address or domain name of the Password Server associated
 *		with a given directory node. Found in a config record named PasswordServer.
 */
#define		kDS1AttrPasswordServerLocation			"dsAttrTypeStandard:PasswordServerLocation"

/*!
 * @defined kDS1AttrPicture
 * @discussion Represents the path of the picture for each user displayed in the login window.
 * Found in user records (kDSStdRecordTypeUsers).
 */
#define		kDS1AttrPicture							"dsAttrTypeStandard:Picture"

/*!
 * @defined kDS1AttrPort
 * @discussion Represents the port number a service is available on.
 *     Typically found in service record types including kDSStdRecordTypeAFPServer,
 *     kDSStdRecordTypeLDAPServer, and kDSStdRecordTypeWebServer.
 */
#define		kDS1AttrPort							"dsAttrTypeStandard:Port"

/*!
 *	@defined kDS1AttrPresetUserIsAdmin
 *	@discussion Flag to indicate whether users created from this preset are administrators
 *		by default. Found in kDSStdRecordTypePresetUsers records.
 */
#define		kDS1AttrPresetUserIsAdmin				"dsAttrTypeStandard:PresetUserIsAdmin"

/*!
 * @defined kDS1AttrPrimaryComputerGUID
 * @discussion Single-valued attribute that defines a primary computer of the computer group.  
 * added via extensible object for computer group record type (kDSStdRecordTypeComputerGroups)
 */
#define     kDS1AttrPrimaryComputerGUID                   "dsAttrTypeStandard:PrimaryComputerGUID"

/*!
 * @defined kDS1AttrPrimaryComputerList
 * @discussion The GUID of the computer list with which this computer record is associated.
 */
#define		kDS1AttrPrimaryComputerList             "dsAttrTypeStandard:PrimaryComputerList"

/*!
 * @defined kDS1AttrPrimaryGroupID
 * @discussion This is the 32 bit unique ID that represents the primary group 
 * a user is part of, or the ID of a group. Format is a signed 32 bit integer
 * represented as a string.
 */
#define		kDS1AttrPrimaryGroupID					"dsAttrTypeStandard:PrimaryGroupID"

/*!
 * @defined kDS1AttrPrinter1284DeviceID
 * @discussion Single-valued attribute that defines the IEEE 1284 DeviceID of a printer.
 *              This is used when configuring a printer.
 */
#define		kDS1AttrPrinter1284DeviceID				"dsAttrTypeStandard:Printer1284DeviceID"

/*!
 * @defined kDS1AttrPrinterLPRHost
 * @discussion Standard attribute type for kDSStdRecordTypePrinters.
 */
#define		kDS1AttrPrinterLPRHost					"dsAttrTypeStandard:PrinterLPRHost"

/*!
 * @defined kDS1AttrPrinterLPRQueue
 * @discussion Standard attribute type for kDSStdRecordTypePrinters.
 */
#define		kDS1AttrPrinterLPRQueue					"dsAttrTypeStandard:PrinterLPRQueue"

/*!
 * @defined kDS1AttrPrinterMakeAndModel
 * @discussion Single-valued attribute for definition of the Printer Make and Model.  An example
 *              Value would be "HP LaserJet 2200".  This would be used to determine the proper PPD
 *              file to be used when configuring a printer from the Directory.  This attribute
 *              is based on the IPP Printing Specification RFC and IETF IPP-LDAP Printer Record.
 */
#define		kDS1AttrPrinterMakeAndModel				"dsAttrTypeStandard:PrinterMakeAndModel"

/*!
 * @defined kDS1AttrPrinterType
 * @discussion Standard attribute type for kDSStdRecordTypePrinters.
 */
#define		kDS1AttrPrinterType						"dsAttrTypeStandard:PrinterType"

/*!
 * @defined kDS1AttrPrinterURI
 * @discussion Single-valued attribute that defines the URI of a printer "ipp://address" or
 *              "smb://server/queue".  This is used when configuring a printer. This attribute
 *				is based on the IPP Printing Specification RFC and IETF IPP-LDAP Printer Record.
 */
#define		kDS1AttrPrinterURI                      "dsAttrTypeStandard:PrinterURI"

/*!
 * @defined kDS1AttrPrinterXRISupported
 * @discussion Multi-valued attribute that defines additional URIs supported by a printer.
 *              This is used when configuring a printer. This attribute is based on the IPP 
 *				Printing Specification RFC and IETF IPP-LDAP Printer Record.
 */
#define		kDSNAttrPrinterXRISupported				"dsAttrTypeStandard:PrinterXRISupported"

/*!
 * @defined kDS1AttrPrintServiceInfoText
 * @discussion Standard attribute type for kDSStdRecordTypePrinters.
 */
#define		kDS1AttrPrintServiceInfoText			"dsAttrTypeStandard:PrintServiceInfoText"

/*!
 * @defined kDS1AttrPrintServiceInfoXML
 * @discussion Standard attribute type for kDSStdRecordTypePrinters.
 */
#define		kDS1AttrPrintServiceInfoXML				"dsAttrTypeStandard:PrintServiceInfoXML"

/*!
 * @defined kDS1AttrPrintServiceUserData
 * @discussion Single-valued attribute for print quota configuration or statistics
 *		(XML data). Found in user records (kDSStdRecordTypeUsers) or print service
 *		statistics records (kDSStdRecordTypePrintServiceUser).
 */
#define		kDS1AttrPrintServiceUserData			"dsAttrTypeStandard:PrintServiceUserData"

/*!
 * @defined kDS1AttrRealUserID
 * @discussion Used by MCX.
 */
#define		kDS1AttrRealUserID						"dsAttrTypeStandard:RealUserID"

/*!
 * @defined kDS1AttrRelativeDNPrefix
 * @discussion Used to map the first native LDAP attribute type required in the building of the
 *  Relative Distinguished Name for LDAP record creation.
 */
#define		kDS1AttrRelativeDNPrefix				"dsAttrTypeStandard:RelativeDNPrefix"

/*!
 * @defined kDS1AttrSMBAcctFlags
 * @discussion Account control flag.
 */
#define		kDS1AttrSMBAcctFlags					"dsAttrTypeStandard:SMBAccountFlags"

/*!
 * @defined kDS1AttrSMBGroupRID
 * @discussion Constant for supporting PDC SMB interaction with DS.
 */
#define 	kDS1AttrSMBGroupRID						"dsAttrTypeStandard:SMBGroupRID"

/*!
 * @defined kDS1AttrSMBHome
 * @discussion
 *     UNC address of Windows homedirectory mount point (\\server\\sharepoint).
 */
#define 	kDS1AttrSMBHome							"dsAttrTypeStandard:SMBHome"

/*!
 * @defined kDS1AttrSMBHomeDrive
 * @discussion
 *     Drive letter for homedirectory mount point.
 */
#define 	kDS1AttrSMBHomeDrive					"dsAttrTypeStandard:SMBHomeDrive"

/*!
 * @defined kDS1AttrSMBKickoffTime
 * @discussion Attribute in support of SMB interaction.
 */
#define		kDS1AttrSMBKickoffTime					"dsAttrTypeStandard:SMBKickoffTime"

/*!
 * @defined kDS1AttrSMBLogoffTime
 * @discussion Attribute in support of SMB interaction.
 */
#define		kDS1AttrSMBLogoffTime					"dsAttrTypeStandard:SMBLogoffTime"

/*!
 * @defined kDS1AttrSMBLogonTime
 * @discussion Attribute in support of SMB interaction.
 */
#define		kDS1AttrSMBLogonTime					"dsAttrTypeStandard:SMBLogonTime"

/*!
 * @defined kDS1AttrSMBPrimaryGroupSID
 * @discussion SMB Primary Group Security ID, stored as a string attribute of
 *	up to 64 bytes. Found in user, group, and computer records
 *	(kDSStdRecordTypeUsers, kDSStdRecordTypeGroups, kDSStdRecordTypeComputers).
 */
#define		kDS1AttrSMBPrimaryGroupSID				"dsAttrTypeStandard:SMBPrimaryGroupSID"

/*!
 * @defined kDS1AttrSMBPWDLastSet
 * @discussion Attribute in support of SMB interaction.
 */
#define		kDS1AttrSMBPWDLastSet					"dsAttrTypeStandard:SMBPasswordLastSet"

/*!
 * @defined kDS1AttrSMBProfilePath
 * @discussion Desktop management info (dock, desktop links, etc).
 */
#define		kDS1AttrSMBProfilePath					"dsAttrTypeStandard:SMBProfilePath"

/*!
 * @defined kDS1AttrSMBRID
 * @discussion Attribute in support of SMB interaction.
 */
#define 	kDS1AttrSMBRID							"dsAttrTypeStandard:SMBRID"

/*!
 * @defined kDS1AttrSMBScriptPath
 * @discussion Login script path.
 */
#define 	kDS1AttrSMBScriptPath					"dsAttrTypeStandard:SMBScriptPath"

/*!
 * @defined kDS1AttrSMBSID
 * @discussion SMB Security ID, stored as a string attribute of up to 64 bytes.
 *	Found in user, group, and computer records (kDSStdRecordTypeUsers, 
 *	kDSStdRecordTypeGroups, kDSStdRecordTypeComputers).
 */
#define		kDS1AttrSMBSID							"dsAttrTypeStandard:SMBSID"

/*!
 * @defined kDS1AttrSMBUserWorkstations
 * @discussion List of workstations user can login from (machine account names).
 */
#define		kDS1AttrSMBUserWorkstations				"dsAttrTypeStandard:SMBUserWorkstations"

/*!
 * @defined kDS1AttrServiceType
 * @discussion Represents the service type for the service.  This is the raw service type of the
 *     service.  For example a service record type of kDSStdRecordTypeWebServer 
 *     might have a service type of "http" or "https".
 */
#define		kDS1AttrServiceType						"dsAttrTypeStandard:ServiceType"

/*!
 * @defined kDS1AttrSetupAdvertising
 * @discussion Used for Setup Assistant automatic population.
 */
#define		kDS1AttrSetupAdvertising				"dsAttrTypeStandard:SetupAssistantAdvertising"

/*!
 * @defined kDS1AttrSetupAutoRegister
 * @discussion Used for Setup Assistant automatic population.
 */
#define		kDS1AttrSetupAutoRegister				"dsAttrTypeStandard:SetupAssistantAutoRegister"

/*!
 * @defined kDS1AttrSetupLocation
 * @discussion Used for Setup Assistant automatic population.
 */
#define		kDS1AttrSetupLocation					"dsAttrTypeStandard:SetupAssistantLocation"

/*!
 * @defined kDS1AttrSetupOccupation
 * @discussion Used for Setup Assistant automatic population.
 */
#define		kDS1AttrSetupOccupation					"dsAttrTypeStandard:Occupation"

/*!
 * @defined kDS1AttrTimeToLive
 * @discussion Attribute recommending how long to cache the record's attribute values.
 * Format is an unsigned 32 bit representing seconds. ie. 300 is 5 minutes.
 */
#define		kDS1AttrTimeToLive						"dsAttrTypeStandard:TimeToLive"

/*!
 * @defined kDS1AttrUniqueID
 * @discussion This is the 32 bit unique ID that represents the user in the legacy manner.
 * Format is a signed integer represented as a string.
 */
#define		kDS1AttrUniqueID						"dsAttrTypeStandard:UniqueID"

/*!
 * @defined kDS1AttrUserCertificate
 * @discussion Attribute containing the binary of the user's certificate.
 * Usually found in user records. The certificate is data which identifies a user.
 * This data is attested to by a known party, and can be independently verified 
 * by a third party.
 */
#define		kDS1AttrUserCertificate					"dsAttrTypeStandard:UserCertificate"

/*!
 * @defined kDS1AttrUserPKCS12Data
 * @discussion Attribute containing binary data in PKCS #12 format. 
 * Usually found in user records. The value can contain keys, certificates,
 * and other related information and is encrypted with a passphrase.
 */
#define		kDS1AttrUserPKCS12Data					"dsAttrTypeStandard:UserPKCS12Data"

/*!
 * @defined kDS1AttrUserShell
 * @discussion Used to represent the user's shell setting.
 */
#define		kDS1AttrUserShell						"dsAttrTypeStandard:UserShell"

/*!
 * @defined kDS1AttrUserSMIMECertificate
 * @discussion Attribute containing the binary of the user's SMIME certificate.
 * Usually found in user records. The certificate is data which identifies a user.
 * This data is attested to by a known party, and can be independently verified 
 * by a third party. SMIME certificates are often used for signed or encrypted
 * emails.
 */
#define		kDS1AttrUserSMIMECertificate			"dsAttrTypeStandard:UserSMIMECertificate"

/*!
 * @defined kDS1AttrVFSDumpFreq
 * @discussion Attribute used to support mount records.
 */
#define		kDS1AttrVFSDumpFreq						"dsAttrTypeStandard:VFSDumpFreq"

/*!
 * @defined kDS1AttrVFSLinkDir
 * @discussion Attribute used to support mount records.
 */
#define		kDS1AttrVFSLinkDir						"dsAttrTypeStandard:VFSLinkDir"

/*!
 * @defined kDS1AttrVFSPassNo
 * @discussion Attribute used to support mount records.
 */
#define		kDS1AttrVFSPassNo						"dsAttrTypeStandard:VFSPassNo"

/*!
 * @defined kDS1AttrVFSType
 * @discussion Attribute used to support mount records.
 */
#define		kDS1AttrVFSType							"dsAttrTypeStandard:VFSType"

/*!
 * @defined kDS1AttrWeblogURI
 * @discussion Single-valued attribute that defines the URI of a user's weblog.
 *	Usually found in user records (kDSStdRecordTypeUsers). 
 *	Example: http://example.com/blog/jsmith
 */
#define		kDS1AttrWeblogURI						"dsAttrTypeStandard:WeblogURI"

/*!
 *	@defined kDS1AttrXMLPlist
 *	@discussion SA config settings plist.
 */
#define		kDS1AttrXMLPlist						"dsAttrTypeStandard:XMLPlist"

/*!
 * @defined kDS1AttrProtocolNumber
 * @discussion Single-valued attribute that defines a protocol number.  Usually found
 *  in protocol records (kDSStdRecordTypeProtocols)
 */
#define     kDS1AttrProtocolNumber                  "dsAttrTypeStandard:ProtocolNumber"

/*!
 * @defined kDS1AttrRPCNumber
 * @discussion Single-valued attribute that defines an RPC number.  Usually found
 *  in RPC records (kDSStdRecordTypeRPC)
 */
#define     kDS1AttrRPCNumber                       "dsAttrTypeStandard:RPCNumber"

/*!
 * @defined kDS1AttrNetworkNumber
 * @discussion Single-valued attribute that defines a network number.  Usually found
 *  in network records (kDSStdRecordTypeNetworks)
 */
#define     kDS1AttrNetworkNumber                   "dsAttrTypeStandard:NetworkNumber"

#pragma mark -
#pragma mark Multiple Valued Specific Attribute Type Constants
#pragma mark -

/*!
 * @functiongroup DirectoryService Multiple Valued Specific Attribute Type Constants
 */

/*!
 * @defined kDSNAttrAccessControlEntry
 * @discussion Attribute type which stores directory access control directives.
 */
#define		kDSNAttrAccessControlEntry				"dsAttrTypeStandard:AccessControlEntry"

/*!
 * @defined kDSNAttrAddressLine1
 * @discussion Line one of multiple lines of address data for a user.
 */
#define		kDSNAttrAddressLine1					"dsAttrTypeStandard:AddressLine1"

/*!
 * @defined kDSNAttrAddressLine2
 * @discussion Line two of multiple lines of address data for a user.
 */
#define		kDSNAttrAddressLine2					"dsAttrTypeStandard:AddressLine2"

/*!
 * @defined kDSNAttrAddressLine3
 * @discussion Line three of multiple lines of address data for a user.
 */
#define		kDSNAttrAddressLine3					"dsAttrTypeStandard:AddressLine3"

/*!
 * @defined kDSNAttrAreaCode
 * @discussion Area code of a user's phone number.
 */
#define		kDSNAttrAreaCode						"dsAttrTypeStandard:AreaCode"

/*!
 * @defined kDSNAttrAuthenticationAuthority
 * @discussion Determines what mechanism is used to verify or set a user's password.
 *     If multiple values are present, the first attributes returned take precedence.
 *     Typically found in User records (kDSStdRecordTypeUsers).
 */
#define		kDSNAttrAuthenticationAuthority			"dsAttrTypeStandard:AuthenticationAuthority"

/*!
 * @defined kDSNAttrAutomountInformation
 * @discussion Used to store automount information in kDSStdRecordTypeAutomount records.
 */
#define		kDSNAttrAutomountInformation			"dsAttrTypeStandard:AutomountInformation"

/*!
 * @defined kDSNAttrBootParams
 * @discussion Attribute type in host or machine records for storing boot params.
 */
#define		kDSNAttrBootParams						"dsAttrTypeStandard:BootParams"

/*!
 * @defined kDSNAttrBuilding
 * @discussion Represents the building name for a user or person record.
 * Usually found in user or people records (kDSStdRecordTypeUsers or 
 * kDSStdRecordTypePeople).
 */
#define		kDSNAttrBuilding						"dsAttrTypeStandard:Building"

/*!
 * @defined kDSNAttrServicesLocator
 * @discussion the URI for a record's calendar
 */
#define		kDSNAttrServicesLocator			"dsAttrTypeStandard:ServicesLocator"

/*!
 * @defined kDSNAttrCity
 * @discussion Usually, city for a user or person record.
 * Usually found in user or people records (kDSStdRecordTypeUsers or 
 * kDSStdRecordTypePeople).
 */
#define		kDSNAttrCity							"dsAttrTypeStandard:City"

/*!
 * @defined kDSNAttrCompany
 * @discussion attribute that defines the user's company.
 * Example: Apple Inc
 */
#define		kDSNAttrCompany						"dsAttrTypeStandard:Company"

/*!
 * @defined kDSNAttrComputerAlias
 * @discussion Attribute type in Neighborhood records describing computer records pointed to by
 * this neighborhood.
 */
#define		kDSNAttrComputerAlias					"dsAttrTypeStandard:ComputerAlias"

/*!
 * @defined kDSNAttrComputers
 * @discussion List of computers.
 */
#define		kDSNAttrComputers						"dsAttrTypeStandard:Computers"

/*!
 * @defined kDSNAttrCountry
 * @discussion Represents country of a record entry.
 * Usually found in user or people records (kDSStdRecordTypeUsers or 
 * kDSStdRecordTypePeople).
 */
#define		kDSNAttrCountry							"dsAttrTypeStandard:Country"

/*!
 * @defined kDSNAttrDepartment
 * @discussion Represents the department name of a user or person.
 * Usually found in user or people records (kDSStdRecordTypeUsers or 
 * kDSStdRecordTypePeople).
 */
#define		kDSNAttrDepartment						"dsAttrTypeStandard:Department"

/*!
 * @defined kDSNAttrDNSName
 * @discussion Domain Name Service name.
 */
#define		kDSNAttrDNSName							"dsAttrTypeStandard:DNSName"

/*!
 * @defined kDSNAttrEMailAddress
 * @discussion Email address of usually a user record.
 */
#define		kDSNAttrEMailAddress					"dsAttrTypeStandard:EMailAddress"

/*!
 * @defined kDSNAttrEMailContacts
 * @discussion multi-valued attribute that defines a record's custom email addresses .
 *	 found in user records (kDSStdRecordTypeUsers). 
 *	Example: home:johndoe@mymail.com
 */
#define		kDSNAttrEMailContacts				"dsAttrTypeStandard:EMailContacts"

/*!
 * @defined kDSNAttrFaxNumber
 * @discussion Represents the FAX numbers of a user or person.
 * Usually found in user or people records (kDSStdRecordTypeUsers or 
 * kDSStdRecordTypePeople).
 */
#define		kDSNAttrFaxNumber						"dsAttrTypeStandard:FAXNumber"

/*!
 * @defined kDSNAttrGroup
 * @discussion List of groups.
 */
#define		kDSNAttrGroup							"dsAttrTypeStandard:Group"

/*!
 * @defined kDSNAttrGroupMembers
 * @discussion Attribute type in group records containing lists of GUID values for members other than groups.
 */
#define		kDSNAttrGroupMembers					"dsAttrTypeStandard:GroupMembers"

/*!
 * @defined kDSNAttrGroupMembership
 * @discussion Usually a list of users that below to a given group record.
 */
#define		kDSNAttrGroupMembership					"dsAttrTypeStandard:GroupMembership"

/*!
 * @defined kDSNAttrGroupServices
 * @discussion xml-plist attribute that defines a group's services .
 *	 found in group records (kDSStdRecordTypeGroups). 
 */
#define		kDSNAttrGroupServices				"dsAttrTypeStandard:GroupServices"

/*!
 * @defined kDSNAttrHomePhoneNumber
 * @discussion Home telephone number of a user or person.
 */
#define		kDSNAttrHomePhoneNumber				"dsAttrTypeStandard:HomePhoneNumber"

/*!
 * @defined kDSNAttrHTML
 * @discussion HTML location.
 */
#define		kDSNAttrHTML							"dsAttrTypeStandard:HTML"

/*!
 * @defined kDSNAttrHomeDirectory
 * @discussion Network home directory URL.
 */
#define		kDSNAttrHomeDirectory					"dsAttrTypeStandard:HomeDirectory"

/*!
 * @defined kDSNAttrIMHandle
 * @discussion Represents the Instant Messaging handles of a user.
 * Values should be prefixed with the appropriate IM type
 * ie. AIM:, Jabber:, MSN:, Yahoo:, or ICQ:
 * Usually found in user records (kDSStdRecordTypeUsers).
 */
#define		kDSNAttrIMHandle						"dsAttrTypeStandard:IMHandle"

/*!
 * @defined kDSNAttrIPAddress
 * @discussion IP address expressed either as domain or IP notation.
 */
#define		kDSNAttrIPAddress						"dsAttrTypeStandard:IPAddress"

/*!
 * @defined	kDSNAttrIPAddressAndENetAddress
 * @discussion A pairing of IPv4 or IPv6 addresses with Ethernet addresses 
 * (e.g., "10.1.1.1/00:16:cb:92:56:41").  Usually found on kDSStdRecordTypeComputers for use by 
 * services that need specific pairing of the two values.  This should be in addition to 
 * kDSNAttrIPAddress, kDSNAttrIPv6Address and kDS1AttrENetAddress. This is necessary because not
 * all directories return attribute values in a guaranteed order.
 */
#define		kDSNAttrIPAddressAndENetAddress			"dsAttrTypeStandard:IPAddressAndENetAddress"

/*!
 * @defined kDSNAttrIPv6Address
 * @discussion IPv6 address expressed in the standard notation (e.g., "fe80::236:caff:fcc2:5641" )
 * Usually found on kDSStdRecordTypeComputers, kDSStdRecordTypeHosts, and 
 * kDSStdRecordTypeMachines.
 */
#define		kDSNAttrIPv6Address						"dsAttrTypeStandard:IPv6Address"

/*!
 * @defined kDSNAttrJPEGPhoto
 * @discussion Used to store binary picture data in JPEG format. 
 * Usually found in user, people or group records (kDSStdRecordTypeUsers, 
 * kDSStdRecordTypePeople, kDSStdRecordTypeGroups).
 */
#define		kDSNAttrJPEGPhoto						"dsAttrTypeStandard:JPEGPhoto"

/*!
 * @defined kDSNAttrJobTitle
 * @discussion Represents the job title of a user.
 * Usually found in user or people records (kDSStdRecordTypeUsers or 
 * kDSStdRecordTypePeople).
 */
#define		kDSNAttrJobTitle						"dsAttrTypeStandard:JobTitle"

/*!
 * @defined kDSNAttrKDCAuthKey
 * @discussion KDC master key RSA encrypted with realm public key.
 */
#define		kDSNAttrKDCAuthKey						"dsAttrTypeStandard:KDCAuthKey"

/*!
 *	@defined kDSNAttrKeywords
 *	@discussion Keywords using for searching capability.
 */
#define		kDSNAttrKeywords						"dsAttrTypeStandard:Keywords"

/*!
 * @defined kDSNAttrLDAPReadReplicas
 * @discussion List of LDAP server URLs which can each be used to read directory data.
 */
#define		kDSNAttrLDAPReadReplicas				"dsAttrTypeStandard:LDAPReadReplicas"

/*!
 * @defined kDSNAttrLDAPWriteReplicas
 * @discussion List of LDAP server URLs which can each be used to write directory data.
 */
#define		kDSNAttrLDAPWriteReplicas				"dsAttrTypeStandard:LDAPWriteReplicas"

/*!
 * @defined kDSNAttrMachineServes
 * @discussion Attribute type in host or machine records for storing NetInfo 
 *		domains served.
 */
#define		kDSNAttrMachineServes					"dsAttrTypeStandard:MachineServes"

/*!
 * @defined kDSNAttrMapCoordinates
 * @discussion attribute that defines coordinates for a user's location .
*	 found in user records (kDSStdRecordTypeUsers) and resource records (kDSStdRecordTypeResources).
 *	Example: 7.7,10.6
 */
#define		kDSNAttrMapCoordinates				"dsAttrTypeStandard:MapCoordinates"

/*!
 * @defined kDSNAttrMapURI
 * @discussion attribute that defines the URI of a user's location.
 *	Usually found in user records (kDSStdRecordTypeUsers). 
 *	Example: http://example.com/bldg1
 */
#define		kDSNAttrMapURI						"dsAttrTypeStandard:MapURI"

/*!
 * @defined kDSNAttrMCXSettings
 * @discussion Used by MCX.
 */
#define		kDSNAttrMCXSettings						"dsAttrTypeStandard:MCXSettings"

/*!
 * @defined kDSNAttrMIME
 * @discussion Data contained in this attribute type is a fully qualified MIME Type. 
 */
#define		kDSNAttrMIME							"dsAttrTypeStandard:MIME"

/*!
 * @defined kDSNAttrMember
 * @discussion List of member records. 
 */
#define		kDSNAttrMember							"dsAttrTypeStandard:Member"

/*!
 * @defined kDSNAttrMobileNumber
 * @discussion Represents the mobile numbers of a user or person.
 * Usually found in user or people records (kDSStdRecordTypeUsers or 
 * kDSStdRecordTypePeople).
 */
#define		kDSNAttrMobileNumber					"dsAttrTypeStandard:MobileNumber"

/*!
 * @defined kDSNAttrNBPEntry
 * @discussion Appletalk data.
 */
#define		kDSNAttrNBPEntry						"dsAttrTypeStandard:NBPEntry"

/*!
 * @defined kDSNAttrNestedGroups
 * @discussion Attribute type in group records for the list of GUID values for nested groups.
 */
#define		kDSNAttrNestedGroups					"dsAttrTypeStandard:NestedGroups"

/*!
 * @defined kDSNAttrNetGroups
 * @discussion Attribute type that indicates which netgroups its record is a member of.
 *		Found in user, host, and netdomain records.
 */
#define		kDSNAttrNetGroups						"dsAttrTypeStandard:NetGroups"

/*!
 * @defined kDSNAttrNickName
 * @discussion Represents the nickname of a user or person.
 * Usually found in user or people records (kDSStdRecordTypeUsers or 
 * kDSStdRecordTypePeople).
 */
#define		kDSNAttrNickName						"dsAttrTypeStandard:NickName"

/*!
 * @defined kDSNAttrNodePathXMLPlist
 * @discussion Attribute type in Neighborhood records describing the DS Node to search while
 * looking up aliases in this neighborhood.
 */
#define		kDSNAttrNodePathXMLPlist				"dsAttrTypeStandard:NodePathXMLPlist"

/*!
 * @defined kDSNAttrOrganizationInfo
 * @discussion Usually the organization info of a user.
 */
#define		kDSNAttrOrganizationInfo				"dsAttrTypeStandard:OrganizationInfo"

/*!
 * @defined kDSNAttrOrganizationName
 * @discussion Usually the organization of a user.
 */
#define		kDSNAttrOrganizationName				"dsAttrTypeStandard:OrganizationName"

/*!
 * @defined kDSNAttrPagerNumber
 * @discussion Represents the pager numbers of a user or person.
 * Usually found in user or people records (kDSStdRecordTypeUsers or 
 * kDSStdRecordTypePeople).
 */
#define		kDSNAttrPagerNumber						"dsAttrTypeStandard:PagerNumber"

/*!
 * @defined kDSNAttrPhoneContacts
 * @discussion multi-valued attribute that defines a record's custom phone numbers .
 *	 found in user records (kDSStdRecordTypeUsers). 
 *	Example: home fax:408-555-4444
 */
#define		kDSNAttrPhoneContacts				"dsAttrTypeStandard:PhoneContacts"


/*!
 * @defined kDSNAttrPhoneNumber
 * @discussion Telephone number of a user.
 */
#define		kDSNAttrPhoneNumber						"dsAttrTypeStandard:PhoneNumber"

/*!
 * @defined kDSNAttrPGPPublicKey
 * @discussion Pretty Good Privacy public encryption key.
 */
#define		kDSNAttrPGPPublicKey					"dsAttrTypeStandard:PGPPublicKey"

/*!
 * @defined kDSNAttrPostalAddress
 * @discussion The postal address usually excluding postal code.
 */
#define		kDSNAttrPostalAddress					"dsAttrTypeStandard:PostalAddress"

/*!
* @defined kDSNAttrPostalAddressContacts
* @discussion multi-valued attribute that defines a record's alternate postal addresses .
*	 found in user records (kDSStdRecordTypeUsers) and resource records (kDSStdRecordTypeResources).
*/
#define		kDSNAttrPostalAddressContacts				"dsAttrTypeStandard:PostalAddressContacts"

/*!
 * @defined kDSNAttrPostalCode
 * @discussion The postal code such as zip code in the USA.
 */
#define		kDSNAttrPostalCode						"dsAttrTypeStandard:PostalCode"

/*!
 * @defined kDSNAttrNamePrefix
 * @discussion Represents the title prefix of a user or person.
 * ie. Mr., Ms., Mrs., Dr., etc.
 * Usually found in user or people records (kDSStdRecordTypeUsers or 
 * kDSStdRecordTypePeople).
 */
#define		kDSNAttrNamePrefix						"dsAttrTypeStandard:NamePrefix"

/*!
 * @defined kDSNAttrProtocols
 * @discussion List of protocols.
 */
#define		kDSNAttrProtocols						"dsAttrTypeStandard:Protocols"

/*!
 * @defined kDSNAttrRecordName
 * @discussion List of names/keys for this record.
 */
#define		kDSNAttrRecordName						"dsAttrTypeStandard:RecordName"

/*!
 * @defined kDSNAttrRelationships
 * @discussion multi-valued attribute that defines the relationship to the record type .
 *	 found in user records (kDSStdRecordTypeUsers). 
 *	Example: brother:John
 */
#define		kDSNAttrRelationships				"dsAttrTypeStandard:Relationships"

/*!
* @defined kDSNAttrResourceInfo
* @discussion multi-valued attribute that defines a resource record's info.
*/
#define		kDSNAttrResourceInfo				"dsAttrTypeStandard:ResourceInfo"

/*!
 * @defined kDSNAttrResourceType
 * @discussion Attribute type for the kind of resource. 
 *	 found in resource records (kDSStdRecordTypeResources). 
 *	Example: ConferenceRoom
 */
#define		kDSNAttrResourceType					"dsAttrTypeStandard:ResourceType"

/*!
 * @defined kDSNAttrState
 * @discussion The state or province of a country.
 */
#define		kDSNAttrState							"dsAttrTypeStandard:State"

/*!
 * @defined kDSNAttrStreet
 * @discussion Represents the street address of a user or person.
 * Usually found in user or people records (kDSStdRecordTypeUsers or 
 * kDSStdRecordTypePeople).
 */
#define		kDSNAttrStreet							"dsAttrTypeStandard:Street"

/*!
 * @defined kDSNAttrNameSuffix
 * @discussion Represents the name suffix of a user or person.
 * ie. Jr., Sr., etc.
 * Usually found in user or people records (kDSStdRecordTypeUsers or 
 * kDSStdRecordTypePeople).
 */
#define		kDSNAttrNameSuffix						"dsAttrTypeStandard:NameSuffix"

/*!
 * @defined kDSNAttrURL
 * @discussion List of URLs.
 */
#define		kDSNAttrURL								"dsAttrTypeStandard:URL"

/*!
 * @defined kDSNAttrURLForNSL
 * @discussion List of URLs used by NSL.
 */
#define		kDSNAttrURLForNSL						"dsAttrTypeStandard:URLForNSL"

/*!
 * @defined kDSNAttrVFSOpts
 * @discussion Used in support of mount records.
 */
#define		kDSNAttrVFSOpts							"dsAttrTypeStandard:VFSOpts"


#pragma mark -
#pragma mark Other Attribute Type Constants
#pragma mark -

/*!
 * @functiongroup DirectoryService Other Attribute Type Constants Not Mapped by Directory Node Plugins
 * Mainly used internally by the DirectoryService Daemon or made available via dsGetDirNodeInfo()
 */

/*!
 * @defined kDS1AttrAdminStatus
 * @discussion Retained only for backward compatibility.
 */
#define		kDS1AttrAdminStatus						"dsAttrTypeStandard:AdminStatus"

/*!
 * @defined kDS1AttrAlias
 * @discussion Alias attribute, contain pointer to another node/record/attribute.
 */
#define		kDS1AttrAlias							"dsAttrTypeStandard:Alias"

/*!
 * @defined kDS1AttrAuthCredential
 * @discussion An "auth" credential, to be used to authenticate to other Directory nodes.
 */
#define		kDS1AttrAuthCredential					"dsAttrTypeStandard:AuthCredential"

/*!
 * @defined kDS1AttrCopyTimestamp
 * @discussion Timestamp used in local account caching.
 */
#define		kDS1AttrCopyTimestamp					"dsAttrTypeStandard:CopyTimestamp"

/*!
 *	@defined kDS1AttrDateRecordCreated
 *	@discussion Date of record creation.
 */
#define     kDS1AttrDateRecordCreated				"dsAttrTypeStandard:DateRecordCreated"

/*!
 * @defined kDS1AttrKerberosRealm
 * @discussion Supports Kerberized SMB Server services.
 */
#define		kDS1AttrKerberosRealm					"dsAttrTypeStandard:KerberosRealm"

/*!
 * @defined kDS1AttrNTDomainComputerAccount
 * @discussion Supports Kerberized SMB Server services.
 */
#define		kDS1AttrNTDomainComputerAccount			"dsAttrTypeStandard:NTDomainComputerAccount"

/*!
 * @defined kDSNAttrOriginalHomeDirectory
 * @discussion Home directory URL used in local account caching.
 */
#define		kDSNAttrOriginalHomeDirectory			"dsAttrTypeStandard:OriginalHomeDirectory"

/*!
 * @defined kDS1AttrOriginalNFSHomeDirectory
 * @discussion NFS home directory used in local account caching.
 */
#define		kDS1AttrOriginalNFSHomeDirectory		"dsAttrTypeStandard:OriginalNFSHomeDirectory"

/*!
 * @defined kDS1AttrOriginalNodeName
 * @discussion Nodename used in local account caching.
 */
#define		kDS1AttrOriginalNodeName				"dsAttrTypeStandard:OriginalNodeName"

/*!
 * @defined kDS1AttrPrimaryNTDomain
 * @discussion Supports Kerberized SMB Server services.
 */
#define		kDS1AttrPrimaryNTDomain					"dsAttrTypeStandard:PrimaryNTDomain"

/*!
 * @defined kDS1AttrPwdAgingPolicy
 * @discussion Contains the password aging policy data for an authentication capable record.
 */
#define		kDS1AttrPwdAgingPolicy					"dsAttrTypeStandard:PwdAgingPolicy"

/*!
 * @defined kDS1AttrRARA
 * @discussion Retained only for backward compatibility.
 */
#define		kDS1AttrRARA							"dsAttrTypeStandard:RARA"

/*!
 * @defined kDS1AttrReadOnlyNode
 * @discussion Can be found using dsGetDirNodeInfo and will return one of
 * ReadOnly, ReadWrite, or WriteOnly strings.
 * Note that ReadWrite does not imply fully readable or writable
 */
#define		kDS1AttrReadOnlyNode					"dsAttrTypeStandard:ReadOnlyNode"

/*!
 * @defined kDS1AttrRecordImage
 * @discussion A binary image of the record and all it's attributes.
 * Has never been supported.
 */
#define		kDS1AttrRecordImage			"dsAttrTypeStandard:RecordImage"

/*!
 * @defined kDS1AttrSMBGroupRID
 * @discussion Attributefor supporting PDC SMB interaction.
 */
#define 	kDS1AttrSMBGroupRID						"dsAttrTypeStandard:SMBGroupRID"

/*!
 * @defined kDS1AttrTimePackage
 * @discussion Data of Create, Modify, Backup time in UTC.
 */
#define		kDS1AttrTimePackage						"dsAttrTypeStandard:TimePackage"

/*!
 * @defined kDS1AttrTotalSize
 * @discussion checksum/meta data.
 */
#define		kDS1AttrTotalSize						"dsAttrTypeStandard:TotalSize"

/*!
 * @defined kDSNAttrAllNames
 * @discussion Backward compatibility only - all possible names for a record.
 * Has never been supported.
 */
#define		kDSNAttrAllNames						"dsAttrTypeStandard:AllNames"

/*!
 * @defined kDSNAttrAuthMethod
 * @discussion Authentication method for an authentication capable record.
 */
#define		kDSNAttrAuthMethod						"dsAttrTypeStandard:AuthMethod"

/*!
 * @defined kDSNAttrMetaNodeLocation
 * @discussion Meta attribute returning registered node name by directory node plugin.
 */
#define		kDSNAttrMetaNodeLocation				"dsAttrTypeStandard:AppleMetaNodeLocation"

/*!
 * @defined kDSNAttrNodePath
 * @discussion Sub strings of a Directory Service Node given in order.
 */
#define		kDSNAttrNodePath						"dsAttrTypeStandard:NodePath"

/*!
 * @defined kDSNAttrPlugInInfo
 * @discussion Information (version, signature, about, credits, etc.) about the plug-in
 * that is actually servicing a particular directory node.
 * Has never been supported.
 */
#define		kDSNAttrPlugInInfo						"dsAttrTypeStandard:PlugInInfo"

/*!
 * @defined kDSNAttrRecordAlias
 * @discussion No longer supported in Mac OS X 10.4 or later.
 */
#define		kDSNAttrRecordAlias						"dsAttrTypeStandard:RecordAlias"

/*!
 * @defined kDSNAttrRecordType
 * @discussion Single Valued for a Record, Multi-valued for a Directory Node.
 */
#define		kDSNAttrRecordType						"dsAttrTypeStandard:RecordType"

/*!
 * @defined kDSNAttrSchema
 * @discussion List of attribute types.
 */
#define		kDSNAttrSchema							"dsAttrTypeStandard:Scheama"

/*!
 * @defined kDSNAttrSetPasswdMethod
 * @discussion Retained only for backward compatibility.
 */
#define		kDSNAttrSetPasswdMethod					"dsAttrTypeStandard:SetPasswdMethod"

/*!
 * @defined kDSNAttrSubNodes
 * @discussion Attribute of a node which lists the available subnodes
 *		of that node.
 */
#define		kDSNAttrSubNodes						"dsAttrTypeStandard:SubNodes"

/*!
 * @defined kStandardSourceAlias
 * @discussion No longer supported in Mac OS X 10.4 or later.
 */
#define		kStandardSourceAlias					"dsAttrTypeStandard:AppleMetaAliasSource"

/*!
 * @defined kStandardTargetAlias
 * @discussion No longer supported in Mac OS X 10.4 or later.
 */
#define		kStandardTargetAlias					"dsAttrTypeStandard:AppleMetaAliasTarget"

/*!
 * @defined kDSNAttrNetGroupTriplet
 * @discussion Multivalued attribute that defines the host, user and domain triplet combinations
 *  to support NetGroups.  Each attribute value is comma separated string to maintain the
 *  triplet (e.g., host,user,domain).
 */
#define     kDSNAttrNetGroupTriplet                 "dsAttrTypeStandard:NetGroupTriplet"

#pragma mark -
#pragma mark Search Node attribute type Constants
#pragma mark -

/*!
 * @functiongroup Search Node attribute type Constants
 */
 
/*!
 * @defined kDS1AttrSearchPath
 * @discussion Search path used by the search node.
 */
#define		kDS1AttrSearchPath						"dsAttrTypeStandard:SearchPath"

/*!
 * @defined kDSNAttrSearchPath
 * @discussion Retained only for backward compatibility.
 */
#define		kDSNAttrSearchPath						"dsAttrTypeStandard:SearchPath"

/*!
 * @defined kDS1AttrSearchPolicy
 * @discussion Search policy for the search node.
 */
#define		kDS1AttrSearchPolicy					"dsAttrTypeStandard:SearchPolicy"

/*!
 * @defined kDS1AttrNSPSearchPath
 * @discussion Automatic search path defined by the search node.
 */
#define		kDS1AttrNSPSearchPath					"dsAttrTypeStandard:NSPSearchPath"

/*!
 * @defined kDSNAttrNSPSearchPath
 * @discussion Retained only for backward compatibility.
 */
#define		kDSNAttrNSPSearchPath					"dsAttrTypeStandard:NSPSearchPath"

/*!
 * @defined kDS1AttrLSPSearchPath
 * @discussion Local only search path defined by the search node.
 */
#define		kDS1AttrLSPSearchPath					"dsAttrTypeStandard:LSPSearchPath"

/*!
 * @defined kDSNAttrLSPSearchPath
 * @discussion Retained only for backward compatibility.
 */
#define		kDSNAttrLSPSearchPath					"dsAttrTypeStandard:LSPSearchPath"

/*!
 * @defined kDS1AttrCSPSearchPath
 * @discussion Admin user configured custom search path defined by the search node.
 */
#define		kDS1AttrCSPSearchPath					"dsAttrTypeStandard:CSPSearchPath"

/*!
 * @defined kDSNAttrCSPSearchPath
 * @discussion Retained only for backward compatibility.
 */
#define		kDSNAttrCSPSearchPath					"dsAttrTypeStandard:CSPSearchPath"

#pragma mark -
#pragma mark Authentication Constants
#pragma mark -

/*!
 * @functiongroup DirectoryService Authentication Constants
 */

/*!
 * @defined kDSStdAuthMethodPrefix
 * @discussion Standard authentication constant prefix.
 */
#define		kDSStdAuthMethodPrefix					"dsAuthMethodStandard:"

/*!
 * @defined kDSNativeAuthMethodPrefix
 * @discussion Native authentication method prefix.
 */
#define		kDSNativeAuthMethodPrefix				"dsAuthMethodNative:"

/*!
 * @defined kDSSetPasswdBestOf
 * @discussion Retained only for backward compatibility.
 */
#define		kDSSetPasswdBestOf						"dsSetPasswdBestOf"

/*!
 * @defined kDSValueAuthAuthorityDefault
 * @discussion The default value to use for the kDSNAttrAuthenticationAuthority attribute.
 *     When creating a user record, set this value for authentication authority before
 *     setting the password with dsDoDirNodeAuth.
 */
#define		kDSValueAuthAuthorityDefault			kDSValueAuthAuthorityBasic
//#define		kDSValueAuthAuthorityDefault				kDSValueAuthAuthorityShadowHash

/*!
 * @defined kDSValueAuthAuthorityBasic
 * @discussion Standard auth authority value for basic (crypt) authentication.
 */
#define		kDSValueAuthAuthorityBasic					";basic;"

/*!
 * @defined kDSTagAuthAuthorityBasic
 * @discussion Standard center tag data of auth authority value for basic (crypt) authentication.
 */
#define		kDSTagAuthAuthorityBasic					"basic"

/*!
 * @defined kDSValueAuthAuthorityLocalWindowsHash
 * @discussion Standard auth authority value for local windows hash authentication.
 * Retained only for backward compatibility.
 */
#define		kDSValueAuthAuthorityLocalWindowsHash		";LocalWindowsHash;"

/*!
 * @defined kDSTagAuthAuthorityLocalWindowsHash
 * @discussion Standard center tag data of auth authority value for local windows hash authentication.
 * Retained only for backward compatibility.
 */
#define		kDSTagAuthAuthorityLocalWindowsHash			"LocalWindowsHash"

/*!
 * @defined kDSValueAuthAuthorityShadowHash
 * @discussion Standard auth authority value for shadowhash authentication.
 */
#define		kDSValueAuthAuthorityShadowHash				";ShadowHash;"

/*!
 * @defined kDSTagAuthAuthorityShadowHash
 * @discussion Standard center tag data of auth authority value for shadowhash authentication.
 */
#define		kDSTagAuthAuthorityShadowHash				"ShadowHash"

/*!
 * @defined kDSTagAuthAuthorityBetterHashOnly
 * @discussion Standard tag data of auth authority value for better hash only selection authentication.
 */
#define		kDSTagAuthAuthorityBetterHashOnly			"BetterHashOnly"

/*!
 * @defined kDSValueAuthAuthorityPasswordServerPrefix
 * @discussion Standard auth authority value for Apple password server authentication.
 */
#define		kDSValueAuthAuthorityPasswordServerPrefix	";ApplePasswordServer;"

/*!
 * @defined kDSTagAuthAuthorityPasswordServer
 * @discussion Standard center tag data of auth authority value for Apple password server authentication.
 */
#define		kDSTagAuthAuthorityPasswordServer			"ApplePasswordServer"

/*!
 * @defined kDSValueAuthAuthorityKerberosv5
 * @discussion Standard auth authority value for Kerberos v5 authentication.
 */
#define		kDSValueAuthAuthorityKerberosv5				";Kerberosv5;"

/*!
 * @defined kDSTagAuthAuthorityKerberosv5
 * @discussion Standard center tag data of auth authority value for Kerberos v5 authentication.
 */
#define		kDSTagAuthAuthorityKerberosv5				"Kerberosv5"

/*!
 * @defined kDSValueAuthAuthorityLocalCachedUser
 * @discussion Standard auth authority value for local cached user authentication.
 */
#define		kDSValueAuthAuthorityLocalCachedUser		";LocalCachedUser;"

/*!
 * @defined kDSTagAuthAuthorityLocalCachedUser
 * @discussion Standard center tag data of auth authority value for local cached user authentication.
 */
#define		kDSTagAuthAuthorityLocalCachedUser			"LocalCachedUser"

/*!
 * @defined kDSValueAuthAuthorityDisabledUser
 * @discussion Standard auth authority value for disabled user authentication.
 */
#define		kDSValueAuthAuthorityDisabledUser			";DisabledUser;"

/*!
 * @defined kDSTagAuthAuthorityDisabledUser
 * @discussion Standard center tag data of auth authority value for disabled user authentication.
 */
#define		kDSTagAuthAuthorityDisabledUser				"DisabledUser"


/*!
 * @defined kDSValueNonCryptPasswordMarker
 * @discussion Marker used for password attribute value indicating non-crypt authentication.
 */
#define		kDSValueNonCryptPasswordMarker			"********"

#pragma mark -
#pragma mark Authentication Type Constants
#pragma mark -

/*!
 * @functiongroup DirectoryService Authentication Type Constants
 */

/*!
 * @defined kDSStdAuth2WayRandom
 * @discussion Two way random authentication method. This method uses two passes to
 *	complete the authentication.
 *
 *     The buffer for pass one is packed as follows:
 *     user name in UTF8 encoding
 *
 *     The buffer for pass two is packed as follows:
 *     8 byte DES digest + 8 bytes of random
 */
#define		kDSStdAuth2WayRandom					"dsAuthMethodStandard:dsAuth2WayRandom"

/*!
 * @defined kDSStdAuth2WayRandomChangePasswd
 * @discussion Change the password for a user using the two-way random method.
 *     Does not require prior authentication.
 *     The buffer is packed as follows:
 *
 *     4 byte length of user name,
 *     user name in UTF8 encoding,
 *     4 byte length of old password encrypted with new (should be 8),
 *     old password encrypted with new,
 *     4 byte length of new password encrypted with old (should be 8),
 *     new password encrypted with old
 */
#define		kDSStdAuth2WayRandomChangePasswd		"dsAuthMethodStandard:dsAuth2WayRandomChangePasswd"

/*!
 * @defined kDSStdAuthAPOP
 * @discussion APOP authentication method.
 *     The buffer is packed as follows:
 *
 *     4 byte length of user name,
 *     user name in UTF8 encoding,
 *     4 byte length of server challenge,
 *     server challenge in UTF8 encoding,
 *     4 byte length of client response,
 *     client response in UTF8 encoding
 */
#define		kDSStdAuthAPOP							"dsAuthMethodStandard:dsAuthAPOP"

/*!
 * @defined kDSStdAuthCHAP
 * @discussion CHAP authentication method.
 * This method is not implemented.
 */
#define		kDSStdAuthCHAP							"dsAuthMethodStandard:dsAuthCHAP"

/*!
 * @defined kDSStdAuthCRAM_MD5
 * @discussion CRAM MD5 authentication method.
 *     The buffer is packed as follows:
 *
 *     4 byte length of user name,
 *     user name in UTF8 encoding,
 *     4 byte length of server challenge,
 *     server challenge in UTF8 encoding,
 *     4 byte length of client response,
 *     client response data
 */
#define		kDSStdAuthCRAM_MD5						"dsAuthMethodStandard:dsAuthNodeCRAM-MD5"

/*!
 * @defined kDSStdAuthChangePasswd
 * @discussion Change the password for a user. Does not require prior authentication.
 *     The buffer is packed as follows:
 *
 *     4 byte length of user name,
 *     user name in UTF8 encoding,
 *     4 byte length of old password,
 *     old password in UTF8 encoding,
 *     4 byte length of new password,
 *     new password in UTF8 encoding
 */
#define		kDSStdAuthChangePasswd					"dsAuthMethodStandard:dsAuthChangePasswd"

/*!
 * @defined kDSStdAuthClearText
 * @discussion Clear text authentication method.
 *     The buffer is packed as follows:
 *
 *     4 byte length of user name,
 *     user name in UTF8 encoding,
 *     4 byte length of password,
 *     password in UTF8 encoding
 */
#define		kDSStdAuthClearText						"dsAuthMethodStandard:dsAuthClearText"

/*!
 * @defined kDSStdAuthCrypt
 * @discussion Use a crypt password stored in the user record if available to
 *     do the authentication. The buffer is packed as follows:
 *
 *     4 byte length of user name,
 *     user name in UTF8 encoding,
 *     4 byte length of password,
 *     password in UTF8 encoding
 *
 *     This method may not be supported by all plug-ins or for all users.
 */
#define		kDSStdAuthCrypt							"dsAuthMethodStandard:dsAuthCrypt"

/*!
 * @defined kDSStdAuthDIGEST_MD5
 * @discussion Digest MD5 authentication method.
 *     The buffer is packed as follows:
 *
 *     4 byte length of user name,
 *     user name in UTF8 encoding,
 *     4 byte length of server challenge,
 *     server challenge in UTF8 encoding,
 *     4 byte length of client response,
 *     client response data,
 *     4 byte length of HTTP method used,
 *     HTTP method in UTF8 encoding
 */
#define		kDSStdAuthDIGEST_MD5					"dsAuthMethodStandard:dsAuthNodeDIGEST-MD5"

/*!
 * @defined kDSStdAuthDeleteUser
 * @discussion Used for Apple password server user deletion.
 *	   This authentication method is only implemented by the PasswordServer node.
 *     The buffer is packed as follows:
 *
 *     4 byte length of authenticator Password Server ID,
 *     Password Server ID in UTF8 encoding,
 *     4 byte length of authenticator password,
 *     authenticator password in UTF8 encoding,
 *     4 byte length of user's Password Server ID,
 *     user's Password Server ID in UTF8 encoding
 */
#define		kDSStdAuthDeleteUser					"dsAuthMethodStandard:dsAuthDeleteUser"

/*!
 * @defined kDSStdAuthGetEffectivePolicy
 * @discussion Used to extract, from a password server, the actual policies that will be applied
 *    to a user; a combination of global and user policies.
 *     The buffer is packed as follows:
 *
 *     4 byte length of user name or Password Server ID,
 *     user name or Password Server ID in UTF8 encoding
 */
#define		kDSStdAuthGetEffectivePolicy			"dsAuthMethodStandard:dsAuthGetEffectivePolicy"

/*!
 * @defined kDSStdAuthGetGlobalPolicy
 * @discussion Used for extraction of global auth policy. Authentication
 *	   is not required to get policies. The authenticator name and password
 *	   fields may be left blank by using eight bytes of zeros.
 *     The buffer is packed as follows:
 *
 *     4 byte length of authenticator name,
 *     user name in UTF8 encoding
 *     4 byte length of authenticator password,
 *     password in UTF8 encoding
 */
#define		kDSStdAuthGetGlobalPolicy				"dsAuthMethodStandard:dsAuthGetGlobalPolicy"

/*!
 * @defined kDSStdAuthGetKerberosPrincipal
 * @discussion Kerberos Principal name.
 *     The buffer is packed as follows:
 *
 *     4 byte length of user name,
 *     user name in UTF8 encoding
 */
#define		kDSStdAuthGetKerberosPrincipal			"dsAuthMethodStandard:dsAuthGetKerberosPrincipal"

/*!
 * @defined kDSStdAuthGetPolicy
 * @discussion The plug-in should determine which specific authentication method to use.
 *	   Authentication is not required to get policies. The authenticator name and password
 *	   fields may be left blank by using a length of 1 and a zero-byte for the data.
 *     The buffer is packed as follows:
 *
 *     4 byte length of authenticator's name or Password Server ID,
 *     authenticator's name or Password Server ID in UTF8 encoding,
 *     4 byte length of authenticator's password,
 *     authenticator's password in UTF8 encoding
 *     4 byte length of account's name or Password Server ID,
 *     account's name or Password Server ID
 *
 *     The Password Server does not require authentication for this auth method.
 *       The first two fields are to cover us for future policy changes and to keep the buffer
 *       format as standardized as possible.
 */
#define		kDSStdAuthGetPolicy						"dsAuthMethodStandard:dsAuthGetPolicy"

/*!
 * @defined kDSStdAuthGetUserData
 * @discussion Used with Apple password server. The password server maintains a space
 *	   for a small amount of miscellaneous data.
 *	   This authentication method is only implemented by the PasswordServer node.
 *     The buffer is packed as follows:
 *
 *     4 byte length of authenticator's Password Server ID,
 *     authenticator's Password Server ID in UTF8 encoding,
 *     4 byte length of authenticator's password,
 *     authenticator's password in UTF8 encoding
 *     4 byte length of user's Password Server ID,
 *     Password Server ID in UTF8 encoding
 */
#define		kDSStdAuthGetUserData					"dsAuthMethodStandard:dsAuthGetUserData"

/*!
 * @defined kDSStdAuthGetUserName
 * @discussion Used with Apple password server. This name is the same as the primary
 *	   short name for the user.
 *	   This authentication method is only implemented by the PasswordServer node.
 *     The buffer is packed as follows:
 *
 *     4 byte length of authenticator's Password Server ID,
 *     authenticator's Password Server ID in UTF8 encoding,
 *     4 byte length of authenticator's password,
 *     authenticator's password in UTF8 encoding
 *     4 byte length of user's Password Server ID,
 *     user's Password Server ID in UTF8 encoding
 */
#define		kDSStdAuthGetUserName					"dsAuthMethodStandard:dsAuthGetUserName"

/*!
 * @defined kDSStdAuthKerberosTickets
 * @discussion Provides write-access to LDAP with an existing Kerberos ticket
 *     The buffer is packed as follows:
 *
 *     4 byte length of authenticator name,
 *     user name in UTF8 encoding
 *     4 byte length of krb5_data
 *     krb5_data struct containing a service ticket
 */
#define		kDSStdAuthKerberosTickets				"dsAuthMethodStandard:dsAuthKerberosTickets"

/*!
 * @defined kDSStdAuthMASKE_A
 * @discussion Retained only for backward compatibility.
 */
#define		kDSStdAuthMASKE_A						"dsAuthMethodStandard:dsAuthMASKE-A"

/*!
 * @defined kDSStdAuthMASKE_B
 * @discussion Retained only for backward compatibility.
 */
#define		kDSStdAuthMASKE_B						"dsAuthMethodStandard:dsAuthMASKE-B"

/*!
 * @defined kDSStdAuthMPPEMasterKeys
 * @discussion Generated 40-bit or 128-bit master keys from MS-CHAPv2 credentials (RFC 3079).
 *     The buffer is packed as follows:
 *
 *     4 byte length of user name,
 *     user name in UTF8 encoding,
 *     4 byte length of session's MS-CHAPv2 digest (P24),
 *     MS-CHAPv2 digest (P24)
 *     4 byte length of key size (always 1)
 *     key size, 8 or 16 (packed as a byte, not a string)
 */
#define		kDSStdAuthMPPEMasterKeys				"dsAuthMethodStandard:dsAuthMPPEMasterKeys"

/*!
 * @defined kDSStdAuthMSCHAP1
 * @discussion MS CHAP 1 authentication method.
 * This method is not implemented.
 */
#define		kDSStdAuthMSCHAP1						"dsAuthMethodStandard:dsAuthMSCHAP1"

/*!
 * @defined kDSStdAuthMSCHAP2
 * @discussion
 *     MS-CHAP2 is a mutual authentication method. The plug-in will generate the data to
 *     send back to the client and put it in the step buffer.
 *
 *     The input buffer format:
 *     4 byte length,
 *     user name in UTF8 encoding,
 *     4 byte length,
 *     server challenge,
 *     4 byte length,
 *     peer challenge,
 *     4 byte length,
 *     client's digest,
 *     4 byte length,
 *     client's user name (the name used for MS-CHAPv2, usually the first short name)
 *
 *     The output buffer format:
 *     4 byte length,
 *     return digest for the client's challenge
 */
#define		kDSStdAuthMSCHAP2						"dsAuthMethodStandard:dsAuthMSCHAP2"

/*!
 * @defined kDSStdAuthNTLMv2
 * @discussion Verifies an NTLMv2 challenge and response. The session keys
 *	(if any) must be retrieved separately with a trusted authentication.
 *	The input buffer is packed as follows:
 *
 *     4 byte length of user name,
 *     user name in UTF8 encoding,
 *     4 byte length of samba server challenge,
 *     samba server challenge
 *     4 byte length of the NTLMv2 client "blob"
 *     the client "blob" which includes 16 bytes of
 *     client digest prefixed to the the blob data
 *     4 byte length of the user name used to calculate the digest,
 *     the user name used to calculate the digest  in UTF8 encoding
 *     4 byte length of the samba domain,
 *     the samba domain in UTF8 encoding
 */
#define		kDSStdAuthNTLMv2						"dsAuthMethodStandard:dsAuthNodeNTLMv2"

/*!
 * @defined kDSStdAuthNTLMv2WithSessionKey
 * @discussion An optimized method that checks the user's challenge and response
 *	and retrieves session keys in a single call. If the NTLMv2 session key is
 *	supported, it is returned in the step buffer.
 *
 *	The input buffer is packed as follows:
 *
 *     4 byte length of user name,
 *     user name in UTF8 encoding,
 *     4 byte length of samba server challenge,
 *     samba server challenge
 *     4 byte length of the NTLMv2 client "blob"
 *     the client "blob" which includes 16 bytes of
 *     client digest prefixed to the the blob data
 *     4 byte length of the user name used to calculate the digest,
 *     the user name used to calculate the digest  in UTF8 encoding
 *     4 byte length of the samba domain,
 *     the samba domain in UTF8 encoding,
 *     4 byte length of authenticator name,
 *     user name in UTF8 encoding,
 *     4 byte length of authenticator password,
 *     user name in UTF8 encoding,
 */
#define		kDSStdAuthNTLMv2WithSessionKey			"dsAuthMethodStandard:dsAuthNodeNTLMv2WithSessionKey"

/*!
 * @defined kDSStdAuthNewUser
 * @discussion
 *     Create a new user record with the authentication authority
 *	   This authentication method is only implemented by the PasswordServer node.
 *     The buffer is packed as follows:
 *
 *     4 byte length of authenticator's Password Server ID,
 *     authenticator's Password Server ID in UTF8 encoding,
 *     4 byte length of authenticator's password,
 *     authenticator's password in UTF8 encoding
 *     4 byte length of new user's short-name,
 *     user's short-name,
 *     4 byte length of new user's password,
 *     user's password
 */
#define		kDSStdAuthNewUser						"dsAuthMethodStandard:dsAuthNewUser"

/*!
 * @defined kDSStdAuthNewUserWithPolicy
 * @discussion
 *     Create a new user record with the authentication authority and initial policy settings
 *	   This authentication method is only implemented by the PasswordServer node.
 *     The buffer is packed as follows:
 *
 *     4 byte length of authenticator's Password Server ID,
 *     authenticator's Password Server ID in UTF8 encoding,
 *     4 byte length of authenticator's password,
 *     authenticator's password in UTF8 encoding
 *     4 byte length of new user's short-name,
 *     user's short-name,
 *     4 byte length of new user's password,
 *     user's password
 *     4 byte length of policy string 
 *     policy string in UTF8 encoding
 */
#define		kDSStdAuthNewUserWithPolicy				"dsAuthMethodStandard:dsAuthNewUserWithPolicy"

/*!
 * @defined kDSStdAuthNodeNativeClearTextOK
 * @discussion The plug-in should determine which specific authentication method to use.
 *     The buffer is packed as follows:
 *
 *     4 byte length of user name,
 *     user name in UTF8 encoding,
 *     4 byte length of password,
 *     password in UTF8 encoding
 *
 *     The plug-in may choose to use a cleartext authentication method if necessary.
 */
#define		kDSStdAuthNodeNativeClearTextOK			"dsAuthMethodStandard:dsAuthNodeNativeCanUseClearText"

/*!
 * @defined kDSStdAuthNodeNativeNoClearText
 * @discussion The plug-in should determine which specific authentication method to use.
 *     The buffer is packed as follows:
 *
 *     4 byte length of user name,
 *     user name in UTF8 encoding,
 *     4 byte length of password,
 *     password in UTF8 encoding
 *
 *     The plug-in must not use an authentication method that sends the password in cleartext.
 */
#define		kDSStdAuthNodeNativeNoClearText			"dsAuthMethodStandard:dsAuthNodeNativeCannotUseClearText"

/*!
 * @defined kDSStdAuthReadSecureHash
 * @discussion Returns the SHA1 or salted SHA1 hash for a local user
 *     Only accessible by root processes. Only implemented by the local node.
 *     The buffer is packed as follows:
 *
 *     4 byte length of user's name,
 *     user's name in UTF8 encoding
 *
 *     The output buffer format:
 *     4 byte length (20 or 24)
 *     value, either the old 20-byte SHA1 or the new salted 24-byte SHA1.
 */
#define		kDSStdAuthReadSecureHash				"dsAuthMethodStandard:dsAuthReadSecureHash"

/*!
 * @defined kDSStdAuthSMBNTv2UserSessionKey
 * @discussion generate the ntlm-v2 user session key. Requires prior authentication with a trusted
 *	   authentication method.
 *     The buffer is packed as follows:
 *
 *     4 byte length of user name,
 *     user name in UTF8 encoding,
 *     4 byte length of server challenge (always 8 bytes)
 *     8 byte server challenge
 *     4 byte length of client response 
 *     client response buffer
 */
#define		kDSStdAuthSMBNTv2UserSessionKey			"dsAuthMethodStandard:dsSMBNTv2UserSessionKey"

/*!
 * @defined kDSStdAuthSMBWorkstationCredentialSessionKey
 * @discussion 
 *     The buffer is packed as follows:
 *
 *     4 byte length of user name,
 *     user name in UTF8 encoding,
 *     4 byte length of server challenge and client challenge (always 16 bytes)
 *     8 byte server challenge + 8 byte client challenge
 */
#define		kDSStdAuthSMBWorkstationCredentialSessionKey	"dsAuthMethodStandard:dsAuthSMBWorkstationCredentialSessionKey"

/*!
 * @defined kDSStdAuthSMB_LM_Key
 * @discussion SMB Lan Manager authentication method.
 *     The buffer is packed as follows:
 *
 *     4 byte length of user name,
 *     user name in UTF8 encoding,
 *     4 byte length of server challenge (always 8 bytes)
 *     8 byte server challenge
 *     4 byte length of client response (always 24 bytes)
 *     24 byte client response
 */
#define		kDSStdAuthSMB_LM_Key					"dsAuthMethodStandard:dsAuthSMBLMKey"

/*!
 * @defined kDSStdAuthSMB_NT_Key
 * @discussion SMB NT authentication method.
 *     The buffer is packed as follows:
 *
 *     4 byte length of user name,
 *     user name in UTF8 encoding,
 *     4 byte length of server challenge (always 8 bytes)
 *     8 byte server challenge
 *     4 byte length of client response (always 24 bytes)
 *     24 byte client response
 */
#define		kDSStdAuthSMB_NT_Key					"dsAuthMethodStandard:dsAuthSMBNTKey"

/*!
 * @defined kDSStdAuthSMB_NT_UserSessionKey
 * @discussion Used by Samba to get session keys
 *         This authentication method is only implemented by the PasswordServer node.
 *     The buffer is packed as follows:
 *
 *     4 byte length of user's Password Server ID,
 *     Password Server ID in UTF8 encoding
 *
 *     The output buffer format:
 *     4 byte length,
 *     MD4( ntHash )
 */
#define		kDSStdAuthSMB_NT_UserSessionKey			"dsAuthMethodStandard:dsAuthSMBNTUserSessionKey"

/*!
 * @defined kDSStdAuthSMB_NT_WithSessionKey
 * @discussion Used by Samba to authenticate and get session keys
 *		The buffer is packed as follows:
 *
 *     4 byte length of user name,
 *     user name in UTF8 encoding,
 *     4 byte length of server challenge (always 8 bytes)
 *     8 byte server challenge
 *     4 byte length of client response (always 24 bytes)
 *     24 byte client response
 *     4 byte length of authenticator name,
 *     authenticator name in UTF8 encoding,
 *     4 byte length of authenticator password,
 *     authenticator password in UTF8 encoding,
 *
 *     The output buffer format:
 *     4 byte length,
 *     MD4( ntHash )
 */
#define		kDSStdAuthSMB_NT_WithUserSessionKey		"dsAuthMethodStandard:dsAuthNTWithSessionKey"

/*!
 * @defined kDSStdAuthSecureHash
 * @discussion Auth specifically using the secure hash.
 *     The buffer is packed as follows:
 *
 *     4 byte length of user name,
 *     user name in UTF8 encoding,
 *     4 byte length of salted SHA1 hash (24 bytes),
 *     salted SHA1 hash
 */
#define		kDSStdAuthSecureHash					"dsAuthMethodStandard:dsAuthSecureHash"

/*!
 * @defined kDSStdAuthSetGlobalPolicy
 * @discussion Used to set the global policy.
 *     The buffer is packed as follows:
 *
 *     4 byte length of authenticator name or Password Server ID,
 *     user name or Password Server ID in UTF8 encoding
 *     4 byte length of authenticator password,
 *     password in UTF8 encoding
 *     4 byte length of policy string,
 *     policy string in UTF8 encoding
 */
#define		kDSStdAuthSetGlobalPolicy				"dsAuthMethodStandard:dsAuthSetGlobalPolicy"

/*!
 * @defined kDSStdAuthSetLMHash
 * @discussion Set the LAN Manager hash for an account. This method requires prior authentication.
 *	   Setting the LM hash for an account instead of the plain text password can cause the Windows
 *	   password to get out-of-sync with the password for other services. Therefore, this
 *	   authentication method should only be used when there is no other choice.
 *     The buffer is packed as follows:
 *
 *     4 byte length of user name,
 *     user name in UTF8 encoding,
 *     4 byte length of LAN Manager hash (16 bytes),
 *     LAN Manager hash buffer
 */
#define		kDSStdAuthSetLMHash						"dsAuthMethodStandard:dsAuthSetLMHash"

/*!
 * @defined kDSStdAuthSetNTHash
 * @discussion Set the NT hash for a user. This method requires prior authentication.
 *	   Setting the NT hash for an account instead of the plain text password can cause the Windows
 *	   password to get out-of-sync with the password for other services. Therefore, this
 *	   authentication method should only be used when there is no other choice.
 *     The buffer is packed as follows:
 *
 *     4 byte length of user name,
 *     user name in UTF8 encoding,
 *     4 byte length of NT hash (16 bytes),
 *     NT hash buffer
 */
#define		kDSStdAuthSetNTHash						"dsAuthMethodStandard:dsAuthSetNTHash"

/*!
 * @defined kDSStdAuthSetPasswd
 * @discussion Set password method.
 *     The buffer is packed as follows:
 *
 *     4 byte length of user name,
 *     user name in UTF8 encoding,
 *     4 byte length of new password,
 *     new password in UTF8 encoding
 *     4 byte length of authenticator's name,
 *     authenticator's name in UTF8 encoding,
 *     4 byte length of authenticator's password,
 *     authenticator's password in UTF8 encoding
 */
#define		kDSStdAuthSetPasswd						"dsAuthMethodStandard:dsAuthSetPasswd"

/*!
 * @defined kDSStdAuthSetPasswdAsRoot
 * @discussion Set password as root user method.
 *     The buffer is packed as follows:
 *
 *     4 byte length of user name,
 *     user name in UTF8 encoding,
 *     4 byte length of new password,
 *     new password in UTF8 encoding
 */
#define		kDSStdAuthSetPasswdAsRoot				"dsAuthMethodStandard:dsAuthSetPasswdAsRoot"

/*!
 * @defined kDSStdAuthSetPolicy
 * @discussion The plug-in should determine which specific authentication method to use.
 *     The buffer is packed as follows:
 *
 *     4 byte length of authenticator's name or Password Server ID,
 *     authenticator's name or Password Server ID in UTF8 encoding,
 *     4 byte length of authenticator's password,
 *     authenticator's password in UTF8 encoding
 *     4 byte length of name or Password Server ID of the target account,
 *     name or Password Server ID of the target account in UTF8 encoding
 *     4 byte length of policy data,
 *     policy data
 *
 */
#define		kDSStdAuthSetPolicy						"dsAuthMethodStandard:dsAuthSetPolicy"

/*!
 * @defined kDSStdAuthSetPolicyAsRoot
 * @discussion A two-item buffer version of set policy for the password server.
 *     The buffer is packed as follows:
 *
 *     4 byte length of user name or Password Server ID of the target account,
 *     user name or Password Server ID of the target account in UTF8 encoding
 *     4 byte length of policy data,
 *     policy data
 */
#define		kDSStdAuthSetPolicyAsRoot				"dsAuthMethodStandard:dsAuthSetPolicyAsRoot"

/*!
 * @defined kDSStdAuthSetUserData
 * @discussion Used for Apple password server.
 *	   This authentication method is only implemented by the PasswordServer node.
 *     The buffer is packed as follows:
 *
 *     4 byte length of authenticator's Password Server ID,
 *     authenticator's Password Server ID in UTF8 encoding,
 *     4 byte length of authenticator's password,
 *     authenticator's password in UTF8 encoding
 *     4 byte length of user's Password Server ID,
 *     Password Server ID in UTF8 encoding
 *     4 byte length of data to store,
 *     data
 */
#define		kDSStdAuthSetUserData					"dsAuthMethodStandard:dsAuthSetUserData"

/*!
 * @defined kDSStdAuthSetUserName
 * @discussion Used for Apple password server.
 *	   This authentication method is only implemented by the PasswordServer node.
 *     The buffer is packed as follows:
 *
 *     4 byte length of authenticator's Password Server ID,
 *     authenticator's Password Server ID in UTF8 encoding,
 *     4 byte length of authenticator's password,
 *     authenticator's password in UTF8 encoding
 *     4 byte length of user's Password Server ID,
 *     Password Server ID in UTF8 encoding
 *     4 byte length of user's short name,
 *     user's short name in UTF8 encoding
 */
#define		kDSStdAuthSetUserName					"dsAuthMethodStandard:dsAuthSetUserName"

/*!
 * @defined kDSStdAuthSetWorkstationPasswd
 * @discussion Supports PDC SMB interaction with DS.
 *     The buffer is packed as follows:
 *
 *     4 byte length of workstation's Password Server ID,
 *     workstation's Password Server ID in UTF8 encoding,
 *     4 byte length of NT hash (16 bytes),
 *     NT hash
 */
#define		kDSStdAuthSetWorkstationPasswd			"dsAuthMethodStandard:dsAuthSetWorkstationPasswd"

/*!
 * @defined kDSStdAuthWithAuthorizationRef
 * @discussion
 *     Allows access to local directories as root with a valid AuthorizationRef.
 *
 *     The input buffer format:
 *     externalized AuthorizationRef
 */
#define		kDSStdAuthWithAuthorizationRef			"dsAuthMethodStandard:dsAuthWithAuthorizationRef"

/*!
 * @defined kDSStdAuthWriteSecureHash
 * @discussion
 *     Supports ONLY a root process to be able to directly write the secure hash of a user record.
 *     The buffer is packed as follows:
 *
 *     4 byte length of user name,
 *     user name in UTF8 encoding,
 *     4 byte length of salted SHA1 hash (24 bytes),
 *     salted SHA1 hash
 */
#define		kDSStdAuthWriteSecureHash				"dsAuthMethodStandard:dsAuthWriteSecureHash"

/*!
 * @defined kDSStdAuthGetMethodsForUser
 * @discussion
 *     Allows a service to query the authentication methods available for a user at the
 *	   time of authentication. 
 *     The buffer is packed as follows:
 *
 *     4 byte length of user name,
 *     user name in UTF8 encoding.
 *
 *     The output buffer contains a list of the form:
 *     4 byte length,
 *     authentication method constant (example: "dsAuthMethodStandard:dsAuthNodeCRAM-MD5")
 *     4 byte length,
 *     next authentication method constant, etc.
 *
 */
#define		kDSStdAuthGetMethodsForUser				"dsAuthMethodStandard:dsAuthGetMethodsForUser"

#pragma mark -
#pragma mark NSL Neighborhood Constants
#pragma mark -

/*!
 * @functiongroup NSL Neighborhood Constants
 */

/*!
 * @defined kDSValueNSLDynamicNeighborhoodType
 * @discussion Value type of Neighborhood record
 */
#define		kDSValueNSLDynamicNeighborhoodType		"NSLDynamicNeighborhoodType"

/*!
 * @defined kDSValueNSLLocalNeighborhoodType
 * @discussion Value type of Neighborhood record
 */
#define		kDSValueNSLLocalNeighborhoodType		"NSLLocalNeighborhoodType"

/*!
 * @defined kDSValueNSLStaticNeighborhoodType
 * @discussion Value type of Neighborhood record
 */
#define		kDSValueNSLStaticNeighborhoodType		"NSLStaticNeighborhoodType"

/*!
 * @defined kDSValueNSLTopLevelNeighborhoodType
 * @discussion Value type of Neighborhood record
 */
#define		kDSValueNSLTopLevelNeighborhoodType		"NSLTopLevelNeighborhoodType"

#pragma mark -
#pragma mark Configure Node attribute type Constants
#pragma mark -

/*!
 * @functiongroup Configure Node attribute type Constants
 */
 
/*!
 * @defined kDS1AttrBuildVersion
 * @discussion Build version for reference.
 */
#define		kDS1AttrBuildVersion				"dsAttrTypeStandard:BuildVersion"

/*!
 * @defined kDS1AttrConfigAvail
 * @discussion Config avail tag.
 */
#define		kDS1AttrConfigAvail					"dsAttrTypeStandard:ConfigAvail"

/*!
 * @defined kDS1AttrConfigFile
 * @discussion Config file name.
 */
#define		kDS1AttrConfigFile					"dsAttrTypeStandard:ConfigFile"

/*!
 * @defined kDS1AttrCoreFWVersion
 * @discussion Core FW version for reference.
 */
#define		kDS1AttrCoreFWVersion					"dsAttrTypeStandard:CoreFWVersion"

/*!
 * @defined kDS1AttrFunctionalState
 * @discussion Functional state of plugin for example.
 */
#define		kDS1AttrFunctionalState				"dsAttrTypeStandard:FunctionalState"

/*!
 * @defined kDS1AttrFWVersion
 * @discussion FW version for reference.
 */
#define		kDS1AttrFWVersion					"dsAttrTypeStandard:FWVersion"

/*!
 * @defined kDS1AttrPluginIndex
 * @discussion Plugin index for reference.
 */
#define		kDS1AttrPluginIndex					"dsAttrTypeStandard:PluginIndex"

/*!
 * @defined kDS1AttrRefNumTableList
 * @discussion Summary of the reference table entries presented as attr values from the Configure node via dsGetDirNodeInfo.
 */
#define		kDS1AttrRefNumTableList				"dsAttrTypeStandard:RefNumTableList"

/*!
 * @defined kDS1AttrVersion
 * @discussion Version label.
 */
#define		kDS1AttrVersion						"dsAttrTypeStandard:Version"

/*!
 * @defined kDS1AttrPIDValue
 * @discussion PID value.
 */
#define		kDS1AttrPIDValue					"dsAttrTypeStandard:PIDValue"

/*!
 * @defined kDS1AttrProcessName
 * @discussion Process Name.
 */
#define		kDS1AttrProcessName					"dsAttrTypeStandard:ProcessName"

/*!
 * @defined kDS1AttrTotalRefCount
 * @discussion Total count of references for a process.
 */
#define		kDS1AttrTotalRefCount				"dsAttrTypeStandard:TotalRefCount"

/*!
 * @defined kDS1AttrDirRefCount
 * @discussion Directory reference count for a process.
 */
#define		kDS1AttrDirRefCount					"dsAttrTypeStandard:DirRefCount"

/*!
 * @defined kDS1AttrNodeRefCount
 * @discussion Node reference count for a process.
 */
#define		kDS1AttrNodeRefCount				"dsAttrTypeStandard:NodeRefCount"

/*!
 * @defined kDS1AttrRecRefCount
 * @discussion Record reference count for a process.
 */
#define		kDS1AttrRecRefCount					"dsAttrTypeStandard:RecRefCount"

/*!
 * @defined kDS1AttrAttrListRefCount
 * @discussion Attr List reference count for a process.
 */
#define		kDS1AttrAttrListRefCount			"dsAttrTypeStandard:AttrListRefCount"

/*!
 * @defined kDS1AttrAttrListValueRefCount
 * @discussion Attr List Value reference count for a process.
 */
#define		kDS1AttrAttrListValueRefCount		"dsAttrTypeStandard:AttrListValueRefCount"

/*!
 * @defined kDSNAttrDirRefs
 * @discussion All the directory references for a process.
 */
#define		kDSNAttrDirRefs						"dsAttrTypeStandard:DirRefs"

/*!
 * @defined kDSNAttrNodeRefs
 * @discussion All the node references for a process.
 */
#define		kDSNAttrNodeRefs					"dsAttrTypeStandard:NodeRefs"

/*!
 * @defined kDSNAttrRecRefs
 * @discussion All the record references for a process.
 */
#define		kDSNAttrRecRefs						"dsAttrTypeStandard:RecRefs"

/*!
 * @defined kDSNAttrAttrListRefs
 * @discussion All the attr list references for a process.
 */
#define		kDSNAttrAttrListRefs				"dsAttrTypeStandard:AttrListRefs"

/*!
 * @defined kDSNAttrAttrListValueRefs
 * @discussion All the attr list value references for a process.
 */
#define		kDSNAttrAttrListValueRefs			"dsAttrTypeStandard:AttrListValueRefs"

#endif
