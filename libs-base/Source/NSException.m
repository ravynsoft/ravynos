/** NSException - Object encapsulation of a general exception handler
   Copyright (C) 1993-2013 Free Software Foundation, Inc.

   Written by:  Adam Fedor <fedor@boulder.colorado.edu>
   Date: Mar 1995

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

   $Date$ $Revision$
*/

#import "common.h"
#define	EXPOSE_NSException_IVARS	1
#define	EXPOSE_NSThread_IVARS	1
#import "GSPrivate.h"
#import "Foundation/NSEnumerator.h"
#import "Foundation/NSException.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSCoder.h"
#import "Foundation/NSData.h"
#import "Foundation/NSLock.h"
#import "Foundation/NSNull.h"
#import "Foundation/NSThread.h"
#import "Foundation/NSLock.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSValue.h"
#import "GNUstepBase/NSString+GNUstepBase.h"

#import "GSPThread.h"

#ifdef __GNUSTEP_RUNTIME__
#include <objc/hooks.h>
#endif

#ifdef HAVE_SET_UNCAUGHT_EXCEPTION_HANDLER
#include <objc/objc-exception.h>
#endif

#ifdef HAVE_MALLOC_H
#if !defined(__OpenBSD__)
#include <malloc.h>
#endif
#endif
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#include <stdio.h>

#ifdef HAVE_BACKTRACE
#include <execinfo.h>
#endif

/*
 * Turn off USE_BFD if we don't have bfd support for it.
 */
#if !(defined(HAVE_BFD_H) && defined(HAVE_LIBBFD) && defined(HAVE_LIBIBERTY))
# if defined(USE_BFD)
#   undef USE_BFD
# endif
#endif

#if	defined(_WIN32) && !defined(USE_BFD)
#include <windows.h>
#if	defined(HAVE_DBGHELP_H)
#include <dbghelp.h>
#else
/* Supply the relevant bits from dbghelp.h if we could't find the header.
 */
#define	SYMOPT_UNDNAME		0x00000002
#define	SYMOPT_DEFERRED_LOADS	0x00000004
typedef struct _SYMBOL_INFO {
  ULONG   SizeOfStruct;
  ULONG   TypeIndex;
  uint64_t Reserved[2];
  ULONG   Index;
  ULONG   Size;
  uint64_t ModBase;
  ULONG   Flags;
  uint64_t Value;
  uint64_t Address;
  ULONG   Register;
  ULONG   Scope;
  ULONG   Tag;
  ULONG   NameLen;
  ULONG   MaxNameLen;
  TCHAR   Name[1];
} SYMBOL_INFO;
#endif
#endif

static  NSUncaughtExceptionHandler *_NSUncaughtExceptionHandler = 0;

#define _e_info (((id*)_reserved)[0])
#define _e_stack (((id*)_reserved)[1])


@interface NSException (GSPrivate)
- (GSStackTrace*) _callStack;
@end


#if	defined(_WIN32)
#if	defined(USE_BFD)
static NSString *
GSPrivateBaseAddress(void *addr, void **base)
{
  MEMORY_BASIC_INFORMATION info;

  /* Found a note saying that according to Matt Pietrek's "Under the Hood"
   * column for the April 1997 issue of Microsoft Systems Journal, the
   * allocation base returned by VirtualQuery can be used as the handle
   * to obtain  module information for a loaded library.
   */
  if (VirtualQuery (addr, &info, sizeof(info)) != 0)
    {
      HMODULE	handle = (HMODULE) info.AllocationBase;
      unichar	path[MAX_PATH+1];

      if (GetModuleFileNameW(handle, path, sizeof(path)-1) != 0)
	{
	  path[sizeof(path)-1] = '\0';
	  
	  *base = info.BaseAddress;
	  return [NSString stringWithCharacters: path length: wcslen(path)];
	}
    }
  return nil;
}
#endif  /* USE_BFD */
#else	/* _WIN32 */

#include <dlfcn.h>

#if	defined(USE_BFD)
static NSString *
GSPrivateBaseAddress(void *addr, void **base)
{
#ifdef HAVE_DLADDR
  Dl_info     info;

  if (!dladdr(addr, &info))
    return nil;

  *base = info.dli_fbase;

  return [NSString stringWithUTF8String: info.dli_fname];
#else
  return nil;
#endif
}
#endif  /* USE_BFD */
#endif	/* _WIN32 */

#if	defined(USE_BFD)

// GSStackTrace inspired by  FYStackTrace.m
// created by Wim Oudshoorn on Mon 11-Apr-2006
// reworked by Lloyd Dupont @ NovaMind.com  on 4-May-2006

#include <bfd.h>

@class GSBinaryFileInfo;

@interface GSFunctionInfo : NSObject
{
  void			*_address;
  NSString		*_fileName;
  NSString		*_functionName;
  int			_lineNo;
  GSBinaryFileInfo	*_module;
}
- (void*) address;
- (NSString *) fileName;
- (NSString *) function;
- (id) initWithModule: (GSBinaryFileInfo*)module
	      address: (void*)address 
		 file: (NSString*)file 
	     function: (NSString*)function 
		 line: (int)lineNo;
- (int) lineNumber;
- (GSBinaryFileInfo*) module;

@end


@interface GSBinaryFileInfo : NSObject
{
  NSString	*_fileName;
  bfd		*_abfd;
  asymbol	**_symbols;
  long		_symbolCount;
}
- (NSString *) fileName;
- (GSFunctionInfo *) functionForAddress: (void*) address;
- (id) initWithBinaryFile: (NSString *)fileName;
- (id) init; // return info for the current executing process

@end



@implementation GSFunctionInfo

- (void*) address
{
  return _address;
}

- (oneway void) dealloc
{
  DESTROY(_module);
  DESTROY(_fileName);
  DESTROY(_functionName);
  [super dealloc];
}

- (NSString *) description
{
  return [NSString stringWithFormat: @"(%@: %p) %@  %@: %d",
    [_module fileName], _address, _functionName, _fileName, _lineNo];
}

- (NSString *) fileName
{
  return _fileName;
}

- (NSString *) function
{
  return _functionName;
}

- (id) init
{
  DESTROY(self);
  return nil;
}

- (id) initWithModule: (GSBinaryFileInfo*)module
	      address: (void*)address 
		 file: (NSString*)file 
	     function: (NSString*)function 
		 line: (int)lineNo
{
  _module = RETAIN(module);
  _address = address;
  _fileName = [file copy];
  _functionName = [function copy];
  _lineNo = lineNo;

  return self;
}

- (int) lineNumber
{
  return _lineNo;
}

- (GSBinaryFileInfo *) module
{
  return _module;
}

@end



@implementation GSBinaryFileInfo

+ (GSBinaryFileInfo*) infoWithBinaryFile: (NSString *)fileName
{
  return [[[self alloc] initWithBinaryFile: fileName] autorelease];
}

+ (void) initialize
{
  static BOOL first = YES;

  if (first == NO)
    {
      return;
    }
  first = NO;
  bfd_init ();
}

- (oneway void) dealloc
{
  DESTROY(_fileName);
  if (_abfd)
    {
      bfd_close (_abfd);
      _abfd = NULL;
    }
  if (_symbols)
    {
      free(_symbols);
      _symbols = NULL;
    }
  [super dealloc];
}

- (NSString *) fileName
{
  return _fileName;
}

- (id) init
{
  NSString *processName;

  processName = [[[NSProcessInfo processInfo] arguments] objectAtIndex: 0];
  return [self initWithBinaryFile: processName];
}

