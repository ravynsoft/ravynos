/* This file is part of the "version" CPAN distribution.  Please avoid
   editing it in the perl core. */

#ifdef PERL_CORE
#  define VXS_CLASS "version"
#  define VXSp(name) XS_##name
/* VXSXSDP = XSUB Details Proto */
#  define VXSXSDP(x) x, 0
#else
#  define VXS_CLASS "version::vxs"
#  define VXSp(name) VXS_##name
/* proto member is unused in version, it is used in CORE by non version xsubs */
#  define VXSXSDP(x)
#endif

#ifndef XS_INTERNAL
#  define XS_INTERNAL(name) static XSPROTO(name)
#endif

#define VXS(name) XS_INTERNAL(VXSp(name)); XS_INTERNAL(VXSp(name))

/* uses PUSHs, so SP must be at start, PUSHs sv on Perl stack, then returns from
   xsub; this is a little more machine code/tailcall friendly than mPUSHs(foo);
   PUTBACK; return; */

#define VXS_RETURN_M_SV(sv) \
    STMT_START {							\
	SV * sv_vtc = sv;						\
	PUSHs(sv_vtc);							\
	PUTBACK;							\
	sv_2mortal(sv_vtc);						\
	return;								\
    } STMT_END


#ifdef VXS_XSUB_DETAILS
#  ifdef PERL_CORE
    {"UNIVERSAL::VERSION", VXSp(universal_version), VXSXSDP(NULL)},
#  endif
    {VXS_CLASS "::_VERSION", VXSp(universal_version), VXSXSDP(NULL)},
    {VXS_CLASS "::()", VXSp(version_noop), VXSXSDP(NULL)},
    {VXS_CLASS "::new", VXSp(version_new), VXSXSDP(NULL)},
    {VXS_CLASS "::parse", VXSp(version_new), VXSXSDP(NULL)},
    {VXS_CLASS "::(\"\"", VXSp(version_stringify), VXSXSDP(NULL)},
    {VXS_CLASS "::stringify", VXSp(version_stringify), VXSXSDP(NULL)},
    {VXS_CLASS "::(0+", VXSp(version_numify), VXSXSDP(NULL)},
    {VXS_CLASS "::numify", VXSp(version_numify), VXSXSDP(NULL)},
    {VXS_CLASS "::normal", VXSp(version_normal), VXSXSDP(NULL)},
    {VXS_CLASS "::(cmp", VXSp(version_vcmp), VXSXSDP(NULL)},
    {VXS_CLASS "::(<=>", VXSp(version_vcmp), VXSXSDP(NULL)},
#  ifdef PERL_CORE
    {VXS_CLASS "::vcmp", XS_version_vcmp, VXSXSDP(NULL)},
#  else
    {VXS_CLASS "::VCMP", VXS_version_vcmp, VXSXSDP(NULL)},
#  endif
    {VXS_CLASS "::(bool", VXSp(version_boolean), VXSXSDP(NULL)},
    {VXS_CLASS "::boolean", VXSp(version_boolean), VXSXSDP(NULL)},
    {VXS_CLASS "::(+", VXSp(version_noop), VXSXSDP(NULL)},
    {VXS_CLASS "::(-", VXSp(version_noop), VXSXSDP(NULL)},
    {VXS_CLASS "::(*", VXSp(version_noop), VXSXSDP(NULL)},
    {VXS_CLASS "::(/", VXSp(version_noop), VXSXSDP(NULL)},
    {VXS_CLASS "::(+=", VXSp(version_noop), VXSXSDP(NULL)},
    {VXS_CLASS "::(-=", VXSp(version_noop), VXSXSDP(NULL)},
    {VXS_CLASS "::(*=", VXSp(version_noop), VXSXSDP(NULL)},
    {VXS_CLASS "::(/=", VXSp(version_noop), VXSXSDP(NULL)},
    {VXS_CLASS "::(abs", VXSp(version_noop), VXSXSDP(NULL)},
    {VXS_CLASS "::(nomethod", VXSp(version_noop), VXSXSDP(NULL)},
    {VXS_CLASS "::noop", VXSp(version_noop), VXSXSDP(NULL)},
    {VXS_CLASS "::is_alpha", VXSp(version_is_alpha), VXSXSDP(NULL)},
    {VXS_CLASS "::qv", VXSp(version_qv), VXSXSDP(NULL)},
    {VXS_CLASS "::declare", VXSp(version_qv), VXSXSDP(NULL)},
    {VXS_CLASS "::is_qv", VXSp(version_is_qv), VXSXSDP(NULL)},
