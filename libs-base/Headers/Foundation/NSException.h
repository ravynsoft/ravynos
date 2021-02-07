/** Interface for NSException for GNUStep
   Copyright (C) 1995, 1996 Free Software Foundation, Inc.

   Written by:  Adam Fedor <fedor@boulder.colorado.edu>
   Date: 1995
   
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

    <title>NSException and NSAssertionHandler class reference</title>

    AutogsdocSource: NSAssertionHandler.m
    AutogsdocSource: NSException.m

   */ 

#ifndef __NSException_h_GNUSTEP_BASE_INCLUDE
#define __NSException_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>
#import	<GNUstepBase/GSConfig.h>

#if     defined(_NATIVE_OBJC_EXCEPTIONS)
#  define USER_NATIVE_OBJC_EXCEPTIONS       1
#elif   BASE_NATIVE_OBJC_EXCEPTIONS && defined(OBJC_ZEROCOST_EXCEPTIONS)
#  define USER_NATIVE_OBJC_EXCEPTIONS       1
#  define _NATIVE_OBJC_EXCEPTIONS           1
#endif

#if     !BASE_NATIVE_OBJC_EXCEPTIONS && USER_NATIVE_OBJC_EXCEPTIONS
#error "There are two separate exception handling mechanisms available ... one based on the standard setjmp() function (which does not require special compiler support), and one 'native' version where the compiler manages the exception handling.  If you try to use both in the same executable, exception handlers will not work... which can be pretty disastrous.  This error is telling you that the gnustep-base library was built using one form of exception handling, but that the gnustep-make package you are using is building code to use the other form of exception handling ... with the consequence that exception handling would be broken in the program you are building.  So, somehow your gnustep-base and gnustep-make package are incompatible, and you need to replace one of them with a version configured to match the other."
#if     BASE_NATIVE_OBJC_EXCEPTIONS
#error  "gnustep-base is configured to use 'native' exceptions, but you are building for 'traditional' exceptions."
#else
#error  "gnustep-base is configured to use 'traditional' exceptions, but you are building for 'native' exceptions."
#endif
#endif

#import	<Foundation/NSString.h>

#include <setjmp.h>
#include <stdarg.h>

#if	defined(__WIN64__)
/* This hack is to deal with the fact that currently (June 2016) the
 * implementation of longjmp in mingw-w64  sometimes crashes in msvcrt.dll
 * but the builtin version provided by gcc seems to work.
 */
#undef	setjmp
#define	setjmp(X)	__builtin_setjmp(X)
#undef	longjmp
#define	longjmp(X,Y)	__builtin_longjmp(X,Y)
#endif

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSDictionary;

/**
   <p>
   The <code>NSException</code> class helps manage errors in a program. It
   provides a mechanism for lower-level methods to provide information about
   problems to higher-level methods, which more often than not, have a
   better ability to decide what to do about the problems.
   </p>
   <p>
   Exceptions are typically handled by enclosing a sensitive section
   of code inside the macros <code>NS_DURING</code> and <code>NS_HANDLER</code>,
   and then handling any problems after this, up to the
   <code>NS_ENDHANDLER</code> macro:
   </p>
   <example>
   NS_DURING
    code that might cause an exception
   NS_HANDLER
    code that deals with the exception. If this code cannot deal with
    it, you can re-raise the exception like this
    [localException raise]
    so the next higher level of code can handle it
   NS_ENDHANDLER
   </example>
   <p>
   The local variable <code>localException</code> is the name of the exception
   object you can use in the <code>NS_HANDLER</code> section.
   The easiest way to cause an exception is using the +raise:format:,...
   method.
   </p>
   <p>
   If there is no NS_HANDLER ... NS_ENDHANDLER block enclosing (directly or
   indirectly) code where an exception is raised, then control passes to
   the <em>uncaught exception handler</em> function and the program is
   then terminated.<br />
   The uncaught exception handler is set using NSSetUncaughtExceptionHandler()
   and if not set, defaults to a function which will simply print an error
   message before the program terminates.
   </p>
*/
GS_EXPORT_CLASS
@interface NSException : NSObject <NSCoding, NSCopying>
{    
#if	GS_EXPOSE(NSException)
@private
  NSString *_e_name;
  NSString *_e_reason;
  void *_reserved;
#endif
}

