/**
 * macho.c
 * Author: Zoe Knox      Created: 2026-03-13
 *
 * Copyright (C) 2026 ravynOS Project. All rights reserved.
 * Portions Copyright (c) 2008, 2012 Apple Inc.  All Rights Reserved.
 *
 * This file contains Original Code and/or Modifications of Original Code as
 * defined in and that are subject to the Apple Public Source License Version 2.0
 * (the 'License').  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS
 * OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES, INCLUDING WITHOUT
 * LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.  Please see the License for the
 * specific language governing rights and limitations under the License.
 */
/*                            !! NOTICE !!
 * The Original Code from Apple has been substantially changed to remove
 * functionality not needed for our specific use case. It has also been modified
 * to compile on Linux. Most of this code originates from macho_util.c.
 */

#include "plktool.h"

boolean_t
macho_swap_64(u_char *file)
{
    boolean_t                  result = FALSE;
    struct mach_header_64     *hdr = (struct mach_header_64 *) file;
    struct load_command       *lc = (struct load_command *) &hdr[1];
    struct segment_command_64 *seg = NULL;
    u_long                     offset = 0;
    u_int                      cmd = 0;
    u_int                      cmdsize = 0;
    u_int                      i = 0;

    if (!hdr || hdr->magic != MH_CIGAM_64) goto finish;

    swap_mach_header_64(hdr, NXHostByteOrder());

    offset = sizeof(*hdr);
    for (i = 0; i < hdr->ncmds; ++i)
    {
        lc = (struct load_command *) (file + offset);

        cmd = bswap_32(lc->cmd);
        cmdsize = bswap_32(lc->cmdsize);
        offset += cmdsize;

        if (cmd == LC_SEGMENT_64)
        {
            seg = (struct segment_command_64 *) lc;
            swap_segment_command_64(seg, NXHostByteOrder());
        }
        else
        {
            swap_load_command(lc, NXHostByteOrder());
        }
    }

    result = TRUE;

finish:
    return result;
}

boolean_t
macho_unswap_64(u_char *file)
{
    boolean_t                  result = FALSE;
    enum NXByteOrder           order = 0;
    struct mach_header_64     *hdr = (struct mach_header_64 *) file;
    struct load_command       *lc = (struct load_command *) &hdr[1];
    struct segment_command_64 *seg = NULL;
    u_long                     offset = 0;
    u_int                      i = 0;

    if (NXHostByteOrder() == NX_LittleEndian)
    {
        order = NX_BigEndian;
    }
    else
    {
        order = NX_LittleEndian;
    }

    if (!hdr || hdr->magic != MH_MAGIC_64) goto finish;

    offset = sizeof(*hdr);
    for (i = 0; i < hdr->ncmds; ++i)
    {
        lc = (struct load_command *) (file + offset);
        offset += lc->cmdsize;

        if (lc->cmd == LC_SEGMENT_64)
        {
            seg = (struct segment_command_64 *) lc;
            swap_segment_command_64(seg, order);
        }
        else
        {
            swap_load_command(lc, order);
        }
    }

    swap_mach_header_64(hdr, order);

    result = TRUE;

finish:
    return result;
}

#define CMDSIZE_MULT_32 (4)
#define CMDSIZE_MULT_64 (8)

