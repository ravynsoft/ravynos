/* dl_win32.xs
 * 
 * Platform:	Win32 (Windows NT/Windows 95)
 * Author:	Wei-Yuen Tan (wyt@hip.com)
 * Created:	A warm day in June, 1995
 *
 * Modified:
 *    August 23rd 1995 - rewritten after losing everything when I
 *                       wiped off my NT partition (eek!)
 */

/* Porting notes:

I merely took Paul's dl_dlopen.xs, took out extraneous stuff and
replaced the appropriate SunOS calls with the corresponding Win32
calls.

*/

#define WIN32_LEAN_AND_MEAN
#ifdef __GNUC__
#define Win32_Winsock
#endif
#include <windows.h>
#include <string.h>

#define PERL_NO_GET_CONTEXT
#define PERL_EXT
#define PERL_IN_DL_WIN32_XS

#include "EXTERN.h"
#include "perl.h"
#include "win32.h"

#include "XSUB.h"

typedef struct {
    SV *	x_error_sv;
} my_cxtx_t;		/* this *must* be named my_cxtx_t */

#define DL_CXT_EXTRA	/* ask for dl_cxtx to be defined in dlutils.c */
#include "dlutils.c"	/* SaveError() etc	*/

#define dl_error_sv	(dl_cxtx.x_error_sv)

static char *
OS_Error_String(pTHX)
{
    dMY_CXT;
    DWORD err = GetLastError();
    STRLEN len;
    SV ** l_dl_error_svp = &dl_error_sv;
    SV * l_dl_error_sv;
    if (!*l_dl_error_svp)
	*l_dl_error_svp = newSVpvs("");
    l_dl_error_sv = *l_dl_error_svp;
    PerlProc_GetOSError(l_dl_error_sv,err);
    return SvPV(l_dl_error_sv,len);
}

static void
dl_private_init(pTHX)
{
    (void)dl_generic_private_init(aTHX);
}

/* 
    This function assumes the list staticlinkmodules
    will be formed from package names with '::' replaced
    with '/'. Thus Win32::OLE is in the list as Win32/OLE
*/
static int
dl_static_linked(char *filename)
{
    const char * const *p;
    char *ptr, *hptr;
    static const char subStr[] = "/auto/";
    char szBuffer[MAX_PATH];

    /* avoid buffer overflow when called with invalid filenames */
    if (strlen(filename) >= sizeof(szBuffer))
        return 0;

    /* change all the '\\' to '/' */
    my_strlcpy(szBuffer, filename, sizeof(szBuffer));
    for(ptr = szBuffer; ptr = strchr(ptr, '\\'); ++ptr)
	*ptr = '/';

    /* delete the file name */
    ptr = strrchr(szBuffer, '/');
    if(ptr != NULL)
	*ptr = '\0';

    /* remove leading lib path */
    ptr = strstr(szBuffer, subStr);
    if(ptr != NULL)
	ptr += sizeof(subStr)-1;
    else
	ptr = szBuffer;

    for (p = staticlinkmodules; *p;p++) {
	if (hptr = strstr(ptr, *p)) {
	    /* found substring, need more detailed check if module name match */
	    if (hptr==ptr) {
		return strEQ(ptr, *p);
	    }
	    if (hptr[strlen(*p)] == 0)
		return hptr[-1]=='/';
	}
    };
    return 0;
}

MODULE = DynaLoader	PACKAGE = DynaLoader

BOOT:
    (void)dl_private_init(aTHX);

void
dl_load_file(filename,flags=0)
    char *		filename
#flags is unused
    SV *		flags = NO_INIT
    PREINIT:
    void *retv;
    SV * retsv;
    CODE:
  {
    PERL_UNUSED_VAR(flags);
    DLDEBUG(1,PerlIO_printf(Perl_debug_log,"dl_load_file(%s):\n", filename));
    if (dl_static_linked(filename) == 0) {
	retv = PerlProc_DynaLoad(filename);
    }
    else
	retv = (void*) Win_GetModuleHandle(NULL);
    DLDEBUG(2,PerlIO_printf(Perl_debug_log," libref=%x\n", retv));

    if (retv == NULL) {
	SaveError(aTHX_ "load_file:%s",
		  OS_Error_String(aTHX)) ;
	retsv = &PL_sv_undef;
    }
    else
	retsv = sv_2mortal(newSViv((IV)retv));
    ST(0) = retsv;
  }

int
dl_unload_file(libref)
    void *	libref
  CODE:
    DLDEBUG(1,PerlIO_printf(Perl_debug_log, "dl_unload_file(%lx):\n", PTR2ul(libref)));
    RETVAL = FreeLibrary((HMODULE)libref);
    if (!RETVAL)
        SaveError(aTHX_ "unload_file:%s", OS_Error_String(aTHX)) ;
    DLDEBUG(2,PerlIO_printf(Perl_debug_log, " retval = %d\n", RETVAL));
  OUTPUT:
    RETVAL

void
dl_find_symbol(libhandle, symbolname, ign_err=0)
    void *	libhandle
    char *	symbolname
    int	        ign_err
    PREINIT:
    void *retv;
    CODE:
    DLDEBUG(2,PerlIO_printf(Perl_debug_log,"dl_find_symbol(handle=%x, symbol=%s)\n",
		      libhandle, symbolname));
    retv = (void*) GetProcAddress((HINSTANCE) libhandle, symbolname);
    DLDEBUG(2,PerlIO_printf(Perl_debug_log,"  symbolref = %x\n", retv));
    ST(0) = sv_newmortal();
    if (retv == NULL) {
        if (!ign_err) SaveError(aTHX_ "find_symbol:%s", OS_Error_String(aTHX));
    } else
	sv_setiv( ST(0), (IV)retv);


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
    DLDEBUG(2,PerlIO_printf(Perl_debug_log,"dl_install_xsub(name=%s, symref=%x)\n",
		      perl_name, symref));
    ST(0) = sv_2mortal(newRV((SV*)newXS(perl_name,
					(void(*)(pTHX_ CV *))symref,
					filename)));


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
