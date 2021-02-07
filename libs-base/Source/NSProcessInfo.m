/** Implementation for NSProcessInfo for GNUStep
   Copyright (C) 1995-2017 Free Software Foundation, Inc.

   Written by:  Georg Tuparev <Tuparev@EMBL-Heidelberg.de>
                Heidelberg, Germany
   Modified by:  Richard Frith-Macdonald <rfm@gnu.org>

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

   <title>NSProcessInfo class reference</title>
   $Date$ $Revision$
*/

/*************************************************************************
 * File Name  : NSProcessInfo.m
 * Date       : 06-aug-1995
 *************************************************************************
 * Notes      :
 * 1) The class functionality depends on the following UNIX functions and
 * global variables: gethostname(), getpid(), and environ. For all system
 * I had the opportunity to test them they are defined and have the same
 * behavior. The same is true for the meaning of argv[0] (process name).
 * 2) The global variable _gnu_sharedProcessInfoObject should NEVER be
 * deallocate during the process runtime. Therefore I implemented a
 * concrete NSProcessInfo subclass (_NSConcreteProcessInfo) with the only
 * purpose to override the autorelease, retain, and release methods.
 * To Do      :
 * 1) To test the class on more platforms;
 * Bugs       : Not known
 * Last update: 07-aug-2002
 * History    : 06-aug-1995    - Birth and the first beta version (v. 0.5);
 *              08-aug-1995    - V. 0.6 (tested on NS, SunOS, Solaris, OSF/1
 *              The use of the environ global var was changed to more
 *              conventional env[] (main function) so now the class could be
 *              used on SunOS and Solaris. [GT]
 *************************************************************************
 * Acknowledgments:
 * - Adam Fedor, Andrew McCallum, and Paul Kunz for their help;
 * - To the NEXTSTEP/GNUStep community
 *************************************************************************/

#import "common.h"

#include <stdio.h>

#ifdef HAVE_WINDOWS_H
#  include <windows.h>
#endif

#if	defined(HAVE_SYS_SIGNAL_H)
#  include	<sys/signal.h>
#elif	defined(HAVE_SIGNAL_H)
#  include	<signal.h>
#endif

#if	defined(HAVE_SYS_FILE_H)
#  include <sys/file.h>
#endif

#if	defined(HAVE_SYS_FCNTL_H)
#  include <sys/fcntl.h>
#elif	defined(HAVE_FCNTL_H)
#  include <fcntl.h>
#endif

#ifdef HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif

#ifdef HAVE_KVM_ENV
#include <kvm.h>

#if	defined(HAVE_SYS_FCNTL_H)
#  include	<sys/fcntl.h>
#elif	defined(HAVE_FCNTL_H)
#  include	<fcntl.h>
#endif

#include <sys/param.h>
#endif /* HAVE_KVM_ENV */

#ifdef HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif

#if HAVE_PROCFS_H
#define id _procfs_avoid_id_collision
#include <procfs.h>
#undef id
#endif

#if defined(__APPLE__) && !GS_FAKE_MAIN
#include <crt_externs.h>
#endif

#import "Foundation/NSArray.h"
#import "Foundation/NSSet.h"
#import "Foundation/NSCharacterSet.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSDate.h"
#import "Foundation/NSException.h"
#import "Foundation/NSFileManager.h"
#import "Foundation/NSProcessInfo.h"
#import "Foundation/NSAutoreleasePool.h"
#import "Foundation/NSHost.h"
#import "Foundation/NSLock.h"
#import "GNUstepBase/NSProcessInfo+GNUstepBase.h"
#import "GNUstepBase/NSString+GNUstepBase.h"

#import "GSPrivate.h"

/* This error message should be called only if the private main function
 * was not executed successfully. This may happen ONLY if another library
 * or kit defines its own main function (as gnustep-base does).
 */
#if GS_FAKE_MAIN
#define _GNU_MISSING_MAIN_FUNCTION_CALL "\nGNUSTEP Internal Error:\n\
The private GNUstep function to establish the argv and environment\n\
variables was not called.\n\
Perhaps your program failed to #include <Foundation/NSObject.h> or\n\
<Foundation/Foundation.h>?\n\
If that is not the problem, Please report the error to bug-gnustep@gnu.org.\n\n"
#else
#ifdef GS_PASS_ARGUMENTS
#define _GNU_MISSING_MAIN_FUNCTION_CALL "\nGNUSTEP Error:\n\
A call to NSProcessInfo +initializeWithArguments:... must be made\n\
as the first ObjC statment in main. This function is used to \n\
establish the argv and environment variables.\n"
#else
#define _GNU_MISSING_MAIN_FUNCTION_CALL "\nGNUSTEP Internal Error:\n\
The private GNUstep function to establish the argv and environment\n\
variables was not called.\n\
\n\
Mismatched library versions between GNUstep Foundation (base) and AppKit\n\
(gui) is most often the cause of this message. Please be sure you\n\
are using known compatible versions and not a mismatched set. Generally,\n\
we recommend you use versions of base and gui which were released together.\n\
\n\
For more detailed assistance, please report the error to bug-gnustep@gnu.org.\n\n"
#endif
#endif

@interface      NSHost (NSProcessInfo)
+ (NSString*) _myHostName;
@end

/*************************************************************************
 *** _NSConcreteProcessInfo
 *************************************************************************/
@interface _NSConcreteProcessInfo: NSProcessInfo
- (id) autorelease;
- (void) release;
- (id) retain;
@end

@implementation _NSConcreteProcessInfo
- (id) autorelease
{
  return self;
}

- (void) release
{
  return;
}

- (id) retain
{
  return self;
}
@end


/*************************************************************************
 *** NSProcessInfo implementation
 *************************************************************************/

/**
 * Instances of this class encapsulate information on the current process.
 * For example, you can get the arguments, environment variables, host name,
 * or process name.  There is only one instance per process, for obvious
 * reasons, and it may be obtained through the +processInfo method.
 */
@implementation NSProcessInfo
/*************************************************************************
 *** Static global vars
 *************************************************************************/

// The lock to protect shared process resources.
static NSRecursiveLock  *procLock = nil;

// The shared NSProcessInfo instance
static NSProcessInfo	*_gnu_sharedProcessInfoObject = nil;

// Host name of the CPU executing the process
static NSString		*_gnu_hostName = nil;

static char		*_gnu_arg_zero = 0;

// Current process name
static NSString		*_gnu_processName = nil;

// Array of NSStrings (argv[1] .. argv[argc-1])
static NSArray		*_gnu_arguments = nil;

// Dictionary of environment vars and their values
static NSDictionary	*_gnu_environment = nil;

// The operating system we are using.
static unsigned int	_operatingSystem = 0;
static NSString		*_operatingSystemName = nil;
static NSString		*_operatingSystemVersion = nil;

// Flag to indicate that fallbackInitialisation was executed.
static BOOL	fallbackInitialisation = NO;