- (id) initWithBinaryFile: (NSString *)fileName
{
  int neededSpace;

  // 1st initialize the bfd
  if ([fileName length] == 0)
    {
      //NSLog (@"GSBinaryFileInfo: No File");
      DESTROY(self);
      return nil;
    }
  _fileName = [fileName copy];
  _abfd = bfd_openr ([fileName cString], NULL);
  if (!_abfd)
    {
      //NSLog (@"GSBinaryFileInfo: No Binary Info");
      DESTROY(self);
      return nil;
    }
  if (!bfd_check_format_matches (_abfd, bfd_object, NULL))
    {
      //NSLog (@"GSBinaryFileInfo: BFD format object error");
      DESTROY(self);
      return nil;
    }

  // second read the symbols from it
  if (!(bfd_get_file_flags (_abfd) & HAS_SYMS))
    {
      //NSLog (@"GSBinaryFileInfo: BFD does not contain any symbols");
      DESTROY(self);
      return nil;
    }

  neededSpace = bfd_get_symtab_upper_bound (_abfd);
  if (neededSpace < 0)
    {
      //NSLog (@"GSBinaryFileInfo: BFD error while deducing needed space");
      DESTROY(self);
      return nil;
    }
  if (neededSpace == 0)
    {
      //NSLog (@"GSBinaryFileInfo: BFD no space for symbols needed");
      DESTROY(self);
      return nil;
    }
  _symbols = malloc (neededSpace);
  if (!_symbols)
    {
      //NSLog (@"GSBinaryFileInfo: Can't allocate buffer");
      DESTROY(self);
      return nil;
    }
  _symbolCount = bfd_canonicalize_symtab (_abfd, _symbols);
  if (_symbolCount < 0)
    {
      //NSLog (@"GSBinaryFileInfo: BFD error while reading symbols");
      DESTROY(self);
      return nil;
    }

  return self;
}

struct SearchAddressStruct
{
  void			*theAddress;
  GSBinaryFileInfo	*module;
  asymbol		**symbols;
  GSFunctionInfo	*theInfo;
};

static void find_address (bfd *abfd, asection *section,
  struct SearchAddressStruct *info)
{
  bfd_vma	address;
  bfd_vma	vma;
  unsigned	size;
  const char	*fileName = 0;
  const char	*functionName = 0;
  unsigned	line = 0;

  if (info->theInfo)
    {
      return;
    }
  if (!(bfd_get_section_flags (abfd, section) & SEC_ALLOC))
    {
      return;	// Only debug in this section
    }
  if (bfd_get_section_flags (abfd, section) & SEC_DATA)
    {
      return;	// Only data in this section
    }

  address = (bfd_vma) (uintptr_t)info->theAddress;

  vma = bfd_get_section_vma (abfd, section);

#if     defined(bfd_get_section_size_before_reloc)
  size = bfd_get_section_size_before_reloc (section);        // recent
#elif     defined(bfd_get_section_size)
  size = bfd_get_section_size (section);        // less recent
#else                                
  size = bfd_section_size (abfd, section);      // older version
#endif                               

  if (address < vma || address >= vma + size)
    {
      return;
    }


  if (bfd_find_nearest_line (abfd, section, info->symbols,
    address - vma, &fileName, &functionName, &line))
    {
      GSFunctionInfo	*fi;
      NSString		*file = nil;
      NSString		*func = nil;

      if (fileName != 0)
        {
	  file = [NSString stringWithCString: fileName 
	    encoding: [NSString defaultCStringEncoding]];
	}
      if (functionName != 0)
        {
	  func = [NSString stringWithCString: functionName 
	    encoding: [NSString defaultCStringEncoding]];
	}
      fi = [GSFunctionInfo alloc];
      fi = [fi initWithModule: info->module
		      address: info->theAddress
			 file: file
		     function: func
			 line: line];
      [fi autorelease];
      info->theInfo = fi;
    }
}

- (GSFunctionInfo *) functionForAddress: (void*) address
{
  struct SearchAddressStruct searchInfo =
    { address, self, _symbols, nil };

  bfd_map_over_sections (_abfd,
    (void (*) (bfd *, asection *, void *)) find_address, &searchInfo);
  return searchInfo.theInfo;
}

@end

static pthread_mutex_t	        modLock;
static NSMutableDictionary	*stackModules = nil;

// initialize stack trace info
static id
GSLoadModule(NSString *fileName)
{
  GSBinaryFileInfo	*module = nil;

  (void)pthread_mutex_lock(&modLock);

  if (stackModules == nil)
    {
      NSEnumerator	*enumerator;
      NSBundle		*bundle;

      stackModules = [NSMutableDictionary new];

      /*
       * Try to ensure we have the main, base and gui library bundles.
       */
      [NSBundle mainBundle];
      [NSBundle bundleForClass: [NSObject class]];
      [NSBundle bundleForClass: NSClassFromString(@"NSView")];

      /*
       * Add file info for all bundles with code.
       */
      enumerator = [[NSBundle allBundles] objectEnumerator];
      while ((bundle = [enumerator nextObject]) != nil)
	{
	  if ([bundle load] == YES)
	    {
	      GSLoadModule([bundle executablePath]);
	    }
	}
    }

  if ([fileName length] > 0)
    {
      module = [stackModules objectForKey: fileName];
      if (module == nil)
	{
	  module = [GSBinaryFileInfo infoWithBinaryFile: fileName];
	  if (module == nil)
	    {
	      module = (id)[NSNull null];
	    }
	  if ([stackModules objectForKey: fileName] == nil)
	    {
	      [stackModules setObject: module forKey: fileName];
	    }
	  else
	    {
	      module = [stackModules objectForKey: fileName];
	    }
	}
    }
  (void)pthread_mutex_unlock(&modLock);

  if (module == (id)[NSNull null])
    {
      module = nil;
    }
  return module;
}

static NSArray*
GSListModules()
{
  NSArray	*result;

  GSLoadModule(nil);	// initialise
  (void)pthread_mutex_lock(&modLock);
  result = [stackModules allValues];
  (void)pthread_mutex_unlock(&modLock);
  return result;
}

#endif	/* USE_BFD */


#if defined(WITH_UNWIND) && !defined(HAVE_BACKTRACE)

#include <unwind.h>
#if	!defined(_WIN32)
#include <dlfcn.h>
#endif

struct GSBacktraceState
{
  void **current;
  void **end;
};

static _Unwind_Reason_Code
GSUnwindCallback(struct _Unwind_Context* context, void* arg)
{
    struct GSBacktraceState *state = (struct GSBacktraceState*)arg;
    uintptr_t pc = _Unwind_GetIP(context);
    if (pc) {
      if (state->current == state->end) {
        return _URC_END_OF_STACK;
      } else {
        *state->current++ = (void*)pc;
      }
    }
    return 0; //_URC_OK/_URC_NO_REASON
}

#endif	/* WITH_UNWIND && !HAVE_BACKTRACE */


#if	defined(_WIN32) && !defined(USE_BFD)
typedef USHORT (WINAPI *CaptureStackBackTraceType)(ULONG,ULONG,PVOID*,PULONG);
typedef BOOL (WINAPI *SymInitializeType)(HANDLE,char*,BOOL);
typedef DWORD (WINAPI *SymSetOptionsType)(DWORD);
typedef BOOL (WINAPI *SymFromAddrType)(HANDLE,DWORD64,PDWORD64,SYMBOL_INFO*);

