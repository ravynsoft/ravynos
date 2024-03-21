/************************************************************
 * Copyright (c) 1994 by Silicon Graphics Computer Systems, Inc.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of Silicon Graphics not be
 * used in advertising or publicity pertaining to distribution
 * of the software without specific prior written permission.
 * Silicon Graphics makes no representation about the suitability
 * of this software for any purpose. It is provided "as is"
 * without any express or implied warranty.
 *
 * SILICON GRAPHICS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SILICON
 * GRAPHICS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
 * THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 ********************************************************/

/*
 * Copyright © 2012 Ran Benita <ran234@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "config.h"

#include "xkbcomp-priv.h"
#include "text.h"
#include "expr.h"
#include "action.h"
#include "vmod.h"
#include "include.h"

enum si_field {
    SI_FIELD_VIRTUAL_MOD = (1 << 0),
    SI_FIELD_ACTION = (1 << 1),
    SI_FIELD_AUTO_REPEAT = (1 << 2),
    SI_FIELD_LEVEL_ONE_ONLY = (1 << 3),
};

typedef struct {
    enum si_field defined;
    enum merge_mode merge;

    struct xkb_sym_interpret interp;
} SymInterpInfo;

enum led_field {
    LED_FIELD_MODS = (1 << 0),
    LED_FIELD_GROUPS = (1 << 1),
    LED_FIELD_CTRLS = (1 << 2),
};

typedef struct {
    enum led_field defined;
    enum merge_mode merge;

    struct xkb_led led;
} LedInfo;

typedef struct {
    char *name;
    int errorCount;
    SymInterpInfo default_interp;
    darray(SymInterpInfo) interps;
    LedInfo default_led;
    LedInfo leds[XKB_MAX_LEDS];
    unsigned int num_leds;
    ActionsInfo *actions;
    struct xkb_mod_set mods;

    struct xkb_context *ctx;
} CompatInfo;

static const char *
siText(SymInterpInfo *si, CompatInfo *info)
{
    char *buf = xkb_context_get_buffer(info->ctx, 128);

    if (si == &info->default_interp)
        return "default";

    snprintf(buf, 128, "%s+%s(%s)",
             KeysymText(info->ctx, si->interp.sym),
             SIMatchText(si->interp.match),
             ModMaskText(info->ctx, &info->mods, si->interp.mods));

    return buf;
}

static inline bool
ReportSINotArray(CompatInfo *info, SymInterpInfo *si, const char *field)
{
    return ReportNotArray(info->ctx, "symbol interpretation", field,
                          siText(si, info));
}

static inline bool
ReportSIBadType(CompatInfo *info, SymInterpInfo *si, const char *field,
                const char *wanted)
{
    return ReportBadType(info->ctx, XKB_ERROR_WRONG_FIELD_TYPE,
                         "symbol interpretation", field,
                         siText(si, info), wanted);
}

static inline bool
ReportLedBadType(CompatInfo *info, LedInfo *ledi, const char *field,
                 const char *wanted)
{
    return ReportBadType(info->ctx, XKB_ERROR_WRONG_FIELD_TYPE,
                         "indicator map", field,
                         xkb_atom_text(info->ctx, ledi->led.name),
                         wanted);
}

static inline bool
ReportLedNotArray(CompatInfo *info, LedInfo *ledi, const char *field)
{
    return ReportNotArray(info->ctx, "indicator map", field,
                          xkb_atom_text(info->ctx, ledi->led.name));
}

static void
InitCompatInfo(CompatInfo *info, struct xkb_context *ctx,
               ActionsInfo *actions, const struct xkb_mod_set *mods)
{
    memset(info, 0, sizeof(*info));
    info->ctx = ctx;
    info->actions = actions;
    info->mods = *mods;
    info->default_interp.merge = MERGE_OVERRIDE;
    info->default_interp.interp.virtual_mod = XKB_MOD_INVALID;
    info->default_led.merge = MERGE_OVERRIDE;
}

static void
ClearCompatInfo(CompatInfo *info)
{
    free(info->name);
    darray_free(info->interps);
}

static SymInterpInfo *
FindMatchingInterp(CompatInfo *info, SymInterpInfo *new)
{
    SymInterpInfo *old;

    darray_foreach(old, info->interps)
        if (old->interp.sym == new->interp.sym &&
            old->interp.mods == new->interp.mods &&
            old->interp.match == new->interp.match)
            return old;

    return NULL;
}

static bool
UseNewInterpField(enum si_field field, SymInterpInfo *old, SymInterpInfo *new,
                  bool report, enum si_field *collide)
{
    if (!(old->defined & field))
        return true;

    if (new->defined & field) {
        if (report)
            *collide |= field;

        if (new->merge != MERGE_AUGMENT)
            return true;
    }

    return false;
}

static bool
AddInterp(CompatInfo *info, SymInterpInfo *new, bool same_file)
{
    SymInterpInfo *old = FindMatchingInterp(info, new);
    if (old) {
        const int verbosity = xkb_context_get_log_verbosity(info->ctx);
        const bool report = (same_file && verbosity > 0) || verbosity > 9;
        enum si_field collide = 0;

        if (new->merge == MERGE_REPLACE) {
            if (report)
                log_warn(info->ctx, XKB_LOG_MESSAGE_NO_ID,
                         "Multiple definitions for \"%s\"; "
                         "Earlier interpretation ignored\n",
                         siText(new, info));
            *old = *new;
            return true;
        }

        if (UseNewInterpField(SI_FIELD_VIRTUAL_MOD, old, new, report,
                              &collide)) {
            old->interp.virtual_mod = new->interp.virtual_mod;
            old->defined |= SI_FIELD_VIRTUAL_MOD;
        }
        if (UseNewInterpField(SI_FIELD_ACTION, old, new, report,
                              &collide)) {
            old->interp.action = new->interp.action;
            old->defined |= SI_FIELD_ACTION;
        }
        if (UseNewInterpField(SI_FIELD_AUTO_REPEAT, old, new, report,
                              &collide)) {
            old->interp.repeat = new->interp.repeat;
            old->defined |= SI_FIELD_AUTO_REPEAT;
        }
        if (UseNewInterpField(SI_FIELD_LEVEL_ONE_ONLY, old, new, report,
                              &collide)) {
            old->interp.level_one_only = new->interp.level_one_only;
            old->defined |= SI_FIELD_LEVEL_ONE_ONLY;
        }

        if (collide) {
            log_warn(info->ctx, XKB_LOG_MESSAGE_NO_ID,
                     "Multiple interpretations of \"%s\"; "
                     "Using %s definition for duplicate fields\n",
                     siText(new, info),
                     (new->merge != MERGE_AUGMENT ? "last" : "first"));
        }

        return true;
    }

    darray_append(info->interps, *new);
    return true;
}

/***====================================================================***/

