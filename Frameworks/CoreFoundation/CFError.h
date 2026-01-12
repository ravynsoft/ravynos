/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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

/*	CFError.h
	Copyright (c) 2006-2014, Apple Inc. All rights reserved.
*/

/*!
	@header CFError
        @discussion
            CFErrors are used to encompass information about errors. At minimum, errors are identified by their domain (a string) and an error code within that domain. In addition a "userInfo" dictionary supplied at creation time enables providing additional info that might be useful for the interpretation and reporting of the error. This dictionary can even contain an "underlying" error, which is wrapped as an error bubbles up through various layers. 
            
            CFErrors have the ability to provide human-readable descriptions for the errors; in fact, they are designed to provide localizable, end-user presentable errors that can appear in the UI. CFError has a number of predefined userInfo keys to enable developers to supply the info.
            
            Usage recommendation for CFErrors is to return them as by-ref parameters in functions. This enables the caller to pass NULL in when they don't actually want information about the error. The presence of an error should be reported by other means, for instance a NULL or false return value from the function call proper:
            
            CFError *error;
            if (!ReadFromFile(fd, &error)) {
                ... process error ...
                CFRelease(error);   // If an error occurs, the returned CFError must be released.
            }
            
            It is the responsibility of anyone returning CFErrors this way to:
            - Not touch the error argument if no error occurs
            - Create and assign the error for return only if the error argument is non-NULL
            
            In addition, it's recommended that CFErrors be used in error situations only (not status), and where there are multiple possible errors to distinguish between. For instance there is no plan to add CFErrors to existing APIs in CF which currently don't return errors; in many cases, there is one possible reason for failure, and a false or NULL return is enough to indicate it.

            CFError is toll-free bridged to NSError in Foundation. NSError in Foundation has some additional guidelines which makes it easy to automatically report errors to users and even try to recover from them.  See http://developer.apple.com/documentation/Cocoa/Conceptual/ErrorHandlingCocoa/ErrorHandling/chapter_1_section_1.html for more info on NSError programming guidelines.
*/

#if !defined(__COREFOUNDATION_CFERROR__)
#define __COREFOUNDATION_CFERROR__ 1

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFDictionary.h>

CF_IMPLICIT_BRIDGING_ENABLED
CF_EXTERN_C_BEGIN

/*!
	@typedef CFErrorRef
	    This is the type of a reference to CFErrors.  CFErrorRef is toll-free bridged with NSError.
*/
typedef struct CF_BRIDGED_TYPE(NSError) __CFError * CFErrorRef;

/*!
	@function CFErrorGetTypeID
	    Returns the type identifier of all CFError instances.
*/
CF_EXPORT
CFTypeID CFErrorGetTypeID(void) CF_AVAILABLE(10_5, 2_0);


// Predefined domains; value of "code" will correspond to preexisting values in these domains.
CF_EXPORT const CFStringRef kCFErrorDomainPOSIX		    CF_AVAILABLE(10_5, 2_0);
CF_EXPORT const CFStringRef kCFErrorDomainOSStatus	    CF_AVAILABLE(10_5, 2_0);
CF_EXPORT const CFStringRef kCFErrorDomainMach		    CF_AVAILABLE(10_5, 2_0);
CF_EXPORT const CFStringRef kCFErrorDomainCocoa		    CF_AVAILABLE(10_5, 2_0);

// Keys in userInfo for localizable, end-user presentable error messages. At minimum provide one of first two; ideally provide all three.
CF_EXPORT const CFStringRef kCFErrorLocalizedDescriptionKey         CF_AVAILABLE(10_5, 2_0);   // Key to identify the end user-presentable description in userInfo.
CF_EXPORT const CFStringRef kCFErrorLocalizedFailureReasonKey       CF_AVAILABLE(10_5, 2_0);   // Key to identify the end user-presentable failure reason in userInfo.
CF_EXPORT const CFStringRef kCFErrorLocalizedRecoverySuggestionKey  CF_AVAILABLE(10_5, 2_0);   // Key to identify the end user-presentable recovery suggestion in userInfo.

// If you do not have localizable error strings, you can provide a value for this key instead.
CF_EXPORT const CFStringRef kCFErrorDescriptionKey                  CF_AVAILABLE(10_5, 2_0);   // Key to identify the description in the userInfo dictionary. Should be a complete sentence if possible. Should not contain domain name or error code.