static CaptureStackBackTraceType capture = 0;
static SymInitializeType initSym = 0;
static SymSetOptionsType optSym = 0;
static SymFromAddrType fromSym = 0;
static HANDLE	hProcess = 0;
static	pthread_mutex_t	traceLock;
#define	MAXFRAMES 62	/* Limitation of windows-xp */
#else
#define MAXFRAMES 128   /* 1KB buffer on 64bit machine */
#endif


#if	!defined(HAVE_BUILTIN_EXTRACT_RETURN_ADDRESS)
# define	__builtin_extract_return_address(X)	X
#endif

#define _NS_FRAME_HACK(a) \
case a: env->addr = __builtin_frame_address(a + 1); break;
#define _NS_RETURN_HACK(a) \
case a: env->addr = (__builtin_frame_address(a + 1) ? \
__builtin_extract_return_address(__builtin_return_address(a + 1)) : 0); break;

/*
 * The following horrible signal handling code is a workaround for the fact
 * that the __builtin_frame_address() and __builtin_return_address()
 * functions are not reliable (at least not on my EM64T based system) and
 * will sometimes walk off the stack and access illegal memory locations.
 * In order to prevent such an occurrance from crashing the application,
 * we use sigsetjmp() and siglongjmp() to ensure that we can recover, and
 * we keep the jump buffer in thread-local memory to avoid possible thread
 * safety issues.
 * Of course this will fail horribly if an exception occurs in one of the
 * few methods we use to manage the per-thread jump buffer.
 */
#if	defined(HAVE_SYS_SIGNAL_H)
#  include	<sys/signal.h>
#elif	defined(HAVE_SIGNAL_H)
#  include	<signal.h>
#endif

#if	defined(_WIN32)
#ifndef SIGBUS
#define SIGBUS  SIGILL
#endif
#endif

/* sigsetjmp may be a function or a macro.  The test for the function is
 * done at configure time so we can tell here if either is available.
 */
#if	!defined(HAVE_SIGSETJMP) && !defined(sigsetjmp)
#define	siglongjmp(A,B)	longjmp(A,B)
#define	sigsetjmp(A,B)	setjmp(A)
#define	sigjmp_buf	jmp_buf
#endif

typedef struct {
  sigjmp_buf    buf;
  void          *addr;
  void          (*bus)(int);
  void          (*segv)(int);
} jbuf_type;

static jbuf_type *
jbuf()
{
  NSMutableData	*d;
  NSMutableDictionary	*dict;

  dict = [[NSThread currentThread] threadDictionary];
  d = [dict objectForKey: @"GSjbuf"];
  if (d == nil)
    {
      d = [[NSMutableData alloc] initWithLength: sizeof(jbuf_type)];
      [dict setObject: d forKey: @"GSjbuf"];
      RELEASE(d);
    }
  return (jbuf_type*)[d mutableBytes];
}

static void
recover(int sig)
{
  siglongjmp(jbuf()->buf, 1);
}

#ifdef	__clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wframe-address"
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wframe-address"
#endif

void *
NSFrameAddress(NSUInteger offset)
{
  jbuf_type     *env;

  env = jbuf();
  if (sigsetjmp(env->buf, 1) == 0)
    {
      env->segv = signal(SIGSEGV, recover);
      env->bus = signal(SIGBUS, recover);
      switch (offset)
	{
	  _NS_FRAME_HACK(0); _NS_FRAME_HACK(1); _NS_FRAME_HACK(2);
	  _NS_FRAME_HACK(3); _NS_FRAME_HACK(4); _NS_FRAME_HACK(5);
	  _NS_FRAME_HACK(6); _NS_FRAME_HACK(7); _NS_FRAME_HACK(8);
	  _NS_FRAME_HACK(9); _NS_FRAME_HACK(10); _NS_FRAME_HACK(11);
	  _NS_FRAME_HACK(12); _NS_FRAME_HACK(13); _NS_FRAME_HACK(14);
	  _NS_FRAME_HACK(15); _NS_FRAME_HACK(16); _NS_FRAME_HACK(17);
	  _NS_FRAME_HACK(18); _NS_FRAME_HACK(19); _NS_FRAME_HACK(20);
	  _NS_FRAME_HACK(21); _NS_FRAME_HACK(22); _NS_FRAME_HACK(23);
	  _NS_FRAME_HACK(24); _NS_FRAME_HACK(25); _NS_FRAME_HACK(26);
	  _NS_FRAME_HACK(27); _NS_FRAME_HACK(28); _NS_FRAME_HACK(29);
	  _NS_FRAME_HACK(30); _NS_FRAME_HACK(31); _NS_FRAME_HACK(32);
	  _NS_FRAME_HACK(33); _NS_FRAME_HACK(34); _NS_FRAME_HACK(35);
	  _NS_FRAME_HACK(36); _NS_FRAME_HACK(37); _NS_FRAME_HACK(38);
	  _NS_FRAME_HACK(39); _NS_FRAME_HACK(40); _NS_FRAME_HACK(41);
	  _NS_FRAME_HACK(42); _NS_FRAME_HACK(43); _NS_FRAME_HACK(44);
	  _NS_FRAME_HACK(45); _NS_FRAME_HACK(46); _NS_FRAME_HACK(47);
	  _NS_FRAME_HACK(48); _NS_FRAME_HACK(49); _NS_FRAME_HACK(50);
	  _NS_FRAME_HACK(51); _NS_FRAME_HACK(52); _NS_FRAME_HACK(53);
	  _NS_FRAME_HACK(54); _NS_FRAME_HACK(55); _NS_FRAME_HACK(56);
	  _NS_FRAME_HACK(57); _NS_FRAME_HACK(58); _NS_FRAME_HACK(59);
	  _NS_FRAME_HACK(60); _NS_FRAME_HACK(61); _NS_FRAME_HACK(62);
	  _NS_FRAME_HACK(63); _NS_FRAME_HACK(64); _NS_FRAME_HACK(65);
	  _NS_FRAME_HACK(66); _NS_FRAME_HACK(67); _NS_FRAME_HACK(68);
	  _NS_FRAME_HACK(69); _NS_FRAME_HACK(70); _NS_FRAME_HACK(71);
	  _NS_FRAME_HACK(72); _NS_FRAME_HACK(73); _NS_FRAME_HACK(74);
	  _NS_FRAME_HACK(75); _NS_FRAME_HACK(76); _NS_FRAME_HACK(77);
	  _NS_FRAME_HACK(78); _NS_FRAME_HACK(79); _NS_FRAME_HACK(80);
	  _NS_FRAME_HACK(81); _NS_FRAME_HACK(82); _NS_FRAME_HACK(83);
	  _NS_FRAME_HACK(84); _NS_FRAME_HACK(85); _NS_FRAME_HACK(86);
	  _NS_FRAME_HACK(87); _NS_FRAME_HACK(88); _NS_FRAME_HACK(89);
	  _NS_FRAME_HACK(90); _NS_FRAME_HACK(91); _NS_FRAME_HACK(92);
	  _NS_FRAME_HACK(93); _NS_FRAME_HACK(94); _NS_FRAME_HACK(95);
	  _NS_FRAME_HACK(96); _NS_FRAME_HACK(97); _NS_FRAME_HACK(98);
	  _NS_FRAME_HACK(99);
	  default: env->addr = NULL; break;
	}
      signal(SIGSEGV, env->segv);
      signal(SIGBUS, env->bus);
    }
  else
    {
      env = jbuf();
      signal(SIGSEGV, env->segv);
      signal(SIGBUS, env->bus);
      env->addr = NULL;
    }
  return env->addr;
}

