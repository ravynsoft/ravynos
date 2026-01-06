/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
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
/*	$OpenBSD: gprof.c,v 1.3 1996/10/02 02:59:49 tholo Exp $	*/
/*	$NetBSD: gprof.c,v 1.8 1995/04/19 07:15:59 cgd Exp $	*/

/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <libc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "stuff/errors.h"
#include "gprof.h"

#ifdef vax
#include "vax.h"
#endif

/*
 * Progname for error messages.
 */
char *progname = NULL;

/*
 * Ticks per second.
 */
uint32_t hz = 0;

/*
 * Filename of the a.out file.
 */
char *a_outname = NULL;

/*
 * Filename of the gmon.out file.
 */
char *gmonname = NULL;

__private_extern__
nltype	*nl = NULL;	/* the whole namelist */
nltype	*npe = NULL;	/* the virtual end of the namelist */
uint32_t nname = 0;	/* the number of function names */

/*
 * The list of file names and the ranges their pc's cover used for building
 * order files with the -S option.
 */
struct file *files = NULL;
uint32_t n_files = 0;

/* 
 * namelist entries for cycle headers.
 * the number of discovered cycles.
 */
nltype	*cyclenl = NULL;	/* cycle header namelist */
int	ncycle = 0;		/* number of cycles discovered */

/*
 * The information for the pc sample sets from the gmon.out file.
 */
struct sample_set *sample_sets = NULL;
uint32_t nsample_sets = 0;

#ifdef __OPENSTEP__
/*
 * The rld loaded state from the gmon.out file.
 */
struct rld_loaded_state *grld_loaded_state = NULL;
uint32_t grld_nloaded_states = 0;
#endif

/*
 * The dyld images from the gmon.out file.
 */
uint32_t image_count = 0;
struct dyld_image *dyld_images = NULL;

unsigned char *textspace = NULL;/* text space of a.out in core */
double	totime = 0.0;		/* total time for all routines */
double	printtime = 0.0;	/* total of time being printed */
double	actime = 0.0;		/* accumulated time thus far for
				   putprofline */

/*
 * Option flags, from a to z.
 */
enum bool aflag = FALSE;	/* suppress static functions */
enum bool bflag = FALSE;	/* blurbs, too */
enum bool cflag = FALSE;	/* discovered call graph, too */
enum bool dflag = FALSE;	/* debugging options */
enum bool eflag = FALSE;	/* specific functions excluded */
enum bool Eflag = FALSE;	/* functions excluded with time */
enum bool fflag = FALSE;	/* specific functions requested */
enum bool Fflag = FALSE;	/* functions requested with time */
enum bool sflag = FALSE;	/* sum multiple gmon.out files */
enum bool Sflag = FALSE;	/* produce order file for scatter loading */
enum bool xflag = FALSE;	/* don't produce gmon.order file */
enum bool zflag = FALSE;	/* zero time/called functions, too */

/*
 * The debug value for debugging gprof.
 */
uint32_t debug = 0;

/*
 * Things which get -E excluded by default.
 */
static char *defaultEs[] = {
/* from the original gprof code */
    "mcount",
    /* "__mcleanup", obsolete */
/* missing from gprof for the 4.3bsd implementaion */
    "_monitor",
    "_monstartup",
    "_moncontrol",
    "_profil",
/* added to the NeXT implementation for 3.0 */
    "_moninit",
    "_monoutput",
    "_monreset",
/* added to the NeXT implementation for 3.1 */
    "_monaddition",
    "_moncount",
    "_add_profil",
    0
};

/*
 * Static function declarations.
 */
static void getpfile(
    char *filename);

#ifdef __OPENSTEP__
static void read_rld_state(
    int fd,
    char *filename,
    uint32_t nbytes);
#endif

static void read_dyld_state(
    uint32_t type,
    int fd,
    char *filename,
    uint32_t nbytes,
    uint32_t magic);

static uint32_t new_sample_set(
    int fd,
    char *filename,
    uint32_t nbytes,
    enum bool old_style,
    uint32_t magic);

static void readarcs(
    int fd,
    char *filename,
    uint32_t nbytes);

static void readarcs_64(
    int fd,
    char *filename,
    uint32_t nbytes);

static void readarcs_orders(
    int fd,
    char *filename,
    uint32_t nbytes);

static void readarcs_orders_64(
    int fd,
    char *filename,
    uint32_t nbytes);

static void tally(
    uint64_t frompc,
    uint64_t selfpc,
    uint32_t count,
    uint32_t order);

static void dumpsum(
    char *sumfile);

static void asgnsamples(
    struct sample_set *s);

static uint64_t min(
    uint64_t a,
    uint64_t b);

static uint64_t max(
    uint64_t a,
    uint64_t b);

static void alignentries(
    void);

/* apple_version is created by the libstuff/Makefile */
extern char apple_version[];
char *version = apple_version;

