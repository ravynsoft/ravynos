#include <process.h>
#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_DOSNLS
#define INCL_WINSWITCHLIST
#define INCL_WINWINDOWMGR
#define INCL_WININPUT
#define INCL_VIO
#define INCL_KBD
#define INCL_WINCLIPBOARD
#define INCL_WINATOM
#include <os2.h>

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

static unsigned long
constant(char *name, int arg)
{
    errno = 0;
    if (name[0] == 'P' && name[1] == '_') {
	if (strEQ(name, "P_BACKGROUND"))
#ifdef P_BACKGROUND
	    return P_BACKGROUND;
#else
	    goto not_there;
#endif
	if (strEQ(name, "P_DEBUG"))
#ifdef P_DEBUG
	    return P_DEBUG;
#else
	    goto not_there;
#endif
	if (strEQ(name, "P_DEFAULT"))
#ifdef P_DEFAULT
	    return P_DEFAULT;
#else
	    goto not_there;
#endif
	if (strEQ(name, "P_DETACH"))
#ifdef P_DETACH
	    return P_DETACH;
#else
	    goto not_there;
#endif
	if (strEQ(name, "P_FOREGROUND"))
#ifdef P_FOREGROUND
	    return P_FOREGROUND;
#else
	    goto not_there;
#endif
	if (strEQ(name, "P_FULLSCREEN"))
#ifdef P_FULLSCREEN
	    return P_FULLSCREEN;
#else
	    goto not_there;
#endif
	if (strEQ(name, "P_MAXIMIZE"))
#ifdef P_MAXIMIZE
	    return P_MAXIMIZE;
#else
	    goto not_there;
#endif
	if (strEQ(name, "P_MINIMIZE"))
#ifdef P_MINIMIZE
	    return P_MINIMIZE;
#else
	    goto not_there;
#endif
	if (strEQ(name, "P_NOCLOSE"))
#ifdef P_NOCLOSE
	    return P_NOCLOSE;
#else
	    goto not_there;
#endif
	if (strEQ(name, "P_NOSESSION"))
#ifdef P_NOSESSION
	    return P_NOSESSION;
#else
	    goto not_there;
#endif
	if (strEQ(name, "P_NOWAIT"))
#ifdef P_NOWAIT
	    return P_NOWAIT;
#else
	    goto not_there;
#endif
	if (strEQ(name, "P_OVERLAY"))
#ifdef P_OVERLAY
	    return P_OVERLAY;
#else
	    goto not_there;
#endif
	if (strEQ(name, "P_PM"))
#ifdef P_PM
	    return P_PM;
#else
	    goto not_there;
#endif
	if (strEQ(name, "P_QUOTE"))
#ifdef P_QUOTE
	    return P_QUOTE;
#else
	    goto not_there;
#endif
	if (strEQ(name, "P_SESSION"))
#ifdef P_SESSION
	    return P_SESSION;
#else
	    goto not_there;
#endif
	if (strEQ(name, "P_TILDE"))
#ifdef P_TILDE
	    return P_TILDE;
#else
	    goto not_there;
#endif
	if (strEQ(name, "P_UNRELATED"))
#ifdef P_UNRELATED
	    return P_UNRELATED;
#else
	    goto not_there;
#endif
	if (strEQ(name, "P_WAIT"))
#ifdef P_WAIT
	    return P_WAIT;
#else
	    goto not_there;
#endif
	if (strEQ(name, "P_WINDOWED"))
#ifdef P_WINDOWED
	    return P_WINDOWED;
#else
	    goto not_there;
#endif
    } else if (name[0] == 'T' && name[1] == '_') {
	if (strEQ(name, "FAPPTYP_NOTSPEC"))
#ifdef FAPPTYP_NOTSPEC
	    return FAPPTYP_NOTSPEC;
#else
	    goto not_there;
#endif
	if (strEQ(name, "T_NOTWINDOWCOMPAT"))
#ifdef FAPPTYP_NOTWINDOWCOMPAT
	    return FAPPTYP_NOTWINDOWCOMPAT;
#else
	    goto not_there;
#endif
	if (strEQ(name, "T_WINDOWCOMPAT"))
#ifdef FAPPTYP_WINDOWCOMPAT
	    return FAPPTYP_WINDOWCOMPAT;
#else
	    goto not_there;
#endif
	if (strEQ(name, "T_WINDOWAPI"))
#ifdef FAPPTYP_WINDOWAPI
	    return FAPPTYP_WINDOWAPI;
#else
	    goto not_there;
#endif
	if (strEQ(name, "T_BOUND"))
#ifdef FAPPTYP_BOUND
	    return FAPPTYP_BOUND;
#else
	    goto not_there;
#endif
	if (strEQ(name, "T_DLL"))
#ifdef FAPPTYP_DLL
	    return FAPPTYP_DLL;
#else
	    goto not_there;
#endif
	if (strEQ(name, "T_DOS"))
#ifdef FAPPTYP_DOS
	    return FAPPTYP_DOS;
#else
	    goto not_there;
#endif
	if (strEQ(name, "T_PHYSDRV"))
#ifdef FAPPTYP_PHYSDRV
	    return FAPPTYP_PHYSDRV;
#else
	    goto not_there;
#endif
	if (strEQ(name, "T_VIRTDRV"))
#ifdef FAPPTYP_VIRTDRV
	    return FAPPTYP_VIRTDRV;
#else
	    goto not_there;
#endif
	if (strEQ(name, "T_PROTDLL"))
#ifdef FAPPTYP_PROTDLL
	    return FAPPTYP_PROTDLL;
#else
	    goto not_there;
#endif
	if (strEQ(name, "T_32BIT"))
#ifdef FAPPTYP_32BIT
	    return FAPPTYP_32BIT;
#else
	    goto not_there;
#endif
    }

    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}

const char* const ptypes[] = { "FS", "DOS", "VIO", "PM", "DETACH" };

static char *
my_type()
{
    int rc;
    TIB *tib;
    PIB *pib;
    
    if (!(_emx_env & 0x200)) return (char*)ptypes[1]; /* not OS/2. */
    if (CheckOSError(DosGetInfoBlocks(&tib, &pib))) 
	return NULL; 
    
    return (pib->pib_ultype <= 4 ? (char*)ptypes[pib->pib_ultype] : "UNKNOWN");
}

static ULONG
file_type(char *path)
{
    int rc;
    ULONG apptype;
    
    if (!(_emx_env & 0x200)) 
	croak("file_type not implemented on DOS"); /* not OS/2. */
    if (CheckOSError(DosQueryAppType(path, &apptype))) {
#if 0
	if (rc == ERROR_INVALID_EXE_SIGNATURE) 
	    croak("Invalid EXE signature"); 
	else if (rc == ERROR_EXE_MARKED_INVALID) {
	    croak("EXE marked invalid"); 
	}
#endif
	croak_with_os2error("DosQueryAppType"); 
    }
    
    return apptype;
}

/* These use different type of wrapper.  Good to check wrappers. ;-)  */
/* XXXX This assumes DOS type return type, without SEVERITY?! */
DeclFuncByORD(HSWITCH, myWinQuerySwitchHandle,  ORD_WinQuerySwitchHandle,
		  (HWND hwnd, PID pid), (hwnd, pid))
DeclFuncByORD(ULONG, myWinQuerySwitchEntry,  ORD_WinQuerySwitchEntry,
		  (HSWITCH hsw, PSWCNTRL pswctl), (hsw, pswctl))
DeclFuncByORD(ULONG, myWinSetWindowText,  ORD_WinSetWindowText,
		  (HWND hwnd, char* text), (hwnd, text))
DeclFuncByORD(BOOL, myWinQueryWindowProcess,  ORD_WinQueryWindowProcess,
		  (HWND hwnd, PPID ppid, PTID ptid), (hwnd, ppid, ptid))
DeclFuncByORD(ULONG, XmyWinSwitchToProgram,  ORD_WinSwitchToProgram,
		  (HSWITCH hsw), (hsw))
#define myWinSwitchToProgram(hsw) (!CheckOSError(XmyWinSwitchToProgram(hsw)))


/* These function croak if the return value is 0. */
DeclWinFunc_CACHE(HWND, QueryWindow, (HWND hwnd, LONG cmd), (hwnd, cmd))
DeclWinFunc_CACHE(BOOL, QueryWindowPos, (HWND hwnd, PSWP pswp),
		  (hwnd, pswp))