macho_seek_result
macho_scan_load_commands(const void       *file_start,
                         const void       *file_end,
                         macho_lc_callback lc_callback,
                         void             *user_data)
{
    macho_seek_result    result = macho_seek_result_not_found;
    struct mach_header  *mach_header = (struct mach_header *) file_start;
    uint8_t              swap = 0;
    uint32_t             cmdsize_mult = CMDSIZE_MULT_32;
    uint32_t             num_cmds;
    uint32_t             sizeofcmds;
    char                *cmds_end;
    uint32_t             cmd_index;
    struct load_command *load_commands;
    struct load_command *seek_lc;

    switch (*(uint32_t *) (file_start))
    {
        case MH_MAGIC_64:
            cmdsize_mult = CMDSIZE_MULT_64;
            break;
        case MH_CIGAM_64:
            cmdsize_mult = CMDSIZE_MULT_64;
            swap = 1;
            break;
        default:
            result = macho_seek_result_error;
            goto finish;
            break;
    }

    if (cmdsize_mult == CMDSIZE_MULT_64)
    {
        load_commands = (struct load_command *) (file_start + sizeof(struct mach_header_64));
    }

    if (file_start >= file_end || (((void *) load_commands) > file_end))
    {
        result = macho_seek_result_error;
        goto finish;
    }

    num_cmds = swap
        ? bswap_32(mach_header->ncmds)
        : mach_header->ncmds;
    sizeofcmds = swap
        ? bswap_32(mach_header->sizeofcmds)
        : mach_header->sizeofcmds;
    cmds_end = (char *) load_commands + sizeofcmds;

    if (cmds_end > (char *) file_end)
    {
        result = macho_seek_result_error;
        goto finish;
    }

    seek_lc = load_commands;

    for (cmd_index = 0; cmd_index < num_cmds; cmd_index++)
    {
        uint32_t cmd_size;
        char    *lc_end;

        cmd_size = swap
            ? bswap_32(seek_lc->cmdsize)
            : seek_lc->cmdsize;
        lc_end = (char *) seek_lc + cmd_size;

        if ((cmd_size % cmdsize_mult != 0) || (lc_end > cmds_end))
        {
            result = macho_seek_result_error;
            goto finish;
        }

        result = lc_callback(seek_lc, file_end, swap, user_data);

        switch (result)
        {
            case macho_seek_result_not_found:
                /* Not found, keep scanning. */
                break;

            case macho_seek_result_stop:
                /* Definitely found that it isn't there. */
                result = macho_seek_result_not_found;
                goto finish;
                break;

            case macho_seek_result_found:
                /* Found it! */
                goto finish;
                break;

            default:
                /* Error, fall through default case. */
                result = macho_seek_result_error;
                goto finish;
                break;
        }

        seek_lc = (struct load_command *) ((char *) seek_lc + cmd_size);
    }

finish:
    return result;
}


struct segment_command_64 *
macho_get_segment_by_name_64(struct mach_header_64 *mach_header,
                             const char            *segname)
{
    struct segment_command_64 *segment = NULL;
    struct load_command       *lc = NULL;
    u_char                    *base = (u_char *) mach_header;
    size_t                     offset = sizeof(*mach_header);
    u_int                      i = 0;

    if (mach_header->magic != MH_MAGIC_64) goto finish;

    for (i = 0; i < mach_header->ncmds; ++i)
    {
        lc = (struct load_command *) (base + offset);

        if (lc->cmd == LC_SEGMENT_64)
        {
            segment = (struct segment_command_64 *) lc;
            if (!strncmp(segment->segname, segname, sizeof(segment->segname)))
            {
                break;
            }
            segment = NULL;
        }

        offset += lc->cmdsize;
    }

finish:
    return segment;
}

struct section_64 *
macho_get_section_by_name_64(struct mach_header_64 *mach_header,
                             const char            *segname,
                             const char            *sectname)
{
    struct segment_command_64 *segment = NULL;
    struct section_64         *section = NULL;
    u_int                      i = 0;

    if (mach_header->magic != MH_MAGIC_64) goto finish;

    segment = macho_get_segment_by_name_64(mach_header, segname);
    if (!segment) goto finish;

    section = (struct section_64 *) (&segment[1]);
    for (i = 0; i < segment->nsects; ++i, ++section)
    {
        if (!strncmp(section->sectname, sectname, sizeof(section->sectname)))
        {
            break;
        }
    }

    if (i == segment->nsects)
    {
        section = NULL;
    }

finish:
    return section;
}
