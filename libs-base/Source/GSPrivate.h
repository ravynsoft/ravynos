/* GSPrivate
   Copyright (C) 2001,2002 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>
   
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

#ifndef _GSPrivate_h_
#define _GSPrivate_h_

#include <errno.h>

#import "Foundation/NSBundle.h"
#import "Foundation/NSError.h"

@class	_GSInsensitiveDictionary;
@class	_GSMutableInsensitiveDictionary;

@class	NSNotification;

#if ( (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 3) ) && HAVE_VISIBILITY_ATTRIBUTE )
#define GS_ATTRIB_PRIVATE __attribute__ ((visibility("internal")))
#else
#define GS_ATTRIB_PRIVATE
#endif

/* Absolute Gregorian date for NSDate reference date Jan 01 2001
 *
 *  N = 1;                 // day of month
 *  N = N + 0;             // days in prior months for year
 *  N = N +                // days this year
 *    + 365 * (year - 1)   // days in previous years ignoring leap days
 *    + (year - 1)/4       // Julian leap days before this year...
 *    - (year - 1)/100     // ...minus prior century years...
 *    + (year - 1)/400     // ...plus prior years divisible by 400
 */
#define GREGORIAN_REFERENCE 730486

NSTimeInterval   GSPrivateTimeNow() GS_ATTRIB_PRIVATE;

#include "GNUstepBase/GSObjCRuntime.h"

#include "Foundation/NSArray.h"

#ifdef __GNUSTEP_RUNTIME__
struct objc_category;
typedef struct objc_category* Category;
#endif

@interface GSArray : NSArray
{
@public
  id		*_contents_array;
  unsigned	_count;
}
@end

@interface GSMutableArray : NSMutableArray
{
@public
  id		*_contents_array;
  unsigned	_count;
  unsigned	_capacity;
  int		_grow_factor;
  unsigned long		_version;
}
@end

@interface GSPlaceholderArray : NSArray
{
}
@end

#include "Foundation/NSString.h"

/**
 * Macro to manage memory for chunks of code that need to work with
 * arrays of items.  Use this to start the block of code using
 * the array and GS_ENDITEMBUF() to end it.  The idea is to ensure that small
 * arrays are allocated on the stack (for speed), but large arrays are
 * allocated from the heap (to avoid stack overflow).
 */
#if __GNUC__ > 3 && !defined(__clang__)
__attribute__((unused)) static void GSFreeTempBuffer(void **b)
{
  if (NULL != *b) free(*b);
}
#  define	GS_BEGINITEMBUF(P, S, T) { \
  T _ibuf[GS_MAX_OBJECTS_FROM_STACK];\
  T *P = _ibuf;\
  __attribute__((cleanup(GSFreeTempBuffer))) void *_base = 0;\
  if (S > GS_MAX_OBJECTS_FROM_STACK)\
    {\
      _base = malloc((S) * sizeof(T));\
      P = _base;\
    }
#  define	GS_BEGINITEMBUF2(P, S, T) { \
  T _ibuf2[GS_MAX_OBJECTS_FROM_STACK];\
  T *P = _ibuf2;\
  __attribute__((cleanup(GSFreeTempBuffer))) void *_base2 = 0;\
  if (S > GS_MAX_OBJECTS_FROM_STACK)\
    {\
      _base2 = malloc((S) * sizeof(T));\
      P = _base2;\
    }
#else
/* Make minimum size of _ibuf 1 to avoid compiler warnings.
 */
#  define	GS_BEGINITEMBUF(P, S, T) { \
  T _ibuf[(S) > 0 && (S) <= GS_MAX_OBJECTS_FROM_STACK ? (S) : 1]; \
  T *_base = ((S) <= GS_MAX_OBJECTS_FROM_STACK) ? _ibuf \
    : (T*)malloc((S) * sizeof(T)); \
  T *(P) = _base;
#  define	GS_BEGINITEMBUF2(P, S, T) { \
  T _ibuf2[(S) > 0 && (S) <= GS_MAX_OBJECTS_FROM_STACK ? (S) : 1]; \
  T *_base2 = ((S) <= GS_MAX_OBJECTS_FROM_STACK) ? _ibuf2 \
    : (T*)malloc((S) * sizeof(T)); \
  T *(P) = _base2;