DeclWinFunc_CACHE(LONG, QueryWindowText,
		  (HWND hwnd, LONG cchBufferMax, PCH pchBuffer),
		  (hwnd, cchBufferMax, pchBuffer))
DeclWinFunc_CACHE(LONG, QueryClassName, (HWND hwnd, LONG cchMax, PCH pch),
		  (hwnd, cchMax, pch))
DeclWinFunc_CACHE(HWND, QueryFocus, (HWND hwndDesktop), (hwndDesktop))
DeclWinFunc_CACHE(BOOL, SetFocus, (HWND hwndDesktop, HWND hwndFocus),
		  (hwndDesktop, hwndFocus))
DeclWinFunc_CACHE(BOOL, ShowWindow, (HWND hwnd, BOOL fShow), (hwnd, fShow))
DeclWinFunc_CACHE(BOOL, EnableWindow, (HWND hwnd, BOOL fEnable),
		      (hwnd, fEnable))
DeclWinFunc_CACHE(BOOL, SetWindowPos,
		  (HWND hwnd, HWND hwndInsertBehind, LONG x, LONG y,
		   LONG cx, LONG cy, ULONG fl),
		  (hwnd, hwndInsertBehind, x, y, cx, cy, fl))
DeclWinFunc_CACHE(HENUM, BeginEnumWindows, (HWND hwnd), (hwnd))
DeclWinFunc_CACHE(BOOL, EndEnumWindows, (HENUM henum), (henum))
DeclWinFunc_CACHE(BOOL, EnableWindowUpdate, (HWND hwnd, BOOL fEnable),
		  (hwnd, fEnable))
DeclWinFunc_CACHE(BOOL, SetWindowBits,
		  (HWND hwnd, LONG index, ULONG flData, ULONG flMask),
		  (hwnd, index, flData, flMask))
DeclWinFunc_CACHE(BOOL, SetWindowPtr, (HWND hwnd, LONG index, PVOID p),
		  (hwnd, index, p))
DeclWinFunc_CACHE(BOOL, SetWindowULong, (HWND hwnd, LONG index, ULONG ul),
		  (hwnd, index, ul))
DeclWinFunc_CACHE(BOOL, SetWindowUShort, (HWND hwnd, LONG index, USHORT us),
		  (hwnd, index, us))
DeclWinFunc_CACHE(HWND, IsChild, (HWND hwnd, HWND hwndParent),
		  (hwnd, hwndParent))
DeclWinFunc_CACHE(HWND, WindowFromId, (HWND hwnd, ULONG id), (hwnd, id))
DeclWinFunc_CACHE(HWND, EnumDlgItem, (HWND hwndDlg, HWND hwnd, ULONG code),
		  (hwndDlg, hwnd, code))
DeclWinFunc_CACHE(HWND, QueryDesktopWindow, (HAB hab, HDC hdc), (hab, hdc));
DeclWinFunc_CACHE(BOOL, SetActiveWindow, (HWND hwndDesktop, HWND hwnd),
		  (hwndDesktop, hwnd));
DeclWinFunc_CACHE(BOOL, QueryActiveDesktopPathname, (PSZ pszPathName, ULONG ulSize),
		  (pszPathName, ulSize));
DeclWinFunc_CACHE(BOOL, InvalidateRect,
		  (HWND hwnd, /*RECTL*/ char *prcl, BOOL fIncludeChildren),
		  (hwnd, prcl, fIncludeChildren));
DeclWinFunc_CACHE(BOOL, CreateFrameControls,
		  (HWND hwndFrame, /*PFRAMECDATA*/ char* pfcdata, PCSZ pszTitle),
		  (hwndFrame, pfcdata, pszTitle));
DeclWinFunc_CACHE(BOOL, OpenClipbrd, (HAB hab), (hab));
DeclWinFunc_CACHE(BOOL, EmptyClipbrd, (HAB hab), (hab));
DeclWinFunc_CACHE(BOOL, CloseClipbrd, (HAB hab), (hab));
DeclWinFunc_CACHE(BOOL, QueryClipbrdFmtInfo, (HAB hab, ULONG fmt, PULONG prgfFmtInfo), (hab, fmt, prgfFmtInfo));
DeclWinFunc_CACHE(ULONG, QueryClipbrdData, (HAB hab, ULONG fmt), (hab, fmt));
DeclWinFunc_CACHE(HWND, SetClipbrdViewer, (HAB hab, HWND hwnd), (hab, hwnd));
DeclWinFunc_CACHE(HWND, SetClipbrdOwner, (HAB hab, HWND hwnd), (hab, hwnd));
DeclWinFunc_CACHE(ULONG, EnumClipbrdFmts, (HAB hab, ULONG fmt), (hab, fmt));
DeclWinFunc_CACHE(ATOM, AddAtom, (HATOMTBL hAtomTbl, PCSZ pszAtomName),
		  (hAtomTbl, pszAtomName));
DeclWinFunc_CACHE(ULONG, QueryAtomUsage, (HATOMTBL hAtomTbl, ATOM atom),
		  (hAtomTbl, atom));
DeclWinFunc_CACHE(ULONG, QueryAtomLength, (HATOMTBL hAtomTbl, ATOM atom),
		  (hAtomTbl, atom));
DeclWinFunc_CACHE(ULONG, QueryAtomName,
		  (HATOMTBL hAtomTbl, ATOM atom, PSZ pchBuffer, ULONG cchBufferMax),
		  (hAtomTbl, atom, pchBuffer, cchBufferMax));
DeclWinFunc_CACHE(HATOMTBL, QuerySystemAtomTable, (VOID), ());
DeclWinFunc_CACHE(HATOMTBL, CreateAtomTable, (ULONG initial, ULONG buckets),
		  (initial, buckets));
DeclWinFunc_CACHE(ULONG, MessageBox, (HWND hwndParent, HWND hwndOwner, PCSZ pszText, PCSZ pszCaption, ULONG idWindow, ULONG flStyle), (hwndParent, hwndOwner, pszText, pszCaption, idWindow, flStyle));
DeclWinFunc_CACHE(ULONG, MessageBox2,
		  (HWND hwndParent, HWND hwndOwner, PCSZ pszText,
		   PCSZ pszCaption, ULONG idWindow, PMB2INFO pmb2info),
		  (hwndParent, hwndOwner, pszText, pszCaption, idWindow, pmb2info));
DeclWinFunc_CACHE(HPOINTER, LoadPointer,
		  (HWND hwndDesktop, HMODULE hmod, ULONG idres),
		  (hwndDesktop, hmod, idres));
DeclWinFunc_CACHE(HPOINTER, QuerySysPointer,
		  (HWND hwndDesktop, LONG lId, BOOL fCopy),
		  (hwndDesktop, lId, fCopy));
DeclWinFunc_CACHE(BOOL, Alarm, (HWND hwndDesktop, ULONG rgfType), (hwndDesktop, rgfType));
DeclWinFunc_CACHE(BOOL, FlashWindow, (HWND hwndFrame, BOOL fFlash), (hwndFrame, fFlash));

#if 0		/* Need to have the entry points described in the parent */
DeclWinFunc_CACHE(BOOL, QueryClassInfo, (HAB hab, char* pszClassName, PCLASSINFO pClassInfo), (hab, pszClassName, pClassInfo));

#define _QueryClassInfo(hab, pszClassName, pClassInfo)	\
	QueryClassInfo(hab, pszClassName, (PCLASSINFO)pClassInfo)

#endif

/* These functions do not croak on error */
DeclWinFunc_CACHE_survive(BOOL, SetClipbrdData,
			  (HAB hab, ULONG ulData, ULONG fmt, ULONG rgfFmtInfo),
			  (hab, ulData, fmt, rgfFmtInfo));

#define get_InvalidateRect	InvalidateRect
#define get_CreateFrameControls	CreateFrameControls

/* These functions may return 0 on success; check $^E/Perl_rc on res==0: */
DeclWinFunc_CACHE_resetError(PVOID, QueryWindowPtr, (HWND hwnd, LONG index),
			     (hwnd, index))
DeclWinFunc_CACHE_resetError(ULONG, QueryWindowULong, (HWND hwnd, LONG index),
			     (hwnd, index))
DeclWinFunc_CACHE_resetError(SHORT, QueryWindowUShort, (HWND hwnd, LONG index),
			     (hwnd, index))