int
main(
int argc,
char **argv,
char **envp)
{
    uint32_t i;
    char **sp;
    nltype **timesortnlp;

	progname = argv[0];
	--argc;
	argv++;
	debug = 0;
	bflag = TRUE;
	while(*argv != 0 && **argv == '-'){
	    (*argv)++;
	    switch(**argv){
	    case 'a':
		aflag = TRUE;
		break;
	    case 'b':
		bflag = FALSE;
		break;
	    case 'c':
		printf("%s: -c not supported\n", progname);
		/* cflag = TRUE; */
		break;
	    case 'd':
		dflag = TRUE;
		(*argv)++;
		debug |= atoi(*argv);
		debug |= ANYDEBUG;
#ifdef DEBUG
		printf("[main] debug = %u\n", debug);
#else
		printf("%s: -d ignored\n", progname);
#endif
		break;
	    case 'E':
		++argv;
		addlist(Elist, *argv);
		Eflag = TRUE;
		addlist(elist, *argv);
		eflag = TRUE;
		break;
	    case 'e':
		addlist(elist, *++argv);
		eflag = TRUE;
		break;
	    case 'F':
		++argv;
		addlist(Flist, *argv);
		Fflag = TRUE;
		addlist(flist, *argv);
		fflag = TRUE;
		break;
	    case 'f':
		addlist(flist, *++argv);
		fflag = TRUE;
		break;
	    case 'S':
		Sflag = TRUE;
		break;
	    case 's':
		sflag = TRUE;
		break;
	    case 'x':
		xflag = TRUE;
		break;
	    case 'z':
		zflag = TRUE;
		break;
	    }
	    argv++;
	}

	if(*argv != 0){
	    a_outname = *argv;
	    argv++;
	}
	else{
	    a_outname = A_OUTNAME;
	}

	if(*argv != 0){
	    gmonname = *argv;
	    argv++;
	}
	else{
	    gmonname = GMONNAME;
	}

	/*
	 * Turn off default functions.
	 */
	for(sp = &defaultEs[0]; *sp; sp++){
	    Eflag = TRUE;
	    addlist(Elist, *sp);
	    eflag = TRUE;
	    addlist(elist, *sp);
	}

	/*
	 * Get information about a.out file.
	 */
	getnfile();
	if(errors != 0)
	    exit(1);

	/*
	 * Get information about mon.out file(s).
	 */
	hz = 0;
	do{
	    getpfile(gmonname);
	    if(*argv != 0){
		gmonname = *argv;
	    }
	}while(*argv++ != 0);

	/*
	 * How many ticks per second?  If we can't tell, report time in ticks.
	 * If this was picked up from the mon.out files use that value.
	 */
#ifndef __OPENSTEP__
	if(hz == 0)
#endif
	    hz = hertz();
	if(hz == 0){
	    hz = 1;
	    fprintf(stderr, "time is in ticks, not seconds\n");
	}

	/*
	 * Dump out a gmon.sum file if requested.
	 */
	if(sflag){
	    dumpsum(GMONSUM);
	    exit(0);
	}

	/*
	 * Assign samples to procedures.
	 */
	alignentries();
	for(i = 0; i < nsample_sets; i++)
	    asgnsamples(sample_sets + i);

	/*
	 * Assemble the dynamic profile.
	 */
	timesortnlp = doarcs();

	/*
	 * Print the dynamic profile.
	 */
	printgprof(timesortnlp);	

	/*
	 * Print the flat profile.
	 */
	printprof();	

	/*
	 * Print the index.
	 */
	printindex();	

	/*
	 * Dump out the order files if requested.
	 */
	if(Sflag)
	    printscatter();

	return(0);
}

static
void
getpfile(
char *filename)
{
    uint32_t magic, left;
    gmon_data_t data;
    int fd;
    struct stat stat;

	if((fd = open(filename, O_RDONLY)) == -1)
	    system_fatal("can't open: %s", filename);
	if(fstat(fd, &stat) == -1)
	    system_fatal("can't stat: %s", filename);
	/*
	 * See if this gmon.out file is an old format or new format by looking
	 * for the magic number of the new format.
	 */
	if(read(fd, &magic, sizeof(uint32_t)) != sizeof(uint32_t))
	    system_fatal("malformed gmon.out file: %s (can't read magic "
			 "number)", filename);
	if(magic == GMON_MAGIC || magic == GMON_MAGIC_64){
#ifdef DEBUG_GMON_OUT
	    if(magic == GMON_MAGIC)
		printf("GMON_MAGIC\n");
	    else
		printf("GMON_MAGIC_64\n");
#endif
	    /*
	     * This is a new format gmon.out file.  After the magic number comes
	     * any number of pairs of gmon_data structs and some typed data.
	     */
	    left = stat.st_size - sizeof(uint32_t);
	    while(left >= sizeof(struct gmon_data)){
		if(read(fd, &data, sizeof(struct gmon_data)) !=
			sizeof(struct gmon_data))
		    system_fatal("malformed gmon.out file: %s (can't read "
				 "gmon_data struct)", filename);
		left -= sizeof(struct gmon_data);
		if(left < data.size)
		    fatal("truncated or malformed gmon.out file: %s (value in "
			  "size field in gmon_data struct more than left in "
			  "file)", filename);
#ifdef DEBUG_GMON_OUT
		printf("gmon_data struct: type = %u size = %u\n", data.type,
		       data.size);
#endif
		switch(data.type){
		case GMONTYPE_SAMPLES:
#ifdef DEBUG_GMON_OUT
		    printf("GMONTYPE_SAMPLES\n");
#endif
		    new_sample_set(fd, filename, data.size, FALSE, magic);
		    break;
		case GMONTYPE_RAWARCS:
#ifdef DEBUG_GMON_OUT
		    printf("GMONTYPE_RAWARCS\n");
#endif
		    if(magic == GMON_MAGIC)
			readarcs(fd, filename, data.size);
		    else
			readarcs_64(fd, filename, data.size);
		    break;
		case GMONTYPE_ARCS_ORDERS:
#ifdef DEBUG_GMON_OUT
		    printf("GMONTYPE_ARCS_ORDERS\n");
#endif
		    if(magic == GMON_MAGIC)
			readarcs_orders(fd, filename, data.size);
		    else
			readarcs_orders_64(fd, filename, data.size);
		    break;
#ifdef __OPENSTEP__
		case GMONTYPE_RLD_STATE:
#ifdef DEBUG_GMON_OUT
		    printf("GMONTYPE_RLD_STATE\n");
#endif
		    if(grld_nloaded_states == 0){
			read_rld_state(fd, filename, data.size);
			get_rld_state_symbols();
		    }
		    else{
			warning("can't process more than one rld loaded state "
			        "(ignoring rld state from: %s)", filename);
			lseek(fd, data.size, L_INCR);
		    }
		    break;
#endif
		case GMONTYPE_DYLD_STATE:
#ifdef DEBUG_GMON_OUT
		    printf("GMONTYPE_DYLD_STATE\n");
#endif
		    goto setup_dyld_state;
		case GMONTYPE_DYLD2_STATE:
#ifdef DEBUG_GMON_OUT
		    printf("GMONTYPE_DYLD2_STATE\n");
#endif
setup_dyld_state:
		    if(image_count == 0){
			read_dyld_state(data.type, fd, filename, data.size,
					magic);
			get_dyld_state_symbols();
		    }
		    else{
			warning("can't process more than one dyld state "
			        "(ignoring dyld state from: %s)", filename);
			lseek(fd, data.size, L_INCR);
		    }
		    break;
		default:
#ifdef DEBUG_GMON_OUT
		    printf("Unknown data.type = %u\n", data.type);
#endif
		    fatal("truncated or malformed gmon.out file: %s (value in "
			  "type field in gmon_data struct unknown)", filename);
		}
		left -= data.size;
	    }
	    if(left != 0)
		fatal("truncated or malformed gmon.out file: %s (file end in "
		      "the middle of a gmon_data struct %u)", filename, left);
	}
	else{
	    /*
	     * This is an old format gmon.out file.  It has a profile header
	     * then * an array of sampling hits within pc ranges, and then arcs.
	     */
	    lseek(fd, 0, L_SET);
	    left = stat.st_size;
	    left -= new_sample_set(fd, filename, left, TRUE, GMON_MAGIC);
	    readarcs(fd, filename, left);
	}
	close(fd);
}

