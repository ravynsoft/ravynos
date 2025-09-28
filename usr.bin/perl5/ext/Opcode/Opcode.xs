#define PERL_NO_GET_CONTEXT
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

/* PL_maxo shouldn't differ from MAXO but leave room anyway (see BOOT:)	*/
#define OP_MASK_BUF_SIZE (MAXO + 100)

/* XXX op_named_bits and opset_all are never freed */
#define MY_CXT_KEY "Opcode::_guts" XS_VERSION

typedef struct {
    HV *	x_op_named_bits;	/* cache shared for whole process */
    SV *	x_opset_all;		/* mask with all bits set	*/
#ifdef OPCODE_DEBUG
    int		x_opcode_debug;		/* unused warn() emitting debugging code */
#endif
} my_cxt_t;

START_MY_CXT

/* length of opmasks in bytes */
static const STRLEN opset_len = (PL_maxo + 7) / 8;

#define op_named_bits		(MY_CXT.x_op_named_bits)
#define opset_all		(MY_CXT.x_opset_all)
#ifdef OPCODE_DEBUG
#  define opcode_debug		(MY_CXT.x_opcode_debug)
#else
 /* no API to turn this on at runtime, so constant fold the code away */
#  define opcode_debug		0
#endif

static SV  *new_opset (pTHX_ SV *old_opset);
static int  verify_opset (pTHX_ SV *opset, int fatal);
static void set_opset_bits (pTHX_ char *bitmap, SV *bitspec, int on, const char *opname);
static void put_op_bitspec (pTHX_ const char *optag,  STRLEN len, SV *opset);
static SV  *get_op_bitspec (pTHX_ const char *opname, STRLEN len, int fatal);


/* Initialise our private op_named_bits HV.
 * It is first loaded with the name and number of each perl operator.
 * Then the builtin tags :none and :all are added.
 * Opcode.pm loads the standard optags from __DATA__
 * XXX leak-alert: data allocated here is never freed, call this
 *     at most once
 */

static void
op_names_init(pTHX)
{
    int i;
    STRLEN len;
    const char *const *op_names;
    U8 *bitmap;
    dMY_CXT;

    op_named_bits = newHV();
    hv_ksplit(op_named_bits, PL_maxo);
    op_names = PL_op_name;
    for(i=0; i < PL_maxo; ++i) {
	SV * const sv = newSViv(i);
	SvREADONLY_on(sv);
	(void) hv_store(op_named_bits, op_names[i], strlen(op_names[i]), sv, 0);
    }

    put_op_bitspec(aTHX_ STR_WITH_LEN(":none"), sv_2mortal(new_opset(aTHX_ Nullsv)));

    opset_all = new_opset(aTHX_ Nullsv);
    bitmap = (U8*)SvPV(opset_all, len);
    memset(bitmap, 0xFF, len-1); /* deal with last byte specially, see below */
    /* Take care to set the right number of bits in the last byte */
    bitmap[len-1] = (PL_maxo & 0x07) ? ((U8) (~(0xFF << (PL_maxo & 0x07))))
                                     : 0xFF;
    put_op_bitspec(aTHX_ STR_WITH_LEN(":all"), opset_all); /* don't mortalise */
}


/* Store a new tag definition. Always a mask.
 * The tag must not already be defined.
 * SV *mask is copied not referenced.
 */

static void
put_op_bitspec(pTHX_ const char *optag, STRLEN len, SV *mask)
{
    SV **svp;
    dMY_CXT;

    verify_opset(aTHX_ mask,1);
    svp = hv_fetch(op_named_bits, optag, len, 1);
    if (SvOK(*svp))
	croak("Opcode tag \"%s\" already defined", optag);
    sv_setsv(*svp, mask);
    SvREADONLY_on(*svp);
}



/* Fetch a 'bits' entry for an opname or optag (IV/PV).
 * Note that we return the actual entry for speed.
 * Always sv_mortalcopy() if returning it to user code.
 */

static SV *
get_op_bitspec(pTHX_ const char *opname, STRLEN len, int fatal)
{
    SV **svp;
    dMY_CXT;

    svp = hv_fetch(op_named_bits, opname, len, 0);
    if (!svp || !SvOK(*svp)) {
	if (!fatal)
	    return Nullsv;
	if (*opname == ':')
	    croak("Unknown operator tag \"%s\"", opname);
	if (*opname == '!')	/* XXX here later, or elsewhere? */
	    croak("Can't negate operators here (\"%s\")", opname);
	if (isALPHA(*opname))
	    croak("Unknown operator name \"%s\"", opname);
	croak("Unknown operator prefix \"%s\"", opname);
    }
    return *svp;
}