DeclWinFunc_CACHE_resetError(LONG,  QueryWindowTextLength, (HWND hwnd), (hwnd))
DeclWinFunc_CACHE_resetError(HWND,  QueryActiveWindow, (HWND hwnd), (hwnd))
DeclWinFunc_CACHE_resetError(BOOL, PostMsg,
			     (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2),
			     (hwnd, msg, mp1, mp2))
DeclWinFunc_CACHE_resetError(HWND, GetNextWindow, (HENUM henum), (henum))
DeclWinFunc_CACHE_resetError(BOOL, IsWindowEnabled, (HWND hwnd), (hwnd))
DeclWinFunc_CACHE_resetError(BOOL, IsWindowVisible, (HWND hwnd), (hwnd))
DeclWinFunc_CACHE_resetError(BOOL, IsWindowShowing, (HWND hwnd), (hwnd))
DeclWinFunc_CACHE_resetError(ATOM, FindAtom, (HATOMTBL hAtomTbl, PCSZ pszAtomName),
			     (hAtomTbl, pszAtomName));
DeclWinFunc_CACHE_resetError(ATOM, DeleteAtom, (HATOMTBL hAtomTbl, ATOM atom),
			     (hAtomTbl, atom));
DeclWinFunc_CACHE_resetError(HATOMTBL, DestroyAtomTable, (HATOMTBL hAtomTbl), (hAtomTbl));
DeclWinFunc_CACHE_resetError(HWND, QueryClipbrdViewer, (HAB hab), (hab));
DeclWinFunc_CACHE_resetError(HWND, QueryClipbrdOwner, (HAB hab), (hab));

#define _DeleteAtom		DeleteAtom
#define _DestroyAtomTable	DestroyAtomTable

/* No die()ing on error */
DeclWinFunc_CACHE_survive(BOOL, IsWindow, (HAB hab, HWND hwnd), (hab, hwnd))

/* These functions are called frow complicated wrappers: */
ULONG (*pWinQuerySwitchList) (HAB hab, PSWBLOCK pswblk, ULONG usDataLength);
ULONG (*pWinChangeSwitchEntry) (HSWITCH hsw, __const__ SWCNTRL *pswctl);
HWND (*pWinWindowFromPoint)(HWND hwnd, __const__ POINTL *pptl, BOOL fChildren);


/* These functions have different names/signatures than what is
   declared above */
#define QueryFocusWindow QueryFocus
#define FocusWindow_set(hwndFocus, hwndDesktop) SetFocus(hwndDesktop, hwndFocus)
#define WindowPos_set(hwnd, x, y, fl, cx, cy, hwndInsertBehind)	\
	SetWindowPos(hwnd, hwndInsertBehind, x, y, cx, cy, fl)
#define myWinQueryWindowPtr(hwnd, i)	((ULONG)QueryWindowPtr(hwnd, i))
#define _ClipbrdData_set SetClipbrdData
#define ClipbrdOwner_set SetClipbrdOwner
#define ClipbrdViewer_set SetClipbrdViewer

int
WindowText_set(HWND hwnd, char* text)
{
   return !CheckWinError(myWinSetWindowText(hwnd, text));
}

SV *
myQueryWindowText(HWND hwnd)
{
    LONG l = QueryWindowTextLength(hwnd), len;
    SV *sv;
    STRLEN n_a;

    if (l == 0) {
	if (Perl_rc)		/* Last error */
	    return &PL_sv_undef;
	return &PL_sv_no;
    }
    sv = newSVpvs("");
    SvGROW(sv, l + 1);
    len = QueryWindowText(hwnd, l + 1, SvPV_force(sv, n_a));
    if (len != l) {
	Safefree(sv);
	croak("WinQueryWindowText() uncompatible with WinQueryWindowTextLength()");
    }
    SvCUR_set(sv, l);
    return sv;
}

SWP
QueryWindowSWP_(HWND hwnd)
{
    SWP swp;

    if (!QueryWindowPos(hwnd, &swp))
	croak("WinQueryWindowPos() error");
    return swp;
}

SV *
QueryWindowSWP(HWND hwnd)
{
    SWP swp = QueryWindowSWP_(hwnd);

    return newSVpvn((char*)&swp, sizeof(swp));
}

SV *
myQueryClassName(HWND hwnd)
{
    SV *sv = newSVpvs("");
    STRLEN l = 46, len = 0, n_a;

    while (l + 1 >= len) {
	if (len)
	    len = 2*len + 10;		/* Grow quick */
	else
	    len = l + 2;
	SvGROW(sv, len);
	l = QueryClassName(hwnd, len, SvPV_force(sv, n_a));
    }
    SvCUR_set(sv, l);
    return sv;
}

HWND
WindowFromPoint(long x, long y, HWND hwnd, BOOL fChildren)
{
    POINTL ppl;

    ppl.x = x; ppl.y = y;
    if (!pWinWindowFromPoint)
	AssignFuncPByORD(pWinWindowFromPoint, ORD_WinWindowFromPoint);
    return SaveWinError(pWinWindowFromPoint(hwnd, &ppl, fChildren));
}

static HSWITCH
switch_of(HWND hwnd, PID pid)
{
	 HSWITCH hSwitch;    

	 if (!(_emx_env & 0x200)) 
	     croak("switch_entry not implemented on DOS"); /* not OS/2. */
	 if (CheckWinError(hSwitch = 
			   myWinQuerySwitchHandle(hwnd, pid)))
	     croak_with_os2error("WinQuerySwitchHandle");
	 return hSwitch;
}


static void
fill_swentry(SWENTRY *swentryp, HWND hwnd, PID pid)
{
	 int rc;
	 HSWITCH hSwitch = switch_of(hwnd, pid);

	 swentryp->hswitch = hSwitch;
	 if (CheckOSError(myWinQuerySwitchEntry(hSwitch, &swentryp->swctl)))
	     croak_with_os2error("WinQuerySwitchEntry");
}

static void
fill_swentry_default(SWENTRY *swentryp)
{
	fill_swentry(swentryp, NULLHANDLE, getpid());
}

static SV*
myWinQueryActiveDesktopPathname()
{
    SV *buf = newSVpv("",0);
    STRLEN n_a;

    SvGROW(buf, MAXPATHLEN);
    QueryActiveDesktopPathname(SvPV(buf,n_a), MAXPATHLEN);
    SvCUR_set(buf, strlen(SvPV(buf, n_a)));
    return buf;
}

SV *
myWinQueryAtomName(ATOM atom, HATOMTBL hAtomTbl)
{
  ULONG len = QueryAtomLength(hAtomTbl, atom);

  if (len) {			/* Probably always so... */
    SV *sv = newSVpvs("");
    STRLEN n_a;

    SvGROW(sv, len + 1);
    len = QueryAtomName(hAtomTbl, atom, SvPV(sv, n_a), len + 1);
    if (len) {			/* Probably always so... */
      SvCUR_set(sv, len);
      *SvEND(sv) = 0;
      return sv;
    }
    SvREFCNT_dec(sv);
  }
  return &PL_sv_undef;
}

#define myWinQueryClipbrdFmtInfo	QueryClipbrdFmtInfo

/* Put data into shared memory, then call SetClipbrdData */
void
ClipbrdData_set(SV *sv, int convert_nl, unsigned long fmt, unsigned long rgfFmtInfo, HAB hab)
{
    STRLEN len;
    char *buf;
    char *pByte = 0, *s, c;
    ULONG nls = 0, rc, handle;

    if (rgfFmtInfo & CFI_POINTER) {
      s = buf = SvPV_force(sv, len);
      if (convert_nl) {
	while ((c = *s++)) {
	    if (c == '\r' && *s == '\n')
		s++;
	    else if (c == '\n')
		nls++;
	}
      }

      if (CheckOSError(DosAllocSharedMem((PPVOID)&pByte, 0, len + nls + 1,
				       PAG_WRITE | PAG_COMMIT | OBJ_GIVEABLE | OBJ_GETTABLE)))
	croak_with_os2error("ClipbrdData_set: DosAllocSharedMem error");

      if (!nls)
	memcpy(pByte, buf, len + 1);
      else {
	char *t = pByte, *e = buf + len;

	while (buf < e) {
	    c = *t++ = *buf++;
	    if (c == '\n' && (t == pByte + 1 || t[-2] != '\r'))
		t[-1] = '\r', *t++ = '\n';
	}
      }
      handle = (ULONG)pByte;
    } else {
      handle = (ULONG)SvUV(sv);
    }

    if (!SetClipbrdData(hab, handle, fmt, rgfFmtInfo)) {
	if (fmt & CFI_POINTER)
	    DosFreeMem((PPVOID)&pByte);
	croak_with_os2error("ClipbrdData_set: WinSetClipbrdData error");
    }
}