#ifdef __OPENSTEP__
static
void
read_rld_state(
int fd,
char *filename,
uint32_t nbytes)
{
    uint32_t i, j, size, size_read, str_size;
    char *strings;

	size_read = 0;
	size = sizeof(uint32_t);
	if(read(fd, &grld_nloaded_states, size) != size)
	    system_fatal("malformed gmon.out file: %s (can't read number of "
			 "rld states)", filename);
	size_read += size;

	grld_loaded_state = (struct rld_loaded_state *)
		malloc(grld_nloaded_states * sizeof(struct rld_loaded_state));
	if(grld_loaded_state == NULL)
	    fatal("no room for rld states (malloc failed)");

	for(i = 0; i < grld_nloaded_states; i++){
	    size = sizeof(uint32_t);
	    if(read(fd, &(grld_loaded_state[i].header_addr), size) != size)
		system_fatal("malformed gmon.out file: %s (can't read header "
		    "address of rld state %lu)", filename, i);
	    size_read += size;

	    size = sizeof(uint32_t);
	    if(read(fd, &(grld_loaded_state[i].nobject_filenames), size) != size)
		system_fatal("malformed gmon.out file: %s (can't read number "
		    "of object file names of rld state %lu)", filename, i);
	    size_read += size;

	    grld_loaded_state[i].object_filenames = (char **)
		malloc((grld_loaded_state[i].nobject_filenames + 1) *
		       sizeof(char *));
	    if(grld_loaded_state[i].object_filenames == NULL)
		fatal("no room for rld state %lu object file names (malloc "
		      "failed)", i);

	    size = grld_loaded_state[i].nobject_filenames * sizeof(char *);
	    if(read(fd, grld_loaded_state[i].object_filenames, size) != size)
		system_fatal("malformed gmon.out file: %s (can't read offsets "
		    "to file names of rld state %lu)", filename, i);
	    size_read += size;

	    size = sizeof(uint32_t);
	    if(read(fd, &str_size, size) != size)
		system_fatal("malformed gmon.out file: %s (can't read string "
		    "size of rld state %lu)", filename, i);
	    size_read += size;

	    strings = (char *)malloc(str_size);
	    if(strings == NULL)
		fatal("no room for rld state %lu object file names (malloc "
		      "failed)", i);

	    size = str_size;
	    if(read(fd, strings, size) != size)
		system_fatal("malformed gmon.out file: %s (can't read strings "
		    "of file names of rld state %lu)", filename, i);
	    size_read += size;

	    for(j = 0; j < grld_loaded_state[i].nobject_filenames; j++){
		if((uint32_t)(grld_loaded_state[i].object_filenames[j]) >
		   str_size)
		    fatal("malformed gmon.out file: %s (bad offset to object "
			  "filename %lu of rld state %lu)", filename, j, i);
		grld_loaded_state[i].object_filenames[j] = strings +
		    (uint32_t)(grld_loaded_state[i].object_filenames[j]);
	    }
	    grld_loaded_state[i].object_filenames[j] = NULL;
	}

#ifdef DEBUG
	if(debug & RLDDEBUG){
	    printf("grld_nloaded_states = %lu\n", grld_nloaded_states);
	    printf("grld_loaded_state 0x%x\n", (unsigned int)grld_loaded_state);
	    for(i = 0; i < grld_nloaded_states; i++){
		printf("state %lu\n\tnobject_filenames %lu\n\tobject_filenames "
		      "0x%x\n\theader_addr 0x%x\n", i,
		      grld_loaded_state[i].nobject_filenames,
		      (unsigned int)(grld_loaded_state[i].object_filenames),
		      (unsigned int)(grld_loaded_state[i].header_addr));
		for(j = 0; j < grld_loaded_state[i].nobject_filenames; j++)
		    printf("\t\t%s\n", grld_loaded_state[i].object_filenames[j]);
	    }
	}
#endif
}
#endif

