/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
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

#include "stuff/breakout.h"
#include "stuff/errors.h"
#include "stuff/ofile.h"
#include "stuff/write64.h"
#include "stuff/port.h" /* cctools-port */

#include <fcntl.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* used by error routines as the name of this program */
char *progname = NULL;

/* command-line flags */
struct flags {
    enum bool	verbose;
    enum bool	dry_run;
    enum bool	start;
    enum bool	no_bss;
    uint64_t	start_addr;
    const char* data_lma_sym;
    const char* data_size_sym;
    uint64_t data_lma_loc;
    uint64_t data_size_loc;
} g_flags;

struct segentry {
    char* segname;
    uint64_t vmaddr;
    uint64_t vmsize;
    uint64_t fileoff;
    uint64_t filesize;
    uint64_t out_fileoff;
    uint64_t out_filesize;
};

struct sectentry {
    struct segentry* seg;
    char* segname;
    char* sectname;
    uint64_t addr;
    uint64_t size;
    uint64_t offset;
    uint32_t align;
    uint32_t flags;
};

struct symbol {
    char* name;
    uint8_t n_type;
    uint8_t n_sect;
    uint64_t n_value;
};

struct segentry* g_segs = NULL;
struct sectentry* g_sects = NULL;
struct symbol* g_syms = NULL;
uint32_t g_nseg = 0;
uint32_t g_nsect = 0;
uint32_t g_nsym = 0;

static void get_segments(const struct ofile* ofile,
                         struct segentry** o_segs, uint32_t* o_nseg,
                         struct sectentry** o_sects, uint32_t* o_nsect);
static void get_symbols(const struct ofile* ofile,
                        struct symbol** o_syms, uint32_t* o_nsym);
static void find_data_symbols(const struct ofile* ofile);
static void process(const struct ofile* ofile, const char* output);
static void usage(void);

/* apple_version is created by the libstuff/Makefile */
extern char apple_version[];
char *apple_version_str = apple_version;

/*
 * The mtor(1) tool makes a raw binary file from a fully linked Mach-O
 * MH_PRELOAD file. These are meant to be in-core images in either a
 * ready-to-run layout, or with an embedded startup that knows how to
 * relocate/re-layout the image at runtime.
 *
 *	mtor [-nv] [-start] [-no_bss] input_Mach-O output_raw
 *
 * Caveat mtor: input files must contain one architecture.
 */