ULONG
QueryMemoryRegionSize(ULONG addr, ULONG *flagp, ULONG len, I32 interrupt)
{
    ULONG l, f;				/* Modifiable copy */
    ULONG rc;

    do {
	l = len;
	rc = DosQueryMem((void *)addr, &l, &f);
    } while ( interrupt ? 0 : rc == ERROR_INTERRUPT );

    /* We assume this is not about addr */
/*
    if (rc == ERROR_INVALID_ADDRESS)
	return 0xFFFFFFFF;
*/
    os2cp_croak(rc,"QueryMemoryRegionSize");
    if (flagp)
	*flagp = f;
    return l;
}

static ULONG
default_fmtInfo(ULONG fmt)
{
   switch (fmt) {
     case CF_PALETTE:	/* Actually, fmtInfo not documented for palette... */
     case CF_BITMAP:
     case CF_METAFILE:
     case CF_DSPBITMAP:
     case CF_DSPMETAFILE:
	return CFI_HANDLE;
     default:
	return CFI_POINTER;
   }
}

#if 0

ULONG
myWinMessageBox(HWND hwndParent, HWND hwndOwner, PCSZ pszText, PCSZ pszCaption, ULONG idWindow, ULONG flStyle)
{
    ULONG rc = MessageBox(hwndParent, hwndOwner, pszText, pszCaption,
			  idWindow, flStyle);

    if (rc == MBID_ERROR)
	rc = 0;
    if (CheckWinError(rc))
	croak_with_os2error("MessageBox");
    return rc;
}

ULONG
myWinMessageBox2(HWND hwndParent, HWND hwndOwner, PCSZ pszText,
		   PCSZ pszCaption, ULONG idWindow, PMB2INFO pmb2info)
{
    ULONG rc = MessageBox2(hwndParent, hwndOwner, pszText, pszCaption, idWindow, pmb2info);

    if (rc == MBID_ERROR)
	rc = 0;
    if (CheckWinError(rc))
	croak_with_os2error("MessageBox2");
    return rc;
}
#endif

/* static ULONG (* APIENTRY16 pDosSmSetTitle)(ULONG, PSZ); */
ULONG _THUNK_FUNCTION(DosSmSetTitle)(ULONG, PSZ);

#if 0			/*  Does not work.  */
static ULONG (*pDosSmSetTitle)(ULONG, PSZ);

static void
sesmgr_title_set(char *s)
{
    SWENTRY swentry;
    static HMODULE hdosc = 0;
    BYTE buf[20];
    long rc;

    fill_swentry_default(&swentry);
    if (!pDosSmSetTitle || !hdosc) {
	if (CheckOSError(DosLoadModule(buf, sizeof buf, "sesmgr", &hdosc)))
	    croak("Cannot load SESMGR: no `%s'", buf);
	if (CheckOSError(DosQueryProcAddr(hdosc, 0, "DOSSMSETTITLE",
					  (PFN*)&pDosSmSetTitle)))
	    croak("Cannot load SESMGR.DOSSMSETTITLE, err=%ld", rc);
    }
/*     (pDosSmSetTitle)(swcntrl.idSession,s); */
    rc = ((USHORT)
          (_THUNK_PROLOG (2+4);
           _THUNK_SHORT (swcntrl.idSession);
           _THUNK_FLAT (s);
           _THUNK_CALLI (*pDosSmSetTitle)));
    if (CheckOSError(rc))
	warn("*DOSSMSETTITLE: err=%ld, ses=%ld, addr=%x, *paddr=%x", 
	     rc, swcntrl.idSession, &_THUNK_FUNCTION(DosSmSetTitle),
	     pDosSmSetTitle);
}

#else /* !0 */

static bool
sesmgr_title_set(char *s)
{
    SWENTRY swentry;
    long rc;

    fill_swentry_default(&swentry);
    rc = ((USHORT)
          (_THUNK_PROLOG (2+4);
           _THUNK_SHORT (swentry.swctl.idSession);
           _THUNK_FLAT (s);
           _THUNK_CALL (DosSmSetTitle)));
#if 0
    if (CheckOSError(rc))
	warn("DOSSMSETTITLE: err=%ld, ses=%ld, addr=%x", 
	     rc, swcntrl.idSession, _THUNK_FUNCTION(DosSmSetTitle));
#endif
    return !CheckOSError(rc);
}
#endif /* !0 */

#if 0			/*  Does not work.  */
USHORT _THUNK_FUNCTION(Win16SetTitle) ();

static void
set_title2(char *s)
{
    long rc;

    rc = ((USHORT)
          (_THUNK_PROLOG (4);
           _THUNK_FLAT (s);
           _THUNK_CALL (Win16SetTitle)));
    if (CheckWinError(rc))
	warn("Win16SetTitle: err=%ld", rc);
}
#endif

SV *
process_swentry(unsigned long pid, HWND hwnd)
{
    SWENTRY swentry;

    if (!(_emx_env & 0x200)) 
	     croak("process_swentry not implemented on DOS"); /* not OS/2. */
    fill_swentry(&swentry, hwnd, pid);
    return newSVpvn((char*)&swentry, sizeof(swentry));
}

SV *
swentries_list()
{
    int num, n = 0;
    STRLEN n_a;
    PSWBLOCK pswblk;
    SV *sv = newSVpvs("");

    if (!(_emx_env & 0x200)) 
	     croak("swentries_list not implemented on DOS"); /* not OS/2. */
    if (!pWinQuerySwitchList)
	AssignFuncPByORD(pWinQuerySwitchList, ORD_WinQuerySwitchList);
    num = pWinQuerySwitchList(0, NULL, 0);	/* HAB is not required */
    if (!num)
	croak("(Unknown) error during WinQuerySwitchList()");
    /* Allow one extra entry to allow overflow detection (may happen
	if the list has been changed). */
    while (num > n) {
	if (n == 0)
	    n = num + 1;
	else
	    n = 2*num + 10;			/* Enlarge quickly */
	SvGROW(sv, sizeof(ULONG) + sizeof(SWENTRY) * n + 1);
	pswblk = (PSWBLOCK) SvPV_force(sv, n_a);
	num = pWinQuerySwitchList(0, pswblk, SvLEN(sv));
    }
    SvCUR_set(sv, sizeof(ULONG) + sizeof(SWENTRY) * num);
    *SvEND(sv) = 0;
    return sv;
}

SWENTRY
swentry( char *title, HWND sw_hwnd, HWND icon_hwnd, HPROGRAM owner_phandle,
	 PID owner_pid, ULONG owner_sid, ULONG visible, ULONG nonswitchable,
	 ULONG jumpable, ULONG ptype, HSWITCH sw_entry)
{
  SWENTRY e;

  strncpy(e.swctl.szSwtitle, title, MAXNAMEL);
  e.swctl.szSwtitle[60] = 0;
  e.swctl.hwnd = sw_hwnd;
  e.swctl.hwndIcon = icon_hwnd;
  e.swctl.hprog = owner_phandle;
  e.swctl.idProcess = owner_pid;
  e.swctl.idSession = owner_sid;
  e.swctl.uchVisibility = ((visible ? SWL_VISIBLE : SWL_INVISIBLE)
			   | (nonswitchable ? SWL_GRAYED : 0));
  e.swctl.fbJump = (jumpable ? SWL_JUMPABLE : 0);
  e.swctl.bProgType = ptype;
  e.hswitch = sw_entry;
  return e;
}

SV *
create_swentry( char *title, HWND owner_hwnd, HWND icon_hwnd, HPROGRAM owner_phandle,
	 PID owner_pid, ULONG owner_sid, ULONG visible, ULONG nonswitchable,
	 ULONG jumpable, ULONG ptype, HSWITCH sw_entry)
{
    SWENTRY e = swentry(title, owner_hwnd, icon_hwnd, owner_phandle, owner_pid,
			owner_sid, visible, nonswitchable, jumpable, ptype,
			sw_entry);

    return newSVpvn((char*)&e, sizeof(e));
}

