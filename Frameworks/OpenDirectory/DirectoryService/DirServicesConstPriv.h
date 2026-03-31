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
 * @header DirServicesConstPriv
 * @discussion This header contains well known record, attribute and
 * authentication type constants plus others.
 * The attribute and record constants are generally used with the
 * API calls dsDataNodeAllocateString() and dsBuildListFromStrings()
 * to create proper data type arguments for the search methods in the
 * Directory Services API.
 * The auth constants are used with dsDataNodeAllocateString().
 */

#ifndef __DirServicesConstPriv_h__
#define	__DirServicesConstPriv_h__	1

/*!
 * @functiongroup DirectoryService Private Constants
 */

/*!
 * @defined kDSStdAuthNewComputer
 * @discussion
 *     Create a new computer record
 *	   This authentication method is only implemented by the PasswordServer node.
 *     The buffer is packed as follows:
 *
 *     4 byte length of authenticator's Password Server ID,
 *     authenticator's Password Server ID,
 *     4 byte length of authenticator's password,
 *     authenticator's password,
 *     4 byte length of new computer's short-name,
 *     computer's short-name,
 *     4 byte length of new computer's password,
 *     computer's password,
 *     4 byte length of owner list,
 *     comma separated list of user slot IDs that can administer the computer account
 */
#define		kDSStdAuthNewComputer					"dsAuthMethodStandard:dsAuthNewComputer"

/*!
 * @defined kDSStdAuthSetComputerAcctPasswdAsRoot
 * @discussion Set password for a computer account using the
 *		current credentials.
 *     The buffer is packed as follows:
 *
 *     4 byte length of user name,
 *     user name in UTF8 encoding,
 *     4 byte length of new password,
 *     new password in UTF8 encoding
 *     4 byte length of service list,
 *     comma-delimited service list,
 *     4 byte length of hostname list,
 *	   comma-delimited hostname list,
 *     4 byte length of local KDC realm,
 *     local KDC realm
 */
#define		kDSStdAuthSetComputerAcctPasswdAsRoot	"dsAuthMethodStandard:dsAuthSetComputerAcctPasswdAsRoot"

/*!
 * @defined kDSStdAuthNodeNativeRetainCredential
 * @discussion The plug-in should determine which specific authentication method to use.
 *		This auth method is identical to kDSStdAuthNodeNativeClearTextOK, except that
 *		it retains the authentication for future calls to dsDoDirNodeAuth(). The behavior
 *		differs from setting authOnly=false in that the method does not try to get write
 *		access to the directory node and therefore doesn't redirect to the master LDAP server.
 *
 *     The buffer is packed as follows:
 *
 *     4 byte length of user name,
 *     user name in UTF8 encoding,
 *     4 byte length of password,
 *     password in UTF8 encoding
 *
 *     The plug-in may choose to use a cleartext authentication method if necessary.
 */
#define		kDSStdAuthNodeNativeRetainCredential			"dsAuthMethodStandard:dsAuthNodeNativeRetainCredential"

/*!
 * @defined kDSNAttrOriginalAuthenticationAuthority
 * @discussion Used by security agent to store copies of auth authority on the local node
 */
#define		kDSNAttrOriginalAuthenticationAuthority		"dsAttrTypeStandard:OriginalAuthenticationAuthority"

/*!
 * @defined kDSNAttrTrustInformation
 * @discussion Clients can use with dsGetDirNodeInfo calls to verify trust information with the directory.
 *             Values include FullTrust, PartialTrust, Authenticated, or Anonymous.
 *             Any combination of the values can be used to signify multiple states or maximum value.
 */
#define		kDSNAttrTrustInformation					"dsAttrTypeStandard:TrustInformation"

/*!
    @defined kDS1AttrOperatingSystem
    @abstract   This returns either server or client operating system
    @discussion Returns one of two values "Mac OS X Server" or "Mac OS X".
*/
#define		kDS1AttrOperatingSystem						"dsAttrTypeStandard:OperatingSystem"

/*!
    @defined kDSNAttrKerberosServices
    @abstract   This is used to store the principals in host records (i.e., "host", "vnc", etc.)
    @discussion This is used to store the principals in host records (i.e., "host", "vnc", etc.)
*/
#define		kDSNAttrKerberosServices			"dsAttrTypeStandard:KerberosServices"

