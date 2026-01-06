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
#ifndef RLD
#include <sys/time.h>
#include <mach/mach.h>
#include "stuff/openstep_mach.h"
#include <libc.h>
#ifndef __OPENSTEP__
#include <time.h>
#include <utime.h>
#endif
#include "stuff/ofile.h"
#include "stuff/breakout.h"
#include "stuff/allocate.h"
#include "stuff/rnd.h"
#include "stuff/errors.h"
#ifdef LTO_SUPPORT
#include "stuff/lto.h"
#endif /* LTO_SUPPORT */
#include "stuff/write64.h"

static void copy_new_symbol_info(
    char *p,
    uint32_t *size,
    struct dysymtab_command *dyst,
    struct dysymtab_command *old_dyst,
    struct twolevel_hints_command *hints_cmd,
    struct twolevel_hints_command *old_hints_cmd,
    struct object *object);

static void make_table_of_contents(
    struct arch *archs,
    char *output,
    time_t toc_time,
    enum bool sort_toc,
    enum bool commons_in_toc,
    enum bool force_64bit_toc,
    enum bool library_warnings);

static enum bool toc_symbol(
    struct nlist *symbol,
    enum bool commons_in_toc,
    struct section **sections);

static enum bool toc_symbol_64(
    struct nlist_64 *symbol64,
    enum bool commons_in_toc,
    struct section_64 **sections64);

static enum bool toc(
    uint32_t n_strx,
    uint8_t n_type,
    uint64_t n_value,
    enum bool commons_in_toc,
    enum bool attr_no_toc);

static int toc_entry_name_qsort(
    const struct toc_entry *toc1,
    const struct toc_entry *toc2);

static int toc_entry_index_qsort(
    const struct toc_entry *toc1,
    const struct toc_entry *toc2);

static enum bool check_sort_toc_entries(
    struct arch *arch,
    char *output,
    enum bool library_warnings);

static void warn_member(
    struct arch *arch,
    struct member *member,
    const char *format, ...)
#ifndef __MWERKS__
    __attribute__ ((format (printf, 3, 4)))
#endif
    ;

/*
 * writeout() creates an ofile from the data structure pointed to by
 * archs (of narchs size) into the specified output file (output).  The file is
 * created with the mode, mode.  If there are libraries in the data structures
 * a new table of contents is created and is sorted if sort_toc is TRUE and
 * commons symbols are included in the table of contents if commons_in_toc is
 * TRUE.  The normal use will have sort_toc == TRUE and commons_in_toc == FALSE.
 * For testing force_64bit_toc can be set to TRUE but it should normally be set
 * to FALSE.  If the output contains archive members that have archive offsets
 * greater than 32-bits then a 64-bit toc will be used.  If warnings about
 * unusual libraries are printed if library_warnings == TRUE.  If throttle is
 * not NULL it is set to a value of bytes per second to limiting the writes to
 * in order to not eat all I/O bandwidth.
 */
__private_extern__
void
writeout(
struct arch *archs,
uint32_t narchs,
char *output,
unsigned short mode,
enum bool sort_toc,
enum bool commons_in_toc,
enum bool force_64bit_toc,
enum bool library_warnings,
uint32_t *throttle)
{
    uint32_t fsync;
    int fd;
#ifndef __OPENSTEP__
    struct timeval toc_timeval;
    struct timespec toc_timespec;
#else
    time_t timep[2];
    time_t toc_time;
#endif
    mach_port_t my_mach_host_self;
    char *file, *p;
    uint64_t file_size;
    enum bool seen_archive;
    kern_return_t r;
    int time_result;

	seen_archive = FALSE;

#ifndef __OPENSTEP__
	/* cctools-port: replaced __builtin_available */
#if 0
	if (__builtin_available(macOS 10.12, *)) {
#endif /* 0 */
#ifdef HAVE_CLOCK_GETTIME
	    if (clock_gettime(CLOCK_REALTIME, &toc_timespec)) {
		system_error("clock_gettime failed");
		return;
	    }
#endif /* HAVE_CLOCK_GETTIME */
#if 0
	} else {
#endif /* 0 */
#ifndef HAVE_CLOCK_GETTIME
	    if (gettimeofday(&toc_timeval, NULL)) {
		system_error("gettimeofday failed");
		return;
	    }
#endif /* !HAVE_CLOCK_GETTIME */
#if 0
	}
#endif /* 0 */
#else
	/*
	 * The environment variable ZERO_AR_DATE is used here and other
	 * places that write archives to allow testing and comparing
	 * things for exact binary equality.
	 */
	if (getenv("ZERO_AR_DATE") == NULL) {
	    toc_time = time(NULL);
	} else {
	    toc_time = 0;
	}
#endif /* !defined(__OPENSTEP__) */

	writeout_to_mem(archs, narchs, output, (void **)&file, &file_size,
                        sort_toc, commons_in_toc, force_64bit_toc,
			library_warnings, &seen_archive);

	/*
	 * Create the output file.  The unlink() is done to handle the problem
	 * when the outputfile is not writable but the directory allows the
	 * file to be removed (since the file may not be there the return code
	 * of the unlink() is ignored).
	 */
	(void)unlink(output);
	if(throttle != NULL)
	    fsync = O_FSYNC;
	else
	    fsync = 0;
        if(output != NULL){
            if((fd = open(output, O_WRONLY|O_CREAT|O_TRUNC|fsync, mode)) == -1){
                system_error("can't create output file: %s", output);
                goto cleanup;
            }
#ifdef F_NOCACHE
            /* tell filesystem to NOT cache the file when reading or writing */
            (void)fcntl(fd, F_NOCACHE, 1);
#endif
        }
        else{
            throttle = NULL;
            fd = fileno(stdout);
        }
        if(throttle != NULL){
#define WRITE_SIZE (32 * 1024)
            struct timeval start, end;
            struct timezone tz;
            uint32_t bytes_written, bytes_per_second, write_size;
            double time_used, time_should_have_took, usecs_to_kill;
            static struct host_sched_info info = { 0 };
            natural_t count; /* cctools-port: unsigned int -> natural_t */
            kern_return_t r;

            p = file;
            bytes_written = 0;
            bytes_per_second = 0;
            count = HOST_SCHED_INFO_COUNT;
            my_mach_host_self = mach_host_self();
            if((r = host_info(my_mach_host_self, HOST_SCHED_INFO, (host_info_t)
                            (&info), &count)) != KERN_SUCCESS){
                mach_port_deallocate(mach_task_self(), my_mach_host_self);
                my_mach_error(r, "can't get host sched info");
            }
            mach_port_deallocate(mach_task_self(), my_mach_host_self);
            if(gettimeofday(&start, &tz) == -1)
                goto no_throttle;
#undef THROTTLE_DEBUG
            do {
                if((file + file_size) - p < WRITE_SIZE)
                    write_size = (uint32_t)((file + file_size) - p);
                else
                    write_size = WRITE_SIZE;
                /* MDT: write(2) is OK here, write_size is less than 2^31-1 */
                if(write(fd, p, write_size) != (ssize_t)write_size){
                    system_error("can't write output file: %s", output);
                    goto cleanup;
                }
                p += write_size;
                if(p < file + file_size || *throttle == UINT_MAX){
                    bytes_written += write_size;
                    (void)gettimeofday(&end, &tz);
#ifdef THROTTLE_DEBUG
                    printf("start sec = %u usec = %u\n", start.tv_sec,
                        start.tv_usec);
                    printf("end sec = %u usec = %u\n", end.tv_sec,
                        end.tv_usec);
#endif
                    time_used = end.tv_sec - start.tv_sec;
                    if(end.tv_usec >= start.tv_usec)
                        time_used +=
                            ((double)(end.tv_usec - start.tv_usec)) / 1000000.0;
                    else
                        time_used += -1.0 +
                            ((double)(1000000 + end.tv_usec - start.tv_usec) /
                            1000000.0);
                    bytes_per_second = ((double)bytes_written / time_used);
#ifdef THROTTLE_DEBUG
                    printf("time_used = %f bytes_written = %lu bytes_per_second"
                            " = %lu throttle = %lu\n", time_used, bytes_written,
                            bytes_per_second, *throttle);
#endif
                    if(bytes_per_second > *throttle){
                        time_should_have_took =
                            (double)bytes_written * (1.0/(double)(*throttle));
                        usecs_to_kill =
                            (time_should_have_took - time_used) * 1000000.0;
#ifdef THROTTLE_DEBUG
                        printf("time should have taken = %f usecs to kill %f\n",
                            time_should_have_took, usecs_to_kill);
#endif
                        usleep((u_int)usecs_to_kill);
                        bytes_written = 0;
                        bytes_per_second = 0;
                        (void)gettimeofday(&start, &tz);
                    }
                }
            } while(p < file + file_size);
            if(*throttle == UINT_MAX)
                *throttle = bytes_per_second;
        }
        else{
no_throttle:
	    if(write64(fd, file, file_size) != (ssize_t)file_size){
		system_error("can't write output file: %s", output);
		goto cleanup;
	    }
	}
	if(output != NULL && close(fd) == -1){
	    system_fatal("can't close output file: %s", output);
	    goto cleanup;
	}
	if(seen_archive == TRUE){
#ifndef __OPENSTEP__
	    /*
	     * If ZERO_AR_DATE is set here, writeout_to_mem has zeroed the
	     * SYMDEF mod time; the other ar content objects are assumed to
	     * have been zeroed out when the library was created. writeout
	     * will not zero out the modification time in the filesystem.
	     */
		/* cctools-port: replaced __builtin_available */
#if 0
	    if (__builtin_available(macOS 10.12, *)) {
#endif /* 0 */
#ifdef HAVE_UTIMESAT
		struct timespec times[2] = {0};
		memcpy(&times[0], &toc_timespec, sizeof(struct timespec));
		memcpy(&times[1], &toc_timespec, sizeof(struct timespec));
		time_result = utimensat(AT_FDCWD, output, times, 0);
#endif /* HAVE_UTIMESAT */
#if 0
	    }
	    else {
#endif /* 0 */
#ifndef HAVE_UTIMESAT
		struct timeval times[2] = {0};
		memcpy(&times[0], &toc_timeval, sizeof(struct timeval));
		memcpy(&times[1], &toc_timeval, sizeof(struct timeval));
		time_result = utimes(output, times);
#endif /* !HAVE_UTIMESAT */
#if 0
	    }
#endif /* 0 */
#else
	    timep[0] = toc_time;
	    timep[1] = toc_time;
	    time_result = utime(output, timep);
#endif
	    if (time_result) {
		system_fatal("can't set the modifiy times in output file: %s",
			     output);
		goto cleanup;
	    }
	}
cleanup:
	if((r = vm_deallocate(mach_task_self(), (vm_address_t)file,
			      file_size)) != KERN_SUCCESS){
	    my_mach_error(r, "can't vm_deallocate() buffer for output file");
	    return;
	}
}