#endif

/**
 * Macro to manage memory for chunks of code that need to work with
 * arrays of items.  Use GS_BEGINITEMBUF() to start the block of code using
 * the array and this macro to end it.
 */
#if __GNUC__ > 3 && !defined(__clang__)
# define	GS_ENDITEMBUF() }
# define	GS_ENDITEMBUF2() }
#else
#  define	GS_ENDITEMBUF() \
  if (_base != _ibuf) \
    free(_base); \
  }
#  define	GS_ENDITEMBUF2() \
  if (_base2 != _ibuf2) \
    free(_base2); \
  }
#endif

/**
 * Macro to manage memory for chunks of code that need to work with
 * arrays of objects.  Use this to start the block of code using
 * the array and GS_ENDIDBUF() to end it.  The idea is to ensure that small
 * arrays are allocated on the stack (for speed), but large arrays are
 * allocated from the heap (to avoid stack overflow).
 */
#define	GS_BEGINIDBUF(P, S) GS_BEGINITEMBUF(P, S, id)

/**
 * Macro to manage memory for chunks of code that need to work with
 * arrays of objects.  Use GS_BEGINIDBUF() to start the block of code using
 * the array and this macro to end it.
 */
#define	GS_ENDIDBUF() GS_ENDITEMBUF()

/**
 * Macro to consistently replace public accessable
 * constant strings with dynamically allocated versions.
 * This method assumes an initialized NSStringClass symbol
 * which contains the Class object of NSString.  <br>
 * Most public accessible strings are used in collection classes
 * like NSDictionary, and therefore tend to receive -isEqual:
 * messages (and therefore -hash) rather often.  Statically
 * allocated strings must calculate their hash values while
 * dynamically allocated strings can store them.  This optimization
 * is by far more effective than using NSString * const.
 * The drawback is that the memory management cannot enforce these values
 * to remain unaltered as it would for variables declared NSString * const.
 * Yet the optimization of the stored hash value is currently deemed
 * more important.
 */
#ifndef GNUSTEP_NEW_STRING_ABI
#define GS_REPLACE_CONSTANT_STRING(ID) [(ID = [NSObject \
  leak: [[NSString alloc] initWithUTF8String: [ID UTF8String]]]) release]
#else
/**
 * In the new constant string ABI, the hash can be stored in the constant
 * string object, so this is not a problem.
 */
#define GS_REPLACE_CONSTANT_STRING(ID)
#endif
/* Using cString here is OK here
   because NXConstantString returns a pointer
   to it's internal pointer.  */

/*
 * Type to hold either UTF-16 (unichar) or 8-bit encodings,
 * while satisfying alignment constraints.
 */
typedef union {
  unichar *u;       // 16-bit unicode characters.
  unsigned char *c; // 8-bit characters.
} GSCharPtr;

/*
 * Private concrete string classes.
 * NB. All these concrete string classes MUST have the same initial ivar
 * layout so that we can swap between them as necessary.
 * The initial layout must also match that of NXConstantString (which is
 * determined by the compiler) - an initial pointer to the string data
 * followed by the string length (number of characters).
 */
@interface GSString : NSString
{
@public
  GSCharPtr _contents;
  unsigned int	_count;
  struct {
    unsigned int	wide: 1;	// 16-bit characters in string?
    unsigned int	owned: 1;	// Set if the instance owns the
					// _contents buffer
    unsigned int	unused: 2;
    unsigned int	hash: 28;
  } _flags;
}
@end

/*
 * GSMutableString - concrete mutable string, capable of changing its storage
 * from holding 8-bit to 16-bit character set.
 */
@interface GSMutableString : NSMutableString
{
@public
  GSCharPtr _contents;
  unsigned int	_count;
  struct {
    unsigned int	wide: 1;
    unsigned int	owned: 1;
    unsigned int	unused: 2;
    unsigned int	hash: 28;
  } _flags;
  unsigned int	_capacity;
  NSZone	*_zone;
}
@end

typedef	GSMutableString *GSStr;