static bool
ResolveStateAndPredicate(ExprDef *expr, enum xkb_match_operation *pred_rtrn,
                         xkb_mod_mask_t *mods_rtrn, CompatInfo *info)
{
    if (expr == NULL) {
        *pred_rtrn = MATCH_ANY_OR_NONE;
        *mods_rtrn = MOD_REAL_MASK_ALL;
        return true;
    }

    *pred_rtrn = MATCH_EXACTLY;
    if (expr->expr.op == EXPR_ACTION_DECL) {
        const char *pred_txt = xkb_atom_text(info->ctx, expr->action.name);
        if (!LookupString(symInterpretMatchMaskNames, pred_txt, pred_rtrn) ||
            !expr->action.args || expr->action.args->common.next) {
            log_err(info->ctx, XKB_LOG_MESSAGE_NO_ID,
                    "Illegal modifier predicate \"%s\"; Ignored\n", pred_txt);
            return false;
        }
        expr = expr->action.args;
    }
    else if (expr->expr.op == EXPR_IDENT) {
        const char *pred_txt = xkb_atom_text(info->ctx, expr->ident.ident);
        if (pred_txt && istreq(pred_txt, "any")) {
            *pred_rtrn = MATCH_ANY;
            *mods_rtrn = MOD_REAL_MASK_ALL;
            return true;
        }
    }

    return ExprResolveModMask(info->ctx, expr, MOD_REAL, &info->mods,
                              mods_rtrn);
}

/***====================================================================***/