/*
 * writeout_to_mem() creates an ofile in memory from the data structure pointed 
 * to by archs (of narchs size).  Upon successful return, *outputbuf will point
 * to a vm_allocate'd buffer representing the ofile which should be 
 * vm_deallocated when it is no longer needed.  length will point to the length
 * of the outputbuf buffer.  The filename parameter is used for error reporting
 * - if filename is NULL, a dummy file name is used.  If there are libraries in
 * the data structures a new table of contents is created and is sorted if 
 * sort_toc is TRUE and commons symbols are included in the table of contents 
 * if commons_in_toc is TRUE.  The normal use will have sort_toc == TRUE and
 * commons_in_toc == FALSE.  For testing force_64bit_toc can be set to TRUE but
 * it should normally be set to FALSE.  If the output contains archive members
 * that have archive offsets greater than 32-bits then a 64-bit toc will be
 * used.  If warnings about unusual libraries are printed if library_warnings
 * == TRUE.  If an archive is in the output then *seen_archive will be set to
 * TRUE else FALSE.
 */
__private_extern__
void
writeout_to_mem(
struct arch *archs,
uint32_t narchs,
char *filename,
void **outputbuf,
uint64_t *length,
enum bool sort_toc,
enum bool commons_in_toc,
enum bool force_64bit_toc,
enum bool library_warnings,
enum bool *seen_archive)
{
    uint32_t i, j, k, pad, size;
    uint64_t file_size, offset;
    uint32_t i32;
    uint64_t i64;
    enum byte_sex target_byte_sex, host_byte_sex;
    char *file, *p;
    kern_return_t r;
    struct fat_header *fat_header;
    struct fat_arch *fat_arch;
    struct fat_arch_64 *fat_arch64;
    struct dysymtab_command dyst;
    struct twolevel_hints_command hints_cmd;
    struct load_command lc, *lcp;
    struct dylib_command dl, *dlp;
    time_t toc_time;
    int32_t timestamp, index;
    uint32_t ncmds;
    enum bool swapped;

	/* 
	 * If filename is NULL, we use a dummy file name.
	 */
	if(filename == NULL)
	    filename = "(file written out to memory)";
        
	/*
	 * The time the table of contents' are set to and the time to base the
	 * modification time of the output file to be set to.
	 */
	*seen_archive = FALSE;
	/*
	 * The environment variable ZERO_AR_DATE is used here and other
	 * places that write archives to allow testing and comparing
	 * things for exact binary equality.
	 */
	if (getenv("ZERO_AR_DATE") != NULL)
	    toc_time = 0;
	else
	    toc_time = time(0) + 5;

	fat_arch = NULL; /* here to quite compiler maybe warning message */
	fat_arch64 = NULL;
	fat_header = NULL;

	if(narchs == 0){
	    error("no contents for file: %s (not created)", filename);
	    return;
	}

	host_byte_sex = get_host_byte_sex();

	/*
	 * Calculate the total size of the file and the final size of each
	 * architecture.
	 */
	if(narchs > 1 ||
	   archs[0].fat_arch != NULL || archs[0].fat_arch64 != NULL){
	    file_size = sizeof(struct fat_header);
	    if(archs[0].fat_arch64 != NULL)
		file_size += sizeof(struct fat_arch_64) * narchs;
	    else
		file_size += sizeof(struct fat_arch) * narchs;
	}
	else
	    file_size = 0;
	for(i = 0; i < narchs; i++){
	    /*
	     * For each arch that is an archive recreate the table of contents.
	     *
	     * Remember, for historical reasons the table of contents is
	     * time-shifted 5 seconds into the future. This 5 second offset is
	     * not applied when zeroing out ar dates.
	     */
	    if(archs[i].type == OFILE_ARCHIVE){
		*seen_archive = TRUE;
		make_table_of_contents(archs + i, filename, toc_time, sort_toc,
				       commons_in_toc, force_64bit_toc,
				       library_warnings);
		archs[i].library_size += SARMAG + archs[i].toc_size;
		if(archs[i].fat_arch64 != NULL)
		    file_size = rnd(file_size, 1 << archs[i].fat_arch64->align);
		else if(archs[i].fat_arch != NULL)
		    file_size = rnd(file_size, 1 << archs[i].fat_arch->align);
		file_size += archs[i].library_size;
		if(archs[i].fat_arch64 != NULL)
		    archs[i].fat_arch64->size = archs[i].library_size;
		else if(archs[i].fat_arch != NULL) {
		    if (archs[i].library_size > 0xFFFFFFFF) {
			error("file too large to create as a fat file because "
			      "size field in struct fat_arch is only 32-bits "
			      "and library_size (%llu) of file %s exceeds that",
			      archs[i].unknown_size, archs[i].file_name);
			return;
		    }
		    archs[i].fat_arch->size = (uint32_t)archs[i].library_size;
		}
	    }
	    else if(archs[i].type == OFILE_Mach_O){
		size = archs[i].object->object_size
		       - archs[i].object->input_sym_info_size
		       + archs[i].object->output_new_content_size
		       + archs[i].object->output_sym_info_size;
		if(archs[i].fat_arch64 != NULL)
		    file_size = rnd(file_size, 1 << archs[i].fat_arch64->align);
		else if(archs[i].fat_arch != NULL)
		    file_size = rnd(file_size, 1 << archs[i].fat_arch->align);
		file_size += size;
		if(archs[i].fat_arch64 != NULL)
		    archs[i].fat_arch64->size = size;
		else if(archs[i].fat_arch != NULL)
		    archs[i].fat_arch->size = size;
	    }
	    else{ /* archs[i].type == OFILE_UNKNOWN */
		if(archs[i].fat_arch64 != NULL)
		    file_size = rnd(file_size, 1 << archs[i].fat_arch64->align);
		else if(archs[i].fat_arch != NULL)
		    file_size = rnd(file_size, 1 << archs[i].fat_arch->align);
		file_size += archs[i].unknown_size;
		if(archs[i].fat_arch64 != NULL)
		    archs[i].fat_arch64->size = archs[i].unknown_size;
		else if(archs[i].fat_arch != NULL) {
		    if (archs[i].unknown_size > 0xFFFFFFFF) {
			error("file too large to create as a fat file because "
			      "size field in struct fat_arch is only 32-bits "
			      "and unknown_size (%llu) of file %s exceeds that",
			      archs[i].unknown_size, archs[i].file_name);
			return;
		    }
		    archs[i].fat_arch->size = (uint32_t)archs[i].unknown_size;
		}
	    }
	}