/*
 * Enumeration for MacOS-X compatibility user defaults settings.
 * For efficiency, we save defaults information which is used by the
 * base library.
 */
typedef enum {
  GSMacOSXCompatible,			// General behavior flag.
  GSOldStyleGeometry,			// Control geometry string output.
  GSLogSyslog,				// Force logging to go to syslog.
  GSLogThread,				// Include thread name in log message.
  GSLogOffset,			        // Include time zone offset in message.
  NSWriteOldStylePropertyLists,		// Control PList output.
  GSExceptionStackTrace,                // Add trace to exception description.
  GSUserDefaultMaxFlag			// End marker.
} GSUserDefaultFlagType;



@interface NSBundle (Private)
+ (NSString *) _absolutePathOfExecutable: (NSString *)path;
+ (NSBundle*) _addFrameworkFromClass: (Class)frameworkClass;
+ (NSMutableArray*) _addFrameworks;
+ (NSString*) _gnustep_target_cpu;
+ (NSString*) _gnustep_target_dir;
+ (NSString*) _gnustep_target_os;
+ (NSString*) _library_combo;
@end

/**
 * This class exists simply as a mechanism for encapsulating arrays
 * encoded using [NSKeyedArchiver-encodeArrayOfObjCType:count:at:]
 */
@interface	_NSKeyedCoderOldStyleArray : NSObject <NSCoding>
{
  char		_t[2];
  unsigned	_c;
  unsigned	_s;
  const void	*_a;
  NSData	*_d;	// Only valid after initWithCoder:
}
- (const void*) bytes;
- (NSUInteger) count;
- (void) encodeWithCoder: (NSCoder*)aCoder;
- (id) initWithCoder: (NSCoder*)aCoder;
- (id) initWithObjCType: (const char*)t count: (NSInteger)c at: (const void*)a;
- (NSUInteger) size;
- (const char*) type;
@end

/* Get error information.
 */
@interface	NSError (GNUstepBase)
+ (NSError*) _last;
+ (NSError*) _systemError: (long)number;
@end

@class  NSRunLoop;
@class  NSLock;
@class  NSThread;

/* Used to handle events performed in one thread from another.
 */
@interface      GSRunLoopThreadInfo : NSObject
{
  @public
  NSRunLoop             *loop;
  NSLock                *lock;
  NSMutableArray        *performers;
#ifdef _WIN32
  HANDLE	        event;
#else
  int                   inputFd;
  int                   outputFd;
#endif	
}
/* Add a performer to be run in the loop's thread.  May be called from
 * any thread.
 */
- (void) addPerformer: (id)performer;
/* Fire all pending performers in the current thread.  May only be called
 * from the runloop when the event/descriptor is triggered.
 */
- (void) fire;
/* Cancel all pending performers.
 */
- (void) invalidate;
@end

/* Return (and optionally create) GSRunLoopThreadInfo for the specified
 * thread (or the current thread if aThread is nil).<br />
 * If aThread is nil and no value is set for the current thread, create
 * a GSRunLoopThreadInfo and set it for the current thread.
 */
GSRunLoopThreadInfo *
GSRunLoopInfoForThread(NSThread *aThread) GS_ATTRIB_PRIVATE;

/* Used by NSException uncaught exception handler - must not call any
 * methods/functions which might cause a recursive exception.
 */
const char*
GSPrivateArgZero() GS_ATTRIB_PRIVATE;

/* get the available string encodings (nul terminated array)
 */
NSStringEncoding *
GSPrivateAvailableEncodings() GS_ATTRIB_PRIVATE;

/* Initialise constant strings
 */
void
GSPrivateBuildStrings(void) GS_ATTRIB_PRIVATE;

/* Used to check for termination of background tasks.
 */
BOOL
GSPrivateCheckTasks(void) GS_ATTRIB_PRIVATE;

/* get the default C-string encoding.
 */
NSStringEncoding
GSPrivateDefaultCStringEncoding() GS_ATTRIB_PRIVATE;

/* Get default locale quickly (usually from cache).
 * External apps would cache the locale themselves.
 */
NSDictionary *
GSPrivateDefaultLocale() GS_ATTRIB_PRIVATE;