static bool
UseNewLEDField(enum led_field field, LedInfo *old, LedInfo *new,
               bool report, enum led_field *collide)
{
    if (!(old->defined & field))
        return true;

    if (new->defined & field) {
        if (report)
            *collide |= field;

        if (new->merge != MERGE_AUGMENT)
            return true;
    }

    return false;
}

static bool
AddLedMap(CompatInfo *info, LedInfo *new, bool same_file)
{
    enum led_field collide;
    const int verbosity = xkb_context_get_log_verbosity(info->ctx);
    const bool report = (same_file && verbosity > 0) || verbosity > 9;

    for (xkb_led_index_t i = 0; i < info->num_leds; i++) {
        LedInfo *old = &info->leds[i];

        if (old->led.name != new->led.name)
            continue;

        if (old->led.mods.mods == new->led.mods.mods &&
            old->led.groups == new->led.groups &&
            old->led.ctrls == new->led.ctrls &&
            old->led.which_mods == new->led.which_mods &&
            old->led.which_groups == new->led.which_groups) {
            old->defined |= new->defined;
            return true;
        }

        if (new->merge == MERGE_REPLACE) {
            if (report)
                log_warn(info->ctx, XKB_LOG_MESSAGE_NO_ID,
                         "Map for indicator %s redefined; "
                         "Earlier definition ignored\n",
                         xkb_atom_text(info->ctx, old->led.name));
            *old = *new;
            return true;
        }

        collide = 0;
        if (UseNewLEDField(LED_FIELD_MODS, old, new, report, &collide)) {
            old->led.which_mods = new->led.which_mods;
            old->led.mods = new->led.mods;
            old->defined |= LED_FIELD_MODS;
        }
        if (UseNewLEDField(LED_FIELD_GROUPS, old, new, report, &collide)) {
            old->led.which_groups = new->led.which_groups;
            old->led.groups = new->led.groups;
            old->defined |= LED_FIELD_GROUPS;
        }
        if (UseNewLEDField(LED_FIELD_CTRLS, old, new, report, &collide)) {
            old->led.ctrls = new->led.ctrls;
            old->defined |= LED_FIELD_CTRLS;
        }

        if (collide) {
            log_warn(info->ctx, XKB_LOG_MESSAGE_NO_ID,
                     "Map for indicator %s redefined; "
                     "Using %s definition for duplicate fields\n",
                     xkb_atom_text(info->ctx, old->led.name),
                     (new->merge == MERGE_AUGMENT ? "first" : "last"));
        }

        return true;
    }

    if (info->num_leds >= XKB_MAX_LEDS) {
        log_err(info->ctx, XKB_LOG_MESSAGE_NO_ID,
                "Too many LEDs defined (maximum %d)\n",
                XKB_MAX_LEDS);
        return false;
    }
    info->leds[info->num_leds++] = *new;
    return true;
}

static void
MergeIncludedCompatMaps(CompatInfo *into, CompatInfo *from,
                        enum merge_mode merge)
{
    if (from->errorCount > 0) {
        into->errorCount += from->errorCount;
        return;
    }

    into->mods = from->mods;

    if (into->name == NULL) {
        into->name = from->name;
        from->name = NULL;
    }

    if (darray_empty(into->interps)) {
        into->interps = from->interps;
        darray_init(from->interps);
    }
    else {
        SymInterpInfo *si;
        darray_foreach(si, from->interps) {
            si->merge = (merge == MERGE_DEFAULT ? si->merge : merge);
            if (!AddInterp(into, si, false))
                into->errorCount++;
        }
    }

    if (into->num_leds == 0) {
        memcpy(into->leds, from->leds, sizeof(*from->leds) * from->num_leds);
        into->num_leds = from->num_leds;
        from->num_leds = 0;
    }
    else {
        for (xkb_led_index_t i = 0; i < from->num_leds; i++) {
            LedInfo *ledi = &from->leds[i];
            ledi->merge = (merge == MERGE_DEFAULT ? ledi->merge : merge);
            if (!AddLedMap(into, ledi, false))
                into->errorCount++;
        }
    }
}

static void
HandleCompatMapFile(CompatInfo *info, XkbFile *file, enum merge_mode merge);