	/*
	 * This buffer is vm_allocate'ed to make sure all holes are filled with
	 * zero bytes.
	 */
	if((r = vm_allocate(mach_task_self(), (vm_address_t *)&file,
			    file_size, TRUE)) != KERN_SUCCESS)
	    mach_fatal(r, "can't vm_allocate() buffer for output file: %s of "
		       "size %llu", filename, file_size);

	/*
	 * If there is more than one architecture then fill in the fat file
	 * header and the fat_arch or fat_arch64 structures in the buffer.
	 */
	if(narchs > 1 ||
	   archs[0].fat_arch != NULL || archs[0].fat_arch64 != NULL){
	    fat_header = (struct fat_header *)file;
	    if(archs[0].fat_arch64 != NULL)
		fat_header->magic = FAT_MAGIC_64;
	    else
		fat_header->magic = FAT_MAGIC;
	    fat_header->nfat_arch = narchs;
	    offset = sizeof(struct fat_header);
	    if(archs[0].fat_arch64 != NULL){
		offset += sizeof(struct fat_arch_64) * narchs;
		fat_arch64 = (struct fat_arch_64 *)
			     (file + sizeof(struct fat_header));
		fat_arch = NULL;
	    }
	    else{
		offset += sizeof(struct fat_arch) * narchs;
		fat_arch = (struct fat_arch *)
			   (file + sizeof(struct fat_header));
		fat_arch64 = NULL;
	    }
	    for(i = 0; i < narchs; i++){
		if(archs[i].fat_arch64 != NULL){
		    fat_arch64[i].cputype = archs[i].fat_arch64->cputype;
		    fat_arch64[i].cpusubtype = archs[i].fat_arch64->cpusubtype;
		}
		else{
		    fat_arch[i].cputype = archs[i].fat_arch->cputype;
		    fat_arch[i].cpusubtype = archs[i].fat_arch->cpusubtype;
		}
		if(offset > UINT32_MAX && archs[i].fat_arch64 == NULL){
		    error("file too large to create as a fat file because "
			  "offset field in struct fat_arch is only 32-bits and "
			  "offset (%llu) to architecture %s exceeds that",
			  offset, archs[i].fat_arch_name);
		    return;
		}
		if(archs[i].fat_arch64 != NULL){
		    offset = rnd(offset, 1 << archs[i].fat_arch64->align);
		    fat_arch64[i].offset = offset;
		    fat_arch64[i].size = archs[i].fat_arch64->size;
		    fat_arch64[i].align = archs[i].fat_arch64->align;
		    offset += archs[i].fat_arch64->size;
		}
		else{
		    offset = rnd(offset, 1 << archs[i].fat_arch->align);
		    fat_arch[i].offset = (uint32_t)offset;
		    fat_arch[i].size = archs[i].fat_arch->size;
		    fat_arch[i].align = archs[i].fat_arch->align;
		    offset += archs[i].fat_arch->size;
		}
	    }
	}