int main(int argc, char **argv)
{
    enum bool read_options = TRUE;
    char* input = NULL;
    char* output = NULL;
    
    progname = *argv++;
    argc--;
    
    memset(&g_flags, 0, sizeof(g_flags));
    
    if (argc == 0)
        usage();
    
    /* parse args */
    while (argc > 0)
    {
        if (read_options && *argv && '-' == **argv)
        {
            if (0 == strcmp("-h", *argv) ||
                0 == strcmp("-help", *argv))
            {
                usage();
            }
            else if (0 == strcmp(*argv, "-no_bss"))
            {
                g_flags.no_bss = TRUE;
            }
            else if (0 == strcmp("-o", *argv) ||
                     0 == strcmp("-output", *argv))
            {
                argv++; argc--;
                if (!*argv) {
                    warning("one output file must be specified");
                    usage();
                }
                if (output) {
                    warning("only one output file must be specified");
                    usage();
                }
                output = strdup(*argv);
            }
            else if (0 == strcmp("-packdata", *argv))
            {
                if (g_flags.data_lma_sym) {
                    warning("-packdata specified more than once");
                    usage();
                }
                
                argv++; argc--;
                if (!*argv) {
                    warning("data lma symbol name must be specified");
                    usage();
                }
                g_flags.data_lma_sym = strdup(*argv);
                
                argv++; argc--;
                if (!*argv) {
                    warning("data size symbol name must be specified");
                    usage();
                }
                g_flags.data_size_sym = strdup(*argv);
            }
            else if (0 == strcmp(*argv, "-start") ||
                     0 == strcmp(*argv, "-image_base") ||
                     0 == strcmp(*argv, "-seg1addr"))
            {
                const char* option = *argv;
                argv++; argc--;
                
                if (!*argv) {
                    warning("one start address must be specified for %s",
                            option);
                    usage();
                }
                
                if (g_flags.start) {
                    warning("only one start address must be specified");
                    usage();
                }
                
                char *endp;
                g_flags.start_addr = (uint64_t)strtoull(*argv, &endp, 16);
                if (*endp != '\0') {
                    fatal("%s not a proper hexadecimal number", *argv);
                }
                
                g_flags.start = TRUE;
            }
            else if (0 == strcmp("-version", *argv))
            {
                printf("%s\n", apple_version_str);
                
                if (argc < 2)
                    exit(EXIT_SUCCESS);
            }
            else if (0 == strcmp("--", *argv))
            {
                read_options = FALSE;
            }
            else {
                for (int j = 1; (*argv)[j]; ++j)
                {
                    if ('n' == (*argv)[j]) {
                        g_flags.verbose = TRUE;
                        g_flags.dry_run = TRUE;
                    }
                    else if ('v' == (*argv)[j]) {
                        g_flags.verbose = TRUE;
                    }
                    else {
                        warning("unknown flag -%c", (*argv)[j]);
                        usage();
                    }
                }
            }
        }
        else {
            if (input) {
                warning("only one input file must be specified");
                usage();
            }
            
            input = strdup(*argv);
        }
        
        argv++; argc--;
    }
    
    /* check for required parameters */
    if (!input) {
        warning("no input file specified");
        usage();
    }
    if (!output) {
        warning("no output file specified");
        usage();
    }
    
    /* breakout the file for processing */
    struct arch *archs;
    uint32_t narchs;
    struct ofile *ofile = breakout(input, &archs, &narchs, FALSE);
    if (errors || !ofile)
        return(EXIT_FAILURE);
    
    /* checkout the file for symbol table replacement processing */
    checkout(archs, narchs);
    
    /* perform other input checking */
    if (OFILE_FAT == ofile->file_type)
        fatal("file: %s is a fat file (%s only operates on Mach-O files, "
              "use lipo(1) on it to get a Mach-O file)", input, progname);
    if (OFILE_Mach_O != archs->type)
        fatal("input file: %s must be a Mach-O file", input);
    if (NULL != archs->object->mh){
        if (MH_PRELOAD != archs->object->mh->filetype) {
            fatal("input file: %s must be an MH_PRELOAD file type",
                  archs->file_name);
        }
    } else {
        if (MH_PRELOAD != archs->object->mh64->filetype) {
            fatal("input file: %s must be an MH_PRELOAD file type",
                  archs->file_name);
        }
    }
    
    get_segments(ofile, &g_segs, &g_nseg, &g_sects, &g_nsect);
    get_symbols(ofile, &g_syms, &g_nsym);
    find_data_symbols(ofile);
    
    /* do it! */
    process(ofile, output);

    /* clean up */
    free_archs(archs, narchs);
    ofile_unmap(ofile);
    free(input);
    free(output);
    
    /* exit */
    if (!errors)
        return(EXIT_SUCCESS);
    else
        return(EXIT_FAILURE);
}

static void get_segments(const struct ofile* ofile,
                         struct segentry** o_segs, uint32_t* o_nseg,
                         struct sectentry** o_sects, uint32_t* o_nsect)
{
    uint32_t nseg = 0;
    uint32_t nsect = 0;

    *o_segs = NULL;
    *o_sects = NULL;
    *o_nseg = 0;
    *o_nsect = 0;

    /* count the segments and sections */
    uint32_t ncmds = ofile->mh ? ofile->mh->ncmds : ofile->mh64->ncmds;
    unsigned char* p = (unsigned char*)(ofile->load_commands);
    for (uint32_t i = 0; i < ncmds; ++i)
    {
        struct load_command* lc = (struct load_command*)p;
        p += lc->cmdsize;
        
        if (LC_SEGMENT == lc->cmd)
        {
            struct segment_command* sg = (struct segment_command*)lc;
            nseg += 1;
            nsect += sg->nsects;
        }
        else if (LC_SEGMENT_64 == lc->cmd)
        {
            struct segment_command_64* sg = (struct segment_command_64*)lc;
            nseg += 1;
            nsect += sg->nsects;
        }
    }
    