static SV *
new_opset(pTHX_ SV *old_opset)
{
    SV *opset;

    if (old_opset) {
	verify_opset(aTHX_ old_opset,1);
	opset = newSVsv(old_opset);
    }
    else {
	opset = newSV(opset_len);
	Zero(SvPVX_const(opset), opset_len + 1, char);
	SvCUR_set(opset, opset_len);
	(void)SvPOK_only(opset);
    }
    /* not mortalised here */
    return opset;
}


static int
verify_opset(pTHX_ SV *opset, int fatal)
{
    const char *err = NULL;

    if      (!SvOK(opset))              err = "undefined";
    else if (!SvPOK(opset))             err = "wrong type";
    else if (SvCUR(opset) != opset_len) err = "wrong size";
    if (err && fatal) {
	croak("Invalid opset: %s", err);
    }
    return !err;
}


static void
set_opset_bits(pTHX_ char *bitmap, SV *bitspec, int on, const char *opname)
{
    if (SvIOK(bitspec)) {
	const int myopcode = SvIV(bitspec);
	const int offset = myopcode >> 3;
	const int bit    = myopcode & 0x07;
	if (myopcode >= PL_maxo || myopcode < 0)
	    croak("panic: opcode \"%s\" value %d is invalid", opname, myopcode);
	if (opcode_debug >= 2)
	    warn("set_opset_bits bit %2d (off=%d, bit=%d) %s %s\n",
			myopcode, offset, bit, opname, (on)?"on":"off");
	if (on)
	    bitmap[offset] |= 1 << bit;
	else
	    bitmap[offset] &= ~(1 << bit);
    }
    else if (SvPOK(bitspec) && SvCUR(bitspec) == opset_len) {

	STRLEN len;
	const char * const specbits = SvPV(bitspec, len);
	if (opcode_debug >= 2)
	    warn("set_opset_bits opset %s %s\n", opname, (on)?"on":"off");
	if (on) 
	    while(len-- > 0) bitmap[len] |=  specbits[len];
	else
	    while(len-- > 0) bitmap[len] &= ~specbits[len];
    }
    else
	croak("panic: invalid bitspec for \"%s\" (type %u)",
		opname, (unsigned)SvTYPE(bitspec));
}


static void
opmask_add(pTHX_ SV *opset)	/* THE ONLY FUNCTION TO EDIT PL_op_mask ITSELF	*/
{
    int j;
    char *bitmask;
    STRLEN len;
    int myopcode = 0;

    verify_opset(aTHX_ opset,1);		/* croaks on bad opset	*/

    if (!PL_op_mask)		/* caller must ensure PL_op_mask exists	*/
	croak("Can't add to uninitialised PL_op_mask");

    /* OPCODES ALREADY MASKED ARE NEVER UNMASKED. See opmask_addlocal()	*/

    bitmask = SvPV(opset, len);
    for (STRLEN i=0; i < opset_len; i++) {
	const U16 bits = bitmask[i];
	if (!bits) {	/* optimise for sparse masks */
	    myopcode += 8;
	    continue;
	}
	for (j=0; j < 8 && myopcode < PL_maxo; )
	    PL_op_mask[myopcode++] |= bits & (1 << j++);
    }
}

static void
opmask_addlocal(pTHX_ SV *opset, char *op_mask_buf) /* Localise PL_op_mask then opmask_add() */
{
    char *orig_op_mask = PL_op_mask;
#ifdef OPCODE_DEBUG
    dMY_CXT;
#endif

    SAVEVPTR(PL_op_mask);
    /* XXX casting to an ordinary function ptr from a member function ptr
     * is disallowed by Borland
     */
    if (opcode_debug >= 2)
	SAVEDESTRUCTOR((void(*)(void*))Perl_warn_nocontext,
            "PL_op_mask restored");
    PL_op_mask = &op_mask_buf[0];
    if (orig_op_mask)
	Copy(orig_op_mask, PL_op_mask, PL_maxo, char);
    else
	Zero(PL_op_mask, PL_maxo, char);
    opmask_add(aTHX_ opset);
}



MODULE = Opcode	PACKAGE = Opcode

PROTOTYPES: ENABLE

BOOT:
{
    MY_CXT_INIT;
    STATIC_ASSERT_STMT(PL_maxo < OP_MASK_BUF_SIZE);
    if (opcode_debug >= 1)
	warn("opset_len %ld\n", (long)opset_len);
    op_names_init(aTHX);
}

void
_safe_pkg_prep(Package)
    SV *Package
PPCODE:
    HV *hv; 
    char *hvname;
    ENTER;
   
    hv = gv_stashsv(Package, GV_ADDWARN); /* should exist already	*/

    hvname = HvNAME_get(hv);
    if (!hvname || strNE(hvname, "main")) {
        /* make it think it's in main:: */
	hv_name_set(hv, "main", 4, 0);
        (void) hv_store(hv,"_",1,(SV *)PL_defgv,0);  /* connect _ to global */
        SvREFCNT_inc((SV *)PL_defgv);  /* want to keep _ around! */
    }
    LEAVE;