/**
 * Create an an exception object with a name, reason and a dictionary
 * userInfo which can be used to provide additional information or
 * access to objects needed to handle the exception. After the
 * exception is created you must -raise it.
 */
+ (NSException*) exceptionWithName: (NSString*)name
			    reason: (NSString*)reason
			  userInfo: (NSDictionary*)userInfo;

/**
 * Creates an exception with a name and a reason using the
 * format string and any additional arguments. The exception is then
 * <em>raised</em> using the -raise method.
 */
+ (void) raise: (NSString*)name
	format: (NSString*)format,...
  NS_FORMAT_FUNCTION(2,3) GS_NORETURN_METHOD;

/**
 * Creates an exception with a name and a reason string using the
 * format string and additional arguments specified as a variable
 * argument list argList. The exception is then <em>raised</em>
 * using the -raise method.
 */
+ (void) raise: (NSString*)name
	format: (NSString*)format
     arguments: (va_list)argList
  NS_FORMAT_FUNCTION(2,0) GS_NORETURN_METHOD;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5,GS_API_LATEST) && GS_API_VERSION( 11501,GS_API_LATEST)
/** Returns an array of the call stack return addresses at the point when
 * the exception was raised.  Re-raising the exception does not change
 * this value.
 */
- (NSArray*) callStackReturnAddresses;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6,GS_API_LATEST) && GS_API_VERSION( 11903,GS_API_LATEST)
/**
 * Returns an array of the symbolic names of the call stack return addresses.  
 * Note that, on some platforms, symbols are only exported in
 * position-independent code and so these may only return numeric addresses for
 * code in static libraries or the main application.  
 */
- (NSArray*) callStackSymbols;
#endif

/**
 * <init/>Initializes a newly allocated NSException object with a
 * name, reason and a dictionary userInfo.
 */
- (id) initWithName: (NSString*)name 
	     reason: (NSString*)reason 
	   userInfo: (NSDictionary*)userInfo;

/** Returns the name of the exception. */
- (NSString*) name;

/**
 * Raises the exception. All code following the raise will not be
 * executed and program control will be transfered to the closest
 * calling method which encapsulates the exception code in an
 * NS_DURING macro.<br />
 * If the exception was not caught in a macro, the currently set
 * uncaught exception handler is called to perform final logging
 * and the program is then terminated.<br />
 * If the uncaught exception handler fails to terminate the program,
 * then the default behavior is to terminate the program as soon as
 * the uncaught exception handler function returns.<br />
 * NB. all other exception raising methods call this one, so if you
 * want to set a breakpoint when debugging, set it in this method.
 */
- (void) raise GS_NORETURN_METHOD;

/** Returns the exception reason. */
- (NSString*) reason;

/** Returns the exception userInfo dictionary.<br />
 */
- (NSDictionary*) userInfo;

@end

/** An exception when character set conversion fails.
 */
GS_EXPORT NSString* const NSCharacterConversionException;

/** Attempt to use an invalidated destination.
 */
GS_EXPORT NSString* const NSDestinationInvalidException;

/** A generic exception for general purpose usage.
 */
GS_EXPORT NSString* const NSGenericException;

/** An exception for cases where unexpected state is detected within an object.
 */
GS_EXPORT NSString* const NSInternalInconsistencyException;

/** An exception used when an invalid argument is passed to a method
 * or function.
 */
GS_EXPORT NSString* const NSInvalidArgumentException;

/** Attempt to use a receive port which has been invalidated.
 */
GS_EXPORT NSString * const NSInvalidReceivePortException;

/** Attempt to use a send port which has been invalidated.
 */
GS_EXPORT NSString * const NSInvalidSendPortException;

/** An exception used when the system fails to allocate required memory.
 */
GS_EXPORT NSString* const NSMallocException;

/**  An exception when a remote object is sent a message from a thread
 *  unable to access the object.
 */
GS_EXPORT NSString* const NSObjectInaccessibleException;

/**  Attempt to send to an object which is no longer available.
 */
GS_EXPORT NSString* const NSObjectNotAvailableException;

/**  UNused ... for MacOS-X compatibility.
 */
GS_EXPORT NSString* const NSOldStyleException;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
/** An exception used when some form of parsing fails.
 */
GS_EXPORT NSString* const NSParseErrorException;
#endif

/** Some failure to receive on a port.
 */
GS_EXPORT NSString * const NSPortReceiveException;

