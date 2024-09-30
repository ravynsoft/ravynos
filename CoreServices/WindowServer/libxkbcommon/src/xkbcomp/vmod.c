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

#include "config.h"

#include "xkbcomp-priv.h"
#include "text.h"
#include "expr.h"
#include "vmod.h"

bool
HandleVModDef(struct xkb_context *ctx, struct xkb_mod_set *mods,
              VModDef *stmt, enum merge_mode merge)
{
    xkb_mod_index_t i;
    struct xkb_mod *mod;
    xkb_mod_mask_t mapping;

    merge = (merge == MERGE_DEFAULT ? stmt->merge : merge);

    if (stmt->value) {
        /*
         * This is a statement such as 'virtualModifiers NumLock = Mod1';
         * it sets the vmod-to-real-mod[s] mapping directly instead of going
         * through modifier_map or some such.
         */
        if (!ExprResolveModMask(ctx, stmt->value, MOD_REAL, mods, &mapping)) {
            log_err(ctx, XKB_LOG_MESSAGE_NO_ID,
                    "Declaration of %s ignored\n",
                    xkb_atom_text(ctx, stmt->name));
            return false;
        }
    }
    else {
        mapping = 0;
    }

    xkb_mods_enumerate(i, mod, mods) {
        if (mod->name == stmt->name) {
            if (mod->type != MOD_VIRT) {
                log_err(ctx, XKB_LOG_MESSAGE_NO_ID,
                        "Can't add a virtual modifier named \"%s\"; "
                        "there is already a non-virtual modifier with this name! Ignored\n",
                        xkb_atom_text(ctx, mod->name));
                return false;
            }

            if (mod->mapping == mapping)
                return true;

            if (mod->mapping != 0) {
                xkb_mod_mask_t use, ignore;

                use = (merge == MERGE_OVERRIDE ? mapping : mod->mapping);
                ignore = (merge == MERGE_OVERRIDE ? mod->mapping : mapping);

                log_warn(ctx, XKB_LOG_MESSAGE_NO_ID,
                         "Virtual modifier %s defined multiple times; "
                         "Using %s, ignoring %s\n",
                         xkb_atom_text(ctx, stmt->name),
                         ModMaskText(ctx, mods, use),
                         ModMaskText(ctx, mods, ignore));

                mapping = use;
            }

            mod->mapping = mapping;
            return true;
        }
    }

    if (mods->num_mods >= XKB_MAX_MODS) {
        log_err(ctx, XKB_LOG_MESSAGE_NO_ID,
                "Too many modifiers defined (maximum %d)\n",
                XKB_MAX_MODS);
        return false;
    }

    mods->mods[mods->num_mods].name = stmt->name;
    mods->mods[mods->num_mods].type = MOD_VIRT;
    mods->mods[mods->num_mods].mapping = mapping;
    mods->num_mods++;
    return true;
}