NSUInteger NSCountFrames(void)
{
  jbuf_type	*env;

  env = jbuf();
  if (sigsetjmp(env->buf, 1) == 0)
    {
      env->segv = signal(SIGSEGV, recover);
      env->bus = signal(SIGBUS, recover);
      env->addr = 0;

#define _NS_COUNT_HACK(X) if (__builtin_frame_address(X + 1) == 0) \
        goto done; else env->addr = (void*)(X + 1);

      _NS_COUNT_HACK(0); _NS_COUNT_HACK(1); _NS_COUNT_HACK(2);
      _NS_COUNT_HACK(3); _NS_COUNT_HACK(4); _NS_COUNT_HACK(5);
      _NS_COUNT_HACK(6); _NS_COUNT_HACK(7); _NS_COUNT_HACK(8);
      _NS_COUNT_HACK(9); _NS_COUNT_HACK(10); _NS_COUNT_HACK(11);
      _NS_COUNT_HACK(12); _NS_COUNT_HACK(13); _NS_COUNT_HACK(14);
      _NS_COUNT_HACK(15); _NS_COUNT_HACK(16); _NS_COUNT_HACK(17);
      _NS_COUNT_HACK(18); _NS_COUNT_HACK(19); _NS_COUNT_HACK(20);
      _NS_COUNT_HACK(21); _NS_COUNT_HACK(22); _NS_COUNT_HACK(23);
      _NS_COUNT_HACK(24); _NS_COUNT_HACK(25); _NS_COUNT_HACK(26);
      _NS_COUNT_HACK(27); _NS_COUNT_HACK(28); _NS_COUNT_HACK(29);
      _NS_COUNT_HACK(30); _NS_COUNT_HACK(31); _NS_COUNT_HACK(32);
      _NS_COUNT_HACK(33); _NS_COUNT_HACK(34); _NS_COUNT_HACK(35);
      _NS_COUNT_HACK(36); _NS_COUNT_HACK(37); _NS_COUNT_HACK(38);
      _NS_COUNT_HACK(39); _NS_COUNT_HACK(40); _NS_COUNT_HACK(41);
      _NS_COUNT_HACK(42); _NS_COUNT_HACK(43); _NS_COUNT_HACK(44);
      _NS_COUNT_HACK(45); _NS_COUNT_HACK(46); _NS_COUNT_HACK(47);
      _NS_COUNT_HACK(48); _NS_COUNT_HACK(49); _NS_COUNT_HACK(50);
      _NS_COUNT_HACK(51); _NS_COUNT_HACK(52); _NS_COUNT_HACK(53);
      _NS_COUNT_HACK(54); _NS_COUNT_HACK(55); _NS_COUNT_HACK(56);
      _NS_COUNT_HACK(57); _NS_COUNT_HACK(58); _NS_COUNT_HACK(59);
      _NS_COUNT_HACK(60); _NS_COUNT_HACK(61); _NS_COUNT_HACK(62);
      _NS_COUNT_HACK(63); _NS_COUNT_HACK(64); _NS_COUNT_HACK(65);
      _NS_COUNT_HACK(66); _NS_COUNT_HACK(67); _NS_COUNT_HACK(68);
      _NS_COUNT_HACK(69); _NS_COUNT_HACK(70); _NS_COUNT_HACK(71);
      _NS_COUNT_HACK(72); _NS_COUNT_HACK(73); _NS_COUNT_HACK(74);
      _NS_COUNT_HACK(75); _NS_COUNT_HACK(76); _NS_COUNT_HACK(77);
      _NS_COUNT_HACK(78); _NS_COUNT_HACK(79); _NS_COUNT_HACK(80);
      _NS_COUNT_HACK(81); _NS_COUNT_HACK(82); _NS_COUNT_HACK(83);
      _NS_COUNT_HACK(84); _NS_COUNT_HACK(85); _NS_COUNT_HACK(86);
      _NS_COUNT_HACK(87); _NS_COUNT_HACK(88); _NS_COUNT_HACK(89);
      _NS_COUNT_HACK(90); _NS_COUNT_HACK(91); _NS_COUNT_HACK(92);
      _NS_COUNT_HACK(93); _NS_COUNT_HACK(94); _NS_COUNT_HACK(95);
      _NS_COUNT_HACK(96); _NS_COUNT_HACK(97); _NS_COUNT_HACK(98);
      _NS_COUNT_HACK(99);

done:
      signal(SIGSEGV, env->segv);
      signal(SIGBUS, env->bus);
    }
  else
    {
      env = jbuf();
      signal(SIGSEGV, env->segv);
      signal(SIGBUS, env->bus);
    }

  return (uintptr_t)env->addr;
}

void *
NSReturnAddress(NSUInteger offset)
{
  jbuf_type	*env;

  env = jbuf();
  if (sigsetjmp(env->buf, 1) == 0)
    {
      env->segv = signal(SIGSEGV, recover);
      env->bus = signal(SIGBUS, recover);
      switch (offset)
	{
	  _NS_RETURN_HACK(0); _NS_RETURN_HACK(1); _NS_RETURN_HACK(2);
	  _NS_RETURN_HACK(3); _NS_RETURN_HACK(4); _NS_RETURN_HACK(5);
	  _NS_RETURN_HACK(6); _NS_RETURN_HACK(7); _NS_RETURN_HACK(8);
	  _NS_RETURN_HACK(9); _NS_RETURN_HACK(10); _NS_RETURN_HACK(11);
	  _NS_RETURN_HACK(12); _NS_RETURN_HACK(13); _NS_RETURN_HACK(14);
	  _NS_RETURN_HACK(15); _NS_RETURN_HACK(16); _NS_RETURN_HACK(17);
	  _NS_RETURN_HACK(18); _NS_RETURN_HACK(19); _NS_RETURN_HACK(20);
	  _NS_RETURN_HACK(21); _NS_RETURN_HACK(22); _NS_RETURN_HACK(23);
	  _NS_RETURN_HACK(24); _NS_RETURN_HACK(25); _NS_RETURN_HACK(26);
	  _NS_RETURN_HACK(27); _NS_RETURN_HACK(28); _NS_RETURN_HACK(29);
	  _NS_RETURN_HACK(30); _NS_RETURN_HACK(31); _NS_RETURN_HACK(32);
	  _NS_RETURN_HACK(33); _NS_RETURN_HACK(34); _NS_RETURN_HACK(35);
	  _NS_RETURN_HACK(36); _NS_RETURN_HACK(37); _NS_RETURN_HACK(38);
	  _NS_RETURN_HACK(39); _NS_RETURN_HACK(40); _NS_RETURN_HACK(41);
	  _NS_RETURN_HACK(42); _NS_RETURN_HACK(43); _NS_RETURN_HACK(44);
	  _NS_RETURN_HACK(45); _NS_RETURN_HACK(46); _NS_RETURN_HACK(47);
	  _NS_RETURN_HACK(48); _NS_RETURN_HACK(49); _NS_RETURN_HACK(50);
	  _NS_RETURN_HACK(51); _NS_RETURN_HACK(52); _NS_RETURN_HACK(53);
	  _NS_RETURN_HACK(54); _NS_RETURN_HACK(55); _NS_RETURN_HACK(56);
	  _NS_RETURN_HACK(57); _NS_RETURN_HACK(58); _NS_RETURN_HACK(59);
	  _NS_RETURN_HACK(60); _NS_RETURN_HACK(61); _NS_RETURN_HACK(62);
	  _NS_RETURN_HACK(63); _NS_RETURN_HACK(64); _NS_RETURN_HACK(65);
	  _NS_RETURN_HACK(66); _NS_RETURN_HACK(67); _NS_RETURN_HACK(68);
	  _NS_RETURN_HACK(69); _NS_RETURN_HACK(70); _NS_RETURN_HACK(71);
	  _NS_RETURN_HACK(72); _NS_RETURN_HACK(73); _NS_RETURN_HACK(74);
	  _NS_RETURN_HACK(75); _NS_RETURN_HACK(76); _NS_RETURN_HACK(77);
	  _NS_RETURN_HACK(78); _NS_RETURN_HACK(79); _NS_RETURN_HACK(80);
	  _NS_RETURN_HACK(81); _NS_RETURN_HACK(82); _NS_RETURN_HACK(83);
	  _NS_RETURN_HACK(84); _NS_RETURN_HACK(85); _NS_RETURN_HACK(86);
	  _NS_RETURN_HACK(87); _NS_RETURN_HACK(88); _NS_RETURN_HACK(89);
	  _NS_RETURN_HACK(90); _NS_RETURN_HACK(91); _NS_RETURN_HACK(92);
	  _NS_RETURN_HACK(93); _NS_RETURN_HACK(94); _NS_RETURN_HACK(95);
	  _NS_RETURN_HACK(96); _NS_RETURN_HACK(97); _NS_RETURN_HACK(98);
	  _NS_RETURN_HACK(99);
	  default: env->addr = NULL; break;
	}
      signal(SIGSEGV, env->segv);
      signal(SIGBUS, env->bus);
    }
  else
    {
      env = jbuf();
      signal(SIGSEGV, env->segv);
      signal(SIGBUS, env->bus);
      env->addr = NULL;
    }

  return env->addr;
}