/** Some failure to send on a port.
 */
GS_EXPORT NSString * const NSPortSendException;

/**
 *  Exception raised by [NSPort], [NSConnection], and friends if sufficient
 *  time elapses while waiting for a response, or if the receiving port is
 *  invalidated before a request can be received.  See
 *  [NSConnection-setReplyTimeout:].
 */
GS_EXPORT NSString * const NSPortTimeoutException; /* OPENSTEP */

/** An exception used when an illegal range is encountered ... usually this
 * is used to provide more information than an invalid argument exception.
 */
GS_EXPORT NSString* const NSRangeException;

/**
 * The actual structure for an NSHandler.  You shouldn't need to worry about it.
 */
typedef struct _NSHandler 
{
    jmp_buf jumpState;			/* place to longjmp to */
    struct _NSHandler *next;		/* ptr to next handler */
    __unsafe_unretained NSException *exception;
} NSHandler;

/**
 *  This is the type of the exception handler called when an exception is
 *  generated and not caught by the programmer.  See
 *  NSGetUncaughtExceptionHandler(), NSSetUncaughtExceptionHandler().
 */
typedef void NSUncaughtExceptionHandler(NSException *exception);

/**
 *  Returns the exception handler called when an exception is generated and
 *  not caught by the programmer (by enclosing in <code>NS_DURING</code> and
 *  <code>NS_HANDLER</code>...<code>NS_ENDHANDLER</code>).  The default prints
 *  an error message and exits the program.  You can change this behavior by
 *  calling NSSetUncaughtExceptionHandler().
 */
GS_EXPORT NSUncaughtExceptionHandler *
NSGetUncaughtExceptionHandler();

/**
 *  <p>Sets the exception handler called when an exception is generated and
 *  not caught by the programmer (by enclosing in <code>NS_DURING</code> and
 *  <code>NS_HANDLER</code>...<code>NS_ENDHANDLER</code>).  The default prints
 *  an error message and exits the program.  proc should take a single argument
 *  of type <code>NSException *</code>.
 *  </p>
 *  <p>NB. If the exception handler set by this function does not terminate
 *  the process, the process will be terminateed anyway.  This is a safety
 *  precaution to ensure that, in the event of an exception being raised
 *  and not handled, the program does not try to continue running in a
 *  confused state (possibly doing horrible things like billing customers
 *  who shouldn't be billed etc), but shuts down as cleanly as possible.
 *  </p>
 *  <p>Process termination is normally accomplished by calling the standard
 *  exit function of the C runtime library, but if the environment variable
 *  CRASH_ON_ABORT is set to YES or TRUE or 1 the termination will be
 *  accomplished by calling the abort function instead, which should cause
 *  a core dump to be made for debugging.
 *  </p>
 *  <p>The value of proc should be a pointer to a function taking an
 *  [NSException] instance as an argument.
 *  </p>
 */
GS_EXPORT void
NSSetUncaughtExceptionHandler(NSUncaughtExceptionHandler *handler);

/* NS_DURING, NS_HANDLER and NS_ENDHANDLER are always used like: 

	NS_DURING
	    some code which might raise an error
	NS_HANDLER
	    code that will be jumped to if an error occurs
	NS_ENDHANDLER

   If any error is raised within the first block of code, the second block
   of code will be jumped to.  Typically, this code will clean up any
   resources allocated in the routine, possibly case on the error code
   and perform special processing, and default to RERAISE the error to
   the next handler.  Within the scope of the handler, a local variable
   called "localException" holds information about the exception raised.

   It is illegal to exit the first block of code by any other means than
   NS_VALRETURN, NS_VOIDRETURN, or just falling out the bottom.
 */
#if     USER_NATIVE_OBJC_EXCEPTIONS

# define NS_DURING       @try {
# define NS_HANDLER      } @catch (NSException * localException) {
# define NS_ENDHANDLER   }

# define NS_VALRETURN(val)              return (val)
# define NS_VALUERETURN(object, id)     return (object)
# define NS_VOIDRETURN                  return

#elif   !USER_NATIVE_OBJC_EXCEPTIONS && !BASE_NATIVE_OBJC_EXCEPTIONS

/** Private support routine.  Do not call directly. */
GS_EXPORT void _NSAddHandler( NSHandler *handler );
/** Private support routine.  Do not call directly. */
GS_EXPORT void _NSRemoveHandler( NSHandler *handler );

