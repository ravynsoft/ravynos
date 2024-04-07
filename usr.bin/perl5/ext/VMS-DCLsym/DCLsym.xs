/* VMS::DCLsym - manipulate DCL symbols
 *
 * Version:  1.0
 * Author:   Charles Bailey  bailey@newman.upenn.edu
 * Revised:  17-Aug-1995
 *
 *
 * Revision History:
 * 
 * 1.0  17-Aug-1995  Charles Bailey  bailey@newman.upenn.edu
 *      original production version
 */

#include <descrip.h>
#include <lib$routines.h>
#include <libclidef.h>
#include <libdef.h>
#include <ssdef.h>
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

MODULE = VMS::DCLsym  PACKAGE = VMS::DCLsym

void
_getsym(name)
  SV *	name
  PPCODE:
  {
    struct dsc$descriptor_s namdsc = {0,DSC$K_DTYPE_T,DSC$K_CLASS_S,0},
                            valdsc = {0,DSC$K_DTYPE_T,DSC$K_CLASS_D,0};
    STRLEN namlen;
    int tbltype;
    unsigned long int retsts;
    SETERRNO(0,SS$_NORMAL);
    if (!name) {
      PUSHs(sv_newmortal());
      SETERRNO(EINVAL,LIB$_INVARG);
      return;
    }
    namdsc.dsc$a_pointer = SvPV(name,namlen);
    namdsc.dsc$w_length = (unsigned short int) namlen;
    retsts = lib$get_symbol(&namdsc,&valdsc,0,&tbltype);
    if (retsts & 1) {
      PUSHs(sv_2mortal(newSVpv(valdsc.dsc$w_length ? 
                               valdsc.dsc$a_pointer : "",valdsc.dsc$w_length)));
      EXTEND(sp,2);  /* just in case we're at the end of the stack */
      if (tbltype == LIB$K_CLI_LOCAL_SYM)
          PUSHs(sv_2mortal(newSVpv("LOCAL",5)));
      else
          PUSHs(sv_2mortal(newSVpv("GLOBAL",6)));
      _ckvmssts(lib$sfree1_dd(&valdsc));
    }
    else {
      /* error - we'll return an empty list */
      switch (retsts) {
        case LIB$_NOSUCHSYM:
          break;   /* nobody home */;
        case LIB$_INVSYMNAM:   /* user errors; set errno return undef */
        case LIB$_INSCLIMEM:
        case LIB$_NOCLI:
          set_errno(EVMSERR);
          set_vaxc_errno(retsts);
          break;
        default:  /* bail out */
          { _ckvmssts(retsts); }
      }
    }
  }


void
_setsym(name,val,typestr="LOCAL")
  SV *	name
  SV *	val
  char *	typestr
  CODE:
  {
    struct dsc$descriptor_s namdsc = {0,DSC$K_DTYPE_T,DSC$K_CLASS_S,0},
                            valdsc = {0,DSC$K_DTYPE_T,DSC$K_CLASS_S,0};
    STRLEN slen;
    int type;
    unsigned long int retsts;
    SETERRNO(0,SS$_NORMAL);
    if (!name || !val) {
      SETERRNO(EINVAL,LIB$_INVARG);
      XSRETURN_UNDEF;
    }
    namdsc.dsc$a_pointer = SvPV(name,slen);
    namdsc.dsc$w_length = (unsigned short int) slen;
    valdsc.dsc$a_pointer = SvPV(val,slen);
    valdsc.dsc$w_length = (unsigned short int) slen;
    type = strNE(typestr,"GLOBAL") ?
              LIB$K_CLI_LOCAL_SYM : LIB$K_CLI_GLOBAL_SYM;
    retsts = lib$set_symbol(&namdsc,&valdsc,&type);
    if (retsts & 1) { XSRETURN_YES; }
    else {
      switch (retsts) {
        case LIB$_AMBSYMDEF:  /* user errors; set errno and return */
        case LIB$_INSCLIMEM:
        case LIB$_INVSYMNAM:
        case LIB$_NOCLI:
          set_errno(EVMSERR);
          set_vaxc_errno(retsts);
          XSRETURN_NO;
          break;  /* NOTREACHED */
        default:  /* bail out */
          { _ckvmssts(retsts); }
      }
    }
  }


void
_delsym(name,typestr="LOCAL")
  SV *	name
  char *	typestr
  CODE:
  {
    struct dsc$descriptor_s namdsc = {0,DSC$K_DTYPE_T,DSC$K_CLASS_S,0};
    STRLEN slen;
    int type;
    unsigned long int retsts;
    SETERRNO(0,SS$_NORMAL);
    if (!name || !typestr) {
      SETERRNO(EINVAL,LIB$_INVARG);
      XSRETURN_UNDEF;
    }
    namdsc.dsc$a_pointer = SvPV(name,slen);
    namdsc.dsc$w_length = (unsigned short int) slen;
    type = strNE(typestr,"GLOBAL") ?
              LIB$K_CLI_LOCAL_SYM : LIB$K_CLI_GLOBAL_SYM;
    retsts = lib$delete_symbol(&namdsc,&type);
    if (retsts & 1) { XSRETURN_YES; }
    else {
      switch (retsts) {
        case LIB$_INVSYMNAM:  /* user errors; set errno and return */
        case LIB$_NOCLI:
        case LIB$_NOSUCHSYM:
          set_errno(EVMSERR);
          set_vaxc_errno(retsts);
          XSRETURN_NO;
          break;  /* NOTREACHED */
        default:  /* bail out */
          { _ckvmssts(retsts); }
      }
    }
  }