	/*
	 * Now put each arch in the buffer.
	 */
	for(i = 0; i < narchs; i++){
	    if(archs[i].fat_arch != NULL || archs[i].fat_arch64 != NULL){
		if(archs[i].fat_arch64 != NULL)
		    p = file + fat_arch64[i].offset;
		else
		    p = file + fat_arch[i].offset;
	    }
	    else
		p = file;

	    if(archs[i].type == OFILE_ARCHIVE){
		*seen_archive = TRUE;
		/*
		 * If the input files only contains non-object files then the
		 * byte sex of the output can't be determined which is needed
		 * for the two binary long's of the table of contents.  But
		 * since these will be zero (the same in both byte sexes)
		 * because there are no symbols in the table of contents if
		 * there are no object files.
		 */

		/* put in the archive magic string */
		memcpy(p, ARMAG, SARMAG);
		p += SARMAG;

		/*
		 * Warn for what really is a bad library that has an empty
		 * table of contents but this is allowed in the original
		 * bsd4.3 ranlib(1) implementation.
		 */
		if(library_warnings == TRUE && archs[i].ntocs == 0){
		    if(narchs > 1 ||
		       archs[i].fat_arch != NULL || archs[i].fat_arch64 != NULL)
			warning("warning library: %s for architecture: %s the "
			        "table of contents is empty (no object file "
			        "members in the library)", filename,
			         archs[i].fat_arch_name);
		    else
			warning("warning for library: %s the table of contents "
				"is empty (no object file members in the "
				"library)", filename);
		}

		/*
		 * Pick the byte sex to write the table of contents in.
		 */
		target_byte_sex = UNKNOWN_BYTE_SEX;
		for(j = 0;
		    j < archs[i].nmembers && target_byte_sex ==UNKNOWN_BYTE_SEX;
		    j++){
		    if(archs[i].members[j].type == OFILE_Mach_O)
			target_byte_sex =
				archs[i].members[j].object->object_byte_sex;
		}
		if(target_byte_sex == UNKNOWN_BYTE_SEX)
		    target_byte_sex = host_byte_sex;

		/*
		 * Put in the table of contents member, which can be either a
		 * 32-bit toc or a 64-bit toc:
		 * Both forms start with:
		 *   the archive header
		 *   the archive member name (if using a long name)
		 * then for a 32-bit toc the rest is this:
		 *   a 32-bit for the number of bytes of the ranlib structs
		 *   the ranlib structs
		 *   a 32-bit for the number of bytes of the ranlib strings
		 *   the strings for the ranlib structs
		 * and for a 64-bit toc the rest is this:
		 *   a 64-bit for the number of bytes of the ranlib_64 structs
		 *   the ranlib_64 structs
		 *   a 64-bit for the number of bytes of the ranlib strings
		 *   the strings for the ranlib structs
		 */
		memcpy(p, (char *)(&archs[i].toc_ar_hdr),sizeof(struct ar_hdr));
		p += sizeof(struct ar_hdr);

		if(archs[i].toc_long_name == TRUE){
		    memcpy(p, archs[i].toc_name, archs[i].toc_name_size);
		    p += archs[i].toc_name_size +
			 (rnd(sizeof(struct ar_hdr), 8) -
			  sizeof(struct ar_hdr));
		}

		if(archs[i].using_64toc == FALSE){
		    if (archs[i].ntocs > 0xFFFFFFFF) {
			error("file too large to create because there are more "
			      "than 2^32 entries (%llu) in the toc for file %s",
			      archs[i].ntocs, archs[i].file_name);
			return;
		    }
		    if (archs[i].ntocs * sizeof(struct ranlib) > 0xFFFFFFFF) {
			error("file too large to create because the library "
			      "toc size is only 32-bits and the required toc "
			      "size (%llu) for file %s exceeds that",
			      archs[i].ntocs * sizeof(struct ranlib),
			      archs[i].file_name);
			return;
		    }
		    if (archs[i].toc_strsize > 0xFFFFFFFF) {
			error("file too large to create because toc symbol "
			      "name length is only 32-bits and the required "
			      "toc symbol name length (%llu) for file %s "
			      "exceeds that",
			      archs[i].toc_strsize, archs[i].file_name);
			return;
		    }
		    i32 = (uint32_t)(archs[i].ntocs * sizeof(struct ranlib));
		    if(target_byte_sex != host_byte_sex)
			i32 = SWAP_INT(i32);
		    memcpy(p, (char *)&i32, sizeof(uint32_t));
		    p += sizeof(uint32_t);

		    if(target_byte_sex != host_byte_sex)
			swap_ranlib(archs[i].toc_ranlibs,
				    (uint32_t)archs[i].ntocs,
				    target_byte_sex);
		    memcpy(p, (char *)archs[i].toc_ranlibs,
			   archs[i].ntocs * sizeof(struct ranlib));
		    p += archs[i].ntocs * sizeof(struct ranlib);

		    i32 = (uint32_t)archs[i].toc_strsize;
		    if(target_byte_sex != host_byte_sex)
			i32 = SWAP_INT(i32);
		    memcpy(p, (char *)&i32, sizeof(uint32_t));
		    p += sizeof(uint32_t);
		}
		else{
		    i64 = archs[i].ntocs * sizeof(struct ranlib_64);
		    if(target_byte_sex != host_byte_sex)
			i64 = SWAP_LONG_LONG(i64);
		    memcpy(p, (char *)&i64, sizeof(uint64_t));
		    p += sizeof(uint64_t);

		    if(target_byte_sex != host_byte_sex)
			swap_ranlib_64(archs[i].toc_ranlibs64, archs[i].ntocs,
				       target_byte_sex);
		    memcpy(p, (char *)archs[i].toc_ranlibs64,
			   archs[i].ntocs * sizeof(struct ranlib_64));
		    p += archs[i].ntocs * sizeof(struct ranlib_64);

		    i64 = archs[i].toc_strsize;
		    if(target_byte_sex != host_byte_sex)
			i64 = SWAP_LONG_LONG(i64);
		    memcpy(p, (char *)&i64, sizeof(uint64_t));
		    p += sizeof(uint64_t);
		}

		memcpy(p, (char *)archs[i].toc_strings, archs[i].toc_strsize);
		p += archs[i].toc_strsize;

		/*
		 * Put in the archive header and member contents for each
		 * member in the buffer.
		 */
		for(j = 0; j < archs[i].nmembers; j++){
		    memcpy(p, (char *)(archs[i].members[j].ar_hdr),
			   sizeof(struct ar_hdr));
		    p += sizeof(struct ar_hdr);

		    if(archs[i].members[j].member_long_name == TRUE){
			memcpy(p, archs[i].members[j].member_name,
			       archs[i].members[j].member_name_size);
			p += rnd(archs[i].members[j].member_name_size, 8) +
				   (rnd(sizeof(struct ar_hdr), 8) -
				    sizeof(struct ar_hdr));
		    }

		    if(archs[i].members[j].type == OFILE_Mach_O){
			/*
			 * ofile_map swaps the headers to the host_byte_sex if
			 * the object's byte sex is not the same as the host
			 * byte sex so if this is the case swap them back
			 * before writing them out.
			 */
			memset(&dyst, '\0', sizeof(struct dysymtab_command));
			if(archs[i].members[j].object->dyst != NULL)
			    dyst = *(archs[i].members[j].object->dyst);
			if(archs[i].members[j].object->hints_cmd != NULL)
			   hints_cmd = *(archs[i].members[j].object->hints_cmd);
			if(archs[i].members[j].object->object_byte_sex !=
								host_byte_sex){
			    if(archs[i].members[j].object->mh != NULL){
				if(swap_object_headers(
				    archs[i].members[j].object->mh,
				    archs[i].members[j].object->load_commands)
				    == FALSE)
					fatal("internal error: "
					    "swap_object_headers() failed");
				if(archs[i].members[j].object->output_nsymbols
				   != 0)
				    swap_nlist(
				       archs[i].members[j].object->
					   output_symbols,
				       archs[i].members[j].object->
					   output_nsymbols,
				       archs[i].members[j].object->
					   object_byte_sex);
			    }
			    else{
				if(swap_object_headers(
				    archs[i].members[j].object->mh64,
				    archs[i].members[j].object->load_commands)
				    == FALSE)
					fatal("internal error: "
					    "swap_object_headers() failed");
				if(archs[i].members[j].object->output_nsymbols
				   != 0)
				    swap_nlist_64(
				       archs[i].members[j].object->
					   output_symbols64,
				       archs[i].members[j].object->
					   output_nsymbols,
				       archs[i].members[j].object->
					   object_byte_sex);
			    }
			}
			if(archs[i].members[j].object->
				output_sym_info_size == 0 &&
			   archs[i].members[j].object->
				input_sym_info_size == 0){
			    size = archs[i].members[j].object->object_size;
			    memcpy(p, archs[i].members[j].object->object_addr,
				   size);
			}
			else{
			    size = archs[i].members[j].object->object_size
				   - archs[i].members[j].object->
							input_sym_info_size;
			    memcpy(p, archs[i].members[j].object->object_addr,
				   size);
			    copy_new_symbol_info(p, &size, &dyst,
				archs[i].members[j].object->dyst, &hints_cmd,
				archs[i].members[j].object->hints_cmd,
				archs[i].members[j].object);
			}
			p += size;
			pad = (uint32_t)rnd(size, 8) - size;
		    }
		    else{
			memcpy(p, archs[i].members[j].unknown_addr, 
			       archs[i].members[j].unknown_size);
			p += archs[i].members[j].unknown_size;
			pad = (uint32_t)
			    (rnd(archs[i].members[j].unknown_size, 8) -
			     archs[i].members[j].unknown_size);
		    }
		    /* as with the UNIX ar(1) program pad with '\n' chars */
		    for(k = 0; k < pad; k++)
			*p++ = '\n';
		}
	    }
	    else if(archs[i].type == OFILE_Mach_O){
		memset(&dyst, '\0', sizeof(struct dysymtab_command));
		if(archs[i].object->dyst != NULL)
		    dyst = *(archs[i].object->dyst);
		if(archs[i].object->hints_cmd != NULL)
		    hints_cmd = *(archs[i].object->hints_cmd);
		if(archs[i].object->mh_filetype == MH_DYLIB){
		    /*
		     * To avoid problems with prebinding and multiple
		     * cpusubtypes we stager the time stamps of fat dylibs
		     * that have more than one cpusubtype.
		     */
		    timestamp = 0;
		    for(index = i - 1; timestamp == 0 && index >= 0; index--){
			if(archs[index].type == OFILE_Mach_O &&
			   archs[index].object->mh_filetype == MH_DYLIB &&
			   archs[index].object->mh_cputype ==
				archs[i].object->mh_cputype){
			    if(archs[index].object->mh != NULL)
				ncmds = archs[index].object->mh->ncmds;
			    else
				ncmds = archs[index].object->mh64->ncmds;
			    lcp = archs[index].object->load_commands;
			    swapped = archs[index].object->object_byte_sex !=
			              host_byte_sex;
			    if(swapped)
				ncmds = SWAP_INT(ncmds);
			    for(j = 0; j < ncmds; j++){
				lc = *lcp;
				if(swapped)
				    swap_load_command(&lc, host_byte_sex);
				if(lc.cmd == LC_ID_DYLIB){
				    dlp = (struct dylib_command *)lcp;
				    dl = *dlp;
				    if(swapped)
					swap_dylib_command(&dl, host_byte_sex);
				    timestamp = dl.dylib.timestamp - 1;
				    break;
				}
				lcp = (struct load_command *)
				      ((char *)lcp + lc.cmdsize);
			    }
			}
		    }
		    if(timestamp == 0)
			timestamp = (uint32_t)toc_time;
		    lcp = archs[i].object->load_commands;
		    if(archs[i].object->mh != NULL)
			ncmds = archs[i].object->mh->ncmds;
		    else
			ncmds = archs[i].object->mh64->ncmds;
		    for(j = 0; j < ncmds; j++){
			if(lcp->cmd == LC_ID_DYLIB){
			    dlp = (struct dylib_command *)lcp;
			    if(archs[i].dont_update_LC_ID_DYLIB_timestamp ==
			       FALSE)
				dlp->dylib.timestamp = timestamp;
			    break;
			}
			lcp = (struct load_command *)((char *)lcp +
						      lcp->cmdsize);
		    }
		}
		if(archs[i].object->object_byte_sex != host_byte_sex){
		    if(archs[i].object->mh != NULL){
			if(swap_object_headers(archs[i].object->mh,
				   archs[i].object->load_commands) == FALSE)
			    fatal("internal error: swap_object_headers() "
				  "failed");
			if(archs[i].object->output_nsymbols != 0)
			    swap_nlist(archs[i].object->output_symbols,
				       archs[i].object->output_nsymbols,
				       archs[i].object->object_byte_sex);
		    }
		    else{
			if(swap_object_headers(archs[i].object->mh64,
				   archs[i].object->load_commands) == FALSE)
			    fatal("internal error: swap_object_headers() "
				  "failed");
			if(archs[i].object->output_nsymbols != 0)
			    swap_nlist_64(archs[i].object->output_symbols64,
					  archs[i].object->output_nsymbols,
					  archs[i].object->object_byte_sex);
		    }
		}
		if(archs[i].object->output_sym_info_size == 0 &&
		   archs[i].object->input_sym_info_size == 0){
		    size = archs[i].object->object_size;
		    memcpy(p, archs[i].object->object_addr, size);
		}
		else{
		    size = archs[i].object->object_size
			   - archs[i].object->input_sym_info_size;
		    memcpy(p, archs[i].object->object_addr, size);
		    if(archs[i].object->output_new_content_size != 0){
			memcpy(p + size, archs[i].object->output_new_content,
			       archs[i].object->output_new_content_size);
			size += archs[i].object->output_new_content_size;
		    }
		    copy_new_symbol_info(p, &size, &dyst,
				archs[i].object->dyst, &hints_cmd,
				archs[i].object->hints_cmd,
				archs[i].object);
		}
	    }
	    else{ /* archs[i].type == OFILE_UNKNOWN */
		memcpy(p, archs[i].unknown_addr, archs[i].unknown_size);
	    }
	}
#ifdef __LITTLE_ENDIAN__
	if(narchs > 1 ||
	   archs[0].fat_arch != NULL || archs[0].fat_arch64 != NULL){
	    swap_fat_header(fat_header, BIG_ENDIAN_BYTE_SEX);
	    if(archs[0].fat_arch64 != NULL)
		swap_fat_arch_64(fat_arch64, narchs, BIG_ENDIAN_BYTE_SEX);
	    else
		swap_fat_arch(fat_arch, narchs, BIG_ENDIAN_BYTE_SEX);
	}
#endif /* __LITTLE_ENDIAN__ */
        *outputbuf = file;
        *length = file_size;
}