#define NS_DURING { NSHandler NSLocalHandler;			\
		    _NSAddHandler(&NSLocalHandler);		\
		    if (!setjmp(NSLocalHandler.jumpState)) {

#define NS_HANDLER _NSRemoveHandler(&NSLocalHandler); } else { \
		    NSException __attribute__((unused)) *localException \
		      = NSLocalHandler.exception; \
		    {

#define NS_ENDHANDLER }}}

#define NS_VALRETURN(val)  do { __typeof__(val) temp = (val);	\
			_NSRemoveHandler(&NSLocalHandler);	\
			return(temp); } while (0)

#define NS_VALUERETURN(object, id) do { id temp = object;	\
			_NSRemoveHandler(&NSLocalHandler);	\
			return(temp); } while (0) 

#define NS_VOIDRETURN	do { _NSRemoveHandler(&NSLocalHandler);	\
			return; } while (0)

#endif // _NATIVE_OBJC_EXCEPTIONS

/* ------------------------------------------------------------------------ */
/*   Assertion Handling */
/* ------------------------------------------------------------------------ */

/**
 * <p>NSAssertionHandler objects are used to raise exceptions on behalf of
 * macros implementing assertions.<br />
 * Each thread has its own assertion handler instance.<br />
 * </p>
 * <p>The macros work together with the assertion handler object to
 * produce meaningful exception messages containing the name of the
 * source file, the position within that file, and the name of the
 * ObjC method or C function in which the assertion failed.
 * </p>
 * <p>An NSAssertionHandler instance is created on demand for each thread
 * and is stored in the thread's dictionary under the key NSAssertionHandler.
 * A custom NSAssertionHandler can be used by adding it to the thread
 * dictionary under this key.
 * </p>
 * The assertion macros are:
 * NSAssert(), NSCAssert(),
 * NSAssert1(), NSCAssert1(),
 * NSAssert2(), NSCAssert2(),
 * NSAssert3(), NSCAssert3(),
 * NSAssert4(), NSCAssert4(),
 * NSAssert5(), NSCAssert5(),
 * NSParameterAssert(), NSCParameterAssert()<br />
 * The numbered macros arre obsolete, dating from a time when NSAssert() and
 * NSCAssert() did not support a variable number of arguments.
 */
GS_EXPORT_CLASS
@interface NSAssertionHandler : NSObject

+ (NSAssertionHandler*) currentHandler;

- (void) handleFailureInFunction: (NSString*)functionName 
			    file: (NSString*)fileName 
		      lineNumber: (NSInteger)line 
		     description: (NSString*)format,... GS_NORETURN_METHOD;

- (void) handleFailureInMethod: (SEL)aSelector 
			object: object 
			  file: (NSString*)fileName 
		    lineNumber: (NSInteger)line 
		   description: (NSString*)format,... GS_NORETURN_METHOD;

@end
extern NSString *const NSAssertionHandlerKey;

#ifdef	NS_BLOCK_ASSERTIONS
#define NSAssert(condition, desc, args...)		
#define NSCAssert(condition, desc, args...)	
#else
/** Used in an ObjC method body.<br />
 * See [NSAssertionHandler] for details.<br />
 * When condition is false, raise an exception using desc and args. */
