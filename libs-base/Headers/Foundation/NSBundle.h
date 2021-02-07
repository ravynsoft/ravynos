/** Interface for NSBundle for GNUStep   -*-objc-*-
   Copyright (C) 1995, 1997, 1999, 2001, 2002 Free Software Foundation, Inc.

   Written by:  Adam Fedor <fedor@boulder.colorado.edu>
   Date: 1995

   Updates by various authors.
   Documentation by Nicola Pero <n.pero@mi.flashnet.it>
  
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
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
  */

#ifndef __NSBundle_h_GNUSTEP_BASE_INCLUDE
#define __NSBundle_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#if	defined(__cplusplus)
extern "C" {
#endif

#import	<Foundation/NSObject.h>
#import	<Foundation/NSString.h>

#ifdef __ANDROID__
#include <android/asset_manager_jni.h>
#endif

@class NSString;
@class NSArray;
@class NSDictionary;
@class NSMutableArray;
@class NSMutableDictionary;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5,GS_API_LATEST) 
enum {
  NSBundleExecutableArchitectureI386      = 0x00000007,
  NSBundleExecutableArchitecturePPC       = 0x00000012,
  NSBundleExecutableArchitecturePPC64     = 0x01000012,
  NSBundleExecutableArchitectureX86_64    = 0x01000007
};
#endif

/**
 *  Notification posted when a bundle is loaded.  The notification object is
 *  the [NSBundle] itself.  The notification also contains a <em>userInfo</em>
 *  dictionary, containing the single key '<code>NSLoadedClasses</code>',
 *  mapped to an [NSArray] containing the names of each class and category
 *  loaded (as strings).
 */
GS_EXPORT NSString* const NSBundleDidLoadNotification;

/**
 * A user default affecting the behavior of
 * [NSBundle-localizedStringForKey:value:table:].  If set, the value of the
 * key will be returned as an uppercase string rather than any localized
 * equivalent found.  This can be useful during development to check where
 * a given string in the UI is "coming from".
 */
GS_EXPORT NSString* const NSShowNonLocalizedStrings;

/**
 *  When an [NSBundle] loads classes and posts a
 *  <code>NSBundleDidLoadNotification</code>, its <em>userInfo</em> dictionary
 *  contains this key, mapped to an [NSArray] containing the names of each
 *  class and category loaded (as strings).
 */
GS_EXPORT NSString* const NSLoadedClasses;