#ifdef	__clang__
#pragma clang diagnostic pop
#else
#pragma GCC diagnostic pop
#endif

unsigned
GSPrivateReturnAddresses(NSUInteger **returns)
{
  unsigned      numReturns;
#if	defined(HAVE_BACKTRACE)
  void          *addr[MAXFRAMES*sizeof(void*)];

  numReturns = backtrace(addr, MAXFRAMES);
  if (numReturns > 0)
    {
      *returns = malloc(numReturns * sizeof(void*));
      memcpy(*returns, addr, numReturns * sizeof(void*));
    }
#elif defined(WITH_UNWIND)
  void          *addr[MAXFRAMES];
  
  struct GSBacktraceState state = {addr, addr + MAXFRAMES};
  _Unwind_Backtrace(GSUnwindCallback, &state);

  numReturns = state.current - addr;
  if (numReturns > 0)
    {
      *returns = malloc(numReturns * sizeof(void*));
      memcpy(*returns, addr, numReturns * sizeof(void*));
    }
#elif	defined(_WIN32) && !defined(USE_BFD)
  NSUInteger	addr[MAXFRAMES];

  (void)pthread_mutex_lock(&traceLock);
  if (0 == hProcess)
    {
      hProcess = GetCurrentProcess();

      if (0 == capture)
	{
	  HANDLE	hModule;

	  hModule = LoadLibrary("kernel32.dll");
	  if (0 == hModule)
	    {
	      fprintf(stderr, "Failed to load kernel32.dll with error: %d\n",
		(int)GetLastError());
	      (void)pthread_mutex_unlock(&traceLock);
	      return 0;
	    }
	  capture = (CaptureStackBackTraceType)GetProcAddress(
	    hModule, "RtlCaptureStackBackTrace");
	  if (0 == capture)
	    {
	      fprintf(stderr, "Failed to find RtlCaptureStackBackTrace: %d\n",
		(int)GetLastError());
	      (void)pthread_mutex_unlock(&traceLock);
	      return 0;
	    }
	  hModule = LoadLibrary("dbghelp.dll");
	  if (0 == hModule)
	    {
	      fprintf(stderr, "Failed to load dbghelp.dll with error: %d\n",
		(int)GetLastError());
	      (void)pthread_mutex_unlock(&traceLock);
	      return 0;
	    }
	  optSym = (SymSetOptionsType)GetProcAddress(
	    hModule, "SymSetOptions");
	  if (0 == optSym)
	    {
	      fprintf(stderr, "Failed to find SymSetOptions: %d\n",
		(int)GetLastError());
	      (void)pthread_mutex_unlock(&traceLock);
	      return 0;
	    }
	  initSym = (SymInitializeType)GetProcAddress(
	    hModule, "SymInitialize");
	  if (0 == initSym)
	    {
	      fprintf(stderr, "Failed to find SymInitialize: %d\n",
		(int)GetLastError());
	      (void)pthread_mutex_unlock(&traceLock);
	      return 0;
	    }
	  fromSym = (SymFromAddrType)GetProcAddress(
	    hModule, "SymFromAddr");
	  if (0 == fromSym)
	    {
	      fprintf(stderr, "Failed to find SymFromAddr: %d\n",
		(int)GetLastError());
	      (void)pthread_mutex_unlock(&traceLock);
	      return 0;
	    }
	}

      (optSym)(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);

      if (!(initSym)(hProcess, NULL, TRUE))
	{
	  fprintf(stderr, "SymInitialize failed with error: %d\n",
	    (int)GetLastError());
	  fromSym = 0;
	  (void)pthread_mutex_unlock(&traceLock);
	  return 0;
	}
    }
  if (0 == capture)
    {
      (void)pthread_mutex_unlock(&traceLock);
      return 0;
    }

  numReturns = (capture)(0, MAXFRAMES, (void**)addr, NULL);
  if (numReturns > 0)
    {
      *returns = malloc(numReturns * sizeof(void*));
      memcpy(*returns, addr, numReturns * sizeof(void*));
    }
  
  (void)pthread_mutex_unlock(&traceLock);

#else
  int   n;

  n = NSCountFrames();
  /* There should be more frame addresses than return addresses.
   */
  if (n > 0)
    {
      n--;
    }
  if (n > 0)
    {
      n--;
    }

  if ((numReturns = n) > 0)
    {
      jbuf_type *env;

      *returns = malloc(numReturns * sizeof(void*));

      env = jbuf();
      if (sigsetjmp(env->buf, 1) == 0)
        {
          unsigned      i;

          env->segv = signal(SIGSEGV, recover);
          env->bus = signal(SIGBUS, recover);

          for (i = 0; i < n; i++)
            {
              switch (i)
                {
                  _NS_RETURN_HACK(0); _NS_RETURN_HACK(1); _NS_RETURN_HACK(2);
                  _NS_RETURN_HACK(3); _NS_RETURN_HACK(4); _NS_RETURN_HACK(5);
                  _NS_RETURN_HACK(6); _NS_RETURN_HACK(7); _NS_RETURN_HACK(8);
                  _NS_RETURN_HACK(9); _NS_RETURN_HACK(10); _NS_RETURN_HACK(11);
                  _NS_RETURN_HACK(12); _NS_RETURN_HACK(13); _NS_RETURN_HACK(14);
                  _NS_RETURN_HACK(15); _NS_RETURN_HACK(16); _NS_RETURN_HACK(17);
                  _NS_RETURN_HACK(18); _NS_RETURN_HACK(19); _NS_RETURN_HACK(20);
                  _NS_RETURN_HACK(21); _NS_RETURN_HACK(22); _NS_RETURN_HACK(23);
                  _NS_RETURN_HACK(24); _NS_RETURN_HACK(25); _NS_RETURN_HACK(26);
                  _NS_RETURN_HACK(27); _NS_RETURN_HACK(28); _NS_RETURN_HACK(29);
                  _NS_RETURN_HACK(30); _NS_RETURN_HACK(31); _NS_RETURN_HACK(32);
                  _NS_RETURN_HACK(33); _NS_RETURN_HACK(34); _NS_RETURN_HACK(35);
                  _NS_RETURN_HACK(36); _NS_RETURN_HACK(37); _NS_RETURN_HACK(38);
                  _NS_RETURN_HACK(39); _NS_RETURN_HACK(40); _NS_RETURN_HACK(41);
                  _NS_RETURN_HACK(42); _NS_RETURN_HACK(43); _NS_RETURN_HACK(44);
                  _NS_RETURN_HACK(45); _NS_RETURN_HACK(46); _NS_RETURN_HACK(47);
                  _NS_RETURN_HACK(48); _NS_RETURN_HACK(49); _NS_RETURN_HACK(50);
                  _NS_RETURN_HACK(51); _NS_RETURN_HACK(52); _NS_RETURN_HACK(53);
                  _NS_RETURN_HACK(54); _NS_RETURN_HACK(55); _NS_RETURN_HACK(56);
                  _NS_RETURN_HACK(57); _NS_RETURN_HACK(58); _NS_RETURN_HACK(59);
                  _NS_RETURN_HACK(60); _NS_RETURN_HACK(61); _NS_RETURN_HACK(62);
                  _NS_RETURN_HACK(63); _NS_RETURN_HACK(64); _NS_RETURN_HACK(65);
                  _NS_RETURN_HACK(66); _NS_RETURN_HACK(67); _NS_RETURN_HACK(68);
                  _NS_RETURN_HACK(69); _NS_RETURN_HACK(70); _NS_RETURN_HACK(71);
                  _NS_RETURN_HACK(72); _NS_RETURN_HACK(73); _NS_RETURN_HACK(74);
                  _NS_RETURN_HACK(75); _NS_RETURN_HACK(76); _NS_RETURN_HACK(77);
                  _NS_RETURN_HACK(78); _NS_RETURN_HACK(79); _NS_RETURN_HACK(80);
                  _NS_RETURN_HACK(81); _NS_RETURN_HACK(82); _NS_RETURN_HACK(83);
                  _NS_RETURN_HACK(84); _NS_RETURN_HACK(85); _NS_RETURN_HACK(86);
                  _NS_RETURN_HACK(87); _NS_RETURN_HACK(88); _NS_RETURN_HACK(89);
                  _NS_RETURN_HACK(90); _NS_RETURN_HACK(91); _NS_RETURN_HACK(92);
                  _NS_RETURN_HACK(93); _NS_RETURN_HACK(94); _NS_RETURN_HACK(95);
                  _NS_RETURN_HACK(96); _NS_RETURN_HACK(97); _NS_RETURN_HACK(98);
                  _NS_RETURN_HACK(99);
                  default: env->addr = 0; break;
                }
              if (env->addr == 0)
                {
                  break;
                }
              memcpy(&(*returns)[i], env->addr, sizeof(void*));
            }
          signal(SIGSEGV, env->segv);
          signal(SIGBUS, env->bus);
        }
      else
        {
          env = jbuf();
          signal(SIGSEGV, env->segv);
          signal(SIGBUS, env->bus);
        }
    }
#endif
  return numReturns;
}


