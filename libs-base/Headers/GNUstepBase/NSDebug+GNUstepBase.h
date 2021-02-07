/** Declaration of extension methods for base additions

   Copyright (C) 2003-2010 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>
   and:         Adam Fedor <fedor@gnu.org>

   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02111 USA.

*/

#ifndef	INCLUDED_NSDebug_GNUstepBase_h
#define	INCLUDED_NSDebug_GNUstepBase_h

#import <GNUstepBase/GSVersionMacros.h>
#if   defined(GNUSTEP_BASE_INTERNAL)
#  import "Foundation/NSDebug.h"
#  import "Foundation/NSProcessInfo.h"
#else
#  import <Foundation/NSDebug.h>
#  import <Foundation/NSProcessInfo.h>
#endif

#if	defined(__cplusplus)
extern "C" {
#endif

#if	OS_API_VERSION(GS_API_NONE,GS_API_LATEST)

GS_EXPORT BOOL GSDebugSet(NSString *level);

/**
 * Used to produce a format string for logging a message with function
 * location details.
 */
GS_EXPORT NSString*	GSDebugFunctionMsg(const char *func, const char *file,
				int line, NSString *fmt);
/**
 * Used to produce a format string for logging a message with method
 * location details.
 */
GS_EXPORT NSString*	GSDebugMethodMsg(id obj, SEL sel, const char *file,
				int line, NSString *fmt);


#ifdef GSDIAGNOSE

/**
   <p>NSDebugLLog() is the basic debug logging macro used to display
   log messages using NSLog(), if debug logging was enabled at compile
   time and the appropriate logging level was set at runtime.
   </p>
   <p>Debug logging which can be enabled/disabled by defining GSDIAGNOSE
   when compiling and also setting values in the mutable set which
   is set up by NSProcessInfo. GSDIAGNOSE is defined automatically
   unless diagnose=no is specified in the make arguments.
   </p>
   <p>NSProcess initialises a set of strings that are the names of active
   debug levels using the '--GNU-Debug=...' command line argument.
   Each command-line argument of that form is removed from
   <code>NSProcessInfo</code>'s list of arguments and the variable part
   (...) is added to the set.
   This means that as far as the program proper is concerned, it is
   running with the same arguments as if debugging had not been enabled.
   </p>
   <p>For instance, to debug the NSBundle class, run your program with 
    '--GNU-Debug=NSBundle'
   You can of course supply multiple '--GNU-Debug=...' arguments to
   output debug information on more than one thing.
   </p>
   <p>NSUserDefaults also adds debug levels from the array given by the
   GNU-Debug key ... but these values will not take effect until the
   +standardUserDefaults method is called ... so they are useless for
   debugging NSUserDefaults itself or for debugging any code executed
   before the defaults system is used.
   </p>
   <p>To embed debug logging in your code you use the NSDebugLLog() or
   NSDebugLog() macro.  NSDebugLog() is just NSDebugLLog() with the debug
   level set to 'dflt'.  So, to activate debug statements that use
   NSDebugLog(), you supply the '--GNU-Debug=dflt' argument to your program.
   </p>
   <p>You can also change the active debug levels under your programs control -
   NSProcessInfo has a [-debugSet] method that returns the mutable set that
   contains the active debug levels - your program can modify this set.
   </p>
   <p>Two debug levels have a special effect - 'dflt' is the level used for
   debug logs statements where no debug level is specified, and 'NoWarn'
   is used to *disable* warning messages.
   </p>
   <p>As a convenience, there are four more logging macros you can use -
   NSDebugFLog(), NSDebugFLLog(), NSDebugMLog() and NSDebugMLLog().
   These are the same as the other macros, but are specifically for use in
   either functions or methods and prepend information about the file, line
   and either function or class/method in which the message was generated.
   </p>
 */
#define NSDebugLLog(level, format, args...) \
  do { if (GSDebugSet(level) == YES) \
    NSLog(format , ## args); } while (0)

/**
 * This macro is a shorthand for NSDebugLLog() using then default debug
 * level ... 'dflt'
 */
#define NSDebugLog(format, args...) \
  do { if (GSDebugSet(@"dflt") == YES) \
    NSLog(format , ## args); } while (0)

/**
 * This macro is like NSDebugLLog() but includes the name and location
 * of the function in which the macro is used as part of the log output.
 */
#define NSDebugFLLog(level, format, args...) \
  do { if (GSDebugSet(level) == YES) { \
    NSString *s = GSDebugFunctionMsg( \
      __PRETTY_FUNCTION__, __FILE__, __LINE__, \
      [NSString stringWithFormat: format, ##args]); \
    NSLog(@"%@", s); }} while (0)

/**
 * This macro is a shorthand for NSDebugFLLog() using then default debug
 * level ... 'dflt'
 */
#define NSDebugFLog(format, args...) \
  do { if (GSDebugSet(@"dflt") == YES) { \
    NSString *s = GSDebugFunctionMsg( \
      __PRETTY_FUNCTION__, __FILE__, __LINE__, \
      [NSString stringWithFormat: format, ##args]); \
    NSLog(@"%@", s); }} while (0)

/**
 * This macro is like NSDebugLLog() but includes the name and location
 * of the <em>method</em> in which the macro is used as part of the log output.
 */
#define NSDebugMLLog(level, format, args...) \
  do { if (GSDebugSet(level) == YES) { \
    NSString *s = GSDebugMethodMsg( \
      self, _cmd, __FILE__, __LINE__, \
      [NSString stringWithFormat: format, ##args]); \
    NSLog(@"%@", s); }} while (0)

/**
 * This macro is a shorthand for NSDebugMLLog() using then default debug
 * level ... 'dflt'
 */
#define NSDebugMLog(format, args...) \
  do { if (GSDebugSet(@"dflt") == YES) { \
    NSString *s = GSDebugMethodMsg( \
      self, _cmd, __FILE__, __LINE__, \
      [NSString stringWithFormat: format, ##args]); \
    NSLog(@"%@", s); }} while (0)

/**
 * This macro saves the name and location of the function in
 * which the macro is used, along with a short string msg as
 * the tag associated with a recorded object.
 */
#define NSDebugFRLog(object, msg) \
  do { \
    NSString *tag = GSDebugFunctionMsg( \
	__PRETTY_FUNCTION__, __FILE__, __LINE__, msg); \
    GSDebugAllocationTagRecordedObject(object, tag); } while (0)

/**
 * This macro saves the name and location of the method in
 * which the macro is used, along with a short string msg as
 * the tag associated with a recorded object.
 */
#define NSDebugMRLog(object, msg) \
  do { \
    NSString *tag = GSDebugMethodMsg( \
	self, _cmd, __FILE__, __LINE__, msg); \
    GSDebugAllocationTagRecordedObject(object, tag); } while (0)

#else
#define NSDebugLLog(level, format, args...)
#define NSDebugLog(format, args...)
#define NSDebugFLLog(level, format, args...)
#define NSDebugFLog(format, args...)
#define NSDebugMLLog(level, format, args...)
#define NSDebugMLog(format, args...)
#define NSDebugFRLog(object, msg)
#define NSDebugMRLog(object, msg)
#endif

/**
 * Macro to log a message only the first time it is encountered.<br />
 * Not entirely thread safe ... but that's not really important,
 * it just means that it's possible for the message to be logged
 * more than once if two threads call it simultaneously when it
 * has not already been called.<br />
 * Use this from inside a function.  Pass an NSString as a format,
 * followed by zero or more arguments for the format string.
 * Example: GSOnceFLog(@"This function is deprecated, use another");
 */
#define GSOnceFLog(format, args...) \
  do { static BOOL beenHere = NO; if (beenHere == NO) {\
    NSString *s = GSDebugFunctionMsg( \
      __PRETTY_FUNCTION__, __FILE__, __LINE__, \
      [NSString stringWithFormat: format, ##args]); \
    beenHere = YES; \
    NSLog(@"%@", s); }} while (0)

/**
 * Macro to log a message only the first time it is encountered.<br />
 * Not entirely thread safe ... but that's not really important,
 * it just means that it's possible for the message to be logged
 * more than once if two threads call it simultaneously when it
 * has not already been called.<br />
 * Use this from inside a method. Pass an NSString as a format
 * followed by zero or more arguments for the format string.<br />
 * Example: GSOnceMLog(@"This method is deprecated, use another");
 */
#define GSOnceMLog(format, args...) \
  do { static BOOL beenHere = NO; if (beenHere == NO) {\
    NSString *s = GSDebugFunctionMsg( \
      __PRETTY_FUNCTION__, __FILE__, __LINE__, \
      [NSString stringWithFormat: format, ##args]); \
    beenHere = YES; \
    NSLog(@"%@", s); }} while (0)


#ifdef GSWARN

/**
   <p>NSWarnLog() is the basic debug logging macro used to display
   warning messages using NSLog(), if warn logging was not disabled at compile
   time and the disabling logging level was not set at runtime.
   </p>
   <p>Warning messages which can be enabled/disabled by defining GSWARN
   when compiling.
   </p>
   <p>You can also disable these messages at runtime by supplying a
   '--GNU-Debug=NoWarn' argument to the program, or by adding 'NoWarn'
   to the user default array named 'GNU-Debug'.
   </p>
   <p>These logging macros are intended to be used when the software detects
   something that it not necessarily fatal or illegal, but looks like it
   might be a programming error.  eg. attempting to remove 'nil' from an
   NSArray, which the Spec/documentation does not prohibit, but which a
   well written program should not be attempting (since an NSArray object
   cannot contain a 'nil').
   </p>
   <p>NB. The 'warn=yes' option is understood by the GNUstep make package
   to mean that GSWARN should be defined, and the 'warn=no' means that
   GSWARN should be undefined.  Default is to define it.
   </p>
   <p>To embed debug logging in your code you use the NSWarnLog() macro.
   </p>
   <p>As a convenience, there are two more logging macros you can use -
   NSWarnFLog(), and NSWarnMLog().
   These are specifically for use in either functions or methods and
   prepend information about the file, line and either function or
   class/method in which the message was generated.
   </p>
 */

#define NSWarnLog(format, args...) \
  do { if (GSDebugSet(@"NoWarn") == NO) { \
    NSLog(format , ## args); }} while (0)

/**
 * This macro is like NSWarnLog() but includes the name and location of the
 * <em>function</em> in which the macro is used as part of the log output.
 */
#define NSWarnFLog(format, args...) \
  do { if (GSDebugSet(@"NoWarn") == NO) { \
    NSString *s = GSDebugFunctionMsg( \
      __PRETTY_FUNCTION__, __FILE__, __LINE__, \
      [NSString stringWithFormat: format, ##args]); \
    NSLog(@"%@", s); }} while (0)

/**
 * This macro is like NSWarnLog() but includes the name and location of the
 * <em>method</em> in which the macro is used as part of the log output.
 */
#define NSWarnMLog(format, args...) \
  do { if (GSDebugSet(@"NoWarn") == NO) { \
    NSString *s = GSDebugFunctionMsg( \
      __PRETTY_FUNCTION__, __FILE__, __LINE__, \
      [NSString stringWithFormat: format, ##args]); \
    NSLog(@"%@", s); }} while (0)
#else
#define NSWarnLog(format, args...)
#define NSWarnFLog(format, args...)
#define NSWarnMLog(format, args...)
#endif

/** The DLog macro is a less powerful but commonly used logging macro,
 * defined here for convenience when porting code.  It will tell you
 * the function name and line number but not the file location.
 * It performs unconditional logging but is only compiled in when the
 * program is built with DEBUG defined.
 */
#if     !defined(DLog)
#ifdef DEBUG
#define DLog(fmt, ...) \
NSLog((@"%s [Line %d] " fmt), __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define DLog(...)
#endif
#endif

/** The Alog macro is the same as the DLog macro, but is always compiled
 * in to the code whether DEBUG is defined or not.
 */
#if     !defined(ALog)
#define ALog(fmt, ...) \
NSLog((@"%s [Line %d] " fmt), __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif


#endif	/* OS_API_VERSION */

#if	defined(__cplusplus)
}
#endif

#endif	/* INCLUDED_NSDebug_GNUstepBase_h */

