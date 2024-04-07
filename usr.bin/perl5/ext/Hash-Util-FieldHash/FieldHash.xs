#define PERL_NO_GET_CONTEXT

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

/* support for Hash::Util::FieldHash, prefix HUF_ */

/* A Perl sub that returns a hashref to the object registry */
#define HUF_OB_REG "Hash::Util::FieldHash::_ob_reg"
/* Identifier for PERL_MAGIC_ext magic */
#define HUF_IDCACHE 0x4944

/* For global cache of object registry */
#define MY_CXT_KEY "Hash::Util::FieldHash::_guts" XS_VERSION
typedef struct {
    HV* ob_reg; /* Cache object registry */
} my_cxt_t;
START_MY_CXT

/* Inquire the object registry (a lexical hash) from perl */
static HV *
HUF_get_ob_reg(pTHX) {
    dSP;
    HV* ob_reg = NULL;
    I32 items;
    ENTER;
    SAVETMPS;

    PUSHMARK(SP);
    items = call_pv(HUF_OB_REG, G_SCALAR|G_NOARGS);
    SPAGAIN;

    if (items == 1 && TOPs && SvROK(TOPs) && SvTYPE(SvRV(TOPs)) == SVt_PVHV)
        ob_reg = (HV*)SvRV(POPs);
    PUTBACK;
    FREETMPS;
    LEAVE;

    if (!ob_reg)
        Perl_die(aTHX_ "Can't get object registry hash");
    return ob_reg;
}

/* Deal with global context */
#define HUF_INIT 1
#define HUF_CLONE 0
#define HUF_RESET -1

static void
HUF_global(pTHX_ I32 how) {
    if (how == HUF_INIT) {
        MY_CXT_INIT;
        MY_CXT.ob_reg = HUF_get_ob_reg(aTHX);
    } else if (how == HUF_CLONE) {
        MY_CXT_CLONE;
        MY_CXT.ob_reg = HUF_get_ob_reg(aTHX);
    } else if (how == HUF_RESET) {
        dMY_CXT;
        MY_CXT.ob_reg = HUF_get_ob_reg(aTHX);
    }
}

/* Object id */

/* definition of id transformation */
#define HUF_OBJ_ID(x) newSVuv(PTR2UV(x))

static SV *
HUF_obj_id(pTHX_ SV *obj) {
    SV *item = SvRV(obj);
    MAGIC *mg;
    SV *id;

    /* Get cached object ID, if it exists */
    if (SvTYPE(item) >= SVt_PVMG) {
        for ( mg = SvMAGIC(item); mg; mg = mg->mg_moremagic ) {
            if ((mg->mg_type == PERL_MAGIC_ext) &&
                (mg->mg_private == HUF_IDCACHE)
            ) {
                return mg->mg_obj;
            }
        }
    }

    /* Create an object ID, cache it */
    id = HUF_OBJ_ID(item);
    mg = sv_magicext(item, id, PERL_MAGIC_ext, NULL, NULL, 0);
    mg->mg_private = HUF_IDCACHE;
    SvREFCNT_dec(id); /* refcnt++ in sv_magicext() */

    /* Return the object ID */
    return id;
}

/* set up uvar magic for any sv */
static void
HUF_add_uvar_magic(
    pTHX_
    SV* sv,                    /* the sv to enchant, visible to get/set */
    I32(* val)(pTHX_ IV, SV*), /* "get" function */
    I32(* set)(pTHX_ IV, SV*), /* "set" function */
    I32 index,                 /* get/set will see this */
    SV* thing                  /* any associated info */
) {
    struct ufuncs uf;
        uf.uf_val = val;
        uf.uf_set = set;
        uf.uf_index = index;
    sv_magic(sv, thing, PERL_MAGIC_uvar, (char*)&uf, sizeof(uf));
}

/* Fetch the data container of a trigger */
static AV *
HUF_get_trigger_content(pTHX_ SV *trigger) {
    MAGIC* mg;
    if (trigger && (mg = mg_find(trigger, PERL_MAGIC_uvar)))
        return (AV*)mg->mg_obj;
    return NULL;
}

/* Delete an object from all field hashes it may occur in.  Also delete
 * the object's entry from the object registry.  This function goes in
 * the uf_set field of the uvar magic of a trigger.
 */
