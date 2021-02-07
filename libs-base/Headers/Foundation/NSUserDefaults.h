/* Interface for <NSUserDefaults> for GNUStep
   Copyright (C) 1995, 1996 Free Software Foundation, Inc.

   Written by:   Georg Tuparev, EMBL & Academia Naturalis, 
                Heidelberg, Germany
                Tuparev@EMBL-Heidelberg.de
   
   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
   MA 02111 USA.
*/ 

#ifndef __NSUserDefaults_h_OBJECTS_INCLUDE
#define __NSUserDefaults_h_OBJECTS_INCLUDE
#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSObject.h>
#import <Foundation/NSString.h>

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSArray;
@class NSMutableArray;
@class NSDictionary;
@class NSMutableDictionary;
@class NSData;
@class NSTimer;
@class NSRecursiveLock;
@class NSDistributedLock;

/* Standard domains */

/**
 * User defaults domain for process arguments.  Command-line arguments
 * (key-value pairs, as in "-NSFoo bar") are placed in this domain.<br />
 * Where there is a sequence of arguments beginning with '-', only the
 * last one is used (so "-a -b -c d" will produce a single user default
 * 'c' with value 'd').<br />
 * NB. On OSX the argument "-" means a key consisting of an empty string
 * (so you can't use a '-' as a default value), while in GNUstep a "-" is
 * a special case which does not mean a default key (so '-' may be used
 * as a value).<br />
 */
GS_EXPORT NSString* const NSArgumentDomain;

/**
 *  User defaults domain for system defaults.
 */
GS_EXPORT NSString* const NSGlobalDomain;

/**
 *  User defaults domain for application-registered "default defaults".
 */
GS_EXPORT NSString* const NSRegistrationDomain;

#if	!NO_GNUSTEP
/**
 *  User defaults domain for GNUstep config file and for any defaults
 *  stored in the GlobalDefaults.plist file alongside the config file.
 */
GS_EXPORT NSString* const GSConfigDomain;
#endif


/* Public notification */

/**
 *  Notification posted when a defaults synchronize has been performed (see
 *  [NSUserDefaults-synchronize]) and changes have been loaded in from disk.
 */
GS_EXPORT NSString* const NSUserDefaultsDidChangeNotification;

/* Backwards compatibility */
#define	NSUserDefaultsChanged NSUserDefaultsDidChangeNotification

/* Keys for language-dependent information */

/** Key for locale dictionary: names of days of week. */
GS_EXPORT NSString* const NSWeekDayNameArray;

/** Key for locale dictionary: abbreviations of days of week. */
GS_EXPORT NSString* const NSShortWeekDayNameArray;

/** Key for locale dictionary: names of months of year. */
GS_EXPORT NSString* const NSMonthNameArray;

/** Key for locale dictionary: abbreviations of months of year. */
GS_EXPORT NSString* const NSShortMonthNameArray;

/** Key for locale dictionary: format string for feeding to [NSDateFormatter].*/
GS_EXPORT NSString* const NSTimeFormatString;

/** Key for locale dictionary: format string for feeding to [NSDateFormatter].*/
GS_EXPORT NSString* const NSDateFormatString;

/** Key for locale dictionary: format string for feeding to [NSDateFormatter].*/
GS_EXPORT NSString* const NSShortDateFormatString;

/** Key for locale dictionary: format string for feeding to [NSDateFormatter].*/
GS_EXPORT NSString* const NSTimeDateFormatString;

/** Key for locale dictionary: format string for feeding to [NSDateFormatter].*/
GS_EXPORT NSString* const NSShortTimeDateFormatString;

/** Key for locale dictionary: currency symbol. */
GS_EXPORT NSString* const NSCurrencySymbol;

/** Key for locale dictionary: decimal separator. */
GS_EXPORT NSString* const NSDecimalSeparator;

/** Key for locale dictionary: thousands separator. */
GS_EXPORT NSString* const NSThousandsSeparator;

/** Key for locale dictionary: three-letter ISO 4217 currency abbreviation. */
GS_EXPORT NSString* const NSInternationalCurrencyString;

/** Key for locale dictionary: text formatter string for monetary amounts. */
GS_EXPORT NSString* const NSCurrencyString;

/** Key for locale dictionary: array of strings for 0-9. */
GS_EXPORT NSString* const NSDecimalDigits;

/** Key for locale dictionary: array of strings for AM and PM. */
GS_EXPORT NSString* const NSAMPMDesignation;

