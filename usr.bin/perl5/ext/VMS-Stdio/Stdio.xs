/* VMS::Stdio - VMS extensions to stdio routines 
 *
 * Author:   Charles Bailey  bailey@newman.upenn.edu
 *
 */

/* We now depend on handy.h macros that are not public API. */
#define PERL_EXT

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#include <file.h>
#include <iodef.h>
#include <rms.h>
#include <starlet.h>

static bool
constant(char *name, IV *pval)
{
    if (! strBEGINs(name, "O_")) return FALSE;

    if (strEQ(name, "O_APPEND"))
#ifdef O_APPEND
	{ *pval = O_APPEND; return TRUE; }
#else
	return FALSE;
#endif
    if (strEQ(name, "O_CREAT"))
#ifdef O_CREAT
	{ *pval = O_CREAT; return TRUE; }
#else
	return FALSE;
#endif
    if (strEQ(name, "O_EXCL"))
#ifdef O_EXCL
	{ *pval = O_EXCL; return TRUE; }
#else
	return FALSE;
#endif
    if (strEQ(name, "O_NDELAY"))
#ifdef O_NDELAY
	{ *pval = O_NDELAY; return TRUE; }
#else
	return FALSE;
#endif
    if (strEQ(name, "O_NOWAIT"))
#ifdef O_NOWAIT
	{ *pval = O_NOWAIT; return TRUE; }
#else
	return FALSE;
#endif
    if (strEQ(name, "O_RDONLY"))
#ifdef O_RDONLY
	{ *pval = O_RDONLY; return TRUE; }
#else
	return FALSE;
#endif
    if (strEQ(name, "O_RDWR"))
#ifdef O_RDWR
	{ *pval = O_RDWR; return TRUE; }
#else
	return FALSE;
#endif
    if (strEQ(name, "O_TRUNC"))
#ifdef O_TRUNC
	{ *pval = O_TRUNC; return TRUE; }
#else
	return FALSE;
#endif
    if (strEQ(name, "O_WRONLY"))
#ifdef O_WRONLY
	{ *pval = O_WRONLY; return TRUE; }
#else
	return FALSE;
#endif

    return FALSE;
}


static SV *
newFH(PerlIO *fp, char type) {
    SV *rv;
    GV **stashp, *gv = (GV *)newSV(0);
    HV *stash;
    IO *io;

    /* Find stash for VMS::Stdio.  We don't do this once at boot
     * to allow for possibility of threaded Perl with per-thread
     * symbol tables.  This code (through io = ...) is really
     * equivalent to gv_fetchpv("VMS::Stdio::__FH__",TRUE,SVt_PVIO),
     * with a little less overhead, and good exercise for me. :-) */
    stashp = (GV **)hv_fetch(PL_defstash,"VMS::",5,TRUE);
    if (!stashp || *stashp == (GV *)&PL_sv_undef) return NULL;
    if (!(stash = GvHV(*stashp))) stash = GvHV(*stashp) = newHV();
    stashp = (GV **)hv_fetch(GvHV(*stashp),"Stdio::",7,TRUE);
    if (!stashp || *stashp == (GV *)&PL_sv_undef) return NULL;
    if (!(stash = GvHV(*stashp))) stash = GvHV(*stashp) = newHV();

    /* Set up GV to point to IO, and then take reference */
    gv_init(gv,stash,"__FH__",6,0);
    io = GvIOp(gv) = newIO();
    IoIFP(io) = fp;
    if (type != '<') IoOFP(io) = fp;
    IoTYPE(io) = type;
    rv = newRV((SV *)gv);
    SvREFCNT_dec(gv);
    return sv_bless(rv,stash);
}

MODULE = VMS::Stdio  PACKAGE = VMS::Stdio

void
constant(name)
	char *	name
	PROTOTYPE: $
	CODE:
	IV i;
	if (constant(name, &i))
	    ST(0) = sv_2mortal(newSViv(i));
	else
	    ST(0) = &PL_sv_undef;