/*
 * copy_new_symbol_info() copies the new and updated symbolic information into
 * the buffer for the object.
 */
static
void
copy_new_symbol_info(
char *p,
uint32_t *size,
struct dysymtab_command *dyst,
struct dysymtab_command *old_dyst,
struct twolevel_hints_command *hints_cmd,
struct twolevel_hints_command *old_hints_cmd,
struct object *object)
{
	if(old_dyst != NULL){
	    if(object->output_dyld_info_size != 0){
		if(object->output_dyld_info != NULL)
		    memcpy(p + *size, object->output_dyld_info,
			   object->output_dyld_info_size);
		*size += object->output_dyld_info_size;
	    }
	    if(object->output_dyld_chained_fixups_data_size != 0){
		if(object->output_dyld_chained_fixups_data != NULL)
		    memcpy(p + *size, object->output_dyld_chained_fixups_data,
			   object->output_dyld_chained_fixups_data_size);
		*size += object->output_dyld_chained_fixups_data_size;
	    }
	    if(object->output_dyld_exports_trie_data_size != 0){
		if(object->output_dyld_exports_trie_data != NULL)
		    memcpy(p + *size, object->output_dyld_exports_trie_data,
			   object->output_dyld_exports_trie_data_size);
		*size += object->output_dyld_exports_trie_data_size;
	    }
	    memcpy(p + *size, object->output_loc_relocs,
		   dyst->nlocrel * sizeof(struct relocation_info));
	    *size += dyst->nlocrel *
		     sizeof(struct relocation_info);
	    if(object->output_split_info_data_size != 0){
		if(object->output_split_info_data != NULL)
		    memcpy(p + *size, object->output_split_info_data,
			   object->output_split_info_data_size);
		*size += object->output_split_info_data_size;
	    }
	    if(object->output_func_start_info_data_size != 0){
		if(object->output_func_start_info_data != NULL)
		    memcpy(p + *size, object->output_func_start_info_data,
			   object->output_func_start_info_data_size);
		*size += object->output_func_start_info_data_size;
	    }
	    if(object->output_data_in_code_info_data_size != 0){
		if(object->output_data_in_code_info_data != NULL)
		    memcpy(p + *size, object->output_data_in_code_info_data,
			   object->output_data_in_code_info_data_size);
		*size += object->output_data_in_code_info_data_size;
	    }
	    if(object->output_code_sign_drs_info_data_size != 0){
		if(object->output_code_sign_drs_info_data != NULL)
		    memcpy(p + *size, object->output_code_sign_drs_info_data,
			   object->output_code_sign_drs_info_data_size);
		*size += object->output_code_sign_drs_info_data_size;
	    }
	    if(object->output_link_opt_hint_info_data_size != 0){
		if(object->output_link_opt_hint_info_data != NULL)
		    memcpy(p + *size, object->output_link_opt_hint_info_data,
			   object->output_link_opt_hint_info_data_size);
		*size += object->output_link_opt_hint_info_data_size;
	    }
	    if(object->mh != NULL){
		memcpy(p + *size, object->output_symbols,
		       object->output_nsymbols * sizeof(struct nlist));
		*size += object->output_nsymbols *
			 sizeof(struct nlist);
	    }
	    else{
		memcpy(p + *size, object->output_symbols64,
		       object->output_nsymbols * sizeof(struct nlist_64));
		*size += object->output_nsymbols *
			 sizeof(struct nlist_64);
	    }
	    if(old_hints_cmd != NULL){
		memcpy(p + *size, object->output_hints,
		       hints_cmd->nhints * sizeof(struct twolevel_hint));
		*size += hints_cmd->nhints *
			 sizeof(struct twolevel_hint);
	    }
	    memcpy(p + *size, object->output_ext_relocs,
		   dyst->nextrel * sizeof(struct relocation_info));
	    *size += dyst->nextrel *
		     sizeof(struct relocation_info);
	    memcpy(p + *size, object->output_indirect_symtab,
		   dyst->nindirectsyms * sizeof(uint32_t));
	    *size += dyst->nindirectsyms * sizeof(uint32_t) +
		     object->input_indirectsym_pad;
	    memcpy(p + *size, object->output_tocs,
		   object->output_ntoc *sizeof(struct dylib_table_of_contents));
	    *size += object->output_ntoc *
		     sizeof(struct dylib_table_of_contents);
	    if(object->mh != NULL){
		memcpy(p + *size, object->output_mods,
		       object->output_nmodtab * sizeof(struct dylib_module));
		*size += object->output_nmodtab *
			 sizeof(struct dylib_module);
	    }
	    else{
		memcpy(p + *size, object->output_mods64,
		       object->output_nmodtab * sizeof(struct dylib_module_64));
		*size += object->output_nmodtab *
			 sizeof(struct dylib_module_64);
	    }
	    memcpy(p + *size, object->output_refs,
		   object->output_nextrefsyms * sizeof(struct dylib_reference));
	    *size += object->output_nextrefsyms *
		     sizeof(struct dylib_reference);
	    memcpy(p + *size, object->output_strings,
		   object->output_strings_size);
	    *size += object->output_strings_size;
	    memset(p + *size, '\0', object->output_strings_size_pad);
	    *size += object->output_strings_size_pad;
	    if(object->output_code_sig_data_size != 0){
		*size = (uint32_t)rnd(*size, 16);
		if(object->output_code_sig_data != NULL)
		    memcpy(p + *size, object->output_code_sig_data,
			   object->output_code_sig_data_size);
		*size += object->output_code_sig_data_size;
	    }
	}
	else{
	    if(object->output_func_start_info_data_size != 0){
		if(object->output_func_start_info_data != NULL)
		    memcpy(p + *size, object->output_func_start_info_data,
			   object->output_func_start_info_data_size);
		*size += object->output_func_start_info_data_size;
	    }
	    if(object->output_data_in_code_info_data_size != 0){
		if(object->output_data_in_code_info_data != NULL)
		    memcpy(p + *size, object->output_data_in_code_info_data,
			   object->output_data_in_code_info_data_size);
		*size += object->output_data_in_code_info_data_size;
	    }
	    if(object->output_link_opt_hint_info_data_size != 0){
		if(object->output_link_opt_hint_info_data != NULL)
		    memcpy(p + *size, object->output_link_opt_hint_info_data,
			   object->output_link_opt_hint_info_data_size);
		*size += object->output_link_opt_hint_info_data_size;
	    }
	    if(object->mh != NULL){
		memcpy(p + *size, object->output_symbols,
		       object->output_nsymbols * sizeof(struct nlist));
		*size += object->output_nsymbols *
			 sizeof(struct nlist);
	    }
	    else{
		memcpy(p + *size, object->output_symbols64,
		       object->output_nsymbols * sizeof(struct nlist_64));
		*size += object->output_nsymbols *
			 sizeof(struct nlist_64);
	    }
	    memcpy(p + *size, object->output_strings,
		   object->output_strings_size);
	    *size += object->output_strings_size;
	    memset(p + *size, '\0', object->output_strings_size_pad);
	    *size += object->output_strings_size_pad;
	    if(object->output_code_sig_data_size != 0){
		*size = (uint32_t)rnd(*size, 16);
		if(object->output_code_sig_data != NULL)
		    memcpy(p + *size, object->output_code_sig_data,
			   object->output_code_sig_data_size);
		*size += object->output_code_sig_data_size;
	    }
	}
}

