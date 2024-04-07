#define PERL_IN_XS_APITEST

/* We want to be able to test things that aren't API yet. */
#define PERL_EXT

/* Do *not* define PERL_NO_GET_CONTEXT.  This is the one place where we get
   to test implicit Perl_get_context().  */

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

typedef FILE NativeFile;

#include "fakesdio.h"   /* Causes us to use PerlIO below */

typedef SV *SVREF;
typedef PTR_TBL_t *XS__APItest__PtrTable;
typedef PerlIO * InputStream;
typedef PerlIO * OutputStream;

#define croak_fail() croak("fail at " __FILE__ " line %d", __LINE__)
#define croak_fail_nep(h, w) croak("fail %p!=%p at " __FILE__ " line %d", (h), (w), __LINE__)
#define croak_fail_nei(h, w) croak("fail %d!=%d at " __FILE__ " line %d", (int)(h), (int)(w), __LINE__)

/* assumes that there is a 'failed' variable in scope */
#define TEST_EXPR(s) STMT_START {           \
    if (s) {                                \
        printf("# ok: %s\n", #s);           \
    } else {                                \
        printf("# not ok: %s\n", #s);       \
        failed++;                           \
    }                                       \
} STMT_END

#if IVSIZE == 8
#  define TEST_64BIT 1
#else
#  define TEST_64BIT 0
#endif

#ifdef EBCDIC

void
cat_utf8a2n(SV* sv, const char * const ascii_utf8, STRLEN len)
{
    /* Converts variant UTF-8 text pointed to by 'ascii_utf8' of length 'len',
     * to UTF-EBCDIC, appending that text to the text already in 'sv'.
     * Currently doesn't work on invariants, as that is unneeded here, and we
     * could get double translations if we did.
     *
     * It has the algorithm for strict UTF-8 hard-coded in to find the code
     * point it represents, then calls uvchr_to_utf8() to convert to
     * UTF-EBCDIC).
     *
     * Note that this uses code points, not characters.  Thus if the input is
     * the UTF-8 for the code point 0xFF, the output will be the UTF-EBCDIC for
     * 0xFF, even though that code point represents different characters on
     * ASCII vs EBCDIC platforms. */

    dTHX;
    char * p = (char *) ascii_utf8;
    const char * const e = p + len;

    while (p < e) {
        UV code_point;
        U8 native_utf8[UTF8_MAXBYTES + 1];
        U8 * char_end;
        U8 start = (U8) *p;

        /* Start bytes are the same in both UTF-8 and I8, therefore we can
         * treat this ASCII UTF-8 byte as an I8 byte.  But PL_utf8skip[] is
         * indexed by NATIVE_UTF8 bytes, so transform to that */
        STRLEN char_bytes_len = PL_utf8skip[I8_TO_NATIVE_UTF8(start)];

        if (start < 0xc2) {
            croak("fail: Expecting start byte, instead got 0x%X at %s line %d",
                                                  (U8) *p, __FILE__, __LINE__);
        }
        code_point = (start & (((char_bytes_len) >= 7)
                                ? 0x00
                                : (0x1F >> ((char_bytes_len)-2))));
        p++;
        while (p < e && ((( (U8) *p) & 0xC0) == 0x80)) {

            code_point = (code_point << 6) | (( (U8) *p) & 0x3F);
            p++;
        }

        char_end = uvchr_to_utf8(native_utf8, code_point);
        sv_catpvn(sv, (char *) native_utf8, char_end - native_utf8);
    }
}

#endif

/* for my_cxt tests */

#define MY_CXT_KEY "XS::APItest::_guts" XS_VERSION

typedef struct {
    int i;
    SV *sv;
    GV *cscgv;
    AV *cscav;
    AV *bhkav;
    bool bhk_record;
    peep_t orig_peep;
    peep_t orig_rpeep;
    int peep_recording;
    AV *peep_recorder;
    AV *rpeep_recorder;
    AV *xop_record;
} my_cxt_t;

START_MY_CXT

static int
S_myset_set(pTHX_ SV* sv, MAGIC* mg)
{
    SV *isv = (SV*)mg->mg_ptr;

    PERL_UNUSED_ARG(sv);
    SvIVX(isv)++;
    return 0;
}

static int
S_myset_set_dies(pTHX_ SV* sv, MAGIC* mg)
{
    PERL_UNUSED_ARG(sv);
    PERL_UNUSED_ARG(mg);
    croak("in S_myset_set_dies");
    return 0;
}


static MGVTBL vtbl_foo, vtbl_bar;
static MGVTBL vtbl_myset = { 0, S_myset_set, 0, 0, 0, 0, 0, 0 };
static MGVTBL vtbl_myset_dies = { 0, S_myset_set_dies, 0, 0, 0, 0, 0, 0 };

static int
S_mycopy_copy(pTHX_ SV *sv, MAGIC* mg, SV *nsv, const char *name, I32 namlen) {
    PERL_UNUSED_ARG(sv);
    PERL_UNUSED_ARG(nsv);
    PERL_UNUSED_ARG(name);
    PERL_UNUSED_ARG(namlen);

    /* Count that we were called to "copy".
       There's actually no point in copying *this* magic onto nsv, as it's a
       SCALAR, whereas mg_copy is only triggered for ARRAYs and HASHes.
       It's not *exactly* generic. :-( */
    ++mg->mg_private;
    return 0;
}

STATIC MGVTBL vtbl_mycopy = { 0, 0, 0, 0, 0, S_mycopy_copy, 0, 0 };

/* indirect functions to test the [pa]MY_CXT macros */

int
my_cxt_getint_p(pMY_CXT)
{
    return MY_CXT.i;
}

void
my_cxt_setint_p(pMY_CXT_ int i)
{
    MY_CXT.i = i;
}

SV*
my_cxt_getsv_interp_context(void)
{
    dTHX;
    dMY_CXT_INTERP(my_perl);
    return MY_CXT.sv;
}

SV*
my_cxt_getsv_interp(void)
{
    dMY_CXT;
    return MY_CXT.sv;
}

void
my_cxt_setsv_p(SV* sv _pMY_CXT)
{
    MY_CXT.sv = sv;
}


/* from exception.c */
int apitest_exception(int);

/* from core_or_not.inc */
bool sv_setsv_cow_hashkey_core(void);
bool sv_setsv_cow_hashkey_notcore(void);

/* A routine to test hv_delayfree_ent
   (which itself is tested by testing on hv_free_ent  */

typedef void (freeent_function)(pTHX_ HV *, HE *);

void
test_freeent(freeent_function *f) {
    dSP;
    HV *test_hash = newHV();
    HE *victim;
    SV *test_scalar;
    U32 results[4];
    int i;

#ifdef PURIFY
    victim = (HE*)safemalloc(sizeof(HE));
#else
    /* Storing then deleting something should ensure that a hash entry is
       available.  */
    (void) hv_stores(test_hash, "", &PL_sv_yes);
    (void) hv_deletes(test_hash, "", 0);

    /* We need to "inline" new_he here as it's static, and the functions we
       test expect to be able to call del_HE on the HE  */
    if (!PL_body_roots[HE_ARENA_ROOT_IX])
        croak("PL_he_root is 0");
    victim = (HE*) PL_body_roots[HE_ARENA_ROOT_IX];
    PL_body_roots[HE_ARENA_ROOT_IX] = HeNEXT(victim);
#endif

#ifdef NODEFAULT_SHAREKEYS
    HvSHAREKEYS_on(test_hash);
#endif

    victim->hent_hek = Perl_share_hek(aTHX_ "", 0, 0);

    test_scalar = newSV(0);
    SvREFCNT_inc(test_scalar);
    HeVAL(victim) = test_scalar;

    /* Need this little game else we free the temps on the return stack.  */
    results[0] = SvREFCNT(test_scalar);
    SAVETMPS;
    results[1] = SvREFCNT(test_scalar);
    f(aTHX_ test_hash, victim);
    results[2] = SvREFCNT(test_scalar);
    FREETMPS;
    results[3] = SvREFCNT(test_scalar);

    i = 0;
    do {
        mXPUSHu(results[i]);
    } while (++i < (int)(sizeof(results)/sizeof(results[0])));

    /* Goodbye to our extra reference.  */
    SvREFCNT_dec(test_scalar);
}

/* Not that it matters much, but it's handy for the flipped character to just
 * be the opposite case (at least for ASCII-range and most Latin1 as well). */
#define FLIP_BIT ('A' ^ 'a')

static I32
bitflip_key(pTHX_ IV action, SV *field) {
    MAGIC *mg = mg_find(field, PERL_MAGIC_uvar);
    SV *keysv;
    PERL_UNUSED_ARG(action);
    if (mg && (keysv = mg->mg_obj)) {
        STRLEN len;
        const char *p = SvPV(keysv, len);

        if (len) {
            /* Allow for the flipped val to be longer than the original.  This
             * is just for testing, so can afford to have some slop */
            const STRLEN newlen = len * 2;

            SV *newkey = newSV(newlen);
            const char * const new_p_orig = SvPVX(newkey);
            char *new_p = (char *) new_p_orig;

            if (SvUTF8(keysv)) {
                const char *const end = p + len;
                while (p < end) {
                    STRLEN curlen;
                    UV chr = utf8_to_uvchr_buf((U8 *)p, (U8 *) end, &curlen);

                    /* Make sure don't exceed bounds */
                    assert(new_p - new_p_orig + curlen < newlen);

                    new_p = (char *)uvchr_to_utf8((U8 *)new_p, chr ^ FLIP_BIT);
                    p += curlen;
                }
                SvUTF8_on(newkey);
            } else {
                while (len--)
                    *new_p++ = *p++ ^ FLIP_BIT;
            }
            *new_p = '\0';
            SvCUR_set(newkey, new_p - new_p_orig);
            SvPOK_on(newkey);

            mg->mg_obj = newkey;
        }
    }
    return 0;
}

static I32
rot13_key(pTHX_ IV action, SV *field) {
    MAGIC *mg = mg_find(field, PERL_MAGIC_uvar);
    SV *keysv;
    PERL_UNUSED_ARG(action);
    if (mg && (keysv = mg->mg_obj)) {
        STRLEN len;
        const char *p = SvPV(keysv, len);

        if (len) {
            SV *newkey = newSV(len);
            char *new_p = SvPVX(newkey);

            /* There's a deliberate fencepost error here to loop len + 1 times
               to copy the trailing \0  */
            do {
                char new_c = *p++;
                /* Try doing this cleanly and clearly in EBCDIC another way: */
                switch (new_c) {
                case 'A': new_c = 'N'; break;
                case 'B': new_c = 'O'; break;
                case 'C': new_c = 'P'; break;
                case 'D': new_c = 'Q'; break;
                case 'E': new_c = 'R'; break;
                case 'F': new_c = 'S'; break;
                case 'G': new_c = 'T'; break;
                case 'H': new_c = 'U'; break;
                case 'I': new_c = 'V'; break;
                case 'J': new_c = 'W'; break;
                case 'K': new_c = 'X'; break;
                case 'L': new_c = 'Y'; break;
                case 'M': new_c = 'Z'; break;
                case 'N': new_c = 'A'; break;
                case 'O': new_c = 'B'; break;
                case 'P': new_c = 'C'; break;
                case 'Q': new_c = 'D'; break;
                case 'R': new_c = 'E'; break;
                case 'S': new_c = 'F'; break;
                case 'T': new_c = 'G'; break;
                case 'U': new_c = 'H'; break;
                case 'V': new_c = 'I'; break;
                case 'W': new_c = 'J'; break;
                case 'X': new_c = 'K'; break;
                case 'Y': new_c = 'L'; break;
                case 'Z': new_c = 'M'; break;
                case 'a': new_c = 'n'; break;
                case 'b': new_c = 'o'; break;
                case 'c': new_c = 'p'; break;
                case 'd': new_c = 'q'; break;
                case 'e': new_c = 'r'; break;
                case 'f': new_c = 's'; break;
                case 'g': new_c = 't'; break;
                case 'h': new_c = 'u'; break;
                case 'i': new_c = 'v'; break;
                case 'j': new_c = 'w'; break;
                case 'k': new_c = 'x'; break;
                case 'l': new_c = 'y'; break;
                case 'm': new_c = 'z'; break;
                case 'n': new_c = 'a'; break;
                case 'o': new_c = 'b'; break;
                case 'p': new_c = 'c'; break;
                case 'q': new_c = 'd'; break;
                case 'r': new_c = 'e'; break;
                case 's': new_c = 'f'; break;
                case 't': new_c = 'g'; break;
                case 'u': new_c = 'h'; break;
                case 'v': new_c = 'i'; break;
                case 'w': new_c = 'j'; break;
                case 'x': new_c = 'k'; break;
                case 'y': new_c = 'l'; break;
                case 'z': new_c = 'm'; break;
                }
                *new_p++ = new_c;
            } while (len--);
            SvCUR_set(newkey, SvCUR(keysv));
            SvPOK_on(newkey);
            if (SvUTF8(keysv))
                SvUTF8_on(newkey);

            mg->mg_obj = newkey;
        }
    }
    return 0;
}

STATIC I32
rmagical_a_dummy(pTHX_ IV idx, SV *sv) {
    PERL_UNUSED_ARG(idx);
    PERL_UNUSED_ARG(sv);
    return 0;
}

/* We could do "= { 0 };" but some versions of gcc do warn
 * (with -Wextra) about missing initializer, this is probably gcc
 * being a bit too paranoid.  But since this is file-static, we can
 * just have it without initializer, since it should get
 * zero-initialized. */
STATIC MGVTBL rmagical_b;

STATIC void
blockhook_csc_start(pTHX_ int full)
{
    dMY_CXT;
    AV *const cur = GvAV(MY_CXT.cscgv);

    PERL_UNUSED_ARG(full);
    SAVEGENERICSV(GvAV(MY_CXT.cscgv));

    if (cur) {
        Size_t i;
        AV *const new_av = newAV();

        for (i = 0; i < av_count(cur); i++) {
            av_store(new_av, i, newSVsv(*av_fetch(cur, i, 0)));
        }

        GvAV(MY_CXT.cscgv) = new_av;
    }
}

STATIC void
blockhook_csc_pre_end(pTHX_ OP **o)
{
    dMY_CXT;

    PERL_UNUSED_ARG(o);
    /* if we hit the end of a scope we missed the start of, we need to
     * unconditionally clear @CSC */
    if (GvAV(MY_CXT.cscgv) == MY_CXT.cscav && MY_CXT.cscav) {
        av_clear(MY_CXT.cscav);
    }

}

STATIC void
blockhook_test_start(pTHX_ int full)
{
    dMY_CXT;
    AV *av;

    if (MY_CXT.bhk_record) {
        av = newAV();
        av_push(av, newSVpvs("start"));
        av_push(av, newSViv(full));
        av_push(MY_CXT.bhkav, newRV_noinc(MUTABLE_SV(av)));
    }
}

STATIC void
blockhook_test_pre_end(pTHX_ OP **o)
{
    dMY_CXT;

    PERL_UNUSED_ARG(o);
    if (MY_CXT.bhk_record)
        av_push(MY_CXT.bhkav, newSVpvs("pre_end"));
}

STATIC void
blockhook_test_post_end(pTHX_ OP **o)
{
    dMY_CXT;

    PERL_UNUSED_ARG(o);
    if (MY_CXT.bhk_record)
        av_push(MY_CXT.bhkav, newSVpvs("post_end"));
}

STATIC void
blockhook_test_eval(pTHX_ OP *const o)
{
    dMY_CXT;
    AV *av;

    if (MY_CXT.bhk_record) {
        av = newAV();
        av_push(av, newSVpvs("eval"));
        av_push(av, newSVpv(OP_NAME(o), 0));
        av_push(MY_CXT.bhkav, newRV_noinc(MUTABLE_SV(av)));
    }
}

STATIC BHK bhk_csc, bhk_test;

STATIC void
my_peep (pTHX_ OP *o)
{
    dMY_CXT;

    if (!o)
        return;

    MY_CXT.orig_peep(aTHX_ o);

    if (!MY_CXT.peep_recording)
        return;

    for (; o; o = o->op_next) {
        if (o->op_type == OP_CONST && cSVOPx_sv(o) && SvPOK(cSVOPx_sv(o))) {
            av_push(MY_CXT.peep_recorder, newSVsv(cSVOPx_sv(o)));
        }
    }
}

STATIC void
my_rpeep (pTHX_ OP *first)
{
    dMY_CXT;
    OP *o, *t;

    if (!first)
        return;

    MY_CXT.orig_rpeep(aTHX_ first);

    if (!MY_CXT.peep_recording)
        return;

    for (o = first, t = first; o; o = o->op_next, t = t->op_next) {
        if (o->op_type == OP_CONST && cSVOPx_sv(o) && SvPOK(cSVOPx_sv(o))) {
            av_push(MY_CXT.rpeep_recorder, newSVsv(cSVOPx_sv(o)));
        }
        o = o->op_next;
        if (!o || o == t) break;
        if (o->op_type == OP_CONST && cSVOPx_sv(o) && SvPOK(cSVOPx_sv(o))) {
            av_push(MY_CXT.rpeep_recorder, newSVsv(cSVOPx_sv(o)));
        }
    }
}

STATIC OP *
THX_ck_entersub_args_lists(pTHX_ OP *entersubop, GV *namegv, SV *ckobj)
{
    PERL_UNUSED_ARG(namegv);
    PERL_UNUSED_ARG(ckobj);
    return ck_entersub_args_list(entersubop);
}

STATIC OP *
THX_ck_entersub_args_scalars(pTHX_ OP *entersubop, GV *namegv, SV *ckobj)
{
    OP *aop = cUNOPx(entersubop)->op_first;
    PERL_UNUSED_ARG(namegv);
    PERL_UNUSED_ARG(ckobj);
    if (!OpHAS_SIBLING(aop))
        aop = cUNOPx(aop)->op_first;
    for (aop = OpSIBLING(aop); OpHAS_SIBLING(aop); aop = OpSIBLING(aop)) {
        op_contextualize(aop, G_SCALAR);
    }
    return entersubop;
}

STATIC OP *
THX_ck_entersub_multi_sum(pTHX_ OP *entersubop, GV *namegv, SV *ckobj)
{
    OP *sumop = NULL;
    OP *parent = entersubop;
    OP *pushop = cUNOPx(entersubop)->op_first;
    PERL_UNUSED_ARG(namegv);
    PERL_UNUSED_ARG(ckobj);
    if (!OpHAS_SIBLING(pushop)) {
        parent = pushop;
        pushop = cUNOPx(pushop)->op_first;
    }
    while (1) {
        OP *aop = OpSIBLING(pushop);
        if (!OpHAS_SIBLING(aop))
            break;
        /* cut out first arg */
        op_sibling_splice(parent, pushop, 1, NULL);
        op_contextualize(aop, G_SCALAR);
        if (sumop) {
            sumop = newBINOP(OP_ADD, 0, sumop, aop);
        } else {
            sumop = aop;
        }
    }
    if (!sumop)
        sumop = newSVOP(OP_CONST, 0, newSViv(0));
    op_free(entersubop);
    return sumop;
}

STATIC void test_op_list_describe_part(SV *res, OP *o);
STATIC void
test_op_list_describe_part(SV *res, OP *o)
{
    sv_catpv(res, PL_op_name[o->op_type]);
    switch (o->op_type) {
        case OP_CONST: {
            sv_catpvf(res, "(%d)", (int)SvIV(cSVOPx(o)->op_sv));
        } break;
    }
    if (o->op_flags & OPf_KIDS) {
        OP *k;
        sv_catpvs(res, "[");
        for (k = cUNOPx(o)->op_first; k; k = OpSIBLING(k))
            test_op_list_describe_part(res, k);
        sv_catpvs(res, "]");
    } else {
        sv_catpvs(res, ".");
    }
}

STATIC char *
test_op_list_describe(OP *o)
{
    SV *res = sv_2mortal(newSVpvs(""));
    if (o)
        test_op_list_describe_part(res, o);
    return SvPVX(res);
}

/* the real new*OP functions have a tendency to call fold_constants, and
 * other such unhelpful things, so we need our own versions for testing */

#define mkUNOP(t, f) THX_mkUNOP(aTHX_ (t), (f))
static OP *
THX_mkUNOP(pTHX_ U32 type, OP *first)
{
    UNOP *unop;
    NewOp(1103, unop, 1, UNOP);
    unop->op_type   = (OPCODE)type;
    op_sibling_splice((OP*)unop, NULL, 0, first);
    return (OP *)unop;
}

#define mkBINOP(t, f, l) THX_mkBINOP(aTHX_ (t), (f), (l))
static OP *
THX_mkBINOP(pTHX_ U32 type, OP *first, OP *last)
{
    BINOP *binop;
    NewOp(1103, binop, 1, BINOP);
    binop->op_type      = (OPCODE)type;
    op_sibling_splice((OP*)binop, NULL, 0, last);
    op_sibling_splice((OP*)binop, NULL, 0, first);
    return (OP *)binop;
}

#define mkLISTOP(t, f, s, l) THX_mkLISTOP(aTHX_ (t), (f), (s), (l))
static OP *
THX_mkLISTOP(pTHX_ U32 type, OP *first, OP *sib, OP *last)
{
    LISTOP *listop;
    NewOp(1103, listop, 1, LISTOP);
    listop->op_type     = (OPCODE)type;
    op_sibling_splice((OP*)listop, NULL, 0, last);
    op_sibling_splice((OP*)listop, NULL, 0, sib);
    op_sibling_splice((OP*)listop, NULL, 0, first);
    return (OP *)listop;
}

static char *
test_op_linklist_describe(OP *start)
{
    SV *rv = sv_2mortal(newSVpvs(""));
    OP *o;
    o = start = LINKLIST(start);
    do {
        sv_catpvs(rv, ".");
        sv_catpv(rv, OP_NAME(o));
        if (o->op_type == OP_CONST)
            sv_catsv(rv, cSVOPo->op_sv);
        o = o->op_next;
    } while (o && o != start);
    return SvPVX(rv);
}

/** establish_cleanup operator, ripped off from Scope::Cleanup **/

STATIC void
THX_run_cleanup(pTHX_ void *cleanup_code_ref)
{
    dSP;
    PUSHSTACK;
    ENTER;
    SAVETMPS;
    PUSHMARK(SP);
    call_sv((SV*)cleanup_code_ref, G_VOID|G_DISCARD);
    FREETMPS;
    LEAVE;
    POPSTACK;
}

STATIC OP *
THX_pp_establish_cleanup(pTHX)
{
    dSP;
    SV *cleanup_code_ref;
    cleanup_code_ref = newSVsv(POPs);
    SAVEFREESV(cleanup_code_ref);
    SAVEDESTRUCTOR_X(THX_run_cleanup, cleanup_code_ref);
    if(GIMME_V != G_VOID) PUSHs(&PL_sv_undef);
    RETURN;
}

STATIC OP *
THX_ck_entersub_establish_cleanup(pTHX_ OP *entersubop, GV *namegv, SV *ckobj)
{
    OP *parent, *pushop, *argop, *estop;
    ck_entersub_args_proto(entersubop, namegv, ckobj);
    parent = entersubop;
    pushop = cUNOPx(entersubop)->op_first;
    if(!OpHAS_SIBLING(pushop)) {
        parent = pushop;
        pushop = cUNOPx(pushop)->op_first;
    }
    /* extract out first arg, then delete the rest of the tree */
    argop = OpSIBLING(pushop);
    op_sibling_splice(parent, pushop, 1, NULL);
    op_free(entersubop);

    estop = mkUNOP(OP_RAND, argop);
    estop->op_ppaddr = THX_pp_establish_cleanup;
    PL_hints |= HINT_BLOCK_SCOPE;
    return estop;
}

STATIC OP *
THX_ck_entersub_postinc(pTHX_ OP *entersubop, GV *namegv, SV *ckobj)
{
    OP *parent, *pushop, *argop;
    ck_entersub_args_proto(entersubop, namegv, ckobj);
    parent = entersubop;
    pushop = cUNOPx(entersubop)->op_first;
    if(!OpHAS_SIBLING(pushop)) {
        parent = pushop;
        pushop = cUNOPx(pushop)->op_first;
    }
    argop = OpSIBLING(pushop);
    op_sibling_splice(parent, pushop, 1, NULL);
    op_free(entersubop);
    return newUNOP(OP_POSTINC, 0,
        op_lvalue(op_contextualize(argop, G_SCALAR), OP_POSTINC));
}

STATIC OP *
THX_ck_entersub_pad_scalar(pTHX_ OP *entersubop, GV *namegv, SV *ckobj)
{
    OP *pushop, *argop;
    PADOFFSET padoff = NOT_IN_PAD;
    SV *a0, *a1;
    ck_entersub_args_proto(entersubop, namegv, ckobj);
    pushop = cUNOPx(entersubop)->op_first;
    if(!OpHAS_SIBLING(pushop))
        pushop = cUNOPx(pushop)->op_first;
    argop = OpSIBLING(pushop);
    if(argop->op_type != OP_CONST || OpSIBLING(argop)->op_type != OP_CONST)
        croak("bad argument expression type for pad_scalar()");
    a0 = cSVOPx_sv(argop);
    a1 = cSVOPx_sv(OpSIBLING(argop));
    switch(SvIV(a0)) {
        case 1: {
            SV *namesv = sv_2mortal(newSVpvs("$"));
            sv_catsv(namesv, a1);
            padoff = pad_findmy_sv(namesv, 0);
        } break;
        case 2: {
            char *namepv;
            STRLEN namelen;
            SV *namesv = sv_2mortal(newSVpvs("$"));
            sv_catsv(namesv, a1);
            namepv = SvPV(namesv, namelen);
            padoff = pad_findmy_pvn(namepv, namelen, SvUTF8(namesv));
        } break;
        case 3: {
            char *namepv;
            SV *namesv = sv_2mortal(newSVpvs("$"));
            sv_catsv(namesv, a1);
            namepv = SvPV_nolen(namesv);
            padoff = pad_findmy_pv(namepv, SvUTF8(namesv));
        } break;
        case 4: {
            padoff = pad_findmy_pvs("$foo", 0);
        } break;
        default: croak("bad type value for pad_scalar()");
    }
    op_free(entersubop);
    if(padoff == NOT_IN_PAD) {
        return newSVOP(OP_CONST, 0, newSVpvs("NOT_IN_PAD"));
    } else if(PAD_COMPNAME_FLAGS_isOUR(padoff)) {
        return newSVOP(OP_CONST, 0, newSVpvs("NOT_MY"));
    } else {
        OP *padop = newOP(OP_PADSV, 0);
        padop->op_targ = padoff;
        return padop;
    }
}

/** RPN keyword parser **/

#define sv_is_glob(sv) (SvTYPE(sv) == SVt_PVGV)
#define sv_is_regexp(sv) (SvTYPE(sv) == SVt_REGEXP)
#define sv_is_string(sv) \
    (!sv_is_glob(sv) && !sv_is_regexp(sv) && \
     (SvFLAGS(sv) & (SVf_IOK|SVf_NOK|SVf_POK|SVp_IOK|SVp_NOK|SVp_POK)))

static SV *hintkey_rpn_sv, *hintkey_calcrpn_sv, *hintkey_stufftest_sv;
static SV *hintkey_swaptwostmts_sv, *hintkey_looprest_sv;
static SV *hintkey_scopelessblock_sv;
static SV *hintkey_stmtasexpr_sv, *hintkey_stmtsasexpr_sv;
static SV *hintkey_loopblock_sv, *hintkey_blockasexpr_sv;
static SV *hintkey_swaplabel_sv, *hintkey_labelconst_sv;
static SV *hintkey_arrayfullexpr_sv, *hintkey_arraylistexpr_sv;
static SV *hintkey_arraytermexpr_sv, *hintkey_arrayarithexpr_sv;
static SV *hintkey_arrayexprflags_sv;
static SV *hintkey_subsignature_sv;
static SV *hintkey_DEFSV_sv;
static SV *hintkey_with_vars_sv;
static SV *hintkey_join_with_space_sv;
static int (*next_keyword_plugin)(pTHX_ char *, STRLEN, OP **);

/* low-level parser helpers */

#define PL_bufptr (PL_parser->bufptr)
#define PL_bufend (PL_parser->bufend)

/* RPN parser */

#define parse_var() THX_parse_var(aTHX)
static OP *THX_parse_var(pTHX)
{
    char *s = PL_bufptr;
    char *start = s;
    PADOFFSET varpos;
    OP *padop;
    if(*s != '$') croak("RPN syntax error");
    while(1) {
        char c = *++s;
        if(!isALNUM(c)) break;
    }
    if(s-start < 2) croak("RPN syntax error");
    lex_read_to(s);
    varpos = pad_findmy_pvn(start, s-start, 0);
    if(varpos == NOT_IN_PAD || PAD_COMPNAME_FLAGS_isOUR(varpos))
        croak("RPN only supports \"my\" variables");
    padop = newOP(OP_PADSV, 0);
    padop->op_targ = varpos;
    return padop;
}

#define push_rpn_item(o) \
    op_sibling_splice(parent, NULL, 0, o);
#define pop_rpn_item() ( \
    (tmpop = op_sibling_splice(parent, NULL, 1, NULL)) \
        ? tmpop : (croak("RPN stack underflow"), (OP*)NULL))

#define parse_rpn_expr() THX_parse_rpn_expr(aTHX)
static OP *THX_parse_rpn_expr(pTHX)
{
    OP *tmpop;
    /* fake parent for splice to mess with */
    OP *parent = mkBINOP(OP_NULL, NULL, NULL);

    while(1) {
        I32 c;
        lex_read_space(0);
        c = lex_peek_unichar(0);
        switch(c) {
            case /*(*/')': case /*{*/'}': {
                OP *result = pop_rpn_item();
                if(cLISTOPx(parent)->op_first)
                    croak("RPN expression must return a single value");
                op_free(parent);
                return result;
            } break;
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9': {
                UV val = 0;
                do {
                    lex_read_unichar(0);
                    val = 10*val + (c - '0');
                    c = lex_peek_unichar(0);
                } while(c >= '0' && c <= '9');
                push_rpn_item(newSVOP(OP_CONST, 0, newSVuv(val)));
            } break;
            case '$': {
                push_rpn_item(parse_var());
            } break;
            case '+': {
                OP *b = pop_rpn_item();
                OP *a = pop_rpn_item();
                lex_read_unichar(0);
                push_rpn_item(newBINOP(OP_I_ADD, 0, a, b));
            } break;
            case '-': {
                OP *b = pop_rpn_item();
                OP *a = pop_rpn_item();
                lex_read_unichar(0);
                push_rpn_item(newBINOP(OP_I_SUBTRACT, 0, a, b));
            } break;
            case '*': {
                OP *b = pop_rpn_item();
                OP *a = pop_rpn_item();
                lex_read_unichar(0);
                push_rpn_item(newBINOP(OP_I_MULTIPLY, 0, a, b));
            } break;
            case '/': {
                OP *b = pop_rpn_item();
                OP *a = pop_rpn_item();
                lex_read_unichar(0);
                push_rpn_item(newBINOP(OP_I_DIVIDE, 0, a, b));
            } break;
            case '%': {
                OP *b = pop_rpn_item();
                OP *a = pop_rpn_item();
                lex_read_unichar(0);
                push_rpn_item(newBINOP(OP_I_MODULO, 0, a, b));
            } break;
            default: {
                croak("RPN syntax error");
            } break;
        }
    }
}

#define parse_keyword_rpn() THX_parse_keyword_rpn(aTHX)
static OP *THX_parse_keyword_rpn(pTHX)
{
    OP *op;
    lex_read_space(0);
    if(lex_peek_unichar(0) != '('/*)*/)
        croak("RPN expression must be parenthesised");
    lex_read_unichar(0);
    op = parse_rpn_expr();
    if(lex_peek_unichar(0) != /*(*/')')
        croak("RPN expression must be parenthesised");
    lex_read_unichar(0);
    return op;
}

#define parse_keyword_calcrpn() THX_parse_keyword_calcrpn(aTHX)
static OP *THX_parse_keyword_calcrpn(pTHX)
{
    OP *varop, *exprop;
    lex_read_space(0);
    varop = parse_var();
    lex_read_space(0);
    if(lex_peek_unichar(0) != '{'/*}*/)
        croak("RPN expression must be braced");
    lex_read_unichar(0);
    exprop = parse_rpn_expr();
    if(lex_peek_unichar(0) != /*{*/'}')
        croak("RPN expression must be braced");
    lex_read_unichar(0);
    return newASSIGNOP(OPf_STACKED, varop, 0, exprop);
}