void
_safe_call_sv(Package, mask, codesv)
    SV *	Package
    SV *	mask
    SV *	codesv
PPCODE:
    char op_mask_buf[OP_MASK_BUF_SIZE];
    GV *gv;
    HV *dummy_hv;

    ENTER;

    opmask_addlocal(aTHX_ mask, op_mask_buf);

    save_aptr(&PL_endav);
    PL_endav = (AV*)sv_2mortal((SV*)newAV()); /* ignore END blocks for now	*/

    save_hptr(&PL_defstash);		/* save current default stash	*/
    /* the assignment to global defstash changes our sense of 'main'	*/
    PL_defstash = gv_stashsv(Package, GV_ADDWARN); /* should exist already	*/

    SAVEGENERICSV(PL_curstash);
    PL_curstash = (HV *)SvREFCNT_inc_simple(PL_defstash);

    /* defstash must itself contain a main:: so we'll add that now	*/
    /* take care with the ref counts (was cause of long standing bug)	*/
    /* XXX I'm still not sure if this is right, GV_ADDWARN should warn!	*/
    gv = gv_fetchpvs("main::", GV_ADDWARN, SVt_PVHV);
    sv_free((SV*)GvHV(gv));
    GvHV(gv) = (HV*)SvREFCNT_inc(PL_defstash);

    /* %INC must be clean for use/require in compartment */
    dummy_hv = save_hash(PL_incgv);
    GvHV(PL_incgv) = (HV*)SvREFCNT_inc(GvHV(gv_HVadd(gv_fetchpvs("INC",GV_ADD,SVt_PVHV))));

    /* Invalidate class and method caches */
    ++PL_sub_generation;
    hv_clear(PL_stashcache);

    PUSHMARK(SP);
    /* use caller’s context */
    perl_call_sv(codesv, GIMME_V|G_EVAL|G_KEEPERR);
    sv_free( (SV *) dummy_hv);  /* get rid of what save_hash gave us*/
    SPAGAIN; /* for the PUTBACK added by xsubpp */
    LEAVE;

    /* Invalidate again */
    ++PL_sub_generation;
    hv_clear(PL_stashcache);


int
verify_opset(opset, fatal = 0)
    SV *opset
    int fatal
CODE:
    RETVAL = verify_opset(aTHX_ opset,fatal);
OUTPUT:
    RETVAL

void
invert_opset(opset)
    SV *opset
CODE:
    {
    char *bitmap;
    STRLEN len = opset_len;

    opset = sv_2mortal(new_opset(aTHX_ opset));	/* verify and clone opset */
    bitmap = SvPVX(opset);
    while(len-- > 0)
	bitmap[len] = ~bitmap[len];
    /* take care of extra bits beyond PL_maxo in last byte	*/
    if (PL_maxo & 07)
	bitmap[opset_len-1] &= ~(char)(0xFF << (PL_maxo & 0x07));
    }
    ST(0) = opset;


void
opset_to_ops(opset, desc = 0)
    SV *opset
    int	desc
PPCODE:
    {
    STRLEN len;
    STRLEN i;
    int j, myopcode;
    const char * const bitmap = SvPV(opset, len);
    const char *const *names = (desc) ? PL_op_desc : PL_op_name;

    verify_opset(aTHX_ opset,1);
    for (myopcode=0, i=0; i < opset_len; i++) {
	const U16 bits = bitmap[i];
	for (j=0; j < 8 && myopcode < PL_maxo; j++, myopcode++) {
	    if ( bits & (1 << j) )
		XPUSHs(newSVpvn_flags(names[myopcode], strlen(names[myopcode]),
				      SVs_TEMP));
	}
    }
    }


void
opset(...)
CODE:
    int i;
    SV *bitspec;
    STRLEN len, on;

    SV * const opset = sv_2mortal(new_opset(aTHX_ Nullsv));
    char * const bitmap = SvPVX(opset);
    for (i = 0; i < items; i++) {
	const char *opname;
	on = 1;
	if (verify_opset(aTHX_ ST(i),0)) {
	    opname = "(opset)";
	    bitspec = ST(i);
	}
	else {
	    opname = SvPV(ST(i), len);
	    if (*opname == '!') { on=0; ++opname;--len; }
	    bitspec = get_op_bitspec(aTHX_ opname, len, 1);
	}
	set_opset_bits(aTHX_ bitmap, bitspec, on, opname);
    }
    ST(0) = opset;


#define PERMITING  (ix == 0 || ix == 1)
#define ONLY_THESE (ix == 0 || ix == 2)