/*
 * make_table_of_contents() make the table of contents for the specified arch
 * and fills in the toc_* fields in the arch.  Output is the name of the output
 * file for error messages.
 */
static
void
make_table_of_contents(
struct arch *arch,
char *output,
time_t toc_time,
enum bool sort_toc,
enum bool commons_in_toc,
enum bool force_64bit_toc,
enum bool library_warnings)
{
    uint32_t i, j, k, r, s, nsects;
    struct member *member;
    struct object *object;
    struct load_command *lc;
    struct segment_command *sg;
    struct segment_command_64 *sg64;
    struct nlist *symbols;
    struct nlist_64 *symbols64;
    uint32_t nsymbols;
    char *strings;
    uint32_t strings_size;
    enum bool sorted;
    unsigned short toc_mode;
    int oumask, numask;
    char *ar_name;
    struct section *section;
    struct section_64 *section64;
    uint32_t ncmds;

	symbols = NULL; /* here to quite compiler maybe warning message */
	symbols64 = NULL;
	strings = NULL; /* here to quite compiler maybe warning message */

	/*
	 * First pass over the members to count how many ranlib structs are
	 * needed and the size of the strings in the toc that are needed.
	 */
	for(i = 0; i < arch->nmembers; i++){
	    member = arch->members + i;
	    if(member->type == OFILE_Mach_O){
		object = member->object;
		nsymbols = 0;
		nsects = 0;
		lc = object->load_commands;
		if(object->mh != NULL)
		    ncmds = object->mh->ncmds;
		else
		    ncmds = object->mh64->ncmds;
		for(j = 0; j < ncmds; j++){
		    if(lc->cmd == LC_SEGMENT){
			sg = (struct segment_command *)lc;
			nsects += sg->nsects;
		    }
		    else if(lc->cmd == LC_SEGMENT_64){
			sg64 = (struct segment_command_64 *)lc;
			nsects += sg64->nsects;
		    }
		    lc = (struct load_command *)((char *)lc + lc->cmdsize);
		}
		if(object->mh != NULL){
		    object->sections = allocate(nsects *
						sizeof(struct section *));
		    object->sections64 = NULL;
		}
		else{
		    object->sections = NULL;
		    object->sections64 = allocate(nsects *
						sizeof(struct section_64 *));
		}
		nsects = 0;
		lc = object->load_commands;
		for(j = 0; j < ncmds; j++){
		    if(lc->cmd == LC_SEGMENT){
			sg = (struct segment_command *)lc;
			section = (struct section *)
			    ((char *)sg + sizeof(struct segment_command));
			for(k = 0; k < sg->nsects; k++){
			    object->sections[nsects++] = section++;
			}
		    }
		    else if(lc->cmd == LC_SEGMENT_64){
			sg64 = (struct segment_command_64 *)lc;
			section64 = (struct section_64 *)
			    ((char *)sg64 + sizeof(struct segment_command_64));
			for(k = 0; k < sg64->nsects; k++){
			    object->sections64[nsects++] = section64++;
			}
		    }
		    lc = (struct load_command *)((char *)lc + lc->cmdsize);
		}
		if(object->output_sym_info_size == 0){
		    lc = object->load_commands;
		    for(j = 0; j < ncmds; j++){
			if(lc->cmd == LC_SYMTAB){
			    object->st = (struct symtab_command *)lc;
			    break;
			}
			lc = (struct load_command *)((char *)lc + lc->cmdsize);
		    }
		    if(object->st != NULL && object->st->nsyms != 0){
			if(object->mh != NULL){
			    symbols = (struct nlist *)(object->object_addr +
						       object->st->symoff);
			    if(object->object_byte_sex != get_host_byte_sex())
				swap_nlist(symbols, object->st->nsyms,
					   get_host_byte_sex());
			}
			else{
			    symbols64 = (struct nlist_64 *)
				(object->object_addr + object->st->symoff);
			    if(object->object_byte_sex != get_host_byte_sex())
				swap_nlist_64(symbols64, object->st->nsyms,
					      get_host_byte_sex());
			}
			nsymbols = object->st->nsyms;
			strings = object->object_addr + object->st->stroff;
			strings_size = object->st->strsize;
		    }
		}
		else /* object->output_sym_info_size != 0 */ {
		    if(object->mh != NULL)
			symbols = object->output_symbols;
		    else
			symbols64 = object->output_symbols64;
		    nsymbols = object->output_nsymbols;
		    strings = object->output_strings;
		    strings_size = object->output_strings_size;
		}
		for(j = 0; j < nsymbols; j++){
		    if(object->mh != NULL){
			if(toc_symbol(symbols + j, commons_in_toc,
			   object->sections) == TRUE){
			    arch->ntocs++;
			    arch->toc_strsize +=
				strlen(strings + symbols[j].n_un.n_strx) + 1;
			}
		    }
		    else{
			if(toc_symbol_64(symbols64 + j, commons_in_toc,
			   object->sections64) == TRUE){
			    arch->ntocs++;
			    arch->toc_strsize +=
				strlen(strings + symbols64[j].n_un.n_strx) + 1;
			}
		    }
		}
	    }
#ifdef LTO_SUPPORT
	    else if(member->type == OFILE_LLVM_BITCODE){
                nsymbols = lto_get_nsyms(member->lto);
                for(j = 0; j < nsymbols; j++){
                    if(lto_toc_symbol(member->lto, j, commons_in_toc) == TRUE){
			arch->ntocs++;
			arch->toc_strsize +=
                            strlen(lto_symbol_name(member->lto, j)) + 1;
                    }
                }
	    }
#endif /* LTO_SUPPORT */
	}

	/*
	 * Allocate the space for the table of content entries, the ranlib
	 * structs and strings for the table of contents.
	 */
	arch->toc_entries = allocate(sizeof(struct toc_entry) * arch->ntocs);
	arch->toc_ranlibs = allocate(sizeof(struct ranlib) * arch->ntocs);
	arch->toc_strsize = rnd(arch->toc_strsize, 8);
	arch->toc_strings = allocate(arch->toc_strsize);
	if(arch->toc_strsize >= 8)
	    memset(arch->toc_strings + arch->toc_strsize - 7, '\0', 7);