/**
   <p>
   NSBundle provides methods for locating and handling application (and tool)
   resources at runtime. Resources includes any time of file that the
   application might need, such as images, nib (gorm or gmodel) files,
   localization files, and any other type of file that an application
   might need to use to function. Resources also include executable
   code, which can be dynamically linked into the application at
   runtime. These files and executable code are commonly put together
   into a directory called a bundle.
   </p>
   <p>
   NSBundle knows how these bundles are organized and can search for
   files inside a bundle. NSBundle also handles locating the
   executable code, linking this in and initializing any classes that
   are located in the code. NSBundle also handles Frameworks, which are
   basically a bundle that contains a library archive. The
   organization of a framework is a little difference, but in most
   respects there is no difference between a bundle and a framework.
   </p>
   <p>
   There is one special bundle, called the mainBundle, which is
   basically the application itself. The mainBundle is always loaded
   (of course), but you can still perform other operations on the
   mainBundle, such as searching for files, just as with any other
   bundle.
   </p>
*/
GS_EXPORT_CLASS
@interface NSBundle : NSObject
{
#if	GS_EXPOSE(NSBundle)
@public
  NSString		*_path;
  NSMutableArray	*_bundleClasses;
  Class			_principalClass;
  NSDictionary		*_infoDict;
  NSMutableDictionary	*_localizations;
  unsigned		_bundleType;
  BOOL			_codeLoaded;
  unsigned		_version;
  NSString      	*_frameworkVersion;
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

/** Return an array enumerating all the bundles in the application.  This
 *  does not include frameworks.  */
+ (NSArray*) allBundles;

/** Return an array enumerating all the frameworks in the application.  This
 *  does not include normal bundles.  */
+ (NSArray*) allFrameworks;

/**
 * <p>Return the bundle containing the resources for the executable.  If
 * the executable is an application, this is the main application
 * bundle (the xxx.app directory); if the executable is a tool, this
 * is a bundle 'naturally' associated with the tool: if the tool
 * executable is xxx/Tools/ix86/linux-gnu/gnu-gnu-gnu/Control then the
 * tool's main bundle directory is xxx/Tools/Resources/Control.
 * </p>
 * <p>NB: traditionally tools didn't have a main bundle -- this is a recent
 * GNUstep extension, but it's quite nice and it's here to stay.
 * </p>
 * <p>The main bundle is where the application should put all of its
 * resources, such as support files (images, html, rtf, txt, ...),
 * localization tables, .gorm (.nib) files, etc.  gnustep-make
 * (/ProjectCenter) allows you to easily specify the resource files to
 * put in the main bundle when you create an application or a tool.
 * </p>
 */
+ (NSBundle*) mainBundle;

/**
 * <p>Return the bundle to which aClass belongs.  If aClass was loaded
 * from a bundle, return the bundle; if it belongs to a framework
 * (either a framework linked into the application, or loaded
 * dynamically), return the framework; if it belongs to a library,
 * return the bundle for that library; in all other cases, return the
 * main bundle.
 * </p>
 * <p>Please note that GNUstep supports plain shared libraries, while the
 * openstep standard, and other openstep-like systems, do not; the
 * behaviour when aClass belongs to a plain shared library is to return
 * a bundle for that library, but might be changed. :-)
 * </p>
 */
+ (NSBundle*) bundleForClass: (Class)aClass;

/** Returns the bundle for the specified identifier (see -bundleIdentifier)
 * as long as the bundle has already been loaded.  This never causes a
 * bundle to be loaded.
 */
+ (NSBundle*) bundleWithIdentifier: (NSString*)identifier;

/** Return a bundle for the path at path.  If path doesn't exist or is
 * not readable, return nil.  If you want the main bundle of an
 * application or a tool, it's better if you use +mainBundle.  */
+ (NSBundle*) bundleWithPath: (NSString*)path;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_6,GS_API_LATEST) 
+ (NSBundle*) bundleWithURL: (NSURL*)url;
#endif

/**
  Returns an absolute path for a resource name with the extension
  in the specified bundlePath.  See also
  -pathForResource:ofType:inDirectory: for more information on
  searching a bundle.  
 */
+ (NSString*) pathForResource: (NSString*)name
		       ofType: (NSString*)extension
		  inDirectory: (NSString*)bundlePath;

/**
  This method has been deprecated. Version numbers were never implemented
  so this method behaves exactly like +pathForResource:ofType:inDirectory:.
 */
+ (NSString*) pathForResource: (NSString*)name
		       ofType: (NSString*)extension
		  inDirectory: (NSString*)bundlePath
		  withVersion: (int)version;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6,GS_API_LATEST) 
+ (NSURL*) URLForResource: (NSString*)name
            withExtension: (NSString*)extension
             subdirectory: (NSString*)subpath
          inBundleWithURL: (NSURL*)bundleURL;
#endif

/** <init />
 * Init the bundle for reading resources from path.<br />
 * The MacOS-X documentation says that the path must be a full path to
 * a directory on disk.  However, it (in MacOS-X) version 10.3 at least)
 * actually accepts relative paths too.<br />
 * The GNUstep behavior is similar in that it accepts a relative path,
 * but GNUstep converts it to an absolute path by referring to the
 * current working directory when the is initialised, so an absolute
 * path is then used and a warning message is printed.<br />
 * On MacOS-X using a bundle initialised with a relative path will cause
 * a crash if the current working directory is changed between the point
 * at which the bundle was initialised and that at which it is used.<br />
 * If path is nil or can't be accessed, initWithPath: deallocates the
 * receiver and returns nil.<br />
 * If a bundle for that path already existed, it is returned in place
 * of the receiver (and the receiver is deallocated).<br />
 * If the -bundleIdentifier is not nil, and a bundle with the same
 * identifier already exists, the existing bundle is returned in place
 * of the receiver (and the receiver is deallocated).
 */