@implementation GSStackTrace : NSObject

/** Offset from the top of the stack (when we generate a trace) to the
 * first frame likely to be of interest for debugging.
 */
#define FrameOffset     4

+ (void) initialize
{
#if	defined(_WIN32) && !defined(USE_BFD)
  GS_INIT_RECURSIVE_MUTEX(traceLock);
#endif
#if     defined(USE_BFD)
  GS_INIT_RECURSIVE_MUTEX(modLock);
#endif
}

- (NSArray*) addresses
{
  if (nil == addresses && numReturns > FrameOffset)
    {
      ENTER_POOL
      NSInteger         count = numReturns - FrameOffset;
      NSValue           *objects[count];
      NSUInteger        index;
      void              **ptrs = (void **)returns;

      for (index = 0; index < count; index++)
        {
          objects[index] = [NSValue valueWithPointer: ptrs[FrameOffset+index]];
        }
      addresses = [[NSArray alloc] initWithObjects: objects count: count];
      LEAVE_POOL
    }
  return addresses;
}

- (oneway void) dealloc
{
  DESTROY(addresses);
  DESTROY(symbols);
  if (returns != NULL)
    {
      free(returns);
      returns = NULL;
    }
  [super dealloc];
}

- (NSString*) description
{
  NSMutableString *result;
  NSArray *s;
  int i;
  int n;

  result = [NSMutableString string];
  s = [self symbols];
  n = [s count];
  for (i = 0; i < n; i++)
    {
      NSString	*line = [s objectAtIndex: i];

      [result appendFormat: @"%3d: %@\n", i, line];
    }
  return result;
}

- (id) init
{
  return self;
}