/* Get one of several standard values.
 */
BOOL
GSPrivateDefaultsFlag(GSUserDefaultFlagType type) GS_ATTRIB_PRIVATE;

/* get the name of a string encoding as an NSString.
 */
NSString *
GSPrivateEncodingName(NSStringEncoding encoding) GS_ATTRIB_PRIVATE;

/* get a flag from an environment variable - return def if not defined.
 */
BOOL
GSPrivateEnvironmentFlag(const char *name, BOOL def) GS_ATTRIB_PRIVATE;

/* Get the path to the xcurrent executable.
 */
NSString *
GSPrivateExecutablePath(void) GS_ATTRIB_PRIVATE;

/* Format arguments into an internal string.
 */
void
GSPrivateFormat(GSStr fb, const unichar *fmt, va_list ap, NSDictionary *loc)
  GS_ATTRIB_PRIVATE;

/* determine whether data in a particular encoding can
 * generally be represented as 8-bit characters including ascii.
 */
BOOL
GSPrivateIsByteEncoding(NSStringEncoding encoding) GS_ATTRIB_PRIVATE;

/* determine whether encoding is currently supported.
 */
BOOL
GSPrivateIsEncodingSupported(NSStringEncoding encoding) GS_ATTRIB_PRIVATE;

/* load a module into the runtime
 */
long
GSPrivateLoadModule(NSString *filename, FILE *errorStream,
  void (*loadCallback)(Class, struct objc_category *),
  void **header, NSString *debugFilename) GS_ATTRIB_PRIVATE;

/* Get the native C-string encoding as used by locale specific code in the
 * operating system.  This may differ from the default C-string encoding
 * if the latter has been set via an environment variable.
 */
NSStringEncoding
GSPrivateNativeCStringEncoding() GS_ATTRIB_PRIVATE;

/* Get the native C-string encoding as used by the ICU library, which may
 * differ from the native locale encoding or the default C-string encoding
 */
NSStringEncoding
GSPrivateICUCStringEncoding() GS_ATTRIB_PRIVATE;

/* Function used by the NSRunLoop and friends for processing
 * queued notifications which should be processed at the first safe moment.
 */
void GSPrivateNotifyASAP(NSString *mode) GS_ATTRIB_PRIVATE;

/* Function used by the NSRunLoop and friends for processing
 * queued notifications which should be processed when the loop is idle.
 */
void GSPrivateNotifyIdle(NSString *mode) GS_ATTRIB_PRIVATE;

/* Function used by the NSRunLoop and friends for determining whether
 * there are more queued notifications to be processed.
 */
BOOL GSPrivateNotifyMore(NSString *mode) GS_ATTRIB_PRIVATE;

/* Function to return the function for searching in a string for a range.
 */
typedef NSRange (*GSRSFunc)(id, id, unsigned, NSRange);
GSRSFunc
GSPrivateRangeOfString(NSString *receiver, NSString *target) GS_ATTRIB_PRIVATE;

/* Function to return the hash value for a small integer (used by NSNumber).
 */
unsigned
GSPrivateSmallHash(int n) GS_ATTRIB_PRIVATE;

/* Function to append data to an GSStr
 */
void
GSPrivateStrAppendUnichars(GSStr s, const unichar *u, unsigned l)
  GS_ATTRIB_PRIVATE;

/* Make the content of this string into unicode if it is not in
 * the external defaults C string encoding.
 */
void
GSPrivateStrExternalize(GSStr s) GS_ATTRIB_PRIVATE;