static I32 HUF_destroy_obj(pTHX_ IV index, SV *trigger) {
    PERL_UNUSED_ARG(index);
    /* Do nothing if the weakref wasn't undef'd.  Also don't bother
     * during global destruction.  (MY_CXT.ob_reg is sometimes funny there) */
    if (!SvROK(trigger) && (!PL_in_clean_all)) {
        dMY_CXT;
        AV* cont = HUF_get_trigger_content(aTHX_ trigger);
        SV* ob_id = *av_fetch(cont, 0, 0);
        HV* field_tab = (HV*) *av_fetch(cont, 1, 0);
        HE* ent;
        hv_iterinit(field_tab);
        while ((ent = hv_iternext(field_tab))) {
            SV* field_ref = HeVAL(ent);
            SV* field = SvRV(field_ref);
            (void) hv_delete_ent((HV*)field, ob_id, 0, 0);
        }
        /* make it safe in case we must run in global clenaup, after all */
        if (PL_in_clean_all)
            HUF_global(aTHX_ HUF_RESET); /* shoudn't be needed */
        (void) hv_delete_ent(MY_CXT.ob_reg, ob_id, 0, 0);
    }
    return 0;
}

/* Create a trigger for an object.  The trigger is a magical SV
 * that holds a weak ref to the object.  The magic fires when the object
 * expires and takes care of garbage collection in registred hashes.
 * For that purpose, the magic structure holds the original id of
 * the object, and a list (a hash, really) of hashes from which the
 * object may * have to be deleted.  The trigger is stored in the
 * object registry and is also deleted when the object expires.
 */
static SV *
HUF_new_trigger(pTHX_ SV *obj, SV *ob_id) {
    dMY_CXT;
    SV* trigger = sv_rvweaken(newRV_inc(SvRV(obj)));
    AV* cont = newAV();
    sv_2mortal((SV*)cont);
    av_store(cont, 0, SvREFCNT_inc(ob_id));
    av_store(cont, 1, (SV*)newHV());
    HUF_add_uvar_magic(aTHX_ trigger, NULL, &HUF_destroy_obj, 0, (SV*)cont);
    (void) hv_store_ent(MY_CXT.ob_reg, ob_id, trigger, 0);
    return trigger;
}

/* retrieve a trigger for obj if one exists, return NULL otherwise */
static SV *
HUF_ask_trigger(pTHX_ SV *ob_id) {
    dMY_CXT;
    HE* ent;
    if ((ent = hv_fetch_ent(MY_CXT.ob_reg, ob_id, 0, 0)))
        return HeVAL(ent);
    return NULL;
}

static SV *
HUF_get_trigger(pTHX_ SV *obj, SV *ob_id) {
    SV* trigger = HUF_ask_trigger(aTHX_ ob_id);
    if (!trigger)
        trigger = HUF_new_trigger(aTHX_ obj, ob_id);
    return( trigger);
}

/* mark an object (trigger) as having been used with a field
   (a clenup-liability)
*/
static void
HUF_mark_field(pTHX_ SV *trigger, SV *field) {
    AV* cont = HUF_get_trigger_content(aTHX_ trigger);
    HV* field_tab = (HV*) *av_fetch(cont, 1, 0);
    SV* field_ref = newRV_inc(field);
    UV field_addr = PTR2UV(field);
    (void) hv_store(field_tab, (char *)&field_addr, sizeof(field_addr), field_ref, 0);
}

/* Determine, from the value of action, whether this call may create a new
 * hash key */
#define HUF_WOULD_CREATE_KEY(x) ((x) != HV_DELETE && ((x) & (HV_FETCH_ISSTORE | HV_FETCH_LVALUE)))

/* The key exchange functions.  They communicate with S_hv_magic_uvar_xkey
 * in hv.c */
static I32 HUF_watch_key_safe(pTHX_ IV action, SV* field) {
    MAGIC* mg = mg_find(field, PERL_MAGIC_uvar);
    SV* keysv;
    if (mg && (keysv = mg->mg_obj)) {
        if (SvROK(keysv)) { /* ref key */
            SV* ob_id = HUF_obj_id(aTHX_ keysv);
            mg->mg_obj = ob_id; /* key replacement */
            if (HUF_WOULD_CREATE_KEY(action)) {
                SV* trigger = HUF_get_trigger(aTHX_ keysv, ob_id);
                HUF_mark_field(aTHX_ trigger, field);
            }
        } else if (HUF_WOULD_CREATE_KEY(action)) { /* string key */
            /* registered as object id? */
            SV* trigger;
            if (( trigger = HUF_ask_trigger(aTHX_ keysv)))
                HUF_mark_field(aTHX_ trigger, field);
        }
    } else {
        Perl_die(aTHX_ "Rogue call of 'HUF_watch_key_safe'");
    }
    return 0;
}