static NSMutableSet	*mySet = nil;

#ifdef __ANDROID__
static jobject _androidContext = NULL;
static NSString *_androidFilesDir = nil;
static NSString *_androidCacheDir = nil;
#endif

/*************************************************************************
 *** Implementing the gnustep_base_user_main function
 *************************************************************************/

static void
_gnu_process_args(int argc, char *argv[], char *env[])
{
  NSAutoreleasePool *arp = [NSAutoreleasePool new];
  NSString	*arg0 = nil;
  int i;

  if (_gnu_arg_zero != 0)
    {
      free(_gnu_arg_zero);
    }

  if (argv != 0 && argv[0] != 0)
    {
      int	len;

      len = strlen(argv[0]) + 1;
      _gnu_arg_zero = (char*)malloc(len);
      memcpy(_gnu_arg_zero, argv[0], len);
      arg0 = [[NSString alloc] initWithCString: _gnu_arg_zero];
    }
  else
    {
#if	defined(_WIN32)
      unichar	*buffer;
      int	buffer_size = 0;
      int	needed_size = 0;
      int	len;
      const char	*tmp;

      while (needed_size == buffer_size)
	{
          buffer_size = buffer_size + 256;
          buffer = (unichar*)malloc(buffer_size * sizeof(unichar));
          needed_size = GetModuleFileNameW(NULL, buffer, buffer_size);
          if (needed_size < buffer_size)
	    {
	      unsigned	i;

	      for (i = 0; i < needed_size; i++)
		{
		  if (buffer[i] == 0)
		    {
		      break;
		    }
		}
	      arg0 = [[NSString alloc] initWithCharacters: buffer length: i];
	    }
          else
	    {
              free(buffer);
	    }
	}
      tmp = [arg0 cStringUsingEncoding: [NSString defaultCStringEncoding]];
      len = strlen(tmp) + 1;
      _gnu_arg_zero = (char*)malloc(len);
      memcpy(_gnu_arg_zero, tmp, len);
#else
      fprintf(stderr, "Error: for some reason, argv not properly set up "
	      "during GNUstep base initialization\n");
      abort();
#endif
    }

  /* Getting the process name */
  IF_NO_GC(RELEASE(_gnu_processName));
  _gnu_processName = [arg0 lastPathComponent];
#if	defined(_WIN32)
  /* On windows we remove any .exe extension for consistency with app names
   * under unix
   */
  {
    NSString	*e = [_gnu_processName pathExtension];

    if (e != nil && [e caseInsensitiveCompare: @"EXE"] == NSOrderedSame)
      {
	_gnu_processName = [_gnu_processName stringByDeletingPathExtension];
      }
  }
#endif
  IF_NO_GC(RETAIN(_gnu_processName));

  /* Copy the argument list */
#if	defined(_WIN32)
{
  unichar **argvw = CommandLineToArgvW(GetCommandLineW(), &argc);
  NSString *str;
  id obj_argv[argc];
  int added = 1;
  
  /* Copy the zero'th argument to the argument list */
  obj_argv[0] = arg0;
  
  if (mySet == nil) mySet = [NSMutableSet new];

  for (i = 1; i < argc; i++)
    {
      str = [NSString stringWithCharacters: argvw[i] length: wcslen(argvw[i])];
      if ([str hasPrefix: @"--GNU-Debug="])
	{
	  [mySet addObject: [str substringFromIndex: 12]];
	}
      else
	{
	  obj_argv[added++] = str;
	}
    }
    
  IF_NO_GC(RELEASE(_gnu_arguments));
  _gnu_arguments = [[NSArray alloc] initWithObjects: obj_argv count: added];
  RELEASE(arg0);
}
#else
  if (argv)
    {
      NSString		*str;
      id		obj_argv[argc];
      int		added = 1;
      NSStringEncoding	enc = GSPrivateDefaultCStringEncoding();

      /* Copy the zero'th argument to the argument list */
      obj_argv[0] = arg0;

      if (mySet == nil) mySet = [NSMutableSet new];

      for (i = 1; i < argc; i++)
	{
	  str = [NSString stringWithCString: argv[i] encoding: enc];

	  if ([str hasPrefix: @"--GNU-Debug="])
	    [mySet addObject: [str substringFromIndex: 12]];
	  else
	    obj_argv[added++] = str;
	}

      IF_NO_GC(RELEASE(_gnu_arguments));
      _gnu_arguments = [[NSArray alloc] initWithObjects: obj_argv count: added];
      RELEASE(arg0);
    }
#endif	
	
  /* Copy the evironment list */
  {
    NSMutableArray	*keys = [NSMutableArray new];
    NSMutableArray	*values = [NSMutableArray new];
    NSStringEncoding	enc = GSPrivateDefaultCStringEncoding();

#if defined(_WIN32)
    if (fallbackInitialisation == NO)
      {
	unichar	*base;

	base = GetEnvironmentStringsW();
	if (base != 0)
	  {
	    const unichar	*wenvp = base;

	    while (*wenvp != 0)
	      {
		const unichar	*start = wenvp;
		NSString		*key;
		NSString		*val;

		start = wenvp;
		while (*wenvp != '=' && *wenvp != 0)
		  {
		    wenvp++;
		  }
		if (*wenvp == '=')
		  {
		    key = [NSString stringWithCharacters: start
						  length: wenvp - start];
		    wenvp++;
		    start = wenvp;
		  }
		else
		  {
		    break;	// Bad format ... expected '='
		  }
		while (*wenvp != 0)
		  {
		    wenvp++;
		  }
		val = [NSString stringWithCharacters: start
					      length: wenvp - start];
		wenvp++;	// Skip past variable terminator
		[keys addObject: key];
		[values addObject: val];
	      }
	    FreeEnvironmentStringsW(base);
	    env = 0;	// Suppress standard code.
	  }
      }
#endif
    if (env != 0)
      {
	i = 0;
	while (env[i])
	  {
	    int		len = strlen(env[i]);
	    char	*cp = strchr(env[i], '=');

	    if (len && cp)
	      {
		char	buf[len+2];

		memcpy(buf, env[i], len + 1);
		cp = &buf[cp - env[i]];
		*cp++ = '\0';
		[keys addObject:
		  [NSString stringWithCString: buf encoding: enc]];
		[values addObject:
		  [NSString stringWithCString: cp encoding: enc]];
	      }
	    i++;
	  }
      }
    IF_NO_GC(RELEASE(_gnu_environment));
    _gnu_environment = [[NSDictionary alloc] initWithObjects: values
						     forKeys: keys];
    IF_NO_GC(RELEASE(keys));
    IF_NO_GC(RELEASE(values));
  }
  [arp drain];
}