int
change_swentrysw(SWENTRY *sw)
{
    ULONG rc;			/* For CheckOSError */

    if (!(_emx_env & 0x200)) 
	     croak("change_entry() not implemented on DOS"); /* not OS/2. */
    if (!pWinChangeSwitchEntry)
	AssignFuncPByORD(pWinChangeSwitchEntry, ORD_WinChangeSwitchEntry);
    return !CheckOSError(pWinChangeSwitchEntry(sw->hswitch, &sw->swctl));
}

int
change_swentry(SV *sv)
{
    STRLEN l;
    PSWENTRY pswentry = (PSWENTRY)SvPV(sv, l);

    if (l != sizeof(SWENTRY))
	croak("Wrong structure size %ld!=%ld in change_swentry()", (long)l, (long)sizeof(SWENTRY));
    return change_swentrysw(pswentry);
}


#define swentry_size()		(sizeof(SWENTRY))

void
getscrsize(int *wp, int *hp)
{
    int i[2];

    _scrsize(i);
    *wp = i[0];
    *hp = i[1];
}

/* Force vio to not cross 64K-boundary: */
#define VIO_FROM_VIOB			\
    vio = viob;				\
    if (!_THUNK_PTR_STRUCT_OK(vio))	\
	vio++

bool
scrsize_set(int w, int h)
{
    VIOMODEINFO viob[2], *vio;
    ULONG rc;

    VIO_FROM_VIOB;

    if (h == -9999)
	h = w, w = 0;
    vio->cb = sizeof(*vio);
    if (CheckOSError(VioGetMode( vio, 0 )))
	return 0;

    if( w > 0 )
      vio->col = (USHORT)w;

    if( h > 0 )
      vio->row = (USHORT)h;

    vio->cb = 8;
    if (CheckOSError(VioSetMode( vio, 0 )))
	return 0;
    return 1;
}

void
cursor(int *sp, int *ep, int *wp, int *ap)
{
    VIOCURSORINFO viob[2], *vio;
    ULONG rc;

    VIO_FROM_VIOB;

    if (CheckOSError(VioGetCurType( vio, 0 )))
	croak_with_os2error("VioGetCurType() error");

    *sp = vio->yStart;
    *ep = vio->cEnd;
    *wp = vio->cx;
    *ep = vio->attr;
}

bool
cursor__(int is_a)
{
    int s,e,w,a;

    cursor(&s, &e, &w, &a);
    if (is_a)
	return a;
    else
	return w;
}

bool
cursor_set(int s, int e, int w, int a)
{
    VIOCURSORINFO viob[2], *vio;
    ULONG rc;

    VIO_FROM_VIOB;

    vio->yStart = s;
    vio->cEnd = e;
    vio->cx = w;
    vio->attr = a;
    return !CheckOSError(VioSetCurType( vio, 0 ));
}

static int
bufsize(void)
{
#if 1
    VIOMODEINFO viob[2], *vio;
    ULONG rc;

    VIO_FROM_VIOB;

    vio->cb = sizeof(*vio);
    if (CheckOSError(VioGetMode( vio, 0 )))
	croak_with_os2error("Can't get size of buffer for screen");
#if 0	/* buf=323552247, full=1118455, partial=0 */
    croak("Lengths: buf=%d, full=%d, partial=%d",vio->buf_length,vio->full_length,vio->partial_length);
    return newSVpvn((char*)vio->buf_addr, vio->full_length);
#endif
    return vio->col * vio->row * 2;	/* How to get bytes/cell?  2 or 4? */
#else	/* 0 */
    int i[2];

    _scrsize(i);
    return i[0]*i[1]*2;
#endif	/* 0 */
}

SV*
_kbdChar(unsigned int nowait, int handle)
{
    KBDKEYINFO viob[2], *vio;
    ULONG rc;

    VIO_FROM_VIOB;

    if (nowait > 2)
	croak("unexpected nowait");
    if (CheckOSError(nowait == 2
		     ? KbdPeek( vio, handle )
		     : KbdCharIn( vio, nowait == 1, handle )))
	croak_with_os2error("Can't _kbdChar");
    return newSVpvn((char*)vio, sizeof(*vio));
}

SV*
_kbdStatus(int handle)
{
    KBDINFO viob[2], *vio;
    ULONG rc;

    VIO_FROM_VIOB;

    vio->cb = sizeof(*vio);
    if (CheckOSError(KbdGetStatus( vio, handle )))
	croak_with_os2error("Can't _kbdStatus");
    return newSVpvn((char*)vio, sizeof(*vio));
}

void
_kbdStatus_set(SV* sv, int handle)
{
    KBDINFO viob[2], *vio;
    ULONG rc;
    STRLEN l;
    char *s = SvPV(sv, l);

    VIO_FROM_VIOB;

    if (l != sizeof(*vio))
	croak("unexpected datasize");
    Copy((KBDINFO*)s, vio, 1, KBDINFO);
    if (vio->cb != sizeof(*vio))
	croak("unexpected datasize");
    if (CheckOSError(KbdSetStatus( vio, handle )))
	croak_with_os2error("Can't kbdStatus_set()");
}

SV*
_vioConfig(int which, int handle)
{
    struct {VIOCONFIGINFO i; short a[20];} viob[2], *vio;
    ULONG rc;

    VIO_FROM_VIOB;

    vio->i.cb = 2;
    if (CheckOSError(VioGetConfig( which, &vio->i, handle )))
	croak_with_os2error("Can't get VIO config size");
    if (vio->i.cb > sizeof(*vio))
	vio->i.cb = sizeof(*vio);
    if (CheckOSError(VioGetConfig( which, &vio->i, handle )))
	croak_with_os2error("Can't get VIO config");
    return newSVpvn((char*)vio, vio->i.cb);
}

SV*
_vioMode(void)
{
    VIOMODEINFO viob[2], *vio;
    ULONG rc;

    VIO_FROM_VIOB;

    vio->cb = sizeof(*vio);
    if (CheckOSError(VioGetMode( vio, 0 )))
	croak_with_os2error("Can't get VIO mode");
    return newSVpvn((char*)vio, sizeof(*vio));
}

void
_vioMode_set(SV* sv)
{
    VIOMODEINFO viob[2], *vio;
    ULONG rc;
    STRLEN l;
    char *s = SvPV(sv, l);

    VIO_FROM_VIOB;

    Copy((VIOMODEINFO*)s, vio, 1, VIOMODEINFO);
    if (vio->cb != sizeof(*vio) || l != vio->cb)
	croak("unexpected datasize");
    if (CheckOSError(VioSetMode( vio, 0 )))
	croak_with_os2error("Can't set VIO mode");
}

SV*
vioFont(int type, int *w, int *h) /* 0 for actual RAM font, 1 for ROM font */
{
    VIOFONTINFO viob[2], *vio;
    ULONG rc;
    UCHAR b[1<<17];
    UCHAR *buf = b;
    SV *sv;

    VIO_FROM_VIOB;

    /* Should not cross 64K boundaries too: */
    if (((ULONG)buf) & 0xFFFF)
	buf += 0x10000 - (((ULONG)buf) & 0xFFFF);

    vio->cb = sizeof(*vio);
    vio->type = type;			/* BIOS or the loaded font. */
    vio->cbData = 0xFFFF;		/* How large is my buffer? */
    vio->pbData = _emx_32to16(buf);	/* Wants an 16:16 pointer */
    if (CheckOSError(VioGetFont( vio, 0 )))
	croak_with_os2error("Can't get VIO font");
    *w = vio->cxCell;
    *h = vio->cyCell;
    return newSVpvn(buf,vio->cbData);
}

void
vioFont_set(SV *sv, int cellwidth, int cellheight, int type)
{
    VIOFONTINFO viob[2], *vio;
    ULONG rc;
    UCHAR b[1<<17];
    UCHAR *buf = b;
    STRLEN l;
    char *s = SvPV(sv, l);

    VIO_FROM_VIOB;

    /* Should not cross 64K boundaries too: */
    if (((ULONG)buf) & 0xFFFF)
	buf += 0x10000 - (((ULONG)buf) & 0xFFFF);

    if (l > 0xFFFF)
	croak("length overflow of VIO font");
    if (l != (cellwidth + 7)/8 * cellheight * 256)
	warn("unexpected length of VIO font");
    vio->cb = sizeof(*vio);
    vio->type = type;			/* BIOS or the loaded font. */
    vio->cbData = l;			/* How large is my buffer? */
    vio->pbData = _emx_32to16(buf);	/* Wants an 16:16 pointer */
    vio->cxCell = cellwidth;
    vio->cyCell = cellheight;
    Copy(s, buf, l, char);

    if (CheckOSError(VioSetFont( vio, 0 )))
	croak_with_os2error("Can't set VIO font");
}