#define NSAssert(condition, desc, args...)			\
  do {							        \
    if (!(condition)) {			                        \
      [[NSAssertionHandler currentHandler] 		        \
        handleFailureInMethod: _cmd 			        \
        object: self 					        \
        file: [NSString stringWithUTF8String: __FILE__] 	\
        lineNumber: __LINE__ 				        \
        description: (desc) , ## args]; 			\
    }							        \
  } while(0)

/** Used in plain C code (not in an ObjC method body).<br />
 * See [NSAssertionHandler] for details.<br />
 * When condition is false, raise an exception using desc
 */
#define NSCAssert(condition, desc, args...)		        \
  do {							        \
    if (!(condition)) {					        \
      [[NSAssertionHandler currentHandler] 		        \
      handleFailureInFunction: [NSString stringWithUTF8String:  \
        __PRETTY_FUNCTION__] 				        \
      file: [NSString stringWithUTF8String: __FILE__] 		\
      lineNumber: __LINE__ 				        \
      description: (desc) , ## args]; 			        \
    }							        \
  } while(0)
#endif

/** Used in an ObjC method body (obsolete ... use NSAssert).<br />
 * See [NSAssertionHandler] for details.<br />
 * When condition is false, raise an exception using desc and arg1, arg2,
 * arg3, arg4, arg5 */
#define NSAssert5(condition, desc, arg1, arg2, arg3, arg4, arg5)	\
  NSAssert((condition), (desc), (arg1), (arg2), (arg3), (arg4), (arg5))

/** Used in an ObjC method body (obsolete ... use NSAssert).<br />
 * See [NSAssertionHandler] for details.<br />
 * When condition is false, raise an exception using desc and arg1, arg2,
 * arg3, arg4 */
#define NSAssert4(condition, desc, arg1, arg2, arg3, arg4)	\
  NSAssert((condition), (desc), (arg1), (arg2), (arg3), (arg4))

/** Used in an ObjC method body (obsolete ... use NSAssert).<br />
 * See [NSAssertionHandler] for details.<br />
 * When condition is false, raise an exception using desc and arg1, arg2,
 * arg3 */
#define NSAssert3(condition, desc, arg1, arg2, arg3)	\
  NSAssert((condition), (desc), (arg1), (arg2), (arg3))

/** Used in an ObjC method body (obsolete ... use NSAssert).<br />
 * See [NSAssertionHandler] for details.<br />
 * When condition is false, raise an exception using desc and arg1, arg2 */
#define NSAssert2(condition, desc, arg1, arg2)		\
  NSAssert((condition), (desc), (arg1), (arg2))

/** Used in an ObjC method body (obsolete ... use NSAssert).<br />
 * See [NSAssertionHandler] for details.<br />
 * When condition is false, raise an exception using desc  and arg1 */
#define NSAssert1(condition, desc, arg1)		\
  NSAssert((condition), (desc), (arg1))

/** Used in an ObjC method body.<br />
 * See [NSAssertionHandler] for details.<br />
 * When condition is false, raise an exception saying that an invalid
 * parameter was supplied to the method. */
#define NSParameterAssert(condition)			\
  NSAssert((condition), @"Invalid parameter not satisfying: %s", #condition)

/** Obsolete ... use NSCAssert().<br />
 * See [NSAssertionHandler] for details.<br />
 * When condition is false, raise an exception using desc and arg1, arg2,
 * arg3, arg4, arg5 */
#define NSCAssert5(condition, desc, arg1, arg2, arg3, arg4, arg5)	\
    NSCAssert((condition), (desc), (arg1), (arg2), (arg3), (arg4), (arg5))

/** Obsolete ... use NSCAssert().<br />
 * See [NSAssertionHandler] for details.<br />
 * When condition is false, raise an exception using desc and arg1, arg2,
 * arg3, arg4 */
#define NSCAssert4(condition, desc, arg1, arg2, arg3, arg4)	\
    NSCAssert((condition), (desc), (arg1), (arg2), (arg3), (arg4))

/** Obsolete ... use NSCAssert().<br />
 * See [NSAssertionHandler] for details.<br />
 * When condition is false, raise an exception using desc and arg1, arg2,
 * arg3 */
#define NSCAssert3(condition, desc, arg1, arg2, arg3)	\
    NSCAssert((condition), (desc), (arg1), (arg2), (arg3))

/** Obsolete ... use NSCAssert().<br />
 * See [NSAssertionHandler] for details.<br />
 * When condition is false, raise an exception using desc and arg1, arg2
 */
#define NSCAssert2(condition, desc, arg1, arg2)		\
    NSCAssert((condition), (desc), (arg1), (arg2))

/** Obsolete ... use NSCAssert().<br />
 * See [NSAssertionHandler] for details.<br />
 * When condition is false, raise an exception using desc and arg1
 */
#define NSCAssert1(condition, desc, arg1)		\
    NSCAssert((condition), (desc), (arg1))

/** Used in plain C code (not in an ObjC method body).<br />
 * See [NSAssertionHandler] for details.<br />
 * When condition is false, raise an exception saying that an invalid
 * parameter was supplied to the method. */
#define NSCParameterAssert(condition)			\
    NSCAssert((condition), @"Invalid parameter not satisfying: %s", #condition)

#if	defined(__cplusplus)
}
#endif

#endif /* __NSException_h_GNUSTEP_BASE_INCLUDE */
