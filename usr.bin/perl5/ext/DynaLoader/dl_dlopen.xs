/* dl_dlopen.xs
 * 
 * Platform:	SunOS/Solaris, possibly others which use dlopen.
 * Author:	Paul Marquess (Paul.Marquess@btinternet.com)
 * Created:	10th July 1994
 *
 * Modified:
 * 15th July 1994     - Added code to explicitly save any error messages.
 * 3rd August 1994    - Upgraded to v3 spec.
 * 9th August 1994    - Changed to use IV
 * 10th August 1994   - Tim Bunce: Added RTLD_LAZY, switchable debugging,
 *                      basic FreeBSD support, removed ClearError
 * 29th February 2000 - Alan Burlison: Added functionality to close dlopen'd
 *                      files when the interpreter exits
 * 2015-03-12         - rurban: Added optional 3rd dl_find_symbol argument
 *
 */

/* Porting notes:


   Definition of Sunos dynamic Linking functions
   =============================================
   In order to make this implementation easier to understand here is a
   quick definition of the SunOS Dynamic Linking functions which are
   used here.

   dlopen
   ------
     void *
     dlopen(path, mode)
     char * path; 
     int    mode;

     This function takes the name of a dynamic object file and returns
     a descriptor which can be used by dlsym later. It returns NULL on
     error.

     The mode parameter must be set to 1 for Solaris 1 and to
     RTLD_LAZY (==2) on Solaris 2.


   dlclose
   -------
     int
     dlclose(handle)
     void * handle;

     This function takes the handle returned by a previous invocation of
     dlopen and closes the associated dynamic object file.  It returns zero
     on success, and non-zero on failure.


   dlsym
   ------
     void *
     dlsym(handle, symbol)
     void * handle; 
     char * symbol;

     Takes the handle returned from dlopen and the name of a symbol to
     get the address of. If the symbol was found a pointer is
     returned.  It returns NULL on error. If DL_PREPEND_UNDERSCORE is
     defined an underscore will be added to the start of symbol. This
     is required on some platforms (freebsd).

   dlerror
   ------
     char * dlerror()

     Returns a null-terminated string which describes the last error
     that occurred with either dlopen or dlsym. After each call to
     dlerror the error message will be reset to a null pointer. The
     SaveError function is used to save the error as soon as it happens.

     Note that the POSIX standard does not require a per-thread buffer for
     the error message, and so on multi-threaded builds, it can be overwritten
     by another thread before SaveError accomplishes its task.  Some systems do
     have a per-thread buffer.  The man page on your system should tell you.
     If your code might be run on a system where this function is not thread
     safe, you should protect your calls with mutexes.  See "Dealing with Error
     Messages" below.


   Return Types
   ============
   In this implementation the two functions, dl_load_file &
   dl_find_symbol, return void *. This is because the underlying SunOS
   dynamic linker calls also return void *.  This is not necessarily
   the case for all architectures. For example, some implementation
   will want to return a char * for dl_load_file.

   If void * is not appropriate for your architecture, you will have to
   change the void * to whatever you require. If you are not certain of
   how Perl handles C data types, I suggest you start by consulting	
   Dean Roerich's Perl 5 API document. Also, have a look in the typemap 
   file (in the ext directory) for a fairly comprehensive list of types 
   that are already supported. If you are completely stuck, I suggest you
   post a message to perl5-porters.

   Remember when you are making any changes that the return value from 
   dl_load_file is used as a parameter in the dl_find_symbol 
   function. Also the return value from find_symbol is used as a parameter 
   to install_xsub.


   Dealing with Error Messages
   ============================
   In order to make the handling of dynamic linking errors as generic as
   possible you should store any error messages associated with your
   implementation with the SaveError function.

   In the case of SunOS the function dlerror returns the error message 
   associated with the last dynamic link error. As the SunOS dynamic 
   linker functions dlopen & dlsym both return NULL on error every call 
   to a SunOS dynamic link routine is coded like this

	RETVAL = dlopen(filename, 1) ;
	if (RETVAL == NULL)
	    SaveError("%s",dlerror()) ;

   Note that SaveError() takes a printf format string. Use a "%s" as
   the first parameter if the error may contain any % characters.
   dlerror() may not be thread-safe on some systems; if this code is run on
   any of those, a mutex should be added.  khw (who added this comment) has no
   idea which systems aren't thread-safe, but consider this possibility when
   debugging.

*/

#define PERL_NO_GET_CONTEXT
#define PERL_EXT

#include "EXTERN.h"
#define PERL_IN_DL_DLOPEN_XS
#include "perl.h"
#include "XSUB.h"

#ifdef I_DLFCN
#include <dlfcn.h>	/* the dynamic linker include file for Sunos/Solaris */
#else
#include <nlist.h>
#include <link.h>
#endif

#ifndef RTLD_LAZY
# define RTLD_LAZY 1	/* Solaris 1 */
#endif