- (id) initWithPath: (NSString*)path;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6,GS_API_LATEST) 
- (id) initWithURL: (NSURL*)url;
#endif

/** Return the path to the bundle - an absolute path.  */
- (NSString*) bundlePath;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6,GS_API_LATEST) 
- (NSURL*) bundleURL;
#endif

/** Returns the class in the bundle with the given name. If no class
    of this name exists in the bundle, then Nil is returned.
 */
- (Class) classNamed: (NSString*)className;

/** Returns the principal class of the bundle. This is the class
    specified by the NSPrincipalClass key in the Info-gnustep property
    list contained in the bundle. If this key or the specified class
    is not found, the class returned is arbitrary, although it is 
    typically the first class compiled into the archive.
 */
- (Class) principalClass;

/**
  <p> Returns an array of paths for all resources with the specified
   extension and residing in the bundlePath directory. bundlePath can
   be any type of directory structure, but typically it is used to
   search for resources in a application or framework. For example,
   one could search for tiff files in the MyApp.app application using [NSBundle
   pathsForResourcesOfType: @"tiff" inDirectory: @"MyApp.app"].  It
   will search in any Resources subdirectory inside bundlePath as well
   as the main directory for resource files. If extension is nil or
   empty, all resources are returned.  </p>
 */
+ (NSArray*) pathsForResourcesOfType: (NSString*)extension
			 inDirectory: (NSString*)bundlePath;

/**
  <p>
   Returns an array of paths for all resources with the specified
   extension and residing in the bundlePath directory. If extension is
   nil or empty, all bundle resources are returned.
   </p>
 */
- (NSArray*) pathsForResourcesOfType: (NSString*)extension
			 inDirectory: (NSString*)subPath;

/**
  <p>
   Returns an absolute path for a resource name with the extension
   in the specified bundlePath. Directories in the bundle are searched
   in the following order:
   </p>
   <example>
     root path/Resources/subPath
     root path/Resources/subPath/"language.lproj"
     root path/subPath
     root path/subPath/"language.lproj"
   </example>
   <p>
   where language.lproj can be any localized language directory inside
   the bundle.
   </p>
   <p>
   If extension is nil or empty, then the first file exactly matching name
   (ie with no extension) is returned.
   </p>
*/
- (NSString*) pathForResource: (NSString*)name
		       ofType: (NSString*)extension
		  inDirectory: (NSString*)subPath;

/**
   Returns an absolute path for a resource name with the extension
   in the receivers bundle path. 
   See -pathForResource:ofType:inDirectory:.
 */
- (NSString*) pathForResource: (NSString*)name
		       ofType: (NSString*)extension;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6,GS_API_LATEST) 
- (NSURL*) URLForResource: (NSString*)name
            withExtension: (NSString*)extension;
- (NSURL*) URLForResource: (NSString*)name
            withExtension: (NSString*)extension
             subdirectory: (NSString*)subpath;
- (NSURL*) URLForResource: (NSString*)name 
            withExtension: (NSString*)extension
             subdirectory: (NSString*)subpath
             localization: (NSString*)localizationName;
#endif

/**
 * <p>Returns the value for the key found in the strings file tableName, or
 * Localizable.strings if tableName is nil.
 * </p>
 * <p>If the user default <code>NSShowNonLocalizedStrings</code> is set, the
 * value of the key will be returned as an uppercase string rather than any
 * localized equivalent found.  This can be useful during development to check
 * where a given string in the UI is "coming from".</p>
 */