#if !GS_FAKE_MAIN && ((defined(HAVE_PROCFS)  || defined(HAVE_KVM_ENV) || defined(HAVE_PROCFS_PSINFO) || defined(__APPLE__)) && (defined(HAVE_LOAD_METHOD)))
/*
 * We have to save program arguments and environment before main () is
 * executed, because main () could modify their values before we get a
 * chance to read them
 */
static int	_gnu_noobjc_argc = 0;
static char	**_gnu_noobjc_argv = NULL;
static char	**_gnu_noobjc_env = NULL;

/*
 * The +load method (an extension of the GNU compiler) is invoked
 * before main and +initialize (for this class) is executed.  This is
 * guaranteed if +load contains only pure C code, as we have here. The
 * code in here either uses libkvm if available, or else procfs.
 */
+ (void) load
{
#ifdef HAVE_KVM_ENV
  /*
   * Use the kvm library to open the kernel and read the environment and
   * arguments. As we are not running as root we cannot open the memory
   * device and thus we fake it using /dev/null. This is allowed under
   * FreeBSD, but may fail on other operating systems which check the
   * file type. The kvm calls used are those which are supposedly backward
   * compatible with Solaris rather than being FreeBSD specific
   */
  kvm_t *kptr = NULL;
  struct kinfo_proc *proc_ptr = NULL;
  int nprocs, i, count;
  char **vectors;

  /* open the kernel */
  kptr = kvm_open(NULL, "/dev/null", NULL, O_RDONLY, "NSProcessInfo");
  if (!kptr)
    {
      fprintf(stderr, "Error: Your system appears to provide libkvm, but the kernel open fails\n");
      fprintf(stderr, "Try to reconfigure gnustep-base with --enable-fake-main. to work\n");
      fprintf(stderr, "around this problem.");
      abort();
    }

  /* find the process */
  proc_ptr = kvm_getprocs(kptr, KERN_PROC_PID, getpid(), &nprocs);
  if (!proc_ptr || (nprocs != 1))
    {
      fprintf(stderr, "Error: libkvm cannot find the current process\n");
      abort();
    }

  /* get the environment vectors the normal way, since this always works.
     On FreeBSD, the only other way is via /proc, and in later versions
     /proc is not mounted.  */
  {
    extern char **environ;
    vectors = environ;
    if (!vectors)
      {
	fprintf(stderr, "Error: for some reason, environ == NULL "
		"during GNUstep base initialization\n"
		"Please check the linking process\n");
	abort();
      }
  }

  /* copy the environment strings */
  for (count = 0; vectors[count]; count++)
    ;
  _gnu_noobjc_env = (char**)malloc(sizeof(char*) * (count + 1));
  if (!_gnu_noobjc_env)
    goto malloc_error;
  for (i = 0; i < count; i++)
    {
      _gnu_noobjc_env[i] = (char *)strdup(vectors[i]);
      if (!_gnu_noobjc_env[i])
	goto malloc_error;
    }
  _gnu_noobjc_env[i] = NULL;

  /* get the argument vectors */
  vectors = kvm_getargv(kptr, proc_ptr, 0);
  if (!vectors)
    {
      fprintf(stderr, "Error: libkvm does not return arguments for the current process\n");
      fprintf(stderr, "this may be due to a bug (undocumented feature) in libkvm\n");
      fprintf(stderr, "which fails to get arguments unless /proc is mounted.\n");
      fprintf(stderr, "If so, you can mount the /proc filesystem or reconfigure/build\n");
      fprintf(stderr, "gnustep-base with --enable-fake-main as a workaround, and\n");
      fprintf(stderr, "should report the bug to the maintainer of libkvm on your operating system.\n");
      abort();
    }

  /* copy the argument strings */
  for (_gnu_noobjc_argc = 0; vectors[_gnu_noobjc_argc]; _gnu_noobjc_argc++)
    ;
  _gnu_noobjc_argv
    = (char**)malloc(sizeof(char*) * (_gnu_noobjc_argc + 1));
  if (!_gnu_noobjc_argv)
    goto malloc_error;
  for (i = 0; i < _gnu_noobjc_argc; i++)
    {
      _gnu_noobjc_argv[i] = (char *)strdup(vectors[i]);
      if (!_gnu_noobjc_argv[i])
	goto malloc_error;
    }
  _gnu_noobjc_argv[i] = NULL;

  return;
#elif defined(HAVE_PROCFS_PSINFO)
  char *proc_file_name = NULL;
  FILE *ifp;
  psinfo_t pinfo;
  char **vectors;
  int i, count;
  
  // Read commandline
  proc_file_name = (char*)malloc(2048);
  snprintf(proc_file_name, 2048, "/proc/%d/psinfo", (int)getpid());
  
  ifp = fopen(proc_file_name, "r");
  if (ifp == NULL)
    {
      fprintf(stderr, "Error: Failed to open the process info file:%s\n", 
              proc_file_name);
      abort();
    }
  
  fread(&pinfo, sizeof(pinfo), 1, ifp);
  fclose(ifp);
  
  vectors = (char **)pinfo.pr_envp;
  if (!vectors)
    {
      fprintf(stderr, "Error: for some reason, environ == NULL "
        "during GNUstep base initialization\n"
        "Please check the linking process\n");
      abort();
    }
  
  /* copy the environment strings */
  for (count = 0; vectors[count]; count++)
    ;
  _gnu_noobjc_env = (char**)malloc(sizeof(char*) * (count + 1));
  if (!_gnu_noobjc_env)
    goto malloc_error;
  for (i = 0; i < count; i++)
    {
      _gnu_noobjc_env[i] = (char *)strdup(vectors[i]);
      if (!_gnu_noobjc_env[i])
        goto malloc_error;
    }
  _gnu_noobjc_env[i] = NULL;

  /* get the argument vectors */
  vectors = (char **)pinfo.pr_argv;
  if (!vectors)
  {
    fprintf(stderr, "Error: psinfo does not return arguments for the current process\n");
    abort();
  }
  /* copy the argument strings */
  for (_gnu_noobjc_argc = 0; vectors[_gnu_noobjc_argc]; _gnu_noobjc_argc++)
    ;
  _gnu_noobjc_argv
    = (char**)malloc(sizeof(char*) * (_gnu_noobjc_argc + 1));
  if (!_gnu_noobjc_argv)
    goto malloc_error;
  for (i = 0; i < _gnu_noobjc_argc; i++)
    {
      _gnu_noobjc_argv[i] = (char *)strdup(vectors[i]);
      if (!_gnu_noobjc_argv[i])
	goto malloc_error;
    }
  _gnu_noobjc_argv[i] = NULL;

  return;
#elif defined(__APPLE__)
  /*
   * Darwin/Mac OS X provides indirect access to command line arguments and
   * the environment with functions defined in the C runtime system.
   */
  int i, n;
  int argc = *_NSGetArgc();
  char **argv = *_NSGetArgv();
  char **environ = *_NSGetEnviron();

  /* copy environment */
  n = 0;
  while (environ[n] != NULL)
    n++;
  _gnu_noobjc_env = (char **)malloc(sizeof(char *) * (n + 1));
  if (_gnu_noobjc_env == NULL)
    goto malloc_error;
  for (i = 0; i < n; i++)
    {
      _gnu_noobjc_env[i] = (char *)strdup(environ[i]);
      if (_gnu_noobjc_env[i] == NULL)
	goto malloc_error;
    }
  _gnu_noobjc_env[i] = NULL;

  /* copy arguments */
  _gnu_noobjc_argc = argc;
  _gnu_noobjc_argv = (char **)malloc(sizeof(char *) * (argc + 1));
  if (_gnu_noobjc_argv == NULL)
    goto malloc_error;
  for (i = 0; i < argc; i++)
    {
      _gnu_noobjc_argv[i] = (char *)strdup(argv[i]);
      if (_gnu_noobjc_argv[i] == NULL)
	goto malloc_error;
    }
  _gnu_noobjc_argv[i] = NULL;

  return;
#else /* !HAVE_KVM_ENV (i.e. HAVE_PROCFS).  */
  /*
   * Now we have the problem of reading program arguments and
   * environment.  We take the environment from extern char **environ, and
   * the program arguments from the /proc filesystem.
   */
  extern char	**environ;
  char		*proc_file_name = NULL;
  FILE		*ifp;
  int		c;
  int		argument;
  int		length;
  int		position;
  int		env_terms;
  BOOL		stripTrailingNewline = NO;
#ifdef HAVE_PROGRAM_INVOCATION_NAME
  extern char	*program_invocation_name;
#endif /* HAVE_PROGRAM_INVOCATION_NAME */

  // Read environment

  /* NB: This should *never* happen if your compiler tools are
     sane.  But, if you are playing with them, you could break
     them to the point you get here. :-) */
  if (environ == NULL)
    {
      /* TODO: Try reading environment from /proc before aborting. */
      fprintf(stderr, "Error: for some reason, environ == NULL "
	      "during GNUstep base initialization\n"
	      "Please check the linking process\n");
      abort();
    }

  c = 0;
  while (environ[c] != NULL)
    c++;
  env_terms = c;
  _gnu_noobjc_env = (char**)malloc(sizeof(char*) * (env_terms + 1));
  if (_gnu_noobjc_env == NULL)
    goto malloc_error;
  for (c = 0; c < env_terms; c++)
    {
      _gnu_noobjc_env[c] = (char *)strdup(environ[c]);
      if (_gnu_noobjc_env[c] == NULL)
	goto malloc_error;
    }
  _gnu_noobjc_env[c] = NULL;

  // Read commandline
  proc_file_name = (char *)malloc(2048);
  snprintf(proc_file_name, 2048, "/proc/%d/cmdline", (int)getpid());

  /*
   * We read the /proc file thrice.
   * First, to know how many arguments there are and allocate memory for them.
   * Second, to know how long each argument is, and allocate memory accordingly.
   * Third, to actually copy the arguments into memory.
   */
  _gnu_noobjc_argc = 0;
#ifdef HAVE_STRERROR
  errno = 0;
#endif /* HAVE_STRERROR */
  ifp = fopen(proc_file_name, "r");
  if (ifp == NULL)
    goto proc_fs_error;
  while (1)
    {
      c = getc(ifp);
      if (c == 0)
	_gnu_noobjc_argc++;
      else if (c == EOF)
	break;
    }
#if (CMDLINE_TERMINATED == 0)
  _gnu_noobjc_argc++;
#endif
  fclose(ifp);

  /*
   * Now _gnu_noobcj_argc is the number of arguments;
   * allocate memory accordingly.
   */
  _gnu_noobjc_argv = (char **)malloc((sizeof(char *)) * (_gnu_noobjc_argc + 1));
  if (_gnu_noobjc_argv == NULL)
    goto malloc_error;

  ifp = fopen(proc_file_name,"r");
  //freopen(proc_file_name, "r", ifp);
  if (ifp == NULL)
    {
      free(_gnu_noobjc_argv);
      goto proc_fs_error;
    }
  argument = 0;
  length = 0;
  while (argument < _gnu_noobjc_argc)
    {
      c = getc(ifp);
      length++;
      if ((c == EOF) || (c == 0)) // End of a parameter
	{
	  _gnu_noobjc_argv[argument]
	    = (char*)malloc((sizeof(char))*length);
	  if (_gnu_noobjc_argv[argument] == NULL)
	    goto malloc_error;
	  argument++;
	  length = 0;
	  if (c == EOF) // End of command line
	    {
	      _gnu_noobjc_argc = argument;
	      break;
	    }
	}
    }
  fclose(ifp);
  ifp = fopen(proc_file_name,"r");
  //freopen(proc_file_name, "r", ifp);
  if (ifp == NULL)
    {
      if (0 != _gnu_noobjc_argv)
	{
	  for (c = 0; c < _gnu_noobjc_argc; c++)
	    {
	      free(_gnu_noobjc_argv[c]);
	    }
	  free(_gnu_noobjc_argv);
	}
      goto proc_fs_error;
    }
  argument = 0;
  position = 0;
  while (argument < _gnu_noobjc_argc)
    {
      c = getc(ifp);
      if ((c == EOF) || (c == 0)) // End of a parameter
	{
	  if (argument == 0 && position > 0
	    && _gnu_noobjc_argv[argument][position-1] == '\n')
	    {
	      stripTrailingNewline = YES;
	    }
	  if (stripTrailingNewline == YES && position > 0
	    && _gnu_noobjc_argv[argument][position-1] == '\n')
	    {
	      position--;
	    }
	  _gnu_noobjc_argv[argument][position] = '\0';
	  argument++;
	  if (c == EOF) // End of command line
	    break;
	  position = 0;
	  continue;
	}
      _gnu_noobjc_argv[argument][position] = c;
      position++;
    }
  _gnu_noobjc_argv[argument] = NULL;
  fclose(ifp);
  free(proc_file_name);
  return;

 proc_fs_error:
#ifdef HAVE_STRERROR
  /* Don't care about thread safety of strerror() here as this is only
   * called in the initial thread and there shouldn't be any other
   * threads at this point.
   */
  fprintf(stderr, "Couldn't open file %s when starting gnustep-base; %s\n",
	   proc_file_name, strerror(errno));
#else  /* !HAVE_FUNCTION_STRERROR */
  fprintf(stderr, "Couldn't open file %s when starting gnustep-base.\n",
	   proc_file_name);
#endif /* HAVE_FUNCTION_STRERROR */
  fprintf(stderr, "Your gnustep-base library is compiled for a kernel supporting the /proc filesystem, but it can't access it.\n");
  fprintf(stderr, "You should recompile or change your kernel.\n");
  free(proc_file_name);
#ifdef HAVE_PROGRAM_INVOCATION_NAME
  fprintf(stderr, "We try to go on anyway; but the program will ignore any argument which were passed to it.\n");
  _gnu_noobjc_argc = 1;
  _gnu_noobjc_argv = malloc(sizeof(char *) * 2);
  if (_gnu_noobjc_argv == NULL)
    goto malloc_error;
  _gnu_noobjc_argv[0] = strdup(program_invocation_name);
  if (_gnu_noobjc_argv[0] == NULL)
    goto malloc_error;
  _gnu_noobjc_argv[1] = NULL;
  return;
#else /* !HAVE_PROGRAM_INVOCATION_NAME */
  /*
   * There is really little sense in going on here, because NSBundle
   * will anyway crash later if we just put something like "_Unknown_"
   * as the program name.
   */
  abort();
#endif /* HAVE_PROGRAM_INVOCATION_NAME */
#endif /* !HAVE_KVM_ENV (e.g. HAVE_PROCFS) */
 malloc_error:
  fprintf(stderr, "malloc() error when starting gnustep-base.\n");
  fprintf(stderr, "Free some memory and then re-run the program.\n");
  abort();
}