void
permit_only(safe, ...)
    SV *safe
ALIAS:
	permit    = 1
	deny_only = 2
	deny      = 3
CODE:
    int i;
    SV *bitspec, *mask;
    char *bitmap;
    STRLEN len;
    dMY_CXT;

    if (!SvROK(safe) || !SvOBJECT(SvRV(safe)) || SvTYPE(SvRV(safe))!=SVt_PVHV)
	croak("Not a Safe object");
    mask = *hv_fetchs((HV*)SvRV(safe), "Mask", 1);
    if (ONLY_THESE)	/* *_only = new mask, else edit current	*/
	sv_setsv(mask, sv_2mortal(new_opset(aTHX_ PERMITING ? opset_all : Nullsv)));
    else
	verify_opset(aTHX_ mask,1); /* croaks */
    bitmap = SvPVX(mask);
    for (i = 1; i < items; i++) {
	const char *opname;
	int on = PERMITING ? 0 : 1;		/* deny = mask bit on	*/
	if (verify_opset(aTHX_ ST(i),0)) {	/* it's a valid mask	*/
	    opname = "(opset)";
	    bitspec = ST(i);
	}
	else {				/* it's an opname/optag	*/
	    opname = SvPV(ST(i), len);
	    /* invert if op has ! prefix (only one allowed)	*/
	    if (*opname == '!') { on = !on; ++opname; --len; }
	    bitspec = get_op_bitspec(aTHX_ opname, len, 1); /* croaks */
	}
	set_opset_bits(aTHX_ bitmap, bitspec, on, opname);
    }
    ST(0) = &PL_sv_yes;



void
opdesc(...)
PPCODE:
    int i;
    STRLEN len;
    SV **args;
    const char *const *op_desc = PL_op_desc;

    /* copy args to a scratch area since we may push output values onto	*/
    /* the stack faster than we read values off it if masks are used.	*/
    args = (SV**)SvPVX(newSVpvn_flags((char*)&ST(0), items*sizeof(SV*), SVs_TEMP));
    for (i = 0; i < items; i++) {
	const char * const opname = SvPV(args[i], len);
	SV *bitspec = get_op_bitspec(aTHX_ opname, len, 1);
	if (SvIOK(bitspec)) {
	    const int myopcode = SvIV(bitspec);
	    if (myopcode < 0 || myopcode >= PL_maxo)
		croak("panic: opcode %d (%s) out of range",myopcode,opname);
	    XPUSHs(newSVpvn_flags(op_desc[myopcode], strlen(op_desc[myopcode]),
				  SVs_TEMP));
	}
        else if (SvPOK(bitspec) && SvCUR(bitspec) == opset_len) {
            STRLEN b;
            int j;
	    const char * const bitmap = SvPV_nolen_const(bitspec);
	    int myopcode = 0;
	    for (b=0; b < opset_len; b++) {
		const U16 bits = bitmap[b];
		for (j=0; j < 8 && myopcode < PL_maxo; j++, myopcode++)
		    if (bits & (1 << j))
			XPUSHs(newSVpvn_flags(op_desc[myopcode],
					      strlen(op_desc[myopcode]),
					      SVs_TEMP));
	    }
	}
	else
	    croak("panic: invalid bitspec for \"%s\" (type %u)",
		opname, (unsigned)SvTYPE(bitspec));
    }


void
define_optag(optagsv, mask)
    SV *optagsv
    SV *mask
CODE:
    STRLEN len;
    const char *optag = SvPV(optagsv, len);

    put_op_bitspec(aTHX_ optag, len, mask); /* croaks */
    ST(0) = &PL_sv_yes;


void
empty_opset()
CODE:
    ST(0) = sv_2mortal(new_opset(aTHX_ Nullsv));

void
full_opset()
CODE:
    dMY_CXT;
    ST(0) = sv_2mortal(new_opset(aTHX_ opset_all));

void
opmask_add(opset)
    SV *opset
PREINIT:
    if (!PL_op_mask)
	Newxz(PL_op_mask, PL_maxo, char);
CODE:
    opmask_add(aTHX_ opset);

void
opcodes()
PPCODE:
    if (GIMME_V == G_LIST) {
	croak("opcodes in list context not yet implemented"); /* XXX */
    }
    else {
	XPUSHs(sv_2mortal(newSViv(PL_maxo)));
    }

void
opmask()
CODE:
    ST(0) = sv_2mortal(new_opset(aTHX_ Nullsv));
    if (PL_op_mask) {
	char * const bitmap = SvPVX(ST(0));
	int myopcode;
	for(myopcode=0; myopcode < PL_maxo; ++myopcode) {
	    if (PL_op_mask[myopcode])
		bitmap[myopcode >> 3] |= 1 << (myopcode & 0x07);
	}
    }