static I32 HUF_watch_key_id(pTHX_ IV action, SV* field) {
    MAGIC* mg = mg_find(field, PERL_MAGIC_uvar);
    SV* keysv;
    PERL_UNUSED_ARG(action);
    if (mg && (keysv = mg->mg_obj)) {
        if (SvROK(keysv)) /* ref key */
            mg->mg_obj = HUF_obj_id(aTHX_ keysv); /* key replacement */
    } else {
        Perl_die(aTHX_ "Rogue call of 'HUF_watch_key_id'");
    }
    return 0;
}

static int HUF_func_2mode( I32(* val)(pTHX_ IV, SV*)) {
    int ans = 0;
    if (val == &HUF_watch_key_id)
        ans = 1;
    if (val == &HUF_watch_key_safe)
        ans = 2;
    return(ans);
}

static I32(* HUF_mode_2func( int mode))(pTHX_ IV, SV*) {
    I32(* ans)(pTHX_ IV, SV*) = NULL;
    switch (mode) {
        case 1:
            ans = &HUF_watch_key_id;
            break;
        case 2:
            ans = &HUF_watch_key_safe;
            break;
    }
    return(ans);
}

/* see if something is a field hash */
static int
HUF_get_status(pTHX_ HV *hash) {
    int ans = 0;
    if (hash && (SvTYPE(hash) == SVt_PVHV)) {
        MAGIC* mg;
        struct ufuncs* uf;
        if ((mg = mg_find((SV*)hash, PERL_MAGIC_uvar)) &&
            (uf = (struct ufuncs *)mg->mg_ptr) &&
            (uf->uf_set == NULL)
        ) {
            ans = HUF_func_2mode(uf->uf_val);
        }
    }
    return ans;
}

/* Thread support.  These routines are called by CLONE (and nothing else) */

/* Fix entries for one object in all field hashes */
static void
HUF_fix_trigger(pTHX_ SV *trigger, SV *new_id) {
    AV* cont = HUF_get_trigger_content(aTHX_ trigger);
    HV* field_tab = (HV*) *av_fetch(cont, 1, 0);
    HV* new_tab = newHV();
    HE* ent;
    SV* old_id = *av_fetch(cont, 0, 0);
    I32 entries = hv_iterinit(field_tab);
    hv_ksplit(new_tab, entries);
    while ((ent = hv_iternext(field_tab))) {
        SV* field_ref = HeVAL(ent);
        HV* field = (HV*)SvRV(field_ref);
        UV field_addr = PTR2UV(field);
        SV* val;
        /* recreate field tab entry */
        (void) hv_store(new_tab, (char *)&field_addr, sizeof(field_addr), SvREFCNT_inc(field_ref), 0);
        /* recreate field entry, if any */
        if ((val = hv_delete_ent(field, old_id, 0, 0)))
            (void) hv_store_ent(field, new_id, SvREFCNT_inc(val), 0);
    }
    /* update the trigger */
    av_store(cont, 0, SvREFCNT_inc(new_id));
    av_store(cont, 1, (SV*)new_tab);
}

/* Go over object registry and fix all objects.  Also fix the object
 * registry.
 */
static void
HUF_fix_objects(pTHX) {
    dMY_CXT;
    I32 i, len;
    HE* ent;
    AV* oblist = (AV*)sv_2mortal((SV*)newAV());
    hv_iterinit(MY_CXT.ob_reg);
    while((ent = hv_iternext(MY_CXT.ob_reg)))
        av_push(oblist, SvREFCNT_inc(hv_iterkeysv(ent)));
    len = av_count(oblist);
    for (i = 0; i < len; ++i) {
        SV* old_id = *av_fetch(oblist, i, 0);
        SV* trigger = hv_delete_ent(MY_CXT.ob_reg, old_id, 0, 0);
        SV* obj = SvRV(trigger);
        MAGIC *mg;

        SV* new_id = HUF_OBJ_ID(obj);

        /* Replace cached object ID with this new one */
        for (mg = SvMAGIC(obj); mg; mg = mg->mg_moremagic) {
            if ((mg->mg_type == PERL_MAGIC_ext) &&
                    (mg->mg_private == HUF_IDCACHE)
                ) {
                mg->mg_obj = new_id;
            }
        }

        HUF_fix_trigger(aTHX_ trigger, new_id);
        (void) hv_store_ent(MY_CXT.ob_reg, new_id, SvREFCNT_inc(trigger), 0);
    }
}

