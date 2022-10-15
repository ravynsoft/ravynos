/*	CFPreferences.h
	Copyright (c) 1998-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
*/

#if !defined(__COREFOUNDATION_CFPREFERENCES__)
#define __COREFOUNDATION_CFPREFERENCES__ 1

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFString.h>

CF_IMPLICIT_BRIDGING_ENABLED
CF_ASSUME_NONNULL_BEGIN
CF_EXTERN_C_BEGIN

CF_EXPORT
const CFStringRef kCFPreferencesAnyApplication;
CF_EXPORT
const CFStringRef kCFPreferencesCurrentApplication;
CF_EXPORT
const CFStringRef kCFPreferencesAnyHost;
CF_EXPORT
const CFStringRef kCFPreferencesCurrentHost;
CF_EXPORT
const CFStringRef kCFPreferencesAnyUser;
CF_EXPORT
const CFStringRef kCFPreferencesCurrentUser;

/* NOTE: All CFPropertyListRef values returned from
         CFPreferences API should be assumed to be immutable.
*/

/*	The "App" functions search the various sources of defaults that
	apply to the given application, and should never be called with
	kCFPreferencesAnyApplication - only kCFPreferencesCurrentApplication
	or an application's ID (its bundle identifier).
*/

/* Searches the various sources of application defaults to find the
value for the given key. key must not be NULL.  If a value is found,
it returns it; otherwise returns NULL.  Caller must release the
returned value */
CF_EXPORT
_Nullable CFPropertyListRef CFPreferencesCopyAppValue(CFStringRef key, CFStringRef applicationID);

/* Convenience to interpret a preferences value as a boolean directly.
Returns false if the key doesn't exist, or has an improper format; under
those conditions, keyExistsAndHasValidFormat (if non-NULL) is set to false */
CF_EXPORT
Boolean CFPreferencesGetAppBooleanValue(CFStringRef key, CFStringRef applicationID,  Boolean * _Nullable keyExistsAndHasValidFormat);

/* Convenience to interpret a preferences value as an integer directly.
Returns 0 if the key doesn't exist, or has an improper format; under
those conditions, keyExistsAndHasValidFormat (if non-NULL) is set to false */
CF_EXPORT
CFIndex CFPreferencesGetAppIntegerValue(CFStringRef key, CFStringRef applicationID, Boolean * _Nullable keyExistsAndHasValidFormat);

/* Sets the given value for the given key in the "normal" place for
application preferences.  key must not be NULL.  If value is NULL,
key is removed instead. */
CF_EXPORT
void CFPreferencesSetAppValue(CFStringRef key, _Nullable CFPropertyListRef value, CFStringRef applicationID);

/* Adds the preferences for the given suite to the app preferences for
   the specified application.  To write to the suite domain, use
   CFPreferencesSetValue(), below, using the suiteName in place
   of the appName */
CF_EXPORT
void CFPreferencesAddSuitePreferencesToApp(CFStringRef applicationID, CFStringRef suiteID);

CF_EXPORT
void CFPreferencesRemoveSuitePreferencesFromApp(CFStringRef applicationID, CFStringRef suiteID);

/* Writes all changes in all sources of application defaults.
Returns success or failure. */
CF_EXPORT
Boolean CFPreferencesAppSynchronize(CFStringRef applicationID);

/* The primitive get mechanism; all arguments must be non-NULL
(use the constants above for common values).  Only the exact
location specified by app-user-host is searched.  The returned
CFType must be released by the caller when it is finished with it. */
CF_EXPORT
_Nullable CFPropertyListRef CFPreferencesCopyValue(CFStringRef key, CFStringRef applicationID, CFStringRef userName, CFStringRef hostName);

/* Convenience to fetch multiple keys at once.  Keys in 
keysToFetch that are not present in the returned dictionary
are not present in the domain.  If keysToFetch is NULL, all
keys are fetched. */
CF_EXPORT
CFDictionaryRef CFPreferencesCopyMultiple(_Nullable CFArrayRef keysToFetch, CFStringRef applicationID, CFStringRef userName, CFStringRef hostName);

/* The primitive set function; all arguments except value must be
non-NULL.  If value is NULL, the given key is removed */
CF_EXPORT
void CFPreferencesSetValue(CFStringRef key, _Nullable CFPropertyListRef value, CFStringRef applicationID, CFStringRef userName, CFStringRef hostName);

/* Convenience to set multiple values at once.  Behavior is undefined
if a key is in both keysToSet and keysToRemove */
CF_EXPORT
void CFPreferencesSetMultiple(_Nullable CFDictionaryRef keysToSet, _Nullable CFArrayRef keysToRemove, CFStringRef applicationID, CFStringRef userName, CFStringRef hostName);

CF_EXPORT
Boolean CFPreferencesSynchronize(CFStringRef applicationID, CFStringRef userName, CFStringRef hostName);

/* Constructs and returns the list of the name of all applications
which have preferences in the scope of the given user and host, or NULL if no applications are there.
The returned value must be released by the caller; neither argument
may be NULL. Does not supported sandboxed applications. */
CF_EXPORT
_Nullable CFArrayRef CFPreferencesCopyApplicationList(CFStringRef userName, CFStringRef hostName) API_DEPRECATED("Unsupported API", macos(10.0,10.9), ios(2.0,7.0), watchos(2.0,2.0), tvos(9.0,9.0));

/* Constructs and returns the list of all keys set in the given
location, or NULL if no keys are set.  The returned value must be released by the caller;
all arguments must be non-NULL */
CF_EXPORT
_Nullable CFArrayRef CFPreferencesCopyKeyList(CFStringRef applicationID, CFStringRef userName, CFStringRef hostName);


CF_EXTERN_C_END
CF_ASSUME_NONNULL_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif /* ! __COREFOUNDATION_CFPREFERENCES__ */