static void
_gnu_noobjc_free_vars(void)
{
  char **p;

  p = _gnu_noobjc_argv;
  while (*p)
    {
      free(*p);
      p++;
    }
  free(_gnu_noobjc_argv);
  _gnu_noobjc_argv = 0;

  p = _gnu_noobjc_env;
  while (*p)
    {
      free(*p);
      p++;
    }
  free(_gnu_noobjc_env);
  _gnu_noobjc_env = 0;
}

+ (void) initialize
{
  if (nil == procLock) procLock = [NSRecursiveLock new];
  if (self == [NSProcessInfo class]
    && !_gnu_processName && !_gnu_arguments && !_gnu_environment)
    {
      if (_gnu_noobjc_argv == 0 || _gnu_noobjc_env == 0)
	{
          fprintf(stderr, _GNU_MISSING_MAIN_FUNCTION_CALL);
          exit(1);
	}
      _gnu_process_args(_gnu_noobjc_argc, _gnu_noobjc_argv, _gnu_noobjc_env);
      _gnu_noobjc_free_vars();
    }
}
#else /*! HAVE_PROCFS !HAVE_LOAD_METHOD !HAVE_KVM_ENV */

#ifdef _WIN32
/* For WindowsAPI Library, we know the global variables (argc, etc) */
+ (void) initialize
{
  if (nil == procLock) procLock = [NSRecursiveLock new];
  if (self == [NSProcessInfo class]
    && !_gnu_processName && !_gnu_arguments && !_gnu_environment)
    {
      _gnu_process_args(__argc, __argv, _environ);
    }
}
#elif defined(__BEOS__)