// Other keys in userInfo.
CF_EXPORT const CFStringRef kCFErrorUnderlyingErrorKey              CF_AVAILABLE(10_5, 2_0);   // Key to identify the underlying error in userInfo.
CF_EXPORT const CFStringRef kCFErrorURLKey                          CF_AVAILABLE(10_7, 5_0);    // Key to identify associated URL in userInfo.  Typically one of this or kCFErrorFilePathKey is provided.
CF_EXPORT const CFStringRef kCFErrorFilePathKey                     CF_AVAILABLE(10_7, 5_0);    // Key to identify associated file path in userInfo.    Typically one of this or kCFErrorURLKey is provided.


/*!
	@function CFErrorCreate
	@abstract Creates a new CFError.
	@param allocator The CFAllocator which should be used to allocate memory for the error. This parameter may be NULL in which case the 
	    current default CFAllocator is used. If this reference is not a valid CFAllocator, the behavior is undefined.
	@param domain A CFString identifying the error domain. If this reference is NULL or is otherwise not a valid CFString, the behavior is undefined.
	@param code A CFIndex identifying the error code. The code is interpreted within the context of the error domain.
	@param userInfo A CFDictionary created with kCFCopyStringDictionaryKeyCallBacks and kCFTypeDictionaryValueCallBacks. It will be copied with CFDictionaryCreateCopy(). 
	    If no userInfo dictionary is desired, NULL may be passed in as a convenience, in which case an empty userInfo dictionary will be assigned.
	@result A reference to the new CFError.
*/
CF_EXPORT
CFErrorRef CFErrorCreate(CFAllocatorRef allocator, CFStringRef domain, CFIndex code, CFDictionaryRef userInfo) CF_AVAILABLE(10_5, 2_0);

/*!
	@function CFErrorCreateWithUserInfoKeysAndValues
	@abstract Creates a new CFError without having to create an intermediate userInfo dictionary.
	@param allocator The CFAllocator which should be used to allocate memory for the error. This parameter may be NULL in which case the 
	    current default CFAllocator is used. If this reference is not a valid CFAllocator, the behavior is undefined.
	@param domain A CFString identifying the error domain. If this reference is NULL or is otherwise not a valid CFString, the behavior is undefined.
	@param code A CFIndex identifying the error code. The code is interpreted within the context of the error domain.
	@param userInfoKeys An array of numUserInfoValues CFStrings used as keys in creating the userInfo dictionary. NULL is valid only if numUserInfoValues is 0.
	@param userInfoValues An array of numUserInfoValues CF types used as values in creating the userInfo dictionary.  NULL is valid only if numUserInfoValues is 0.
	@param numUserInfoValues CFIndex representing the number of keys and values in the userInfoKeys and userInfoValues arrays.
	@result A reference to the new CFError. numUserInfoValues CF types are gathered from each of userInfoKeys and userInfoValues to create the userInfo dictionary.
*/
CF_EXPORT
CFErrorRef CFErrorCreateWithUserInfoKeysAndValues(CFAllocatorRef allocator, CFStringRef domain, CFIndex code, const void *const *userInfoKeys, const void *const *userInfoValues, CFIndex numUserInfoValues) CF_AVAILABLE(10_5, 2_0);

/*!
	@function CFErrorGetDomain
	@abstract Returns the error domain the CFError was created with.
	@param err The CFError whose error domain is to be returned. If this reference is not a valid CFError, the behavior is undefined.
	@result The error domain of the CFError. Since this is a "Get" function, the caller shouldn't CFRelease the return value.
*/
CF_EXPORT
CFStringRef CFErrorGetDomain(CFErrorRef err) CF_AVAILABLE(10_5, 2_0);

/*!
	@function CFErrorGetCode
	@abstract Returns the error code the CFError was created with.
	@param err The CFError whose error code is to be returned. If this reference is not a valid CFError, the behavior is undefined.
	@result The error code of the CFError (not an error return for the current call).
*/
CF_EXPORT
CFIndex CFErrorGetCode(CFErrorRef err) CF_AVAILABLE(10_5, 2_0);

