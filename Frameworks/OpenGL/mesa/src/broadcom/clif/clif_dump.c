/*
 * Copyright © 2016 Broadcom
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
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "drm-uapi/v3d_drm.h"
#include "clif_dump.h"
#include "clif_private.h"
#include "util/list.h"
#include "util/ralloc.h"

#include "broadcom/cle/v3d_decoder.h"

struct reloc_worklist_entry *
clif_dump_add_address_to_worklist(struct clif_dump *clif,
                                  enum reloc_worklist_type type,
                                  uint32_t addr)
{
        struct reloc_worklist_entry *entry =
                rzalloc(clif, struct reloc_worklist_entry);
        if (!entry)
                return NULL;

        entry->type = type;
        entry->addr = addr;

        list_addtail(&entry->link, &clif->worklist);

        return entry;
}

struct clif_dump *
clif_dump_init(const struct v3d_device_info *devinfo,
               FILE *out, bool pretty, bool nobin)
{
        struct clif_dump *clif = rzalloc(NULL, struct clif_dump);

        clif->devinfo = devinfo;
        clif->out = out;
        clif->spec = v3d_spec_load(devinfo);
        clif->pretty = pretty;
        clif->nobin = nobin;

        list_inithead(&clif->worklist);

        return clif;
}

void
clif_dump_destroy(struct clif_dump *clif)
{
        ralloc_free(clif);
}

struct clif_bo *
clif_lookup_bo(struct clif_dump *clif, uint32_t addr)
{
        for (int i = 0; i < clif->bo_count; i++) {
                struct clif_bo *bo = &clif->bo[i];

                if (addr >= bo->offset &&
                    addr < bo->offset + bo->size) {
                        return bo;
                }
        }

        return NULL;
}

static bool
clif_lookup_vaddr(struct clif_dump *clif, uint32_t addr, void **vaddr)
{
        struct clif_bo *bo = clif_lookup_bo(clif, addr);
        if (!bo)
                return false;

        *vaddr = bo->vaddr + addr - bo->offset;
        return true;
}

#define out_uint(_clif, field) out(_clif, "    /* %s = */ %u\n",        \
                            #field,  values-> field);

static bool
clif_dump_packet(struct clif_dump *clif, uint32_t offset, const uint8_t *cl,
                 uint32_t *size, bool reloc_mode)
{

        switch (clif->devinfo->ver) {
        case 42:
                return v3d42_clif_dump_packet(clif, offset, cl, size, reloc_mode);
        case 71:
                return v3d71_clif_dump_packet(clif, offset, cl, size, reloc_mode);
        default:
                break;
        };
        unreachable("Unknown HW version");
}

static uint32_t
clif_dump_cl(struct clif_dump *clif, uint32_t start, uint32_t end,
             bool reloc_mode)
{
        struct clif_bo *bo = clif_lookup_bo(clif, start);
        if (!bo) {
                out(clif, "Failed to look up address 0x%08x\n",
                    start);
                return 0;
        }

        void *start_vaddr = bo->vaddr + start - bo->offset;

        /* The end address is optional (for example, a BRANCH instruction
         * won't set an end), but is used for BCL/RCL termination.
         */
        void *end_vaddr = NULL;
        if (end && !clif_lookup_vaddr(clif, end, &end_vaddr)) {
                out(clif, "Failed to look up address 0x%08x\n",
                    end);
                return 0;
        }

        if (!reloc_mode)
                out(clif, "@format ctrllist  /* [%s+0x%08x] */\n",
                    bo->name, start - bo->offset);

        uint32_t size;
        uint8_t *cl = start_vaddr;
        while (clif_dump_packet(clif, start, cl, &size, reloc_mode)) {
                cl += size;
                start += size;

                if (cl == end_vaddr)
                        break;
        }

        return (void *)cl - bo->vaddr;
}

/* Walks the worklist, parsing the relocs for any memory regions that might
 * themselves have additional relocations.
 */
static uint32_t
clif_dump_gl_shader_state_record(struct clif_dump *clif,
                                 struct reloc_worklist_entry *reloc,
                                 void *vaddr,
                                 bool including_gs)
{
        struct v3d_group *state = v3d_spec_find_struct(clif->spec,
                                                       "GL Shader State Record");
        struct v3d_group *attr = v3d_spec_find_struct(clif->spec,
                                                      "GL Shader State Attribute Record");
        assert(state);
        assert(attr);
        uint32_t offset = 0;

        if (including_gs) {
                struct v3d_group *gs_state = v3d_spec_find_struct(clif->spec,
                                                                  "Geometry Shader State Record");
                assert(gs_state);
                out(clif, "@format shadrec_gl_geom\n");
                v3d_print_group(clif, gs_state, 0, vaddr + offset);
                offset += v3d_group_get_length(gs_state);
                /* Extra pad when geometry/tessellation shader is present */
                offset += 20;
        }
        out(clif, "@format shadrec_gl_main\n");
        v3d_print_group(clif, state, 0, vaddr + offset);
        offset += v3d_group_get_length(state);

        for (int i = 0; i < reloc->shader_state.num_attrs; i++) {
                out(clif, "@format shadrec_gl_attr /* %d */\n", i);
                v3d_print_group(clif, attr, 0, vaddr + offset);
                offset += v3d_group_get_length(attr);
        }

        return offset;
}