extern int __libc_argc;
extern char **__libc_argv;
+ (void) initialize
{
  if (nil == procLock) procLock = [NSRecursiveLock new];
  if (self == [NSProcessInfo class]
    && !_gnu_processName && !_gnu_arguments && !_gnu_environment)
    {
      _gnu_process_args(__libc_argc, __libc_argv, environ);
    }
}

#else
+ (void) initialize
{
  if (nil == procLock) procLock = [NSRecursiveLock new];
}
#ifndef GS_PASS_ARGUMENTS
#undef main
/* The gnustep_base_user_main function is declared 'weak' so that the linker
 * should actually use the one compiled as the program's 'main' function.
 * The internal version gets called only if the program does not implement
 * the function (ie the prgram was compiled with the wrong version of
 * GSConfig.h included/imported).  The other possible reason for the internal
 * function to be called would be a compiler/linker issue (eg 'weak' not
 * supported).
 */
int gnustep_base_user_main () __attribute__((weak));
int gnustep_base_user_main (int argc, char *argv[], char *env[])
{
  fprintf(stderr, "\nGNUSTEP Internal Error:\n"
"The GNUstep function to establish the argv and environment variables could\n"
"not find the main function of your program.\n"
"Perhaps your program failed to #include <Foundation/NSObject.h> or\n"
"<Foundation/Foundation.h> (or included/imported a different version of the\n"
"header from the one supplied with this copy of the gnustep-base library)?\n"
"If that is not the case, Please report the error to bug-gnustep@gnu.org.\n");
  exit(1);
}
int main(int argc, char *argv[], char *env[])
{
#ifdef NeXT_RUNTIME
  /* This memcpy has to be done before the first message is sent to any
     constant string object. See Apple Radar 2870817 */
  memcpy(&_NSConstantStringClassReference,
         objc_getClass(STRINGIFY(NXConstantString)),
         sizeof(_NSConstantStringClassReference));
#endif

#if defined(_WIN32)
  WSADATA lpWSAData;

  // Initialize Windows Sockets
  if (WSAStartup(MAKEWORD(1,1), &lpWSAData))
    {
      printf("Could not startup Windows Sockets\n");
      exit(1);
    }
#endif /* _WIN32 */

#ifdef __MS_WIN__
  _MB_init_runtime();
#endif /* __MS_WIN__ */

  _gnu_process_args(argc, argv, env);

  /* Call the user defined main function */
  return gnustep_base_user_main(argc, argv, env);
}
#endif /* !GS_PASS_ARGUMENTS */
#endif /* _WIN32 */

#endif /* HAS_LOAD_METHOD && HAS_PROCFS */

+ (NSProcessInfo *) processInfo
{
  // Check if the main() function was successfully called
  // We can't use NSAssert, which calls NSLog, which calls NSProcessInfo...
  if (!(_gnu_processName && _gnu_arguments && _gnu_environment))
    {
      fprintf(stderr, _GNU_MISSING_MAIN_FUNCTION_CALL);
      exit(1);
    }

  if (!_gnu_sharedProcessInfoObject)
    {
      _gnu_sharedProcessInfoObject = [[_NSConcreteProcessInfo alloc] init];
      [procLock lock];
      if (mySet != nil)
	{
	  NSEnumerator	*e = [mySet objectEnumerator];
	  NSMutableSet	*s = [_gnu_sharedProcessInfoObject debugSet];
	  id		o;

	  while ((o = [e nextObject]) != nil)
	    {
              [s addObject: o];
	    }
	  [mySet release];
	  mySet = nil;
        }
      [procLock unlock];
    }

  return _gnu_sharedProcessInfoObject;
}

+ (BOOL) _exists: (int)pid
{
  if (pid > 0)
    {
#if	defined(_WIN32)
      HANDLE        h = OpenProcess(PROCESS_QUERY_INFORMATION,0,pid);
      if (h == NULL && GetLastError() != ERROR_ACCESS_DENIED)
        {
          return NO;
        }
      CloseHandle(h);
#else
      if (kill(pid, 0) < 0 && errno == ESRCH)
        {
          return NO;
        }
#endif
      return YES;
    }
  return NO;
}

- (NSArray *) arguments
{
  return _gnu_arguments;
}

- (NSDictionary *) environment
{
  return _gnu_environment;
}