#if OS_API_VERSION(GSAPI_MACOSX, GS_API_LATEST)

/**
 *  Array of arrays of NSStrings, first member of each specifying a time,
 *  followed by one or more colloquial names for the time, as in "(0,
 *  midnight), (12, noon, lunch)".
 */
GS_EXPORT NSString* const NSHourNameDesignations;

/** Strings for "year", "month", "week". */
GS_EXPORT NSString* const NSYearMonthWeekDesignations;

/** Key for locale dictionary: adjectives that modify values in
    NSYearMonthWeekDesignations, as in "last", "previous", etc.. */
GS_EXPORT NSString* const NSEarlierTimeDesignations;

/** Key for locale dictionary: adjectives that modify values in
    NSYearMonthWeekDesignations, as in "next", "subsequent", etc.. */
GS_EXPORT NSString* const NSLaterTimeDesignations;

/** Key for locale dictionary: one or more strings designating the current
    day, such as "today". */
GS_EXPORT NSString* const NSThisDayDesignations;

/** Key for locale dictionary: one or more strings designating the next
    day, such as "tomorrow". */
GS_EXPORT NSString* const NSNextDayDesignations;

/** Key for locale dictionary: one or more strings designating the next
    day, such as "day after tomorrow". */
GS_EXPORT NSString* const NSNextNextDayDesignations;

/** Key for locale dictionary: one or more strings designating the previous
    day, such as "yesterday". */
GS_EXPORT NSString* const NSPriorDayDesignations;

/** Key for locale dictionary: string with 'Y', 'M', 'D', and 'H' designating
    the default method of writing dates, as in "MDYH" for the U.S.. */
GS_EXPORT NSString* const NSDateTimeOrdering;

/** Key for locale dictionary: name of language. */
GS_EXPORT NSString* const NSLanguageName;

/** Key for locale dictionary: two-letter ISO code. */
GS_EXPORT NSString* const NSLanguageCode;

/** Key for locale dictionary: formal name of language. */
GS_EXPORT NSString* const NSFormalName;
#if	!NO_GNUSTEP
/** Key for locale dictionary: name of locale. */
GS_EXPORT NSString* const GSLocale;
#endif
#endif

/* General implementation notes: 

   OpenStep spec currently is neither complete nor consistent. Therefore
   we had to make several implementation decisions which may vary in
   other OpenStep implementations.
  
  - We add two new class methods for getting and setting a list of 
    user languages (userLanguages and setUserLanguages: ). They are 
    somehow equivalent to the NS3.x Application's systemLanguages 
    method.

  - Definition of argument (command line parameters)
  	(-GSxxxx || --GSxxx) [value]
	
  To Do: 
	- polish & optimize;
	- when tested, fix NSBundle (the system languages stuff);
	- write docs : -(
	*/

GS_EXPORT_CLASS
@interface NSUserDefaults : NSObject
{
#if	GS_EXPOSE(NSUserDefaults)
@private
  NSMutableArray	*_searchList;    // Current search list;
  NSMutableDictionary	*_persDomains;   // Contains persistent defaults info;
  NSMutableDictionary	*_tempDomains;   // Contains volatile defaults info;
  NSMutableArray	*_changedDomains; /* ..after first time that persistent 
					    user defaults are changed */
  NSDictionary		*_dictionaryRep; // Cached dictionary representation
  NSString		*_defaultsDatabase;
  NSDate		*_lastSync;
  NSRecursiveLock	*_lock;
  NSDistributedLock	*_fileLock;
#endif
#if     GS_NONFRAGILE
#else
  /* Pointer to private additional data used to avoid breaking ABI
   * when we don't have the non-fragile ABI available.
   * Use this mechanism rather than changing the instance variable
   * layout (see Source/GSInternal.h for details).
   */
  @private id _internal GS_UNUSED_IVAR;
#endif
}

/**
 * Returns the shared defaults object. If it doesn't exist yet, it's
 * created. The defaults are initialized for the current user.
 * The search list is guaranteed to be standard only the first time
 * this method is invoked. The shared instance is provided as a
 * convenience; other instances may also be created.
 */
+ (NSUserDefaults*) standardUserDefaults;

#if OS_API_VERSION(GSAPI_MACOSX, GS_API_LATEST)
/**
 * Resets the shared user defaults object to reflect the current
 * user ID.  Needed by setuid processes which change the user they
 * are running as.<br />
 * In GNUstep you should call GSSetUserName() when changing your
 * effective user ID, and that function will call this function for you.
 */