    /*
     * build the segment and section lists.
     */
    struct segentry* segs = calloc(nseg, sizeof(*segs));
    struct sectentry* sects = calloc(nsect, sizeof(*sects));
    uint32_t iseg = 0;
    uint32_t isect = 0;
    p = (unsigned char*)(ofile->load_commands);
    for (uint32_t i = 0; i < ncmds; ++i)
    {
        struct load_command* lc = (struct load_command*)p;
        p += lc->cmdsize;

        if (LC_SEGMENT == lc->cmd)
        {
            struct segentry* segentry = &segs[iseg++];
            
            struct segment_command* sg = (struct segment_command*)lc;

            segentry->segname = strdup(sg->segname);
            segentry->vmaddr = sg->vmaddr;
            segentry->vmsize = sg->vmsize;
            segentry->fileoff = sg->fileoff;
            segentry->filesize = sg->filesize;

            unsigned char* q = (unsigned char*)(sg + 1);
            for (uint32_t j = 0; j < sg->nsects; ++j)
            {
                sects = reallocf(sects, sizeof(struct sectentry) * (nsect + 1));
                struct sectentry* sectentry = &sects[isect++];

                struct section* sc = (struct section*)q;
                q += sizeof(struct section);

                if (0 != strcmp(sg->segname, sc->segname)) {
                    warning("input file %s: %s section %s points to different "
                            "section name: %s\n", ofile->file_name, sg->segname,
                            sc->sectname, sc->segname);
                }

                sectentry->seg = segentry;
                sectentry->sectname = strdup(sc->sectname);
                sectentry->segname = strdup(sg->segname);
                sectentry->addr = sc->addr;
                sectentry->size = sc->size;
                sectentry->offset = sc->offset;
                sectentry->align = sc->align;
                sectentry->flags = sc->flags;
            }
        }
        else if (LC_SEGMENT_64 == lc->cmd)
        {
            struct segentry* segentry = &segs[iseg++];

            struct segment_command_64* sg = (struct segment_command_64*)lc;

            segentry->segname = strdup(sg->segname);
            segentry->vmaddr = sg->vmaddr;
            segentry->vmsize = sg->vmsize;
            segentry->fileoff = sg->fileoff;
            segentry->filesize = sg->filesize;

            unsigned char* q = (unsigned char*)(sg + 1);
            for (uint32_t j = 0; j < sg->nsects; ++j)
            {
                sects = reallocf(sects, sizeof(struct sectentry) * (nsect + 1));
                struct sectentry* sectentry = &sects[isect++];

                struct section_64* sc = (struct section_64*)q;
                q += sizeof(struct section_64);

                if (0 != strcmp(sg->segname, sc->segname)) {
                    warning("input file %s: %s section %s points to different "
                            "section name: %s\n", ofile->file_name, sg->segname,
                            sc->sectname, sc->segname);
                }

                sectentry->seg = segentry;
                sectentry->sectname = strdup(sc->sectname);
                sectentry->segname = strdup(sg->segname);
                sectentry->addr = sc->addr;
                sectentry->size = sc->size;
                sectentry->offset = sc->offset;
                sectentry->align = sc->align;
                sectentry->flags = sc->flags;
            }
        }
    }
    
    *o_segs = segs;
    *o_nseg = nseg;
    *o_sects = sects;
    *o_nsect = nsect;
}

static void get_symbols(const struct ofile* ofile,
                        struct symbol** o_syms, uint32_t* o_nsym)
{
    struct symtab_command* st = NULL;

    *o_syms = NULL;
    *o_nsym = 0;

    /*
     * find the symbol table, if any.
     */
    uint32_t ncmds = ofile->mh ? ofile->mh->ncmds : ofile->mh64->ncmds;
    unsigned char* p = (unsigned char*)(ofile->load_commands);
    for (uint32_t i = 0; i < ncmds; ++i)
    {
        struct load_command* lc = (struct load_command*)p;
        p += lc->cmdsize;

        if (LC_SYMTAB == lc->cmd)
        {
            if (st) {
                fatal("input file: %s contains multiple symbol tables",
                      ofile->file_name);
            }
            st = (struct symtab_command*)lc;
        }
    }