/*!
	@function CFErrorCopyUserInfo
        @abstract Returns CFError userInfo dictionary.
	@discussion Returns a dictionary containing the same keys and values as in the userInfo dictionary the CFError was created with. Returns an empty dictionary if NULL was supplied to CFErrorCreate().
	@param err The CFError whose error user info is to be returned. If this reference is not a valid CFError, the behavior is undefined.
	@result The user info of the CFError.
*/
CF_EXPORT
CFDictionaryRef CFErrorCopyUserInfo(CFErrorRef err) CF_AVAILABLE(10_5, 2_0);

/*!
	@function CFErrorCopyDescription
	@abstract Returns a human-presentable description for the error. CFError creators should strive to make sure the return value is human-presentable and localized by providing a value for kCFErrorLocalizedDescriptionKey at the time of CFError creation.
        @discussion This is a complete sentence or two which says what failed and why it failed. Rules for computing the return value:
            - Look for kCFErrorLocalizedDescriptionKey in the user info and if not NULL, returns that as-is.  
            - Otherwise, if there is a kCFErrorLocalizedFailureReasonKey in the user info, generate an error from that. Something like: "Operation code not be completed. " + kCFErrorLocalizedFailureReasonKey
            - Otherwise, generate a semi-user presentable string from kCFErrorDescriptionKey, the domain, and code. Something like: "Operation could not be completed. Error domain/code occurred. " or "Operation could not be completed. " + kCFErrorDescriptionKey + " (Error domain/code)"
            Toll-free bridged NSError instances might provide additional behaviors for manufacturing a description string.  Do not count on the exact contents or format of the returned string, it might change.
	@param err The CFError whose description is to be returned. If this reference is not a valid CFError, the behavior is undefined.
	@result A CFString with human-presentable description of the CFError. Never NULL.
*/
CF_EXPORT
CFStringRef CFErrorCopyDescription(CFErrorRef err) CF_AVAILABLE(10_5, 2_0);

/*!
	@function CFErrorCopyFailureReason
        @abstract Returns a human-presentable failure reason for the error.  May return NULL.  CFError creators should strive to make sure the return value is human-presentable and localized by providing a value for kCFErrorLocalizedFailureReasonKey at the time of CFError creation.
        @discussion This is a complete sentence which describes why the operation failed. In many cases this will be just the "because" part of the description (but as a complete sentence, which makes localization easier). By default this looks for kCFErrorLocalizedFailureReasonKey in the user info. Toll-free bridged NSError instances might provide additional behaviors for manufacturing this value. If no user-presentable string is available, returns NULL.
            Example Description: "Could not save file 'Letter' in folder 'Documents' because the volume 'MyDisk' doesn't have enough space."
            Corresponding FailureReason: "The volume 'MyDisk' doesn't have enough space."
	@param err The CFError whose failure reason is to be returned. If this reference is not a valid CFError, the behavior is undefined.
	@result A CFString with the localized, end-user presentable failure reason of the CFError, or NULL. 
*/
CF_EXPORT
CFStringRef CFErrorCopyFailureReason(CFErrorRef err) CF_AVAILABLE(10_5, 2_0);

/*!
	@function CFErrorCopyRecoverySuggestion
        @abstract Returns a human presentable recovery suggestion for the error.  May return NULL.  CFError creators should strive to make sure the return value is human-presentable and localized by providing a value for kCFErrorLocalizedRecoverySuggestionKey at the time of CFError creation.
        @discussion This is the string that can be displayed as the "informative" (aka "secondary") message on an alert panel. By default this looks for kCFErrorLocalizedRecoverySuggestionKey in the user info. Toll-free bridged NSError instances might provide additional behaviors for manufacturing this value. If no user-presentable string is available, returns NULL.
            Example Description: "Could not save file 'Letter' in folder 'Documents' because the volume 'MyDisk' doesn't have enough space."
            Corresponding RecoverySuggestion: "Remove some files from the volume and try again."
	@param err The CFError whose recovery suggestion is to be returned. If this reference is not a valid CFError, the behavior is undefined.
	@result A CFString with the localized, end-user presentable recovery suggestion of the CFError, or NULL. 
*/
CF_EXPORT
CFStringRef CFErrorCopyRecoverySuggestion(CFErrorRef err) CF_AVAILABLE(10_5, 2_0);



CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif /* ! __COREFOUNDATION_CFERROR__ */