+ (void) resetStandardUserDefaults;
#endif

#if OS_API_VERSION(GSAPI_NONE, GSAPI_NONE)
/**
 * Returns the array of user languages preferences.  Uses the
 * <em>NSLanguages</em> user default if available, otherwise
 * tries to infer setup from operating system information etc
 * (in particular, uses the <em>LANGUAGES</em> environment variable).
 */
+ (NSArray*) userLanguages;

/**
 * Sets the array of user languages preferences.  Places the specified
 * array in the <em>NSLanguages</em> user default.
 */
+ (void) setUserLanguages: (NSArray*)languages;
#endif

#if OS_API_VERSION(GSAPI_MACOSX, GS_API_LATEST)
/**
 * Adds the domain names aName to the search list of the receiver.<br />
 * The domain is added after the application domain.<br />
 * Suites may be removed using the -removeSuiteNamed: method.
 */
- (void) addSuiteNamed: (NSString*)aName;
#endif

/**
 * Looks up a value for a specified default using -objectForKey:
 * and checks that it is an NSArray object.  Returns nil if it is not.
 */
- (NSArray*) arrayForKey: (NSString*)defaultName;

/**
 * Looks up a value for a specified default using -objectForKey:
 * and returns its boolean representation.<br />
 * Returns NO if it is not a boolean.<br />
 * The text 'yes' or 'true' or any non zero numeric value is considered
 * to be a boolean YES.  Other string values are NO.<br />
 * NB. This differs slightly from the documented behavior for MacOS-X
 * (August 2002) in that the GNUstep version accepts the string 'TRUE'
 * as equivalent to 'YES'.
 */
- (BOOL) boolForKey: (NSString*)defaultName;

/**
 * Looks up a value for a specified default using -objectForKey:
 * and checks that it is an NSData object.  Returns nil if it is not.
 */
- (NSData*) dataForKey: (NSString*)defaultName;

/**
 * Looks up a value for a specified default using -objectForKey:
 * and checks that it is an NSDictionary object.  Returns nil if it is not.
 */
- (NSDictionary*) dictionaryForKey: (NSString*)defaultName;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
/**
 * Looks up a value for a specified default using -objectForKey:
 * and checks that it is a double.  Returns 0.0 if it is not.
 */
- (double) doubleForKey: (NSString*)defaultName;
#endif

/**
 * Looks up a value for a specified default using -objectForKey:
 * and checks that it is a float.  Returns 0.0 if it is not.
 */
- (float) floatForKey: (NSString*)defaultName;

/**
 * Initializes defaults for current user calling initWithUser:
 */
- (id) init;

/** <init />
 * Initializes defaults for the specified user.<br />
 * Returns an object with an empty search list.
 */
- (id) initWithUser: (NSString*)userName;

/**
 * Looks up a value for a specified default using -objectForKey:
 * and returns its integer value or 0 if it is not representable
 * as an integer.
 */
- (NSInteger) integerForKey: (NSString*)defaultName;

/**
 * Looks up a value for a specified default using.
 * The lookup is performed by accessing the domains in the order
 * given in the search list.
 * <br />Returns nil if defaultName cannot be found.
 */
- (id) objectForKey: (NSString*)defaultName;

/**
 * Removes the default with the specified name from the application
 * domain.
 */
- (void) removeObjectForKey: (NSString*)defaultName;

#if OS_API_VERSION(GSAPI_MACOSX, GS_API_LATEST)
/**
 * Removes the named domain from the search list of the receiver.<br />
 * Suites may be added using the -addSuiteNamed: method.
 */
- (void) removeSuiteNamed: (NSString*)aName;
#endif

/**
 * Returns an array listing the domains searched in order to look up
 * a value in the defaults system.  The order of the names in the
 * array is the order in which the domains are searched.
 */
- (NSArray*) searchList;

/**
 * Sets a boolean value for defaultName in the application domain.<br />
 * Calls -setObject:forKey: to make the change by storing a string
 * containing either the word YES or NO.
 */
- (void) setBool: (BOOL)value forKey: (NSString*)defaultName;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
/**
 * Sets a double value for defaultName in the application domain.<br />
 * Calls -setObject:forKey: to make the change by storing a double
 * [NSNumber] instance.
 */
- (void) setDouble: (double)value forKey: (NSString*)defaultName;
#endif