static void
clif_process_worklist(struct clif_dump *clif)
{
        list_for_each_entry_safe(struct reloc_worklist_entry, reloc,
                                 &clif->worklist, link) {
                void *vaddr;
                if (!clif_lookup_vaddr(clif, reloc->addr, &vaddr)) {
                        out(clif, "Failed to look up address 0x%08x\n",
                            reloc->addr);
                        continue;
                }

                switch (reloc->type) {
                case reloc_cl:
                        clif_dump_cl(clif, reloc->addr, reloc->cl.end, true);
                        break;

                case reloc_gl_shader_state:
                case reloc_gl_including_gs_shader_state:
                        break;
                case reloc_generic_tile_list:
                        clif_dump_cl(clif, reloc->addr,
                                     reloc->generic_tile_list.end, true);
                        break;
                }
        }
}

static int
worklist_entry_compare(const void *a, const void *b)
{
        return ((*(struct reloc_worklist_entry **)a)->addr -
                (*(struct reloc_worklist_entry **)b)->addr);
}

static bool
clif_dump_if_blank(struct clif_dump *clif, struct clif_bo *bo,
                   uint32_t start, uint32_t end)
{
        for (int i = start; i < end; i++) {
                if (((uint8_t *)bo->vaddr)[i] != 0)
                        return false;
        }

        out(clif, "\n");
        out(clif, "@format blank %d /* [%s+0x%08x..0x%08x] */\n", end - start,
            bo->name, start, end - 1);
        return true;
}

/* Dumps the binary data in the BO from start to end (relative to the start of
 * the BO).
 */
static void
clif_dump_binary(struct clif_dump *clif, struct clif_bo *bo,
                 uint32_t start, uint32_t end)
{
        if (clif->pretty && clif->nobin)
                return;

        if (start == end)
                return;

        if (clif_dump_if_blank(clif, bo, start, end))
                return;

        out(clif, "@format binary /* [%s+0x%08x] */\n",
            bo->name, start);

        uint32_t offset = start;
        int dumped_in_line = 0;
        while (offset < end) {
                if (clif_dump_if_blank(clif, bo, offset, end))
                        return;

                if (end - offset >= 4) {
                        out(clif, "0x%08x ", *(uint32_t *)(bo->vaddr + offset));
                        offset += 4;
                } else {
                        out(clif, "0x%02x ", *(uint8_t *)(bo->vaddr + offset));
                        offset++;
                }

                if (++dumped_in_line == 8) {
                        out(clif, "\n");
                        dumped_in_line = 0;
                }
        }
        if (dumped_in_line)
                out(clif, "\n");
}

/* Walks the list of relocations, dumping each buffer's contents (using our
 * codegenned dump routines for pretty printing, and most importantly proper
 * address references so that the CLIF parser can relocate buffers).
 */