#define parse_keyword_stufftest() THX_parse_keyword_stufftest(aTHX)
static OP *THX_parse_keyword_stufftest(pTHX)
{
    I32 c;
    bool do_stuff;
    lex_read_space(0);
    do_stuff = lex_peek_unichar(0) == '+';
    if(do_stuff) {
        lex_read_unichar(0);
        lex_read_space(0);
    }
    c = lex_peek_unichar(0);
    if(c == ';') {
        lex_read_unichar(0);
    } else if(c != /*{*/'}') {
        croak("syntax error");
    }
    if(do_stuff) lex_stuff_pvs(" ", 0);
    return newOP(OP_NULL, 0);
}

#define parse_keyword_swaptwostmts() THX_parse_keyword_swaptwostmts(aTHX)
static OP *THX_parse_keyword_swaptwostmts(pTHX)
{
    OP *a, *b;
    a = parse_fullstmt(0);
    b = parse_fullstmt(0);
    if(a && b)
        PL_hints |= HINT_BLOCK_SCOPE;
    return op_append_list(OP_LINESEQ, b, a);
}

#define parse_keyword_looprest() THX_parse_keyword_looprest(aTHX)
static OP *THX_parse_keyword_looprest(pTHX)
{
    return newWHILEOP(0, 1, NULL, newSVOP(OP_CONST, 0, &PL_sv_yes),
                        parse_stmtseq(0), NULL, 1);
}

#define parse_keyword_scopelessblock() THX_parse_keyword_scopelessblock(aTHX)
static OP *THX_parse_keyword_scopelessblock(pTHX)
{
    I32 c;
    OP *body;
    lex_read_space(0);
    if(lex_peek_unichar(0) != '{'/*}*/) croak("syntax error");
    lex_read_unichar(0);
    body = parse_stmtseq(0);
    c = lex_peek_unichar(0);
    if(c != /*{*/'}' && c != /*[*/']' && c != /*(*/')') croak("syntax error");
    lex_read_unichar(0);
    return body;
}

#define parse_keyword_stmtasexpr() THX_parse_keyword_stmtasexpr(aTHX)
static OP *THX_parse_keyword_stmtasexpr(pTHX)
{
    OP *o = parse_barestmt(0);
    if (!o) o = newOP(OP_STUB, 0);
    if (PL_hints & HINT_BLOCK_SCOPE) o->op_flags |= OPf_PARENS;
    return op_scope(o);
}

#define parse_keyword_stmtsasexpr() THX_parse_keyword_stmtsasexpr(aTHX)
static OP *THX_parse_keyword_stmtsasexpr(pTHX)
{
    OP *o;
    lex_read_space(0);
    if(lex_peek_unichar(0) != '{'/*}*/) croak("syntax error");
    lex_read_unichar(0);
    o = parse_stmtseq(0);
    lex_read_space(0);
    if(lex_peek_unichar(0) != /*{*/'}') croak("syntax error");
    lex_read_unichar(0);
    if (!o) o = newOP(OP_STUB, 0);
    if (PL_hints & HINT_BLOCK_SCOPE) o->op_flags |= OPf_PARENS;
    return op_scope(o);
}

#define parse_keyword_loopblock() THX_parse_keyword_loopblock(aTHX)
static OP *THX_parse_keyword_loopblock(pTHX)
{
    return newWHILEOP(0, 1, NULL, newSVOP(OP_CONST, 0, &PL_sv_yes),
                        parse_block(0), NULL, 1);
}

#define parse_keyword_blockasexpr() THX_parse_keyword_blockasexpr(aTHX)
static OP *THX_parse_keyword_blockasexpr(pTHX)
{
    OP *o = parse_block(0);
    if (!o) o = newOP(OP_STUB, 0);
    if (PL_hints & HINT_BLOCK_SCOPE) o->op_flags |= OPf_PARENS;
    return op_scope(o);
}

#define parse_keyword_swaplabel() THX_parse_keyword_swaplabel(aTHX)
static OP *THX_parse_keyword_swaplabel(pTHX)
{
    OP *sop = parse_barestmt(0);
    SV *label = parse_label(PARSE_OPTIONAL);
    if (label) sv_2mortal(label);
    return newSTATEOP(label ? SvUTF8(label) : 0,
                      label ? savepv(SvPVX(label)) : NULL,
                      sop);
}

#define parse_keyword_labelconst() THX_parse_keyword_labelconst(aTHX)
static OP *THX_parse_keyword_labelconst(pTHX)
{
    return newSVOP(OP_CONST, 0, parse_label(0));
}

#define parse_keyword_arrayfullexpr() THX_parse_keyword_arrayfullexpr(aTHX)
static OP *THX_parse_keyword_arrayfullexpr(pTHX)
{
    return newANONLIST(parse_fullexpr(0));
}

#define parse_keyword_arraylistexpr() THX_parse_keyword_arraylistexpr(aTHX)
static OP *THX_parse_keyword_arraylistexpr(pTHX)
{
    return newANONLIST(parse_listexpr(0));
}

#define parse_keyword_arraytermexpr() THX_parse_keyword_arraytermexpr(aTHX)
static OP *THX_parse_keyword_arraytermexpr(pTHX)
{
    return newANONLIST(parse_termexpr(0));
}

#define parse_keyword_arrayarithexpr() THX_parse_keyword_arrayarithexpr(aTHX)
static OP *THX_parse_keyword_arrayarithexpr(pTHX)
{
    return newANONLIST(parse_arithexpr(0));
}

#define parse_keyword_arrayexprflags() THX_parse_keyword_arrayexprflags(aTHX)
static OP *THX_parse_keyword_arrayexprflags(pTHX)
{
    U32 flags = 0;
    I32 c;
    OP *o;
    lex_read_space(0);
    c = lex_peek_unichar(0);
    if (c != '!' && c != '?') croak("syntax error");
    lex_read_unichar(0);
    if (c == '?') flags |= PARSE_OPTIONAL;
    o = parse_listexpr(flags);
    return o ? newANONLIST(o) : newANONHASH(newOP(OP_STUB, 0));
}

#define parse_keyword_subsignature() THX_parse_keyword_subsignature(aTHX)
static OP *THX_parse_keyword_subsignature(pTHX)
{
    OP *retop = NULL, *listop, *sigop = parse_subsignature(0);
    OP *kid;
    int seen_nextstate = 0;

    /* We can't yield the optree as is to the caller because it won't be
     * executable outside of a called sub. We'll have to convert it into
     * something safe for them to invoke.
     * sigop should be an OP_NULL above a OP_LINESEQ containing
     * OP_NEXTSTATE-separated OP_ARGCHECK and OP_ARGELEMs
     */
    if(sigop->op_type != OP_NULL)
        croak("Expected parse_subsignature() to yield an OP_NULL");

    if(!(sigop->op_flags & OPf_KIDS))
        croak("Expected parse_subsignature() to yield an OP_NULL with kids");
    listop = cUNOPx(sigop)->op_first;

    if(listop->op_type != OP_LINESEQ)
        croak("Expected parse_subsignature() to yield an OP_LINESEQ");

    for(kid = cLISTOPx(listop)->op_first; kid; kid = OpSIBLING(kid)) {
        switch(kid->op_type) {
            case OP_NEXTSTATE:
                /* Only emit the first one otherwise they get boring */
                if(seen_nextstate)
                    break;
                seen_nextstate++;
                retop = op_append_list(OP_LIST, retop, newSVOP(OP_CONST, 0,
                    /* newSVpvf("nextstate:%s:%d", CopFILE(cCOPx(kid)), cCOPx(kid)->cop_line))); */
                    newSVpvf("nextstate:%u", (unsigned int)cCOPx(kid)->cop_line)));
                break;
            case OP_ARGCHECK: {
                struct op_argcheck_aux *p =
                    (struct op_argcheck_aux*)(cUNOP_AUXx(kid)->op_aux);
                retop = op_append_list(OP_LIST, retop, newSVOP(OP_CONST, 0,
                    newSVpvf("argcheck:%" UVuf ":%" UVuf ":%c",
                            p->params, p->opt_params,
                            p->slurpy ? p->slurpy : '-')));
                break;
            }
            case OP_ARGELEM: {
                PADOFFSET padix = kid->op_targ;
                PADNAMELIST *names = PadlistNAMES(CvPADLIST(find_runcv(0)));
                char *namepv = PadnamePV(padnamelist_fetch(names, padix));
                retop = op_append_list(OP_LIST, retop, newSVOP(OP_CONST, 0,
                    newSVpvf(kid->op_flags & OPf_KIDS ? "argelem:%s:d" : "argelem:%s", namepv)));
                break;
            }
            default:
                fprintf(stderr, "TODO: examine kid %p (optype=%s)\n", kid, PL_op_name[kid->op_type]);
                break;
        }
    }

    op_free(sigop);
    return newANONLIST(retop);
}

#define parse_keyword_DEFSV() THX_parse_keyword_DEFSV(aTHX)
static OP *THX_parse_keyword_DEFSV(pTHX)
{
    return newDEFSVOP();
}

#define sv_cat_c(a,b) THX_sv_cat_c(aTHX_ a, b)
static void THX_sv_cat_c(pTHX_ SV *sv, U32 c) {
    char ds[UTF8_MAXBYTES + 1], *d;
    d = (char *)uvchr_to_utf8((U8 *)ds, c);
    if (d - ds > 1) {
        sv_utf8_upgrade(sv);
    }
    sv_catpvn(sv, ds, d - ds);
}

#define parse_keyword_with_vars() THX_parse_keyword_with_vars(aTHX)
static OP *THX_parse_keyword_with_vars(pTHX)
{
    I32 c;
    IV count;
    int save_ix;
    OP *vardeclseq, *body;

    save_ix = block_start(TRUE);
    vardeclseq = NULL;

    count = 0;

    lex_read_space(0);
    c = lex_peek_unichar(0);
    while (c != '{') {
        SV *varname;
        PADOFFSET padoff;

        if (c == -1) {
            croak("unexpected EOF; expecting '{'");
        }

        if (!isIDFIRST_uni(c)) {
            croak("unexpected '%c'; expecting an identifier", (int)c);
        }

        varname = newSVpvs("$");
        if (lex_bufutf8()) {
            SvUTF8_on(varname);
        }

        sv_cat_c(varname, c);
        lex_read_unichar(0);

        while (c = lex_peek_unichar(0), c != -1 && isIDCONT_uni(c)) {
            sv_cat_c(varname, c);
            lex_read_unichar(0);
        }

        padoff = pad_add_name_sv(varname, padadd_NO_DUP_CHECK, NULL, NULL);

        {
            OP *my_var = newOP(OP_PADSV, OPf_MOD | (OPpLVAL_INTRO << 8));
            my_var->op_targ = padoff;

            vardeclseq = op_append_list(
                OP_LINESEQ,
                vardeclseq,
                newSTATEOP(
                    0, NULL,
                    newASSIGNOP(
                        OPf_STACKED,
                        my_var, 0,
                        newSVOP(
                            OP_CONST, 0,
                            newSViv(++count)
                        )
                    )
                )
            );
        }

        lex_read_space(0);
        c = lex_peek_unichar(0);
    }

    intro_my();

    body = parse_block(0);

    return block_end(save_ix, op_append_list(OP_LINESEQ, vardeclseq, body));
}

#define parse_join_with_space() THX_parse_join_with_space(aTHX)
static OP *THX_parse_join_with_space(pTHX)
{
    OP *delim, *args;

    args = parse_listexpr(0);
    delim = newSVOP(OP_CONST, 0, newSVpvs(" "));
    return op_convert_list(OP_JOIN, 0, op_prepend_elem(OP_LIST, delim, args));
}

/* plugin glue */

#define keyword_active(hintkey_sv) THX_keyword_active(aTHX_ hintkey_sv)
static int THX_keyword_active(pTHX_ SV *hintkey_sv)
{
    HE *he;
    if(!GvHV(PL_hintgv)) return 0;
    he = hv_fetch_ent(GvHV(PL_hintgv), hintkey_sv, 0,
                SvSHARED_HASH(hintkey_sv));
    return he && SvTRUE(HeVAL(he));
}

static int my_keyword_plugin(pTHX_
    char *keyword_ptr, STRLEN keyword_len, OP **op_ptr)
{
    if (memEQs(keyword_ptr, keyword_len, "rpn") &&
                    keyword_active(hintkey_rpn_sv)) {
        *op_ptr = parse_keyword_rpn();
        return KEYWORD_PLUGIN_EXPR;
    } else if (memEQs(keyword_ptr, keyword_len, "calcrpn") &&
                    keyword_active(hintkey_calcrpn_sv)) {
        *op_ptr = parse_keyword_calcrpn();
        return KEYWORD_PLUGIN_STMT;
    } else if (memEQs(keyword_ptr, keyword_len, "stufftest") &&
                    keyword_active(hintkey_stufftest_sv)) {
        *op_ptr = parse_keyword_stufftest();
        return KEYWORD_PLUGIN_STMT;
    } else if (memEQs(keyword_ptr, keyword_len, "swaptwostmts") &&
                    keyword_active(hintkey_swaptwostmts_sv)) {
        *op_ptr = parse_keyword_swaptwostmts();
        return KEYWORD_PLUGIN_STMT;
    } else if (memEQs(keyword_ptr, keyword_len, "looprest") &&
                    keyword_active(hintkey_looprest_sv)) {
        *op_ptr = parse_keyword_looprest();
        return KEYWORD_PLUGIN_STMT;
    } else if (memEQs(keyword_ptr, keyword_len, "scopelessblock") &&
                    keyword_active(hintkey_scopelessblock_sv)) {
        *op_ptr = parse_keyword_scopelessblock();
        return KEYWORD_PLUGIN_STMT;
    } else if (memEQs(keyword_ptr, keyword_len, "stmtasexpr") &&
                    keyword_active(hintkey_stmtasexpr_sv)) {
        *op_ptr = parse_keyword_stmtasexpr();
        return KEYWORD_PLUGIN_EXPR;
    } else if (memEQs(keyword_ptr, keyword_len, "stmtsasexpr") &&
                    keyword_active(hintkey_stmtsasexpr_sv)) {
        *op_ptr = parse_keyword_stmtsasexpr();
        return KEYWORD_PLUGIN_EXPR;
    } else if (memEQs(keyword_ptr, keyword_len, "loopblock") &&
                    keyword_active(hintkey_loopblock_sv)) {
        *op_ptr = parse_keyword_loopblock();
        return KEYWORD_PLUGIN_STMT;
    } else if (memEQs(keyword_ptr, keyword_len, "blockasexpr") &&
                    keyword_active(hintkey_blockasexpr_sv)) {
        *op_ptr = parse_keyword_blockasexpr();
        return KEYWORD_PLUGIN_EXPR;
    } else if (memEQs(keyword_ptr, keyword_len, "swaplabel") &&
                    keyword_active(hintkey_swaplabel_sv)) {
        *op_ptr = parse_keyword_swaplabel();
        return KEYWORD_PLUGIN_STMT;
    } else if (memEQs(keyword_ptr, keyword_len, "labelconst") &&
                    keyword_active(hintkey_labelconst_sv)) {
        *op_ptr = parse_keyword_labelconst();
        return KEYWORD_PLUGIN_EXPR;
    } else if (memEQs(keyword_ptr, keyword_len, "arrayfullexpr") &&
                    keyword_active(hintkey_arrayfullexpr_sv)) {
        *op_ptr = parse_keyword_arrayfullexpr();
        return KEYWORD_PLUGIN_EXPR;
    } else if (memEQs(keyword_ptr, keyword_len, "arraylistexpr") &&
                    keyword_active(hintkey_arraylistexpr_sv)) {
        *op_ptr = parse_keyword_arraylistexpr();
        return KEYWORD_PLUGIN_EXPR;
    } else if (memEQs(keyword_ptr, keyword_len, "arraytermexpr") &&
                    keyword_active(hintkey_arraytermexpr_sv)) {
        *op_ptr = parse_keyword_arraytermexpr();
        return KEYWORD_PLUGIN_EXPR;
    } else if (memEQs(keyword_ptr, keyword_len, "arrayarithexpr") &&
                    keyword_active(hintkey_arrayarithexpr_sv)) {
        *op_ptr = parse_keyword_arrayarithexpr();
        return KEYWORD_PLUGIN_EXPR;
    } else if (memEQs(keyword_ptr, keyword_len, "arrayexprflags") &&
                    keyword_active(hintkey_arrayexprflags_sv)) {
        *op_ptr = parse_keyword_arrayexprflags();
        return KEYWORD_PLUGIN_EXPR;
    } else if (memEQs(keyword_ptr, keyword_len, "DEFSV") &&
                    keyword_active(hintkey_DEFSV_sv)) {
        *op_ptr = parse_keyword_DEFSV();
        return KEYWORD_PLUGIN_EXPR;
    } else if (memEQs(keyword_ptr, keyword_len, "with_vars") &&
                    keyword_active(hintkey_with_vars_sv)) {
        *op_ptr = parse_keyword_with_vars();
        return KEYWORD_PLUGIN_STMT;
    } else if (memEQs(keyword_ptr, keyword_len, "join_with_space") &&
                    keyword_active(hintkey_join_with_space_sv)) {
        *op_ptr = parse_join_with_space();
        return KEYWORD_PLUGIN_EXPR;
    } else if (memEQs(keyword_ptr, keyword_len, "subsignature") &&
                    keyword_active(hintkey_subsignature_sv)) {
        *op_ptr = parse_keyword_subsignature();
        return KEYWORD_PLUGIN_EXPR;
    } else {
        assert(next_keyword_plugin != my_keyword_plugin);
        return next_keyword_plugin(aTHX_ keyword_ptr, keyword_len, op_ptr);
    }
}

static XOP my_xop;

static OP *
pp_xop(pTHX)
{
    return PL_op->op_next;
}

static void
peep_xop(pTHX_ OP *o, OP *oldop)
{
    dMY_CXT;
    av_push(MY_CXT.xop_record, newSVpvf("peep:%" UVxf, PTR2UV(o)));
    av_push(MY_CXT.xop_record, newSVpvf("oldop:%" UVxf, PTR2UV(oldop)));
}

static I32
filter_call(pTHX_ int idx, SV *buf_sv, int maxlen)
{
    char *p;
    char *end;
    int n = FILTER_READ(idx + 1, buf_sv, maxlen);

    if (n<=0) return n;

    p = SvPV_force_nolen(buf_sv);
    end = p + SvCUR(buf_sv);
    while (p < end) {
        if (*p == 'o') *p = 'e';
        p++;
    }
    return SvCUR(buf_sv);
}

static AV *
myget_linear_isa(pTHX_ HV *stash, U32 level) {
    GV **gvp = (GV **)hv_fetchs(stash, "ISA", 0);
    PERL_UNUSED_ARG(level);
    return gvp && *gvp && GvAV(*gvp)
         ? GvAV(*gvp)
         : (AV *)sv_2mortal((SV *)newAV());
}


XS_EXTERNAL(XS_XS__APItest__XSUB_XS_VERSION_undef);
XS_EXTERNAL(XS_XS__APItest__XSUB_XS_VERSION_empty);
XS_EXTERNAL(XS_XS__APItest__XSUB_XS_APIVERSION_invalid);

static struct mro_alg mymro;

static Perl_check_t addissub_nxck_add;

static OP *
addissub_myck_add(pTHX_ OP *op)
{
    SV **flag_svp = hv_fetchs(GvHV(PL_hintgv), "XS::APItest/addissub", 0);
    OP *aop, *bop;
    U8 flags;
    if (!(flag_svp && SvTRUE(*flag_svp) && (op->op_flags & OPf_KIDS) &&
            (aop = cBINOPx(op)->op_first) && (bop = OpSIBLING(aop)) &&
            !OpHAS_SIBLING(bop)))
        return addissub_nxck_add(aTHX_ op);
    flags = op->op_flags;
    op_sibling_splice(op, NULL, 1, NULL); /* excise aop */
    op_sibling_splice(op, NULL, 1, NULL); /* excise bop */
    op_free(op); /* free the empty husk */
    flags &= ~OPf_KIDS;
    return newBINOP(OP_SUBTRACT, flags, aop, bop);
}

static Perl_check_t old_ck_rv2cv;

static OP *
my_ck_rv2cv(pTHX_ OP *o)
{
    SV *ref;
    SV **flag_svp = hv_fetchs(GvHV(PL_hintgv), "XS::APItest/addunder", 0);
    OP *aop;

    if (flag_svp && SvTRUE(*flag_svp) && (o->op_flags & OPf_KIDS)
     && (aop = cUNOPx(o)->op_first) && aop->op_type == OP_CONST
     && aop->op_private & (OPpCONST_ENTERED|OPpCONST_BARE)
     && (ref = cSVOPx(aop)->op_sv) && SvPOK(ref) && SvCUR(ref)
     && *(SvEND(ref)-1) == 'o')
    {
        SvGROW(ref, SvCUR(ref)+2);
        *SvEND(ref) = '_';
        SvCUR(ref)++; /* Not _set, so we don't accidentally break non-PERL_CORE */
        *SvEND(ref) = '\0';
    }
    return old_ck_rv2cv(aTHX_ o);
}