void
binmode(fh)
	SV *	fh
	PROTOTYPE: $
	CODE:
           SV *name;
	   IO *io;
	   char iotype;
	   char filespec[NAM$C_MAXRSS], *acmode, *s, *colon, *dirend = NULL;
	   int ret = 0, saverrno = errno, savevmserrno = vaxc$errno;
           SV pos;
           PerlIO *fp;
	   io = sv_2io(fh);
           fp = io ? IoOFP(io) : NULL;
	   iotype = io ? IoTYPE(io) : '\0';
	    if (fp == NULL || memCHRs(">was+-|",iotype) == NULL) {
	      set_errno(EBADF); set_vaxc_errno(SS$_IVCHAN); XSRETURN_UNDEF;
	    }
           if (!PerlIO_getname(fp,filespec)) XSRETURN_UNDEF;
	    for (s = filespec; *s; s++) {
	      if (*s == ':') colon = s;
	      else if (*s == ']' || *s == '>') dirend = s;
	    }
	    /* Looks like a tmpfile, which will go away if reopened */
	    if (s == dirend + 3) {
	      set_errno(EBADF); set_vaxc_errno(RMS$_IOP); XSRETURN_UNDEF;
	    }
	    /* If we've got a non-file-structured device, clip off the trailing
	     * junk, and don't lose sleep if we can't get a stream position.  */
	    if (dirend == NULL) *(colon+1) = '\0'; 
           if (iotype != '-' && (ret = PerlIO_getpos(fp, &pos)) == -1 && dirend)
	      XSRETURN_UNDEF;
	    switch (iotype) {
	      case '<': case 'r':           acmode = "rb";                      break;
	      case '>': case 'w': case '|':
	        /* use 'a' instead of 'w' to avoid creating new file;
	           fsetpos below will take care of restoring file position */
	      case 'a':                     acmode = "ab";                      break;
	      case '+':  case 's':          acmode = "rb+";                     break;
             case '-':                     acmode = PerlIO_fileno(fp) ? "ab" : "rb";  break;
	      /* iotype'll be null for the SYS$INPUT:/SYS$OUTPUT:/SYS$ERROR: files */
	      /* since we didn't really open them and can't really */
	      /* reopen them */
	      case 0:                       XSRETURN_UNDEF;
	      default:
	        if (PL_dowarn) warn("Unrecognized iotype %c for %s in binmode",
	                         iotype, filespec);
	        acmode = "rb+";
	    }
           /* appearances to the contrary, this is an freopen substitute */
           name = sv_2mortal(newSVpvn(filespec,strlen(filespec)));
           if (PerlIO_openn(aTHX_ NULL,acmode,-1,0,0,fp,1,&name) == NULL) XSRETURN_UNDEF;
           if (iotype != '-' && ret != -1 && PerlIO_setpos(fp,&pos) == -1) XSRETURN_UNDEF;
	    if (ret == -1) { set_errno(saverrno); set_vaxc_errno(savevmserrno); }
	    XSRETURN_YES;


void
flush(fp)
       PerlIO * fp
	PROTOTYPE: $
	CODE:
           FILE *stdio = PerlIO_exportFILE(fp,0);
           if (fflush(stdio)) { ST(0) = &PL_sv_undef; }
           else            { clearerr(stdio); ST(0) = &PL_sv_yes; }
           PerlIO_releaseFILE(fp,stdio);

char *
getname(fp)
	PerlIO * fp
	PROTOTYPE: $
	CODE:
            FILE *stdio = PerlIO_exportFILE(fp,0);
	    char fname[NAM$C_MAXRSS+1];
	    ST(0) = sv_newmortal();
            if (fgetname(stdio,fname) != NULL) sv_setpv(ST(0),fname);
            PerlIO_releaseFILE(fp,stdio);

void
rewind(fp)
       PerlIO * fp
	PROTOTYPE: $
	CODE:
           FILE *stdio = PerlIO_exportFILE(fp,0);
           ST(0) = rewind(stdio) ? &PL_sv_undef : &PL_sv_yes;
           PerlIO_releaseFILE(fp,stdio);

void
remove(name)
	char *name
	PROTOTYPE: $
	CODE:
	    ST(0) = remove(name) ? &PL_sv_undef : &PL_sv_yes;