/*
  uses use32,os2def,os2base,crt,defs;
  var   Plt :Plt256;
  const Pal :VioPalState=(Cb:sizeof(VioPalState);rType:0;iFirst:0;
    Acolor:($FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF));
        CReg:VioColorReg=(Cb:sizeof(VioColorReg);rType:3;FirstColorReg:0;
    NumColorRegs:256; ColorRegAddr:@Plt);
  var   ii:Pointer;
  begin
   VioGetState(Pal,0);
   Pal.Acolor[09]:=$0F;
   Pal.Acolor[10]:=$A;
   Pal.Acolor[13]:=$2F;
   VioSetState(Pal,0); // ce smena EGA registrov
   asm
  lea   eax,Plt
     call  DosFlatToSel
     mov   ii,eax
   end;
   CReg.ColorRegAddr:=ii;
   VioGetState(CReg,0);
   Plt[10,0]:=$00;
   Plt[10,1]:=$32;
   Plt[10,2]:=$2A;
   VioSetState(CReg,0); // a ce - VGA registrov
  end.
*/

typedef union {
  VIOPALSTATE pal;
  struct { VIOPALSTATE pal; USHORT a[15]; } pal_padded;
  VIOOVERSCAN overscan;
  VIOINTENSITY intensity;
  VIOCOLORREG colorreg;
  struct { VIOCOLORREG reg; char rgb[3*256]; } colorreg_padded;
  VIOSETULINELOC lineloc;
  VIOSETTARGET target;
} my_VIOSTATE;

int
vio_state_size(int what)
{
    static const char sizes[] = {
	sizeof(VIOPALSTATE),
	sizeof(VIOOVERSCAN),
	sizeof(VIOINTENSITY),
	sizeof(VIOCOLORREG),
	6,				/* Random number: Reserved entry */
	sizeof(VIOSETULINELOC),
	sizeof(VIOSETTARGET)
    };
    if (what < 0 || what >= sizeof(sizes))
	croak("Unexpected VIO state type");
    return sizes[what];
}

SV*
_vioState(int what, int first, int count)
{
    my_VIOSTATE viob[2], *vio;
    ULONG rc, size = vio_state_size(what);

    VIO_FROM_VIOB;

    vio->pal.cb = size;
    vio->pal.type = what;
    if (what == 0) {
	vio->pal.iFirst = first;
	if (first < 0 || first >= 16)
	    croak("unexpected palette start value");
	if (count < 0 || count > 16)
	    croak("unexpected palette count");
	vio->pal.cb = (size += (count - 1) * sizeof(short));
    } else if (what == 3) {
	/* Wants an 16:16 pointer */
	if (count < 0 || count > 256)
	    croak("unexpected palette count");
	vio->colorreg.colorregaddr = (PCH)_emx_32to16(vio->colorreg_padded.rgb);
	vio->colorreg.numcolorregs = count;		/* 256 is max */
	vio->colorreg.firstcolorreg = first;
	size += 3 * count;
    }
    if (CheckOSError(VioGetState( (void*)vio, 0 )))
	croak_with_os2error("Can't get VIO state");
    return newSVpvn((char*)vio, size);
}

void
_vioState_set(SV *sv)
{
    my_VIOSTATE viob[2], *ovio = (my_VIOSTATE*)SvPV_nolen(sv), *vio = ovio;
    int what = ovio->pal.type, cb = ovio->pal.cb;
    ULONG rc, size = vio_state_size(what);
    STRLEN l;
    char *s = SvPV(sv, l);

    VIO_FROM_VIOB;

    switch (what) {
    case 0:
	if ( cb < size || cb > size + 15*sizeof(SHORT) || l != cb)
	    croak("unexpected datasize");
	size = l;
	break;
    case 3:
	if (l != cb + 3 * ovio->colorreg.numcolorregs || cb != size)
	    croak("unexpected datasize");
	size = l;
	break;
    default:
	if (l != cb || l != size )
	    croak("unexpected datasize");
	break;
    }
    Copy(s, (char*)vio, size, char);
    if (what == 3)	/* We expect colors put after VIOCOLORREG */
	vio->colorreg.colorregaddr = (PCH)_emx_32to16(vio->colorreg_padded.rgb);

    if (CheckOSError(VioSetState( (void*)vio, 0 )))
	croak_with_os2error("Can't set VIO state");
}

SV *
screen(void)
{
    ULONG rc;
    USHORT bufl = bufsize();
    char b[(1<<16) * 3]; /* This/3 is enough for 16-bit calls, we need
			    2x overhead due to 2 vs 4 issue, and extra
			    64K due to alignment logic */
    char *buf = b;
    
    if (((ULONG)buf) & 0xFFFF)
	buf += 0x10000 - (((ULONG)buf) & 0xFFFF);
    if ((sizeof(b) - (buf - b)) < 2*bufl)
	croak("panic: VIO buffer allocation");
    if (CheckOSError(VioReadCellStr( buf, &bufl, 0, 0, 0 )))
	return &PL_sv_undef;
    return newSVpvn(buf,bufl);
}

bool
screen_set(SV *sv)
{
    ULONG rc;
    STRLEN l = SvCUR(sv), bufl = bufsize();
    char b[(1<<16) * 2]; /* This/2 is enough for 16-bit calls, we need
			    extra 64K due to alignment logic */
    char *buf = b;
    
    if (((ULONG)buf) & 0xFFFF)
	buf += 0x10000 - (((ULONG)buf) & 0xFFFF);
    if (!SvPOK(sv) || ((l != bufl) && (l != 2*bufl)))
	croak("Wrong size %d of saved screen data", SvCUR(sv));
    if ((sizeof(b) - (buf - b)) < l)
	croak("panic: VIO buffer allocation");
    Copy(SvPV(sv,l), buf, bufl, char);
    if (CheckOSError(VioWrtCellStr( buf, bufl, 0, 0, 0 )))
	return 0;
    return 1;
}

int
process_codepages()
{
    ULONG cps[4], cp, rc;

    if (CheckOSError(DosQueryCp( sizeof(cps), cps, &cp )))
	croak_with_os2error("DosQueryCp()");
    return cp;
}

int
out_codepage()
{
    USHORT cp, rc;

    if (CheckOSError(VioGetCp( 0, &cp, 0 )))
	croak_with_os2error("VioGetCp()");
    return cp;
}

bool
out_codepage_set(int cp)
{
    USHORT rc;

    return !(CheckOSError(VioSetCp( 0, cp, 0 )));
}

int
in_codepage()
{
    USHORT cp, rc;

    if (CheckOSError(KbdGetCp( 0, &cp, 0 )))
	croak_with_os2error("KbdGetCp()");
    return cp;
}

bool
in_codepage_set(int cp)
{
    USHORT rc;

    return !(CheckOSError(KbdSetCp( 0, cp, 0 )));
}

bool
process_codepage_set(int cp)
{
    USHORT rc;

    return !(CheckOSError(DosSetProcessCp( cp )));
}

int
ppidOf(int pid)
{
  PQTOPLEVEL psi;
  int ppid;

  if (!pid)
      return -1;
  psi = get_sysinfo(pid, QSS_PROCESS);
  if (!psi)
      return -1;
  ppid = psi->procdata->ppid;
  Safefree(psi);
  return ppid;
}

int
sidOf(int pid)
{
  PQTOPLEVEL psi;
  int sid;

  if (!pid)
      return -1;
  psi = get_sysinfo(pid, QSS_PROCESS);
  if (!psi)
      return -1;
  sid = psi->procdata->sessid;
  Safefree(psi);
  return sid;
}

STRLEN
StrLen(ULONG addr, ULONG lim, I32 unitsize)
{
    switch (unitsize) {
      case 1:
	{
	    char *s = (char *)addr;
	    char *s1 = s, *e = (char *)(addr + lim);

	    while (s < e && *s)
		s++;
	    return s - s1;
	}
	break;
      case 2:
	{
	    short *s = (short *)addr;
	    short *s1 = s, *e = (short *)(addr + lim);

	    while (s < e && *s)
		s++;
	    return (char*)s - (char*)s1;
	}
	break;
      case 4:
	{
	    int *s = (int *)addr;
	    int *s1 = s, *e = (int *)(addr + lim);

	    while (s < e && *s)
		s++;
	    return (char*)s - (char*)s1;
	}
	break;
      case 8:
	{
	    long long *s = (long long *)addr;
	    long long *s1 = s, *e = (long long *)(addr + lim);

	    while (s < e && *s)
		s++;
	    return (char*)s - (char*)s1;
	}
	break;
      default:
	croak("StrLen: unknown unitsize %d", (int)unitsize);
    }
}