#else

#ifndef dVAR
#  define dVAR
#endif

#ifdef HvNAME_HEK
typedef HEK HVNAME;
#  ifndef HEKf
#    define HEKfARG(arg)	((void*)(sv_2mortal(newSVhek(arg))))
#    define HEKf		SVf
#  endif
#else
typedef char HVNAME;
#  define HvNAME_HEK	HvNAME_get
#  define HEKfARG(arg)	arg
#  define HEKf		"s"
#endif

VXS(universal_version)
{
    dXSARGS;
    HV *pkg;
    GV **gvp;
    GV *gv;
    SV *sv;
    const char *undef;
    PERL_UNUSED_ARG(cv);

    if (items < 1)
       Perl_croak(aTHX_ "Usage: UNIVERSAL::VERSION(sv, ...)");

    sv = ST(0);

    if (SvROK(sv)) {
        sv = (SV*)SvRV(sv);
        if (!SvOBJECT(sv))
            Perl_croak(aTHX_ "Cannot find version of an unblessed reference");
        pkg = SvSTASH(sv);
    }
    else {
        pkg = gv_stashsv(sv, FALSE);
    }

    gvp = pkg ? (GV**)hv_fetchs(pkg,"VERSION",FALSE) : (GV**)NULL;

    if (gvp && isGV(gv = *gvp) && (sv = GvSV(gv)) && SvOK(sv)) {
        sv = sv_mortalcopy(sv);
	if ( ! ISA_VERSION_OBJ(sv) )
	    UPG_VERSION(sv, FALSE);
        undef = NULL;
    }
    else {
        sv = &PL_sv_undef;
        undef = "(undef)";
    }

    if (items > 1) {
	SV *req = ST(1);

	if (undef) {
	    if (pkg) {
		const HVNAME* const name = HvNAME_HEK(pkg);
		Perl_croak(aTHX_
			   "%" HEKf " does not define $%" HEKf
			   "::VERSION--version check failed",
			   HEKfARG(name), HEKfARG(name));
	    }
	    else {
#if PERL_VERSION_GE(5,8,0)
		Perl_croak(aTHX_
			     "%" SVf " defines neither package nor VERSION--"
                             "version check failed",
			     (void*)(ST(0)) );
#else
		Perl_croak(aTHX_ "%s does not define $%s::VERSION--version check failed",
			   SvPVx_nolen_const(ST(0)),
			   SvPVx_nolen_const(ST(0)) );
#endif
	    }
	}

	if ( ! ISA_VERSION_OBJ(req) ) {
	    /* req may very well be R/O, so create a new object */
	    req = sv_2mortal( NEW_VERSION(req) );
	}

	if ( VCMP( req, sv ) > 0 ) {
	    if ( hv_exists(MUTABLE_HV(SvRV(req)), "qv", 2 ) ) {
		req = VNORMAL(req);
		sv  = VNORMAL(sv);
	    }
	    else {
		req = VSTRINGIFY(req);
		sv  = VSTRINGIFY(sv);
	    }
	    Perl_croak(aTHX_ "%" HEKf " version %" SVf " required--"
		"this is only version %" SVf, HEKfARG(HvNAME_HEK(pkg)),
		SVfARG(sv_2mortal(req)),
		SVfARG(sv_2mortal(sv)));
	}
    }

    /* if the package's $VERSION is not undef, it is upgraded to be a version object */
    if (ISA_VERSION_OBJ(sv)) {
	ST(0) = sv_2mortal(VSTRINGIFY(sv));
    } else {
	ST(0) = sv;
    }

    XSRETURN(1);
}