- (NSArray*) symbols
{
  if (nil == symbols && numReturns > FrameOffset)
    {
      NSInteger	        count = numReturns - FrameOffset;
      NSUInteger        i;

#if	defined(USE_BFD)
      void              **ptrs = (void**)&returns[FrameOffset];
      NSMutableArray	*a;

      a = [[NSMutableArray alloc] initWithCapacity: count];

      for (i = 0; i < count; i++)
        {
          GSFunctionInfo	*aFrame = nil;
          void		        *address = (void*)*ptrs++;
          void		        *base;
          NSString		*modulePath;
          GSBinaryFileInfo	*bfi;

          modulePath = GSPrivateBaseAddress(address, &base);
          if (modulePath != nil && (bfi = GSLoadModule(modulePath)) != nil)
            {
              aFrame = [bfi functionForAddress: (void*)(address - base)];
              if (aFrame == nil)
                {
                  /* We know we have the right module but function lookup
                   * failed ... perhaps we need to use the absolute
                   * address rather than offest by 'base' in this case.
                   */
                  aFrame = [bfi functionForAddress: address];
                }
            }
          else
            {
              NSArray	*modules;
              int	j;
              int	m;

              modules = GSListModules();
              m = [modules count];
              for (j = 0; j < m; j++)
                {
                  bfi = [modules objectAtIndex: j];

                  if ((id)bfi != (id)[NSNull null])
                    {
                      aFrame = [bfi functionForAddress: address];
                      if (aFrame != nil)
                        {
                          break;
                        }
                    }
                }
            }

          // not found (?!), add an 'unknown' function
          if (aFrame == nil)
            {
              aFrame = [GSFunctionInfo alloc];
              [aFrame initWithModule: nil
                             address: address 
                                file: nil
                            function: nil
                                line: 0];
              [aFrame autorelease];
            }
          [a addObject: [aFrame description]];
        }
      symbols = [a copy];
      [a release];
#elif	defined(_WIN32)
      void              **ptrs = (void**)&returns[FrameOffset];
      SYMBOL_INFO	*symbol;
      NSString	        *syms[MAXFRAMES];

      symbol = (SYMBOL_INFO *)calloc(sizeof(SYMBOL_INFO)
        + 1024 * sizeof(char), 1);
      symbol->MaxNameLen = 1024;
      symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

      (void)pthread_mutex_lock(&traceLock);
      for (i = 0; i < count; i++)
        {
          NSUInteger	addr = (NSUInteger)*ptrs++; 

          if ((fromSym)(hProcess, (DWORD64)addr, 0, symbol))
            {
              syms[i] = [NSString stringWithFormat:
                @"%s - %p", symbol->Name, addr];
            }
          else
            {
              syms[i] = [NSString stringWithFormat:
                @"unknown - %p", symbol->Name, addr];
            }
        }
      (void)pthread_mutex_unlock(&traceLock);
      free(symbol);

      symbols = [[NSArray alloc] initWithObjects: syms count: count];
#elif	defined(HAVE_BACKTRACE)
      void              **ptrs = (void**)&returns[FrameOffset];
      char		**strs;
      NSString	        **symbolArray;

      strs = backtrace_symbols(ptrs, count);
      symbolArray = alloca(count * sizeof(NSString*));
      for (i = 0; i < count; i++)
        {
          symbolArray[i] = [NSString stringWithUTF8String: strs[i]];
        }
      symbols = [[NSArray alloc] initWithObjects: symbolArray count: count];
      free(strs);
#elif defined(WITH_UNWIND)
      void              **ptrs = (void**)&returns[FrameOffset];
      NSString	        **symbolArray;

      symbolArray = alloca(count * sizeof(NSString*));
      for (i = 0; i < count; i++)
        {
          const void *addr = ptrs[i];
          Dl_info info;
          if (dladdr(addr, &info)) {
            const char *libname = "unknown";
            if (info.dli_fname) {
              // strip library path
              char *delim = strrchr(info.dli_fname, '/');
              libname = delim ? delim + 1 : info.dli_fname;
            }
            if (info.dli_sname) {
              symbolArray[i] = [NSString stringWithFormat:
                @"%lu: %p %s %s + %d", (unsigned long)i, addr, libname,
                info.dli_sname, (int)(addr - info.dli_saddr)];
            } else {
              symbolArray[i] = [NSString stringWithFormat:
                @"%lu: %p %s unknown", (unsigned long)i, addr, libname];
            }
          } else {
            symbolArray[i] = [NSString stringWithFormat:
              @"%lu: %p unknown", (unsigned long)i, addr];
          }
        }
      symbols = [[NSArray alloc] initWithObjects: symbolArray count: count];
#else
      NSMutableArray	*a;

      symbols = a = [[self addresses] mutableCopy];
      for (i = 0; i < count; i++)
        {
          NSString      *s;

          s = [[NSString alloc] initWithFormat: @"%p: symbol not available",
            [[a objectAtIndex: i] pointerValue]];
          [a replaceObjectAtIndex: i withObject: s];
          RELEASE(s);
        }
#endif
    }
  return symbols;
}

- (void) trace
{
  DESTROY(addresses);
  DESTROY(symbols);
  if (returns != NULL)
    {
      free(returns);
      returns = NULL;
    }
  numReturns = GSPrivateReturnAddresses(&returns);
}

@end


GS_DECLARE NSString* const NSCharacterConversionException
  = @"NSCharacterConversionException";

GS_DECLARE NSString* const NSGenericException
  = @"NSGenericException";

GS_DECLARE NSString* const NSInternalInconsistencyException
  = @"NSInternalInconsistencyException";

GS_DECLARE NSString* const NSInvalidArgumentException
  = @"NSInvalidArgumentException";

GS_DECLARE NSString* const NSMallocException
  = @"NSMallocException";

GS_DECLARE NSString* const NSOldStyleException
  = @"NSOldStyleException";

GS_DECLARE NSString* const NSParseErrorException
  = @"NSParseErrorException";

GS_DECLARE NSString* const NSRangeException
 = @"NSRangeException";

static void _terminate()
{
  BOOL			shouldAbort;

#ifdef	DEBUG
  shouldAbort = YES;		// abort() by default.
#else
  shouldAbort = NO;		// exit() by default.
#endif
  shouldAbort = GSPrivateEnvironmentFlag("CRASH_ON_ABORT", shouldAbort);
  if (shouldAbort == YES)
    {
      abort();
    }
  else
    {
      exit(1);
    }
}

static void
_NSFoundationUncaughtExceptionHandler (NSException *exception)
{
  NSAutoreleasePool	*pool = [NSAutoreleasePool new];

  fprintf(stderr, "%s: Uncaught exception %s, reason: %s\n",
    GSPrivateArgZero(),
    [[exception name] lossyCString], [[exception reason] lossyCString]);
  fflush(stderr);	/* NEEDED UNDER MINGW */
  if (GSPrivateEnvironmentFlag("GNUSTEP_STACK_TRACE", NO) == YES
    || GSPrivateDefaultsFlag(GSExceptionStackTrace) == YES)
    {
      fprintf(stderr, "Stack\n%s\n",
	[[[exception _callStack] description] lossyCString]);
    }
  fflush(stderr);	/* NEEDED UNDER MINGW */
  [pool drain];
  _terminate();
}

static void
callUncaughtHandler(id value)
{
  if (_NSUncaughtExceptionHandler != NULL)
    {
      (*_NSUncaughtExceptionHandler)(value);
    }
  /* The uncaught exception handler which is set has not exited,
   * so we MUST call the builtin handler, (normal behavior of MacOS-X).
   * The standard handler is guaranteed to exit/abort, which is the
   * required behavior for OSX compatibility.
   * NB Cocoa's Exception Handling framework might bypass this behavior
   * somehow (it's not clear if it does that or simply wraps various
   * things with its own exception handlers thus preventing the
   * uncaught handler from ever being needed) ... if anyone contributes
   * an implementation, perhaps we could integrate it here.
   */
  _NSFoundationUncaughtExceptionHandler(value);
}

@implementation NSException

+ (void) initialize
{
  if (self == [NSException class])
    {
#if defined(_NATIVE_OBJC_EXCEPTIONS)
#  ifdef HAVE_SET_UNCAUGHT_EXCEPTION_HANDLER
      objc_setUncaughtExceptionHandler(callUncaughtHandler);
#  elif defined(HAVE_UNEXPECTED)
      _objc_unexpected_exception = callUncaughtHandler;
#  elif defined(HAVE_SET_UNEXPECTED)
      objc_set_unexpected(callUncaughtHandler);
#  endif
#endif
    }
}

+ (NSException*) exceptionWithName: (NSString*)name
			    reason: (NSString*)reason
			  userInfo: (NSDictionary*)userInfo
{
  return AUTORELEASE([[self alloc] initWithName: name reason: reason
				   userInfo: userInfo]);
}

+ (void) raise: (NSString*)name
	format: (NSString*)format,...
{
  va_list args;

  va_start(args, format);
  [self raise: name format: format arguments: args];
  // This probably doesn't matter, but va_end won't get called
  va_end(args);
  while (1);    // does not return
}

+ (void) raise: (NSString*)name
	format: (NSString*)format
     arguments: (va_list)argList
{
  NSString	*reason;
  NSException	*except;

  reason = [NSString stringWithFormat: format arguments: argList];
  except = [self exceptionWithName: name reason: reason userInfo: nil];
  [except raise];
  while (1);    // does not return
}

/* For OSX compatibility -init returns nil.
 */
- (id) init
{
  DESTROY(self);
  return nil;
}