	/*
	 * Second pass over the members to fill in the toc_entry structs and
	 * the strings for the table of contents.  The symbol_name field is
	 * filled in with a pointer to a string contained in arch->toc_strings
	 * for easy sorting and conversion to an index.  The member_index field
         * is filled in with the member index plus one to allow marking with
	 * its negative value by check_sort_toc_entries() and easy conversion to
	 * the real offset.
	 */
	r = 0;
	s = 0;
	for(i = 0; i < arch->nmembers; i++){
	    member = arch->members + i;
	    if(member->type == OFILE_Mach_O){
		object = member->object;
		nsymbols = 0;
		if(object->output_sym_info_size == 0){
		    if(object->st != NULL){
			if(object->mh != NULL)
			    symbols = (struct nlist *)
				(object->object_addr + object->st->symoff);
			else
			    symbols64 = (struct nlist_64 *)
				(object->object_addr + object->st->symoff);
			nsymbols = object->st->nsyms;
			strings = object->object_addr + object->st->stroff;
			strings_size = object->st->strsize;
		    }
		    else{
			symbols = NULL;
			nsymbols = 0;
			strings = NULL;
			strings_size = 0;
		    }
		}
		else{
		    if(object->mh != NULL)
			symbols = object->output_symbols;
		    else
			symbols64 = object->output_symbols64;
		    nsymbols = object->output_nsymbols;
		    strings = object->output_strings;
		    strings_size = object->output_strings_size;
		}
		for(j = 0; j < nsymbols; j++){
		    if(object->mh != NULL){
			if((uint32_t)symbols[j].n_un.n_strx > strings_size)
			    continue;
			if(toc_symbol(symbols + j, commons_in_toc,
			   object->sections) == TRUE){
			    strcpy(arch->toc_strings + s, 
				   strings + symbols[j].n_un.n_strx);
			    arch->toc_entries[r].symbol_name =
							arch->toc_strings + s;
			    arch->toc_entries[r].member_index = i + 1;
			    r++;
			    s += strlen(strings + symbols[j].n_un.n_strx) + 1;
			}
		    }
		    else{
			if((uint32_t)symbols64[j].n_un.n_strx >
			   strings_size)
			    continue;
			if(toc_symbol_64(symbols64 + j, commons_in_toc,
			   object->sections64) == TRUE){
			    strcpy(arch->toc_strings + s, 
				   strings + symbols64[j].n_un.n_strx);
			    arch->toc_entries[r].symbol_name =
							arch->toc_strings + s;
			    arch->toc_entries[r].member_index = i + 1;
			    r++;
			    s += strlen(strings + symbols64[j].n_un.n_strx) + 1;
			}
		    }
		}
		if(object->output_sym_info_size == 0){
		    if(object->object_byte_sex != get_host_byte_sex()){
			if(object->mh != NULL)
			    swap_nlist(symbols, nsymbols,
				       object->object_byte_sex);
			else
			    swap_nlist_64(symbols64, nsymbols,
				          object->object_byte_sex);
		    }
		}
	    }
#ifdef LTO_SUPPORT
	    else if(member->type == OFILE_LLVM_BITCODE){
                nsymbols = lto_get_nsyms(member->lto);
                for(j = 0; j < nsymbols; j++){
                    if(lto_toc_symbol(member->lto, j, commons_in_toc) == TRUE){
			strcpy(arch->toc_strings + s, 
			       lto_symbol_name(member->lto, j));
			arch->toc_entries[r].symbol_name =
						    arch->toc_strings + s;
			arch->toc_entries[r].member_index = i + 1;
			r++;
			s += strlen(lto_symbol_name(member->lto, j)) + 1;
                    }
                }
	    }
#endif /* LTO_SUPPORT */
	}

	/*
	 * If the table of contents is to be sorted by symbol name then try to
	 * sort it and leave it sorted if no duplicates.
	 */
	if(sort_toc == TRUE){
	    qsort(arch->toc_entries, arch->ntocs, sizeof(struct toc_entry),
		  (int (*)(const void *, const void *))toc_entry_name_qsort);
	    sorted = check_sort_toc_entries(arch, output, library_warnings);
	    if(sorted == FALSE){
		qsort(arch->toc_entries, arch->ntocs, sizeof(struct toc_entry),
		      (int (*)(const void *, const void *))
		      toc_entry_index_qsort);
	    }
	}
	else{
	    sorted = FALSE;
	}

	/*
	 * Now set the ran_off and ran_un.ran_strx fields of the ranlib structs.
	 * To do this the size of the toc member must be know because it comes
	 * first in the library.  The size of the toc member is made up of the
	 * sizeof an archive header struct, plus the sizeof the name if we are
	 * using extended format #1 for the long name, then the toc which is
	 * (as defined in ranlib.h):
	 *	a 32-bit int for the number of bytes of the ranlib structs
	 *	the ranlib structures
	 *	a 32-bit int for the number of bytes of the strings
	 *	the strings
	 */
	/*
	 * We use a long name for the table of contents for both the sorted
	 * and non-sorted case because it is needed to get the 8 byte alignment
	 * of the first archive member by padding the long name since
	 * sizeof(struct ar_hdr) is not a mutiple of 8.
	 */
	if(arch->toc_long_name == FALSE)
	    fatal("internal error: make_table_of_contents() called with "
		  "arch->toc_long_name == FALSE");

	if(sorted == TRUE){
	    /*
	     * This assumes that "__.SYMDEF SORTED" is 16 bytes and
	     * (rnd(sizeof(struct ar_hdr), 8) - sizeof(struct ar_hdr)
	     * is 4 bytes.
	     */
	    ar_name = AR_EFMT1 "20";
	    arch->toc_name_size = sizeof(SYMDEF_SORTED) - 1;
	    arch->toc_name = SYMDEF_SORTED;
	}
	else{
	    /*
	     * This  assumes that "__.SYMDEF\0\0\0\0\0\0\0" is 16 bytes and
	     * (rnd(sizeof(struct ar_hdr), 8) - sizeof(struct ar_hdr)
	     * is 4 bytes.
	     */
	    ar_name = AR_EFMT1 "20";
	    arch->toc_name_size = 16;
	    arch->toc_name = SYMDEF "\0\0\0\0\0\0\0";
	}
	arch->toc_size = (uint32_t)
			(sizeof(struct ar_hdr) +
			sizeof(uint32_t) +
			arch->ntocs * sizeof(struct ranlib) +
			sizeof(uint32_t) +
			arch->toc_strsize);
	if(arch->toc_long_name == TRUE)
	    arch->toc_size += arch->toc_name_size +
			      (rnd(sizeof(struct ar_hdr), 8) -
			       sizeof(struct ar_hdr));
	/*
	 * Now with the size of the 32-bit toc known we can now see if it will
	 * work or if we have offsets to members that are more than 32-bits and
	 * we need to switch to the 64-bit toc, or switch to that if we are
	 * forcing a 64-bit toc.
	 */
	if(force_64bit_toc == TRUE)
	    arch->using_64toc = TRUE;
	else{
	    arch->using_64toc = FALSE;
	    for(i = 0; i < arch->nmembers; i++){
		if(arch->members[i].offset + SARMAG + arch->toc_size >
		   UINT32_MAX){
		    arch->using_64toc = TRUE;
		    break;
		}
	    }
	}
	if(arch->using_64toc){
	    /* it is already checked above that arch->toc_long_name == TRUE */
	    if(sorted == FALSE){
		/*
		 * This  assumes that "__.SYMDEF_64\0\0\0\0" is 16 bytes
		 * and
		 * (rnd(sizeof(struct ar_hdr), 8) - sizeof(struct ar_hdr)
		 * is 4 bytes.
		 */
		ar_name = AR_EFMT1 "20";
		arch->toc_name_size = 16;
		arch->toc_name = SYMDEF_64 "\0\0\0\0";
	    }
	    else{
		arch->toc_name = SYMDEF_64_SORTED;
		arch->toc_name_size = sizeof(SYMDEF_64_SORTED) - 1;
		/*
		 * This assumes that "__.SYMDEF_64 SORTED\0\0\0\0\0" is 24 bytes
		 * and
		 * (rnd(sizeof(struct ar_hdr), 8) - sizeof(struct ar_hdr)
		 * is 4 bytes.
		 */
		ar_name = AR_EFMT1 "28";
		arch->toc_name_size = 24;
		arch->toc_name = SYMDEF_64_SORTED "\0\0\0\0\0";
	    }
	    /*
	     * Free the space for the 32-bit ranlib structs and allocate space
	     * for the 64-bit ranlib structs.
	     */
	    free(arch->toc_ranlibs);
	    arch->toc_ranlibs = NULL;
	    arch->toc_ranlibs64 = allocate(sizeof(struct ranlib_64) *
				           arch->ntocs);
	    /*
	     * Now the size of the toc member when it is a 64-bit toc can be
	     * set.  It is made up of the sizeof an archive header struct (the
	     * size of the name which is always a long name to get 8-byte
	     * alignment then the toc which is (as defined in ranlib.h):
	     *   a uint64_t for the number of bytes of the ranlib_64 structs
	     *   the ranlib_64 structures
	     *   a uint64_t for the number of bytes of the strings
	     *   the strings
	     */
	    arch->toc_size = (uint32_t)
		    (sizeof(struct ar_hdr) +
		     sizeof(uint64_t) +
		     arch->ntocs * sizeof(struct ranlib_64) +
		     sizeof(uint64_t) +
		     arch->toc_strsize);
	    /* add the size of the name as a long name is always used */
	    arch->toc_size += arch->toc_name_size +
			      (rnd(sizeof(struct ar_hdr), 8) -
			       sizeof(struct ar_hdr));
	}