- (NSString*) localizedStringForKey: (NSString*)key
			      value: (NSString*)value
			      table: (NSString*)tableName;

/** Returns the absolute path to the resources directory of the bundle.  */
- (NSString*) resourcePath;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6,GS_API_LATEST)
/** Returns the absolute path to the resources directory of the bundle.  */
- (NSURL *) resourceURL;
#endif

/** Returns the full path to the plug-in subdirectory of the bundle.  */
- (NSString *) builtInPlugInsPath;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6,GS_API_LATEST) 
/** Returns the full path to the plug-in subdirectory of the bundle.  */
- (NSURL *) builtInPlugInsURL;
#endif

/** Returns the full path to the private frameworks subdirectory of the bundle.  */
- (NSString *) privateFrameworksPath;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6,GS_API_LATEST) 
/** Returns the full path to the private frameworks subdirectory of the bundle.  */
- (NSURL *) privateFrameworksURL;
#endif




/** Returns the bundle identifier, as defined by the CFBundleIdentifier
    key in the infoDictionary */
- (NSString *) bundleIdentifier;

/** Returns the bundle version. */
- (unsigned) bundleVersion;

/** Set the bundle version */
- (void) setBundleVersion: (unsigned)version;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
/**
 *  Returns subarray of given array containing those localizations that are
 *  used to locate resources given environment and user preferences.
 */
+ (NSArray *) preferredLocalizationsFromArray: (NSArray *)localizationsArray;

/**
 *  Returns subarray of given array containing those localizations that are
 *  used to locate resources given environment given user preferences (which
 *  are used instead of looking up the preferences of the current user).
 */
+ (NSArray *) preferredLocalizationsFromArray: (NSArray *)localizationsArray 
			       forPreferences: (NSArray *)preferencesArray;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_2,GS_API_LATEST) 
/**
 * Returns a boolean indicating whether code for the bundle has been loaded.
 */
- (BOOL) isLoaded;
#endif

/**
 * This method returns the same information as
 * -pathsForResourcesOfType:inDirectory: except that only non-localized
 * resources and resources that match the localization localizationName
 * are returned.<br />
 * The GNUstep implementation places localised resources in the array
 * before any non-localised resources.
 */
- (NSArray*) pathsForResourcesOfType: (NSString*)extension
			 inDirectory: (NSString*)subPath
		     forLocalization: (NSString*)localizationName;
/**
 * This is like -pathForResource:ofType:inDirectory: but returns only
 * resources matching localizationName (preferentially), or non-localized
 * resources.
 */
- (NSString*) pathForResource: (NSString*)name
		       ofType: (NSString*)extension
		  inDirectory: (NSString*)subPath
	      forLocalization: (NSString*)localizationName;

/** Returns the info property list associated with the bundle. */
- (NSDictionary*) infoDictionary;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_2,GS_API_LATEST) 
/** Returns a localized info property list based on the preferred
 *  localization or the most appropriate localization if the preferred
 *  one cannot be found.
 */
- (NSDictionary*) localizedInfoDictionary;

/** Not implemented
 */
- (NSString*) developmentLocalization;

/** Not implemented
 */
- (id) objectForInfoDictionaryKey: (NSString *)key;
#endif

/** Returns all the localizations in the bundle. */
- (NSArray*) localizations;

/**
 * Returns the list of localizations that the bundle uses to search
 * for information. This is based on the user's preferences.
 */
- (NSArray*) preferredLocalizations;

/** Loads any executable code contained in the bundle into the
    application. Load will be called implicitly if any information
    about the bundle classes is requested, such as -principalClass or
    -classNamed:. 
 */
- (BOOL) load;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_2,GS_API_LATEST) 
/** * Not implemented
 */
- (BOOL) unload;
#endif

/** Returns the path to the executable code in the bundle */
- (NSString *) executablePath;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6,GS_API_LATEST)
- (NSURL *) executableURL;
#endif