/*!
    @defined	kDSNAttrAltSecurityIdentities
    @abstract   Used to store alternate identities for the record
    @discussion Used to store alternate identities for the record. Values will have standardized form as
				specified by Microsoft LDAP schema (1.2.840.113556.1.4.867).
 
 				Kerberos:user\@REALM
*/
#define		kDSNAttrAltSecurityIdentities		"dsAttrTypeStandard:AltSecurityIdentities"

/*!
    @defined	kDS1AttrHardwareUUID
    @abstract   Used to store the UUID of the hardware
    @discussion Used to store the UUID of the hardware
*/
#define		kDS1AttrHardwareUUID			"dsAttrTypeStandard:HardwareUUID"

/*!
	@defined kDS1AttrOperatingSystemVersion
	@abstract   This returns the version of operating system
	@discussion Returns the version of the operating system "10.6"
 */
#define		kDS1AttrOperatingSystemVersion				"dsAttrTypeStandard:OperatingSystemVersion"

/*!
 * @defined kDSNotifyGlobalRecordUpdatePrefix
 * @discussion Can be used in conjunction with arbitrary types "users", "groups", etc.
 *             Example:  kDSNotifyGlobalRecordUpdatePrefix "users"
 */
#define		kDSNotifyGlobalRecordUpdatePrefix			"com.apple.system.DirectoryService.update."

/*!
 * @defined kDSNotifyLocalRecordUpdatePrefix
 * @discussion Can be used in conjunction with arbitrary types "users", "groups", etc.
 *             Example:  kDSNotifyLocalRecordUpdatePrefix "users"
 */
#define		kDSNotifyLocalRecordUpdatePrefix			"com.apple.system.DirectoryService.update.Local."

/*!
 * @defined kDSNotifyLocalRecordUpdateUsers
 * @discussion Notification sent when a local user(s) record is updated
 */
#define		kDSNotifyLocalRecordUpdateUsers				"com.apple.system.DirectoryService.update.Local.users"

/*!
 * @defined kDSNotifyLocalRecordUpdateGroups
 * @discussion Notification sent when a local group(s) record is updated
 */
#define		kDSNotifyLocalRecordUpdateGroups				"com.apple.system.DirectoryService.update.Local.groups"

/*!
 * @defined kDSStdAuthSetCertificateHashAsRoot
 * @discussion Set certificate using the authenticated user's credentials.
 *     The buffer is packed as follows:
 *
 *     4 byte length of user name,
 *     user name in UTF8 encoding,
 *     4 byte length of certificate hash (40),
 *     hashed certificate data (40 hex characters)
 */
#define		kDSStdAuthSetCertificateHashAsRoot				"dsAuthMethodStandard:dsAuthSetCertificateHashAsRoot"

/*!
 * @defined kDSStdAuthSASLProxy
 * @discussion Allow a DS client to proxy a generic SASL negotiation through DS.
 *     The buffer is packed as follows:
 *
 *     4 byte length of user name,
 *     user name in UTF8 encoding,
 *     4 byte length of SASL mechanism,
 *     SASL mechanism in UTF8 encoding,
 *     4 byte length of data from sasl_client_start() or sasl_client_step()
 *     data from sasl_client_start() or sasl_client_step()
 *
 *     The step buffer contains the reply from the OD node formatted:
 *	   4 byte length of sasl_server_xxx() data
 *	   sasl_server_xxx() data
 *
 *     For session security, it is essential that a new nodeRef is acquired for
 *     each user.
 */
#define		kDSStdAuthSASLProxy								"dsAuthMethodStandard:dsAuthSASLProxy"

/*!
 * @defined kDSValueAuthAuthorityKerberosv5Cert
 * @discussion Standard auth authority value for Kerberos v5 authentication.
 */
#define		kDSValueAuthAuthorityKerberosv5Cert				";Kerberosv5Cert;"

/*!
 * @defined kDSTagAuthAuthorityKerberosv5Cert
 * @discussion Standard center tag data of auth authority value for Kerberos v5 authentication.
 */
#define		kDSTagAuthAuthorityKerberosv5Cert				"Kerberosv5Cert"

/*!
 * @defined kDSStdMachMembershipPortName
 * @discussion Registered name used with mach_init for DirectoryService Membership MIG server for the DirectoryService daemon.
 */
#define		kDSStdMachMembershipPortName	"com.apple.system.DirectoryService.membership_v1"

#endif	// __DirServicesConstPriv_h__