- (NSString *) globallyUniqueString
{
  static unsigned long	counter = 0;
  unsigned long		count;
  static NSString	*host = nil;
  NSString              *thost = nil;
  static int		pid = 0;
  int                   tpid = 0;
  static unsigned long	start;

  /* We obtain the host name and pid outside the locked region in case
   * the lookup is slow or indirectly calls this method fromm another
   * thread (as unlikely as that is ... some subclass/category could
   * do it).
   */
  if (nil == host)
    {
      thost = [[self hostName] stringByReplacingString: @"." withString: @"_"];
      tpid = [self processIdentifier];
    }
  [procLock lock];
  if (nil == host)
    {
      start = (unsigned long)GSPrivateTimeNow();
      ASSIGN(host, thost);
      pid = tpid;
    }
  count = counter++;
  [procLock unlock];

  // $$$ The format of the string is not specified by the OpenStep
  // specification.
  return [NSString stringWithFormat: @"%@_%x_%lx_%lx",
    host, pid, start, count];
}

- (NSString *) hostName
{
  if (!_gnu_hostName)
    {
      _gnu_hostName = [[NSHost _myHostName] copy];
    }
  return _gnu_hostName;
}

static void determineOperatingSystem()
{
  if (_operatingSystem == 0)
    {
      NSString	*os = nil;
      BOOL	parseOS = YES;

#if	defined(_WIN32)
      OSVERSIONINFOW	osver;

      osver.dwOSVersionInfoSize = sizeof(osver);
      GetVersionExW (&osver);
      /* Hmm, we could use this to determine operating system version, but
       * that would not distinguish between mingw and cygwin, so we just
       * use the information from NSBundle and only get the version info
       * here.
       */
      _operatingSystemVersion = [[NSString alloc] initWithFormat: @"%d.%d",
        osver.dwMajorVersion, osver.dwMinorVersion];
#else
#if	defined(HAVE_SYS_UTSNAME_H)
      struct utsname uts;

      /* The system supports uname, so we can use it rather than the
       * value determined at configure/compile time.
       * That's good if the binary is running on a system other than
       * the one it was built for (rare, but can happen).
       */
      if (!(uname(&uts) < 0))
	{
	  os = [NSString stringWithCString: uts.sysname                                                           encoding: [NSString defaultCStringEncoding]];
	  os = [os lowercaseString];
	  /* Get the operating system version ... usually the version string
	   * is pretty horrible, and the kernel release string actually
	   * makes more sense.
	   */
	  _operatingSystemVersion = [[NSString alloc]
	    initWithCString: uts.release
	    encoding: [NSString defaultCStringEncoding]];

	  /* Hack for sunos/solaris ... sunos version 5 is solaris
	   */
	  if ([os isEqualToString: @"sunos"] == YES
	    && [_operatingSystemVersion intValue] > 4)
	    {
	      os = @"solaris";
	    }
	}
#endif	/* HAVE_SYS_UTSNAME_H */
#endif	/* _WIN32 */

      if (_operatingSystemVersion == nil)
        {
	  NSWarnFLog(@"Unable to determine system version, using 0.0");
	  _operatingSystemVersion = @"0.0";
	}

      while (parseOS == YES)
	{
	  NSString	*fallback = [NSBundle _gnustep_target_os];

	  if (os == nil)
	    {
	      os = fallback;
	    }
	  parseOS = NO;

	  if ([os hasPrefix: @"linux"] == YES)
	    {
	      _operatingSystemName = @"GSGNULinuxOperatingSystem";
	      _operatingSystem = GSGNULinuxOperatingSystem;
	    }
	  else if ([os hasPrefix: @"mingw"] == YES)
	    {
	      _operatingSystemName = @"NSWindowsNTOperatingSystem";
	      _operatingSystem = NSWindowsNTOperatingSystem;
	    }
	  else if ([os isEqualToString: @"cygwin"] == YES)
	    {
	      _operatingSystemName = @"GSCygwinOperatingSystem";
	      _operatingSystem = GSCygwinOperatingSystem;
	    }
	  else if ([os hasPrefix: @"bsd"] == YES
	    || [os hasPrefix: @"freebsd"] == YES
	    || [os hasPrefix: @"netbsd"] == YES
	    || [os hasPrefix: @"openbsd"] == YES)
	    {
	      _operatingSystemName = @"GSBSDOperatingSystem";
	      _operatingSystem = GSBSDOperatingSystem;
	    }
	  else if ([os hasPrefix: @"beos"] == YES)
	    {
	      _operatingSystemName = @"GSBeOperatingSystem";
	      _operatingSystem = GSBeOperatingSystem;
	    }
	  else if ([os hasPrefix: @"darwin"] == YES)
	    {
	      _operatingSystemName = @"NSMACHOperatingSystem";
	      _operatingSystem = NSMACHOperatingSystem;
	    }
	  else if ([os hasPrefix: @"solaris"] == YES)
	    {
	      _operatingSystemName = @"NSSolarisOperatingSystem";
	      _operatingSystem = NSSolarisOperatingSystem;
	    }
	  else if ([os hasPrefix: @"hpux"] == YES)
	    {
	      _operatingSystemName = @"NSHPUXOperatingSystem";
	      _operatingSystem = NSHPUXOperatingSystem;
	    }
	  else if ([os hasPrefix: @"sunos"] == YES)
	    {
	      _operatingSystemName = @"NSSunOSOperatingSystem";
	      _operatingSystem = NSSunOSOperatingSystem;
	    }
	  else if ([os hasPrefix: @"osf"] == YES)
	    {
	      _operatingSystemName = @"NSOSF1OperatingSystem";
	      _operatingSystem = NSOSF1OperatingSystem;
	    }
	  if (_operatingSystem == 0 && [os isEqual: fallback] == NO)
	    {
	      os = fallback;
	      parseOS = YES;	// Try again with fallback
	    }
	}

      if (_operatingSystem == 0)
        {
	  NSWarnFLog(@"Unable to determine O/S ... assuming GNU/Linux");
	  _operatingSystemName = @"GSGNULinuxOperatingSystem";
	  _operatingSystem = GSGNULinuxOperatingSystem;
	}
    }
}

- (NSUInteger) operatingSystem
{
  if (_operatingSystem == 0)
    {
      determineOperatingSystem();
    }
  return _operatingSystem;
}

- (NSString*) operatingSystemName
{
  if (_operatingSystemName == 0)
    {
      determineOperatingSystem();
    }
  return _operatingSystemName;
}

- (NSString *) operatingSystemVersionString
{
  if (_operatingSystemVersion == nil)
    {
      determineOperatingSystem();
    }
  return _operatingSystemVersion;
}

- (int) processIdentifier
{
  int	pid;

#if defined(_WIN32)
  pid = (int)GetCurrentProcessId();
#else
  pid = (int)getpid();
#endif
  return pid;
}