#define ulMPFROMSHORT(i)		((unsigned long)MPFROMSHORT(i))
#define ulMPVOID()			((unsigned long)MPVOID)
#define ulMPFROMCHAR(i)			((unsigned long)MPFROMCHAR(i))
#define ulMPFROM2SHORT(x1,x2)		((unsigned long)MPFROM2SHORT(x1,x2))
#define ulMPFROMSH2CH(s, c1, c2)	((unsigned long)MPFROMSH2CH(s, c1, c2))
#define ulMPFROMLONG(x)			((unsigned long)MPFROMLONG(x))

#define _MessageBox			MessageBox
#define _MessageBox2			MessageBox2

MODULE = OS2::Process		PACKAGE = OS2::Process

PROTOTYPES: ENABLE

unsigned long
constant(name,arg)
	char *		name
	int		arg

char *
my_type()

U32
file_type(path)
    char *path

SV *
swentry_expand( SV *sv )
    PPCODE:
     {
	 STRLEN l;
	 PSWENTRY pswentry = (PSWENTRY)SvPV(sv, l);

	 if (l != sizeof(SWENTRY))
		croak("Wrong structure size %ld!=%ld in swentry_expand()", (long)l, (long)sizeof(SWENTRY));
	 EXTEND(sp,11);
	 PUSHs(sv_2mortal(newSVpv(pswentry->swctl.szSwtitle, 0)));
	 PUSHs(sv_2mortal(newSVnv(pswentry->swctl.hwnd)));
	 PUSHs(sv_2mortal(newSVnv(pswentry->swctl.hwndIcon)));
	 PUSHs(sv_2mortal(newSViv(pswentry->swctl.hprog)));
	 PUSHs(sv_2mortal(newSViv(pswentry->swctl.idProcess)));
	 PUSHs(sv_2mortal(newSViv(pswentry->swctl.idSession)));
	 PUSHs(sv_2mortal(newSViv(pswentry->swctl.uchVisibility & SWL_VISIBLE)));
	 PUSHs(sv_2mortal(newSViv(pswentry->swctl.uchVisibility & SWL_GRAYED)));
	 PUSHs(sv_2mortal(newSViv(pswentry->swctl.fbJump == SWL_JUMPABLE)));
	 PUSHs(sv_2mortal(newSViv(pswentry->swctl.bProgType)));
	 PUSHs(sv_2mortal(newSViv(pswentry->hswitch)));
     }

SV *
create_swentry( char *title, unsigned long sw_hwnd, unsigned long icon_hwnd, unsigned long owner_phandle, unsigned long owner_pid, unsigned long owner_sid, unsigned long visible, unsigned long switchable,	 unsigned long jumpable, unsigned long ptype, unsigned long sw_entry)
PROTOTYPE: DISABLE

int
change_swentry( SV *sv )

bool
sesmgr_title_set(s)
    char *s

SV *
process_swentry(unsigned long pid = getpid(), HWND hwnd = NULLHANDLE);
  PROTOTYPE: DISABLE

int
swentry_size()

SV *
swentries_list()

void
ResetWinError()
   POSTCALL:
	XSRETURN_YES;

int
WindowText_set(HWND hwndFrame, char *title)

bool
FocusWindow_set(HWND hwndFocus, HWND hwndDesktop = HWND_DESKTOP)

bool
ShowWindow(HWND hwnd, bool fShow = TRUE)

bool
EnableWindow(HWND hwnd, bool fEnable = TRUE)

bool
PostMsg(HWND hwnd, unsigned long msg, unsigned long mp1 = 0, unsigned long mp2 = 0)
    C_ARGS: hwnd, msg, (MPARAM)mp1, (MPARAM)mp2

bool
WindowPos_set(HWND hwnd, long x, long y, unsigned long fl = SWP_MOVE, long cx = 0, long cy = 0, HWND hwndInsertBehind = HWND_TOP)
  PROTOTYPE: DISABLE

unsigned long
BeginEnumWindows(HWND hwnd)

bool
EndEnumWindows(unsigned long henum)

unsigned long
GetNextWindow(unsigned long henum)

bool
IsWindowVisible(HWND hwnd)

bool
IsWindowEnabled(HWND hwnd)

bool
IsWindowShowing(HWND hwnd)

unsigned long
QueryWindow(HWND hwnd, long cmd)

unsigned long
IsChild(HWND hwnd, HWND hwndParent)

unsigned long
WindowFromId(HWND hwndParent, unsigned long id)

unsigned long
WindowFromPoint(long x, long y, HWND hwnd = HWND_DESKTOP, bool fChildren = TRUE)
PROTOTYPE: DISABLE

unsigned long
EnumDlgItem(HWND hwndDlg, unsigned long code, HWND hwnd = NULLHANDLE)
   C_ARGS: hwndDlg, hwnd, code

bool
EnableWindowUpdate(HWND hwnd, bool fEnable = TRUE)

bool
SetWindowBits(HWND hwnd, long index, unsigned long flData, unsigned long flMask)

bool
SetWindowPtr(HWND hwnd, long index, unsigned long p)
    C_ARGS: hwnd, index, (PVOID)p

bool
SetWindowULong(HWND hwnd, long index, unsigned long i)

bool
SetWindowUShort(HWND hwnd, long index, unsigned short i)

bool
IsWindow(HWND hwnd, HAB hab = Acquire_hab())
    C_ARGS: hab, hwnd

BOOL
ActiveWindow_set(HWND hwnd, HWND hwndDesktop = HWND_DESKTOP)
    CODE:
	RETVAL = SetActiveWindow(hwndDesktop, hwnd);

unsigned long
LoadPointer(unsigned long idres, unsigned long hmod = 0, HWND hwndDesktop = HWND_DESKTOP)
    C_ARGS: hwndDesktop, hmod, idres

int
out_codepage()

bool
out_codepage_set(int cp)

int
in_codepage()

bool
in_codepage_set(int cp)

SV *
screen()

bool
screen_set(SV *sv)

SV *
process_codepages()
  PPCODE:
  {
    ULONG cps[4], c, i = 0, rc;

    if (CheckOSError(DosQueryCp( sizeof(cps), cps, &c )))
	c = 0;
    c /= sizeof(ULONG);
    if (c >= 3)
    EXTEND(sp, c);
    while (i < c)
	PUSHs(sv_2mortal(newSViv(cps[i++])));
  }

bool
process_codepage_set(int cp)

void
cursor(OUTLIST int stp, OUTLIST int ep, OUTLIST int wp, OUTLIST int ap)
  PROTOTYPE:

bool
cursor_set(int s, int e, int w = cursor__(0), int a = cursor__(1))

SV*
_kbdChar(int nowait = 0, int handle = 0)

SV*
_kbdStatus(int handle = 0)

void
_kbdStatus_set(SV *sv, int handle = 0)
   POSTCALL:
	XSRETURN_YES;

SV*
_vioConfig(int which = 0, int handle = 0)

SV*
_vioMode()

void
_vioMode_set(SV *buffer)
   POSTCALL:
	XSRETURN_YES;

SV*
_vioState(int what, int first = -1, int count = -1)

void
_vioState_set(SV *buffer)
   POSTCALL:
	XSRETURN_YES;

SV*
vioFont( int type = 0, OUTLIST int w, OUTLIST int h)

void
vioFont_set(SV *buffer, int cellwidth, int cellheight, int type = 0)
   POSTCALL:
	XSRETURN_YES;

NO_OUTPUT bool
_ClipbrdData_set(unsigned long ulData, unsigned long fmt = CF_TEXT, unsigned long rgfFmtInfo = default_fmtInfo(fmt), HAB hab = perl_hab_GET())
    PROTOTYPE: DISABLE
    C_ARGS: hab, ulData, fmt, rgfFmtInfo
    POSTCALL:
	if (CheckWinError(RETVAL))
	    croak_with_os2error("_ClipbrdData_set() error");
	XSRETURN_YES;