    if (!st)
        return;

    struct symbol* syms = calloc(st->nsyms, sizeof(struct symbol));
    
    /*
     * record the symbols
     */
    if (ofile->mh) {
        struct nlist* nl = (struct nlist*)(ofile->file_addr + st->symoff);
        char* strings = (char*)ofile->file_addr + st->stroff;
        for (uint32_t i = 0; i < st->nsyms; ++i) {
            if (nl[i].n_un.n_strx)
                syms[i].name = strdup((char*)(strings + nl[i].n_un.n_strx));
            else
                syms[i].name = strdup("");
            syms[i].n_type = nl[i].n_type;
            syms[i].n_sect = nl[i].n_sect;
            syms[i].n_value = nl[i].n_value;
        }
    }
    else if (ofile->mh64) {
        struct nlist_64* nl = (struct nlist_64*)(ofile->file_addr + st->symoff);
        char* strings = (char*)ofile->file_addr + st->stroff;
        for (uint32_t i = 0; i < st->nsyms; ++i) {
            if (nl[i].n_un.n_strx)
                syms[i].name = strdup((char*)(strings + nl[i].n_un.n_strx));
            else
                syms[i].name = strdup("");
            syms[i].n_type = nl[i].n_type;
            syms[i].n_sect = nl[i].n_sect;
            syms[i].n_value = nl[i].n_value;
        }
    }
    
    *o_syms = syms;
    *o_nsym = st->nsyms;
}

static int find_symbol(const struct ofile* ofile, const struct symbol* sym,
                       const char* name, uint64_t* o_value)
{
    if (0 == strcmp(sym->name, name) &&
        0 == (sym->n_type & N_STAB))
    {
//        if (sym->n_type & N_STAB) {
//            fatal("input file %s: symbol %s is a STAB symbol",
//                  ofile->file_name, name);
//        }
        if (*o_value != 0) {
            fatal("input file %s: contains multiple %s Mach-O symbols",
                  ofile->file_name, name);
        }
        if ((sym->n_type & N_SECT) != N_SECT) {
            fatal("input file %s: symbol %s is not directly defined",
                  ofile->file_name, name);
        }
        uint32_t sectord = sym->n_sect;
        if (sectord == 0) {
            fatal("input file %s: symbol %s is not in any section",
                  ofile->file_name, name);
        }
        if (sectord > g_nsect) {
            fatal("input file %s: symbol %s has a bad section ordinal "
                  "(%d > %d)", ofile->file_name, name, sectord, g_nsect);
        }
        struct sectentry* section = &g_sects[sectord-1];
        if (strcmp(SEG_TEXT, section->segname)) {
            fatal("input file %s: symbol %s is not in __TEXT segment: (%s, %s)",
                  ofile->file_name, name, section->segname, section->sectname);
        }
        if (0 == sym->n_value) {
            fatal("input file %s: symbol %s has no storage (n_value)",
                    ofile->file_name, name);
        }
        uint64_t offset = section->seg->fileoff + sym->n_value;
        uint32_t value = *(uint32_t*)(ofile->file_addr + offset);
        if (value) {
            warning("input file %s symbol %s has non-zero value: 0x%08x",
                    ofile->file_name, name, value);
        }

        *o_value = sym->n_value;
        return 0;
    }
    return -1;
}

static void find_data_symbols(const struct ofile* ofile)
{
    if (!g_flags.data_lma_sym || !g_flags.data_size_sym)
        return;

    for (uint32_t i = 0; i < g_nsym; ++i) {
        find_symbol(ofile, &g_syms[i], g_flags.data_lma_sym,
                    &g_flags.data_lma_loc);
        find_symbol(ofile, &g_syms[i], g_flags.data_size_sym,
                    &g_flags.data_size_loc);
        if (g_flags.data_lma_loc && g_flags.data_size_loc) {
            break;
        }
    }

    if (!g_flags.data_lma_loc || !g_flags.data_size_loc) {
        if (!g_flags.data_lma_loc)
            error("input file %s: symbol not found: %s",
                  ofile->file_name, g_flags.data_lma_sym);
        if (!g_flags.data_size_loc)
            error("input file %s: symbol not found: %s",
                  ofile->file_name, g_flags.data_size_sym);
        exit(EXIT_FAILURE);
    }
}