static bool
HandleIncludeCompatMap(CompatInfo *info, IncludeStmt *include)
{
    CompatInfo included;

    InitCompatInfo(&included, info->ctx, info->actions, &info->mods);
    included.name = include->stmt;
    include->stmt = NULL;

    for (IncludeStmt *stmt = include; stmt; stmt = stmt->next_incl) {
        CompatInfo next_incl;
        XkbFile *file;

        file = ProcessIncludeFile(info->ctx, stmt, FILE_TYPE_COMPAT);
        if (!file) {
            info->errorCount += 10;
            ClearCompatInfo(&included);
            return false;
        }

        InitCompatInfo(&next_incl, info->ctx, info->actions, &included.mods);
        next_incl.default_interp = info->default_interp;
        next_incl.default_interp.merge = stmt->merge;
        next_incl.default_led = info->default_led;
        next_incl.default_led.merge = stmt->merge;

        HandleCompatMapFile(&next_incl, file, MERGE_OVERRIDE);

        MergeIncludedCompatMaps(&included, &next_incl, stmt->merge);

        ClearCompatInfo(&next_incl);
        FreeXkbFile(file);
    }

    MergeIncludedCompatMaps(info, &included, include->merge);
    ClearCompatInfo(&included);

    return (info->errorCount == 0);
}

static bool
SetInterpField(CompatInfo *info, SymInterpInfo *si, const char *field,
               ExprDef *arrayNdx, ExprDef *value)
{
    xkb_mod_index_t ndx;

    if (istreq(field, "action")) {
        if (arrayNdx)
            return ReportSINotArray(info, si, field);

        if (!HandleActionDef(info->ctx, info->actions, &info->mods,
                             value, &si->interp.action))
            return false;

        si->defined |= SI_FIELD_ACTION;
    }
    else if (istreq(field, "virtualmodifier") ||
             istreq(field, "virtualmod")) {
        if (arrayNdx)
            return ReportSINotArray(info, si, field);

        if (!ExprResolveMod(info->ctx, value, MOD_VIRT, &info->mods, &ndx))
            return ReportSIBadType(info, si, field, "virtual modifier");

        si->interp.virtual_mod = ndx;
        si->defined |= SI_FIELD_VIRTUAL_MOD;
    }
    else if (istreq(field, "repeat")) {
        bool set;

        if (arrayNdx)
            return ReportSINotArray(info, si, field);

        if (!ExprResolveBoolean(info->ctx, value, &set))
            return ReportSIBadType(info, si, field, "boolean");

        si->interp.repeat = set;

        si->defined |= SI_FIELD_AUTO_REPEAT;
    }
    else if (istreq(field, "locking")) {
        log_dbg(info->ctx, XKB_LOG_MESSAGE_NO_ID,
                "The \"locking\" field in symbol interpretation is unsupported; "
                "Ignored\n");
    }
    else if (istreq(field, "usemodmap") ||
             istreq(field, "usemodmapmods")) {
        unsigned int val;

        if (arrayNdx)
            return ReportSINotArray(info, si, field);

        if (!ExprResolveEnum(info->ctx, value, &val, useModMapValueNames))
            return ReportSIBadType(info, si, field, "level specification");

        si->interp.level_one_only = val;
        si->defined |= SI_FIELD_LEVEL_ONE_ONLY;
    }
    else {
        return ReportBadField(info->ctx, "symbol interpretation", field,
                              siText(si, info));
    }

    return true;
}

