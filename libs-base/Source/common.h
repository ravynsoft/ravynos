/* Header file for all objective-c code in the base library.
 * This imports all the common headers in a consistent order such that
 * we can be sure only local headers are used rather than any which
 * might be from an earlier build.
 */

#ifndef COMMON_H
#define COMMON_H

#include "config.h"

#if	defined(HAVE_STRING_H)
/* For POSIX strerror_r() and others
 */
#include <string.h>
#endif

#if	defined(HAVE_STRINGS_H)
/* For strcasecmp() and others
 */
#include <strings.h>
#endif

#include <errno.h>

/* If this is included in a file in the Additions subdirectory, and we are
 * building for use with the NeXT/Apple Foundation, then we need to import
 * the native headers in preference to any of our own.
 */
#if	defined(NeXT_Foundation_LIBRARY)
#import	<Foundation/Foundation.h>
#endif

#import	"GNUstepBase/GSConfig.h"
#import	"GNUstepBase/GSVersionMacros.h"

/* Set localisation macro for use within the base library itsself.
 */
#define GS_LOCALISATION_BUNDLE \
  [NSBundle bundleForLibrary: @"gnustep-base" version: \
  OBJC_STRINGIFY(GNUSTEP_BASE_MAJOR_VERSION.GNUSTEP_BASE_MINOR_VERSION)]

#import	"GNUstepBase/GNUstep.h"

/* Foundation/NSObject.h imports <Foundation/NSZone.h> and
 * <Foundation/NSObjCRuntime.h> so we import local versions first.
 */
#import	"Foundation/NSZone.h"
#import	"Foundation/NSObjCRuntime.h"

/* Almost all headers import <Foundation/NSObject.h> so we import
 * "Foundation/NSObject.h" first, to ensure we have a local copy.
 */
#import	"Foundation/NSObject.h"
#import	"GNUstepBase/NSObject+GNUstepBase.h"

/* These headers are used in almost every file.
 */
#import	"Foundation/NSString.h"
#import	"Foundation/NSDebug.h"

/* These headers needed for string localisation ... hopefully we will
 * localise all the exceptions and debug/error messages in all the source
 * some day, so localisation needs ot be in the common header for all code.
 */
#import	"Foundation/NSBundle.h"
#import	"GNUstepBase/NSBundle+GNUstepBase.h"

/* We need to wrap unistd.h because it is used throughout the code and some
 * versions include __block as a variable name, and clang also defines that
 * and depends on the definition later ... so we resort to the fragile hack
 * of redefining according to the observed definition.
 */
#ifdef HAVE_UNISTD_H
#  ifdef __block
#    undef __block
#    define __block __gs_unistd_block
#    include <unistd.h>
#    undef __block
#    define __block __attribute__((__blocks__(byref)))
#  else
#    define __block __gs_unistd_block
#    include <unistd.h>
#    undef __block
#  endif
#endif

/* Redefine some function names when using the MSVC ABI on Windows.
 */
#ifdef _MSC_VER
#  define strncasecmp _strnicmp
#  define strcasecmp _stricmp
#  define write(fd, buffer, count) _write(fd, buffer, count)
#  define close(fd) _close(fd)
#endif

#endif /* COMMON_H */