/*
 * process() computes both the source regions of the input file and the
 * destination regions of the output file based on the input file and program
 * arguments.
 *
 * The output file represents the Mach-O file's layout in memory. As such, the
 * __TEXT and __DATA segments will move from fileoff within the input file to
 * vmaddr within the output file. The -start option can be used to override
 * the starting location of segment data within the output file. If necessary,
 * the output file will be zero-padded to the start of the __TEXT segment.
 *
 * By default the entire contents of __TEXT and DATA will be copied. The
 * -no_bss option can be used to omit the bss and other S_ZEROFILL sections
 * from the __DATA segment. __DATA segments truncated in this way will not be
 * aligned to page boundaries.
 *
 * Caveat mtor: The mtor tool cannot relink the input file. Sections and
 * segments cannot be re-padded to new page alignments or moved in any way
 * other than to slide them all as a group. Make sure the input file is as
 * correct as possible by specifying -seg1addr and -segalign to ld(1).
 */

void process(const struct ofile* ofile, const char* output)
{
    /*
     * Locate the text and data segments. Assume / require there to be one
     * text segment, and no more than one data segment.
     */
    struct segentry* text = NULL;
    struct segentry* data = NULL;
    for (uint32_t i = 0; i < g_nseg; ++i)
    {
        struct segentry* seg = &g_segs[i];
        if (0 == strcmp(SEG_TEXT, seg->segname)) {
            if (text) {
                fatal("input file %s contains multiple __TEXT segments",
                      ofile->file_name);
            }
            text = seg;
        }
        else if (0 == strcmp(SEG_DATA, seg->segname)) {
            if (data) {
                fatal("input file %s contains multiple __DATA segments",
                      ofile->file_name);
            }
            data = seg;
        }
    }

    if (!text) {
        error("input file: %s does not contain __TEXT", ofile->file_name);
        exit(EXIT_FAILURE);
    }

    /*
     * compute the vmaddr adjustment necessary to start the
     * segments at the requested start address.
     */
    int64_t startadj = 0;
    if (g_flags.start) {
        startadj = g_flags.start_addr - text->vmaddr;
    }

    /*
     * verify the sections will still be aligned and compute the remaining
     * data size after omitting / truncating any bss sections (if requested).
     */
    uint64_t datasize = 0;
    uint64_t bsssize = 0;
    for (uint32_t i = 0; i < g_nsect; ++i) {
        struct sectentry* sect = &g_sects[i];
        if ((sect->addr + startadj) % (1 << sect->align)) {
            error("input file: %s section %.16s,%.16s addr "
                  "0x%08llx cannot be aligned to 2^%d (%d)",
                  ofile->file_name, sect->segname, sect->sectname,
                  sect->addr + startadj, sect->align,
                  1 << sect->align);
        }
        
        if (sect->seg == data) {
            if (((sect->flags & SECTION_TYPE) == S_ZEROFILL) ||
            0 == strcmp(sect->sectname, "__common") ||
            0 == strcmp(sect->sectname, "__bss")) {
                if (g_flags.no_bss)
                    break;
                else
                    bsssize += sect->size;
            }
            
            datasize += sect->size;
        }
    }

    /*
     * set compute the destination fileoff and filesizes
     *
     * if we're writing out bss we're going to lie a little bit here and
     * reserve space for the bss data even though the Mach-O does not really
     * include this data. This is kind of a hack ...
     */
    text->out_fileoff  = text->vmaddr + startadj;
    text->out_filesize = text->vmsize;
    if (data) {
        if (g_flags.data_lma_sym) { /* pack data */
            data->out_fileoff = text->out_fileoff + text->out_filesize;
        }
        else {
            data->out_fileoff  = data->vmaddr + startadj;
        }
        data->out_filesize = datasize;
    }
    
    /*
     * determine the size of the final file by measuring the extent of the
     * __TEXT and __DATA segments.
     */
    size_t output_size = 0;
    if (text->vmaddr + text->vmsize > output_size)
        output_size = text->out_fileoff + text->out_filesize;
    if (data && (data->vmaddr + data->vmsize > output_size))
        output_size = data->out_fileoff + data->out_filesize;

    /* allocate an output buffer */
    unsigned char* buf = calloc(1, output_size);
    if (!buf)
        fatal("Can't allocate buffer for output file (size = %lu)",
              output_size);
    
    /*
     * copy the segments into the buffer. __DATA may be omitted / truncated
     * if no_bss
     */
    for (uint32_t i = 0; i < g_nseg; ++i) {
        struct segentry* seg = &g_segs[i];

        if (seg != text && seg != data)
            break;

        if (g_flags.verbose) {
            printf("writing %.16s at VMA: 0x%08llx-0x%08llx LMA: "
                   "0x%08llx-0x%08llx\n", seg->segname,
                   seg->vmaddr, seg->vmaddr + seg->vmsize,
                   seg->out_fileoff, seg->out_fileoff + seg->out_filesize);
        }

        if (!g_flags.dry_run) {
            /*
             * The Mach-O file does not actually include bss data on disk.
             * If we are including bss in the raw data we will need to avoid
             * trying to copy bss off disk; otherwise we'll end up pulling in
             * fragments of linkedit or we'll spill over our buffer.
             *
             * Note that if we are not copying bss, this adjustment has already
             * been baked into the out_filesize.
             */
            uint64_t size = seg->out_filesize;
            if (seg == data) {
                size -= bsssize;
            }

            /*
             * When copying __DATA, the segment filesize will be non-zero in
             * the case where there exist non-bss / zerofill sections. But if
             * all the __DATA sections are bss / zerofill, the filesize will
             * be zero. So, only copy __DATA from the input file if it has a
             * non-zero filesize.
             */
            if (size) {
                memcpy(buf + seg->out_fileoff, ofile->file_addr + seg->fileoff,
                       size);
            }
        }
    }
    
    /* poke the packdata values into the text segment */
    if (g_flags.data_lma_sym) { /* pack data */
        uint32_t* addr;
        addr = (uint32_t*)(buf + text->out_fileoff + g_flags.data_lma_loc);
        *addr = (uint32_t)(data ? data->out_fileoff : 0);
        addr = (uint32_t*)(buf + text->out_fileoff + g_flags.data_size_loc);
        *addr = (uint32_t)(data ? data->out_filesize : 0);
    }

    /* write the raw output file safely */
    if (!g_flags.dry_run) {
        const char* suffix = ".XXXXXX";
        size_t tempsize = strlen(output) + strlen(suffix) + 1;
        char* tempname = calloc(1, tempsize);
        if (!tempname)
            fatal("Can't allocate buffer for output filename (size = %lu)",
                  tempsize);
        
        if (snprintf(tempname, tempsize, "%s%s", output, suffix) != tempsize -1)
            fatal("Can't create temporary name for output file %s", output);
        
        int fd = mkstemp(tempname);
        if (-1 == fd) {
            system_fatal("Can't create temporary file: %s", tempname);
        }
        
        if (write64(fd, buf, output_size) != output_size) {
            system_fatal("Can't write temporary file: %s", tempname);
        }
        
        if (fchmod(fd, 0644) == -1) {
            system_fatal("Can't chmod temporary file: %s", tempname);
        }
        
        if (close(fd) == -1) {
            system_fatal("Can't close temporary file: %s", tempname);
        }
        
        if (rename(tempname, output) == -1) {
            system_fatal("Can't write output file: %s", output);
        }
    }
    
    free(buf);
}

static void usage(void)
{
    const char* basename = strrchr(progname, '/');
    if (basename)
        basename++;
    else
        basename = progname;
    
    fprintf(stderr,
            "usage: %s [-nv] [-no_bss] [-start <start_address>] "
            "[-packdata <lma symbol name> <size symbol name>] "
            "-output <out_file> <file>\n", basename);
    fprintf(stderr,
            "       %s -help [-version]\n", basename);
    fprintf(stderr,
            "       %s -version\n", basename);
    exit(EXIT_FAILURE);
}
