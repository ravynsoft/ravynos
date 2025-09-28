/* Extracted from perl-5.004/universal.c, contributed by Graham Barr */

static SV *
isa_lookup(stash, name, len, level)
HV *stash;
char *name;
int len;
int level;
{
    AV* av;
    GV* gv;
    GV** gvp;
    HV* hv = Nullhv;

    if (!stash)
	return &sv_undef;

    if(strEQ(HvNAME(stash), name))
	return &sv_yes;

    if (level > 100)
	croak("Recursive inheritance detected");

    gvp = (GV**)hv_fetch(stash, "::ISA::CACHE::", 14, FALSE);

    if (gvp && (gv = *gvp) != (GV*)&sv_undef && (hv = GvHV(gv))) {
	SV* sv;
	SV** svp = (SV**)hv_fetch(hv, name, len, FALSE);
	if (svp && (sv = *svp) != (SV*)&sv_undef)
	    return sv;
    }

    gvp = (GV**)hv_fetch(stash,"ISA",3,FALSE);
    
    if (gvp && (gv = *gvp) != (GV*)&sv_undef && (av = GvAV(gv))) {
	if(!hv) {
	    gvp = (GV**)hv_fetch(stash, "::ISA::CACHE::", 14, TRUE);

	    gv = *gvp;

	    if (SvTYPE(gv) != SVt_PVGV)
		gv_init(gv, stash, "::ISA::CACHE::", 14, TRUE);

	    hv = GvHVn(gv);
	}
	if(hv) {
	    SV** svp = AvARRAY(av);
	    I32 items = AvFILL(av) + 1;
	    while (items--) {
		SV* sv = *svp++;
		HV* basestash = gv_stashsv(sv, FALSE);
		if (!basestash) {
		    if (dowarn)
			warn("Can't locate package %s for @%s::ISA",
			    SvPVX(sv), HvNAME(stash));
		    continue;
		}
		if(&sv_yes == isa_lookup(basestash, name, len, level + 1)) {
		    (void)hv_store(hv,name,len,&sv_yes,0);
		    return &sv_yes;
		}
	    }
	    (void)hv_store(hv,name,len,&sv_no,0);
	}
    }

    return &sv_no;
}

static bool
sv_derived_from(sv, name)
SV * sv ;
char * name ;
{
    SV *rv;
    char *type;
    HV *stash;
  
    stash = Nullhv;
    type = Nullch;
 
    if (SvGMAGICAL(sv))
        mg_get(sv) ;

    if (SvROK(sv)) {
        sv = SvRV(sv);
        type = sv_reftype(sv,0);
        if(SvOBJECT(sv))
            stash = SvSTASH(sv);
    }
    else {
        stash = gv_stashsv(sv, FALSE);
    }
 
    return (type && strEQ(type,name)) ||
            (stash && isa_lookup(stash, name, strlen(name), 0) == &sv_yes)
        ? TRUE
        : FALSE ;
 
}