- (NSString *) pathForAuxiliaryExecutable: (NSString *) executableName;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6,GS_API_LATEST)
- (NSURL *)URLForAuxiliaryExecutable: (NSString *) executableName;
#endif


#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5,GS_API_LATEST) 
/** Not implemented */
- (NSArray *) executableArchitectures;
/** Not implemented */
- (BOOL) preflightAndReturnError: (NSError **)error;
/** Not implemented */
- (BOOL) loadAndReturnError: (NSError **)error;
#endif
@end

#if GS_API_VERSION(GS_API_NONE, 011700)
/**
 *  Augments [NSBundle], including methods for handling libraries in the GNUstep
 *  fashion, for rapid localization, and other purposes.
 */
@interface NSBundle (GNUstep)

/** This method is an experimental GNUstep extension, and might change.
 * <p>Return a bundle to access the resources for the (static or shared) library
 * libraryName, with interface version interfaceVersion.
 * </p>
 * <p>Resources for shared libraries are stored into
 * GNUSTEP_LIBRARY/Libraries/libraryName/Versions/interfaceVersion/Resources/;
 * this method will search for the first such existing directory and return it.
 *</p>
 * <p>libraryName should be the name of a library without the
 * <em>lib</em> prefix or any extensions; interfaceVersion is the
 * interface version of the library (eg, it's 1.13 in libgnustep-base.so.1.13; 
 * see library.make on how to control it).
 * </p>
 * <p>This method exists to provide resource bundles for libraries and has no
 * particular relationship to the library code itsself.  The named library
 * could be a dynamic library linked in to the running program, a static
 * library (whose code may not even exist on the host machine except where
 * it is linked in to the program), or even a library which is not linked
 * into the program at all (eg. where you want to share resources provided
 * for a library you do not actually use).
 * </p>
 * <p>The bundle for the library <em>gnustep-base</em> is a special case ...
 * for this bundle the -principalClass method returns [NSObject] and the
 * -executablePath method returns the path to the gnustep-base dynamic
 *  library (if it can be found).  As a general rule, library bundles are
 *  not guaranteed to return values for these methods as the library may
 *  not exist on disk.
 * </p>
 */
+ (NSBundle *) bundleForLibrary: (NSString *)libraryName
                        version: (NSString *)interfaceVersion;

/** This method is a equivalent to bundleForLibrary:version: with a nil
 * version.
 */
+ (NSBundle *) bundleForLibrary: (NSString *)libraryName;



/** Find a resource in the "Library" directory. */
+ (NSString*) pathForLibraryResource: (NSString*)name
			      ofType: (NSString*)extension
			 inDirectory: (NSString*)bundlePath;

/** Cleans up the path cache for the bundle. */
- (void) cleanPathCache;

#ifdef __ANDROID__

/**
 * Sets the Java Android asset manager.
 * The developer can call this method to enable asset loading via NSBundle.
 */
+ (void) setJavaAssetManager: (jobject)jassetManager withJNIEnv: (JNIEnv *)env;

/**
 * Returns the native Android asset manager.
 */
+ (AAssetManager *) assetManager;

/**
 * Returns the Android asset for the given path if path is in main bundle
 * resources and asset exists.
 * Uses `AASSET_MODE_UNKNOWN` to open the asset if it exists.
 * The returned object must be released using AAsset_close().
 */
+ (AAsset *) assetForPath: (NSString *)path;

/**
 * Returns the Android asset for the given path if path is in main bundle
 * resources and asset exists.
 * Uses the given mode to open the AAsset if it exists.
 * The returned object must be released using AAsset_close().
 */
+ (AAsset *) assetForPath: (NSString *)path withMode: (int)mode;

/**
 * Returns the Android asset dir for the given path if path is in main bundle
 * resources and the asset directory exists.
 * The returned object must be released using AAssetDir_close().
 */
+ (AAssetDir *) assetDirForPath: (NSString *)path;

#endif /* __ANDROID__ */

@end

#endif /* GNUSTEP */