static bool
SetLedMapField(CompatInfo *info, LedInfo *ledi, const char *field,
               ExprDef *arrayNdx, ExprDef *value)
{
    bool ok = true;

    if (istreq(field, "modifiers") || istreq(field, "mods")) {
        if (arrayNdx)
            return ReportLedNotArray(info, ledi, field);

        if (!ExprResolveModMask(info->ctx, value, MOD_BOTH,
                                &info->mods, &ledi->led.mods.mods))
            return ReportLedBadType(info, ledi, field, "modifier mask");

        ledi->defined |= LED_FIELD_MODS;
    }
    else if (istreq(field, "groups")) {
        unsigned int mask;

        if (arrayNdx)
            return ReportLedNotArray(info, ledi, field);

        if (!ExprResolveMask(info->ctx, value, &mask, groupMaskNames))
            return ReportLedBadType(info, ledi, field, "group mask");

        ledi->led.groups = mask;
        ledi->defined |= LED_FIELD_GROUPS;
    }
    else if (istreq(field, "controls") || istreq(field, "ctrls")) {
        unsigned int mask;

        if (arrayNdx)
            return ReportLedNotArray(info, ledi, field);

        if (!ExprResolveMask(info->ctx, value, &mask, ctrlMaskNames))
            return ReportLedBadType(info, ledi, field, "controls mask");

        ledi->led.ctrls = mask;
        ledi->defined |= LED_FIELD_CTRLS;
    }
    else if (istreq(field, "allowexplicit")) {
        log_dbg(info->ctx, XKB_LOG_MESSAGE_NO_ID,
                "The \"allowExplicit\" field in indicator statements is unsupported; "
                "Ignored\n");
    }
    else if (istreq(field, "whichmodstate") ||
             istreq(field, "whichmodifierstate")) {
        unsigned int mask;

        if (arrayNdx)
            return ReportLedNotArray(info, ledi, field);

        if (!ExprResolveMask(info->ctx, value, &mask,
                             modComponentMaskNames))
            return ReportLedBadType(info, ledi, field,
                                    "mask of modifier state components");

        ledi->led.which_mods = mask;
    }
    else if (istreq(field, "whichgroupstate")) {
        unsigned mask;

        if (arrayNdx)
            return ReportLedNotArray(info, ledi, field);

        if (!ExprResolveMask(info->ctx, value, &mask,
                             groupComponentMaskNames))
            return ReportLedBadType(info, ledi, field,
                                    "mask of group state components");

        ledi->led.which_groups = mask;
    }
    else if (istreq(field, "driveskbd") ||
             istreq(field, "driveskeyboard") ||
             istreq(field, "leddriveskbd") ||
             istreq(field, "leddriveskeyboard") ||
             istreq(field, "indicatordriveskbd") ||
             istreq(field, "indicatordriveskeyboard")) {
        log_dbg(info->ctx, XKB_LOG_MESSAGE_NO_ID,
                "The \"%s\" field in indicator statements is unsupported; "
                "Ignored\n", field);
    }
    else if (istreq(field, "index")) {
        /* Users should see this, it might cause unexpected behavior. */
        log_err(info->ctx, XKB_LOG_MESSAGE_NO_ID,
                "The \"index\" field in indicator statements is unsupported; "
                "Ignored\n");
    }
    else {
        log_err(info->ctx, XKB_LOG_MESSAGE_NO_ID,
                "Unknown field %s in map for %s indicator; "
                "Definition ignored\n",
                field, xkb_atom_text(info->ctx, ledi->led.name));
        ok = false;
    }

    return ok;
}

static bool
HandleGlobalVar(CompatInfo *info, VarDef *stmt)
{
    const char *elem, *field;
    ExprDef *ndx;
    bool ret;

    if (!ExprResolveLhs(info->ctx, stmt->name, &elem, &field, &ndx))
        ret = false;
    else if (elem && istreq(elem, "interpret"))
        ret = SetInterpField(info, &info->default_interp, field, ndx,
                             stmt->value);
    else if (elem && istreq(elem, "indicator"))
        ret = SetLedMapField(info, &info->default_led, field, ndx,
                             stmt->value);
    else
        ret = SetActionField(info->ctx, info->actions, &info->mods,
                             elem, field, ndx, stmt->value);
    return ret;
}

static bool
HandleInterpBody(CompatInfo *info, VarDef *def, SymInterpInfo *si)
{
    bool ok = true;
    const char *elem, *field;
    ExprDef *arrayNdx;

    for (; def; def = (VarDef *) def->common.next) {
        if (def->name && def->name->expr.op == EXPR_FIELD_REF) {
            log_err(info->ctx, XKB_LOG_MESSAGE_NO_ID,
                    "Cannot set a global default value from within an interpret statement; "
                    "Move statements to the global file scope\n");
            ok = false;
            continue;
        }

        ok = ExprResolveLhs(info->ctx, def->name, &elem, &field, &arrayNdx);
        if (!ok)
            continue;

        ok = SetInterpField(info, si, field, arrayNdx, def->value);
    }

    return ok;
}