- (NSString *) processName
{
  return _gnu_processName;
}

- (void) setProcessName: (NSString *)newName
{
  if (newName && [newName length])
    {
      [_gnu_processName autorelease];
      _gnu_processName = [newName copyWithZone: [self zone]];
    }
  return;
}

- (NSUInteger) processorCount
{
  static NSUInteger	procCount = 0;
  static BOOL		beenHere = NO;

  if (beenHere == NO)
    {
#if	defined(_WIN32)
      SYSTEM_INFO info;

      GetSystemInfo(&info);
      return info.dwNumberOfProcessors;
#elif	defined(_SC_NPROCESSORS_CONF)
      procCount = sysconf(_SC_NPROCESSORS_CONF);
#elif	defined(HAVE_SYSCTLBYNAME)
      int	val;
      size_t	len = sizeof(val);

      if (sysctlbyname("hw.ncpu", &val, &len, 0, 0) == 0)
        {
          procCount = val;
        }
#elif	defined(HAVE_PROCFS)
      NSFileManager	*fileManager = [NSFileManager defaultManager];

      if ([fileManager fileExistsAtPath: @"/proc/cpuinfo"])
	{
	  NSString	*cpuInfo;
	  NSArray	*a;
	  unsigned	i;

	  cpuInfo = [NSString stringWithContentsOfFile: @"/proc/cpuinfo"];
	  a = [cpuInfo componentsSeparatedByCharactersInSet:
	    [NSCharacterSet whitespaceAndNewlineCharacterSet]];
	  // syntax is processor : #
	  // count up each one
	  for (i = 0; i < [a count]; ++i)
	    {
	      if ([[a objectAtIndex: i] isEqualToString: @"processor"])
		{
		  if (((i+1) < [a count])
		    && [[a objectAtIndex: i+1] isEqualToString: @":"])
		    {
		      procCount++;
		    }
		}
	    }
	}
#else
#warning	"no known way to determine number of processors on this system"
#endif

      beenHere = YES;
      if (procCount == 0)
	{
	  NSLog(@"Cannot determine processor count.");
	}
    }    
  return procCount;
}

- (NSUInteger) activeProcessorCount
{
#if	defined(_WIN32)
  SYSTEM_INFO info;
  int	index;
  int	count = 0;

  GetSystemInfo(&info);
  for (index = 0; index < 32; index++)
    {
      if (info.dwActiveProcessorMask & (1<<index))
	{
	  count++;
	}
    }
  return count;
#elif	defined(_SC_NPROCESSORS_ONLN)
  return sysconf(_SC_NPROCESSORS_ONLN);
#elif	defined(HAVE_SYSCTLBYNAME)
  int		val;
  size_t	len = sizeof(val);

  if (sysctlbyname("kern.smp.cpus", &val, &len, 0, 0) == 0)
    {
      return val;
    }
  else if (sysctlbyname("hw.activecpu", &val, &len, 0, 0) == 0)
    {
      return val;
    }
  return [self processorCount];
#else
  return [self processorCount];
#endif
}

- (unsigned long long) physicalMemory
{
  static NSUInteger availMem = 0;
  static BOOL beenHere = NO;

  if (beenHere == NO)
    {
#if	defined(_WIN32)
      MEMORYSTATUSEX memory;

      memory.dwLength = sizeof(memory);
      GlobalMemoryStatusEx(&memory);
      return memory.ullTotalPhys;
#elif	defined(_SC_PHYS_PAGES)
      availMem = sysconf(_SC_PHYS_PAGES) * NSPageSize();
#elif	defined(HAVE_SYSCTLBYNAME)
      long	val;
      size_t	len = val;

      if (sysctlbyname("hw.physmem", &val, &len, 0, 0) == 0)
        {
          availMem = val;
        }
#elif	defined(HAVE_PROCFS)
      NSFileManager *fileManager = [NSFileManager defaultManager];

      if ([fileManager fileExistsAtPath: @"/proc/meminfo"])
	{
	  NSString	*memInfo;
	  NSString	*s;
	  NSArray	*a;
	  NSRange	r;

	  memInfo = [NSString stringWithContentsOfFile: @"/proc/meminfo"];
	  r = [memInfo rangeOfString: @"MemTotal:"];

	  if (r.location == NSNotFound)
	    {
	      NSLog(@"Cannot determine amount of physical memory.");
	      return 0;
	    }
	  s = [[memInfo substringFromIndex: (r.location + r.length)]
	    stringByTrimmingCharactersInSet:
	    [NSCharacterSet whitespaceAndNewlineCharacterSet]];
	  a = [s componentsSeparatedByString: @" "];
	  s = [a objectAtIndex: 0];
	  availMem = (NSUInteger)[s longLongValue];
	  availMem *= NSPageSize();
	}
#else
#warning	"no known way to determine amount of memory on this system"
#endif
  
      beenHere = YES;
      if (availMem == 0)
	{
	  NSLog(@"Cannot determine amount of physical memory.");
	}
    }
  return availMem;
}

- (NSUInteger) systemUptime
{
  NSUInteger uptime = 0;

#if	defined(_WIN32)

#if _WIN32_WINNT < 0x0600 /* less than Vista */
  uptime = GetTickCount() / 1000;
#else
  uptime = GetTickCount64() / 1000;
#endif
  
#elif	defined(HAVE_SYSCTLBYNAME)
  struct timeval	tval;
  size_t		len = sizeof(tval);

  if (sysctlbyname("kern.boottime", &tval, &len, 0, 0) == 0)
    {
      uptime = tval.tv_sec;
    }
#elif	defined(HAVE_PROCFS)
  NSFileManager *fileManager = [NSFileManager defaultManager];

  if ([fileManager fileExistsAtPath: @"/proc/uptime"])
    {
      NSString *uptimeContent = [NSString
	stringWithContentsOfFile: @"/proc/uptime"];
      NSString *uptimeString = [[uptimeContent
	componentsSeparatedByString:@" "] objectAtIndex:0];
      uptime = [uptimeString intValue];
    }
#else
#warning	"no known way to determine uptime on this system"
#endif

  if (uptime == 0)
    {
      NSLog(@"Cannot determine uptime.");
    }

  return uptime;
}

- (void) enableSuddenTermination
{
  // FIXME: unimplemented
  return;
}

- (void) disableSuddenTermination
{
  // FIXME: unimplemented
  return;
}

- (id) beginActivityWithOptions: (NSActivityOptions)options
                         reason: (NSString *)reason
{
  // FIXME: unimplemented
  return nil;
}

- (void) endActivity:(id<NSObject>)activity
{
  // FIXME: unimplemented
  return;
}

- (void) performActivityWithOptions:(NSActivityOptions)options
                            reason: (NSString *)reason
                        usingBlock: (GSPerformActivityBlock)block
{
  // FIXME: unimplemented
  return;
}

