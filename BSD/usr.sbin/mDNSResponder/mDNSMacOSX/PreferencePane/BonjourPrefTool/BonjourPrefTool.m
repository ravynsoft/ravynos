/*
 *
 * Copyright (c) 2016 Apple Inc. All rights reserved.
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

#import "BonjourPrefTool.h"
#import "BonjourSCStore.h"
#import <Security/Security.h>
#import <dns_sd.h>

#define DYNDNS_KEYCHAIN_DESCRIPTION "Dynamic DNS Key"

#pragma mark - Keychain Funcs

static SecAccessRef
MyMakeUidAccess(uid_t uid)
{
    // make the "uid/gid" ACL subject
    // this is a CSSM_LIST_ELEMENT chain
    CSSM_ACL_PROCESS_SUBJECT_SELECTOR selector = {
        CSSM_ACL_PROCESS_SELECTOR_CURRENT_VERSION,	// selector version
        CSSM_ACL_MATCH_UID,	// set mask: match uids (only)
        uid,				// uid to match
        0					// gid (not matched here)
    };
    CSSM_LIST_ELEMENT subject2 = { NULL, 0, 0, {{0,0,0}} };
    subject2.Element.Word.Data = (UInt8 *)&selector;
    subject2.Element.Word.Length = sizeof(selector);
    CSSM_LIST_ELEMENT subject1 = { &subject2, CSSM_ACL_SUBJECT_TYPE_PROCESS, CSSM_LIST_ELEMENT_WORDID, {{0,0,0}} };
    
    
    // rights granted (replace with individual list if desired)
    CSSM_ACL_AUTHORIZATION_TAG rights[] = {
        CSSM_ACL_AUTHORIZATION_ANY	// everything
    };
    // owner component (right to change ACL)
    CSSM_ACL_OWNER_PROTOTYPE owner = {
        // TypedSubject
        { CSSM_LIST_TYPE_UNKNOWN, &subject1, &subject2 },
        // Delegate
        false
    };
    // ACL entries (any number, just one here)
    CSSM_ACL_ENTRY_INFO acls =
    {
        // CSSM_ACL_ENTRY_PROTOTYPE
        {
            { CSSM_LIST_TYPE_UNKNOWN, &subject1, &subject2 }, // TypedSubject
            false,	// Delegate
            { sizeof(rights) / sizeof(rights[0]), rights }, // Authorization rights for this entry
            { { 0, 0 }, { 0, 0 } }, // CSSM_ACL_VALIDITY_PERIOD
            "" // CSSM_STRING EntryTag
        },
        // CSSM_ACL_HANDLE
        0
    };
    
    SecAccessRef a = NULL;
    (void) SecAccessCreateFromOwnerAndACL(&owner, 1, &acls, &a);
    return a;
}

static OSStatus
MyAddDynamicDNSPassword(SecKeychainRef keychain, SecAccessRef a, UInt32 serviceNameLength, const char *serviceName,
                        UInt32 accountNameLength, const char *accountName, UInt32 passwordLength, const void *passwordData)
{
    char * description       = DYNDNS_KEYCHAIN_DESCRIPTION;
    UInt32 descriptionLength = strlen(DYNDNS_KEYCHAIN_DESCRIPTION);
    UInt32 type              = 'ddns';
    UInt32 creator           = 'ddns';
    UInt32 typeLength        = sizeof(type);
    UInt32 creatorLength     = sizeof(creator);
    OSStatus err;
    
    // set up attribute vector (each attribute consists of {tag, length, pointer})
    SecKeychainAttribute attrs[] = { { kSecLabelItemAttr,       serviceNameLength,   (char *)serviceName },
        { kSecAccountItemAttr,     accountNameLength,   (char *)accountName },
        { kSecServiceItemAttr,     serviceNameLength,   (char *)serviceName },
        { kSecDescriptionItemAttr, descriptionLength,   (char *)description },
        { kSecTypeItemAttr,               typeLength, (UInt32 *)&type       },
        { kSecCreatorItemAttr,         creatorLength, (UInt32 *)&creator    } };
    SecKeychainAttributeList attributes = { sizeof(attrs) / sizeof(attrs[0]), attrs };
    
    err = SecKeychainItemCreateFromContent(kSecGenericPasswordItemClass, &attributes, passwordLength, passwordData, keychain, a, NULL);
    return err;
}

static int
SetKeychainEntry(NSDictionary * secretDictionary)
// Create a new entry in system keychain, or replace existing
{
    CFStringRef         keyNameString;
    CFStringRef         domainString;
    CFStringRef         secretString;
    SecKeychainItemRef	item = NULL;
    int					result = 0;
    char                keyname[kDNSServiceMaxDomainName];
    char                domain[kDNSServiceMaxDomainName];
    char                secret[kDNSServiceMaxDomainName];
    
    keyNameString = (__bridge CFStringRef)[secretDictionary objectForKey:(NSString *)SC_DYNDNS_KEYNAME_KEY];
    require(keyNameString != NULL, exit);
    
    domainString  = (__bridge CFStringRef)[secretDictionary objectForKey:(NSString *)SC_DYNDNS_DOMAIN_KEY];
    require(domainString != NULL, exit);
    
    secretString  = (__bridge CFStringRef)[secretDictionary objectForKey:(NSString *)SC_DYNDNS_SECRET_KEY];
    require(secretString != NULL, exit);
    
    CFStringGetCString(keyNameString, keyname, kDNSServiceMaxDomainName, kCFStringEncodingUTF8);
    CFStringGetCString(domainString,   domain, kDNSServiceMaxDomainName, kCFStringEncodingUTF8);
    CFStringGetCString(secretString,   secret, kDNSServiceMaxDomainName, kCFStringEncodingUTF8);
    
    result = SecKeychainSetPreferenceDomain(kSecPreferencesDomainSystem);
    if (result == noErr)
    {
        result = SecKeychainFindGenericPassword(NULL, strlen(domain), domain, 0, NULL, 0, NULL, &item);
        if (result == noErr)
        {
            result = SecKeychainItemDelete(item);
            if (result != noErr) fprintf(stderr, "SecKeychainItemDelete returned %d\n", result);
        }
        
        result = MyAddDynamicDNSPassword(NULL, MyMakeUidAccess(0), strlen(domain), domain, strlen(keyname)+1, keyname, strlen(secret)+1, secret);
        if (result != noErr) fprintf(stderr, "MyAddDynamicDNSPassword returned %d\n", result);
        if (item) CFRelease(item);
    }
    
exit:
    return result;
}


@implementation BonjourPrefTool

- (void) setKeychainEntry:(NSDictionary *_Nonnull)secretDictionary withStatus:(void (^ _Nonnull)(OSStatus))status
{
    OSStatus result;
    
    result = SetKeychainEntry (secretDictionary);
//    NSLog(@"setKeychainEntry: %@ result: %d", secretDictionary, result);
   
    status (result);
}

@end