/**
 * Sets a float value for defaultName in the application domain.<br />
 * Calls -setObject:forKey: to make the change by storing a float
 * [NSNumber] instance.
 */
- (void) setFloat: (float)value forKey: (NSString*)defaultName;

/**
 * Sets an integer value for defaultName in the application domain.<br />
 * Calls -setObject:forKey: to make the change by storing an intege
 * [NSNumber] instance.
 */
- (void) setInteger: (NSInteger)value forKey: (NSString*)defaultName;

/**
 * Sets a copy of an object value for defaultName in the
 * application domain.<br />
 * The defaultName must be a non-empty string.<br />
 * The value to be copied into the domain must be an instance
 * of one of the [NSString-propertyList] classes.<br />
 * <p>Causes a NSUserDefaultsDidChangeNotification to be posted
 * if this is the first change to a persistent-domain since the
 * last -synchronize.
 * </p>
 * If value is nil, this is equivalent to the -removeObjectForKey: method.
 */
- (void) setObject: (id)value forKey: (NSString*)defaultName;

/**
 * Sets the list of the domains searched in order to look up
 * a value in the defaults system.  The order of the names in the
 * array is the order in which the domains are searched.<br />
 * On lookup, the first match is used.
 */
- (void) setSearchList: (NSArray*)newList;

/**
 * Calls -arrayForKey: to get an array value for defaultName and checks
 * that the array contents are string objects ... if not, returns nil.
 */
- (NSArray*) stringArrayForKey: (NSString*)defaultName;

/**
 * Looks up a value for a specified default using -objectForKey:
 * and checks that it is an NSString.  Returns nil if it is not.
 */
- (NSString*) stringForKey: (NSString*)defaultName;

/**
 * Returns the persistent domain specified by domainName.
 */
- (NSDictionary*) persistentDomainForName: (NSString*)domainName;

/**
 * Returns an array listing the name of all the persistent domains.
 */
- (NSArray*) persistentDomainNames;

/**
 * Removes the persistent domain specified by domainName from the
 * user defaults.
 * <br />Causes a NSUserDefaultsDidChangeNotification to be posted
 * if this is the first change to a persistent-domain since the
 * last -synchronize.
 */
- (void) removePersistentDomainForName: (NSString*)domainName;

/**
 * Replaces the persistent-domain specified by domainName with
 * domain ... a dictionary containing keys and defaults values.
 * <br />Raises an NSInvalidArgumentException if domainName already
 * exists as a volatile-domain.
 * <br />Causes a NSUserDefaultsDidChangeNotification to be posted
 * if this is the first change to a persistent-domain since the
 * last -synchronize.
 */
- (void) setPersistentDomain: (NSDictionary*)domain 
		     forName: (NSString*)domainName;

/**
 * Ensures that the in-memory and on-disk representations of the defaults
 * are in sync.  You may call this yourself, but probably don't need to
 * since it is invoked at intervals whenever a runloop is running.<br />
 * If any persistent domain is changed by reading new values from disk,
 * an NSUserDefaultsDidChangeNotification is posted.
 */
- (BOOL) synchronize;

/**
 * Removes the volatile domain specified by domainName from the
 * user defaults.
 */
- (void) removeVolatileDomainForName: (NSString*)domainName;

/**
 * Sets the volatile-domain specified by domainName to
 * domain ... a dictionary containing keys and defaults values.<br />
 * Raises an NSInvalidArgumentException if domainName already
 * exists as either a volatile-domain or a persistent-domain.
 */
- (void) setVolatileDomain: (NSDictionary*)domain 
		   forName: (NSString*)domainName;

/**
 * Returns the volatile domain specified by domainName.
 */
- (NSDictionary*) volatileDomainForName: (NSString*)domainName;

/**
 * Returns an array listing the name of all the volatile domains.
 */
- (NSArray*) volatileDomainNames;

/**
 * Returns a dictionary representing the current state of the defaults
 * system ... this is a merged version of all the domains in the
 * search list.
 */
- (NSDictionary*) dictionaryRepresentation;

/**
 * Merges the contents of the dictionary newVals into the registration
 * domain.  Registration defaults may be added to or replaced using this
 * method, but may never be removed.  Thus, setting registration defaults
 * at any point in your program guarantees that the defaults will be
 * available thereafter.
 */
- (void) registerDefaults: (NSDictionary*)newVals;
@end

#if	defined(__cplusplus)
}
#endif

#endif /* __NSUserDefaults_h_OBJECTS_INCLUDE */