void
setdef(...)
	PROTOTYPE: @
	CODE:
	    char vmsdef[NAM$C_MAXRSS+1], es[NAM$C_MAXRSS], sep;
	    unsigned long int retsts;
	    struct FAB deffab = cc$rms_fab;
	    struct NAM defnam = cc$rms_nam;
	    struct dsc$descriptor_s dirdsc = {0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0};
	    STRLEN n_a;
	    if (items) {
		SV *defsv = ST(items-1);  /* mimic chdir() */
		ST(0) = &PL_sv_undef;
		if (!SvPOK(defsv)) { SETERRNO(EINVAL,LIB$_INVARG); XSRETURN(1); }
		if (tovmsspec(SvPV(defsv,n_a),vmsdef) == NULL) { XSRETURN(1); }
		deffab.fab$l_fna = vmsdef; deffab.fab$b_fns = strlen(vmsdef);
	    }
	    else {
		deffab.fab$l_fna = "SYS$LOGIN"; deffab.fab$b_fns = 9;
		EXTEND(sp,1);  ST(0) = &PL_sv_undef;
	    }
	    defnam.nam$l_esa = es;  defnam.nam$b_ess = sizeof es;
	    deffab.fab$l_nam = &defnam;
	    retsts = sys$parse(&deffab,0,0);
	    if (retsts & 1) {
		if (defnam.nam$v_wildcard) retsts = RMS$_WLD;
		else if (defnam.nam$b_name || defnam.nam$b_type > 1 ||
	             defnam.nam$b_ver > 1) retsts = RMS$_DIR;
		}
	    defnam.nam$b_nop |= NAM$M_SYNCHK; defnam.nam$l_rlf = NULL; deffab.fab$b_dns = 0;
	    if (!(retsts & 1)) {
		set_vaxc_errno(retsts);
		switch (retsts) {
		    case RMS$_DNF:
			set_errno(ENOENT); break;
		    case RMS$_SYN: case RMS$_DIR: case RMS$_DEV:
			set_errno(EINVAL); break;
		    case RMS$_PRV:
			set_errno(EACCES); break;
		    default:
			set_errno(EVMSERR); break;
		}
		(void) sys$parse(&deffab,0,0);  /* free up context */
		XSRETURN(1);
	    }
	    sep = *defnam.nam$l_dir;
	    *defnam.nam$l_dir = '\0';
	    my_setenv("SYS$DISK",defnam.nam$b_node ? defnam.nam$l_node : defnam.nam$l_dev);
	    *defnam.nam$l_dir = sep;
	    dirdsc.dsc$a_pointer = defnam.nam$l_dir; dirdsc.dsc$w_length = defnam.nam$b_dir;
	    if ((retsts = sys$setddir(&dirdsc,0,0)) & 1) ST(0) = &PL_sv_yes;
	    else { set_errno(EVMSERR); set_vaxc_errno(retsts); }
	    (void) sys$parse(&deffab,0,0);  /* free up context */

void
sync(fp)
       PerlIO * fp
	PROTOTYPE: $
	CODE:
           FILE *stdio = PerlIO_exportFILE(fp,0);
           if (fsync(fileno(stdio))) { ST(0) = &PL_sv_undef; }
           else                   { clearerr(stdio); ST(0) = &PL_sv_yes; }
           PerlIO_releaseFILE(fp,stdio);

char *
tmpnam()
	PROTOTYPE:
	CODE:
	    char fname[L_tmpnam];
	    ST(0) = sv_newmortal();
	    if (tmpnam(fname) != NULL) sv_setpv(ST(0),fname);

void
vmsopen(spec,...)
	char *	spec
	PROTOTYPE: @
	CODE:
	    char *args[8],mode[3] = {'r','\0','\0'}, type = '<';
	    int i, myargc;
	    FILE *fp;
            SV *fh;
           PerlIO *pio_fp;
	    STRLEN n_a;
	
	    if (!spec || !*spec) {
	       SETERRNO(EINVAL,LIB$_INVARG);
	       XSRETURN_UNDEF;
	    }
	    if (items > 9) croak("too many args");
	
	    /* First, set up name and mode args from perl's string */
	    if (*spec == '+') {
	      mode[1] = '+';
	      spec++;
	    }
	    if (*spec == '>') {
	      if (*(spec+1) == '>') *mode = 'a', spec += 2;
	      else *mode = 'w',  spec++;
	    }
	    else if (*spec == '<') spec++;
	    myargc = items - 1;
	    for (i = 0; i < myargc; i++) args[i] = SvPV(ST(i+1),n_a);
	    /* This hack brought to you by C's opaque arglist management */
	    switch (myargc) {
	      case 0:
	        fp = fopen(spec,mode);
	        break;
	      case 1:
	        fp = fopen(spec,mode,args[0]);
	        break;
	      case 2:
	        fp = fopen(spec,mode,args[0],args[1]);
	        break;
	      case 3:
	        fp = fopen(spec,mode,args[0],args[1],args[2]);
	        break;
	      case 4:
	        fp = fopen(spec,mode,args[0],args[1],args[2],args[3]);
	        break;
	      case 5:
	        fp = fopen(spec,mode,args[0],args[1],args[2],args[3],args[4]);
	        break;
	      case 6:
	        fp = fopen(spec,mode,args[0],args[1],args[2],args[3],args[4],args[5]);
	        break;
	      case 7:
	        fp = fopen(spec,mode,args[0],args[1],args[2],args[3],args[4],args[5],args[6]);
	        break;
	      case 8:
	        fp = fopen(spec,mode,args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7]);
	        break;
	    }
	    if (fp != NULL) {
             pio_fp = PerlIO_fdopen(fileno(fp),mode);
             fh = newFH(pio_fp,(mode[1] ? '+' : (mode[0] == 'r' ? '<' : (mode[0] == 'a' ? 'a' : '>'))));
	     ST(0) = (fh ? sv_2mortal(fh) : &PL_sv_undef);
	    }
	    else { ST(0) = &PL_sv_undef; }