static bool
HandleInterpDef(CompatInfo *info, InterpDef *def, enum merge_mode merge)
{
    enum xkb_match_operation pred;
    xkb_mod_mask_t mods;
    SymInterpInfo si;

    if (!ResolveStateAndPredicate(def->match, &pred, &mods, info)) {
        log_err(info->ctx, XKB_LOG_MESSAGE_NO_ID,
                "Couldn't determine matching modifiers; "
                "Symbol interpretation ignored\n");
        return false;
    }

    si = info->default_interp;
    si.merge = (def->merge == MERGE_DEFAULT ? merge : def->merge);
    si.interp.sym = def->sym;
    si.interp.match = pred;
    si.interp.mods = mods;

    if (!HandleInterpBody(info, def->def, &si)) {
        info->errorCount++;
        return false;
    }

    if (!AddInterp(info, &si, true)) {
        info->errorCount++;
        return false;
    }

    return true;
}

static bool
HandleLedMapDef(CompatInfo *info, LedMapDef *def, enum merge_mode merge)
{
    LedInfo ledi;
    VarDef *var;
    bool ok;

    if (def->merge != MERGE_DEFAULT)
        merge = def->merge;

    ledi = info->default_led;
    ledi.merge = merge;
    ledi.led.name = def->name;

    ok = true;
    for (var = def->body; var != NULL; var = (VarDef *) var->common.next) {
        const char *elem, *field;
        ExprDef *arrayNdx;
        if (!ExprResolveLhs(info->ctx, var->name, &elem, &field, &arrayNdx)) {
            ok = false;
            continue;
        }

        if (elem) {
            log_err(info->ctx, XKB_LOG_MESSAGE_NO_ID,
                    "Cannot set defaults for \"%s\" element in indicator map; "
                    "Assignment to %s.%s ignored\n", elem, elem, field);
            ok = false;
        }
        else {
            ok = SetLedMapField(info, &ledi, field, arrayNdx, var->value) && ok;
        }
    }

    if (ok)
        return AddLedMap(info, &ledi, true);

    return false;
}

static void
HandleCompatMapFile(CompatInfo *info, XkbFile *file, enum merge_mode merge)
{
    bool ok;

    merge = (merge == MERGE_DEFAULT ? MERGE_AUGMENT : merge);

    free(info->name);
    info->name = strdup_safe(file->name);

    for (ParseCommon *stmt = file->defs; stmt; stmt = stmt->next) {
        switch (stmt->type) {
        case STMT_INCLUDE:
            ok = HandleIncludeCompatMap(info, (IncludeStmt *) stmt);
            break;
        case STMT_INTERP:
            ok = HandleInterpDef(info, (InterpDef *) stmt, merge);
            break;
        case STMT_GROUP_COMPAT:
            log_dbg(info->ctx, XKB_LOG_MESSAGE_NO_ID,
                    "The \"group\" statement in compat is unsupported; "
                    "Ignored\n");
            ok = true;
            break;
        case STMT_LED_MAP:
            ok = HandleLedMapDef(info, (LedMapDef *) stmt, merge);
            break;
        case STMT_VAR:
            ok = HandleGlobalVar(info, (VarDef *) stmt);
            break;
        case STMT_VMOD:
            ok = HandleVModDef(info->ctx, &info->mods, (VModDef *) stmt, merge);
            break;
        default:
            log_err(info->ctx, XKB_LOG_MESSAGE_NO_ID,
                    "Compat files may not include other types; "
                    "Ignoring %s\n", stmt_type_to_string(stmt->type));
            ok = false;
            break;
        }

        if (!ok)
            info->errorCount++;

        if (info->errorCount > 10) {
            log_err(info->ctx, XKB_LOG_MESSAGE_NO_ID,
                    "Abandoning compatibility map \"%s\"\n", file->name);
            break;
        }
    }
}

/* Temporary struct for CopyInterps. */
struct collect {
    darray(struct xkb_sym_interpret) sym_interprets;
};

static void
CopyInterps(CompatInfo *info, bool needSymbol, enum xkb_match_operation pred,
            struct collect *collect)
{
    SymInterpInfo *si;