/* test support (not needed for functionality) */

static SV* counter;
I32 HUF_inc_var(pTHX_ IV index, SV* which) {
    PERL_UNUSED_ARG(index);
    PERL_UNUSED_ARG(which);
    sv_setiv(counter, 1 + SvIV(counter));
    return 0;
}

MODULE = Hash::Util::FieldHash          PACKAGE = Hash::Util::FieldHash

BOOT:
{
    HUF_global(aTHX_ HUF_INIT); /* create variables */
}

int
_fieldhash(SV* href, int mode)
PROTOTYPE: $$
CODE:
    HV* field;
    RETVAL = 0;
    if (mode &&
        href && SvROK(href) &&
        (field = (HV*)SvRV(href)) &&
        SvTYPE(field) == SVt_PVHV
    ) {
        
        HUF_add_uvar_magic(
            aTHX_
            SvRV(href),
            HUF_mode_2func(mode),
            NULL,
            0,
            NULL
        );
        RETVAL = HUF_get_status(aTHX_ field);
    }
OUTPUT:
    RETVAL

void
id(SV* ref)
PROTOTYPE: $
PPCODE:
    if (SvROK(ref)) {
        XPUSHs(HUF_obj_id(aTHX_ ref));
    } else {
        XPUSHs(ref);
    }

SV*
id_2obj(SV* id)
PROTOTYPE: $
CODE:
    SV* obj = HUF_ask_trigger(aTHX_ id);
    if (obj) {
        RETVAL = newRV_inc(SvRV(obj));
    } else {
        RETVAL = &PL_sv_undef;
    }
OUTPUT:
    RETVAL

SV*
register(SV* obj, ...)
PROTOTYPE: $@
CODE:
    SV* trigger;
    int i;
    RETVAL = NULL;
    if (!SvROK(obj)) {
        Perl_die(aTHX_ "Attempt to register a non-ref");
    } else {
        RETVAL = newRV_inc(SvRV(obj));
    }
    trigger = HUF_get_trigger(aTHX_ obj, HUF_obj_id(aTHX_ obj));
    for (i = 1; i < items; ++ i) {
        SV* field_ref = POPs;
        if (SvROK(field_ref) && (SvTYPE(SvRV(field_ref)) == SVt_PVHV)) {
            HUF_mark_field(aTHX_ trigger, SvRV(field_ref));
        }
    }
OUTPUT:
    RETVAL

void
CLONE(char* classname)
CODE:
    if (strEQ(classname, "Hash::Util::FieldHash")) {
        HUF_global(aTHX_ HUF_CLONE);
        HUF_fix_objects(aTHX);
    }

void
_active_fields(SV* obj)
PPCODE:
    if (SvROK(obj)) {
        SV* ob_id = HUF_obj_id(aTHX_ obj);
        SV* trigger = HUF_ask_trigger(aTHX_ ob_id);
        if (trigger) {
            AV* cont = HUF_get_trigger_content(aTHX_ trigger);
            HV* field_tab = (HV*) *av_fetch(cont, 1, 0);
            HE* ent;
            hv_iterinit(field_tab);
            while ((ent = hv_iternext(field_tab))) {
                HV* field = (HV*)SvRV(HeVAL(ent));
                if (hv_exists_ent(field, ob_id, 0))
                    XPUSHs(sv_2mortal(newRV_inc((SV*)field)));
            }
        }
    }

void
_test_uvar_get(SV* svref, SV* countref)
ALIAS:
_test_uvar_get = 1
_test_uvar_set = 2
_test_uvar_same = 3
CODE:
    if (SvROK(svref) && SvROK(countref)) {
        counter = SvRV(countref);
        sv_setiv(counter, 0);
        HUF_add_uvar_magic(
            aTHX_
            SvRV(svref),
            ix & 1 ? &HUF_inc_var : 0,
            ix & 2 ? &HUF_inc_var : 0,
	    0,
            SvRV(countref)
        );
    }