- (id) initWithName: (NSString*)name
	     reason: (NSString*)reason
	   userInfo: (NSDictionary*)userInfo
{
  ASSIGN(_e_name, name);
  ASSIGN(_e_reason, reason);
  if (userInfo != nil)
    {
      if (_reserved == 0)
        {
          _reserved = NSZoneCalloc([self zone], 2, sizeof(id));
        }
      ASSIGN(_e_info, userInfo);
    }
  return self;
}

- (NSArray*) callStackReturnAddresses
{
  if (_reserved == 0)
    {
      return nil;
    }
  return [_e_stack addresses];
}

- (NSArray *) callStackSymbols
{
  if (_reserved == 0)
    {
      return nil;
    }
  return [_e_stack symbols];
}

- (void) dealloc
{
  DESTROY(_e_name);
  DESTROY(_e_reason);
  if (_reserved != 0)
    {
      DESTROY(_e_info);
      DESTROY(_e_stack);
      NSZoneFree([self zone], _reserved);
      _reserved = 0;
    }
  [super dealloc];
}

- (NSString*) description
{
  NSAutoreleasePool	*pool = [NSAutoreleasePool new];
  NSString      	*result;

  if (_e_name == nil)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"Atttempt to use uninitialised NSException"];
    }
  if (_reserved != 0)
    {
      if (_e_stack != nil
        && (GSPrivateEnvironmentFlag("GNUSTEP_STACK_TRACE", NO) == YES
          || GSPrivateDefaultsFlag(GSExceptionStackTrace) == YES))
        {
          if (_e_info != nil)
            {
              result = [NSString stringWithFormat:
                @"%@ NAME:%@ REASON:%@ INFO:%@ STACK:%@",
                [super description], _e_name, _e_reason, _e_info, _e_stack];
            }
          else
            {
              result = [NSString stringWithFormat:
                @"%@ NAME:%@ REASON:%@ STACK:%@",
                [super description], _e_name, _e_reason, _e_stack];
            }
        }
      else
        {
          result = [NSString stringWithFormat:
            @"%@ NAME:%@ REASON:%@ INFO:%@",
            [super description], _e_name, _e_reason, _e_info];
        }
    }
  else
    {
      result = [NSString stringWithFormat: @"%@ NAME:%@ REASON:%@",
        [super description], _e_name, _e_reason];
    }
  [result retain];
  [pool drain];
  return [result autorelease];
}

- (void) raise
{
  if (_reserved == 0)
    {
      _reserved = NSZoneCalloc([self zone], 2, sizeof(id));
    }
  if (nil == _e_stack)
    {
      // Only set the stack when first raised
      _e_stack = [GSStackTrace new];
      [_e_stack trace];
    }

#if     defined(_NATIVE_OBJC_EXCEPTIONS)
  @throw self;
#else
{
  NSThread      *thread;
  NSHandler	*handler;

  thread = GSCurrentThread();
  handler = thread->_exception_handler;
  if (NULL == handler)
    {
      static	int	recursion = 0;

      /*
       * Set/check a counter to prevent recursive uncaught exceptions.
       * Allow a little recursion in case we have different handlers
       * being tried.
       */
      if (recursion++ > 3)
	{
	  fprintf(stderr,
	    "recursion encountered handling uncaught exception\n");
	  fflush(stderr);	/* NEEDED UNDER MINGW */
	  _terminate();
	}

      /*
       * Call the uncaught exception handler (if there is one).
       * The calls the built-in default handler to terminate the program!
       */
      callUncaughtHandler(self);
    }
  else
    {
      thread->_exception_handler = handler->next;
      handler->exception = self;
      longjmp(handler->jumpState, 1);
    }
}
#endif
  while (1);    // does not return
}

- (NSString*) name
{
  if (_e_name != nil)
    {
      return _e_name;
    }
  else
    {
      return NSStringFromClass([self class]);
    }
}

- (NSString*) reason
{
  if (_e_reason != nil)
    {
      return _e_reason;
    }
  else
    {
      return @"unspecified reason";
    }
}

- (NSDictionary*) userInfo
{
  if (_reserved == 0)
    {
      return nil;
    }
  return _e_info;
}

- (Class) classForPortCoder
{
  return [self class];
}

- (id) replacementObjectForPortCoder: (NSPortCoder*)aCoder
{
  return self;
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  id    info = (_reserved == 0) ? nil : _e_info;

  [aCoder encodeValueOfObjCType: @encode(id) at: &_e_name];
  [aCoder encodeValueOfObjCType: @encode(id) at: &_e_reason];
  [aCoder encodeValueOfObjCType: @encode(id) at: &info];
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  id    info;

  [aDecoder decodeValueOfObjCType: @encode(id) at: &_e_name];
  [aDecoder decodeValueOfObjCType: @encode(id) at: &_e_reason];
  [aDecoder decodeValueOfObjCType: @encode(id) at: &info];
  if (info != nil)
    {
      if (_reserved == 0)
        {
          _reserved = NSZoneCalloc([self zone], 2, sizeof(id));
        }
      _e_info = info;
    }
  return self;
}

- (id) copyWithZone: (NSZone*)zone
{
  if (NSShouldRetainWithZone(self, zone))
    {
      return RETAIN(self);
    }
  else
    {
      return [[[self class] alloc] initWithName: [self name]
                                         reason: [self reason]
                                       userInfo: [self userInfo]];
    }
}

@end

@implementation	NSException (GSPrivate)

- (GSStackTrace*) _callStack
{
  if (_reserved == 0)
    {
      return nil;
    }
  return _e_stack;
}

@end

@implementation NSThread (CallStackSymbols)

+ (NSArray *) callStackSymbols
{
  GSStackTrace *stackTrace = AUTORELEASE([GSStackTrace new]);
  [stackTrace trace];
  return [stackTrace symbols];
}

@end

void
_NSAddHandler (NSHandler* handler)
{
  NSThread *thread;

  thread = GSCurrentThread();
#if defined(_WIN32) && defined(DEBUG)
  if (thread->_exception_handler
    && IsBadReadPtr(thread->_exception_handler, sizeof(NSHandler)))
    {
      fprintf(stderr, "ERROR: Current exception handler is bogus.\n");
    }
#endif  
  handler->next = thread->_exception_handler;
  thread->_exception_handler = handler;
}

void
_NSRemoveHandler (NSHandler* handler)
{
  NSThread	*thread;

  thread = GSCurrentThread();
#if defined(DEBUG)  
  if (thread->_exception_handler != handler)
    {
      fprintf(stderr, "ERROR: Removing exception handler that is not on top "
	"of the stack. (You probably called return in an NS_DURING block.)\n");
    }
#if defined(_WIN32)
  if (IsBadReadPtr(handler, sizeof(NSHandler)))
    {
      fprintf(stderr, "ERROR: Could not remove exception handler, "
	"handler is bad pointer.\n");
      thread->_exception_handler = 0;
      return;
    }
  if (handler->next && IsBadReadPtr(handler->next, sizeof(NSHandler)))
    {
      fprintf(stderr, "ERROR: Could not restore exception handler, "
	"handler->next is bad pointer.\n");
      thread->_exception_handler = 0;
      return;
    }
#endif
#endif
  thread->_exception_handler = handler->next;
}

NSUncaughtExceptionHandler *
NSGetUncaughtExceptionHandler()
{
  return _NSUncaughtExceptionHandler;
}

void
NSSetUncaughtExceptionHandler(NSUncaughtExceptionHandler *handler)
{
  _NSUncaughtExceptionHandler = handler;
}