/**
 * <p>
 *   This function (macro) is used to get the localized
 *   translation of the string <code>key</code>.
 *   <code>key</code> is looked up in the
 *   <code>Localizable.strings</code> file for the current
 *   language.  The current language is determined by the
 *   available languages in which the application is
 *   translated, and by using the <code>NSLanguages</code> user
 *   defaults (which should contain an array of the languages
 *   preferred by the user, in order of preference).
 * </p>
 * <p>
 *   Technically, the function works by calling
 *   <code>localizedStringForKey:value:table:</code> on the
 *   main bundle, using <code>@""</code> as value, and
 *   <code>nil</code> as the table.  The <code>comment</code>
 *   is ignored when the macro is expanded; but when we have
 *   tools which can generate the
 *   <code>Localizable.strings</code> files automatically from
 *   source code, the <code>comment</code> will be used by the
 *   tools and added as a comment before the string to
 *   translate.  Upon finding something like
 * </p>
 * <p>
 *   <code>
 *      NSLocalizedString (@"My useful string",
 *        @"My useful comment about the string");
 *   </code>
 * </p>
 * <p>
 *   in the source code, the tools will generate a comment and the line
 * </p>
 * <p>
 *   <code>
 *      " My useful string" = "My useful string";
 *   </code>
 * </p>
 * <p>
 *   in the <code>Localizable.strings</code> file (the
 *   translator then can use this as a skeleton for the
 *   <code>Localizable.strings</code> for his/her own language,
 *   where she/he can replace the right hand side with the
 *   translation in her/his own language).  The comment can
 *   help the translator to decide how to translate when it is
 *   not clear how to translate (because the original string is
 *   now out of context, and out of context might not be so
 *   clear what the string means).  The comment is totally
 *   ignored by the library code.
 * </p>
 * <p>
 *   If you don't have a comment (because the string is so
 *   self-explanatory that it doesn't need it), you can leave
 *   it blank, by using <code>@""</code> as a comment.  If the
 *   string might be unclear out of context, it is recommended
 *   that you add a comment (even if it is unused for now).
 * </p>
 */
#define NSLocalizedString(key, comment) \
  [[NSBundle mainBundle] localizedStringForKey: (key) value: @"" table: nil]

/**
 * This function (macro) does the same as
 * <code>NSLocalizedString</code>, but uses the table
 * <code>table</code> rather than the default table.  This
 * means that the string to translate will be looked up in a
 * different file than <code>Localizable.strings</code>.  For
 * example, if you pass <code>DatabaseErrors</code> as the
 * <code>table</code>, the string will be looked up for
 * translation in the file
 * <code>DatabaseErrors.strings</code>.  This allows you to
 * have the same string translated in different ways, by
 * having a different translation in different tables, and
 * choosing between the different translation by choosing a
 * different table.
 */
#define NSLocalizedStringFromTable(key, tbl, comment) \
  [[NSBundle mainBundle] localizedStringForKey: (key) value: @"" table: (tbl)]

/**
 * This function is the full-blown localization function (it
 * is actually a macro).  It looks up the string
 * <code>key</code> for translation in the table
 * <code>table</code> of the bundle <code>bundle</code>
 * (please refer to the NSBundle documentation for more
 * information on how this lookup is done).
 * <code>comment</code> is a comment, which is ignored by the
 * library (it is discarded when the macro is expanded) but which
 * can be used by tools which parse the source code and generate
 * strings table to provide a comment which the translator can
 * use when translating the string.
 */
#define NSLocalizedStringFromTableInBundle(key, tbl, bundle, comment) \
  [bundle localizedStringForKey: (key) value: @"" table: (tbl)]


#if	defined(__cplusplus)
}
#endif

#if     !NO_GNUSTEP && !defined(GNUSTEP_BASE_INTERNAL)
#import <GNUstepBase/NSBundle+GNUstepBase.h>
#endif

#endif	/* __NSBundle_h_GNUSTEP_BASE_INCLUDE */
