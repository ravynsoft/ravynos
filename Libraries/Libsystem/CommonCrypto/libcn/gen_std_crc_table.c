/* 
 * Copyright (c) 2012 Apple, Inc. All Rights Reserved.
 * 
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */



#include "crc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline uint64_t
cm_tab(crcDescriptorPtr crcdesc, uint8_t index)
{
    uint64_t retval;
    const crcModelParms *desc = &crcdesc->def.parms;
    uint64_t topbit = 1LL << ((desc->width * 8) - 1);
    uint64_t mask = descmaskfunc(crcdesc);

    retval = (desc->reflect_reverse) ?reflect_byte(index): index;
    retval <<= (desc->width*8-8);
    for (int i=0; i<8; i++) {
        if (retval & topbit) retval = (retval << 1) ^ desc->poly;
        else retval <<= 1;
    }
    retval = (desc->reflect_reverse) ?reflect(retval, desc->width*8): retval;
    return retval & mask;
}

void
gen_std_crc_table(void *c)
{
    crcInfoPtr crc = c;
    
    size_t width = crc->descriptor->def.parms.width;
    if((crc->table.bytes = malloc(width * 256)) == NULL) return;
    for(int i=0; i<256; i++){
        uint8_t c8 = i&0xFF;
        switch (width) {
            case 1: crc->table.bytes[i] = (uint8_t) cm_tab(crc->descriptor, c8); break;
            case 2: crc->table.b16[i] = (uint16_t) cm_tab(crc->descriptor, c8); break;
            case 4: crc->table.b32[i] = (uint32_t) cm_tab(crc->descriptor, c8); break;
            case 8: crc->table.b64[i] = (uint64_t) cm_tab(crc->descriptor, c8); break;
        }
    }
    
}

static char * cc_strndup (char const *s, size_t n)
{
	if (s == NULL) return NULL;
    size_t len = strnlen (s, n);
    char *dup = malloc (len + 1);
    
    if (dup == NULL) return NULL;
    
    memcpy(dup, s, len);
    dup [len] = '\0';
    return dup;
}

void
dump_crc_table(crcInfoPtr crc)
{
    size_t width = crc->descriptor->def.parms.width;
    char *name = cc_strndup(crc->descriptor->name, 64);
    int per_line = 8;
    
    for(size_t i=0; i<strlen(name); i++) if(name[i] == '-') name[i] = '_';
    
    switch (width) {
        case 1: printf("const uint8_t %s_crc_table[] = {\n", name); per_line = 16; break;
        case 2: printf("const uint16_t %s_crc_table[] = {\n", name); per_line = 8; break;
        case 4: printf("const uint32_t %s_crc_table[] = {\n", name); per_line = 8; break;
        case 8: printf("const uint64_t %s_crc_table[] = {\n", name); per_line = 4; break;
    }
    
    for(int i=0; i<256; i++) {
        switch (width) {
            case 1: printf(" 0x%02x,", crc->table.bytes[i]); break;
            case 2: printf(" 0x%04x,", crc->table.b16[i]); break;
            case 4: printf(" 0x%08x,", crc->table.b32[i]); break;
            case 8: printf(" 0x%016llx,", crc->table.b64[i]); break;
        }
        if(((i+1) % per_line) == 0) printf("\n");
    }
    printf("};\n\n");
    free(name);
}