/*
 * GSPrivateSymbolPath() returns the path to the object file from
 * which a certain class was loaded.
 *
 * If the class was loaded from a shared library, this returns the
 * filesystem path to the shared library; if it was loaded from a
 * dynamical object (such as a bundle or framework dynamically
 * loaded), it returns the filesystem path to the object file; if the
 * class was loaded from the main executable, it returns the
 * filesystem path to the main executable path.
 *
 * This function is implemented by using the available features of
 * the dynamic linker on the specific platform we are running on.
 *
 * On some platforms, the dynamic linker does not provide enough
 * facilities to support the GSPrivateSymbolPath() function at all;
 * in this case, GSPrivateSymbolPath() always returns nil.
 *
 * On my platform (a Debian GNU Linux), it seems the dynamic linker
 * always returns the filesystem path that was used to load the
 * module.  So it returns the full filesystem path for shared libraries
 * and bundles (which is very nice), but unfortunately it returns 
 * argv[0] (which might be something as horrible as './obj/test')
 * for classes in the main executable.
 *
 * Currently, the function will return nil if any of the following
 * conditions is satisfied:
 *  - the required functionality is not available on the platform we are
 *    running on;
 *  - memory allocation fails;
 *  - the symbol for that class could not be found.
 *
 * In general, if the function returns nil, it means something serious
 * went wrong in the system preventing it from getting the symbol path.
 * If your code is to be portable, you (unfortunately) have to be prepared
 * to work around it in some way when this happens.
 *
 * It seems that this function has no corresponding function in the NeXT
 * runtime ... as far as I know.
 */
NSString *
GSPrivateSymbolPath(Class theClass) GS_ATTRIB_PRIVATE;

/* Combining class for composite unichars
 */
unsigned char
GSPrivateUniCop(unichar u) GS_ATTRIB_PRIVATE;

/* unload a module from the runtime (not implemented)
 */
long
GSPrivateUnloadModule(FILE *errorStream,
  void (*unloadCallback)(Class, struct objc_category *)) GS_ATTRIB_PRIVATE;


/* Memory to use to put executable code in.
 */
@interface      GSCodeBuffer : NSObject
{
  unsigned      size;
  void          *buffer;
  void		*executable;
  id            frame;
}
+ (GSCodeBuffer*) memoryWithSize: (NSUInteger)_size;
- (void*) buffer;
- (void*) executable;
- (id) initWithSize: (NSUInteger)_size;
- (void) protect;
- (void) setFrame: (id)aFrame;
@end

BOOL
GSPrivateIsCollectable(const void *ptr) GS_ATTRIB_PRIVATE;

NSZone*
GSAtomicMallocZone (void);

/* Generate a 32bit hash from supplied byte data.
 */
uint32_t
GSPrivateHash(uint32_t seed, const void *bytes, int length)
  GS_ATTRIB_PRIVATE;

/* Incorporate 'l' bytes of data from the buffer pointed to by 'b' into
 * the hash state information pointed to by p0 and p1.
 * The hash state variables should have been initialised to zero before
 * the first call to this function, and the result should be produced
 * by calling the GSPrivateFinishHash() function.
 */
void
GSPrivateIncrementalHash(uint32_t *p0, uint32_t *p1, const void *b, int l)
  GS_ATTRIB_PRIVATE;

/* Generate a 32bit hash from supplied state variables resulting from
 * calls to the GSPrivateIncrementalHash() function.
 */
uint32_t
GSPrivateFinishHash(uint32_t s0, uint32_t s1, uint32_t totalLength)
  GS_ATTRIB_PRIVATE;

@class  NSHashTable;
/* If 'self' is not a member of 'exclude', adds to the hash
 * table and returns the memory footprint of 'self' assuming
 * it contains no pointers and has no extra memory allocated.
 * Otherwise returns 0.
 */
NSUInteger
GSPrivateMemorySize(NSObject *self, NSHashTable *exclude)
  GS_ATTRIB_PRIVATE;

/* Return the current thread ID as an NSUInteger.
 * Ideally, we use the operating-system's notion of a thread ID so
 * that external process monitoring software will be using the same
 * value that we log.  If we don't know the system's mechanism, we
 * use the address of the current NSThread object so that, even if
 * it makes no sense externally, it can still be used to show that
 * different threads generated different logs.
 */
NSUInteger
GSPrivateThreadID()
  GS_ATTRIB_PRIVATE;

/** Function to base64 encode data.  The destination buffer must be of
 * size (((length + 2) / 3) * 4) or more.
 */
void
GSPrivateEncodeBase64(const uint8_t *src, NSUInteger length, uint8_t *dst)
  GS_ATTRIB_PRIVATE;

#endif /* _GSPrivate_h_ */

