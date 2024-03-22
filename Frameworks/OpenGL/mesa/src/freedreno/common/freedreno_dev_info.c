/*
 * Copyright © 2020 Valve Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */

#include "freedreno_dev_info.h"
#include "util/macros.h"

/**
 * Table entry for a single GPU version
 */
struct fd_dev_rec {
   struct fd_dev_id id;
   const char *name;
   const struct fd_dev_info *info;
};

#include "freedreno_devices.h"

/**
 * Compare device 'id' against reference id ('ref') from gpu table.
 */
static bool
dev_id_compare(const struct fd_dev_id *ref, const struct fd_dev_id *id)
{
   if (ref->gpu_id && id->gpu_id) {
      return ref->gpu_id == id->gpu_id;
   } else {
      if (!id->chip_id)
         return false;

      /* Match on either:
       * (a) exact match:
       */
      if (ref->chip_id == id->chip_id)
         return true;
      /* (b) device table entry has 0xff wildcard patch_id and core/
       *     major/minor match:
       */
      if (((ref->chip_id & 0xff) == 0xff) &&
            ((ref->chip_id & UINT64_C(0xffffff00)) ==
             (id->chip_id & UINT64_C(0xffffff00))))
         return true;
#define WILDCARD_FUSE_ID UINT64_C(0x0000ffff00000000)
      /* If the reference id has wildcard fuse-id value (ie. bits 47..32
       * are all ones, then try matching ignoring the device fuse-id:
       */
      if ((ref->chip_id & WILDCARD_FUSE_ID) == WILDCARD_FUSE_ID) {
         uint64_t chip_id = id->chip_id | WILDCARD_FUSE_ID;
         /* (c) exact match (ignoring the fuse-id from kernel):
          */
         if (ref->chip_id == chip_id)
            return true;
         /* (d) device table entry has 0xff wildcard patch_id and core/
          *     major/minor match (ignoring fuse-id from kernel):
          */
         if (((ref->chip_id & 0xff) == 0xff) &&
               ((ref->chip_id & UINT64_C(0xffffff00)) ==
                (chip_id & UINT64_C(0xffffff00))))
            return true;
      }
      return false;
   }
}

const struct fd_dev_info *
fd_dev_info_raw(const struct fd_dev_id *id)
{
   for (int i = 0; i < ARRAY_SIZE(fd_dev_recs); i++) {
      if (dev_id_compare(&fd_dev_recs[i].id, id)) {
         return fd_dev_recs[i].info;
      }
   }
   return NULL;
}

const struct fd_dev_info
fd_dev_info(const struct fd_dev_id *id)
{
   struct fd_dev_info modified = {};
   const struct fd_dev_info *orig = fd_dev_info_raw(id);
   if (orig) {
      modified = *orig;
      fd_dev_info_apply_dbg_options(&modified);
   }

   return modified;
}

const char *
fd_dev_name(const struct fd_dev_id *id)
{
   for (int i = 0; i < ARRAY_SIZE(fd_dev_recs); i++) {
      if (dev_id_compare(&fd_dev_recs[i].id, id)) {
         return fd_dev_recs[i].name;
      }
   }
   return NULL;
}