    darray_foreach(si, info->interps)
        if (si->interp.match == pred &&
            (si->interp.sym != XKB_KEY_NoSymbol) == needSymbol)
            darray_append(collect->sym_interprets, si->interp);
}

static void
CopyLedMapDefsToKeymap(struct xkb_keymap *keymap, CompatInfo *info)
{
    for (xkb_led_index_t idx = 0; idx < info->num_leds; idx++) {
        LedInfo *ledi = &info->leds[idx];
        xkb_led_index_t i;
        struct xkb_led *led;

        /*
         * Find the LED with the given name, if it was already declared
         * in keycodes.
         */
        xkb_leds_enumerate(i, led, keymap)
            if (led->name == ledi->led.name)
                break;

        /* Not previously declared; create it with next free index. */
        if (i >= keymap->num_leds) {
            log_dbg(keymap->ctx, XKB_LOG_MESSAGE_NO_ID,
                    "Indicator name \"%s\" was not declared in the keycodes section; "
                    "Adding new indicator\n",
                    xkb_atom_text(keymap->ctx, ledi->led.name));

            xkb_leds_enumerate(i, led, keymap)
                if (led->name == XKB_ATOM_NONE)
                    break;

            if (i >= keymap->num_leds) {
                /* Not place to put it; ignore. */
                if (i >= XKB_MAX_LEDS) {
                    log_err(keymap->ctx, XKB_LOG_MESSAGE_NO_ID,
                            "Too many indicators (maximum is %d); "
                            "Indicator name \"%s\" ignored\n",
                            XKB_MAX_LEDS,
                            xkb_atom_text(keymap->ctx, ledi->led.name));
                    continue;
                }

                /* Add a new LED. */
                led = &keymap->leds[keymap->num_leds++];
            }
        }

        *led = ledi->led;
        if (led->groups != 0 && led->which_groups == 0)
            led->which_groups = XKB_STATE_LAYOUT_EFFECTIVE;
        if (led->mods.mods != 0 && led->which_mods == 0)
            led->which_mods = XKB_STATE_MODS_EFFECTIVE;
    }
}

static bool
CopyCompatToKeymap(struct xkb_keymap *keymap, CompatInfo *info)
{
    keymap->compat_section_name = strdup_safe(info->name);
    XkbEscapeMapName(keymap->compat_section_name);

    keymap->mods = info->mods;

    if (!darray_empty(info->interps)) {
        struct collect collect;
        darray_init(collect.sym_interprets);

        /* Most specific to least specific. */
        CopyInterps(info, true, MATCH_EXACTLY, &collect);
        CopyInterps(info, true, MATCH_ALL, &collect);
        CopyInterps(info, true, MATCH_NONE, &collect);
        CopyInterps(info, true, MATCH_ANY, &collect);
        CopyInterps(info, true, MATCH_ANY_OR_NONE, &collect);
        CopyInterps(info, false, MATCH_EXACTLY, &collect);
        CopyInterps(info, false, MATCH_ALL, &collect);
        CopyInterps(info, false, MATCH_NONE, &collect);
        CopyInterps(info, false, MATCH_ANY, &collect);
        CopyInterps(info, false, MATCH_ANY_OR_NONE, &collect);

        darray_steal(collect.sym_interprets,
                     &keymap->sym_interprets, &keymap->num_sym_interprets);
    }

    CopyLedMapDefsToKeymap(keymap, info);

    return true;
}

bool
CompileCompatMap(XkbFile *file, struct xkb_keymap *keymap,
                 enum merge_mode merge)
{
    CompatInfo info;
    ActionsInfo *actions;

    actions = NewActionsInfo();
    if (!actions)
        return false;

    InitCompatInfo(&info, keymap->ctx, actions, &keymap->mods);
    info.default_interp.merge = merge;
    info.default_led.merge = merge;

    HandleCompatMapFile(&info, file, merge);
    if (info.errorCount != 0)
        goto err_info;

    if (!CopyCompatToKeymap(keymap, &info))
        goto err_info;

    ClearCompatInfo(&info);
    FreeActionsInfo(actions);
    return true;

err_info:
    ClearCompatInfo(&info);
    FreeActionsInfo(actions);
    return false;
}