#ifndef HAS_DLERROR
# ifdef __NetBSD__
#  define dlerror() strerror(errno)
# else
#  define dlerror() "Unknown error - dlerror() not implemented"
# endif
#endif


#include "dlutils.c"	/* SaveError() etc	*/


static void
dl_private_init(pTHX)
{
    (void)dl_generic_private_init(aTHX);
}

MODULE = DynaLoader	PACKAGE = DynaLoader

BOOT:
    (void)dl_private_init(aTHX);


void
dl_load_file(filename, flags=0)
    char *	filename
    int		flags
  PREINIT:
    int mode = RTLD_LAZY;
    void *handle;
  CODE:
{
#if defined(DLOPEN_WONT_DO_RELATIVE_PATHS)
    char pathbuf[PATH_MAX + 2];
    if (*filename != '/' && strchr(filename, '/')) {
        const size_t filename_len = strlen(filename);
        if (getcwd(pathbuf, PATH_MAX - filename_len)) {
            const size_t path_len = strlen(pathbuf);
            pathbuf[path_len] = '/';
            filename = (char *) memcpy(pathbuf + path_len + 1, filename, filename_len + 1);
	}
    }
#endif
#ifdef RTLD_NOW
    {
	dMY_CXT;
	if (dl_nonlazy)
	    mode = RTLD_NOW;
    }
#endif
    if (flags & 0x01)
#ifdef RTLD_GLOBAL
	mode |= RTLD_GLOBAL;
#else
	Perl_warn(aTHX_ "Can't make loaded symbols global on this platform while loading %s",filename);
#endif
    DLDEBUG(1,PerlIO_printf(Perl_debug_log, "dl_load_file(%s,%x):\n", filename,flags));
    handle = dlopen(filename, mode) ;
    DLDEBUG(2,PerlIO_printf(Perl_debug_log, " libref=%lx\n", (unsigned long) handle));
    ST(0) = sv_newmortal() ;
    if (handle == NULL)
	SaveError(aTHX_ "%s",dlerror()) ;
    else
	sv_setiv( ST(0), PTR2IV(handle));
}


int
dl_unload_file(libref)
    void *	libref
  CODE:
    DLDEBUG(1,PerlIO_printf(Perl_debug_log, "dl_unload_file(%lx):\n", PTR2ul(libref)));
    RETVAL = (dlclose(libref) == 0 ? 1 : 0);
    if (!RETVAL)
        SaveError(aTHX_ "%s", dlerror()) ;
    DLDEBUG(2,PerlIO_printf(Perl_debug_log, " retval = %d\n", RETVAL));
  OUTPUT:
    RETVAL


void
dl_find_symbol(libhandle, symbolname, ign_err=0)
    void *	libhandle
    char *	symbolname
    int	        ign_err
    PREINIT:
    void *sym;
    CODE:
#ifdef DLSYM_NEEDS_UNDERSCORE
    symbolname = Perl_form_nocontext("_%s", symbolname);
#endif
    DLDEBUG(2, PerlIO_printf(Perl_debug_log,
			     "dl_find_symbol(handle=%lx, symbol=%s)\n",
			     (unsigned long) libhandle, symbolname));
    sym = dlsym(libhandle, symbolname);
    DLDEBUG(2, PerlIO_printf(Perl_debug_log,
			     "  symbolref = %lx\n", (unsigned long) sym));
    ST(0) = sv_newmortal();
    if (sym == NULL) {
        if (!ign_err)
	    SaveError(aTHX_ "%s", dlerror());
    } else
	sv_setiv( ST(0), PTR2IV(sym));


void
dl_undef_symbols()
    CODE:



# These functions should not need changing on any platform:

void
dl_install_xsub(perl_name, symref, filename="$Package")
    char *		perl_name
    void *		symref 
    const char *	filename
    CODE:
    DLDEBUG(2,PerlIO_printf(Perl_debug_log, "dl_install_xsub(name=%s, symref=%" UVxf ")\n",
		perl_name, PTR2UV(symref)));
    ST(0) = sv_2mortal(newRV((SV*)newXS_flags(perl_name,
					      DPTR2FPTR(XSUBADDR_t, symref),
					      filename, NULL,
					      XS_DYNAMIC_FILENAME)));


SV *
dl_error()
    CODE:
    dMY_CXT;
    RETVAL = newSVsv(MY_CXT.x_dl_last_error);
    OUTPUT:
    RETVAL

#if defined(USE_ITHREADS)

void
CLONE(...)
    CODE:
    MY_CXT_CLONE;

    PERL_UNUSED_VAR(items);

    /* MY_CXT_CLONE just does a memcpy on the whole structure, so to avoid
     * using Perl variables that belong to another thread, we create our 
     * own for this thread.
     */
    MY_CXT.x_dl_last_error = newSVpvs("");

#endif

# end.