void
ClipbrdData_set(SV *text, int convert_nl = 1, unsigned long fmt = CF_TEXT, unsigned long rgfFmtInfo = default_fmtInfo(fmt), HAB hab = perl_hab_GET())
    PROTOTYPE: DISABLE
    POSTCALL:
	XSRETURN_YES;

void
ClipbrdOwner_set(HWND hwnd, HAB hab = perl_hab_GET())
    C_ARGS: hab, hwnd
    POSTCALL:
	XSRETURN_YES;

void
ClipbrdViewer_set(HWND hwnd, HAB hab = perl_hab_GET())
    C_ARGS: hab, hwnd
    POSTCALL:
	XSRETURN_YES;

unsigned long
EnumClipbrdFmts(unsigned long fmt = 0, HAB hab = perl_hab_GET())
    C_ARGS: hab, fmt

unsigned long
AddAtom(char *pszAtomName, HATOMTBL hAtomTbl = QuerySystemAtomTable())
    C_ARGS: hAtomTbl, pszAtomName

unsigned long
FindAtom(char *pszAtomName, HATOMTBL hAtomTbl = QuerySystemAtomTable())
    C_ARGS: hAtomTbl, pszAtomName

unsigned long
_DeleteAtom(ATOM atom, HATOMTBL hAtomTbl = QuerySystemAtomTable())
    PROTOTYPE: DISABLE
    C_ARGS: hAtomTbl, atom

#if 0

unsigned long
WinDeleteAtom(ATOM atom, HATOMTBL hAtomTbl = QuerySystemAtomTable())
    C_ARGS: hAtomTbl, atom

#endif

void
Alarm(unsigned long rgfType = WA_ERROR, HWND hwndDesktop = HWND_DESKTOP)
    C_ARGS: hwndDesktop, rgfType
    POSTCALL:
	XSRETURN_YES;

void
FlashWindow(HWND hwndFrame, bool fFlash)
    POSTCALL:
	XSRETURN_YES;

STRLEN
StrLen(ULONG addr, ULONG lim, I32 unitsize = 1)

MODULE = OS2::Process		PACKAGE = OS2::Process	PREFIX = myQuery

SV *
myQueryWindowText(HWND hwnd)

SV *
myQueryClassName(HWND hwnd)

MODULE = OS2::Process		PACKAGE = OS2::Process	PREFIX = Query

unsigned long
QueryFocusWindow(HWND hwndDesktop = HWND_DESKTOP)

long
QueryWindowTextLength(HWND hwnd)

SV *
QueryWindowSWP(HWND hwnd)

unsigned long
QueryWindowULong(HWND hwnd, long index)

unsigned short
QueryWindowUShort(HWND hwnd, long index)

unsigned long
QueryActiveWindow(HWND hwnd = HWND_DESKTOP)

unsigned long
QueryDesktopWindow(HAB hab = Acquire_hab(), unsigned long hdc = NULLHANDLE)

unsigned long
QueryClipbrdData(unsigned long fmt = CF_TEXT, HAB hab = perl_hab_GET())
    C_ARGS: hab, fmt
    PROTOTYPE: DISABLE

ULONG
QueryMemoryRegionSize(ULONG addr, OUTLIST ULONG flagp, ULONG len = 0xFFFFFFFF - addr, I32 interrupt = 1)

unsigned long
QueryClipbrdViewer(HAB hab = perl_hab_GET())

unsigned long
QueryClipbrdOwner(HAB hab = perl_hab_GET())

void
CloseClipbrd(HAB hab = perl_hab_GET())
    POSTCALL:
	XSRETURN_YES;

void
EmptyClipbrd(HAB hab = perl_hab_GET())
   POSTCALL:
	XSRETURN_YES;

bool
OpenClipbrd(HAB hab = perl_hab_GET())

unsigned long
QueryAtomUsage(ATOM atom, HATOMTBL hAtomTbl = QuerySystemAtomTable())
    C_ARGS: hAtomTbl, atom

unsigned long
QueryAtomLength(ATOM atom, HATOMTBL hAtomTbl = QuerySystemAtomTable())
    C_ARGS: hAtomTbl, atom
   POSTCALL:
	if (!RETVAL)
	    XSRETURN_EMPTY;

unsigned long
QuerySystemAtomTable()

unsigned long
QuerySysPointer(long lId, bool fCopy = 1, HWND hwndDesktop = HWND_DESKTOP)
    C_ARGS: hwndDesktop, lId, fCopy

unsigned long
CreateAtomTable(unsigned long initial = 0, unsigned long buckets = 0)

unsigned long
_DestroyAtomTable(HATOMTBL hAtomTbl)
    PROTOTYPE: DISABLE


MODULE = OS2::Process		PACKAGE = OS2::Process	PREFIX = myWinQuery

unsigned long
myWinQueryWindowPtr(HWND hwnd, long index)

NO_OUTPUT BOOL
myWinQueryWindowProcess(HWND hwnd, OUTLIST unsigned long pid, OUTLIST unsigned long tid)
   PROTOTYPE: $
   POSTCALL:
	if (CheckWinError(RETVAL))
	    croak_with_os2error("WindowProcess() error");

SV *
myWinQueryActiveDesktopPathname()

void
myWinQueryClipbrdFmtInfo(OUTLIST unsigned long prgfFmtInfo, unsigned long fmt = CF_TEXT, HAB hab = perl_hab_GET())
   C_ARGS: hab, fmt, &prgfFmtInfo

SV *
myWinQueryAtomName(ATOM atom, HATOMTBL hAtomTbl = QuerySystemAtomTable())

MODULE = OS2::Process		PACKAGE = OS2::Process	PREFIX = myWin

int
myWinSwitchToProgram(HSWITCH hsw = switch_of(NULLHANDLE, getpid()))
    PREINIT:
	ULONG rc;

#if 0

unsigned long
myWinMessageBox(unsigned long pszText, char* pszCaption = "Perl script message", unsigned long flStyle = MB_CANCEL | MB_ICONHAND, HWND hwndParent = HWND_DESKTOP, HWND hwndOwner = HWND_DESKTOP, unsigned long idWindow = 0)
    C_ARGS: hwndParent, hwndOwner, pszText, pszCaption, idWindow, flStyle

#endif

unsigned long
_MessageBox(char* pszText, char* pszCaption = "Perl script message", unsigned long flStyle = MB_CANCEL | MB_INFORMATION | MB_MOVEABLE, HWND hwndParent = HWND_DESKTOP, HWND hwndOwner = NULLHANDLE, unsigned long idWindow = 0)
    C_ARGS: hwndParent, hwndOwner, pszText, pszCaption, idWindow, flStyle
    POSTCALL:
	if (RETVAL == MBID_ERROR)
	    RETVAL = 0;

unsigned long
_MessageBox2(char *pszText, char* pmb2info, char *pszCaption = "Perl script message", HWND hwndParent = HWND_DESKTOP, HWND hwndOwner = NULLHANDLE, unsigned long idWindow = 0)
    C_ARGS: hwndParent, hwndOwner, pszText, pszCaption, idWindow, (PMB2INFO)pmb2info
    POSTCALL:
	if (RETVAL == MBID_ERROR)
	    RETVAL = 0;

MODULE = OS2::Process		PACKAGE = OS2::Process	PREFIX = myWinQuery

MODULE = OS2::Process		PACKAGE = OS2::Process	PREFIX = get

int
getppid()

int
ppidOf(int pid = getpid())

int
sidOf(int pid = getpid())

void
getscrsize(OUTLIST int wp, OUTLIST int hp)
  PROTOTYPE:

bool
scrsize_set(int w_or_h, int h = -9999)

void
get_InvalidateRect(HWND hwnd, char *prcl, bool fIncludeChildren)

void
get_CreateFrameControls(HWND hwndFrame, char *pfcdata, char* pszTitle)

MODULE = OS2::Process		PACKAGE = OS2::Process	PREFIX = ul

unsigned long
ulMPFROMSHORT(unsigned short i)

unsigned long
ulMPVOID()

unsigned long
ulMPFROMCHAR(unsigned char i)

unsigned long
ulMPFROM2SHORT(unsigned short x1, unsigned short x2)
  PROTOTYPE: DISABLE

unsigned long
ulMPFROMSH2CH(unsigned short s, unsigned char c1, unsigned char c2)
  PROTOTYPE: DISABLE

unsigned long
ulMPFROMLONG(unsigned long x)