	for(i = 0; i < arch->nmembers; i++)
	    arch->members[i].offset += SARMAG + arch->toc_size;

	for(i = 0; i < arch->ntocs; i++){
	    if(arch->using_64toc){
		arch->toc_ranlibs64[i].ran_un.ran_strx =
		    arch->toc_entries[i].symbol_name - arch->toc_strings;
		arch->toc_ranlibs64[i].ran_off =
		    arch->members[arch->toc_entries[i].member_index - 1].offset;
	    }
	    else{
		arch->toc_ranlibs[i].ran_un.ran_strx = (uint32_t)
		    (arch->toc_entries[i].symbol_name - arch->toc_strings);
		arch->toc_ranlibs[i].ran_off = (uint32_t)
		    arch->members[arch->toc_entries[i].member_index - 1].offset;
	    }
	}

	numask = 0;
	oumask = umask(numask);
	toc_mode = S_IFREG | (0666 & ~oumask);
	(void)umask(oumask);

	sprintf((char *)(&arch->toc_ar_hdr), "%-*s%-*ld%-*u%-*u%-*o%-*ld",
	   (int)sizeof(arch->toc_ar_hdr.ar_name),
	       ar_name,
	   (int)sizeof(arch->toc_ar_hdr.ar_date),
	       toc_time,
	   (int)sizeof(arch->toc_ar_hdr.ar_uid),
	       (unsigned short)getuid(),
	   (int)sizeof(arch->toc_ar_hdr.ar_gid),
	       (unsigned short)getgid(),
	   (int)sizeof(arch->toc_ar_hdr.ar_mode),
	       (unsigned int)toc_mode,
	   (int)sizeof(arch->toc_ar_hdr.ar_size),
	       (long)(arch->toc_size - sizeof(struct ar_hdr)));
	/*
	 * This has to be done by hand because sprintf puts a null
	 * at the end of the buffer.
	 */
	memcpy(arch->toc_ar_hdr.ar_fmag, ARFMAG,
	       (int)sizeof(arch->toc_ar_hdr.ar_fmag));
}

/*
 * toc_symbol() returns TRUE if the symbol is to be included in the table of
 * contents otherwise it returns FALSE.
 */
static
enum bool
toc_symbol(
struct nlist *symbol,
enum bool commons_in_toc,
struct section **sections)
{
	return(toc(symbol->n_un.n_strx,
		   symbol->n_type,
		   symbol->n_value,
		   commons_in_toc,
		   (symbol->n_type & N_TYPE) == N_SECT &&
		       sections[symbol->n_sect - 1]->flags & S_ATTR_NO_TOC));
}

static
enum bool
toc_symbol_64(
struct nlist_64 *symbol64,
enum bool commons_in_toc,
struct section_64 **sections64)
{
	return(toc(symbol64->n_un.n_strx,
		   symbol64->n_type,
		   symbol64->n_value,
		   commons_in_toc,
		   (symbol64->n_type & N_TYPE) == N_SECT &&
		       sections64[symbol64->n_sect-1]->flags & S_ATTR_NO_TOC));
}

static
enum bool
toc(
uint32_t n_strx,
uint8_t n_type,
uint64_t n_value,
enum bool commons_in_toc,
enum bool attr_no_toc)
{
	/* if the name is NULL then it won't be in the table of contents */
	if(n_strx == 0)
	    return(FALSE);
	/* if symbol is not external then it won't be in the toc */
	if((n_type & N_EXT) == 0)
	    return(FALSE);
	/* if symbol is undefined then it won't be in the toc */
	if((n_type & N_TYPE) == N_UNDF && n_value == 0)
	    return(FALSE);
	/* if symbol is common and the commons are not to be in the toc */
	if((n_type & N_TYPE) == N_UNDF && n_value != 0 &&
	   commons_in_toc == FALSE)
	    return(FALSE);
	/* if the symbols is in a section marked NO_TOC then ... */
	if(attr_no_toc != 0)
	    return(FALSE);

	return(TRUE);
}

/*
 * Function for qsort() for comparing toc_entry structures by name.
 */
static
int
toc_entry_name_qsort(
const struct toc_entry *toc1,
const struct toc_entry *toc2)
{
	return(strcmp(toc1->symbol_name, toc2->symbol_name));
}

/*
 * Function for qsort() for comparing toc_entry structures by index.
 */
static
int
toc_entry_index_qsort(
const struct toc_entry *toc1,
const struct toc_entry *toc2)
{
	if(toc1->member_index < toc2->member_index)
	    return(-1);
	if(toc1->member_index > toc2->member_index)
	    return(1);
	/* toc1->member_index == toc2->member_index */
	    return(0);
}

/*
 * check_sort_toc_entries() checks the table of contents for the specified arch
 * which is sorted by name for more then one object defining the same symbol.
 * If this is the case it prints each symbol that is defined in more than one
 * object along with the object it is defined in.  It returns TRUE if there are
 * no multiple definitions and FALSE otherwise.
 */
static
enum bool
check_sort_toc_entries(
struct arch *arch,
char *output,
enum bool library_warnings)
{
    uint32_t i;
    enum bool multiple_defs;
    struct member *member;

	if(arch->ntocs == 0 || arch->ntocs == 1)
	    return(TRUE);

	/*
	 * Since the symbol table is sorted by name look to any two adjcent
	 * entries with the same name.  If such entries are found print them
	 * only once (marked by changing the sign of their member_index).
	 */
	multiple_defs = FALSE;
	for(i = 0; i < arch->ntocs - 1; i++){
	    if(strcmp(arch->toc_entries[i].symbol_name,
		      arch->toc_entries[i+1].symbol_name) == 0){
		if(multiple_defs == FALSE){
		    if(library_warnings == FALSE)
			return(FALSE);
		    fprintf(stderr, "%s: same symbol defined in more than one "
			    "member ", progname);
		    if(arch->fat_arch != NULL || arch->fat_arch64 != NULL)
			fprintf(stderr, "for architecture: %s ",
				arch->fat_arch_name);
		    fprintf(stderr, "in: %s (table of contents will not be "
			    "sorted)\n", output);
		    multiple_defs = TRUE;
		}
		if(arch->toc_entries[i].member_index > 0){
		    member = arch->members +
			     arch->toc_entries[i].member_index - 1;
		    warn_member(arch, member, "defines symbol: %s",
				arch->toc_entries[i].symbol_name);
		    arch->toc_entries[i].member_index =
				-(arch->toc_entries[i].member_index);
		}
		if(arch->toc_entries[i+1].member_index > 0){
		    member = arch->members +
			     arch->toc_entries[i+1].member_index - 1;
		    warn_member(arch, member, "defines symbol: %s",
				arch->toc_entries[i+1].symbol_name);
		    arch->toc_entries[i+1].member_index =
				-(arch->toc_entries[i+1].member_index);
		}
	    }
	}

	if(multiple_defs == FALSE)
	    return(TRUE);
	else{
	    for(i = 0; i < arch->ntocs; i++)
		if(arch->toc_entries[i].member_index < 0)
		    arch->toc_entries[i].member_index =
			-(arch->toc_entries[i].member_index);
	    return(FALSE);
	}
}

/*
 * warn_member() is like the error routines it prints the program name the
 * member name specified and message specified.
 */
static
void
warn_member(
struct arch *arch,
struct member *member,
const char *format, ...)
{
    va_list ap;

	fprintf(stderr, "%s: ", progname);
	if(arch->fat_arch != NULL || arch->fat_arch64 != NULL)
	    fprintf(stderr, "for architecture: %s ", arch->fat_arch_name);

	if(member->input_ar_hdr != NULL){
	    fprintf(stderr, "file: %s(%.*s) ", member->input_file_name,
		    (int)member->member_name_size, member->member_name);
	}
	else
	    fprintf(stderr, "file: %s ", member->input_file_name);

	va_start(ap, format);
	vfprintf(stderr, format, ap);
        fprintf(stderr, "\n");
	va_end(ap);
}
#endif /* !defined(RLD) */