static
void
read_dyld_state(
uint32_t type,
int fd,
char *filename,
uint32_t nbytes,
uint32_t magic)
{
    uint32_t i, offset, sizeof_vmaddr, vmaddr32;
    uint64_t vmaddr64;
    char *buf;

	buf = malloc(nbytes);
	if(buf == NULL)
	    fatal("no room for dyld state (malloc failed)");

	if(read(fd, buf, nbytes) != (int)nbytes)
	    system_fatal("malformed gmon.out file: %s (can't read dyld state)",
			 filename);

	offset = 0;
	if(offset + sizeof(uint32_t) > nbytes)
	    fatal("truncated or malformed gmon.out file: %s (image count "
		  "extends past the end of the dyld state)", filename);
	memcpy(&image_count, buf + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	dyld_images = (struct dyld_image *)malloc(sizeof(struct dyld_image) *
						  image_count);
	if(dyld_images == NULL)
	    fatal("no room for dyld images (malloc failed)");

	if(magic == GMON_MAGIC)
	    sizeof_vmaddr = sizeof(uint32_t);
	else
	    sizeof_vmaddr = sizeof(uint64_t);
	for(i = 0; i < image_count; i++){
	    if(offset + sizeof_vmaddr > nbytes)
		fatal("truncated or malformed gmon.out file: %s (vmaddr "
		      "for image %u extends past the end of the dyld state)",
		      filename, i);

	    if(magic == GMON_MAGIC){
		memcpy(&vmaddr32, buf + offset, sizeof_vmaddr);
		vmaddr64 = vmaddr32;
	    }
	    else{
		memcpy(&vmaddr64, buf + offset, sizeof_vmaddr);
	    }
	    offset += sizeof_vmaddr;
	
	    if(type == GMONTYPE_DYLD_STATE){
		dyld_images[i].vmaddr_slide = vmaddr64;
		dyld_images[i].image_header = 0;
	    }
	    else{
		dyld_images[i].image_header = vmaddr64;
		dyld_images[i].vmaddr_slide = 0;
	    }

	    dyld_images[i].name = buf + offset;
	    while(buf[offset] != '\0' && offset < nbytes)
		offset++;
	    if(buf[offset] == '\0' && offset < nbytes)
		offset++;
	    if(offset > nbytes)
		fatal("truncated or malformed gmon.out file: %s (name "
		      "for image %u extends past the end of the dyld state)",
		      filename, i);
	}
#ifdef DEBUG
	if(debug & DYLDDEBUG){
	    printf("image_count = %u\n", image_count);
	    for(i = 0; i < image_count; i++){
		printf("dyld_image[%u].name = %s\n", i, dyld_images[i].name);
		printf("dyld_image[%u].vmaddr_slide = 0x%llx\n", i,
			dyld_images[i].vmaddr_slide);
		printf("dyld_image[%u].image_header = 0x%llx\n", i,
			dyld_images[i].image_header);
	    }
	}
#endif
}

static
uint32_t
new_sample_set(
int fd,
char *filename,
uint32_t nbytes,
enum bool old_style,
uint32_t magic)
{
#ifdef __OPENSTEP__
    struct phdr header;
#else
    struct gmonhdr header32;
    struct gmonhdr_64 header;
#endif
    uint64_t i, j, size;
    uint32_t sizeof_header;
    unsigned UNIT sample, *samples;

	if(magic == GMON_MAGIC)
	    sizeof_header = sizeof(struct gmonhdr);
	else
	    sizeof_header = sizeof(struct gmonhdr_64);

	if(nbytes < sizeof_header)
	    fatal("gmon.out file: %s malformed (byte count less than the "
		  "size of the expected header)", filename);
	/*
	 * Read the profile header and check to see if it is valid.
	 */
	if(magic == GMON_MAGIC)
	    size = read(fd, &header32, sizeof_header);
	else
	    size = read(fd, &header, sizeof_header);

	if(size != sizeof_header)
	    system_fatal("malformed gmon.out file: %s (can't read header)",
			 filename);

	if(magic == GMON_MAGIC){
	    header.lpc = header32.lpc;
	    header.hpc = header32.hpc;
	    header.ncnt = header32.ncnt;
	    header.version = header32.version;
	    header.profrate = header32.profrate;
	    header.spare[0] = header32.spare[0];
	    header.spare[1] = header32.spare[1];
	    header.spare[2] = header32.spare[2];
	}

	/*
	 * The ncnt field is a byte count of the header and the number of
	 * bytes of the 2byte samples that follow it.  Try to make sure this
	 * is reasonable and that what's left in the file is at least that big.
	 */
	if(header.ncnt < sizeof_header)
	    fatal("gmon.out file: %s malformed (ncnt field less than the "
		  "size of the expected header)", filename);
	if(header.ncnt > nbytes)
	    fatal("gmon.out file: %s malformed (ncnt field greater than "
		  "the byte count for it in the file)", filename);
	if(header.hpc < header.lpc)
	    fatal("gmon.out file: %s malformed (high pc less than low pc "
		  "in header)", filename);
	if(((header.ncnt - sizeof_header) % sizeof(unsigned short)) != 0)
	    warning("gmon.out file: %s malformed (number of sample bytes "
		    "not a multiple of sizeof(unsigned short))", filename);
#ifndef __OPENSTEP__
	if(header.version != GMONVERSION)
	    warning("gmon.out file: %s unknown version (0x%x)", filename,
		    header.version);
	if(hz == 0){
	    if(header.profrate == 0)
		warning("gmon.out file: %s profrate is zero", filename);
	    else
		hz = header.profrate;
	}
	else{
	    if(hz != header.profrate)
		warning("gmon.out file: %s profrate (%d) does not prevous "
			"profrate (%u)\n", filename, header.profrate, hz);
	}
#endif

	/*
	 * See if this sample range matches another range
	 */
	for(i = 0; i < nsample_sets; i++){
	    if(sample_sets[i].s_lowpc == header.lpc &&
	       sample_sets[i].s_highpc == header.hpc &&
	       sample_sets[i].sampbytes == header.ncnt - sizeof_header)
		break;
	}
	/*
	 * For old_style gmon.out files with one only pc sample range it must
	 * match up if there are any ranges.
	 */
	if(old_style == TRUE && sample_sets != NULL && i != nsample_sets)
	    fatal("gmon.out file: %s incompatible with first gmon.out file",
		  filename);

	/*
	 * If this range does not match up with a previous range then create
	 * a new sample set for it.
	 */
	if(i == nsample_sets){
	    sample_sets = realloc(sample_sets, (nsample_sets + 1) *
				  sizeof(struct sample_set));
	    if(sample_sets == NULL)
		fatal("no room for next pc sample set (malloc failed)");
	    i = nsample_sets;
	    nsample_sets++;
	    memset(sample_sets + i, '\0', sizeof(struct sample_set));

	    sample_sets[i].s_lowpc = header.lpc;
	    sample_sets[i].s_highpc = header.hpc;
	    sample_sets[i].lowpc = header.lpc / sizeof(UNIT);
	    sample_sets[i].highpc = header.hpc / sizeof(UNIT);
	    sample_sets[i].sampbytes = header.ncnt - sizeof_header;
	    sample_sets[i].nsamples = sample_sets[i].sampbytes /
				      sizeof(unsigned UNIT);
	    sample_sets[i].scale = ((double)(sample_sets[i].highpc) -
				    (double)(sample_sets[i].lowpc)) /
				    (double)(sample_sets[i].nsamples);
	    sample_sets[i].samples = calloc(sample_sets[i].sampbytes,
					    sizeof(unsigned UNIT));
	    if(sample_sets[i].samples == NULL)
		fatal("no room for %llu sample pc's\n", 
		      sample_sets[i].sampbytes / sizeof(unsigned UNIT));
#ifndef __OPENSTEP__
	    sample_sets[i].version = header.version;
	    sample_sets[i].profrate = header.profrate;
	    sample_sets[i].spare[0] = header.spare[0];
	    sample_sets[i].spare[1] = header.spare[1];
	    sample_sets[i].spare[2] = header.spare[2];
#endif
	}
#ifdef DEBUG
	if(debug & SAMPLEDEBUG){
	    printf("[new_sample_set] header.lpc 0x%llx header.hpc 0x%llx "
		   "header.ncnt %u\n", header.lpc, header.hpc, header.ncnt);
	    printf("[new_sample_set]   s_lowpc 0x%llx   s_highpc 0x%llx\n",
		   sample_sets[i].s_lowpc, sample_sets[i].s_highpc);
	    printf("[new_sample_set]     lowpc 0x%llx     highpc 0x%llx\n" ,
		   sample_sets[i].lowpc, sample_sets[i].highpc);
	    printf("[new_sample_set] sampbytes %llu nsamples %llu\n" ,
		   sample_sets[i].sampbytes, sample_sets[i].nsamples);
#ifndef __OPENSTEP__
	    printf("[new_sample_set] version 0x%x\n",
		   (unsigned int)(sample_sets[i].version));
	    printf("[new_sample_set] profrate %d\n" ,sample_sets[i].profrate);
	    printf("[new_sample_set] spare[0] %d\n" ,sample_sets[i].spare[0]);
	    printf("[new_sample_set] spare[1] %d\n" ,sample_sets[i].spare[1]);
	    printf("[new_sample_set] spare[2] %d\n" ,sample_sets[i].spare[2]);
#endif
	}
#endif
	/*
	 * Read and sum up the sample counts for this sample set.
	 */
	size = sample_sets[i].nsamples * sizeof(unsigned UNIT);
	samples = malloc(size);
	if(samples == NULL)
	    system_fatal("can't allocate buffer of size: %llu for samples from "
			 "gmon.out file: %s", size, filename);
	if(read(fd, samples, size) != size)
	    system_fatal("can't read samples from gmon.out file: %s", filename);
	for(j = 0; j < sample_sets[i].nsamples; j++){
	    sample = samples[j];
#ifdef DEBUG
	if(debug & 8192){
	    if(sample != 0)
		printf("sample[%llu] = %u\n", j, sample);
	}
#endif
	    sample_sets[i].samples[j] += sample;
	}
	free(samples);
	return(header.ncnt);
}

static
void
readarcs(
int fd,
char *filename,
uint32_t nbytes)
{
    uint32_t i;
    struct rawarc arc;

	/*
	 * Arcs consists of a bunch of <from,self,count> tuples.
	 */
	for(i = 0; i < nbytes; i += sizeof(struct rawarc)){
	    if(read(fd, &arc, sizeof(struct rawarc)) != sizeof(struct rawarc))
		system_fatal("malformed gmon.out file: %s (can't read arcs)",
			     filename);
#ifdef DEBUG
	    if(debug & SAMPLEDEBUG){
		printf("[readarcs] frompc 0x%x selfpc 0x%x count %u\n",
		       arc.raw_frompc, arc.raw_selfpc, arc.raw_count);
	    }
#endif
	    /*
	     * Add this arc.
	     */
	    tally(arc.raw_frompc, arc.raw_selfpc, arc.raw_count, 0);
	}
}

static
void
readarcs_64(
int fd,
char *filename,
uint32_t nbytes)
{
    uint32_t i;
    struct rawarc_64 arc;

	/*
	 * Arcs consists of a bunch of <from,self,count> tuples.
	 */
	for(i = 0; i < nbytes; i += sizeof(struct rawarc_64)){
	    if(read(fd, &arc, sizeof(struct rawarc_64)) !=
	       sizeof(struct rawarc_64))
		system_fatal("malformed gmon.out file: %s (can't read arcs)",
			     filename);
#ifdef DEBUG
	    if(debug & SAMPLEDEBUG){
		printf("[readarcs] frompc 0x%llx selfpc 0x%llx count %u\n",
		       arc.raw_frompc, arc.raw_selfpc, arc.raw_count);
	    }
#endif
	    /*
	     * Add this arc.
	     */
	    tally(arc.raw_frompc, arc.raw_selfpc, arc.raw_count, 0);
	}
}

static
void
readarcs_orders(
int fd,
char *filename,
uint32_t nbytes)
{
    uint32_t i;
    struct rawarc_order arc_order;

	/*
	 * Arcs consists of a bunch of <from,self,count> tuples.
	 */
	for(i = 0; i < nbytes; i += sizeof(struct rawarc_order)){
	    if(read(fd, &arc_order, sizeof(struct rawarc_order)) !=
					sizeof(struct rawarc_order))
		system_fatal("malformed gmon.out file: %s (can't read arcs)",
			     filename);
#ifdef DEBUG
	    if(debug & SAMPLEDEBUG){
		printf("[readarcs_order] frompc 0x%x selfpc 0x%x count %u "
		       "order %u\n", arc_order.raw_frompc, arc_order.raw_selfpc,
		       arc_order.raw_count, arc_order.raw_order);
	    }
#endif

	    /*
	     * Add this arc.
	     */
	    tally(arc_order.raw_frompc, arc_order.raw_selfpc,
		  arc_order.raw_count, arc_order.raw_order);
	}
}

static
void
readarcs_orders_64(
int fd,
char *filename,
uint32_t nbytes)
{
    uint32_t i;
    struct rawarc_order_64 arc_order;

	/*
	 * Arcs consists of a bunch of <from,self,count> tuples.
	 */
	for(i = 0; i < nbytes; i += sizeof(struct rawarc_order_64)){
	    if(read(fd, &arc_order, sizeof(struct rawarc_order_64)) !=
					sizeof(struct rawarc_order_64))
		system_fatal("malformed gmon.out file: %s (can't read arcs)",
			     filename);
#ifdef DEBUG
	    if(debug & SAMPLEDEBUG){
		printf("[readarcs_order] frompc 0x%llx selfpc 0x%llx count %u "
		       "order %u\n" , arc_order.raw_frompc,arc_order.raw_selfpc,
		       arc_order.raw_count, arc_order.raw_order);
	    }
#endif

	    /*
	     * Add this arc.
	     */
	    tally(arc_order.raw_frompc, arc_order.raw_selfpc,
		  arc_order.raw_count, arc_order.raw_order);
	}
}

static
void
tally(
uint64_t frompc,
uint64_t selfpc,
uint32_t count,
uint32_t order)
{
    uint32_t i;
    nltype *parentp;
    nltype *childp;

	parentp = nllookup(frompc);
	childp = nllookup(selfpc);

	if(childp == NULL || parentp == NULL){
	    fprintf(stderr, "bad arc from 0x%llx to 0x%llx, count %u "
		    "(ignored).\n", frompc, selfpc, count);
	    return;
	}

	/*
	 * See if this arc comes from inside a shlib's text.
	 */
	for(i = 0; i < nshlib_text_ranges; i++){
	    if(frompc >= shlib_text_ranges[i].lowpc &&
	       frompc < shlib_text_ranges[i].highpc)
		break;
	}
	if(parentp->name[0] != '+' && parentp->name[0] != '-' &&
	   nshlib_text_ranges != 0 && i != nshlib_text_ranges){
#ifdef DEBUG
	    if(debug & SAMPLEDEBUG){
		printf("[tally] tossing arc frompc 0x%llx selfpc 0x%llx "
		       "count %u order %u\n", frompc, selfpc, count, order);
	    }
#endif
	    return;
	}

	childp->ncall += count;

	/*
	 * Set the call order to the lowest order in the arcs for a call this
	 * child.  Orders in the arcs are start at 1 and thus orders with the
	 * value of zero are unordered.
	 */
	if(order != 0){
	    if(childp->order == 0)
	        childp->order = order;
	    else if(order < childp->order)
	        childp->order = order;
	}

#ifdef DEBUG
	if(debug & TALLYDEBUG){
	    printf("[tally] arc from %s to %s traversed %u times\n",
		   parentp->name, childp->name, count);
	}
#endif /* DEBUG */

	addarc(parentp, childp, count, order);
}

/*
 * Dump out the gmon.sum file.
 */
static
void
dumpsum(
char *sumfile)
{
    uint32_t i, j, magic;
#ifdef __OPENSTEP__
    uint32_t strsize;
    struct phdr header;
#else
    struct gmonhdr header;
#endif
    int fd;
    struct gmon_data data;
    nltype *nlp;
    arctype *arcp;
    struct rawarc_order arc_order;

	if((fd = open(sumfile, O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1)
	    fatal("can't create gmon.sum file: %s", sumfile);

	/*
	 * Write the magic number.
	 */
        magic = GMON_MAGIC;
        if(write(fd, &magic, sizeof(uint32_t)) != sizeof(uint32_t))
	    fatal("can't write magic number to gmon.sum file: %s", sumfile);

#ifdef __OPENSTEP__
	/*
	 * Write the rld_state if any.
	 */
	if(grld_nloaded_states != 0){
	    data.type = GMONTYPE_RLD_STATE;
	    data.size = sizeof(uint32_t) +
		sizeof(uint32_t) * 3 * grld_nloaded_states;
	    for(i = 0; i < grld_nloaded_states; i++){
		data.size += sizeof(uint32_t) *
				 grld_loaded_state[i].nobject_filenames;
		for(j = 0; j < grld_loaded_state[i].nobject_filenames; j++)
		    data.size +=
			strlen(grld_loaded_state[i].object_filenames[j]) + 1;
	    }

	    if(write(fd, &data, sizeof(struct gmon_data)) !=
		     sizeof(struct gmon_data))
		fatal("can't write gmon_data struct to gmon.sum file: %s",
		      sumfile);
	    if(write(fd, &grld_nloaded_states, sizeof(uint32_t)) !=
		     sizeof(uint32_t))
		fatal("can't write to gmon.sum file: %s", sumfile);
	    for(i = 0; i < grld_nloaded_states; i++){
		if(write(fd, &(grld_loaded_state[i].header_addr),
		         sizeof(uint32_t)) != sizeof(uint32_t))
		    fatal("can't write to gmon.sum file: %s", sumfile);
		if(write(fd, &(grld_loaded_state[i].nobject_filenames),
		         sizeof(uint32_t)) != sizeof(uint32_t))
		    fatal("can't write to gmon.sum file: %s", sumfile);
		strsize = 0;
		for(j = 0; j < grld_loaded_state[i].nobject_filenames; j++){
		    if(write(fd, &strsize, sizeof(uint32_t)) != 
			     sizeof(uint32_t))
			fatal("can't write to gmon.sum file: %s", sumfile);
		    strsize +=
			strlen(grld_loaded_state[i].object_filenames[j]) + 1;
		}
		if(write(fd, &strsize, sizeof(uint32_t)) != 
			 sizeof(uint32_t))
		    fatal("can't write to gmon.sum file: %s", sumfile);
		for(j = 0; j < grld_loaded_state[i].nobject_filenames; j++){
		    strsize =
			strlen(grld_loaded_state[i].object_filenames[j]) + 1;
		    if(write(fd, grld_loaded_state[i].object_filenames[j],
			     strsize) != strsize)
			fatal("can't write to gmon.sum file: %s", sumfile);
		}
	    }
	}
#endif

	/*
	 * Write the samples.
	 */
	for(i = 0; i < nsample_sets; i++){
	    data.type = GMONTYPE_SAMPLES;
	    data.size = sample_sets[i].sampbytes + sizeof(header);
	    if(write(fd, &data, sizeof(struct gmon_data)) !=
               sizeof(struct gmon_data))
		fatal("can't write gmon_data struct to gmon.sum file: %s",
		      sumfile);

	    memset(&header, '\0', sizeof(header));
#ifdef __OPENSTEP__
	    header.lpc = (char *)sample_sets[i].s_lowpc;
	    header.hpc = (char *)sample_sets[i].s_highpc;
#else
	    header.lpc = sample_sets[i].s_lowpc;
	    header.hpc = sample_sets[i].s_highpc;
	    header.version = sample_sets[i].version;
	    header.profrate = sample_sets[i].profrate;
	    header.spare[0] = sample_sets[i].spare[0];
	    header.spare[1] = sample_sets[i].spare[1];
	    header.spare[2] = sample_sets[i].spare[2];
#endif
	    header.ncnt = sample_sets[i].sampbytes + sizeof(header);
	    if(write(fd, &header, sizeof(header)) != sizeof(header))
		fatal("can't write header to gmon.sum file: %s", sumfile);

	    for(j = 0; j < sample_sets[i].nsamples; j++){
		if(write(fd, &(sample_sets[i].samples[j]),
			 sizeof(unsigned UNIT)) != sizeof(unsigned UNIT))
		    system_fatal("can't write samples to gmon.sum file: %s",
				 sumfile);
	    }
	}

	/*
	 * Write the arc information
	 */
	data.type = GMONTYPE_ARCS_ORDERS;
	data.size = 0;
	for(nlp = nl; nlp < npe; nlp++){
	    for(arcp = nlp->children; arcp; arcp = arcp->arc_childlist){
		data.size += sizeof(struct rawarc_order);
	    }
	}
	if(write(fd, &data, sizeof(struct gmon_data)) !=
	   sizeof(struct gmon_data))
	    fatal("can't write gmon_data struct to gmon.sum file: %s",
		  sumfile);

	for(nlp = nl; nlp < npe; nlp++){
	    for(arcp = nlp->children; arcp; arcp = arcp->arc_childlist){
		arc_order.raw_frompc = arcp->arc_parentp->value;
		arc_order.raw_selfpc = arcp->arc_childp->value;
		arc_order.raw_count  = arcp->arc_count;
		arc_order.raw_order  = arcp->arc_order;
		if(write(fd, &arc_order, sizeof(struct rawarc_order)) != 
		   sizeof(struct rawarc_order))
		    fatal("can't write arc to gmon.sum file: %s", sumfile);
#ifdef DEBUG
		if(debug & SAMPLEDEBUG){
		    printf("[dumpsum] frompc 0x%x selfpc 0x%x count %u "
			   "order %u\n" ,
			   (unsigned int)arc_order.raw_frompc,
			   (unsigned int)arc_order.raw_selfpc,
			   arc_order.raw_count,
			   arc_order.raw_order);
		}
#endif
	    }
	}
	close(fd);
}


/*
 *	Assign samples to the procedures to which they belong.
 *
 *	There are three cases as to where pcl and pch can be
 *	with respect to the routine entry addresses svalue0 and svalue1
 *	as shown in the following diagram.  overlap computes the
 *	distance between the arrows, the fraction of the sample
 *	that is to be credited to the routine which starts at svalue0.
 *
 *	    svalue0                                         svalue1
 *	       |                                               |
 *	       v                                               v
 *
 *	       +-----------------------------------------------+
 *	       |					       |
 *	  |  ->|    |<-		->|         |<-		->|    |<-  |
 *	  |         |		  |         |		  |         |
 *	  +---------+		  +---------+		  +---------+
 *
 *	  ^         ^		  ^         ^		  ^         ^
 *	  |         |		  |         |		  |         |
 *	 pcl       pch		 pcl       pch		 pcl       pch
 *
 *	For the vax we assert that samples will never fall in the first
 *	two bytes of any routine, since that is the entry mask,
 *	thus we give call alignentries() to adjust the entry points if
 *	the entry mask falls in one bucket but the code for the routine
 *	doesn't start until the next bucket.  In conjunction with the
 *	alignment of routine addresses, this should allow us to have
 *	only one sample for every four bytes of text space and never
 *	have any overlap (the two end cases, above).
 */
static
void
asgnsamples(
struct sample_set *s)
{
    uint32_t i, j;
    unsigned UNIT ccnt;
    double time;
    uint32_t pcl, pch, overlap, svalue0, svalue1;

	/* read samples and assign to namelist symbols */
	for(i = 0, j = 1; i < s->nsamples; i++){
	    ccnt = s->samples[i];
	    if (ccnt == 0)
		continue;
	    pcl = s->lowpc + s->scale * i;
	    pch = s->lowpc + s->scale * (i + 1);
	    time = ccnt;
#ifdef DEBUG
	    if(debug & SAMPLEDEBUG){
		printf("[asgnsamples] pcl 0x%x pch 0x%x ccnt %d\n",
		       (unsigned int)pcl, (unsigned int)pch, ccnt);
	    }
#endif
	    totime += time;
	    for(j = j - 1; j < nname; j++){
		svalue0 = nl[j].svalue;
		svalue1 = nl[j+1].svalue;
		/*
		 * if high end of tick is below entry address, 
		 * go for next tick.
		 */
		if(pch < svalue0)
		    break;
		/*
		 * if low end of tick into next routine,
		 * go for next routine.
		 */
		if(pcl >= svalue1)
		    continue;
		overlap = min(pch, svalue1) - max(pcl, svalue0);
		if(overlap > 0){
#ifdef DEBUG
		    if (debug & SAMPLEDEBUG) {
			printf("[asgnsamples] (0x%x->0x%x-0x%x) %s gets %f "
			       "ticks %u overlap\n",
			       (unsigned int)(nl[j].value/sizeof(UNIT)),
			       (unsigned int)svalue0, (unsigned int)svalue1,
			       nl[j].name, overlap * time / s->scale, overlap);
		    }
#endif
		    nl[j].time += overlap * time / s->scale;
		}
	    }
	}
#ifdef DEBUG
	if(debug & SAMPLEDEBUG){
	    printf("[asgnsamples] totime %f\n", totime);
	}
#endif
}

static
uint64_t
min(
uint64_t a,
uint64_t b)
{
    if(a < b)
	return(a);
    return(b);
}

static
uint64_t
max(
uint64_t a,
uint64_t b)
{
    if(a > b)
	return(a);
    return(b);
}

/*
 * Calculate scaled entry point addresses (to save time in asgnsamples),
 * and possibly push the scaled entry points over the entry mask (vax),
 * if it turns out that the entry point is in one bucket and the code
 * for a routine is in the next bucket.
 */
static
void
alignentries(
void)
{
    struct nl *nlp;
#ifdef vax
    uint32_t i;
    uint32_t bucket_of_entry;
    uint32_t bucket_of_code;
#endif /* vax */

	for(nlp = nl; nlp < npe; nlp++){
	    nlp->svalue = nlp->value / sizeof(UNIT);
#ifdef vax
	    for(i = 0; i < nsample_sets; i++){
		if(nlp->svalue >= sample_sets[i].s_lowpc &&
		   nlp->svalue < sample_sets[i].s_lowpc)
		    break;
	    }
	    if(i == nsample_sets)
		continue;
	    bucket_of_entry = (nlp->svalue - sample_sets[i].lowpc) /
			      sample_sets[i].scale;
	    bucket_of_code = (nlp->svalue + UNITS_TO_CODE -
			      sample_sets[i].lowpc) / sample_sets[i].scale;
	    if(bucket_of_entry < bucket_of_code) {
#ifdef DEBUG
		if(debug & SAMPLEDEBUG){
		    printf("[alignentries] pushing svalue 0x%x to 0x%x\n",
			   (unsigned int)nlp->svalue,
			   (unsigned int)(nlp->svalue + UNITS_TO_CODE));
		}
#endif /* DEBUG */
		nlp->svalue += UNITS_TO_CODE;
	    }
#endif /* vax */
	}
}