VXS(version_new)
{
    dXSARGS;
    SV *vs;
    SV *rv;
    const char * classname = "";
    STRLEN len;
    U32 flags = 0;
    SV * svarg0 = NULL;
    PERL_UNUSED_VAR(cv);

    SP -= items;

    switch((U32)items) {
    case 3: {
        SV * svarg2;
        vs = sv_newmortal();
        svarg2 = ST(2);
        Perl_sv_setpvf(aTHX_ vs,"v%s",SvPV_nolen_const(svarg2));
        break;
    }
    case 2:
        vs = ST(1);
    /* Just in case this is something like a tied hash */
        SvGETMAGIC(vs);
        if(SvOK(vs))
            break;
        /* fall through */
    case 1:
        /* no param or explicit undef */
        /* create empty object */
        vs = sv_newmortal();
        sv_setpvs(vs,"undef");
        break;
    default:
    case 0:
        Perl_croak_nocontext("Usage: version::new(class, version)");
    }

    svarg0 = ST(0);
    if ( sv_isobject(svarg0) ) {
	/* get the class if called as an object method */
	const HV * stash = SvSTASH(SvRV(svarg0));
	classname = HvNAME_get(stash);
	len	  = HvNAMELEN_get(stash);
#ifdef HvNAMEUTF8
	flags	  = HvNAMEUTF8(stash) ? SVf_UTF8 : 0;
#endif
    }
    else {
	classname = SvPV_nomg(svarg0, len);
	flags     = SvUTF8(svarg0);
    }

    rv = NEW_VERSION(vs);
    if ( len != sizeof(VXS_CLASS)-1
      || strcmp(classname,VXS_CLASS) != 0 ) /* inherited new() */
        sv_bless(rv, gv_stashpvn(classname, len, GV_ADD | flags));

    VXS_RETURN_M_SV(rv);
}

#define VTYPECHECK(var, val, varname) \
    STMT_START {							\
	SV * sv_vtc = val;						\
	if (ISA_VERSION_OBJ(sv_vtc)) {				\
	    (var) = SvRV(sv_vtc);						\
	}								\
	else								\
	    Perl_croak_nocontext(varname " is not of type version");	\
    } STMT_END

VXS(version_stringify)
{
     dXSARGS;
     if (items < 1)
	 croak_xs_usage(cv, "lobj, ...");
     SP -= items;
     {
	  SV *	lobj;
	  VTYPECHECK(lobj, ST(0), "lobj");

	  VXS_RETURN_M_SV(VSTRINGIFY(lobj));
     }
}

VXS(version_numify)
{
     dXSARGS;
     if (items < 1)
	 croak_xs_usage(cv, "lobj, ...");
     SP -= items;
     {
	  SV *	lobj;
	  VTYPECHECK(lobj, ST(0), "lobj");
	  VXS_RETURN_M_SV(VNUMIFY(lobj));
     }
}

VXS(version_normal)
{
     dXSARGS;
     if (items != 1)
	 croak_xs_usage(cv, "ver");
     SP -= items;
     {
	  SV *	ver;
	  VTYPECHECK(ver, ST(0), "ver");

	  VXS_RETURN_M_SV(VNORMAL(ver));
     }
}