static void
clif_dump_buffers(struct clif_dump *clif)
{
        int num_relocs = 0;
        list_for_each_entry(struct reloc_worklist_entry, reloc,
                            &clif->worklist, link) {
                num_relocs++;
        }
        struct reloc_worklist_entry **relocs =
                ralloc_array(clif, struct reloc_worklist_entry *, num_relocs);
        int i = 0;
        list_for_each_entry(struct reloc_worklist_entry, reloc,
                            &clif->worklist, link) {
                relocs[i++] = reloc;
        }
        qsort(relocs, num_relocs, sizeof(*relocs), worklist_entry_compare);

        struct clif_bo *bo = NULL;
        uint32_t offset = 0;

        for (i = 0; i < num_relocs; i++) {
                struct reloc_worklist_entry *reloc = relocs[i];
                struct clif_bo *new_bo = clif_lookup_bo(clif, reloc->addr);

                if (!new_bo) {
                        out(clif, "Failed to look up address 0x%08x\n",
                            reloc->addr);
                        continue;
                }

                if (new_bo != bo) {
                        if (bo) {
                                /* Finish out the last of the last BO. */
                                clif_dump_binary(clif, bo,
                                                 offset,
                                                 bo->size);
                        }

                        out(clif, "\n");
                        out(clif, "@buffer %s\n", new_bo->name);
                        bo = new_bo;
                        offset = 0;
                        bo->dumped = true;
                }

                int reloc_offset = reloc->addr - bo->offset;
                if (offset != reloc_offset)
                        clif_dump_binary(clif, bo, offset, reloc_offset);
                offset = reloc_offset;

                switch (reloc->type) {
                case reloc_cl:
                        offset = clif_dump_cl(clif, reloc->addr, reloc->cl.end,
                                              false);
                        out(clif, "\n");
                        break;

                case reloc_gl_shader_state:
                case reloc_gl_including_gs_shader_state:
                        offset += clif_dump_gl_shader_state_record(clif,
                                                                   reloc,
                                                                   bo->vaddr +
                                                                   offset,
                                                                   reloc->type == reloc_gl_including_gs_shader_state);
                        break;
                case reloc_generic_tile_list:
                        offset = clif_dump_cl(clif, reloc->addr,
                                              reloc->generic_tile_list.end,
                                              false);
                        break;
                }
                out(clif, "\n");
        }

        if (bo) {
                clif_dump_binary(clif, bo, offset, bo->size);
        }

        /* For any BOs that didn't have relocations, just dump them raw. */
        for (int i = 0; i < clif->bo_count; i++) {
                bo = &clif->bo[i];
                if (bo->dumped)
                        continue;
                out(clif, "@buffer %s\n", bo->name);
                clif_dump_binary(clif, bo, 0, bo->size);
                out(clif, "\n");
        }
}

void
clif_dump_add_cl(struct clif_dump *clif, uint32_t start, uint32_t end)
{
        struct reloc_worklist_entry *entry =
                clif_dump_add_address_to_worklist(clif, reloc_cl, start);

        entry->cl.end = end;
}

static int
clif_bo_offset_compare(const void *a, const void *b)
{
        return ((struct clif_bo *)a)->offset - ((struct clif_bo *)b)->offset;
}

void
clif_dump(struct clif_dump *clif, const struct drm_v3d_submit_cl *submit)
{
        clif_dump_add_cl(clif, submit->bcl_start, submit->bcl_end);
        clif_dump_add_cl(clif, submit->rcl_start, submit->rcl_end);

        qsort(clif->bo, clif->bo_count, sizeof(clif->bo[0]),
              clif_bo_offset_compare);

        /* A buffer needs to be defined before we can emit a CLIF address
         * referencing it, so emit them all now.
         */
        for (int i = 0; i < clif->bo_count; i++) {
                out(clif, "@createbuf_aligned 4096 %s\n", clif->bo[i].name);
        }

        /* Walk the worklist figuring out the locations of structs based on
         * the CL contents.
         */
        clif_process_worklist(clif);

        /* Dump the contents of the buffers using the relocations we found to
         * pretty-print structures.
         */
        clif_dump_buffers(clif);

        out(clif, "@add_bin 0\n  ");
        out_address(clif, submit->bcl_start);
        out(clif, "\n  ");
        out_address(clif, submit->bcl_end);
        out(clif, "\n  ");
        out_address(clif, submit->qma);
        out(clif, "\n  %d\n  ", submit->qms);
        out_address(clif, submit->qts);
        out(clif, "\n");
        out(clif, "@wait_bin_all_cores\n");

        out(clif, "@add_render 0\n  ");
        out_address(clif, submit->rcl_start);
        out(clif, "\n  ");
        out_address(clif, submit->rcl_end);
        out(clif, "\n  ");
        out_address(clif, submit->qma);
        out(clif, "\n");
        out(clif, "@wait_render_all_cores\n");
}

void
clif_dump_add_bo(struct clif_dump *clif, const char *name,
                 uint32_t offset, uint32_t size, void *vaddr)
{
        if (clif->bo_count >= clif->bo_array_size) {
                clif->bo_array_size = MAX2(4, clif->bo_array_size * 2);
                clif->bo = reralloc(clif, clif->bo, struct clif_bo,
                                    clif->bo_array_size);
        }

        /* CLIF relocs use the buffer name, so make sure they're unique. */
        for (int i = 0; i < clif->bo_count; i++)
                assert(strcmp(clif->bo[i].name, name) != 0);

        clif->bo[clif->bo_count].name = ralloc_strdup(clif, name);
        clif->bo[clif->bo_count].offset = offset;
        clif->bo[clif->bo_count].size = size;
        clif->bo[clif->bo_count].vaddr = vaddr;
        clif->bo[clif->bo_count].dumped = false;
        clif->bo_count++;
}