#define test_bool_internals_macro(true_sv, false_sv) \
    test_bool_internals_func(true_sv, false_sv,\
        #true_sv " and " #false_sv)

U32
test_bool_internals_func(SV *true_sv, SV *false_sv, const char *msg) {
    U32 failed = 0;
    printf("# Testing '%s'\n", msg);
    TEST_EXPR(SvCUR(true_sv) == 1);
    TEST_EXPR(SvCUR(false_sv) == 0);
    TEST_EXPR(SvLEN(true_sv) == 0);
    TEST_EXPR(SvLEN(false_sv) == 0);
    TEST_EXPR(SvIV(true_sv) == 1);
    TEST_EXPR(SvIV(false_sv) == 0);
    TEST_EXPR(SvIsCOW(true_sv));
    TEST_EXPR(SvIsCOW(false_sv));
    TEST_EXPR(strEQ(SvPV_nolen(true_sv),"1"));
    TEST_EXPR(strEQ(SvPV_nolen(false_sv),""));
    TEST_EXPR(SvIOK(true_sv));
    TEST_EXPR(SvIOK(false_sv));
    TEST_EXPR(SvPOK(true_sv));
    TEST_EXPR(SvPOK(false_sv));
    TEST_EXPR(SvBoolFlagsOK(true_sv));
    TEST_EXPR(SvBoolFlagsOK(false_sv));
    TEST_EXPR(SvTYPE(true_sv) >= SVt_PVNV);
    TEST_EXPR(SvTYPE(false_sv) >= SVt_PVNV);
    TEST_EXPR(SvBoolFlagsOK(true_sv) && BOOL_INTERNALS_sv_isbool(true_sv));
    TEST_EXPR(SvBoolFlagsOK(false_sv) && BOOL_INTERNALS_sv_isbool(false_sv));
    TEST_EXPR(SvBoolFlagsOK(true_sv) && BOOL_INTERNALS_sv_isbool_true(true_sv));
    TEST_EXPR(SvBoolFlagsOK(false_sv) && BOOL_INTERNALS_sv_isbool_false(false_sv));
    TEST_EXPR(SvBoolFlagsOK(true_sv) && !BOOL_INTERNALS_sv_isbool_false(true_sv));
    TEST_EXPR(SvBoolFlagsOK(false_sv) && !BOOL_INTERNALS_sv_isbool_true(false_sv));
    TEST_EXPR(SvTRUE(true_sv));
    TEST_EXPR(!SvTRUE(false_sv));
    if (failed) {
        PerlIO_printf(Perl_debug_log, "# '%s' the tested true_sv:\n", msg);
        sv_dump(true_sv);
        PerlIO_printf(Perl_debug_log, "# PL_sv_yes:\n");
        sv_dump(&PL_sv_yes);
        PerlIO_printf(Perl_debug_log, "# '%s' tested false_sv:\n",msg);
        sv_dump(false_sv);
        PerlIO_printf(Perl_debug_log, "# PL_sv_no:\n");
        sv_dump(&PL_sv_no);
    }
    fflush(stdout);
    SvREFCNT_dec(true_sv);
    SvREFCNT_dec(false_sv);
    return failed;
}
#include "const-c.inc"

void
destruct_test(pTHX_ void *p) {
    warn("In destruct_test: %" SVf "\n", (SV*)p);
}

MODULE = XS::APItest            PACKAGE = XS::APItest

INCLUDE: const-xs.inc

INCLUDE: numeric.xs

void
assertx(int x)
    CODE:
        /* this only needs to compile and checks that assert() can be
           used this way syntactically */
        (void)(assert(x), 1);
        (void)(x);

MODULE = XS::APItest::utf8      PACKAGE = XS::APItest::utf8

int
bytes_cmp_utf8(bytes, utf8)
        SV *bytes
        SV *utf8
    PREINIT:
        const U8 *b;
        STRLEN blen;
        const U8 *u;
        STRLEN ulen;
    CODE:
        b = (const U8 *)SvPVbyte(bytes, blen);
        u = (const U8 *)SvPVbyte(utf8, ulen);
        RETVAL = bytes_cmp_utf8(b, blen, u, ulen);
    OUTPUT:
        RETVAL

AV *
test_utf8_to_bytes(bytes, len)
        U8 * bytes
        STRLEN len
    PREINIT:
        char * ret;
    CODE:
        RETVAL = newAV();
        sv_2mortal((SV*)RETVAL);

        ret = (char *) utf8_to_bytes(bytes, &len);
        av_push(RETVAL, newSVpv(ret, 0));

        /* utf8_to_bytes uses (STRLEN)-1 to signal errors, and we want to
         * return that as -1 to perl, so cast to SSize_t in case
         * sizeof(IV) > sizeof(STRLEN) */
        av_push(RETVAL, newSViv((SSize_t)len));
        av_push(RETVAL, newSVpv((const char *) bytes, 0));

    OUTPUT:
        RETVAL

AV *
test_utf8n_to_uvchr_msgs(s, len, flags)
        char *s
        STRLEN len
        U32 flags
    PREINIT:
        STRLEN retlen;
        UV ret;
        U32 errors;
        AV *msgs = NULL;

    CODE:
        RETVAL = newAV();
        sv_2mortal((SV*)RETVAL);

        ret = utf8n_to_uvchr_msgs((U8*)  s,
                                         len,
                                         &retlen,
                                         flags,
                                         &errors,
                                         &msgs);

        /* Returns the return value in [0]; <retlen> in [1], <errors> in [2] */
        av_push(RETVAL, newSVuv(ret));
        if (retlen == (STRLEN) -1) {
            av_push(RETVAL, newSViv(-1));
        }
        else {
            av_push(RETVAL, newSVuv(retlen));
        }
        av_push(RETVAL, newSVuv(errors));

        /* And any messages in [3] */
        if (msgs) {
            av_push(RETVAL, newRV_noinc((SV*)msgs));
        }

    OUTPUT:
        RETVAL

AV *
test_utf8n_to_uvchr_error(s, len, flags)

        char *s
        STRLEN len
        U32 flags
    PREINIT:
        STRLEN retlen;
        UV ret;
        U32 errors;

    CODE:
        /* Now that utf8n_to_uvchr() is a trivial wrapper for
         * utf8n_to_uvchr_error(), call the latter with the inputs.  It always
         * asks for the actual length to be returned and errors to be returned
         *
         * Length to assume <s> is; not checked, so could have buffer overflow
         */
        RETVAL = newAV();
        sv_2mortal((SV*)RETVAL);

        ret = utf8n_to_uvchr_error((U8*) s,
                                         len,
                                         &retlen,
                                         flags,
                                         &errors);

        /* Returns the return value in [0]; <retlen> in [1], <errors> in [2] */
        av_push(RETVAL, newSVuv(ret));
        if (retlen == (STRLEN) -1) {
            av_push(RETVAL, newSViv(-1));
        }
        else {
            av_push(RETVAL, newSVuv(retlen));
        }
        av_push(RETVAL, newSVuv(errors));

    OUTPUT:
        RETVAL

AV *
test_valid_utf8_to_uvchr(s)

        SV *s
    PREINIT:
        STRLEN retlen;
        UV ret;

    CODE:
        /* Call utf8n_to_uvchr() with the inputs.  It always asks for the
         * actual length to be returned
         *
         * Length to assume <s> is; not checked, so could have buffer overflow
         */
        RETVAL = newAV();
        sv_2mortal((SV*)RETVAL);

        ret = valid_utf8_to_uvchr((U8*) SvPV_nolen(s), &retlen);

        /* Returns the return value in [0]; <retlen> in [1] */
        av_push(RETVAL, newSVuv(ret));
        av_push(RETVAL, newSVuv(retlen));

    OUTPUT:
        RETVAL

SV *
test_uvchr_to_utf8_flags(uv, flags)

        SV *uv
        SV *flags
    PREINIT:
        U8 dest[UTF8_MAXBYTES + 1];
        U8 *ret;

    CODE:
        /* Call uvchr_to_utf8_flags() with the inputs.  */
        ret = uvchr_to_utf8_flags(dest, SvUV(uv), SvUV(flags));
        if (! ret) {
            XSRETURN_UNDEF;
        }
        RETVAL = newSVpvn((char *) dest, ret - dest);

    OUTPUT:
        RETVAL

AV *
test_uvchr_to_utf8_flags_msgs(uv, flags)

        SV *uv
        SV *flags
    PREINIT:
        U8 dest[UTF8_MAXBYTES + 1];
        U8 *ret;

    CODE:
        HV *msgs = NULL;
        RETVAL = newAV();
        sv_2mortal((SV*)RETVAL);

        ret = uvchr_to_utf8_flags_msgs(dest, SvUV(uv), SvUV(flags), &msgs);

        if (ret) {
            av_push(RETVAL, newSVpvn((char *) dest, ret - dest));
        }
        else {
            av_push(RETVAL,  &PL_sv_undef);
        }

        if (msgs) {
            av_push(RETVAL, newRV_noinc((SV*)msgs));
        }

    OUTPUT:
        RETVAL

MODULE = XS::APItest:Overload   PACKAGE = XS::APItest::Overload

void
does_amagic_apply(sv, method, flags)
    SV *sv
    int method
    int flags
    PPCODE:
        if(Perl_amagic_applies(aTHX_ sv, method, flags))
            XSRETURN_YES;
        else
            XSRETURN_NO;


void
amagic_deref_call(sv, what)
        SV *sv
        int what
    PPCODE:
        /* The reference is owned by something else.  */
        PUSHs(amagic_deref_call(sv, what));

# I'd certainly like to discourage the use of this macro, given that we now
# have amagic_deref_call

void
tryAMAGICunDEREF_var(sv, what)
        SV *sv
        int what
    PPCODE:
        {
            SV **sp = &sv;
            switch(what) {
            case to_av_amg:
                tryAMAGICunDEREF(to_av);
                break;
            case to_cv_amg:
                tryAMAGICunDEREF(to_cv);
                break;
            case to_gv_amg:
                tryAMAGICunDEREF(to_gv);
                break;
            case to_hv_amg:
                tryAMAGICunDEREF(to_hv);
                break;
            case to_sv_amg:
                tryAMAGICunDEREF(to_sv);
                break;
            default:
                croak("Invalid value %d passed to tryAMAGICunDEREF_var", what);
            }
        }
        /* The reference is owned by something else.  */
        PUSHs(sv);

MODULE = XS::APItest            PACKAGE = XS::APItest::XSUB

BOOT:
    newXS("XS::APItest::XSUB::XS_VERSION_undef", XS_XS__APItest__XSUB_XS_VERSION_undef, __FILE__);
    newXS("XS::APItest::XSUB::XS_VERSION_empty", XS_XS__APItest__XSUB_XS_VERSION_empty, __FILE__);
    newXS("XS::APItest::XSUB::XS_APIVERSION_invalid", XS_XS__APItest__XSUB_XS_APIVERSION_invalid, __FILE__);

void
XS_VERSION_defined(...)
    PPCODE:
        XS_VERSION_BOOTCHECK;
        XSRETURN_EMPTY;

void
XS_APIVERSION_valid(...)
    PPCODE:
        XS_APIVERSION_BOOTCHECK;
        XSRETURN_EMPTY;

void
xsreturn( int len )
    PPCODE:
        int i = 0;
        EXTEND( SP, len );
        for ( ; i < len; i++ ) {
            ST(i) = sv_2mortal( newSViv(i) );
        }
        XSRETURN( len );

void
xsreturn_iv()
    PPCODE:
        XSRETURN_IV(I32_MIN + 1);

void
xsreturn_uv()
    PPCODE:
        XSRETURN_UV( (U32)((1U<<31) + 1) );

void
xsreturn_nv()
    PPCODE:
        XSRETURN_NV(0.25);

void
xsreturn_pv()
    PPCODE:
        XSRETURN_PV("returned");

void
xsreturn_pvn()
    PPCODE:
        XSRETURN_PVN("returned too much",8);

void
xsreturn_no()
    PPCODE:
        XSRETURN_NO;

void
xsreturn_yes()
    PPCODE:
        XSRETURN_YES;

void
xsreturn_undef()
    PPCODE:
        XSRETURN_UNDEF;

void
xsreturn_empty()
    PPCODE:
        XSRETURN_EMPTY;

MODULE = XS::APItest:Hash               PACKAGE = XS::APItest::Hash

void
rot13_hash(hash)
        HV *hash
        CODE:
        {
            struct ufuncs uf;
            uf.uf_val = rot13_key;
            uf.uf_set = 0;
            uf.uf_index = 0;

            sv_magic((SV*)hash, NULL, PERL_MAGIC_uvar, (char*)&uf, sizeof(uf));
        }

void
bitflip_hash(hash)
        HV *hash
        CODE:
        {
            struct ufuncs uf;
            uf.uf_val = bitflip_key;
            uf.uf_set = 0;
            uf.uf_index = 0;

            sv_magic((SV*)hash, NULL, PERL_MAGIC_uvar, (char*)&uf, sizeof(uf));
        }

#define UTF8KLEN(sv, len)   (SvUTF8(sv) ? -(I32)len : (I32)len)

bool
exists(hash, key_sv)
        PREINIT:
        STRLEN len;
        const char *key;
        INPUT:
        HV *hash
        SV *key_sv
        CODE:
        key = SvPV(key_sv, len);
        RETVAL = hv_exists(hash, key, UTF8KLEN(key_sv, len));
        OUTPUT:
        RETVAL

bool
exists_ent(hash, key_sv)
        PREINIT:
        INPUT:
        HV *hash
        SV *key_sv
        CODE:
        RETVAL = hv_exists_ent(hash, key_sv, 0);
        OUTPUT:
        RETVAL

SV *
delete(hash, key_sv, flags = 0)
        PREINIT:
        STRLEN len;
        const char *key;
        INPUT:
        HV *hash
        SV *key_sv
        I32 flags;
        CODE:
        key = SvPV(key_sv, len);
        /* It's already mortal, so need to increase reference count.  */
        RETVAL
            = SvREFCNT_inc(hv_delete(hash, key, UTF8KLEN(key_sv, len), flags));
        OUTPUT:
        RETVAL

SV *
delete_ent(hash, key_sv, flags = 0)
        INPUT:
        HV *hash
        SV *key_sv
        I32 flags;
        CODE:
        /* It's already mortal, so need to increase reference count.  */
        RETVAL = SvREFCNT_inc(hv_delete_ent(hash, key_sv, flags, 0));
        OUTPUT:
        RETVAL

SV *
store_ent(hash, key, value)
        PREINIT:
        SV *copy;
        HE *result;
        INPUT:
        HV *hash
        SV *key
        SV *value
        CODE:
        copy = newSV(0);
        result = hv_store_ent(hash, key, copy, 0);
        SvSetMagicSV(copy, value);
        if (!result) {
            SvREFCNT_dec(copy);
            XSRETURN_EMPTY;
        }
        /* It's about to become mortal, so need to increase reference count.
         */
        RETVAL = SvREFCNT_inc(HeVAL(result));
        OUTPUT:
        RETVAL

SV *
store(hash, key_sv, value)
        PREINIT:
        STRLEN len;
        const char *key;
        SV *copy;
        SV **result;
        INPUT:
        HV *hash
        SV *key_sv
        SV *value
        CODE:
        key = SvPV(key_sv, len);
        copy = newSV(0);
        result = hv_store(hash, key, UTF8KLEN(key_sv, len), copy, 0);
        SvSetMagicSV(copy, value);
        if (!result) {
            SvREFCNT_dec(copy);
            XSRETURN_EMPTY;
        }
        /* It's about to become mortal, so need to increase reference count.
         */
        RETVAL = SvREFCNT_inc(*result);
        OUTPUT:
        RETVAL

SV *
fetch_ent(hash, key_sv)
        PREINIT:
        HE *result;
        INPUT:
        HV *hash
        SV *key_sv
        CODE:
        result = hv_fetch_ent(hash, key_sv, 0, 0);
        if (!result) {
            XSRETURN_EMPTY;
        }
        /* Force mg_get  */
        RETVAL = newSVsv(HeVAL(result));
        OUTPUT:
        RETVAL

SV *
fetch(hash, key_sv)
        PREINIT:
        STRLEN len;
        const char *key;
        SV **result;
        INPUT:
        HV *hash
        SV *key_sv
        CODE:
        key = SvPV(key_sv, len);
        result = hv_fetch(hash, key, UTF8KLEN(key_sv, len), 0);
        if (!result) {
            XSRETURN_EMPTY;
        }
        /* Force mg_get  */
        RETVAL = newSVsv(*result);
        OUTPUT:
        RETVAL

SV *
common(params)
        INPUT:
        HV *params
        PREINIT:
        HE *result;
        HV *hv = NULL;
        SV *keysv = NULL;
        const char *key = NULL;
        STRLEN klen = 0;
        int flags = 0;
        int action = 0;
        SV *val = NULL;
        U32 hash = 0;
        SV **svp;
        CODE:
        if ((svp = hv_fetchs(params, "hv", 0))) {
            SV *const rv = *svp;
            if (!SvROK(rv))
                croak("common passed a non-reference for parameter hv");
            hv = (HV *)SvRV(rv);
        }
        if ((svp = hv_fetchs(params, "keysv", 0)))
            keysv = *svp;
        if ((svp = hv_fetchs(params, "keypv", 0))) {
            key = SvPV_const(*svp, klen);
            if (SvUTF8(*svp))
                flags = HVhek_UTF8;
        }
        if ((svp = hv_fetchs(params, "action", 0)))
            action = SvIV(*svp);
        if ((svp = hv_fetchs(params, "val", 0)))
            val = newSVsv(*svp);
        if ((svp = hv_fetchs(params, "hash", 0)))
            hash = SvUV(*svp);

        if (hv_fetchs(params, "hash_pv", 0)) {
            assert(key);
            PERL_HASH(hash, key, klen);
        }
        if (hv_fetchs(params, "hash_sv", 0)) {
            assert(keysv);
            {
              STRLEN len;
              const char *const p = SvPV(keysv, len);
              PERL_HASH(hash, p, len);
            }
        }

        result = (HE *)hv_common(hv, keysv, key, klen, flags, action, val, hash);
        if (!result) {
            XSRETURN_EMPTY;
        }
        /* Force mg_get  */
        RETVAL = newSVsv(HeVAL(result));
        OUTPUT:
        RETVAL

void
test_hv_free_ent()
        PPCODE:
        test_freeent(&Perl_hv_free_ent);
        XSRETURN(4);

void
test_hv_delayfree_ent()
        PPCODE:
        test_freeent(&Perl_hv_delayfree_ent);
        XSRETURN(4);

SV *
test_share_unshare_pvn(input)
        PREINIT:
        STRLEN len;
        U32 hash;
        char *pvx;
        char *p;
        INPUT:
        SV *input
        CODE:
        pvx = SvPV(input, len);
        PERL_HASH(hash, pvx, len);
        p = sharepvn(pvx, len, hash);
        RETVAL = newSVpvn(p, len);
        unsharepvn(p, len, hash);
        OUTPUT:
        RETVAL

bool
refcounted_he_exists(key, level=0)
        SV *key
        IV level
        CODE:
        if (level) {
            croak("level must be zero, not %" IVdf, level);
        }
        RETVAL = (cop_hints_fetch_sv(PL_curcop, key, 0, 0) != &PL_sv_placeholder);
        OUTPUT:
        RETVAL

SV *
refcounted_he_fetch(key, level=0)
        SV *key
        IV level
        CODE:
        if (level) {
            croak("level must be zero, not %" IVdf, level);
        }
        RETVAL = cop_hints_fetch_sv(PL_curcop, key, 0, 0);
        SvREFCNT_inc(RETVAL);
        OUTPUT:
        RETVAL

void
test_force_keys(HV *hv)
    PREINIT:
        HE *he;
        SSize_t count = 0;
    PPCODE:
        hv_iterinit(hv);
        he = hv_iternext(hv);
        while (he) {
            SV *sv = HeSVKEY_force(he);
            ++count;
            EXTEND(SP, count);
            PUSHs(sv_mortalcopy(sv));
            he = hv_iternext(hv);
        }

=pod

sub TIEHASH  { bless {}, $_[0] }
sub STORE    { $_[0]->{$_[1]} = $_[2] }
sub FETCH    { $_[0]->{$_[1]} }
sub FIRSTKEY { my $a = scalar keys %{$_[0]}; each %{$_[0]} }
sub NEXTKEY  { each %{$_[0]} }
sub EXISTS   { exists $_[0]->{$_[1]} }
sub DELETE   { delete $_[0]->{$_[1]} }
sub CLEAR    { %{$_[0]} = () }

=cut

MODULE = XS::APItest:TempLv         PACKAGE = XS::APItest::TempLv

void
make_temp_mg_lv(sv)
SV* sv
    PREINIT:
        SV * const lv = newSV_type(SVt_PVLV);
        STRLEN len;
    PPCODE:
        SvPV(sv, len);

        sv_magic(lv, NULL, PERL_MAGIC_substr, NULL, 0);
        LvTYPE(lv) = 'x';
        LvTARG(lv) = SvREFCNT_inc_simple(sv);
        LvTARGOFF(lv) = len == 0 ? 0 : 1;
        LvTARGLEN(lv) = len < 2 ? 0 : len-2;

        EXTEND(SP, 1);
        ST(0) = sv_2mortal(lv);
        XSRETURN(1);


MODULE = XS::APItest::PtrTable  PACKAGE = XS::APItest::PtrTable PREFIX = ptr_table_

void
ptr_table_new(classname)
const char * classname
    PPCODE:
    PUSHs(sv_setref_pv(sv_newmortal(), classname, (void*)ptr_table_new()));

void
DESTROY(table)
XS::APItest::PtrTable table
    CODE:
    ptr_table_free(table);

void
ptr_table_store(table, from, to)
XS::APItest::PtrTable table
SVREF from
SVREF to
   CODE:
   ptr_table_store(table, from, to);

UV
ptr_table_fetch(table, from)
XS::APItest::PtrTable table
SVREF from
   CODE:
   RETVAL = PTR2UV(ptr_table_fetch(table, from));
   OUTPUT:
   RETVAL

void
ptr_table_split(table)
XS::APItest::PtrTable table

MODULE = XS::APItest::AutoLoader        PACKAGE = XS::APItest::AutoLoader

SV *
AUTOLOAD()
    CODE:
        RETVAL = newSVpvn_flags(SvPVX(cv), SvCUR(cv), SvUTF8(cv));
    OUTPUT:
        RETVAL

SV *
AUTOLOADp(...)
    PROTOTYPE: *$
    CODE:
        PERL_UNUSED_ARG(items);
        RETVAL = newSVpvn_flags(SvPVX(cv), SvCUR(cv), SvUTF8(cv));
    OUTPUT:
        RETVAL


MODULE = XS::APItest            PACKAGE = XS::APItest

PROTOTYPES: DISABLE

BOOT:
    mymro.resolve = myget_linear_isa;
    mymro.name    = "justisa";
    mymro.length  = 7;
    mymro.kflags  = 0;
    mymro.hash    = 0;
    Perl_mro_register(aTHX_ &mymro);

HV *
xop_custom_ops ()
    CODE:
        RETVAL = PL_custom_ops;
    OUTPUT:
        RETVAL

HV *
xop_custom_op_names ()
    CODE:
        PL_custom_op_names = newHV();
        RETVAL = PL_custom_op_names;
    OUTPUT:
        RETVAL

HV *
xop_custom_op_descs ()
    CODE:
        PL_custom_op_descs = newHV();
        RETVAL = PL_custom_op_descs;
    OUTPUT:
        RETVAL

void
xop_register ()
    CODE:
        XopENTRY_set(&my_xop, xop_name, "my_xop");
        XopENTRY_set(&my_xop, xop_desc, "XOP for testing");
        XopENTRY_set(&my_xop, xop_class, OA_UNOP);
        XopENTRY_set(&my_xop, xop_peep, peep_xop);
        Perl_custom_op_register(aTHX_ pp_xop, &my_xop);

void
xop_clear ()
    CODE:
        XopDISABLE(&my_xop, xop_name);
        XopDISABLE(&my_xop, xop_desc);
        XopDISABLE(&my_xop, xop_class);
        XopDISABLE(&my_xop, xop_peep);

IV
xop_my_xop ()
    CODE:
        RETVAL = PTR2IV(&my_xop);
    OUTPUT:
        RETVAL

IV
xop_ppaddr ()
    CODE:
        RETVAL = PTR2IV(pp_xop);
    OUTPUT:
        RETVAL

IV
xop_OA_UNOP ()
    CODE:
        RETVAL = OA_UNOP;
    OUTPUT:
        RETVAL

AV *
xop_build_optree ()
    CODE:
        dMY_CXT;
        UNOP *unop;
        OP *kid;

        MY_CXT.xop_record = newAV();

        kid = newSVOP(OP_CONST, 0, newSViv(42));

        unop = (UNOP*)mkUNOP(OP_CUSTOM, kid);
        unop->op_ppaddr     = pp_xop;
        unop->op_private    = 0;
        unop->op_next       = NULL;
        kid->op_next        = (OP*)unop;

        av_push(MY_CXT.xop_record, newSVpvf("unop:%" UVxf, PTR2UV(unop)));
        av_push(MY_CXT.xop_record, newSVpvf("kid:%" UVxf, PTR2UV(kid)));

        av_push(MY_CXT.xop_record, newSVpvf("NAME:%s", OP_NAME((OP*)unop)));
        av_push(MY_CXT.xop_record, newSVpvf("DESC:%s", OP_DESC((OP*)unop)));
        av_push(MY_CXT.xop_record, newSVpvf("CLASS:%d", (int)OP_CLASS((OP*)unop)));

        PL_rpeepp(aTHX_ kid);

        FreeOp(kid);
        FreeOp(unop);

        RETVAL = MY_CXT.xop_record;
        MY_CXT.xop_record = NULL;
    OUTPUT:
        RETVAL

IV
xop_from_custom_op ()
    CODE:
/* author note: this test doesn't imply Perl_custom_op_xop is or isn't public
   API or that Perl_custom_op_xop is known to be used outside the core */
        UNOP *unop;
        XOP *xop;

        unop = (UNOP*)mkUNOP(OP_CUSTOM, NULL);
        unop->op_ppaddr     = pp_xop;
        unop->op_private    = 0;
        unop->op_next       = NULL;

        xop = Perl_custom_op_xop(aTHX_ (OP *)unop);
        FreeOp(unop);
        RETVAL = PTR2IV(xop);
    OUTPUT:
        RETVAL

BOOT:
{
    MY_CXT_INIT;

    MY_CXT.i  = 99;
    MY_CXT.sv = newSVpv("initial",0);

    MY_CXT.bhkav = get_av("XS::APItest::bhkav", GV_ADDMULTI);
    MY_CXT.bhk_record = 0;

    BhkENTRY_set(&bhk_test, bhk_start, blockhook_test_start);
    BhkENTRY_set(&bhk_test, bhk_pre_end, blockhook_test_pre_end);
    BhkENTRY_set(&bhk_test, bhk_post_end, blockhook_test_post_end);
    BhkENTRY_set(&bhk_test, bhk_eval, blockhook_test_eval);
    Perl_blockhook_register(aTHX_ &bhk_test);

    MY_CXT.cscgv = gv_fetchpvs("XS::APItest::COMPILE_SCOPE_CONTAINER",
        GV_ADDMULTI, SVt_PVAV);
    MY_CXT.cscav = GvAV(MY_CXT.cscgv);

    BhkENTRY_set(&bhk_csc, bhk_start, blockhook_csc_start);
    BhkENTRY_set(&bhk_csc, bhk_pre_end, blockhook_csc_pre_end);
    Perl_blockhook_register(aTHX_ &bhk_csc);

    MY_CXT.peep_recorder = newAV();
    MY_CXT.rpeep_recorder = newAV();

    MY_CXT.orig_peep = PL_peepp;
    MY_CXT.orig_rpeep = PL_rpeepp;
    PL_peepp = my_peep;
    PL_rpeepp = my_rpeep;
}

void
CLONE(...)
    CODE:
    MY_CXT_CLONE;
    PERL_UNUSED_VAR(items);
    MY_CXT.sv = newSVpv("initial_clone",0);
    MY_CXT.cscgv = gv_fetchpvs("XS::APItest::COMPILE_SCOPE_CONTAINER",
        GV_ADDMULTI, SVt_PVAV);
    MY_CXT.cscav = NULL;
    MY_CXT.bhkav = get_av("XS::APItest::bhkav", GV_ADDMULTI);
    MY_CXT.bhk_record = 0;
    MY_CXT.peep_recorder = newAV();
    MY_CXT.rpeep_recorder = newAV();

void
print_double(val)
        double val
        CODE:
        printf("%5.3f\n",val);

int
have_long_double()
        CODE:
#ifdef HAS_LONG_DOUBLE
        RETVAL = 1;
#else
        RETVAL = 0;
#endif
        OUTPUT:
        RETVAL

void
print_long_double()
        CODE:
#ifdef HAS_LONG_DOUBLE
#   if defined(PERL_PRIfldbl) && (LONG_DOUBLESIZE > DOUBLESIZE)
        long double val = 7.0;
        printf("%5.3" PERL_PRIfldbl "\n",val);
#   else
        double val = 7.0;
        printf("%5.3f\n",val);
#   endif
#endif

void
print_long_doubleL()
        CODE:
#ifdef HAS_LONG_DOUBLE
        /* used to test we allow the length modifier required by the standard */
        long double val = 7.0;
        printf("%5.3Lf\n",val);
#else
        double val = 7.0;
        printf("%5.3f\n",val);
#endif

void
print_int(val)
        int val
        CODE:
        printf("%d\n",val);

void
print_long(val)
        long val
        CODE:
        printf("%ld\n",val);

void
print_float(val)
        float val
        CODE:
        printf("%5.3f\n",val);

void
print_flush()
        CODE:
        fflush(stdout);

void
mpushp()
        PPCODE:
        EXTEND(SP, 3);
        mPUSHp("one", 3);
        mPUSHp("two", 3);
        mPUSHpvs("three");
        XSRETURN(3);

void
mpushn()
        PPCODE:
        EXTEND(SP, 3);
        mPUSHn(0.5);
        mPUSHn(-0.25);
        mPUSHn(0.125);
        XSRETURN(3);

void
mpushi()
        PPCODE:
        EXTEND(SP, 3);
        mPUSHi(-1);
        mPUSHi(2);
        mPUSHi(-3);
        XSRETURN(3);

void
mpushu()
        PPCODE:
        EXTEND(SP, 3);
        mPUSHu(1);
        mPUSHu(2);
        mPUSHu(3);
        XSRETURN(3);

void
mxpushp()
        PPCODE:
        mXPUSHp("one", 3);
        mXPUSHp("two", 3);
        mXPUSHpvs("three");
        XSRETURN(3);

void
mxpushn()
        PPCODE:
        mXPUSHn(0.5);
        mXPUSHn(-0.25);
        mXPUSHn(0.125);
        XSRETURN(3);

void
mxpushi()
        PPCODE:
        mXPUSHi(-1);
        mXPUSHi(2);
        mXPUSHi(-3);
        XSRETURN(3);

void
mxpushu()
        PPCODE:
        mXPUSHu(1);
        mXPUSHu(2);
        mXPUSHu(3);
        XSRETURN(3);


 # test_EXTEND(): excerise the EXTEND() macro.
 # After calling EXTEND(), it also does *(p+n) = NULL and
 # *PL_stack_max = NULL to allow valgrind etc to spot if the stack hasn't
 # actually been extended properly.
 #
 # max_offset specifies the SP to use.  It is treated as a signed offset
 #              from PL_stack_max.
 # nsv        is the SV holding the value of n indicating how many slots
 #              to extend the stack by.
 # use_ss     is a boolean indicating that n should be cast to a SSize_t

void
test_EXTEND(max_offset, nsv, use_ss)
    IV   max_offset;
    SV  *nsv;
    bool use_ss;
PREINIT:
    SV **new_sp = PL_stack_max + max_offset;
    SSize_t new_offset = new_sp - PL_stack_base;
PPCODE:
    if (use_ss) {
        SSize_t n = (SSize_t)SvIV(nsv);
        EXTEND(new_sp, n);
        new_sp = PL_stack_base + new_offset;
        assert(new_sp + n <= PL_stack_max);
        if ((new_sp + n) > PL_stack_sp)
            *(new_sp + n) = NULL;
    }
    else {
        IV n = SvIV(nsv);
        EXTEND(new_sp, n);
        new_sp = PL_stack_base + new_offset;
        assert(new_sp + n <= PL_stack_max);
        if ((new_sp + n) > PL_stack_sp)
            *(new_sp + n) = NULL;
    }
    if (PL_stack_max > PL_stack_sp)
        *PL_stack_max = NULL;


void
call_sv_C()
PREINIT:
    CV * i_sub;
    GV * i_gv;
    I32 retcnt;
    SV * errsv;
    char * errstr;
    STRLEN errlen;
    SV * miscsv = sv_newmortal();
    HV * hv = (HV*)sv_2mortal((SV*)newHV());
CODE:
    i_sub = get_cv("i", 0);
    PUSHMARK(SP);
    /* PUTBACK not needed since this sub was called with 0 args, and is calling
      0 args, so global SP doesn't need to be moved before a call_* */
    retcnt = call_sv((SV*)i_sub, 0); /* try a CV* */
    SPAGAIN;
    SP -= retcnt; /* dont care about return count, wipe everything off */
    sv_setpvs(miscsv, "i");
    PUSHMARK(SP);
    retcnt = call_sv(miscsv, 0); /* try a PV */
    SPAGAIN;
    SP -= retcnt;
    /* no add and SVt_NULL are intentional, sub i should be defined already */
    i_gv = gv_fetchpvn_flags("i", sizeof("i")-1, 0, SVt_NULL);
    PUSHMARK(SP);
    retcnt = call_sv((SV*)i_gv, 0); /* try a GV* */
    SPAGAIN;
    SP -= retcnt;
    /* the tests below are not declaring this being public API behavior,
       only current internal behavior, these tests can be changed in the
       future if necessery */
    PUSHMARK(SP);
    retcnt = call_sv(&PL_sv_yes, G_EVAL);
    SPAGAIN;
    SP -= retcnt;
    errsv = ERRSV;
    errstr = SvPV(errsv, errlen);
    if(memBEGINs(errstr, errlen, "Undefined subroutine &main::1 called at")) {
        PUSHMARK(SP);
        retcnt = call_sv((SV*)i_sub, 0); /* call again to increase counter */
        SPAGAIN;
        SP -= retcnt;
    }
    PUSHMARK(SP);
    retcnt = call_sv(&PL_sv_no, G_EVAL);
    SPAGAIN;
    SP -= retcnt;
    errsv = ERRSV;
    errstr = SvPV(errsv, errlen);
    if(memBEGINs(errstr, errlen, "Undefined subroutine &main:: called at")) {
        PUSHMARK(SP);
        retcnt = call_sv((SV*)i_sub, 0); /* call again to increase counter */
        SPAGAIN;
        SP -= retcnt;
    }
    PUSHMARK(SP);
    retcnt = call_sv(&PL_sv_undef,  G_EVAL);
    SPAGAIN;
    SP -= retcnt;
    errsv = ERRSV;
    errstr = SvPV(errsv, errlen);
    if(memBEGINs(errstr, errlen, "Can't use an undefined value as a subroutine reference at")) {
        PUSHMARK(SP);
        retcnt = call_sv((SV*)i_sub, 0); /* call again to increase counter */
        SPAGAIN;
        SP -= retcnt;
    }
    PUSHMARK(SP);
    retcnt = call_sv((SV*)hv,  G_EVAL);
    SPAGAIN;
    SP -= retcnt;
    errsv = ERRSV;
    errstr = SvPV(errsv, errlen);
    if(memBEGINs(errstr, errlen, "Not a CODE reference at")) {
        PUSHMARK(SP);
        retcnt = call_sv((SV*)i_sub, 0); /* call again to increase counter */
        SPAGAIN;
        SP -= retcnt;
    }

void
call_sv(sv, flags, ...)
    SV* sv
    I32 flags
    PREINIT:
        I32 i;
    PPCODE:
        for (i=0; i<items-2; i++)
            ST(i) = ST(i+2); /* pop first two args */
        PUSHMARK(SP);
        SP += items - 2;
        PUTBACK;
        i = call_sv(sv, flags);
        SPAGAIN;
        EXTEND(SP, 1);
        PUSHs(sv_2mortal(newSViv(i)));

void
call_pv(subname, flags, ...)
    char* subname
    I32 flags
    PREINIT:
        I32 i;
    PPCODE:
        for (i=0; i<items-2; i++)
            ST(i) = ST(i+2); /* pop first two args */
        PUSHMARK(SP);
        SP += items - 2;
        PUTBACK;
        i = call_pv(subname, flags);
        SPAGAIN;
        EXTEND(SP, 1);
        PUSHs(sv_2mortal(newSViv(i)));

void
call_argv(subname, flags, ...)
    char* subname
    I32 flags
    PREINIT:
        I32 i;
        char *tmpary[4];
    PPCODE:
        for (i=0; i<items-2; i++)
            tmpary[i] = SvPV_nolen(ST(i+2)); /* ignore first two args */
        tmpary[i] = NULL;
        PUTBACK;
        i = call_argv(subname, flags, tmpary);
        SPAGAIN;
        EXTEND(SP, 1);
        PUSHs(sv_2mortal(newSViv(i)));

void
call_method(methname, flags, ...)
    char* methname
    I32 flags
    PREINIT:
        I32 i;
    PPCODE:
        for (i=0; i<items-2; i++)
            ST(i) = ST(i+2); /* pop first two args */
        PUSHMARK(SP);
        SP += items - 2;
        PUTBACK;
        i = call_method(methname, flags);
        SPAGAIN;
        EXTEND(SP, 1);
        PUSHs(sv_2mortal(newSViv(i)));

void
newCONSTSUB(stash, name, flags, sv)
    HV* stash
    SV* name
    I32 flags
    SV* sv
    ALIAS:
        newCONSTSUB_flags = 1
    PREINIT:
        CV* mycv = NULL;
        STRLEN len;
        const char *pv = SvPV(name, len);
    PPCODE:
        switch (ix) {
           case 0:
               mycv = newCONSTSUB(stash, pv, SvOK(sv) ? SvREFCNT_inc(sv) : NULL);
               break;
           case 1:
               mycv = newCONSTSUB_flags(
                 stash, pv, len, flags | SvUTF8(name), SvOK(sv) ? SvREFCNT_inc(sv) : NULL
               );
               break;
        }
        EXTEND(SP, 2);
        assert(mycv);
        PUSHs( CvCONST(mycv) ? &PL_sv_yes : &PL_sv_no );
        PUSHs((SV*)CvGV(mycv));

void
gv_init_type(namesv, multi, flags, type)
    SV* namesv
    int multi
    I32 flags
    int type
    PREINIT:
        STRLEN len;
        const char * const name = SvPV_const(namesv, len);
        GV *gv = *(GV**)hv_fetch(PL_defstash, name, len, TRUE);
    PPCODE:
        if (SvTYPE(gv) == SVt_PVGV)
            Perl_croak(aTHX_ "GV is already a PVGV");
        if (multi) flags |= GV_ADDMULTI;
        switch (type) {
           case 0:
               gv_init(gv, PL_defstash, name, len, multi);
               break;
           case 1:
               gv_init_sv(gv, PL_defstash, namesv, flags);
               break;
           case 2:
               gv_init_pv(gv, PL_defstash, name, flags | SvUTF8(namesv));
               break;
           case 3:
               gv_init_pvn(gv, PL_defstash, name, len, flags | SvUTF8(namesv));
               break;
        }
        XPUSHs( gv ? (SV*)gv : &PL_sv_undef);

void
gv_fetchmeth_type(stash, methname, type, level, flags)
    HV* stash
    SV* methname
    int type
    I32 level
    I32 flags
    PREINIT:
        STRLEN len;
        const char * const name = SvPV_const(methname, len);
        GV* gv = NULL;
    PPCODE:
        switch (type) {
           case 0:
               gv = gv_fetchmeth(stash, name, len, level);
               break;
           case 1:
               gv = gv_fetchmeth_sv(stash, methname, level, flags);
               break;
           case 2:
               gv = gv_fetchmeth_pv(stash, name, level, flags | SvUTF8(methname));
               break;
           case 3:
               gv = gv_fetchmeth_pvn(stash, name, len, level, flags | SvUTF8(methname));
               break;
        }
        XPUSHs( gv ? MUTABLE_SV(gv) : &PL_sv_undef );

void
gv_fetchmeth_autoload_type(stash, methname, type, level, flags)
    HV* stash
    SV* methname
    int type
    I32 level
    I32 flags
    PREINIT:
        STRLEN len;
        const char * const name = SvPV_const(methname, len);
        GV* gv = NULL;
    PPCODE:
        switch (type) {
           case 0:
               gv = gv_fetchmeth_autoload(stash, name, len, level);
               break;
           case 1:
               gv = gv_fetchmeth_sv_autoload(stash, methname, level, flags);
               break;
           case 2:
               gv = gv_fetchmeth_pv_autoload(stash, name, level, flags | SvUTF8(methname));
               break;
           case 3:
               gv = gv_fetchmeth_pvn_autoload(stash, name, len, level, flags | SvUTF8(methname));
               break;
        }
        XPUSHs( gv ? MUTABLE_SV(gv) : &PL_sv_undef );

void
gv_fetchmethod_flags_type(stash, methname, type, flags)
    HV* stash
    SV* methname
    int type
    I32 flags
    PREINIT:
        GV* gv = NULL;
    PPCODE:
        switch (type) {
           case 0:
               gv = gv_fetchmethod_flags(stash, SvPVX_const(methname), flags);
               break;
           case 1:
               gv = gv_fetchmethod_sv_flags(stash, methname, flags);
               break;
           case 2:
               gv = gv_fetchmethod_pv_flags(stash, SvPV_nolen(methname), flags | SvUTF8(methname));
               break;
           case 3: {
               STRLEN len;
               const char * const name = SvPV_const(methname, len);
               gv = gv_fetchmethod_pvn_flags(stash, name, len, flags | SvUTF8(methname));
               break;
            }
           case 4:
               gv = gv_fetchmethod_pvn_flags(stash, SvPV_nolen(methname),
                                             flags, SvUTF8(methname));
        }
        XPUSHs( gv ? (SV*)gv : &PL_sv_undef);

void
gv_autoload_type(stash, methname, type, method)
    HV* stash
    SV* methname
    int type
    I32 method
    PREINIT:
        STRLEN len;
        const char * const name = SvPV_const(methname, len);
        GV* gv = NULL;
        I32 flags = method ? GV_AUTOLOAD_ISMETHOD : 0;
    PPCODE:
        switch (type) {
           case 0:
               gv = gv_autoload4(stash, name, len, method);
               break;
           case 1:
               gv = gv_autoload_sv(stash, methname, flags);
               break;
           case 2:
               gv = gv_autoload_pv(stash, name, flags | SvUTF8(methname));
               break;
           case 3:
               gv = gv_autoload_pvn(stash, name, len, flags | SvUTF8(methname));
               break;
        }
        XPUSHs( gv ? (SV*)gv : &PL_sv_undef);

SV *
gv_const_sv(SV *name)
    PREINIT:
        GV *gv;
    CODE:
        if (SvPOK(name)) {
            HV *stash = gv_stashpv("main",0);
            HE *he = hv_fetch_ent(stash, name, 0, 0);
            gv = (GV *)HeVAL(he);
        }
        else {
            gv = (GV *)name;
        }
        RETVAL = gv_const_sv(gv);
        if (!RETVAL)
            XSRETURN_EMPTY;
        RETVAL = newSVsv(RETVAL);
    OUTPUT:
        RETVAL

void
whichsig_type(namesv, type)
    SV* namesv
    int type
    PREINIT:
        STRLEN len;
        const char * const name = SvPV_const(namesv, len);
        I32 i = 0;
    PPCODE:
        switch (type) {
           case 0:
              i = whichsig(name);
               break;
           case 1:
               i = whichsig_sv(namesv);
               break;
           case 2:
               i = whichsig_pv(name);
               break;
           case 3:
               i = whichsig_pvn(name, len);
               break;
        }
        XPUSHs(sv_2mortal(newSViv(i)));

void
eval_sv(sv, flags)
    SV* sv
    I32 flags
    PREINIT:
        I32 i;
    PPCODE:
        PUTBACK;
        i = eval_sv(sv, flags);
        SPAGAIN;
        EXTEND(SP, 1);
        PUSHs(sv_2mortal(newSViv(i)));

void
eval_pv(p, croak_on_error)
    const char* p
    I32 croak_on_error
    PPCODE:
        PUTBACK;
        EXTEND(SP, 1);
        PUSHs(eval_pv(p, croak_on_error));

void
require_pv(pv)
    const char* pv
    PPCODE:
        PUTBACK;
        require_pv(pv);

int
apitest_exception(throw_e)
    int throw_e
    OUTPUT:
        RETVAL

void
mycroak(sv)
    SV* sv
    CODE:
    if (SvOK(sv)) {
        Perl_croak(aTHX_ "%s", SvPV_nolen(sv));
    }
    else {
        Perl_croak(aTHX_ NULL);
    }

SV*
strtab()
   CODE:
   RETVAL = newRV_inc((SV*)PL_strtab);
   OUTPUT:
   RETVAL

int
my_cxt_getint()
    CODE:
        dMY_CXT;
        RETVAL = my_cxt_getint_p(aMY_CXT);
    OUTPUT:
        RETVAL

void
my_cxt_setint(i)
    int i;
    CODE:
        dMY_CXT;
        my_cxt_setint_p(aMY_CXT_ i);

void
my_cxt_getsv(how)
    bool how;
    PPCODE:
        EXTEND(SP, 1);
        ST(0) = how ? my_cxt_getsv_interp_context() : my_cxt_getsv_interp();
        XSRETURN(1);

void
my_cxt_setsv(sv)
    SV *sv;
    CODE:
        dMY_CXT;
        SvREFCNT_dec(MY_CXT.sv);
        my_cxt_setsv_p(sv _aMY_CXT);
        SvREFCNT_inc(sv);

bool
sv_setsv_cow_hashkey_core()

bool
sv_setsv_cow_hashkey_notcore()

void
sv_set_deref(SV *sv, SV *sv2, int which)
    CODE:
    {
        STRLEN len;
        const char *pv = SvPV(sv2,len);
        if (!SvROK(sv)) croak("Not a ref");
        sv = SvRV(sv);
        switch (which) {
            case 0: sv_setsv(sv,sv2); break;
            case 1: sv_setpv(sv,pv); break;
            case 2: sv_setpvn(sv,pv,len); break;
        }
    }

void
rmagical_cast(sv, type)
    SV *sv;
    SV *type;
    PREINIT:
        struct ufuncs uf;
    PPCODE:
        if (!SvOK(sv) || !SvROK(sv) || !SvOK(type)) { XSRETURN_UNDEF; }
        sv = SvRV(sv);
        if (SvTYPE(sv) != SVt_PVHV) { XSRETURN_UNDEF; }
        uf.uf_val = rmagical_a_dummy;
        uf.uf_set = NULL;
        uf.uf_index = 0;
        if (SvTRUE(type)) { /* b */
            sv_magicext(sv, NULL, PERL_MAGIC_ext, &rmagical_b, NULL, 0);
        } else { /* a */
            sv_magic(sv, NULL, PERL_MAGIC_uvar, (char *) &uf, sizeof(uf));
        }
        XSRETURN_YES;

void
rmagical_flags(sv)
    SV *sv;
    PPCODE:
        if (!SvOK(sv) || !SvROK(sv)) { XSRETURN_UNDEF; }
        sv = SvRV(sv);
        EXTEND(SP, 3);
        mXPUSHu(SvFLAGS(sv) & SVs_GMG);
        mXPUSHu(SvFLAGS(sv) & SVs_SMG);
        mXPUSHu(SvFLAGS(sv) & SVs_RMG);
        XSRETURN(3);

void
my_caller(level)
        I32 level
    PREINIT:
        const PERL_CONTEXT *cx, *dbcx;
        const char *pv;
        const GV *gv;
        HV *hv;
    PPCODE:
        cx = caller_cx(level, &dbcx);
        EXTEND(SP, 8);

        pv = CopSTASHPV(cx->blk_oldcop);
        ST(0) = pv ? sv_2mortal(newSVpv(pv, 0)) : &PL_sv_undef;
        gv = CvGV(cx->blk_sub.cv);
        ST(1) = isGV(gv) ? sv_2mortal(newSVpv(GvNAME(gv), 0)) : &PL_sv_undef;

        pv = CopSTASHPV(dbcx->blk_oldcop);
        ST(2) = pv ? sv_2mortal(newSVpv(pv, 0)) : &PL_sv_undef;
        gv = CvGV(dbcx->blk_sub.cv);
        ST(3) = isGV(gv) ? sv_2mortal(newSVpv(GvNAME(gv), 0)) : &PL_sv_undef;

        ST(4) = cop_hints_fetch_pvs(cx->blk_oldcop, "foo", 0);
        ST(5) = cop_hints_fetch_pvn(cx->blk_oldcop, "foo", 3, 0, 0);
        ST(6) = cop_hints_fetch_sv(cx->blk_oldcop,
                sv_2mortal(newSVpvs("foo")), 0, 0);

        hv = cop_hints_2hv(cx->blk_oldcop, 0);
        ST(7) = hv ? sv_2mortal(newRV_noinc((SV *)hv)) : &PL_sv_undef;

        XSRETURN(8);

void
DPeek (sv)
    SV   *sv

  PPCODE:
    ST (0) = newSVpv (Perl_sv_peek (aTHX_ sv), 0);
    XSRETURN (1);

void
BEGIN()
    CODE:
        sv_inc(get_sv("XS::APItest::BEGIN_called", GV_ADD|GV_ADDMULTI));

void
CHECK()
    CODE:
        sv_inc(get_sv("XS::APItest::CHECK_called", GV_ADD|GV_ADDMULTI));

void
UNITCHECK()
    CODE:
        sv_inc(get_sv("XS::APItest::UNITCHECK_called", GV_ADD|GV_ADDMULTI));

void
INIT()
    CODE:
        sv_inc(get_sv("XS::APItest::INIT_called", GV_ADD|GV_ADDMULTI));

void
END()
    CODE:
        sv_inc(get_sv("XS::APItest::END_called", GV_ADD|GV_ADDMULTI));

void
utf16_to_utf8 (sv, ...)
    SV* sv
        ALIAS:
            utf16_to_utf8_reversed = 1
    PREINIT:
        STRLEN len;
        U8 *source;
        SV *dest;
        Size_t got;
    CODE:
        source = (U8 *)SvPVbyte(sv, len);
        /* Optionally only convert part of the buffer.  */
        if (items > 1) {
            len = SvUV(ST(1));
        }
        /* Mortalise this right now, as we'll be testing croak()s  */
        dest = sv_2mortal(newSV(len * 2 + 1));
        if (ix) {
            utf16_to_utf8_reversed(source, (U8 *)SvPVX(dest), len, &got);
        } else {
            utf16_to_utf8(source, (U8 *)SvPVX(dest), len, &got);
        }
        SvCUR_set(dest, got);
        SvPVX(dest)[got] = '\0';
        SvPOK_on(dest);
        ST(0) = dest;
        XSRETURN(1);

void
utf8_to_utf16 (sv, ...)
    SV* sv
        ALIAS:
            utf8_to_utf16_reversed = 1
    PREINIT:
        STRLEN len;
        U8 *source;
        SV *dest;
        Size_t got;
    CODE:
        source = (U8 *)SvPV(sv, len);
        /* Optionally only convert part of the buffer.  */
        if (items > 1) {
            len = SvUV(ST(1));
        }
        /* Mortalise this right now, as we'll be testing croak()s  */
        dest = sv_2mortal(newSV(len * 2 + 1));
        if (ix) {
            utf8_to_utf16_reversed(source, (U8 *)SvPVX(dest), len, &got);
        } else {
            utf8_to_utf16(source, (U8 *)SvPVX(dest), len, &got);
        }
        SvCUR_set(dest, got);
        SvPVX(dest)[got] = '\0';
        SvPOK_on(dest);
        ST(0) = dest;
        XSRETURN(1);

void
my_exit(int exitcode)
        PPCODE:
        my_exit(exitcode);

U8
first_byte(sv)
        SV *sv
   CODE:
    char *s;
    STRLEN len;
        s = SvPVbyte(sv, len);
        RETVAL = s[0];
   OUTPUT:
    RETVAL

I32
sv_count()
        CODE:
            RETVAL = PL_sv_count;
        OUTPUT:
            RETVAL

void
bhk_record(bool on)
    CODE:
        dMY_CXT;
        MY_CXT.bhk_record = on;
        if (on)
            av_clear(MY_CXT.bhkav);

void
test_magic_chain()
    PREINIT:
        SV *sv;
        MAGIC *callmg, *uvarmg;
    CODE:
        sv = sv_2mortal(newSV(0));
        if (SvTYPE(sv) >= SVt_PVMG) croak_fail();
        if (SvMAGICAL(sv)) croak_fail();
        sv_magic(sv, &PL_sv_yes, PERL_MAGIC_checkcall, (char*)&callmg, 0);
        if (SvTYPE(sv) < SVt_PVMG) croak_fail();
        if (!SvMAGICAL(sv)) croak_fail();
        if (mg_find(sv, PERL_MAGIC_uvar)) croak_fail();
        callmg = mg_find(sv, PERL_MAGIC_checkcall);
        if (!callmg) croak_fail();
        if (callmg->mg_obj != &PL_sv_yes || callmg->mg_ptr != (char*)&callmg)
            croak_fail();
        sv_magic(sv, &PL_sv_no, PERL_MAGIC_uvar, (char*)&uvarmg, 0);
        if (SvTYPE(sv) < SVt_PVMG) croak_fail();
        if (!SvMAGICAL(sv)) croak_fail();
        if (mg_find(sv, PERL_MAGIC_checkcall) != callmg) croak_fail();
        uvarmg = mg_find(sv, PERL_MAGIC_uvar);
        if (!uvarmg) croak_fail();
        if (callmg->mg_obj != &PL_sv_yes || callmg->mg_ptr != (char*)&callmg)
            croak_fail();
        if (uvarmg->mg_obj != &PL_sv_no || uvarmg->mg_ptr != (char*)&uvarmg)
            croak_fail();
        mg_free_type(sv, PERL_MAGIC_vec);
        if (SvTYPE(sv) < SVt_PVMG) croak_fail();
        if (!SvMAGICAL(sv)) croak_fail();
        if (mg_find(sv, PERL_MAGIC_checkcall) != callmg) croak_fail();
        if (mg_find(sv, PERL_MAGIC_uvar) != uvarmg) croak_fail();
        if (callmg->mg_obj != &PL_sv_yes || callmg->mg_ptr != (char*)&callmg)
            croak_fail();
        if (uvarmg->mg_obj != &PL_sv_no || uvarmg->mg_ptr != (char*)&uvarmg)
            croak_fail();
        mg_free_type(sv, PERL_MAGIC_uvar);
        if (SvTYPE(sv) < SVt_PVMG) croak_fail();
        if (!SvMAGICAL(sv)) croak_fail();
        if (mg_find(sv, PERL_MAGIC_checkcall) != callmg) croak_fail();
        if (mg_find(sv, PERL_MAGIC_uvar)) croak_fail();
        if (callmg->mg_obj != &PL_sv_yes || callmg->mg_ptr != (char*)&callmg)
            croak_fail();
        sv_magic(sv, &PL_sv_no, PERL_MAGIC_uvar, (char*)&uvarmg, 0);
        if (SvTYPE(sv) < SVt_PVMG) croak_fail();
        if (!SvMAGICAL(sv)) croak_fail();
        if (mg_find(sv, PERL_MAGIC_checkcall) != callmg) croak_fail();
        uvarmg = mg_find(sv, PERL_MAGIC_uvar);
        if (!uvarmg) croak_fail();
        if (callmg->mg_obj != &PL_sv_yes || callmg->mg_ptr != (char*)&callmg)
            croak_fail();
        if (uvarmg->mg_obj != &PL_sv_no || uvarmg->mg_ptr != (char*)&uvarmg)
            croak_fail();
        mg_free_type(sv, PERL_MAGIC_checkcall);
        if (SvTYPE(sv) < SVt_PVMG) croak_fail();
        if (!SvMAGICAL(sv)) croak_fail();
        if (mg_find(sv, PERL_MAGIC_uvar) != uvarmg) croak_fail();
        if (mg_find(sv, PERL_MAGIC_checkcall)) croak_fail();
        if (uvarmg->mg_obj != &PL_sv_no || uvarmg->mg_ptr != (char*)&uvarmg)
            croak_fail();
        mg_free_type(sv, PERL_MAGIC_uvar);
        if (SvMAGICAL(sv)) croak_fail();
        if (mg_find(sv, PERL_MAGIC_checkcall)) croak_fail();
        if (mg_find(sv, PERL_MAGIC_uvar)) croak_fail();

void
test_op_contextualize()
    PREINIT:
        OP *o;
    CODE:
        o = newSVOP(OP_CONST, 0, newSViv(0));
        o->op_flags &= ~OPf_WANT;
        o = op_contextualize(o, G_SCALAR);
        if (o->op_type != OP_CONST ||
                (o->op_flags & OPf_WANT) != OPf_WANT_SCALAR)
            croak_fail();
        op_free(o);
        o = newSVOP(OP_CONST, 0, newSViv(0));
        o->op_flags &= ~OPf_WANT;
        o = op_contextualize(o, G_LIST);
        if (o->op_type != OP_CONST ||
                (o->op_flags & OPf_WANT) != OPf_WANT_LIST)
            croak_fail();
        op_free(o);
        o = newSVOP(OP_CONST, 0, newSViv(0));
        o->op_flags &= ~OPf_WANT;
        o = op_contextualize(o, G_VOID);
        if (o->op_type != OP_NULL) croak_fail();
        op_free(o);

void
test_rv2cv_op_cv()
    PROTOTYPE:
    PREINIT:
        GV *troc_gv;
        CV *troc_cv;
        OP *o;
    CODE:
        troc_gv = gv_fetchpv("XS::APItest::test_rv2cv_op_cv", 0, SVt_PVGV);
        troc_cv = get_cv("XS::APItest::test_rv2cv_op_cv", 0);
        o = newCVREF(0, newGVOP(OP_GV, 0, troc_gv));
        if (rv2cv_op_cv(o, 0) != troc_cv) croak_fail();
        if (rv2cv_op_cv(o, RV2CVOPCV_RETURN_NAME_GV) != (CV*)troc_gv)
            croak_fail();
        o->op_private |= OPpENTERSUB_AMPER;
        if (rv2cv_op_cv(o, 0)) croak_fail();
        if (rv2cv_op_cv(o, RV2CVOPCV_RETURN_NAME_GV)) croak_fail();
        o->op_private &= ~OPpENTERSUB_AMPER;
        if (cUNOPx(o)->op_first->op_private & OPpEARLY_CV) croak_fail();
        if (rv2cv_op_cv(o, RV2CVOPCV_MARK_EARLY) != troc_cv) croak_fail();
        if (cUNOPx(o)->op_first->op_private & OPpEARLY_CV) croak_fail();
        op_free(o);
        o = newSVOP(OP_CONST, 0, newSVpv("XS::APItest::test_rv2cv_op_cv", 0));
        o->op_private = OPpCONST_BARE;
        o = newCVREF(0, o);
        if (rv2cv_op_cv(o, 0) != troc_cv) croak_fail();
        if (rv2cv_op_cv(o, RV2CVOPCV_RETURN_NAME_GV) != (CV*)troc_gv)
            croak_fail();
        o->op_private |= OPpENTERSUB_AMPER;
        if (rv2cv_op_cv(o, 0)) croak_fail();
        if (rv2cv_op_cv(o, RV2CVOPCV_RETURN_NAME_GV)) croak_fail();
        op_free(o);
        o = newCVREF(0, newSVOP(OP_CONST, 0, newRV_inc((SV*)troc_cv)));
        if (rv2cv_op_cv(o, 0) != troc_cv) croak_fail();
        if (rv2cv_op_cv(o, RV2CVOPCV_RETURN_NAME_GV) != (CV*)troc_gv)
            croak_fail();
        o->op_private |= OPpENTERSUB_AMPER;
        if (rv2cv_op_cv(o, 0)) croak_fail();
        if (rv2cv_op_cv(o, RV2CVOPCV_RETURN_NAME_GV)) croak_fail();
        o->op_private &= ~OPpENTERSUB_AMPER;
        if (cUNOPx(o)->op_first->op_private & OPpEARLY_CV) croak_fail();
        if (rv2cv_op_cv(o, RV2CVOPCV_MARK_EARLY) != troc_cv) croak_fail();
        if (cUNOPx(o)->op_first->op_private & OPpEARLY_CV) croak_fail();
        op_free(o);
        o = newCVREF(0, newUNOP(OP_RAND, 0, newSVOP(OP_CONST, 0, newSViv(0))));
        if (rv2cv_op_cv(o, 0)) croak_fail();
        if (rv2cv_op_cv(o, RV2CVOPCV_RETURN_NAME_GV)) croak_fail();
        o->op_private |= OPpENTERSUB_AMPER;
        if (rv2cv_op_cv(o, 0)) croak_fail();
        if (rv2cv_op_cv(o, RV2CVOPCV_RETURN_NAME_GV)) croak_fail();
        o->op_private &= ~OPpENTERSUB_AMPER;
        if (cUNOPx(o)->op_first->op_private & OPpEARLY_CV) croak_fail();
        if (rv2cv_op_cv(o, RV2CVOPCV_MARK_EARLY)) croak_fail();
        if (cUNOPx(o)->op_first->op_private & OPpEARLY_CV) croak_fail();
        op_free(o);
        o = newUNOP(OP_RAND, 0, newSVOP(OP_CONST, 0, newSViv(0)));
        if (rv2cv_op_cv(o, 0)) croak_fail();
        if (rv2cv_op_cv(o, RV2CVOPCV_RETURN_NAME_GV)) croak_fail();
        op_free(o);

void
test_cv_getset_call_checker()
    PREINIT:
        CV *troc_cv, *tsh_cv;
        Perl_call_checker ckfun;
        SV *ckobj;
        U32 ckflags;
    CODE:
#define check_cc(cv, xckfun, xckobj, xckflags) \
    do { \
        cv_get_call_checker((cv), &ckfun, &ckobj); \
        if (ckfun != (xckfun)) croak_fail_nep(FPTR2DPTR(void *, ckfun), xckfun); \
        if (ckobj != (xckobj)) croak_fail_nep(FPTR2DPTR(void *, ckobj), xckobj); \
        cv_get_call_checker_flags((cv), CALL_CHECKER_REQUIRE_GV, &ckfun, &ckobj, &ckflags); \
        if (ckfun != (xckfun)) croak_fail_nep(FPTR2DPTR(void *, ckfun), xckfun); \
        if (ckobj != (xckobj)) croak_fail_nep(FPTR2DPTR(void *, ckobj), xckobj); \
        if (ckflags != CALL_CHECKER_REQUIRE_GV) croak_fail_nei(ckflags, CALL_CHECKER_REQUIRE_GV); \
        cv_get_call_checker_flags((cv), 0, &ckfun, &ckobj, &ckflags); \
        if (ckfun != (xckfun)) croak_fail_nep(FPTR2DPTR(void *, ckfun), xckfun); \
        if (ckobj != (xckobj)) croak_fail_nep(FPTR2DPTR(void *, ckobj), xckobj); \
        if (ckflags != (xckflags)) croak_fail_nei(ckflags, (xckflags)); \
    } while(0)
        troc_cv = get_cv("XS::APItest::test_rv2cv_op_cv", 0);
        tsh_cv = get_cv("XS::APItest::test_savehints", 0);
        check_cc(troc_cv, Perl_ck_entersub_args_proto_or_list, (SV*)troc_cv, 0);
        check_cc(tsh_cv, Perl_ck_entersub_args_proto_or_list, (SV*)tsh_cv, 0);
        cv_set_call_checker(tsh_cv, Perl_ck_entersub_args_proto_or_list,
                                    &PL_sv_yes);
        check_cc(troc_cv, Perl_ck_entersub_args_proto_or_list, (SV*)troc_cv, 0);
        check_cc(tsh_cv, Perl_ck_entersub_args_proto_or_list, &PL_sv_yes, CALL_CHECKER_REQUIRE_GV);
        cv_set_call_checker(troc_cv, THX_ck_entersub_args_scalars, &PL_sv_no);
        check_cc(troc_cv, THX_ck_entersub_args_scalars, &PL_sv_no, CALL_CHECKER_REQUIRE_GV);
        check_cc(tsh_cv, Perl_ck_entersub_args_proto_or_list, &PL_sv_yes, CALL_CHECKER_REQUIRE_GV);
        cv_set_call_checker(tsh_cv, Perl_ck_entersub_args_proto_or_list,
                                    (SV*)tsh_cv);
        check_cc(troc_cv, THX_ck_entersub_args_scalars, &PL_sv_no, CALL_CHECKER_REQUIRE_GV);
        check_cc(tsh_cv, Perl_ck_entersub_args_proto_or_list, (SV*)tsh_cv, 0);
        cv_set_call_checker(troc_cv, Perl_ck_entersub_args_proto_or_list,
                                    (SV*)troc_cv);
        check_cc(troc_cv, Perl_ck_entersub_args_proto_or_list, (SV*)troc_cv, 0);
        check_cc(tsh_cv, Perl_ck_entersub_args_proto_or_list, (SV*)tsh_cv, 0);
        if (SvMAGICAL((SV*)troc_cv) || SvMAGIC((SV*)troc_cv)) croak_fail();
        if (SvMAGICAL((SV*)tsh_cv) || SvMAGIC((SV*)tsh_cv)) croak_fail();
        cv_set_call_checker_flags(tsh_cv, Perl_ck_entersub_args_proto_or_list,
                                    &PL_sv_yes, 0);
        check_cc(tsh_cv, Perl_ck_entersub_args_proto_or_list, &PL_sv_yes, 0);
        cv_set_call_checker_flags(tsh_cv, Perl_ck_entersub_args_proto_or_list,
                                    &PL_sv_yes, CALL_CHECKER_REQUIRE_GV);
        check_cc(tsh_cv, Perl_ck_entersub_args_proto_or_list, &PL_sv_yes, CALL_CHECKER_REQUIRE_GV);
        cv_set_call_checker_flags(tsh_cv, Perl_ck_entersub_args_proto_or_list,
                                    (SV*)tsh_cv, 0);
        check_cc(tsh_cv, Perl_ck_entersub_args_proto_or_list, (SV*)tsh_cv, 0);
        if (SvMAGICAL((SV*)tsh_cv) || SvMAGIC((SV*)tsh_cv)) croak_fail();
        cv_set_call_checker_flags(tsh_cv, Perl_ck_entersub_args_proto_or_list,
                                    &PL_sv_yes, CALL_CHECKER_REQUIRE_GV);
        check_cc(tsh_cv, Perl_ck_entersub_args_proto_or_list, &PL_sv_yes, CALL_CHECKER_REQUIRE_GV);
        cv_set_call_checker_flags(tsh_cv, Perl_ck_entersub_args_proto_or_list,
                                    (SV*)tsh_cv, CALL_CHECKER_REQUIRE_GV);
        check_cc(tsh_cv, Perl_ck_entersub_args_proto_or_list, (SV*)tsh_cv, 0);
        if (SvMAGICAL((SV*)tsh_cv) || SvMAGIC((SV*)tsh_cv)) croak_fail();
#undef check_cc

void
cv_set_call_checker_lists(CV *cv)
    CODE:
        cv_set_call_checker(cv, THX_ck_entersub_args_lists, &PL_sv_undef);

void
cv_set_call_checker_scalars(CV *cv)
    CODE:
        cv_set_call_checker(cv, THX_ck_entersub_args_scalars, &PL_sv_undef);

void
cv_set_call_checker_proto(CV *cv, SV *proto)
    CODE:
        if (SvROK(proto))
            proto = SvRV(proto);
        cv_set_call_checker(cv, Perl_ck_entersub_args_proto, proto);

void
cv_set_call_checker_proto_or_list(CV *cv, SV *proto)
    CODE:
        if (SvROK(proto))
            proto = SvRV(proto);
        cv_set_call_checker(cv, Perl_ck_entersub_args_proto_or_list, proto);

void
cv_set_call_checker_multi_sum(CV *cv)
    CODE:
        cv_set_call_checker(cv, THX_ck_entersub_multi_sum, &PL_sv_undef);

void
test_cophh()
    PREINIT:
        COPHH *a, *b;
#ifdef EBCDIC
        SV* key_sv;
        char * key_name;
        STRLEN key_len;
#endif
    CODE:
#define check_ph(EXPR) \
            do { if((EXPR) != &PL_sv_placeholder) croak("fail"); } while(0)
#define check_iv(EXPR, EXPECT) \
            do { if(SvIV(EXPR) != (EXPECT)) croak("fail"); } while(0)
#define msvpvs(STR) sv_2mortal(newSVpvs(STR))
#define msviv(VALUE) sv_2mortal(newSViv(VALUE))
        a = cophh_new_empty();
        check_ph(cophh_fetch_pvn(a, "foo_1", 5, 0, 0));
        check_ph(cophh_fetch_pvs(a, "foo_1", 0));
        check_ph(cophh_fetch_pv(a, "foo_1", 0, 0));
        check_ph(cophh_fetch_sv(a, msvpvs("foo_1"), 0, 0));
        a = cophh_store_pvn(a, "foo_1abc", 5, 0, msviv(111), 0);
        a = cophh_store_pvs(a, "foo_2", msviv(222), 0);
        a = cophh_store_pv(a, "foo_3", 0, msviv(333), 0);
        a = cophh_store_sv(a, msvpvs("foo_4"), 0, msviv(444), 0);
        check_iv(cophh_fetch_pvn(a, "foo_1xyz", 5, 0, 0), 111);
        check_iv(cophh_fetch_pvs(a, "foo_1", 0), 111);
        check_iv(cophh_fetch_pv(a, "foo_1", 0, 0), 111);
        check_iv(cophh_fetch_sv(a, msvpvs("foo_1"), 0, 0), 111);
        check_iv(cophh_fetch_pvs(a, "foo_2", 0), 222);
        check_iv(cophh_fetch_pvs(a, "foo_3", 0), 333);
        check_iv(cophh_fetch_pvs(a, "foo_4", 0), 444);
        check_ph(cophh_fetch_pvs(a, "foo_5", 0));
        b = cophh_copy(a);
        b = cophh_store_pvs(b, "foo_1", msviv(1111), 0);
        check_iv(cophh_fetch_pvs(a, "foo_1", 0), 111);
        check_iv(cophh_fetch_pvs(a, "foo_2", 0), 222);
        check_iv(cophh_fetch_pvs(a, "foo_3", 0), 333);
        check_iv(cophh_fetch_pvs(a, "foo_4", 0), 444);
        check_ph(cophh_fetch_pvs(a, "foo_5", 0));
        check_iv(cophh_fetch_pvs(b, "foo_1", 0), 1111);
        check_iv(cophh_fetch_pvs(b, "foo_2", 0), 222);
        check_iv(cophh_fetch_pvs(b, "foo_3", 0), 333);
        check_iv(cophh_fetch_pvs(b, "foo_4", 0), 444);
        check_ph(cophh_fetch_pvs(b, "foo_5", 0));
        a = cophh_delete_pvn(a, "foo_1abc", 5, 0, 0);
        a = cophh_delete_pvs(a, "foo_2", 0);
        b = cophh_delete_pv(b, "foo_3", 0, 0);
        b = cophh_delete_sv(b, msvpvs("foo_4"), 0, 0);
        check_ph(cophh_fetch_pvs(a, "foo_1", 0));
        check_ph(cophh_fetch_pvs(a, "foo_2", 0));
        check_iv(cophh_fetch_pvs(a, "foo_3", 0), 333);
        check_iv(cophh_fetch_pvs(a, "foo_4", 0), 444);
        check_ph(cophh_fetch_pvs(a, "foo_5", 0));
        check_iv(cophh_fetch_pvs(b, "foo_1", 0), 1111);
        check_iv(cophh_fetch_pvs(b, "foo_2", 0), 222);
        check_ph(cophh_fetch_pvs(b, "foo_3", 0));
        check_ph(cophh_fetch_pvs(b, "foo_4", 0));
        check_ph(cophh_fetch_pvs(b, "foo_5", 0));
        b = cophh_delete_pvs(b, "foo_3", 0);
        b = cophh_delete_pvs(b, "foo_5", 0);
        check_iv(cophh_fetch_pvs(b, "foo_1", 0), 1111);
        check_iv(cophh_fetch_pvs(b, "foo_2", 0), 222);
        check_ph(cophh_fetch_pvs(b, "foo_3", 0));
        check_ph(cophh_fetch_pvs(b, "foo_4", 0));
        check_ph(cophh_fetch_pvs(b, "foo_5", 0));
        cophh_free(b);
        check_ph(cophh_fetch_pvs(a, "foo_1", 0));
        check_ph(cophh_fetch_pvs(a, "foo_2", 0));
        check_iv(cophh_fetch_pvs(a, "foo_3", 0), 333);
        check_iv(cophh_fetch_pvs(a, "foo_4", 0), 444);
        check_ph(cophh_fetch_pvs(a, "foo_5", 0));
        a = cophh_store_pvs(a, "foo_1", msviv(11111), COPHH_KEY_UTF8);
        a = cophh_store_pvs(a, "foo_\xaa", msviv(123), 0);
#ifndef EBCDIC
        a = cophh_store_pvs(a, "foo_\xc2\xbb", msviv(456), COPHH_KEY_UTF8);
#else
        /* On EBCDIC, we need to translate the UTF-8 in the ASCII test to the
         * equivalent UTF-EBCDIC for the code page.  This is done at runtime
         * (with the helper function in this file).  Therefore we can't use
         * cophhh_store_pvs(), as we don't have literal string */
        key_sv = sv_2mortal(newSVpvs("foo_"));
        cat_utf8a2n(key_sv, STR_WITH_LEN("\xc2\xbb"));
        key_name = SvPV(key_sv, key_len);
        a = cophh_store_pvn(a, key_name, key_len, 0, msviv(456), COPHH_KEY_UTF8);
#endif
#ifndef EBCDIC
        a = cophh_store_pvs(a, "foo_\xc3\x8c", msviv(789), COPHH_KEY_UTF8);
#else
        sv_setpvs(key_sv, "foo_");
        cat_utf8a2n(key_sv, STR_WITH_LEN("\xc3\x8c"));
        key_name = SvPV(key_sv, key_len);
        a = cophh_store_pvn(a, key_name, key_len, 0, msviv(789), COPHH_KEY_UTF8);
#endif
#ifndef EBCDIC
        a = cophh_store_pvs(a, "foo_\xd9\xa6", msviv(666), COPHH_KEY_UTF8);
#else
        sv_setpvs(key_sv, "foo_");
        cat_utf8a2n(key_sv, STR_WITH_LEN("\xd9\xa6"));
        key_name = SvPV(key_sv, key_len);
        a = cophh_store_pvn(a, key_name, key_len, 0, msviv(666), COPHH_KEY_UTF8);
#endif
        check_iv(cophh_fetch_pvs(a, "foo_1", 0), 11111);
        check_iv(cophh_fetch_pvs(a, "foo_1", COPHH_KEY_UTF8), 11111);
        check_iv(cophh_fetch_pvs(a, "foo_\xaa", 0), 123);
#ifndef EBCDIC
        check_iv(cophh_fetch_pvs(a, "foo_\xc2\xaa", COPHH_KEY_UTF8), 123);
        check_ph(cophh_fetch_pvs(a, "foo_\xc2\xaa", 0));
#else
        sv_setpvs(key_sv, "foo_");
        cat_utf8a2n(key_sv, STR_WITH_LEN("\xc2\xaa"));
        key_name = SvPV(key_sv, key_len);
        check_iv(cophh_fetch_pvn(a, key_name, key_len, 0, COPHH_KEY_UTF8), 123);
        check_ph(cophh_fetch_pvn(a, key_name, key_len, 0, 0));
#endif
        check_iv(cophh_fetch_pvs(a, "foo_\xbb", 0), 456);
#ifndef EBCDIC
        check_iv(cophh_fetch_pvs(a, "foo_\xc2\xbb", COPHH_KEY_UTF8), 456);
        check_ph(cophh_fetch_pvs(a, "foo_\xc2\xbb", 0));
#else
        sv_setpvs(key_sv, "foo_");
        cat_utf8a2n(key_sv, STR_WITH_LEN("\xc2\xbb"));
        key_name = SvPV(key_sv, key_len);
        check_iv(cophh_fetch_pvn(a, key_name, key_len, 0, COPHH_KEY_UTF8), 456);
        check_ph(cophh_fetch_pvn(a, key_name, key_len, 0, 0));
#endif
        check_iv(cophh_fetch_pvs(a, "foo_\xcc", 0), 789);
#ifndef EBCDIC
        check_iv(cophh_fetch_pvs(a, "foo_\xc3\x8c", COPHH_KEY_UTF8), 789);
        check_ph(cophh_fetch_pvs(a, "foo_\xc2\x8c", 0));
#else
        sv_setpvs(key_sv, "foo_");
        cat_utf8a2n(key_sv, STR_WITH_LEN("\xc3\x8c"));
        key_name = SvPV(key_sv, key_len);
        check_iv(cophh_fetch_pvn(a, key_name, key_len, 0, COPHH_KEY_UTF8), 789);
        check_ph(cophh_fetch_pvn(a, key_name, key_len, 0, 0));
#endif
#ifndef EBCDIC
        check_iv(cophh_fetch_pvs(a, "foo_\xd9\xa6", COPHH_KEY_UTF8), 666);
        check_ph(cophh_fetch_pvs(a, "foo_\xd9\xa6", 0));
#else
        sv_setpvs(key_sv, "foo_");
        cat_utf8a2n(key_sv, STR_WITH_LEN("\xd9\xa6"));
        key_name = SvPV(key_sv, key_len);
        check_iv(cophh_fetch_pvn(a, key_name, key_len, 0, COPHH_KEY_UTF8), 666);
        check_ph(cophh_fetch_pvn(a, key_name, key_len, 0, 0));
#endif
        ENTER;
        SAVEFREECOPHH(a);
        LEAVE;
#undef check_ph
#undef check_iv
#undef msvpvs
#undef msviv

void
test_coplabel()
    PREINIT:
        COP *cop;
        const char *label;
        STRLEN len;
        U32 utf8;
    CODE:
        cop = &PL_compiling;
        Perl_cop_store_label(aTHX_ cop, "foo", 3, 0);
        label = Perl_cop_fetch_label(aTHX_ cop, &len, &utf8);
        if (strNE(label,"foo")) croak("fail # cop_fetch_label label");
        if (len != 3) croak("fail # cop_fetch_label len");
        if (utf8) croak("fail # cop_fetch_label utf8");
        /* SMALL GERMAN UMLAUT A */
        Perl_cop_store_label(aTHX_ cop, "fo\xc3\xa4", 4, SVf_UTF8);
        label = Perl_cop_fetch_label(aTHX_ cop, &len, &utf8);
        if (strNE(label,"fo\xc3\xa4")) croak("fail # cop_fetch_label label");
        if (len != 4) croak("fail # cop_fetch_label len");
        if (!utf8) croak("fail # cop_fetch_label utf8");


HV *
example_cophh_2hv()
    PREINIT:
        COPHH *a;
#ifdef EBCDIC
        SV* key_sv;
        char * key_name;
        STRLEN key_len;
#endif
    CODE:
#define msviv(VALUE) sv_2mortal(newSViv(VALUE))
        a = cophh_new_empty();
        a = cophh_store_pvs(a, "foo_0", msviv(999), 0);
        a = cophh_store_pvs(a, "foo_1", msviv(111), 0);
        a = cophh_store_pvs(a, "foo_\xaa", msviv(123), 0);
#ifndef EBCDIC
        a = cophh_store_pvs(a, "foo_\xc2\xbb", msviv(456), COPHH_KEY_UTF8);
#else
        key_sv = sv_2mortal(newSVpvs("foo_"));
        cat_utf8a2n(key_sv, STR_WITH_LEN("\xc2\xbb"));
        key_name = SvPV(key_sv, key_len);
        a = cophh_store_pvn(a, key_name, key_len, 0, msviv(456), COPHH_KEY_UTF8);
#endif
#ifndef EBCDIC
        a = cophh_store_pvs(a, "foo_\xc3\x8c", msviv(789), COPHH_KEY_UTF8);
#else
        sv_setpvs(key_sv, "foo_");
        cat_utf8a2n(key_sv, STR_WITH_LEN("\xc3\x8c"));
        key_name = SvPV(key_sv, key_len);
        a = cophh_store_pvn(a, key_name, key_len, 0, msviv(789), COPHH_KEY_UTF8);
#endif
#ifndef EBCDIC
        a = cophh_store_pvs(a, "foo_\xd9\xa6", msviv(666), COPHH_KEY_UTF8);
#else
        sv_setpvs(key_sv, "foo_");
        cat_utf8a2n(key_sv, STR_WITH_LEN("\xd9\xa6"));
        key_name = SvPV(key_sv, key_len);
        a = cophh_store_pvn(a, key_name, key_len, 0, msviv(666), COPHH_KEY_UTF8);
#endif
        a = cophh_delete_pvs(a, "foo_0", 0);
        a = cophh_delete_pvs(a, "foo_2", 0);
        RETVAL = cophh_2hv(a, 0);
        cophh_free(a);
#undef msviv
    OUTPUT:
        RETVAL

void
test_savehints()
    PREINIT:
        SV **svp, *sv;
    CODE:
#define store_hint(KEY, VALUE) \
                sv_setiv_mg(*hv_fetchs(GvHV(PL_hintgv), KEY, 1), (VALUE))
#define hint_ok(KEY, EXPECT) \
                ((svp = hv_fetchs(GvHV(PL_hintgv), KEY, 0)) && \
                    (sv = *svp) && SvIV(sv) == (EXPECT) && \
                    (sv = cop_hints_fetch_pvs(&PL_compiling, KEY, 0)) && \
                    SvIV(sv) == (EXPECT))
#define check_hint(KEY, EXPECT) \
                do { if (!hint_ok(KEY, EXPECT)) croak_fail(); } while(0)
        PL_hints |= HINT_LOCALIZE_HH;
        ENTER;
        SAVEHINTS();
        PL_hints &= HINT_INTEGER;
        store_hint("t0", 123);
        store_hint("t1", 456);
        if (PL_hints & HINT_INTEGER) croak_fail();
        check_hint("t0", 123); check_hint("t1", 456);
        ENTER;
        SAVEHINTS();
        if (PL_hints & HINT_INTEGER) croak_fail();
        check_hint("t0", 123); check_hint("t1", 456);
        PL_hints |= HINT_INTEGER;
        store_hint("t0", 321);
        if (!(PL_hints & HINT_INTEGER)) croak_fail();
        check_hint("t0", 321); check_hint("t1", 456);
        LEAVE;
        if (PL_hints & HINT_INTEGER) croak_fail();
        check_hint("t0", 123); check_hint("t1", 456);
        ENTER;
        SAVEHINTS();
        if (PL_hints & HINT_INTEGER) croak_fail();
        check_hint("t0", 123); check_hint("t1", 456);
        store_hint("t1", 654);
        if (PL_hints & HINT_INTEGER) croak_fail();
        check_hint("t0", 123); check_hint("t1", 654);
        LEAVE;
        if (PL_hints & HINT_INTEGER) croak_fail();
        check_hint("t0", 123); check_hint("t1", 456);
        LEAVE;
#undef store_hint
#undef hint_ok
#undef check_hint

void
test_copyhints()
    PREINIT:
        HV *a, *b;
    CODE:
        PL_hints |= HINT_LOCALIZE_HH;
        ENTER;
        SAVEHINTS();
        sv_setiv_mg(*hv_fetchs(GvHV(PL_hintgv), "t0", 1), 123);
        if (SvIV(cop_hints_fetch_pvs(&PL_compiling, "t0", 0)) != 123)
            croak_fail();
        a = newHVhv(GvHV(PL_hintgv));
        sv_2mortal((SV*)a);
        sv_setiv_mg(*hv_fetchs(a, "t0", 1), 456);
        if (SvIV(cop_hints_fetch_pvs(&PL_compiling, "t0", 0)) != 123)
            croak_fail();
        b = hv_copy_hints_hv(a);
        sv_2mortal((SV*)b);
        sv_setiv_mg(*hv_fetchs(b, "t0", 1), 789);
        if (SvIV(cop_hints_fetch_pvs(&PL_compiling, "t0", 0)) != 789)
            croak_fail();
        LEAVE;

void
test_op_list()
    PREINIT:
        OP *a;
    CODE:
#define iv_op(iv) newSVOP(OP_CONST, 0, newSViv(iv))
#define check_op(o, expect) \
    do { \
        if (strNE(test_op_list_describe(o), (expect))) \
            croak("fail %s %s", test_op_list_describe(o), (expect)); \
    } while(0)
        a = op_append_elem(OP_LIST, NULL, NULL);
        check_op(a, "");
        a = op_append_elem(OP_LIST, iv_op(1), a);
        check_op(a, "const(1).");
        a = op_append_elem(OP_LIST, NULL, a);
        check_op(a, "const(1).");
        a = op_append_elem(OP_LIST, a, iv_op(2));
        check_op(a, "list[pushmark.const(1).const(2).]");
        a = op_append_elem(OP_LIST, a, iv_op(3));
        check_op(a, "list[pushmark.const(1).const(2).const(3).]");
        a = op_append_elem(OP_LIST, a, NULL);
        check_op(a, "list[pushmark.const(1).const(2).const(3).]");
        a = op_append_elem(OP_LIST, NULL, a);
        check_op(a, "list[pushmark.const(1).const(2).const(3).]");
        a = op_append_elem(OP_LIST, iv_op(4), a);
        check_op(a, "list[pushmark.const(4)."
                "list[pushmark.const(1).const(2).const(3).]]");
        a = op_append_elem(OP_LIST, a, iv_op(5));
        check_op(a, "list[pushmark.const(4)."
                "list[pushmark.const(1).const(2).const(3).]const(5).]");
        a = op_append_elem(OP_LIST, a,
                op_append_elem(OP_LIST, iv_op(7), iv_op(6)));
        check_op(a, "list[pushmark.const(4)."
                "list[pushmark.const(1).const(2).const(3).]const(5)."
                "list[pushmark.const(7).const(6).]]");
        op_free(a);
        a = op_append_elem(OP_LINESEQ, iv_op(1), iv_op(2));
        check_op(a, "lineseq[const(1).const(2).]");
        a = op_append_elem(OP_LINESEQ, a, iv_op(3));
        check_op(a, "lineseq[const(1).const(2).const(3).]");
        op_free(a);
        a = op_append_elem(OP_LINESEQ,
                op_append_elem(OP_LIST, iv_op(1), iv_op(2)),
                iv_op(3));
        check_op(a, "lineseq[list[pushmark.const(1).const(2).]const(3).]");
        op_free(a);
        a = op_prepend_elem(OP_LIST, NULL, NULL);
        check_op(a, "");
        a = op_prepend_elem(OP_LIST, a, iv_op(1));
        check_op(a, "const(1).");
        a = op_prepend_elem(OP_LIST, a, NULL);
        check_op(a, "const(1).");
        a = op_prepend_elem(OP_LIST, iv_op(2), a);
        check_op(a, "list[pushmark.const(2).const(1).]");
        a = op_prepend_elem(OP_LIST, iv_op(3), a);
        check_op(a, "list[pushmark.const(3).const(2).const(1).]");
        a = op_prepend_elem(OP_LIST, NULL, a);
        check_op(a, "list[pushmark.const(3).const(2).const(1).]");
        a = op_prepend_elem(OP_LIST, a, NULL);
        check_op(a, "list[pushmark.const(3).const(2).const(1).]");
        a = op_prepend_elem(OP_LIST, a, iv_op(4));
        check_op(a, "list[pushmark."
                "list[pushmark.const(3).const(2).const(1).]const(4).]");
        a = op_prepend_elem(OP_LIST, iv_op(5), a);
        check_op(a, "list[pushmark.const(5)."
                "list[pushmark.const(3).const(2).const(1).]const(4).]");
        a = op_prepend_elem(OP_LIST,
                op_prepend_elem(OP_LIST, iv_op(6), iv_op(7)), a);
        check_op(a, "list[pushmark.list[pushmark.const(6).const(7).]const(5)."
                "list[pushmark.const(3).const(2).const(1).]const(4).]");
        op_free(a);
        a = op_prepend_elem(OP_LINESEQ, iv_op(2), iv_op(1));
        check_op(a, "lineseq[const(2).const(1).]");
        a = op_prepend_elem(OP_LINESEQ, iv_op(3), a);
        check_op(a, "lineseq[const(3).const(2).const(1).]");
        op_free(a);
        a = op_prepend_elem(OP_LINESEQ, iv_op(3),
                op_prepend_elem(OP_LIST, iv_op(2), iv_op(1)));
        check_op(a, "lineseq[const(3).list[pushmark.const(2).const(1).]]");
        op_free(a);
        a = op_append_list(OP_LINESEQ, NULL, NULL);
        check_op(a, "");
        a = op_append_list(OP_LINESEQ, iv_op(1), a);
        check_op(a, "const(1).");
        a = op_append_list(OP_LINESEQ, NULL, a);
        check_op(a, "const(1).");
        a = op_append_list(OP_LINESEQ, a, iv_op(2));
        check_op(a, "lineseq[const(1).const(2).]");
        a = op_append_list(OP_LINESEQ, a, iv_op(3));
        check_op(a, "lineseq[const(1).const(2).const(3).]");
        a = op_append_list(OP_LINESEQ, iv_op(4), a);
        check_op(a, "lineseq[const(4).const(1).const(2).const(3).]");
        a = op_append_list(OP_LINESEQ, a, NULL);
        check_op(a, "lineseq[const(4).const(1).const(2).const(3).]");
        a = op_append_list(OP_LINESEQ, NULL, a);
        check_op(a, "lineseq[const(4).const(1).const(2).const(3).]");
        a = op_append_list(OP_LINESEQ, a,
                op_append_list(OP_LINESEQ, iv_op(5), iv_op(6)));
        check_op(a, "lineseq[const(4).const(1).const(2).const(3)."
                "const(5).const(6).]");
        op_free(a);
        a = op_append_list(OP_LINESEQ,
                op_append_list(OP_LINESEQ, iv_op(1), iv_op(2)),
                op_append_list(OP_LIST, iv_op(3), iv_op(4)));
        check_op(a, "lineseq[const(1).const(2)."
                "list[pushmark.const(3).const(4).]]");
        op_free(a);
        a = op_append_list(OP_LINESEQ,
                op_append_list(OP_LIST, iv_op(1), iv_op(2)),
                op_append_list(OP_LINESEQ, iv_op(3), iv_op(4)));
        check_op(a, "lineseq[list[pushmark.const(1).const(2).]"
                "const(3).const(4).]");
        op_free(a);
#undef check_op

void
test_op_linklist ()
    PREINIT:
        OP *o;
    CODE:
#define check_ll(o, expect) \
    STMT_START { \
        if (strNE(test_op_linklist_describe(o), (expect))) \
            croak("fail %s %s", test_op_linklist_describe(o), (expect)); \
    } STMT_END
        o = iv_op(1);
        check_ll(o, ".const1");
        op_free(o);

        o = mkUNOP(OP_NOT, iv_op(1));
        check_ll(o, ".const1.not");
        op_free(o);

        o = mkUNOP(OP_NOT, mkUNOP(OP_NEGATE, iv_op(1)));
        check_ll(o, ".const1.negate.not");
        op_free(o);

        o = mkBINOP(OP_ADD, iv_op(1), iv_op(2));
        check_ll(o, ".const1.const2.add");
        op_free(o);

        o = mkBINOP(OP_ADD, mkUNOP(OP_NOT, iv_op(1)), iv_op(2));
        check_ll(o, ".const1.not.const2.add");
        op_free(o);

        o = mkUNOP(OP_NOT, mkBINOP(OP_ADD, iv_op(1), iv_op(2)));
        check_ll(o, ".const1.const2.add.not");
        op_free(o);

        o = mkLISTOP(OP_LINESEQ, iv_op(1), iv_op(2), iv_op(3));
        check_ll(o, ".const1.const2.const3.lineseq");
        op_free(o);

        o = mkLISTOP(OP_LINESEQ,
                mkBINOP(OP_ADD, iv_op(1), iv_op(2)),
                mkUNOP(OP_NOT, iv_op(3)),
                mkLISTOP(OP_SUBSTR, iv_op(4), iv_op(5), iv_op(6)));
        check_ll(o, ".const1.const2.add.const3.not"
                    ".const4.const5.const6.substr.lineseq");
        op_free(o);

        o = mkBINOP(OP_ADD, iv_op(1), iv_op(2));
        LINKLIST(o);
        o = mkBINOP(OP_SUBTRACT, o, iv_op(3));
        check_ll(o, ".const1.const2.add.const3.subtract");
        op_free(o);
#undef check_ll
#undef iv_op

void
peep_enable ()
    PREINIT:
        dMY_CXT;
    CODE:
        av_clear(MY_CXT.peep_recorder);
        av_clear(MY_CXT.rpeep_recorder);
        MY_CXT.peep_recording = 1;

void
peep_disable ()
    PREINIT:
        dMY_CXT;
    CODE:
        MY_CXT.peep_recording = 0;

SV *
peep_record ()
    PREINIT:
        dMY_CXT;
    CODE:
        RETVAL = newRV_inc((SV *)MY_CXT.peep_recorder);
    OUTPUT:
        RETVAL

SV *
rpeep_record ()
    PREINIT:
        dMY_CXT;
    CODE:
        RETVAL = newRV_inc((SV *)MY_CXT.rpeep_recorder);
    OUTPUT:
        RETVAL

=pod

multicall_each: call a sub for each item in the list. Used to test MULTICALL

=cut

void
multicall_each(block,...)
    SV * block
PROTOTYPE: &@
CODE:
{
    dMULTICALL;
    int index;
    GV *gv;
    HV *stash;
    I32 gimme = G_SCALAR;
    SV **args = &PL_stack_base[ax];
    CV *cv;

    if(items <= 1) {
        XSRETURN_UNDEF;
    }
    cv = sv_2cv(block, &stash, &gv, 0);
    if (cv == Nullcv) {
       croak("multicall_each: not a subroutine reference");
    }
    PUSH_MULTICALL(cv);
    SAVESPTR(GvSV(PL_defgv));

    for(index = 1 ; index < items ; index++) {
        GvSV(PL_defgv) = args[index];
        MULTICALL;
    }
    POP_MULTICALL;
    XSRETURN_UNDEF;
}

=pod

multicall_return(): call the passed sub once in the specificed context
and return whatever it returns

=cut

void
multicall_return(block, context)
    SV *block
    I32 context
PROTOTYPE: &$
CODE:
{
    dSP;
    dMULTICALL;
    GV *gv;
    HV *stash;
    I32 gimme = context;
    CV *cv;
    AV *av;
    SV **p;
    SSize_t i, size;

    cv = sv_2cv(block, &stash, &gv, 0);
    if (cv == Nullcv) {
       croak("multicall_return not a subroutine reference");
    }
    PUSH_MULTICALL(cv);

    MULTICALL;

    /* copy returned values into an array so they're not freed during
     * POP_MULTICALL */

    av = newAV();
    SPAGAIN;

    switch (context) {
    case G_VOID:
        break;

    case G_SCALAR:
        av_push(av, SvREFCNT_inc(TOPs));
        break;

    case G_LIST:
        for (p = PL_stack_base + 1; p <= SP; p++)
            av_push(av, SvREFCNT_inc(*p));
        break;
    }

    POP_MULTICALL;

    size = AvFILLp(av) + 1;
    EXTEND(SP, size);
    for (i = 0; i < size; i++)
        ST(i) = *av_fetch(av, i, FALSE);
    sv_2mortal((SV*)av);
    XSRETURN(size);
}


#ifdef USE_ITHREADS

void
clone_with_stack()
CODE:
{
    PerlInterpreter *interp = aTHX; /* The original interpreter */
    PerlInterpreter *interp_dup;    /* The duplicate interpreter */
    int oldscope = 1; /* We are responsible for all scopes */

    interp_dup = perl_clone(interp, CLONEf_COPY_STACKS | CLONEf_CLONE_HOST );

    /* destroy old perl */
    PERL_SET_CONTEXT(interp);

    POPSTACK_TO(PL_mainstack);
    if (cxstack_ix >= 0) {
        dounwind(-1);
        cx_popblock(cxstack);
    }
    LEAVE_SCOPE(0);
    PL_scopestack_ix = oldscope;
    FREETMPS;

    perl_destruct(interp);
    perl_free(interp);

    /* switch to new perl */
    PERL_SET_CONTEXT(interp_dup);

    /* continue after 'clone_with_stack' */
    if (interp_dup->Iop)
        interp_dup->Iop = interp_dup->Iop->op_next;

    /* run with new perl */
    Perl_runops_standard(interp_dup);

    /* We may have additional unclosed scopes if fork() was called
     * from within a BEGIN block.  See perlfork.pod for more details.
     * We cannot clean up these other scopes because they belong to a
     * different interpreter, but we also cannot leave PL_scopestack_ix
     * dangling because that can trigger an assertion in perl_destruct().
     */
    if (PL_scopestack_ix > oldscope) {
        PL_scopestack[oldscope-1] = PL_scopestack[PL_scopestack_ix-1];
        PL_scopestack_ix = oldscope;
    }

    perl_destruct(interp_dup);
    perl_free(interp_dup);

    /* call the real 'exit' not PerlProc_exit */
#undef exit
    exit(0);
}

#endif /* USE_ITHREADS */

SV*
take_svref(SVREF sv)
CODE:
    RETVAL = newRV_inc(sv);
OUTPUT:
    RETVAL

SV*
take_avref(AV* av)
CODE:
    RETVAL = newRV_inc((SV*)av);
OUTPUT:
    RETVAL

SV*
take_hvref(HV* hv)
CODE:
    RETVAL = newRV_inc((SV*)hv);
OUTPUT:
    RETVAL


SV*
take_cvref(CV* cv)
CODE:
    RETVAL = newRV_inc((SV*)cv);
OUTPUT:
    RETVAL


BOOT:
        {
        HV* stash;
        SV** meth = NULL;
        CV* cv;
        stash = gv_stashpv("XS::APItest::TempLv", 0);
        if (stash)
            meth = hv_fetchs(stash, "make_temp_mg_lv", 0);
        if (!meth)
            croak("lost method 'make_temp_mg_lv'");
        cv = GvCV(*meth);
        CvLVALUE_on(cv);
        }

BOOT:
{
    hintkey_rpn_sv = newSVpvs_share("XS::APItest/rpn");
    hintkey_calcrpn_sv = newSVpvs_share("XS::APItest/calcrpn");
    hintkey_stufftest_sv = newSVpvs_share("XS::APItest/stufftest");
    hintkey_swaptwostmts_sv = newSVpvs_share("XS::APItest/swaptwostmts");
    hintkey_looprest_sv = newSVpvs_share("XS::APItest/looprest");
    hintkey_scopelessblock_sv = newSVpvs_share("XS::APItest/scopelessblock");
    hintkey_stmtasexpr_sv = newSVpvs_share("XS::APItest/stmtasexpr");
    hintkey_stmtsasexpr_sv = newSVpvs_share("XS::APItest/stmtsasexpr");
    hintkey_loopblock_sv = newSVpvs_share("XS::APItest/loopblock");
    hintkey_blockasexpr_sv = newSVpvs_share("XS::APItest/blockasexpr");
    hintkey_swaplabel_sv = newSVpvs_share("XS::APItest/swaplabel");
    hintkey_labelconst_sv = newSVpvs_share("XS::APItest/labelconst");
    hintkey_arrayfullexpr_sv = newSVpvs_share("XS::APItest/arrayfullexpr");
    hintkey_arraylistexpr_sv = newSVpvs_share("XS::APItest/arraylistexpr");
    hintkey_arraytermexpr_sv = newSVpvs_share("XS::APItest/arraytermexpr");
    hintkey_arrayarithexpr_sv = newSVpvs_share("XS::APItest/arrayarithexpr");
    hintkey_arrayexprflags_sv = newSVpvs_share("XS::APItest/arrayexprflags");
    hintkey_subsignature_sv = newSVpvs_share("XS::APItest/subsignature");
    hintkey_DEFSV_sv = newSVpvs_share("XS::APItest/DEFSV");
    hintkey_with_vars_sv = newSVpvs_share("XS::APItest/with_vars");
    hintkey_join_with_space_sv = newSVpvs_share("XS::APItest/join_with_space");
    wrap_keyword_plugin(my_keyword_plugin, &next_keyword_plugin);
}

void
establish_cleanup(...)
PROTOTYPE: $
CODE:
    PERL_UNUSED_VAR(items);
    croak("establish_cleanup called as a function");

BOOT:
{
    CV *estcv = get_cv("XS::APItest::establish_cleanup", 0);
    cv_set_call_checker(estcv, THX_ck_entersub_establish_cleanup, (SV*)estcv);
}

void
postinc(...)
PROTOTYPE: $
CODE:
    PERL_UNUSED_VAR(items);
    croak("postinc called as a function");

void
filter()
CODE:
    filter_add(filter_call, NULL);

BOOT:
{
    CV *asscv = get_cv("XS::APItest::postinc", 0);
    cv_set_call_checker(asscv, THX_ck_entersub_postinc, (SV*)asscv);
}

SV *
lv_temp_object()
CODE:
    RETVAL =
          sv_bless(
            newRV_noinc(newSV(0)),
            gv_stashpvs("XS::APItest::TempObj",GV_ADD)
          );             /* Package defined in test script */
OUTPUT:
    RETVAL

void
fill_hash_with_nulls(HV *hv)
PREINIT:
    UV i = 0;
CODE:
    for(; i < 1000; ++i) {
        HE *entry = hv_fetch_ent(hv, sv_2mortal(newSVuv(i)), 1, 0);
        SvREFCNT_dec(HeVAL(entry));
        HeVAL(entry) = NULL;
    }

HV *
newHVhv(HV *hv)
CODE:
    RETVAL = newHVhv(hv);
OUTPUT:
    RETVAL

U32
SvIsCOW(SV *sv)
CODE:
    RETVAL = SvIsCOW(sv);
OUTPUT:
    RETVAL

void
pad_scalar(...)
PROTOTYPE: $$
CODE:
    PERL_UNUSED_VAR(items);
    croak("pad_scalar called as a function");

BOOT:
{
    CV *pscv = get_cv("XS::APItest::pad_scalar", 0);
    cv_set_call_checker(pscv, THX_ck_entersub_pad_scalar, (SV*)pscv);
}

SV*
fetch_pad_names( cv )
CV* cv
 PREINIT:
  I32 i;
  PADNAMELIST *pad_namelist;
  AV *retav = newAV();
 CODE:
  pad_namelist = PadlistNAMES(CvPADLIST(cv));

  for ( i = PadnamelistMAX(pad_namelist); i >= 0; i-- ) {
    PADNAME* name = PadnamelistARRAY(pad_namelist)[i];

    if (PadnameLEN(name)) {
        av_push(retav, newSVpadname(name));
    }
  }
  RETVAL = newRV_noinc((SV*)retav);
 OUTPUT:
  RETVAL

STRLEN
underscore_length()
PROTOTYPE:
PREINIT:
    SV *u;
    U8 *pv;
    STRLEN bytelen;
CODE:
    u = find_rundefsv();
    pv = (U8*)SvPV(u, bytelen);
    RETVAL = SvUTF8(u) ? utf8_length(pv, pv+bytelen) : bytelen;
OUTPUT:
    RETVAL

void
stringify(SV *sv)
CODE:
    (void)SvPV_nolen(sv);

SV *
HvENAME(HV *hv)
CODE:
    RETVAL = hv && HvHasENAME(hv)
              ? newSVpvn_flags(
                  HvENAME(hv),HvENAMELEN(hv),
                  (HvENAMEUTF8(hv) ? SVf_UTF8 : 0)
                )
              : NULL;
OUTPUT:
    RETVAL

int
xs_cmp(int a, int b)
CODE:
    /* Odd sorting (odd numbers first), to make sure we are actually
       being called */
    RETVAL = a % 2 != b % 2
               ? a % 2 ? -1 : 1
               : a < b ? -1 : a == b ? 0 : 1;
OUTPUT:
    RETVAL

SV *
xs_cmp_undef(SV *a, SV *b)
CODE:
    PERL_UNUSED_ARG(a);
    PERL_UNUSED_ARG(b);
    RETVAL = &PL_sv_undef;
OUTPUT:
    RETVAL

char *
SvPVbyte(SV *sv, OUT STRLEN len)
CODE:
    RETVAL = SvPVbyte(sv, len);
OUTPUT:
    RETVAL

char *
SvPVbyte_nolen(SV *sv)
CODE:
    RETVAL = SvPVbyte_nolen(sv);
OUTPUT:
    RETVAL

char *
SvPVbyte_nomg(SV *sv, OUT STRLEN len)
CODE:
    RETVAL = SvPVbyte_nomg(sv, len);
OUTPUT:
    RETVAL

char *
SvPVutf8(SV *sv, OUT STRLEN len)
CODE:
    RETVAL = SvPVutf8(sv, len);
OUTPUT:
    RETVAL

char *
SvPVutf8_nolen(SV *sv)
CODE:
    RETVAL = SvPVutf8_nolen(sv);
OUTPUT:
    RETVAL

char *
SvPVutf8_nomg(SV *sv, OUT STRLEN len)
CODE:
    RETVAL = SvPVutf8_nomg(sv, len);
OUTPUT:
    RETVAL

bool
SvIsBOOL(SV *sv)
CODE:
    RETVAL = SvIsBOOL(sv);
OUTPUT:
    RETVAL

void
setup_addissub()
CODE:
    wrap_op_checker(OP_ADD, addissub_myck_add, &addissub_nxck_add);

void
setup_rv2cv_addunderbar()
CODE:
    wrap_op_checker(OP_RV2CV, my_ck_rv2cv, &old_ck_rv2cv);

#ifdef USE_ITHREADS

bool
test_alloccopstash()
CODE:
    RETVAL = PL_stashpad[alloccopstash(PL_defstash)] == PL_defstash;
OUTPUT:
    RETVAL

#endif

bool
test_newFOROP_without_slab()
CODE:
    {
        const I32 floor = start_subparse(0,0);
        OP *o;
        /* The slab allocator does not like CvROOT being set. */
        CvROOT(PL_compcv) = (OP *)1;
        o = newFOROP(0, 0, newOP(OP_PUSHMARK, 0), 0, 0);
        if (cLOOPx(cUNOPo->op_first)->op_last->op_sibparent
                != cUNOPo->op_first)
        {
            Perl_warn(aTHX_ "Op parent pointer is stale");
            RETVAL = FALSE;
        }
        else
            /* If we do not crash before returning, the test passes. */
            RETVAL = TRUE;
        op_free(o);
        CvROOT(PL_compcv) = NULL;
        SvREFCNT_dec(PL_compcv);
        LEAVE_SCOPE(floor);
    }
OUTPUT:
    RETVAL

 # provide access to CALLREGEXEC, except replace pointers within the
 # string with offsets from the start of the string

I32
callregexec(SV *prog, STRLEN stringarg, STRLEN strend, I32 minend, SV *sv, U32 nosave)
CODE:
    {
        STRLEN len;
        char *strbeg;
        if (SvROK(prog))
            prog = SvRV(prog);
        strbeg = SvPV_force(sv, len);
        RETVAL = CALLREGEXEC((REGEXP *)prog,
                            strbeg + stringarg,
                            strbeg + strend,
                            strbeg,
                            minend,
                            sv,
                            NULL, /* data */
                            nosave);
    }
OUTPUT:
    RETVAL

void
lexical_import(SV *name, CV *cv)
    CODE:
    {
        PADLIST *pl;
        PADOFFSET off;
        if (!PL_compcv)
            Perl_croak(aTHX_
                      "lexical_import can only be called at compile time");
        pl = CvPADLIST(PL_compcv);
        ENTER;
        SAVESPTR(PL_comppad_name); PL_comppad_name = PadlistNAMES(pl);
        SAVESPTR(PL_comppad);      PL_comppad      = PadlistARRAY(pl)[1];
        SAVESPTR(PL_curpad);       PL_curpad       = PadARRAY(PL_comppad);
        off = pad_add_name_sv(sv_2mortal(newSVpvf("&%" SVf,name)),
                              padadd_STATE, 0, 0);
        SvREFCNT_dec(PL_curpad[off]);
        PL_curpad[off] = SvREFCNT_inc(cv);
        intro_my();
        LEAVE;
    }

SV *
sv_mortalcopy(SV *sv)
    CODE:
        RETVAL = SvREFCNT_inc(sv_mortalcopy(sv));
    OUTPUT:
        RETVAL

SV *
newRV(SV *sv)

SV *
newAVav(AV *av)
    CODE:
        RETVAL = newRV_noinc((SV *)newAVav(av));
    OUTPUT:
        RETVAL

SV *
newAVhv(HV *hv)
    CODE:
        RETVAL = newRV_noinc((SV *)newAVhv(hv));
    OUTPUT:
        RETVAL

void
alias_av(AV *av, IV ix, SV *sv)
    CODE:
        av_store(av, ix, SvREFCNT_inc(sv));

SV *
cv_name(SVREF ref, ...)
    CODE:
        RETVAL = SvREFCNT_inc(cv_name((CV *)ref,
                                      items>1 && ST(1) != &PL_sv_undef
                                        ? ST(1)
                                        : NULL,
                                      items>2 ? SvUV(ST(2)) : 0));
    OUTPUT:
        RETVAL

void
sv_catpvn(SV *sv, SV *sv2)
    CODE:
    {
        STRLEN len;
        const char *s = SvPV(sv2,len);
        sv_catpvn_flags(sv,s,len, SvUTF8(sv2) ? SV_CATUTF8 : SV_CATBYTES);
    }

bool
test_newOP_CUSTOM()
    CODE:
    {
        OP *o = newLISTOP(OP_CUSTOM, 0, NULL, NULL);
        op_free(o);
        o = newOP(OP_CUSTOM, 0);
        op_free(o);
        o = newUNOP(OP_CUSTOM, 0, NULL);
        op_free(o);
        o = newUNOP_AUX(OP_CUSTOM, 0, NULL, NULL);
        op_free(o);
        o = newMETHOP(OP_CUSTOM, 0, newOP(OP_NULL,0));
        op_free(o);
        o = newMETHOP_named(OP_CUSTOM, 0, newSV(0));
        op_free(o);
        o = newBINOP(OP_CUSTOM, 0, NULL, NULL);
        op_free(o);
        o = newPMOP(OP_CUSTOM, 0);
        op_free(o);
        o = newSVOP(OP_CUSTOM, 0, newSV(0));
        op_free(o);
#ifdef USE_ITHREADS
        ENTER;
        lex_start(NULL, NULL, 0);
        {
            I32 ix = start_subparse(FALSE,0);
            o = newPADOP(OP_CUSTOM, 0, newSV(0));
            op_free(o);
            LEAVE_SCOPE(ix);
        }
        LEAVE;
#endif
        o = newPVOP(OP_CUSTOM, 0, NULL);
        op_free(o);
        o = newLOGOP(OP_CUSTOM, 0, newOP(OP_NULL,0), newOP(OP_NULL,0));
        op_free(o);
        o = newLOOPEX(OP_CUSTOM, newOP(OP_NULL,0));
        op_free(o);
        RETVAL = TRUE;
    }
    OUTPUT:
        RETVAL

void
test_sv_catpvf(SV *fmtsv)
    PREINIT:
        SV *sv;
        char *fmt;
    CODE:
        fmt = SvPV_nolen(fmtsv);
        sv = sv_2mortal(newSVpvn("", 0));
        sv_catpvf(sv, fmt, 5, 6, 7, 8);

void
load_module(flags, name, ...)
    U32 flags
    SV *name
CODE:
    if (items == 2) {
        Perl_load_module(aTHX_ flags, SvREFCNT_inc(name), NULL);
    } else if (items == 3) {
        Perl_load_module(aTHX_ flags, SvREFCNT_inc(name), SvREFCNT_inc(ST(2)));
    } else
        Perl_croak(aTHX_ "load_module can't yet support %" IVdf " items",
                          (IV)items);

SV *
string_without_null(SV *sv)
    CODE:
    {
        STRLEN len;
        const char *s = SvPV(sv, len);
        RETVAL = newSVpvn_flags(s, len, SvUTF8(sv));
        *SvEND(RETVAL) = 0xff;
    }
    OUTPUT:
        RETVAL

CV *
get_cv(SV *sv)
    CODE:
    {
        STRLEN len;
        const char *s = SvPV(sv, len);
        RETVAL = get_cvn_flags(s, len, 0);
    }
    OUTPUT:
        RETVAL

CV *
get_cv_flags(SV *sv, UV flags)
    CODE:
    {
        STRLEN len;
        const char *s = SvPV(sv, len);
        RETVAL = get_cvn_flags(s, len, flags);
    }
    OUTPUT:
        RETVAL

void
unshift_and_set_defav(SV *sv,...)
    CODE:
        av_unshift(GvAVn(PL_defgv), 1);
        av_store(GvAV(PL_defgv), 0, newSVuv(42));
        sv_setuv(sv, 43);

PerlIO *
PerlIO_stderr()

OutputStream
PerlIO_stdout()

InputStream
PerlIO_stdin()

#undef FILE
#define FILE NativeFile

FILE *
PerlIO_exportFILE(PerlIO *f, const char *mode)

SV *
test_MAX_types()
    CODE:
        /* tests that IV_MAX and UV_MAX have types suitable
           for the IVdf and UVdf formats.
           If this warns then don't add casts here.
        */
        RETVAL = newSVpvf("iv %" IVdf " uv %" UVuf, IV_MAX, UV_MAX);
    OUTPUT:
        RETVAL

SV *
test_HvNAMEf(sv)
    SV *sv
    CODE:
        if (!sv_isobject(sv)) XSRETURN_UNDEF;
        HV *pkg = SvSTASH(SvRV(sv));
        RETVAL = newSVpvf("class='%" HvNAMEf "'", pkg);
    OUTPUT:
        RETVAL

SV *
test_HvNAMEf_QUOTEDPREFIX(sv)
    SV *sv
    CODE:
        if (!sv_isobject(sv)) XSRETURN_UNDEF;
        HV *pkg = SvSTASH(SvRV(sv));
        RETVAL = newSVpvf("class=%" HvNAMEf_QUOTEDPREFIX, pkg);
    OUTPUT:
        RETVAL


bool
sv_numeq(SV *sv1, SV *sv2)
    CODE:
        RETVAL = sv_numeq(sv1, sv2);
    OUTPUT:
        RETVAL

bool
sv_numeq_flags(SV *sv1, SV *sv2, U32 flags)
    CODE:
        RETVAL = sv_numeq_flags(sv1, sv2, flags);
    OUTPUT:
        RETVAL

bool
sv_streq(SV *sv1, SV *sv2)
    CODE:
        RETVAL = sv_streq(sv1, sv2);
    OUTPUT:
        RETVAL

bool
sv_streq_flags(SV *sv1, SV *sv2, U32 flags)
    CODE:
        RETVAL = sv_streq_flags(sv1, sv2, flags);
    OUTPUT:
        RETVAL

MODULE = XS::APItest PACKAGE = XS::APItest::AUTOLOADtest

int
AUTOLOAD(...)
  INIT:
    SV* comms;
    SV* class_and_method;
  CODE:
    PERL_UNUSED_ARG(items);
    class_and_method = GvSV(CvGV(cv));
    comms = get_sv("main::the_method", 1);
    if (class_and_method == NULL) {
      RETVAL = 1;
    } else if (!SvOK(class_and_method)) {
      RETVAL = 2;
    } else if (!SvPOK(class_and_method)) {
      RETVAL = 3;
    } else {
      sv_setsv(comms, class_and_method);
      RETVAL = 0;
    }
  OUTPUT: RETVAL


MODULE = XS::APItest            PACKAGE = XS::APItest::Magic

PROTOTYPES: DISABLE

void
sv_magic_foo(SV *sv, SV *thingy)
ALIAS:
    sv_magic_bar = 1
    sv_magic_baz = 2
CODE:
    sv_magicext(sv, NULL, ix == 2 ? PERL_MAGIC_extvalue : PERL_MAGIC_ext, ix ? &vtbl_bar : &vtbl_foo, (const char *)thingy, 0);

SV *
mg_find_foo(SV *sv)
ALIAS:
    mg_find_bar = 1
    mg_find_baz = 2
CODE:
	RETVAL = &PL_sv_undef;
	if (SvTYPE(sv) >= SVt_PVMG) {
		MAGIC *mg = mg_findext(sv, ix == 2 ? PERL_MAGIC_extvalue : PERL_MAGIC_ext, ix ? &vtbl_bar : &vtbl_foo);
		if (mg)
			RETVAL = SvREFCNT_inc((SV *)mg->mg_ptr);
	}
OUTPUT:
    RETVAL

void
sv_unmagic_foo(SV *sv)
ALIAS:
    sv_unmagic_bar = 1
    sv_unmagic_baz = 2
CODE:
    sv_unmagicext(sv, ix == 2 ? PERL_MAGIC_extvalue : PERL_MAGIC_ext, ix ? &vtbl_bar : &vtbl_foo);

void
sv_magic(SV *sv, SV *thingy)
CODE:
    sv_magic(sv, NULL, PERL_MAGIC_ext, (const char *)thingy, 0);

UV
test_get_vtbl()
    PREINIT:
        MGVTBL *have;
        MGVTBL *want;
    CODE:
#define test_get_this_vtable(name) \
        want = (MGVTBL*)CAT2(&PL_vtbl_, name); \
        have = get_vtbl(CAT2(want_vtbl_, name)); \
        if (have != want) \
            croak("fail %p!=%p for get_vtbl(want_vtbl_" STRINGIFY(name) ") at " __FILE__ " line %d", have, want, __LINE__)

        test_get_this_vtable(sv);
        test_get_this_vtable(env);
        test_get_this_vtable(envelem);
        test_get_this_vtable(sigelem);
        test_get_this_vtable(pack);
        test_get_this_vtable(packelem);
        test_get_this_vtable(dbline);
        test_get_this_vtable(isa);
        test_get_this_vtable(isaelem);
        test_get_this_vtable(arylen);
        test_get_this_vtable(mglob);
        test_get_this_vtable(nkeys);
        test_get_this_vtable(taint);
        test_get_this_vtable(substr);
        test_get_this_vtable(vec);
        test_get_this_vtable(pos);
        test_get_this_vtable(bm);
        test_get_this_vtable(fm);
        test_get_this_vtable(uvar);
        test_get_this_vtable(defelem);
        test_get_this_vtable(regexp);
        test_get_this_vtable(regdata);
        test_get_this_vtable(regdatum);
#ifdef USE_LOCALE_COLLATE
        test_get_this_vtable(collxfrm);
#endif
        test_get_this_vtable(backref);
        test_get_this_vtable(utf8);

        RETVAL = PTR2UV(get_vtbl(-1));
    OUTPUT:
        RETVAL


    # attach ext magic to the SV pointed to by rsv that only has set magic,
    # where that magic's job is to increment thingy

void
sv_magic_myset_dies(SV *rsv, SV *thingy)
CODE:
    sv_magicext(SvRV(rsv), NULL, PERL_MAGIC_ext, &vtbl_myset_dies,
        (const char *)thingy, 0);


void
sv_magic_myset(SV *rsv, SV *thingy)
CODE:
    sv_magicext(SvRV(rsv), NULL, PERL_MAGIC_ext, &vtbl_myset,
        (const char *)thingy, 0);

void
sv_magic_mycopy(SV *rsv)
    PREINIT:
        MAGIC *mg;
    CODE:
        /* It's only actually useful to attach this to arrays and hashes. */
        mg = sv_magicext(SvRV(rsv), NULL, PERL_MAGIC_ext, &vtbl_mycopy, NULL, 0);
        mg->mg_flags = MGf_COPY;

SV *
sv_magic_mycopy_count(SV *rsv)
    PREINIT:
        MAGIC *mg;
    CODE:
        mg = mg_findext(SvRV(rsv), PERL_MAGIC_ext, &vtbl_mycopy);
        RETVAL = mg ? newSViv(mg->mg_private) : &PL_sv_undef;
    OUTPUT:
        RETVAL

int
my_av_store(SV *rsv, IV i, SV *sv)
    CODE:
        if (av_store((AV*)SvRV(rsv), i, sv)) {
            SvREFCNT_inc(sv);
            RETVAL = 1;
        } else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

STRLEN
sv_refcnt(SV *sv)
    CODE:
        RETVAL = SvREFCNT(sv);
    OUTPUT:
        RETVAL

void
test_mortal_destructor_sv(SV *coderef, SV *args)
    CODE:
        MORTALDESTRUCTOR_SV(coderef,args);

void
test_mortal_destructor_av(SV *coderef, AV *args)
    CODE:
        /* passing in an AV cast to SV is different from a SV ref to an AV */
        MORTALDESTRUCTOR_SV(coderef, (SV *)args);

void
test_mortal_svfunc_x(SV *args)
    CODE:
        MORTALSVFUNC_X(&destruct_test,args);




MODULE = XS::APItest            PACKAGE = XS::APItest

bool
test_isBLANK_uni(UV ord)
    CODE:
        RETVAL = isBLANK_uni(ord);
    OUTPUT:
        RETVAL

bool
test_isBLANK_uvchr(UV ord)
    CODE:
        RETVAL = isBLANK_uvchr(ord);
    OUTPUT:
        RETVAL

bool
test_isBLANK_LC_uvchr(UV ord)
    CODE:
        RETVAL = isBLANK_LC_uvchr(ord);
    OUTPUT:
        RETVAL

bool
test_isBLANK(UV ord)
    CODE:
        RETVAL = isBLANK(ord);
    OUTPUT:
        RETVAL

bool
test_isBLANK_A(UV ord)
    CODE:
        RETVAL = isBLANK_A(ord);
    OUTPUT:
        RETVAL

bool
test_isBLANK_L1(UV ord)
    CODE:
        RETVAL = isBLANK_L1(ord);
    OUTPUT:
        RETVAL

bool
test_isBLANK_LC(UV ord)
    CODE:
        RETVAL = isBLANK_LC(ord);
    OUTPUT:
        RETVAL

bool
test_isBLANK_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:

        /* In this function and those that follow, the boolean 'type'
         * indicates if to pass a malformed UTF-8 string to the tested macro
         * (malformed by making it too short) */
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isBLANK_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

bool
test_isBLANK_LC_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isBLANK_LC_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

bool
test_isVERTWS_uni(UV ord)
    CODE:
        RETVAL = isVERTWS_uni(ord);
    OUTPUT:
        RETVAL

bool
test_isVERTWS_uvchr(UV ord)
    CODE:
        RETVAL = isVERTWS_uvchr(ord);
    OUTPUT:
        RETVAL

bool
test_isVERTWS_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isVERTWS_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

bool
test_isUPPER_uni(UV ord)
    CODE:
        RETVAL = isUPPER_uni(ord);
    OUTPUT:
        RETVAL

bool
test_isUPPER_uvchr(UV ord)
    CODE:
        RETVAL = isUPPER_uvchr(ord);
    OUTPUT:
        RETVAL

bool
test_isUPPER_LC_uvchr(UV ord)
    CODE:
        RETVAL = isUPPER_LC_uvchr(ord);
    OUTPUT:
        RETVAL

bool
test_isUPPER(UV ord)
    CODE:
        RETVAL = isUPPER(ord);
    OUTPUT:
        RETVAL

bool
test_isUPPER_A(UV ord)
    CODE:
        RETVAL = isUPPER_A(ord);
    OUTPUT:
        RETVAL

bool
test_isUPPER_L1(UV ord)
    CODE:
        RETVAL = isUPPER_L1(ord);
    OUTPUT:
        RETVAL

bool
test_isUPPER_LC(UV ord)
    CODE:
        RETVAL = isUPPER_LC(ord);
    OUTPUT:
        RETVAL

bool
test_isUPPER_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isUPPER_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

bool
test_isUPPER_LC_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isUPPER_LC_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

bool
test_isLOWER_uni(UV ord)
    CODE:
        RETVAL = isLOWER_uni(ord);
    OUTPUT:
        RETVAL

bool
test_isLOWER_uvchr(UV ord)
    CODE:
        RETVAL = isLOWER_uvchr(ord);
    OUTPUT:
        RETVAL

bool
test_isLOWER_LC_uvchr(UV ord)
    CODE:
        RETVAL = isLOWER_LC_uvchr(ord);
    OUTPUT:
        RETVAL

bool
test_isLOWER(UV ord)
    CODE:
        RETVAL = isLOWER(ord);
    OUTPUT:
        RETVAL

bool
test_isLOWER_A(UV ord)
    CODE:
        RETVAL = isLOWER_A(ord);
    OUTPUT:
        RETVAL

bool
test_isLOWER_L1(UV ord)
    CODE:
        RETVAL = isLOWER_L1(ord);
    OUTPUT:
        RETVAL

bool
test_isLOWER_LC(UV ord)
    CODE:
        RETVAL = isLOWER_LC(ord);
    OUTPUT:
        RETVAL

bool
test_isLOWER_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isLOWER_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

bool
test_isLOWER_LC_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isLOWER_LC_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

bool
test_isALPHA_uni(UV ord)
    CODE:
        RETVAL = isALPHA_uni(ord);
    OUTPUT:
        RETVAL

bool
test_isALPHA_uvchr(UV ord)
    CODE:
        RETVAL = isALPHA_uvchr(ord);
    OUTPUT:
        RETVAL

bool
test_isALPHA_LC_uvchr(UV ord)
    CODE:
        RETVAL = isALPHA_LC_uvchr(ord);
    OUTPUT:
        RETVAL

bool
test_isALPHA(UV ord)
    CODE:
        RETVAL = isALPHA(ord);
    OUTPUT:
        RETVAL

bool
test_isALPHA_A(UV ord)
    CODE:
        RETVAL = isALPHA_A(ord);
    OUTPUT:
        RETVAL

bool
test_isALPHA_L1(UV ord)
    CODE:
        RETVAL = isALPHA_L1(ord);
    OUTPUT:
        RETVAL

bool
test_isALPHA_LC(UV ord)
    CODE:
        RETVAL = isALPHA_LC(ord);
    OUTPUT:
        RETVAL

bool
test_isALPHA_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isALPHA_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

bool
test_isALPHA_LC_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isALPHA_LC_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

bool
test_isWORDCHAR_uni(UV ord)
    CODE:
        RETVAL = isWORDCHAR_uni(ord);
    OUTPUT:
        RETVAL

bool
test_isWORDCHAR_uvchr(UV ord)
    CODE:
        RETVAL = isWORDCHAR_uvchr(ord);
    OUTPUT:
        RETVAL

bool
test_isWORDCHAR_LC_uvchr(UV ord)
    CODE:
        RETVAL = isWORDCHAR_LC_uvchr(ord);
    OUTPUT:
        RETVAL

bool
test_isWORDCHAR(UV ord)
    CODE:
        RETVAL = isWORDCHAR(ord);
    OUTPUT:
        RETVAL

bool
test_isWORDCHAR_A(UV ord)
    CODE:
        RETVAL = isWORDCHAR_A(ord);
    OUTPUT:
        RETVAL

bool
test_isWORDCHAR_L1(UV ord)
    CODE:
        RETVAL = isWORDCHAR_L1(ord);
    OUTPUT:
        RETVAL

bool
test_isWORDCHAR_LC(UV ord)
    CODE:
        RETVAL = isWORDCHAR_LC(ord);
    OUTPUT:
        RETVAL

bool
test_isWORDCHAR_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isWORDCHAR_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

bool
test_isWORDCHAR_LC_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isWORDCHAR_LC_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

bool
test_isALPHANUMERIC_uni(UV ord)
    CODE:
        RETVAL = isALPHANUMERIC_uni(ord);
    OUTPUT:
        RETVAL

bool
test_isALPHANUMERIC_uvchr(UV ord)
    CODE:
        RETVAL = isALPHANUMERIC_uvchr(ord);
    OUTPUT:
        RETVAL

bool
test_isALPHANUMERIC_LC_uvchr(UV ord)
    CODE:
        RETVAL = isALPHANUMERIC_LC_uvchr(ord);
    OUTPUT:
        RETVAL

bool
test_isALPHANUMERIC(UV ord)
    CODE:
        RETVAL = isALPHANUMERIC(ord);
    OUTPUT:
        RETVAL

bool
test_isALPHANUMERIC_A(UV ord)
    CODE:
        RETVAL = isALPHANUMERIC_A(ord);
    OUTPUT:
        RETVAL

bool
test_isALPHANUMERIC_L1(UV ord)
    CODE:
        RETVAL = isALPHANUMERIC_L1(ord);
    OUTPUT:
        RETVAL

bool
test_isALPHANUMERIC_LC(UV ord)
    CODE:
        RETVAL = isALPHANUMERIC_LC(ord);
    OUTPUT:
        RETVAL

bool
test_isALPHANUMERIC_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isALPHANUMERIC_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

bool
test_isALPHANUMERIC_LC_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isALPHANUMERIC_LC_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

bool
test_isALNUM(UV ord)
    CODE:
        RETVAL = isALNUM(ord);
    OUTPUT:
        RETVAL

bool
test_isALNUM_uni(UV ord)
    CODE:
        RETVAL = isALNUM_uni(ord);
    OUTPUT:
        RETVAL

bool
test_isALNUM_LC_uvchr(UV ord)
    CODE:
        RETVAL = isALNUM_LC_uvchr(ord);
    OUTPUT:
        RETVAL

bool
test_isALNUM_LC(UV ord)
    CODE:
        RETVAL = isALNUM_LC(ord);
    OUTPUT:
        RETVAL

bool
test_isALNUM_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isWORDCHAR_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

bool
test_isALNUM_LC_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isWORDCHAR_LC_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

bool
test_isDIGIT_uni(UV ord)
    CODE:
        RETVAL = isDIGIT_uni(ord);
    OUTPUT:
        RETVAL

bool
test_isDIGIT_uvchr(UV ord)
    CODE:
        RETVAL = isDIGIT_uvchr(ord);
    OUTPUT:
        RETVAL

bool
test_isDIGIT_LC_uvchr(UV ord)
    CODE:
        RETVAL = isDIGIT_LC_uvchr(ord);
    OUTPUT:
        RETVAL

bool
test_isDIGIT_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isDIGIT_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

bool
test_isDIGIT_LC_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isDIGIT_LC_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

bool
test_isDIGIT(UV ord)
    CODE:
        RETVAL = isDIGIT(ord);
    OUTPUT:
        RETVAL

bool
test_isDIGIT_A(UV ord)
    CODE:
        RETVAL = isDIGIT_A(ord);
    OUTPUT:
        RETVAL

bool
test_isDIGIT_L1(UV ord)
    CODE:
        RETVAL = isDIGIT_L1(ord);
    OUTPUT:
        RETVAL

bool
test_isDIGIT_LC(UV ord)
    CODE:
        RETVAL = isDIGIT_LC(ord);
    OUTPUT:
        RETVAL

bool
test_isOCTAL(UV ord)
    CODE:
        RETVAL = isOCTAL(ord);
    OUTPUT:
        RETVAL

bool
test_isOCTAL_A(UV ord)
    CODE:
        RETVAL = isOCTAL_A(ord);
    OUTPUT:
        RETVAL

bool
test_isOCTAL_L1(UV ord)
    CODE:
        RETVAL = isOCTAL_L1(ord);
    OUTPUT:
        RETVAL

bool
test_isIDFIRST_uni(UV ord)
    CODE:
        RETVAL = isIDFIRST_uni(ord);
    OUTPUT:
        RETVAL

bool
test_isIDFIRST_uvchr(UV ord)
    CODE:
        RETVAL = isIDFIRST_uvchr(ord);
    OUTPUT:
        RETVAL

bool
test_isIDFIRST_LC_uvchr(UV ord)
    CODE:
        RETVAL = isIDFIRST_LC_uvchr(ord);
    OUTPUT:
        RETVAL

bool
test_isIDFIRST(UV ord)
    CODE:
        RETVAL = isIDFIRST(ord);
    OUTPUT:
        RETVAL

bool
test_isIDFIRST_A(UV ord)
    CODE:
        RETVAL = isIDFIRST_A(ord);
    OUTPUT:
        RETVAL

bool
test_isIDFIRST_L1(UV ord)
    CODE:
        RETVAL = isIDFIRST_L1(ord);
    OUTPUT:
        RETVAL

bool
test_isIDFIRST_LC(UV ord)
    CODE:
        RETVAL = isIDFIRST_LC(ord);
    OUTPUT:
        RETVAL

bool
test_isIDFIRST_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isIDFIRST_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

bool
test_isIDFIRST_LC_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isIDFIRST_LC_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

bool
test_isIDCONT_uni(UV ord)
    CODE:
        RETVAL = isIDCONT_uni(ord);
    OUTPUT:
        RETVAL

bool
test_isIDCONT_uvchr(UV ord)
    CODE:
        RETVAL = isIDCONT_uvchr(ord);
    OUTPUT:
        RETVAL

bool
test_isIDCONT_LC_uvchr(UV ord)
    CODE:
        RETVAL = isIDCONT_LC_uvchr(ord);
    OUTPUT:
        RETVAL

bool
test_isIDCONT(UV ord)
    CODE:
        RETVAL = isIDCONT(ord);
    OUTPUT:
        RETVAL

bool
test_isIDCONT_A(UV ord)
    CODE:
        RETVAL = isIDCONT_A(ord);
    OUTPUT:
        RETVAL

bool
test_isIDCONT_L1(UV ord)
    CODE:
        RETVAL = isIDCONT_L1(ord);
    OUTPUT:
        RETVAL

bool
test_isIDCONT_LC(UV ord)
    CODE:
        RETVAL = isIDCONT_LC(ord);
    OUTPUT:
        RETVAL

bool
test_isIDCONT_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isIDCONT_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

bool
test_isIDCONT_LC_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isIDCONT_LC_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

bool
test_isSPACE_uni(UV ord)
    CODE:
        RETVAL = isSPACE_uni(ord);
    OUTPUT:
        RETVAL

bool
test_isSPACE_uvchr(UV ord)
    CODE:
        RETVAL = isSPACE_uvchr(ord);
    OUTPUT:
        RETVAL

bool
test_isSPACE_LC_uvchr(UV ord)
    CODE:
        RETVAL = isSPACE_LC_uvchr(ord);
    OUTPUT:
        RETVAL

bool
test_isSPACE(UV ord)
    CODE:
        RETVAL = isSPACE(ord);
    OUTPUT:
        RETVAL

bool
test_isSPACE_A(UV ord)
    CODE:
        RETVAL = isSPACE_A(ord);
    OUTPUT:
        RETVAL

bool
test_isSPACE_L1(UV ord)
    CODE:
        RETVAL = isSPACE_L1(ord);
    OUTPUT:
        RETVAL

bool
test_isSPACE_LC(UV ord)
    CODE:
        RETVAL = isSPACE_LC(ord);
    OUTPUT:
        RETVAL

bool
test_isSPACE_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isSPACE_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

bool
test_isSPACE_LC_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isSPACE_LC_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

bool
test_isASCII_uni(UV ord)
    CODE:
        RETVAL = isASCII_uni(ord);
    OUTPUT:
        RETVAL

bool
test_isASCII_uvchr(UV ord)
    CODE:
        RETVAL = isASCII_uvchr(ord);
    OUTPUT:
        RETVAL

bool
test_isASCII_LC_uvchr(UV ord)
    CODE:
        RETVAL = isASCII_LC_uvchr(ord);
    OUTPUT:
        RETVAL

bool
test_isASCII(UV ord)
    CODE:
        RETVAL = isASCII(ord);
    OUTPUT:
        RETVAL

bool
test_isASCII_A(UV ord)
    CODE:
        RETVAL = isASCII_A(ord);
    OUTPUT:
        RETVAL

bool
test_isASCII_L1(UV ord)
    CODE:
        RETVAL = isASCII_L1(ord);
    OUTPUT:
        RETVAL

bool
test_isASCII_LC(UV ord)
    CODE:
        RETVAL = isASCII_LC(ord);
    OUTPUT:
        RETVAL

bool
test_isASCII_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:
#ifndef DEBUGGING
        PERL_UNUSED_VAR(e);
#endif
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isASCII_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

bool
test_isASCII_LC_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:
#ifndef DEBUGGING
        PERL_UNUSED_VAR(e);
#endif
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isASCII_LC_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

bool
test_isCNTRL_uni(UV ord)
    CODE:
        RETVAL = isCNTRL_uni(ord);
    OUTPUT:
        RETVAL

bool
test_isCNTRL_uvchr(UV ord)
    CODE:
        RETVAL = isCNTRL_uvchr(ord);
    OUTPUT:
        RETVAL

bool
test_isCNTRL_LC_uvchr(UV ord)
    CODE:
        RETVAL = isCNTRL_LC_uvchr(ord);
    OUTPUT:
        RETVAL

bool
test_isCNTRL(UV ord)
    CODE:
        RETVAL = isCNTRL(ord);
    OUTPUT:
        RETVAL

bool
test_isCNTRL_A(UV ord)
    CODE:
        RETVAL = isCNTRL_A(ord);
    OUTPUT:
        RETVAL

bool
test_isCNTRL_L1(UV ord)
    CODE:
        RETVAL = isCNTRL_L1(ord);
    OUTPUT:
        RETVAL

bool
test_isCNTRL_LC(UV ord)
    CODE:
        RETVAL = isCNTRL_LC(ord);
    OUTPUT:
        RETVAL

bool
test_isCNTRL_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isCNTRL_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

bool
test_isCNTRL_LC_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isCNTRL_LC_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

bool
test_isPRINT_uni(UV ord)
    CODE:
        RETVAL = isPRINT_uni(ord);
    OUTPUT:
        RETVAL

bool
test_isPRINT_uvchr(UV ord)
    CODE:
        RETVAL = isPRINT_uvchr(ord);
    OUTPUT:
        RETVAL

bool
test_isPRINT_LC_uvchr(UV ord)
    CODE:
        RETVAL = isPRINT_LC_uvchr(ord);
    OUTPUT:
        RETVAL

bool
test_isPRINT(UV ord)
    CODE:
        RETVAL = isPRINT(ord);
    OUTPUT:
        RETVAL

bool
test_isPRINT_A(UV ord)
    CODE:
        RETVAL = isPRINT_A(ord);
    OUTPUT:
        RETVAL

bool
test_isPRINT_L1(UV ord)
    CODE:
        RETVAL = isPRINT_L1(ord);
    OUTPUT:
        RETVAL

bool
test_isPRINT_LC(UV ord)
    CODE:
        RETVAL = isPRINT_LC(ord);
    OUTPUT:
        RETVAL

bool
test_isPRINT_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isPRINT_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

bool
test_isPRINT_LC_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isPRINT_LC_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

bool
test_isGRAPH_uni(UV ord)
    CODE:
        RETVAL = isGRAPH_uni(ord);
    OUTPUT:
        RETVAL

bool
test_isGRAPH_uvchr(UV ord)
    CODE:
        RETVAL = isGRAPH_uvchr(ord);
    OUTPUT:
        RETVAL

bool
test_isGRAPH_LC_uvchr(UV ord)
    CODE:
        RETVAL = isGRAPH_LC_uvchr(ord);
    OUTPUT:
        RETVAL

bool
test_isGRAPH(UV ord)
    CODE:
        RETVAL = isGRAPH(ord);
    OUTPUT:
        RETVAL

bool
test_isGRAPH_A(UV ord)
    CODE:
        RETVAL = isGRAPH_A(ord);
    OUTPUT:
        RETVAL

bool
test_isGRAPH_L1(UV ord)
    CODE:
        RETVAL = isGRAPH_L1(ord);
    OUTPUT:
        RETVAL

bool
test_isGRAPH_LC(UV ord)
    CODE:
        RETVAL = isGRAPH_LC(ord);
    OUTPUT:
        RETVAL

bool
test_isGRAPH_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isGRAPH_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

bool
test_isGRAPH_LC_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isGRAPH_LC_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

bool
test_isPUNCT_uni(UV ord)
    CODE:
        RETVAL = isPUNCT_uni(ord);
    OUTPUT:
        RETVAL

bool
test_isPUNCT_uvchr(UV ord)
    CODE:
        RETVAL = isPUNCT_uvchr(ord);
    OUTPUT:
        RETVAL

bool
test_isPUNCT_LC_uvchr(UV ord)
    CODE:
        RETVAL = isPUNCT_LC_uvchr(ord);
    OUTPUT:
        RETVAL

bool
test_isPUNCT(UV ord)
    CODE:
        RETVAL = isPUNCT(ord);
    OUTPUT:
        RETVAL

bool
test_isPUNCT_A(UV ord)
    CODE:
        RETVAL = isPUNCT_A(ord);
    OUTPUT:
        RETVAL

bool
test_isPUNCT_L1(UV ord)
    CODE:
        RETVAL = isPUNCT_L1(ord);
    OUTPUT:
        RETVAL

bool
test_isPUNCT_LC(UV ord)
    CODE:
        RETVAL = isPUNCT_LC(ord);
    OUTPUT:
        RETVAL

bool
test_isPUNCT_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isPUNCT_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

bool
test_isPUNCT_LC_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isPUNCT_LC_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

bool
test_isXDIGIT_uni(UV ord)
    CODE:
        RETVAL = isXDIGIT_uni(ord);
    OUTPUT:
        RETVAL

bool
test_isXDIGIT_uvchr(UV ord)
    CODE:
        RETVAL = isXDIGIT_uvchr(ord);
    OUTPUT:
        RETVAL

bool
test_isXDIGIT_LC_uvchr(UV ord)
    CODE:
        RETVAL = isXDIGIT_LC_uvchr(ord);
    OUTPUT:
        RETVAL

bool
test_isXDIGIT(UV ord)
    CODE:
        RETVAL = isXDIGIT(ord);
    OUTPUT:
        RETVAL

bool
test_isXDIGIT_A(UV ord)
    CODE:
        RETVAL = isXDIGIT_A(ord);
    OUTPUT:
        RETVAL

bool
test_isXDIGIT_L1(UV ord)
    CODE:
        RETVAL = isXDIGIT_L1(ord);
    OUTPUT:
        RETVAL

bool
test_isXDIGIT_LC(UV ord)
    CODE:
        RETVAL = isXDIGIT_LC(ord);
    OUTPUT:
        RETVAL

bool
test_isXDIGIT_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isXDIGIT_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

bool
test_isXDIGIT_LC_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isXDIGIT_LC_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

bool
test_isPSXSPC_uni(UV ord)
    CODE:
        RETVAL = isPSXSPC_uni(ord);
    OUTPUT:
        RETVAL

bool
test_isPSXSPC_uvchr(UV ord)
    CODE:
        RETVAL = isPSXSPC_uvchr(ord);
    OUTPUT:
        RETVAL

bool
test_isPSXSPC_LC_uvchr(UV ord)
    CODE:
        RETVAL = isPSXSPC_LC_uvchr(ord);
    OUTPUT:
        RETVAL

bool
test_isPSXSPC(UV ord)
    CODE:
        RETVAL = isPSXSPC(ord);
    OUTPUT:
        RETVAL

bool
test_isPSXSPC_A(UV ord)
    CODE:
        RETVAL = isPSXSPC_A(ord);
    OUTPUT:
        RETVAL

bool
test_isPSXSPC_L1(UV ord)
    CODE:
        RETVAL = isPSXSPC_L1(ord);
    OUTPUT:
        RETVAL

bool
test_isPSXSPC_LC(UV ord)
    CODE:
        RETVAL = isPSXSPC_LC(ord);
    OUTPUT:
        RETVAL

bool
test_isPSXSPC_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isPSXSPC_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

bool
test_isPSXSPC_LC_utf8(U8 * p, int type)
    PREINIT:
        const U8 * e;
    CODE:
        if (type >= 0) {
            e = p + UTF8SKIP(p) - type;
            RETVAL = isPSXSPC_LC_utf8_safe(p, e);
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

STRLEN
test_UTF8_IS_REPLACEMENT(char *s, STRLEN len)
    CODE:
        RETVAL = UTF8_IS_REPLACEMENT(s, s + len);
    OUTPUT:
        RETVAL

bool
test_isQUOTEMETA(UV ord)
    CODE:
        RETVAL = _isQUOTEMETA(ord);
    OUTPUT:
        RETVAL

UV
test_OFFUNISKIP(UV ord)
    CODE:
        RETVAL = OFFUNISKIP(ord);
    OUTPUT:
        RETVAL

bool
test_OFFUNI_IS_INVARIANT(UV ord)
    CODE:
        RETVAL = OFFUNI_IS_INVARIANT(ord);
    OUTPUT:
        RETVAL

bool
test_UVCHR_IS_INVARIANT(UV ord)
    CODE:
        RETVAL = UVCHR_IS_INVARIANT(ord);
    OUTPUT:
        RETVAL

bool
test_UTF8_IS_INVARIANT(char ch)
    CODE:
        RETVAL = UTF8_IS_INVARIANT(ch);
    OUTPUT:
        RETVAL

UV
test_UVCHR_SKIP(UV ord)
    CODE:
        RETVAL = UVCHR_SKIP(ord);
    OUTPUT:
        RETVAL

UV
test_UTF8_SKIP(char * ch)
    CODE:
        RETVAL = UTF8_SKIP(ch);
    OUTPUT:
        RETVAL

bool
test_UTF8_IS_START(char ch)
    CODE:
        RETVAL = UTF8_IS_START(ch);
    OUTPUT:
        RETVAL

bool
test_UTF8_IS_CONTINUATION(char ch)
    CODE:
        RETVAL = UTF8_IS_CONTINUATION(ch);
    OUTPUT:
        RETVAL

bool
test_UTF8_IS_CONTINUED(char ch)
    CODE:
        RETVAL = UTF8_IS_CONTINUED(ch);
    OUTPUT:
        RETVAL

bool
test_UTF8_IS_DOWNGRADEABLE_START(char ch)
    CODE:
        RETVAL = UTF8_IS_DOWNGRADEABLE_START(ch);
    OUTPUT:
        RETVAL

bool
test_UTF8_IS_ABOVE_LATIN1(char ch)
    CODE:
        RETVAL = UTF8_IS_ABOVE_LATIN1(ch);
    OUTPUT:
        RETVAL

bool
test_isUTF8_POSSIBLY_PROBLEMATIC(char ch)
    CODE:
        RETVAL = isUTF8_POSSIBLY_PROBLEMATIC(ch);
    OUTPUT:
        RETVAL

STRLEN
test_isUTF8_CHAR(char *s, STRLEN len)
    CODE:
        RETVAL = isUTF8_CHAR((U8 *) s, (U8 *) s + len);
    OUTPUT:
        RETVAL

STRLEN
test_isUTF8_CHAR_flags(char *s, STRLEN len, U32 flags)
    CODE:
        RETVAL = isUTF8_CHAR_flags((U8 *) s, (U8 *) s + len, flags);
    OUTPUT:
        RETVAL

STRLEN
test_isSTRICT_UTF8_CHAR(char *s, STRLEN len)
    CODE:
        RETVAL = isSTRICT_UTF8_CHAR((U8 *) s, (U8 *) s + len);
    OUTPUT:
        RETVAL

STRLEN
test_isC9_STRICT_UTF8_CHAR(char *s, STRLEN len)
    CODE:
        RETVAL = isC9_STRICT_UTF8_CHAR((U8 *) s, (U8 *) s + len);
    OUTPUT:
        RETVAL

IV
test_is_utf8_valid_partial_char_flags(char *s, STRLEN len, U32 flags)
    CODE:
        /* RETVAL should be bool (here and in tests below), but making it IV
         * allows us to test it returning 0 or 1 */
        RETVAL = is_utf8_valid_partial_char_flags((U8 *) s, (U8 *) s + len, flags);
    OUTPUT:
        RETVAL

IV
test_is_utf8_string(char *s, STRLEN len)
    CODE:
        RETVAL = is_utf8_string((U8 *) s, len);
    OUTPUT:
        RETVAL

#define WORDSIZE            sizeof(PERL_UINTMAX_T)

AV *
test_is_utf8_invariant_string_loc(U8 *s, STRLEN offset, STRLEN len)
    PREINIT:
        AV *av;
        const U8 * ep = NULL;
        PERL_UINTMAX_T* copy;
    CODE:
        /* 'offset' is number of bytes past a word boundary the testing of 's'
         * is to start at.  Allocate space that does start at the word
         * boundary, and copy 's' to the correct offset past it.  Then call the
         * tested function with that position */
        Newx(copy, 1 + ((len + WORDSIZE - 1) / WORDSIZE), PERL_UINTMAX_T);
        Copy(s, (U8 *) copy + offset, len, U8);
        av = newAV();
        av_push(av, newSViv(is_utf8_invariant_string_loc((U8 *) copy + offset, len, &ep)));
        av_push(av, newSViv(ep - ((U8 *) copy + offset)));
        RETVAL = av;
        Safefree(copy);
    OUTPUT:
        RETVAL

STRLEN
test_variant_under_utf8_count(U8 *s, STRLEN offset, STRLEN len)
    PREINIT:
        PERL_UINTMAX_T * copy;
    CODE:
        Newx(copy, 1 + ((len + WORDSIZE - 1) / WORDSIZE), PERL_UINTMAX_T);
        Copy(s, (U8 *) copy + offset, len, U8);
        RETVAL = variant_under_utf8_count((U8 *) copy + offset, (U8 *) copy + offset + len);
        Safefree(copy);
    OUTPUT:
        RETVAL

STRLEN
test_utf8_length(U8 *s, STRLEN offset, STRLEN len)
CODE:
    RETVAL = utf8_length(s + offset, s + len);
OUTPUT:
    RETVAL

AV *
test_is_utf8_string_loc(char *s, STRLEN len)
    PREINIT:
        AV *av;
        const U8 * ep;
    CODE:
        av = newAV();
        av_push(av, newSViv(is_utf8_string_loc((U8 *) s, len, &ep)));
        av_push(av, newSViv(ep - (U8 *) s));
        RETVAL = av;
    OUTPUT:
        RETVAL

AV *
test_is_utf8_string_loclen(char *s, STRLEN len)
    PREINIT:
        AV *av;
        STRLEN ret_len;
        const U8 * ep;
    CODE:
        av = newAV();
        av_push(av, newSViv(is_utf8_string_loclen((U8 *) s, len, &ep, &ret_len)));
        av_push(av, newSViv(ep - (U8 *) s));
        av_push(av, newSVuv(ret_len));
        RETVAL = av;
    OUTPUT:
        RETVAL

IV
test_is_utf8_string_flags(char *s, STRLEN len, U32 flags)
    CODE:
        RETVAL = is_utf8_string_flags((U8 *) s, len, flags);
    OUTPUT:
        RETVAL

AV *
test_is_utf8_string_loc_flags(char *s, STRLEN len, U32 flags)
    PREINIT:
        AV *av;
        const U8 * ep;
    CODE:
        av = newAV();
        av_push(av, newSViv(is_utf8_string_loc_flags((U8 *) s, len, &ep, flags)));
        av_push(av, newSViv(ep - (U8 *) s));
        RETVAL = av;
    OUTPUT:
        RETVAL

AV *
test_is_utf8_string_loclen_flags(char *s, STRLEN len, U32 flags)
    PREINIT:
        AV *av;
        STRLEN ret_len;
        const U8 * ep;
    CODE:
        av = newAV();
        av_push(av, newSViv(is_utf8_string_loclen_flags((U8 *) s, len, &ep, &ret_len, flags)));
        av_push(av, newSViv(ep - (U8 *) s));
        av_push(av, newSVuv(ret_len));
        RETVAL = av;
    OUTPUT:
        RETVAL

IV
test_is_strict_utf8_string(char *s, STRLEN len)
    CODE:
        RETVAL = is_strict_utf8_string((U8 *) s, len);
    OUTPUT:
        RETVAL

AV *
test_is_strict_utf8_string_loc(char *s, STRLEN len)
    PREINIT:
        AV *av;
        const U8 * ep;
    CODE:
        av = newAV();
        av_push(av, newSViv(is_strict_utf8_string_loc((U8 *) s, len, &ep)));
        av_push(av, newSViv(ep - (U8 *) s));
        RETVAL = av;
    OUTPUT:
        RETVAL

AV *
test_is_strict_utf8_string_loclen(char *s, STRLEN len)
    PREINIT:
        AV *av;
        STRLEN ret_len;
        const U8 * ep;
    CODE:
        av = newAV();
        av_push(av, newSViv(is_strict_utf8_string_loclen((U8 *) s, len, &ep, &ret_len)));
        av_push(av, newSViv(ep - (U8 *) s));
        av_push(av, newSVuv(ret_len));
        RETVAL = av;
    OUTPUT:
        RETVAL

IV
test_is_c9strict_utf8_string(char *s, STRLEN len)
    CODE:
        RETVAL = is_c9strict_utf8_string((U8 *) s, len);
    OUTPUT:
        RETVAL

AV *
test_is_c9strict_utf8_string_loc(char *s, STRLEN len)
    PREINIT:
        AV *av;
        const U8 * ep;
    CODE:
        av = newAV();
        av_push(av, newSViv(is_c9strict_utf8_string_loc((U8 *) s, len, &ep)));
        av_push(av, newSViv(ep - (U8 *) s));
        RETVAL = av;
    OUTPUT:
        RETVAL

AV *
test_is_c9strict_utf8_string_loclen(char *s, STRLEN len)
    PREINIT:
        AV *av;
        STRLEN ret_len;
        const U8 * ep;
    CODE:
        av = newAV();
        av_push(av, newSViv(is_c9strict_utf8_string_loclen((U8 *) s, len, &ep, &ret_len)));
        av_push(av, newSViv(ep - (U8 *) s));
        av_push(av, newSVuv(ret_len));
        RETVAL = av;
    OUTPUT:
        RETVAL

IV
test_is_utf8_fixed_width_buf_flags(char *s, STRLEN len, U32 flags)
    CODE:
        RETVAL = is_utf8_fixed_width_buf_flags((U8 *) s, len, flags);
    OUTPUT:
        RETVAL

AV *
test_is_utf8_fixed_width_buf_loc_flags(char *s, STRLEN len, U32 flags)
    PREINIT:
        AV *av;
        const U8 * ep;
    CODE:
        av = newAV();
        av_push(av, newSViv(is_utf8_fixed_width_buf_loc_flags((U8 *) s, len, &ep, flags)));
        av_push(av, newSViv(ep - (U8 *) s));
        RETVAL = av;
    OUTPUT:
        RETVAL

AV *
test_is_utf8_fixed_width_buf_loclen_flags(char *s, STRLEN len, U32 flags)
    PREINIT:
        AV *av;
        STRLEN ret_len;
        const U8 * ep;
    CODE:
        av = newAV();
        av_push(av, newSViv(is_utf8_fixed_width_buf_loclen_flags((U8 *) s, len, &ep, &ret_len, flags)));
        av_push(av, newSViv(ep - (U8 *) s));
        av_push(av, newSVuv(ret_len));
        RETVAL = av;
    OUTPUT:
        RETVAL

IV
test_utf8_hop_safe(SV *s_sv, STRLEN s_off, IV hop)
    PREINIT:
        STRLEN len;
        U8 *p;
        U8 *r;
    CODE:
        p = (U8 *)SvPV(s_sv, len);
        r = utf8_hop_safe(p + s_off, hop, p, p + len);
        RETVAL = r - p;
    OUTPUT:
        RETVAL

UV
test_toLOWER(UV ord)
    CODE:
        RETVAL = toLOWER(ord);
    OUTPUT:
        RETVAL

UV
test_toLOWER_L1(UV ord)
    CODE:
        RETVAL = toLOWER_L1(ord);
    OUTPUT:
        RETVAL

UV
test_toLOWER_LC(UV ord)
    CODE:
        RETVAL = toLOWER_LC(ord);
    OUTPUT:
        RETVAL

AV *
test_toLOWER_uni(UV ord)
    PREINIT:
        U8 s[UTF8_MAXBYTES_CASE + 1];
        STRLEN len;
        AV *av;
        SV *utf8;
    CODE:
        av = newAV();
        av_push(av, newSVuv(toLOWER_uni(ord, s, &len)));

        utf8 = newSVpvn((char *) s, len);
        SvUTF8_on(utf8);
        av_push(av, utf8);

        av_push(av, newSVuv(len));
        RETVAL = av;
    OUTPUT:
        RETVAL

AV *
test_toLOWER_uvchr(UV ord)
    PREINIT:
        U8 s[UTF8_MAXBYTES_CASE + 1];
        STRLEN len;
        AV *av;
        SV *utf8;
    CODE:
        av = newAV();
        av_push(av, newSVuv(toLOWER_uvchr(ord, s, &len)));

        utf8 = newSVpvn((char *) s, len);
        SvUTF8_on(utf8);
        av_push(av, utf8);

        av_push(av, newSVuv(len));
        RETVAL = av;
    OUTPUT:
        RETVAL

AV *
test_toLOWER_utf8(SV * p, int type)
    PREINIT:
        U8 *input;
        U8 s[UTF8_MAXBYTES_CASE + 1];
        STRLEN len;
        AV *av;
        SV *utf8;
        const U8 * e;
        UV resultant_cp = UV_MAX;   /* Initialized because of dumb compilers */
    CODE:
        input = (U8 *) SvPV(p, len);
        av = newAV();
        if (type >= 0) {
            e = input + UTF8SKIP(input) - type;
            resultant_cp = toLOWER_utf8_safe(input, e, s, &len);
            av_push(av, newSVuv(resultant_cp));

            utf8 = newSVpvn((char *) s, len);
            SvUTF8_on(utf8);
            av_push(av, utf8);

            av_push(av, newSVuv(len));
            RETVAL = av;
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

UV
test_toFOLD(UV ord)
    CODE:
        RETVAL = toFOLD(ord);
    OUTPUT:
        RETVAL

UV
test_toFOLD_LC(UV ord)
    CODE:
        RETVAL = toFOLD_LC(ord);
    OUTPUT:
        RETVAL

AV *
test_toFOLD_uni(UV ord)
    PREINIT:
        U8 s[UTF8_MAXBYTES_CASE + 1];
        STRLEN len;
        AV *av;
        SV *utf8;
    CODE:
        av = newAV();
        av_push(av, newSVuv(toFOLD_uni(ord, s, &len)));

        utf8 = newSVpvn((char *) s, len);
        SvUTF8_on(utf8);
        av_push(av, utf8);

        av_push(av, newSVuv(len));
        RETVAL = av;
    OUTPUT:
        RETVAL

AV *
test_toFOLD_uvchr(UV ord)
    PREINIT:
        U8 s[UTF8_MAXBYTES_CASE + 1];
        STRLEN len;
        AV *av;
        SV *utf8;
    CODE:
        av = newAV();
        av_push(av, newSVuv(toFOLD_uvchr(ord, s, &len)));

        utf8 = newSVpvn((char *) s, len);
        SvUTF8_on(utf8);
        av_push(av, utf8);

        av_push(av, newSVuv(len));
        RETVAL = av;
    OUTPUT:
        RETVAL

AV *
test_toFOLD_utf8(SV * p, int type)
    PREINIT:
        U8 *input;
        U8 s[UTF8_MAXBYTES_CASE + 1];
        STRLEN len;
        AV *av;
        SV *utf8;
        const U8 * e;
        UV resultant_cp = UV_MAX;
    CODE:
        input = (U8 *) SvPV(p, len);
        av = newAV();
        if (type >= 0) {
            e = input + UTF8SKIP(input) - type;
            resultant_cp = toFOLD_utf8_safe(input, e, s, &len);
            av_push(av, newSVuv(resultant_cp));

            utf8 = newSVpvn((char *) s, len);
            SvUTF8_on(utf8);
            av_push(av, utf8);

            av_push(av, newSVuv(len));
            RETVAL = av;
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

UV
test_toUPPER(UV ord)
    CODE:
        RETVAL = toUPPER(ord);
    OUTPUT:
        RETVAL

UV
test_toUPPER_LC(UV ord)
    CODE:
        RETVAL = toUPPER_LC(ord);
    OUTPUT:
        RETVAL

AV *
test_toUPPER_uni(UV ord)
    PREINIT:
        U8 s[UTF8_MAXBYTES_CASE + 1];
        STRLEN len;
        AV *av;
        SV *utf8;
    CODE:
        av = newAV();
        av_push(av, newSVuv(toUPPER_uni(ord, s, &len)));

        utf8 = newSVpvn((char *) s, len);
        SvUTF8_on(utf8);
        av_push(av, utf8);

        av_push(av, newSVuv(len));
        RETVAL = av;
    OUTPUT:
        RETVAL

AV *
test_toUPPER_uvchr(UV ord)
    PREINIT:
        U8 s[UTF8_MAXBYTES_CASE + 1];
        STRLEN len;
        AV *av;
        SV *utf8;
    CODE:
        av = newAV();
        av_push(av, newSVuv(toUPPER_uvchr(ord, s, &len)));

        utf8 = newSVpvn((char *) s, len);
        SvUTF8_on(utf8);
        av_push(av, utf8);

        av_push(av, newSVuv(len));
        RETVAL = av;
    OUTPUT:
        RETVAL

AV *
test_toUPPER_utf8(SV * p, int type)
    PREINIT:
        U8 *input;
        U8 s[UTF8_MAXBYTES_CASE + 1];
        STRLEN len;
        AV *av;
        SV *utf8;
        const U8 * e;
        UV resultant_cp = UV_MAX;
    CODE:
        input = (U8 *) SvPV(p, len);
        av = newAV();
        if (type >= 0) {
            e = input + UTF8SKIP(input) - type;
            resultant_cp = toUPPER_utf8_safe(input, e, s, &len);
            av_push(av, newSVuv(resultant_cp));

            utf8 = newSVpvn((char *) s, len);
            SvUTF8_on(utf8);
            av_push(av, utf8);

            av_push(av, newSVuv(len));
            RETVAL = av;
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

UV
test_toTITLE(UV ord)
    CODE:
        RETVAL = toTITLE(ord);
    OUTPUT:
        RETVAL

AV *
test_toTITLE_uni(UV ord)
    PREINIT:
        U8 s[UTF8_MAXBYTES_CASE + 1];
        STRLEN len;
        AV *av;
        SV *utf8;
    CODE:
        av = newAV();
        av_push(av, newSVuv(toTITLE_uni(ord, s, &len)));

        utf8 = newSVpvn((char *) s, len);
        SvUTF8_on(utf8);
        av_push(av, utf8);

        av_push(av, newSVuv(len));
        RETVAL = av;
    OUTPUT:
        RETVAL

AV *
test_toTITLE_uvchr(UV ord)
    PREINIT:
        U8 s[UTF8_MAXBYTES_CASE + 1];
        STRLEN len;
        AV *av;
        SV *utf8;
    CODE:
        av = newAV();
        av_push(av, newSVuv(toTITLE_uvchr(ord, s, &len)));

        utf8 = newSVpvn((char *) s, len);
        SvUTF8_on(utf8);
        av_push(av, utf8);

        av_push(av, newSVuv(len));
        RETVAL = av;
    OUTPUT:
        RETVAL

AV *
test_toTITLE_utf8(SV * p, int type)
    PREINIT:
        U8 *input;
        U8 s[UTF8_MAXBYTES_CASE + 1];
        STRLEN len;
        AV *av;
        SV *utf8;
        const U8 * e;
        UV resultant_cp = UV_MAX;
    CODE:
        input = (U8 *) SvPV(p, len);
        av = newAV();
        if (type >= 0) {
            e = input + UTF8SKIP(input) - type;
            resultant_cp = toTITLE_utf8_safe(input, e, s, &len);
            av_push(av, newSVuv(resultant_cp));

            utf8 = newSVpvn((char *) s, len);
            SvUTF8_on(utf8);
            av_push(av, utf8);

            av_push(av, newSVuv(len));
            RETVAL = av;
        }
        else {
            RETVAL = 0;
        }
    OUTPUT:
        RETVAL

AV *
test_delimcpy(SV * from_sv, STRLEN trunc_from, char delim, STRLEN to_len, STRLEN trunc_to, char poison = '?')
    PREINIT:
        char * from;
        I32 retlen;
        char * from_pos_after_copy;
        char * to;
    CODE:
        from = SvPV_nolen(from_sv);
        Newx(to, to_len, char);
        PoisonWith(to, to_len, char, poison);
        assert(trunc_from <= SvCUR(from_sv));
        /* trunc_to allows us to throttle the output size available */
        assert(trunc_to <= to_len);
        from_pos_after_copy = delimcpy(to, to + trunc_to,
                                       from, from + trunc_from,
                                       delim, &retlen);
        RETVAL = newAV();
        sv_2mortal((SV*)RETVAL);
        av_push(RETVAL, newSVpvn(to, to_len));
        av_push(RETVAL, newSVuv(retlen));
        av_push(RETVAL, newSVuv(from_pos_after_copy - from));
        Safefree(to);
    OUTPUT:
        RETVAL

AV *
test_delimcpy_no_escape(SV * from_sv, STRLEN trunc_from, char delim, STRLEN to_len, STRLEN trunc_to, char poison = '?')
    PREINIT:
        char * from;
        AV *av;
        I32 retlen;
        char * from_pos_after_copy;
        char * to;
    CODE:
        from = SvPV_nolen(from_sv);
        Newx(to, to_len, char);
        PoisonWith(to, to_len, char, poison);
        assert(trunc_from <= SvCUR(from_sv));
        /* trunc_to allows us to throttle the output size available */
        assert(trunc_to <= to_len);
        from_pos_after_copy = delimcpy_no_escape(to, to + trunc_to,
                                       from, from + trunc_from,
                                       delim, &retlen);
        av = newAV();
        av_push(av, newSVpvn(to, to_len));
        av_push(av, newSVuv(retlen));
        av_push(av, newSVuv(from_pos_after_copy - from));
        Safefree(to);
        RETVAL = av;
    OUTPUT:
        RETVAL

SV *
test_Gconvert(SV * number, SV * num_digits)
    PREINIT:
        char buffer[100];
        int len;
        int extras;
    CODE:
        len = (int) SvIV(num_digits);
        /* To silence a -Wformat-overflow compiler warning we     *
         * make allowance for the following characters that may   *
         * appear, in addition to the digits of the significand:  *
         * a leading "-", a single byte radix point, "e-", the    *
         * terminating NULL, and a 3 or 4 digit exponent.         *
         * Ie, allow 8 bytes if nvtype is "double", otherwise 9   *
         * bytes (as the exponent could then contain 4 digits ).  */
        extras = sizeof(NV) == 8 ? 8 : 9;
        if(len > 100 - extras)
            croak("Too long a number for test_Gconvert");
        if (len < 0)
            croak("Too short a number for test_Gconvert");
        PERL_UNUSED_RESULT(Gconvert(SvNV(number), len,
                 0,    /* No trailing zeroes */
                 buffer));
        RETVAL = newSVpv(buffer, 0);
    OUTPUT:
        RETVAL

SV *
test_Perl_langinfo(SV * item)
    CODE:
        RETVAL = newSVpv(Perl_langinfo(SvIV(item)), 0);
    OUTPUT:
        RETVAL

SV *
gimme()
    CODE:
        /* facilitate tests that GIMME_V gives the right result
         * in XS calls */
        int gimme = GIMME_V;
        SV* sv = get_sv("XS::APItest::GIMME_V", GV_ADD);
        sv_setiv_mg(sv, (IV)gimme);
        RETVAL = &PL_sv_undef;
    OUTPUT:
        RETVAL


MODULE = XS::APItest            PACKAGE = XS::APItest::Backrefs

void
apitest_weaken(SV *sv)
    PROTOTYPE: $
    CODE:
        sv_rvweaken(sv);

SV *
has_backrefs(SV *sv)
    CODE:
        if (SvROK(sv) && sv_get_backrefs(SvRV(sv)))
            RETVAL = &PL_sv_yes;
        else
            RETVAL = &PL_sv_no;
    OUTPUT:
        RETVAL

#ifdef WIN32
#ifdef PERL_IMPLICIT_SYS

const char *
PerlDir_mapA(const char *path)

const WCHAR *
PerlDir_mapW(const WCHAR *wpath)

#endif

void
Comctl32Version()
    PREINIT:
        HMODULE dll;
        VS_FIXEDFILEINFO *info;
        UINT len;
        HRSRC hrsc;
        HGLOBAL ver;
        void * vercopy;
    PPCODE:
        dll = GetModuleHandle("comctl32.dll"); /* must already be in proc */
        if(!dll)
            croak("Comctl32Version: comctl32.dll not in process???");
        hrsc = FindResource(dll,    MAKEINTRESOURCE(VS_VERSION_INFO),
                                    MAKEINTRESOURCE((Size_t)VS_FILE_INFO));
        if(!hrsc)
            croak("Comctl32Version: comctl32.dll no version???");
        ver = LoadResource(dll, hrsc);
        len = SizeofResource(dll, hrsc);
        vercopy = (void *)sv_grow(sv_newmortal(),len);
        memcpy(vercopy, ver, len);
        if (VerQueryValue(vercopy, "\\", (void**)&info, &len)) {
            int dwValueMS1 = (info->dwFileVersionMS>>16);
            int dwValueMS2 = (info->dwFileVersionMS&0xffff);
            int dwValueLS1 = (info->dwFileVersionLS>>16);
            int dwValueLS2 = (info->dwFileVersionLS&0xffff);
            EXTEND(SP, 4);
            mPUSHi(dwValueMS1);
            mPUSHi(dwValueMS2);
            mPUSHi(dwValueLS1);
            mPUSHi(dwValueLS2);
        }

#endif


MODULE = XS::APItest                PACKAGE = XS::APItest::RWMacro

#if defined(USE_ITHREADS)

void
compile_macros()
    PREINIT:
        perl_RnW1_mutex_t m;
        perl_RnW1_mutex_t *pm = &m;
    CODE:
        PERL_RW_MUTEX_INIT(&m);
        PERL_WRITE_LOCK(&m);
        PERL_WRITE_UNLOCK(&m);
        PERL_READ_LOCK(&m);
        PERL_READ_UNLOCK(&m);
        PERL_RW_MUTEX_DESTROY(&m);
        PERL_RW_MUTEX_INIT(pm);
        PERL_WRITE_LOCK(pm);
        PERL_WRITE_UNLOCK(pm);
        PERL_READ_LOCK(pm);
        PERL_READ_UNLOCK(pm);
        PERL_RW_MUTEX_DESTROY(pm);

#endif

MODULE = XS::APItest                PACKAGE = XS::APItest::HvMacro


UV
u8_to_u16_le(SV *sv, STRLEN ofs)
    ALIAS:
        u8_to_u32_le = 1
        u8_to_u64_le = 2
    CODE:
    {
        STRLEN len;
        char *pv= SvPV(sv,len);
        STRLEN minlen= 2<<ix;
        U16 u16;
        U32 u32;
        U64 u64;
        RETVAL= 0; /* silence warnings about uninitialized RETVAL */
        switch (ix) {
            case 0:
                if (ofs+minlen>len) croak("cowardly refusing to read past end of string in u8_to_u16_le");
                u16= U8TO16_LE(pv+ofs);
                RETVAL= (UV)u16;
                break;
            case 1:
                if (ofs+minlen>len) croak("cowardly refusing to read past end of string in u8_to_u32_le");
                u32= U8TO32_LE(pv+ofs);
                RETVAL= (UV)u32;
                break;
            case 2:
#if TEST_64BIT
                if (ofs+minlen>len) croak("cowardly refusing to read past end of string in u8_to_u64_le");
                u64= U8TO64_LE(pv+ofs);
                RETVAL= (UV)u64;
#else
                PERL_UNUSED_VAR(u64);
                croak("not a 64 bit perl IVSIZE=%d",IVSIZE);
#endif
                break;
        }
    }
    OUTPUT:
        RETVAL

U32
rotl32(U32 n, U8 r)
    CODE:
    {
        RETVAL= ROTL32(n,r);
    }
    OUTPUT:
        RETVAL

U32
rotr32(U32 n, U8 r)
    CODE:
    {
        RETVAL= ROTR32(n,r);
    }
    OUTPUT:
        RETVAL

#if TEST_64BIT

UV
rotl64(UV n, U8 r)
    CODE:
    {
        RETVAL= ROTL64(n,r);
    }
    OUTPUT:
        RETVAL

UV
rotr64(UV n, U8 r)
    CODE:
    {
        RETVAL= ROTR64(n,r);
    }
    OUTPUT:
        RETVAL

SV *
siphash_seed_state(SV *seed_sv)
    CODE:
    {
        U8 state_buf[sizeof(U64)*4];
        STRLEN seed_len;
        U8 *seed_pv= (U8*)SvPV(seed_sv,seed_len);
        if (seed_len<16)  croak("seed should be 16 bytes long");
        else if (seed_len>16) warn("only using the first 16 bytes of seed");
        RETVAL= newSV(sizeof(U64)*4+3);
        S_perl_siphash_seed_state(seed_pv,state_buf);
        sv_setpvn(RETVAL,(char*)state_buf,sizeof(U64)*4);
    }
    OUTPUT:
        RETVAL


UV
siphash24(SV *state_sv, SV *str_sv)
    ALIAS:
        siphash13 = 1
    CODE:
    {
        STRLEN state_len;
        STRLEN str_len;
        U8 *str_pv= (U8*)SvPV(str_sv,str_len);
        /* (U8*)SvPV(state_sv, state_len) return differs between little-endian *
         * and big-endian. It's the same values, but in a different order.     *
         * On big-endian architecture, we transpose the values into the same   *
         * order as for little-endian, so that we can test against the same    *
         * test vectors.                                                       *
         * We could alternatively alter the code that produced state_sv to     *
         * output identical arrangements for big-endian and little-endian.     */
#if BYTEORDER == 0x1234 || BYTEORDER == 0x12345678
        U8 *state_pv= (U8*)SvPV(state_sv,state_len);
        if (state_len!=32) croak("siphash state should be exactly 32 bytes");
#else
        U8 *temp_pv = (U8*)SvPV(state_sv, state_len);
        U8 state_pv[32];
        int i;
        if (state_len!=32) croak("siphash state should be exactly 32 bytes");
        for( i = 0; i < 32; i++ ) {
            if     (i <  8) state_pv[ 7 - i] = temp_pv[i];
            else if(i < 16) state_pv[23 - i] = temp_pv[i];
            else if(i < 24) state_pv[39 - i] = temp_pv[i];
            else            state_pv[55 - i] = temp_pv[i];
        }
#endif
        if (ix) {
            RETVAL= S_perl_hash_siphash_1_3_with_state_64(state_pv,str_pv,str_len);
        } else {
            RETVAL= S_perl_hash_siphash_2_4_with_state_64(state_pv,str_pv,str_len);
        }
    }
    OUTPUT:
        RETVAL


UV
test_siphash24()
    CODE:
    {
        U8 vectors[64][8] = {
              { 0x31, 0x0e, 0x0e, 0xdd, 0x47, 0xdb, 0x6f, 0x72, },
              { 0xfd, 0x67, 0xdc, 0x93, 0xc5, 0x39, 0xf8, 0x74, },
              { 0x5a, 0x4f, 0xa9, 0xd9, 0x09, 0x80, 0x6c, 0x0d, },
              { 0x2d, 0x7e, 0xfb, 0xd7, 0x96, 0x66, 0x67, 0x85, },
              { 0xb7, 0x87, 0x71, 0x27, 0xe0, 0x94, 0x27, 0xcf, },
              { 0x8d, 0xa6, 0x99, 0xcd, 0x64, 0x55, 0x76, 0x18, },
              { 0xce, 0xe3, 0xfe, 0x58, 0x6e, 0x46, 0xc9, 0xcb, },
              { 0x37, 0xd1, 0x01, 0x8b, 0xf5, 0x00, 0x02, 0xab, },
              { 0x62, 0x24, 0x93, 0x9a, 0x79, 0xf5, 0xf5, 0x93, },
              { 0xb0, 0xe4, 0xa9, 0x0b, 0xdf, 0x82, 0x00, 0x9e, },
              { 0xf3, 0xb9, 0xdd, 0x94, 0xc5, 0xbb, 0x5d, 0x7a, },
              { 0xa7, 0xad, 0x6b, 0x22, 0x46, 0x2f, 0xb3, 0xf4, },
              { 0xfb, 0xe5, 0x0e, 0x86, 0xbc, 0x8f, 0x1e, 0x75, },
              { 0x90, 0x3d, 0x84, 0xc0, 0x27, 0x56, 0xea, 0x14, },
              { 0xee, 0xf2, 0x7a, 0x8e, 0x90, 0xca, 0x23, 0xf7, },
              { 0xe5, 0x45, 0xbe, 0x49, 0x61, 0xca, 0x29, 0xa1, },
              { 0xdb, 0x9b, 0xc2, 0x57, 0x7f, 0xcc, 0x2a, 0x3f, },
              { 0x94, 0x47, 0xbe, 0x2c, 0xf5, 0xe9, 0x9a, 0x69, },
              { 0x9c, 0xd3, 0x8d, 0x96, 0xf0, 0xb3, 0xc1, 0x4b, },
              { 0xbd, 0x61, 0x79, 0xa7, 0x1d, 0xc9, 0x6d, 0xbb, },
              { 0x98, 0xee, 0xa2, 0x1a, 0xf2, 0x5c, 0xd6, 0xbe, },
              { 0xc7, 0x67, 0x3b, 0x2e, 0xb0, 0xcb, 0xf2, 0xd0, },
              { 0x88, 0x3e, 0xa3, 0xe3, 0x95, 0x67, 0x53, 0x93, },
              { 0xc8, 0xce, 0x5c, 0xcd, 0x8c, 0x03, 0x0c, 0xa8, },
              { 0x94, 0xaf, 0x49, 0xf6, 0xc6, 0x50, 0xad, 0xb8, },
              { 0xea, 0xb8, 0x85, 0x8a, 0xde, 0x92, 0xe1, 0xbc, },
              { 0xf3, 0x15, 0xbb, 0x5b, 0xb8, 0x35, 0xd8, 0x17, },
              { 0xad, 0xcf, 0x6b, 0x07, 0x63, 0x61, 0x2e, 0x2f, },
              { 0xa5, 0xc9, 0x1d, 0xa7, 0xac, 0xaa, 0x4d, 0xde, },
              { 0x71, 0x65, 0x95, 0x87, 0x66, 0x50, 0xa2, 0xa6, },
              { 0x28, 0xef, 0x49, 0x5c, 0x53, 0xa3, 0x87, 0xad, },
              { 0x42, 0xc3, 0x41, 0xd8, 0xfa, 0x92, 0xd8, 0x32, },
              { 0xce, 0x7c, 0xf2, 0x72, 0x2f, 0x51, 0x27, 0x71, },
              { 0xe3, 0x78, 0x59, 0xf9, 0x46, 0x23, 0xf3, 0xa7, },
              { 0x38, 0x12, 0x05, 0xbb, 0x1a, 0xb0, 0xe0, 0x12, },
              { 0xae, 0x97, 0xa1, 0x0f, 0xd4, 0x34, 0xe0, 0x15, },
              { 0xb4, 0xa3, 0x15, 0x08, 0xbe, 0xff, 0x4d, 0x31, },
              { 0x81, 0x39, 0x62, 0x29, 0xf0, 0x90, 0x79, 0x02, },
              { 0x4d, 0x0c, 0xf4, 0x9e, 0xe5, 0xd4, 0xdc, 0xca, },
              { 0x5c, 0x73, 0x33, 0x6a, 0x76, 0xd8, 0xbf, 0x9a, },
              { 0xd0, 0xa7, 0x04, 0x53, 0x6b, 0xa9, 0x3e, 0x0e, },
              { 0x92, 0x59, 0x58, 0xfc, 0xd6, 0x42, 0x0c, 0xad, },
              { 0xa9, 0x15, 0xc2, 0x9b, 0xc8, 0x06, 0x73, 0x18, },
              { 0x95, 0x2b, 0x79, 0xf3, 0xbc, 0x0a, 0xa6, 0xd4, },
              { 0xf2, 0x1d, 0xf2, 0xe4, 0x1d, 0x45, 0x35, 0xf9, },
              { 0x87, 0x57, 0x75, 0x19, 0x04, 0x8f, 0x53, 0xa9, },
              { 0x10, 0xa5, 0x6c, 0xf5, 0xdf, 0xcd, 0x9a, 0xdb, },
              { 0xeb, 0x75, 0x09, 0x5c, 0xcd, 0x98, 0x6c, 0xd0, },
              { 0x51, 0xa9, 0xcb, 0x9e, 0xcb, 0xa3, 0x12, 0xe6, },
              { 0x96, 0xaf, 0xad, 0xfc, 0x2c, 0xe6, 0x66, 0xc7, },
              { 0x72, 0xfe, 0x52, 0x97, 0x5a, 0x43, 0x64, 0xee, },
              { 0x5a, 0x16, 0x45, 0xb2, 0x76, 0xd5, 0x92, 0xa1, },
              { 0xb2, 0x74, 0xcb, 0x8e, 0xbf, 0x87, 0x87, 0x0a, },
              { 0x6f, 0x9b, 0xb4, 0x20, 0x3d, 0xe7, 0xb3, 0x81, },
              { 0xea, 0xec, 0xb2, 0xa3, 0x0b, 0x22, 0xa8, 0x7f, },
              { 0x99, 0x24, 0xa4, 0x3c, 0xc1, 0x31, 0x57, 0x24, },
              { 0xbd, 0x83, 0x8d, 0x3a, 0xaf, 0xbf, 0x8d, 0xb7, },
              { 0x0b, 0x1a, 0x2a, 0x32, 0x65, 0xd5, 0x1a, 0xea, },
              { 0x13, 0x50, 0x79, 0xa3, 0x23, 0x1c, 0xe6, 0x60, },
              { 0x93, 0x2b, 0x28, 0x46, 0xe4, 0xd7, 0x06, 0x66, },
              { 0xe1, 0x91, 0x5f, 0x5c, 0xb1, 0xec, 0xa4, 0x6c, },
              { 0xf3, 0x25, 0x96, 0x5c, 0xa1, 0x6d, 0x62, 0x9f, },
              { 0x57, 0x5f, 0xf2, 0x8e, 0x60, 0x38, 0x1b, 0xe5, },
              { 0x72, 0x45, 0x06, 0xeb, 0x4c, 0x32, 0x8a, 0x95, }
            };
        U32 vectors_32[64] = {
            0xaf61d576,
            0xe7245e38,
            0xd4c5cf53,
            0x529c18bb,
            0xe8561357,
            0xd5eff3e9,
            0x9337a5a0,
            0x2003d1c2,
            0x0966d11b,
            0x95a9666f,
            0xee800236,
            0xd6d882e1,
            0xf3106a47,
            0xd46e6bb7,
            0x7959387e,
            0xe8978f84,
            0x68e857a4,
            0x4524ae61,
            0xdd4c606c,
            0x1c14a8a0,
            0xa474b26a,
            0xfec9ac77,
            0x70f0591d,
            0x6550cd44,
            0x4ee4ff52,
            0x36642a34,
            0x4c63204b,
            0x2845aece,
            0x79506309,
            0x21373517,
            0xf1ce4c7b,
            0xea9951b8,
            0x03d52de1,
            0x5eaa5ba5,
            0xa9e5a222,
            0x1a41a37a,
            0x39585c0a,
            0x2b1ba971,
            0x5428d8a8,
            0xf08cab2a,
            0x5d3a0ebb,
            0x51541b44,
            0x83b11361,
            0x27df2129,
            0x1dc758ef,
            0xb026d883,
            0x2ef668cf,
            0x8c65ed26,
            0x78d90a9a,
            0x3bcb49ba,
            0x7936bd28,
            0x13d7c32c,
            0x844cf30d,
            0xa1077c52,
            0xdc1acee1,
            0x18f31558,
            0x8d003c12,
            0xd830cf6e,
            0xc39f4c30,
            0x202efc77,
            0x30fb7d50,
            0xc3f44852,
            0x6be96737,
            0x7e8c773e
        };

        const U8 MAXLEN= 64;
        U8 in[64], seed_pv[16], state_pv[32];
        union {
            U64 hash;
            U32 h32[2];
            U8 bytes[8];
        } out;
        int i,j;
        int failed = 0;
        U32 hash32;
        /* S_perl_siphash_seed_state(seed_pv, state_pv) sets state_pv          *
         * differently between little-endian and big-endian. It's the same     *
         * values, but in a different order.                                   *
         * On big-endian architecture, we transpose the values into the same   *
         * order as for little-endian, so that we can test against the same    *
         * test vectors.                                                       *
         * We could alternatively alter the code that produces state_pv to     *
         * output identical arrangements for big-endian and little-endian.     */
#if BYTEORDER == 0x1234 || BYTEORDER == 0x12345678
        for( i = 0; i < 16; ++i ) seed_pv[i] = i;
        S_perl_siphash_seed_state(seed_pv, state_pv);
#else
        U8 temp_pv[32];
        for( i = 0; i < 16; ++i ) seed_pv[i] = i;
        S_perl_siphash_seed_state(seed_pv, temp_pv);
        for( i = 0; i < 32; ++i ) {
            if     (i <  8) state_pv[ 7 - i] = temp_pv[i];
            else if(i < 16) state_pv[23 - i] = temp_pv[i];
            else if(i < 24) state_pv[39 - i] = temp_pv[i];
            else            state_pv[55 - i] = temp_pv[i];
        }
#endif
        for( i = 0; i < MAXLEN; ++i )
        {
            in[i] = i;

            out.hash= S_perl_hash_siphash_2_4_with_state_64( state_pv, in, i );

            hash32= S_perl_hash_siphash_2_4_with_state( state_pv, in, i);
            /* The test vectors need to reversed here for big-endian architecture   *
             * Alternatively we could rewrite S_perl_hash_siphash_2_4_with_state_64 *
             * to produce reversed vectors when run on big-endian architecture      */
#if BYTEORDER == 0x4321 || BYTEORDER == 0x87654321 /* reverse order of vectors[i] */
            temp_pv   [0] = vectors[i][0]; /* temp_pv is temporary holder of vectors[i][0] */
            vectors[i][0] = vectors[i][7];
            vectors[i][7] = temp_pv[0];

            temp_pv   [0] = vectors[i][1]; /* temp_pv is temporary holder of vectors[i][1] */
            vectors[i][1] = vectors[i][6];
            vectors[i][6] = temp_pv[0];

            temp_pv   [0] = vectors[i][2]; /* temp_pv is temporary holder of vectors[i][2] */
            vectors[i][2] = vectors[i][5];
            vectors[i][5] = temp_pv[0];

            temp_pv   [0] = vectors[i][3]; /* temp_pv is temporary holder of vectors[i][3] */
            vectors[i][3] = vectors[i][4];
            vectors[i][4] = temp_pv[0];
#endif
            if ( memcmp( out.bytes, vectors[i], 8 ) )
            {
                failed++;
                printf( "Error in 64 bit result on test vector of length %d for siphash24\n    have: {", i );
                for (j=0;j<7;j++)
                    printf( "0x%02x, ", out.bytes[j]);
                printf( "0x%02x },\n", out.bytes[7]);
                printf( "    want: {" );
                for (j=0;j<7;j++)
                    printf( "0x%02x, ", vectors[i][j]);
                printf( "0x%02x },\n", vectors[i][7]);
            }
            if (hash32 != vectors_32[i]) {
                failed++;
                printf( "Error in 32 bit result on test vector of length %d for siphash24\n"
                        "    have: 0x%08" UVxf "\n"
                        "    want: 0x%08" UVxf "\n",
                    i, (UV)hash32, (UV)vectors_32[i]);
            }
        }
        RETVAL= failed;
    }
    OUTPUT:
        RETVAL

UV
test_siphash13()
    CODE:
    {
        U8 vectors[64][8] = {
            {0xdc, 0xc4, 0x0f, 0x05, 0x58, 0x01, 0xac, 0xab },
            {0x93, 0xca, 0x57, 0x7d, 0xf3, 0x9b, 0xf4, 0xc9 },
            {0x4d, 0xd4, 0xc7, 0x4d, 0x02, 0x9b, 0xcb, 0x82 },
            {0xfb, 0xf7, 0xdd, 0xe7, 0xb8, 0x0a, 0xf8, 0x8b },
            {0x28, 0x83, 0xd3, 0x88, 0x60, 0x57, 0x75, 0xcf },
            {0x67, 0x3b, 0x53, 0x49, 0x2f, 0xd5, 0xf9, 0xde },
            {0xa7, 0x22, 0x9f, 0xc5, 0x50, 0x2b, 0x0d, 0xc5 },
            {0x40, 0x11, 0xb1, 0x9b, 0x98, 0x7d, 0x92, 0xd3 },
            {0x8e, 0x9a, 0x29, 0x8d, 0x11, 0x95, 0x90, 0x36 },
            {0xe4, 0x3d, 0x06, 0x6c, 0xb3, 0x8e, 0xa4, 0x25 },
            {0x7f, 0x09, 0xff, 0x92, 0xee, 0x85, 0xde, 0x79 },
            {0x52, 0xc3, 0x4d, 0xf9, 0xc1, 0x18, 0xc1, 0x70 },
            {0xa2, 0xd9, 0xb4, 0x57, 0xb1, 0x84, 0xa3, 0x78 },
            {0xa7, 0xff, 0x29, 0x12, 0x0c, 0x76, 0x6f, 0x30 },
            {0x34, 0x5d, 0xf9, 0xc0, 0x11, 0xa1, 0x5a, 0x60 },
            {0x56, 0x99, 0x51, 0x2a, 0x6d, 0xd8, 0x20, 0xd3 },
            {0x66, 0x8b, 0x90, 0x7d, 0x1a, 0xdd, 0x4f, 0xcc },
            {0x0c, 0xd8, 0xdb, 0x63, 0x90, 0x68, 0xf2, 0x9c },
            {0x3e, 0xe6, 0x73, 0xb4, 0x9c, 0x38, 0xfc, 0x8f },
            {0x1c, 0x7d, 0x29, 0x8d, 0xe5, 0x9d, 0x1f, 0xf2 },
            {0x40, 0xe0, 0xcc, 0xa6, 0x46, 0x2f, 0xdc, 0xc0 },
            {0x44, 0xf8, 0x45, 0x2b, 0xfe, 0xab, 0x92, 0xb9 },
            {0x2e, 0x87, 0x20, 0xa3, 0x9b, 0x7b, 0xfe, 0x7f },
            {0x23, 0xc1, 0xe6, 0xda, 0x7f, 0x0e, 0x5a, 0x52 },
            {0x8c, 0x9c, 0x34, 0x67, 0xb2, 0xae, 0x64, 0xf4 },
            {0x79, 0x09, 0x5b, 0x70, 0x28, 0x59, 0xcd, 0x45 },
            {0xa5, 0x13, 0x99, 0xca, 0xe3, 0x35, 0x3e, 0x3a },
            {0x35, 0x3b, 0xde, 0x4a, 0x4e, 0xc7, 0x1d, 0xa9 },
            {0x0d, 0xd0, 0x6c, 0xef, 0x02, 0xed, 0x0b, 0xfb },
            {0xf4, 0xe1, 0xb1, 0x4a, 0xb4, 0x3c, 0xd9, 0x88 },
            {0x63, 0xe6, 0xc5, 0x43, 0xd6, 0x11, 0x0f, 0x54 },
            {0xbc, 0xd1, 0x21, 0x8c, 0x1f, 0xdd, 0x70, 0x23 },
            {0x0d, 0xb6, 0xa7, 0x16, 0x6c, 0x7b, 0x15, 0x81 },
            {0xbf, 0xf9, 0x8f, 0x7a, 0xe5, 0xb9, 0x54, 0x4d },
            {0x3e, 0x75, 0x2a, 0x1f, 0x78, 0x12, 0x9f, 0x75 },
            {0x91, 0x6b, 0x18, 0xbf, 0xbe, 0xa3, 0xa1, 0xce },
            {0x06, 0x62, 0xa2, 0xad, 0xd3, 0x08, 0xf5, 0x2c },
            {0x57, 0x30, 0xc3, 0xa3, 0x2d, 0x1c, 0x10, 0xb6 },
            {0xa1, 0x36, 0x3a, 0xae, 0x96, 0x74, 0xf4, 0xb3 },
            {0x92, 0x83, 0x10, 0x7b, 0x54, 0x57, 0x6b, 0x62 },
            {0x31, 0x15, 0xe4, 0x99, 0x32, 0x36, 0xd2, 0xc1 },
            {0x44, 0xd9, 0x1a, 0x3f, 0x92, 0xc1, 0x7c, 0x66 },
            {0x25, 0x88, 0x13, 0xc8, 0xfe, 0x4f, 0x70, 0x65 },
            {0xa6, 0x49, 0x89, 0xc2, 0xd1, 0x80, 0xf2, 0x24 },
            {0x6b, 0x87, 0xf8, 0xfa, 0xed, 0x1c, 0xca, 0xc2 },
            {0x96, 0x21, 0x04, 0x9f, 0xfc, 0x4b, 0x16, 0xc2 },
            {0x23, 0xd6, 0xb1, 0x68, 0x93, 0x9c, 0x6e, 0xa1 },
            {0xfd, 0x14, 0x51, 0x8b, 0x9c, 0x16, 0xfb, 0x49 },
            {0x46, 0x4c, 0x07, 0xdf, 0xf8, 0x43, 0x31, 0x9f },
            {0xb3, 0x86, 0xcc, 0x12, 0x24, 0xaf, 0xfd, 0xc6 },
            {0x8f, 0x09, 0x52, 0x0a, 0xd1, 0x49, 0xaf, 0x7e },
            {0x9a, 0x2f, 0x29, 0x9d, 0x55, 0x13, 0xf3, 0x1c },
            {0x12, 0x1f, 0xf4, 0xa2, 0xdd, 0x30, 0x4a, 0xc4 },
            {0xd0, 0x1e, 0xa7, 0x43, 0x89, 0xe9, 0xfa, 0x36 },
            {0xe6, 0xbc, 0xf0, 0x73, 0x4c, 0xb3, 0x8f, 0x31 },
            {0x80, 0xe9, 0xa7, 0x70, 0x36, 0xbf, 0x7a, 0xa2 },
            {0x75, 0x6d, 0x3c, 0x24, 0xdb, 0xc0, 0xbc, 0xb4 },
            {0x13, 0x15, 0xb7, 0xfd, 0x52, 0xd8, 0xf8, 0x23 },
            {0x08, 0x8a, 0x7d, 0xa6, 0x4d, 0x5f, 0x03, 0x8f },
            {0x48, 0xf1, 0xe8, 0xb7, 0xe5, 0xd0, 0x9c, 0xd8 },
            {0xee, 0x44, 0xa6, 0xf7, 0xbc, 0xe6, 0xf4, 0xf6 },
            {0xf2, 0x37, 0x18, 0x0f, 0xd8, 0x9a, 0xc5, 0xae },
            {0xe0, 0x94, 0x66, 0x4b, 0x15, 0xf6, 0xb2, 0xc3 },
            {0xa8, 0xb3, 0xbb, 0xb7, 0x62, 0x90, 0x19, 0x9d }
        };
        U32 vectors_32[64] = {
            0xaea3c584,
            0xb4a35160,
            0xcf0c4f4f,
            0x6c25fd43,
            0x47a6d448,
            0x97aaee48,
            0x009209f7,
            0x48236cd8,
            0xbbb90f9f,
            0x49a2b357,
            0xeb218c91,
            0x898cdb93,
            0x2f175d13,
            0x224689ab,
            0xa0a3fc25,
            0xf971413b,
            0xb1df567c,
            0xff29b09c,
            0x3b8fdea2,
            0x7f36e0f9,
            0x6610cf06,
            0x92d753ba,
            0xdcdefcb5,
            0x88bccf5c,
            0x9350323e,
            0x35965051,
            0xf0a72646,
            0xe3c3fc7b,
            0x14673d0f,
            0xc268dd40,
            0x17caf7b5,
            0xaf510ca3,
            0x97b2cd61,
            0x37db405a,
            0x6ab56746,
            0x71b9c82f,
            0x81576ad5,
            0x15d32c7a,
            0x1dce4237,
            0x197bd4c6,
            0x58362303,
            0x596618d6,
            0xad63c7db,
            0xe67bc977,
            0x38329b86,
            0x5d126a6a,
            0xc9df4ab0,
            0xc2aa0261,
            0x40360fbe,
            0xd4312997,
            0x74fd405e,
            0x81da3ccf,
            0x66be2fcf,
            0x755df759,
            0x427f0faa,
            0xd2dd56b6,
            0x9080adae,
            0xde4fcd41,
            0x297ed545,
            0x6f7421ad,
            0x0152a252,
            0xa1ddad2a,
            0x88d462f5,
            0x2aa223ca,
        };

        const U8 MAXLEN= 64;
        U8 in[64], seed_pv[16], state_pv[32];
        union {
            U64 hash;
            U32 h32[2];
            U8 bytes[8];
        } out;
        int i,j;
        int failed = 0;
        U32 hash32;
        /* S_perl_siphash_seed_state(seed_pv, state_pv) sets state_pv          *
         * differently between little-endian and big-endian. It's the same     *
         * values, but in a different order.                                   *
         * On big-endian architecture, we transpose the values into the same   *
         * order as for little-endian, so that we can test against the same    *
         * test vectors.                                                       *
         * We could alternatively alter the code that produces state_pv to     *
         * output identical arrangements for big-endian and little-endian.     */
#if BYTEORDER == 0x1234 || BYTEORDER == 0x12345678
        for( i = 0; i < 16; ++i ) seed_pv[i] = i;
        S_perl_siphash_seed_state(seed_pv, state_pv);
#else
        U8 temp_pv[32];
        for( i = 0; i < 16; ++i ) seed_pv[i] = i;
        S_perl_siphash_seed_state(seed_pv, temp_pv);
        for( i = 0; i < 32; ++i ) {
            if     (i <  8) state_pv[ 7 - i] = temp_pv[i];
            else if(i < 16) state_pv[23 - i] = temp_pv[i];
            else if(i < 24) state_pv[39 - i] = temp_pv[i];
            else            state_pv[55 - i] = temp_pv[i];
        }
#endif
        for( i = 0; i < MAXLEN;  ++i )
        {
            in[i] = i;

            out.hash= S_perl_hash_siphash_1_3_with_state_64( state_pv, in, i );

            hash32= S_perl_hash_siphash_1_3_with_state( state_pv, in, i);
            /* The test vectors need to reversed here for big-endian architecture   *
             * Alternatively we could rewrite S_perl_hash_siphash_1_3_with_state_64 *
             * to produce reversed vectors when run on big-endian architecture      */
#if BYTEORDER == 0x4321 || BYTEORDER == 0x87654321
            temp_pv   [0] = vectors[i][0]; /* temp_pv is temporary holder of vectors[i][0] */
            vectors[i][0] = vectors[i][7];
            vectors[i][7] = temp_pv[0];

            temp_pv   [0] = vectors[i][1]; /* temp_pv is temporary holder of vectors[i][1] */
            vectors[i][1] = vectors[i][6];
            vectors[i][6] = temp_pv[0];

            temp_pv   [0] = vectors[i][2]; /* temp_pv is temporary holder of vectors[i][2] */
            vectors[i][2] = vectors[i][5];
            vectors[i][5] = temp_pv[0];

            temp_pv   [0] = vectors[i][3]; /* temp_pv is temporary holder of vectors[i][3] */
            vectors[i][3] = vectors[i][4];
            vectors[i][4] = temp_pv[0];
#endif
            if ( memcmp( out.bytes, vectors[i], 8 ) )
            {
                failed++;
                printf( "Error in 64 bit result on test vector of length %d for siphash13\n    have: {", i );
                for (j=0;j<7;j++)
                    printf( "0x%02x, ", out.bytes[j]);
                printf( "0x%02x },\n", out.bytes[7]);
                printf( "    want: {" );
                for (j=0;j<7;j++)
                    printf( "0x%02x, ", vectors[i][j]);
                printf( "0x%02x },\n", vectors[i][7]);
            }
            if (hash32 != vectors_32[i]) {
                failed++;
                printf( "Error in 32 bit result on test vector of length %d for siphash13\n"
                        "    have: 0x%08" UVxf"\n"
                        "    want: 0x%08" UVxf"\n",
                    i, (UV)hash32, (UV)vectors_32[i]);
            }
        }
        RETVAL= failed;
    }
    OUTPUT:
        RETVAL

#endif /* END 64 BIT SIPHASH TESTS */

MODULE = XS::APItest            PACKAGE = XS::APItest::BoolInternals

UV
test_bool_internals()
    CODE:
    {
        U32 failed = 0;
        SV *true_sv_setsv = newSV(0);
        SV *false_sv_setsv = newSV(0);
        SV *true_sv_set_true = newSV(0);
        SV *false_sv_set_false = newSV(0);
        SV *true_sv_set_bool = newSV(0);
        SV *false_sv_set_bool = newSV(0);
        SV *sviv = newSViv(1);
        SV *svpv = newSVpvs("whatever");
        TEST_EXPR(SvIOK(sviv) && !SvIandPOK(sviv));
        TEST_EXPR(SvPOK(svpv) && !SvIandPOK(svpv));
        TEST_EXPR(SvIOK(sviv) && !SvBoolFlagsOK(sviv));
        TEST_EXPR(SvPOK(svpv) && !SvBoolFlagsOK(svpv));
        sv_setsv(true_sv_setsv, &PL_sv_yes);
        sv_setsv(false_sv_setsv, &PL_sv_no);
        sv_set_true(true_sv_set_true);
        sv_set_false(false_sv_set_false);
        sv_set_bool(true_sv_set_bool, true);
        sv_set_bool(false_sv_set_bool, false);
        /* note that test_bool_internals_macro() SvREFCNT_dec's its arguments
         * after the tests */
        failed += test_bool_internals_macro(newSVsv(&PL_sv_yes), newSVsv(&PL_sv_no));
        failed += test_bool_internals_macro(newSV_true(), newSV_false());
        failed += test_bool_internals_macro(newSVbool(1), newSVbool(0));
        failed += test_bool_internals_macro(true_sv_setsv, false_sv_setsv);
        failed += test_bool_internals_macro(true_sv_set_true, false_sv_set_false);
        failed += test_bool_internals_macro(true_sv_set_bool, false_sv_set_bool);
        SvREFCNT_dec(sviv);
        SvREFCNT_dec(svpv);
        RETVAL = failed;
    }
    OUTPUT:
        RETVAL

MODULE = XS::APItest            PACKAGE = XS::APItest::CvREFCOUNTED_ANYSV

UV
test_CvREFCOUNTED_ANYSV()
    CODE:
    {
        U32 failed = 0;

        /* Doesn't matter what actual function we wrap because we're never
         * actually going to call it. */
        CV *cv = newXS("XS::APItest::(test-cv-1)", XS_XS__APItest__XSUB_XS_VERSION_undef, __FILE__);
        SV *sv = newSV(0);
        CvXSUBANY(cv).any_sv = SvREFCNT_inc(sv);
        CvREFCOUNTED_ANYSV_on(cv);
        TEST_EXPR(SvREFCNT(sv) == 2);

        SvREFCNT_dec((SV *)cv);
        TEST_EXPR(SvREFCNT(sv) == 1);

        SvREFCNT_dec(sv);

        RETVAL = failed;
    }
    OUTPUT:
        RETVAL

MODULE = XS::APItest            PACKAGE = XS::APItest::global_locale

char *
switch_to_global_and_setlocale(int category, const char * locale)
    CODE:
        switch_to_global_locale();
        RETVAL = setlocale(category, locale);
    OUTPUT:
        RETVAL

bool
sync_locale()
    CODE:
        RETVAL = sync_locale();
    OUTPUT:
        RETVAL

NV
newSvNV(const char * string)
    CODE:
        RETVAL = SvNV(newSVpv(string, 0));
    OUTPUT:
        RETVAL

MODULE = XS::APItest            PACKAGE = XS::APItest::savestack

IV
get_savestack_ix()
    CODE:
        RETVAL = PL_savestack_ix;
    OUTPUT:
        RETVAL
