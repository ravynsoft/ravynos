#include "dlfcn.h"
#include "string.h"
#include "stdio.h"

#define INCL_BASE
#include <os2.h>
#include <float.h>
#include <stdlib.h>

static ULONG retcode;
static char fail[300];

static ULONG dllHandle;
static int handle_found;
static int handle_loaded;
#ifdef PERL_CORE

#include "EXTERN.h"
#include "perl.h"

#else

char *os2error(int rc);

#endif

#ifdef DLOPEN_INITTERM
unsigned long _DLL_InitTerm(unsigned long modHandle, unsigned long flag)
{
    switch (flag) {
    case 0:     /* INIT */
        /* Save handle */
        dllHandle = modHandle;
        handle_found = 1;
        return TRUE;

    case 1:     /* TERM */
        handle_found = 0;
        dllHandle = (unsigned long)NULLHANDLE;
        return TRUE;
    }

    return FALSE;
}

#endif

HMODULE
find_myself(void)
{

  static APIRET APIENTRY (*pDosQueryModFromEIP) (HMODULE * hmod, ULONG * obj, ULONG BufLen, PCHAR Buf,
                    ULONG * Offset, ULONG Address);
  HMODULE doscalls_h, mod;
  static int failed;
  ULONG obj, offset, rc;
  char buf[260];

  if (failed)
        return 0;
  failed = 1;
  doscalls_h = (HMODULE)dlopen("DOSCALLS",0);
  if (!doscalls_h)
        return 0;
/*  {&doscalls_handle, NULL, 360}, */	/* DosQueryModFromEIP */
  rc = DosQueryProcAddr(doscalls_h, 360, 0, (PFN*)&pDosQueryModFromEIP);
  if (rc)
        return 0;
  rc = pDosQueryModFromEIP(&mod, &obj, sizeof(buf), buf, &offset, (ULONG)dlopen);
  if (rc)
        return 0;
  failed = 0;
  handle_found = 1;
  dllHandle = mod;
  return mod;
}

void *
dlopen(const char *path, int mode)
{
        HMODULE handle;
        char tmp[260];
        const char *beg, *dot;
        ULONG rc;
        unsigned fpflag = _control87(0,0);

        fail[0] = 0;
        if (!path) {			/* Our own handle. */
            if (handle_found || find_myself()) {
                char dllname[260];

                if (handle_loaded)
                    return (void*)dllHandle;
                rc = DosQueryModuleName(dllHandle, sizeof(dllname), dllname);
                if (rc) {
                    strcpy(fail, "can't find my DLL name by the handle");
                    retcode = rc;
                    return 0;
                }
                rc = DosLoadModule(fail, sizeof fail, dllname, &handle);
                if (rc) {
                    strcpy(fail, "can't load my own DLL");
                    retcode = rc;
                    return 0;
                }
                handle_loaded = 1;
                goto ret;
            }
            retcode = ERROR_MOD_NOT_FOUND;
            strcpy(fail, "can't load from myself: compiled without -DDLOPEN_INITTERM");
            return 0;
        }
        if ((rc = DosLoadModule(fail, sizeof fail, (char*)path, &handle)) == 0)
                goto ret;

        retcode = rc;

        if (strlen(path) >= sizeof(tmp))
            return NULL;

        /* Not found. Check for non-FAT name and try truncated name. */
        /* Don't know if this helps though... */
        for (beg = dot = path + strlen(path);
             beg > path && !memCHRs(":/\\", *(beg-1));
             beg--)
                if (*beg == '.')
                        dot = beg;
        if (dot - beg > 8) {
                int n = beg+8-path;

                memmove(tmp, path, n);
                memmove(tmp+n, dot, strlen(dot)+1);
                if (DosLoadModule(fail, sizeof fail, tmp, &handle) == 0)
                    goto ret;
        }
        handle = 0;

      ret:
        _control87(fpflag, MCW_EM); /* Some modules reset FP flags on load */
        return (void *)handle;
}

#define ERROR_WRONG_PROCTYPE 0xffffffff

void *
dlsym(void *handle, const char *symbol)
{
        ULONG rc, type;
        PFN addr;

        fail[0] = 0;
        rc = DosQueryProcAddr((HMODULE)handle, 0, symbol, &addr);
        if (rc == 0) {
                rc = DosQueryProcType((HMODULE)handle, 0, symbol, &type);
                if (rc == 0 && type == PT_32BIT)
                        return (void *)addr;
                rc = ERROR_WRONG_PROCTYPE;
        }
        retcode = rc;
        return NULL;
}

char *
dlerror(void)
{
        static char buf[700];
        ULONG len;
        char *err;

        if (retcode == 0)
                return NULL;
        if (retcode == ERROR_WRONG_PROCTYPE)
            err = "Wrong procedure type";
        else
            err = os2error(retcode);
        len = strlen(err);
        if (len > sizeof(buf) - 1)
            len = sizeof(buf) - 1;
        strncpy(buf, err, len+1);
        if (fail[0] && len + strlen(fail) < sizeof(buf) - 100)
            sprintf(buf + len, ", possible problematic module: '%s'", fail);
        retcode = 0;
        return buf;
}

int
dlclose(void *handle)
{
        ULONG rc;

        if ((rc = DosFreeModule((HMODULE)handle)) == 0) return 0;

        retcode = rc;
        return 2;
}