void
vmssysopen(spec,mode,perm,...)
	char *	spec
	int	mode
	int	perm
	PROTOTYPE: @
	CODE:
	    char *args[8];
	    int i, myargc, fd;
	    PerlIO *pio_fp;
	    SV *fh;
	    STRLEN n_a;
	    if (!spec || !*spec) {
	       SETERRNO(EINVAL,LIB$_INVARG);
	       XSRETURN_UNDEF;
	    }
	    if (items > 11) croak("too many args");
	    myargc = items - 3;
	    for (i = 0; i < myargc; i++) args[i] = SvPV(ST(i+3),n_a);
	    /* More fun with C calls; can't combine with above because
	       args 2,3 of different types in fopen() and open() */
	    switch (myargc) {
	      case 0:
	        fd = open(spec,mode,perm);
	        break;
	      case 1:
	        fd = open(spec,mode,perm,args[0]);
	        break;
	      case 2:
	        fd = open(spec,mode,perm,args[0],args[1]);
	        break;
	      case 3:
	        fd = open(spec,mode,perm,args[0],args[1],args[2]);
	        break;
	      case 4:
	        fd = open(spec,mode,perm,args[0],args[1],args[2],args[3]);
	        break;
	      case 5:
	        fd = open(spec,mode,perm,args[0],args[1],args[2],args[3],args[4]);
	        break;
	      case 6:
	        fd = open(spec,mode,perm,args[0],args[1],args[2],args[3],args[4],args[5]);
	        break;
	      case 7:
	        fd = open(spec,mode,perm,args[0],args[1],args[2],args[3],args[4],args[5],args[6]);
	        break;
	      case 8:
	        fd = open(spec,mode,perm,args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7]);
	        break;
	    }
	    i = mode & 3;
	    if (fd >= 0 &&
              ((pio_fp = PerlIO_fdopen(fd, &("r\000w\000r+"[2*i]))) != NULL)) {
             fh = newFH(pio_fp,"<>++"[i]);
	     ST(0) = (fh ? sv_2mortal(fh) : &PL_sv_undef);
	    }
	    else { ST(0) = &PL_sv_undef; }

void
waitfh(fp)
       PerlIO * fp
	PROTOTYPE: $
	CODE:
           FILE *stdio = PerlIO_exportFILE(fp,0);
           ST(0) = fwait(stdio) ? &PL_sv_undef : &PL_sv_yes;
           PerlIO_releaseFILE(fp,stdio);

void
writeof(mysv)
	SV *	mysv
	PROTOTYPE: $
	CODE:
	    char devnam[257], *cp;
	    unsigned long int chan, iosb[2], retsts, retsts2;
	    struct dsc$descriptor devdsc = {0, DSC$K_DTYPE_T, DSC$K_CLASS_S, devnam};
	    IO *io = sv_2io(mysv);
           PerlIO *fp = io ? IoOFP(io) : NULL;
	    if (fp == NULL || memCHRs(">was+-|",IoTYPE(io)) == NULL) {
	      set_errno(EBADF); set_vaxc_errno(SS$_IVCHAN); XSRETURN_UNDEF;
	    }
           if (PerlIO_getname(fp,devnam) == NULL) { ST(0) = &PL_sv_undef; XSRETURN(1); }
	    if ((cp = strrchr(devnam,':')) != NULL) *(cp+1) = '\0';
	    devdsc.dsc$w_length = strlen(devnam);
	    retsts = sys$assign(&devdsc,&chan,0,0);
	    if (retsts & 1) retsts = sys$qiow(0,chan,IO$_WRITEOF,iosb,0,0,0,0,0,0,0,0);
	    if (retsts & 1) retsts = iosb[0];
	    retsts2 = sys$dassgn(chan);  /* Be sure to deassign the channel */
	    if (retsts & 1) retsts = retsts2;
	    if (retsts & 1) { ST(0) = &PL_sv_yes; }
	    else {
	      set_vaxc_errno(retsts);
	      switch (retsts) {
	        case SS$_EXQUOTA:  case SS$_INSFMEM:  case SS$_MBFULL:
	        case SS$_MBTOOSML: case SS$_NOIOCHAN: case SS$_NOLINKS:
	        case SS$_BUFFEROVF:
	          set_errno(ENOSPC); break;
	        case SS$_ILLIOFUNC: case SS$_DEVOFFLINE: case SS$_NOSUCHDEV:
	          set_errno(EBADF);  break;
	        case SS$_NOPRIV:
	          set_errno(EACCES); break;
	        default:  /* Includes "shouldn't happen" cases that might map */
	          set_errno(EVMSERR); break;         /* to other errno values */
	      }
	      ST(0) = &PL_sv_undef;
	    }