- (void) performExpiringActivityWithReason: (NSString *)reason
		usingBlock: (GSPerformExpiringActivityBlock)block
{
  // FIXME: unimplemented
  return;
}

@end

void
GSInitializeProcess(int argc, char **argv, char **envp)
{
  [NSProcessInfo class];
  [procLock lock];
  fallbackInitialisation = YES;
  _gnu_process_args(argc, argv, envp);
  [procLock unlock];
}

#ifdef __ANDROID__
void
GSInitializeProcessAndroid(JNIEnv *env, jobject context)
{
  [NSProcessInfo class];

  // create global reference to to prevent garbage collection
  _androidContext = (*env)->NewGlobalRef(env, context);

  // get package code path (path to APK)
  jclass cls = (*env)->GetObjectClass(env, context);
  jmethodID packageCodePathMethod = (*env)->GetMethodID(env, cls, "getPackageCodePath", "()Ljava/lang/String;");
  jstring packageCodePathJava = (*env)->CallObjectMethod(env, context, packageCodePathMethod);
  const char *packageCodePath = (*env)->GetStringUTFChars(env, packageCodePathJava, NULL);

  // get package name
  jmethodID packageNameMethod = (*env)->GetMethodID(env, cls, "getPackageName", "()Ljava/lang/String;");
  jstring packageNameJava = (*env)->CallObjectMethod(env, context, packageNameMethod);
  const char *packageName = (*env)->GetStringUTFChars(env, packageNameJava, NULL);

  // create fake executable path consisting of package code path (without .apk)
  // and package name as executable
  char *lastSlash = strrchr(packageCodePath, '/');
  if (lastSlash == NULL)
    {
      lastSlash = (char *)packageCodePath + strlen(packageCodePath);
    }
  char *arg0;
  asprintf(&arg0, "%.*s/%s", (int)(lastSlash - packageCodePath), packageCodePath, packageName);

  (*env)->ReleaseStringUTFChars(env, packageCodePathJava, packageCodePath);
  (*env)->ReleaseStringUTFChars(env, packageNameJava, packageName);

  // initialize process
  [procLock lock];
  fallbackInitialisation = YES;
  char *argv[] = {
    arg0,
    "-GSLogSyslog", "YES" // use syslog (available via logcat) instead of stdout/stderr (not available on Android)
  };
  _gnu_process_args(sizeof(argv)/sizeof(char *), argv, NULL);
  [procLock unlock];

  free(arg0);

  // get File class and path method
  jclass fileCls = (*env)->FindClass(env, "java/io/File");
  jmethodID getAbsolutePathMethod = (*env)->GetMethodID(env, fileCls, "getAbsolutePath", "()Ljava/lang/String;");

  // get Android files dir
  jmethodID filesDirMethod = (*env)->GetMethodID(env, cls, "getFilesDir", "()Ljava/io/File;");
  jobject filesDirObj = (*env)->CallObjectMethod(env, context, filesDirMethod);
  jstring filesDirJava = (*env)->CallObjectMethod(env, filesDirObj, getAbsolutePathMethod);
	const jchar *filesDirUnichars = (*env)->GetStringChars(env, filesDirJava, NULL);
  jsize filesDirLength = (*env)->GetStringLength(env, filesDirJava);
  _androidFilesDir = [NSString stringWithCharacters:filesDirUnichars length:filesDirLength];
  (*env)->ReleaseStringChars(env, filesDirJava, filesDirUnichars);

  // get Android cache dir
  jmethodID cacheDirMethod = (*env)->GetMethodID(env, cls, "getCacheDir", "()Ljava/io/File;");
  jobject cacheDirObj = (*env)->CallObjectMethod(env, context, cacheDirMethod);
  jstring cacheDirJava = (*env)->CallObjectMethod(env, cacheDirObj, getAbsolutePathMethod);
	const jchar *cacheDirUnichars = (*env)->GetStringChars(env, cacheDirJava, NULL);
  jsize cacheDirLength = (*env)->GetStringLength(env, cacheDirJava);
  _androidCacheDir = [NSString stringWithCharacters:cacheDirUnichars length:cacheDirLength];
  (*env)->ReleaseStringChars(env, cacheDirJava, cacheDirUnichars);

  // get asset manager and initialize NSBundle
  jmethodID assetManagerMethod = (*env)->GetMethodID(env, cls, "getAssets", "()Landroid/content/res/AssetManager;");
  jstring assetManagerJava = (*env)->CallObjectMethod(env, context, assetManagerMethod);
  [NSBundle setJavaAssetManager:assetManagerJava withJNIEnv:env];

  // clean up our NSTemporaryDirectory() if it exists
  NSString *tempDirName = [_androidCacheDir stringByAppendingPathComponent: @"tmp"];
  [[NSFileManager defaultManager] removeItemAtPath:tempDirName error:NULL];
}
#endif

@implementation	NSProcessInfo (GNUstep)

+ (void) initializeWithArguments: (char**)argv
                           count: (int)argc
                     environment: (char**)env
{
  GSInitializeProcess(argc, argv, env);
}

- (BOOL) setLogFile: (NSString*)path
{
  extern int	_NSLogDescriptor;
  int		desc;

#if	defined(_WIN32)
  desc = _wopen((wchar_t*)[path fileSystemRepresentation],
    O_RDWR|O_CREAT|O_APPEND, 0644);
#else
  desc = open([path fileSystemRepresentation], O_RDWR|O_CREAT|O_APPEND, 0644);
#endif
  if (desc >= 0)
    {
      if (_NSLogDescriptor >= 0 && _NSLogDescriptor != 2)
	{
	  close(_NSLogDescriptor);
	}
      _NSLogDescriptor = desc;
      return YES;
    }
  return NO;
}

#ifdef __ANDROID__
- (jobject) androidContext
{
  return _androidContext;
}

- (NSString *) androidFilesDir
{
  return _androidFilesDir;
}

- (NSString *) androidCacheDir
{
  return _androidCacheDir;
}
#endif

@end

BOOL
GSPrivateEnvironmentFlag(const char *name, BOOL def)
{
  const char	*c = getenv(name);
  BOOL		a = def;

  if (c != 0)
    {
      a = NO;
      if ((c[0] == 'y' || c[0] == 'Y') && (c[1] == 'e' || c[1] == 'E')
	&& (c[2] == 's' || c[2] == 'S') && c[3] == 0)
	{
	  a = YES;
	}
      else if ((c[0] == 't' || c[0] == 'T') && (c[1] == 'r' || c[1] == 'R')
	&& (c[2] == 'u' || c[2] == 'U') && (c[3] == 'e' || c[3] == 'E')
	&& c[4] == 0)
	{
	  a = YES;
	}
      else if (isdigit(c[0]) && c[0] != '0')
	{
	  a = YES;
	}
    }
  return a;
}

const char*
GSPrivateArgZero()
{
  if (_gnu_arg_zero == 0)
    return "";
  else
    return _gnu_arg_zero;
}