VXS(version_vcmp)
{
     dXSARGS;
     if (items < 2)
	 croak_xs_usage(cv, "lobj, robj, ...");
     SP -= items;
     {
	  SV *	lobj;
	  VTYPECHECK(lobj, ST(0), "lobj");
	  {
	       SV	*rs;
	       SV	*rvs;
	       SV * robj = ST(1);
	       const int swap = items > 2 ? SvTRUE(ST(2)) : 0;

	       if ( !ISA_VERSION_OBJ(robj) )
	       {
		    robj = sv_2mortal(NEW_VERSION(SvOK(robj) ? robj : newSVpvs_flags("0", SVs_TEMP)));
	       }
	       rvs = SvRV(robj);

	       if ( swap )
	       {
		    rs = newSViv(VCMP(rvs,lobj));
	       }
	       else
	       {
		    rs = newSViv(VCMP(lobj,rvs));
	       }

	       VXS_RETURN_M_SV(rs);
	  }
     }
}

VXS(version_boolean)
{
    dXSARGS;
    SV *lobj;
    if (items < 1)
	croak_xs_usage(cv, "lobj, ...");
    SP -= items;
    VTYPECHECK(lobj, ST(0), "lobj");
    {
	SV * const rs =
	    newSViv( VCMP(lobj,
			  sv_2mortal(NEW_VERSION(
					sv_2mortal(newSVpvs("0"))
				    ))
			 )
		   );
	VXS_RETURN_M_SV(rs);
    }
}

VXS(version_noop)
{
    dXSARGS;
    if (items < 1)
	croak_xs_usage(cv, "lobj, ...");
    if (ISA_VERSION_OBJ(ST(0)))
	Perl_croak(aTHX_ "operation not supported with version object");
    else
	Perl_croak(aTHX_ "lobj is not of type version");
    XSRETURN_EMPTY;
}

static
void
S_version_check_key(pTHX_ CV * cv, const char * key, int keylen)
{
    dXSARGS;
    if (items != 1)
	croak_xs_usage(cv, "lobj");
    {
	SV *lobj = POPs;
	SV *ret;
	VTYPECHECK(lobj, lobj, "lobj");
	if ( hv_exists(MUTABLE_HV(lobj), key, keylen ) )
	    ret = &PL_sv_yes;
	else
	    ret = &PL_sv_no;
	PUSHs(ret);
	PUTBACK;
	return;
    }
}

VXS(version_is_alpha)
{
    S_version_check_key(aTHX_ cv, "alpha", 5);
}

VXS(version_qv)
{
    dXSARGS;
    PERL_UNUSED_ARG(cv);
    SP -= items;
    {
	SV * ver = ST(0);
	SV * sv0 = ver;
	SV * rv;
        STRLEN len = 0;
        const char * classname = "";
        U32 flags = 0;
        if ( items == 2 ) {
	    SV * sv1 = ST(1);
	    SvGETMAGIC(sv1);
	    if (SvOK(sv1)) {
		ver = sv1;
	    }
	    else {
		Perl_croak(aTHX_ "Invalid version format (version required)");
	    }
            if ( sv_isobject(sv0) ) { /* class called as an object method */
                const HV * stash = SvSTASH(SvRV(sv0));
                classname = HvNAME_get(stash);
                len       = HvNAMELEN_get(stash);
#ifdef HvNAMEUTF8
                flags     = HvNAMEUTF8(stash) ? SVf_UTF8 : 0;
#endif
            }
            else {
	       classname = SvPV(sv0, len);
                flags     = SvUTF8(sv0);
            }
	}
	if ( !SvVOK(ver) ) { /* not already a v-string */
	    rv = sv_newmortal();
	    SvSetSV_nosteal(rv,ver); /* make a duplicate */
	    UPG_VERSION(rv, TRUE);
	} else {
	    rv = sv_2mortal(NEW_VERSION(ver));
	}
	if ( items == 2 && (len != 7
                || strcmp(classname,"version")) ) { /* inherited new() */
	    sv_bless(rv, gv_stashpvn(classname, len, GV_ADD | flags));
        }
	PUSHs(rv);
    }
    PUTBACK;
    return;
}


VXS(version_is_qv)
{
    S_version_check_key(aTHX_ cv, "qv", 2);
}

#endif
