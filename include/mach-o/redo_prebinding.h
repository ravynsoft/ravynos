/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
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
#ifndef _MACH_O_REDO_PREBINDING_H_
#define _MACH_O_REDO_PREBINDING_H_

#define REDO_PREBINDING_VERSION 3
#include <mach/machine.h>
/*
 * For all APIs in this file the parameters program_name and error_message
 * are used the same.  For unrecoverable resource errors like being unable to
 * allocate memory each API prints a message to stderr precede with program_name
 * then calls exit(2) with the value EXIT_FAILURE.  If an API is unsuccessful
 * and if error_message pass to it is not NULL it is set to a malloc(3)'ed
 * buffer with a NULL terminated string with the error message.  For all APIs 
 * when they return they release all resources (memory, open file descriptors,
 * etc). 
 * 
 * The file_name parameter for these APIs may be of the form "foo(bar)" which is
 * NOT interpreted as an archive name and a member name in that archive.  As
 * these API deal with prebinding and prebound binaries ready for execution
 * can't be in archives.
 * 
 * If the executable_path parameter for these APIs is not NULL it is used for
 * any dependent library has a path that starts with "@executable_path". Then
 * "@executable_path" is replaced with executable_path. 
 * 
 * If the root_dir parameter is not NULL it is prepended to all the rooted
 * dependent library paths. 
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * dependent_libs() takes a file_name of a binary and returns a malloc(3)'ed
 * array of pointers (NULL terminated) to names (also malloc(3)'ed and '\0'
 * terminated names) of all the dependent libraries for that binary (not
 * recursive) for all of the architectures of that binary.  If successful
 * dependent_libs() returns a non NULL value (at minimum a pointer to one NULL
 * pointer). If unsuccessful dependent_libs() returns NULL.
 */ 
extern
char **
dependent_libs(
const char *file_name,
const char *program_name,
char **error_message);

/*
 * install_name() takes a file_name of a binary and returns a malloc(3)'ed
 * pointer to a NULL terminated string containing the install_name value for
 * the binary. If unsuccessful install_name() returns NULL.  In particular,
 * NULL is returned if the binary is not a dylib and there is no error_message
 * set.  If the all of the arch's are dylibs but all the install names don't
 * match NULL is returned and a error_message is set.  If some but not all of
 * the archs are dylibs NULL is returned and a error_message is set.
 */ 
extern
char *
install_name(
const char *file_name,
const char *program_name,
char **error_message);

/* return values for redo_prebinding() */
enum redo_prebinding_retval {
    REDO_PREBINDING_SUCCESS,
    REDO_PREBINDING_FAILURE,
    /* the following can only be returned if the parameter only_if_needed set */
    REDO_PREBINDING_NOT_NEEDED,
    REDO_PREBINDING_NOT_PREBOUND,
    REDO_PREBINDING_NEEDS_REBUILDING
};

/*
 * redo_prebinding() takes a file_name of a binary and redoes the prebinding on
 * it.  If output_file is not NULL the update file is written to output_file,
 * if not it is written to file_name.  If redo_prebinding() is successful it
 * returns REDO_PREBINDING_SUCCESS otherwise it returns REDO_PREBINDING_FAILURE.
 * If the parameter allow_missing_architectures is zero and not all
 * architectures can be updated it is not successful and nothing is done and
 * this returns REDO_PREBINDING_FAILURE.  If the parameter
 * allow_missing_architectures is non-zero then only problems with missing
 * architectures for the architecure of the cputype specified by 
 * allow_missing_architectures will cause this call to fail.  Other
 * architectures that could not be prebound due to missing architectures in
 * depending libraries will not have their prebinding updated but will not
 * cause this call to fail.
 * If the slide_to_address parameter is non-zero and the binary is a
 * dynamic library it is relocated to have that has its prefered address.  If
 * only_if_needed is non-zero the prebinding is checked first and only done if
 * needed.  The checking includes checking the prefered address against the
 * slide_to_address value if it is non-zero.  If only_if_needed is non-zero
 * and the prebinding does not have to be redone REDO_PREBINDING_NOT_NEEDED is
 * returned, if the binary is not prebound REDO_PREBINDING_NOT_PREBOUND is
 * returned and if the new load commands do not fit in the binary and it needs
 * to be rebuilt REDO_PREBINDING_NEEDS_REBUILDING is returned.
 * If zero_out_prebind_checksum is non-zero then the cksum field of the
 * LC_PREBIND_CKSUM load command (if any) is set to zero on output (this should
 * always be set by B&I tools and never set by the update_prebinding(1)
 * command).
 * If throttle is non-NULL it points to a value of the maximum bytes per second
 * to use for writting the output.  If the value is ULONG_MAX then the actual
 * bytes per second is returned indirectly through *throttle.
 */
extern 
enum redo_prebinding_retval
redo_prebinding(
const char *file_name,
const char *executable_path,
const char *root_dir,
const char *output_file,
const char *program_name,
char **error_message,
uint32_t slide_to_address,
int only_if_needed,
int zero_out_prebind_checksum,
cpu_type_t allow_missing_architectures,
uint32_t *throttle);


/* return values for needs_redo_prebinding() */
enum needs_redo_prebinding_retval {
    PREBINDING_UPTODATE,  /* a binary who's prebinding is up todate */
    PREBINDING_OUTOFDATE, /* a binary who's prebinding is out of date */
    NOT_PREBOUND,	  /* a binary, but not built prebound */
    NOT_PREBINDABLE,	  /* not a binary or statically linked,
			     prebinding does not apply */
    PREBINDING_UNKNOWN	  /* a binary who's prebinding can't be determined
			     because it is malformed, a library it depends
			     on is missing, etc. */
};

/*
 * needs_redo_prebinding() takes a file_name and determines if it is a binary
 * and if its prebinding is uptodate.  It returns one of the return values
 * above depending on the state of the binary and libraries. If the parameter
 * allow_missing_architectures is zero then the value returned is based on the
 * first architecture for fat files.  If the parameter
 * allow_missing_architectures is non-zero then the value returned is based on
 * the cputype specified by allow_missing_architectures.  If that architecture
 * is not present then PREBINDING_UPTODATE is returned.  If the parameter
 * expected_address is not zero and the binary is a dynamic library then the
 * library is checked to see if it is at the expected_address if not the
 * prebinding is assumed to be out of date and PREBINDING_OUTOFDATE is returned.
 */
extern
enum needs_redo_prebinding_retval
needs_redo_prebinding(
const char *file_name,
const char *executable_path,
const char *root_dir,
const char *program_name,
char **error_message,
uint32_t expected_address,
cpu_type_t allow_missing_architectures);


/*
 * unprebind() takes a file_name of a binary and resets or removes prebinding
 * information from it.  If inbuf is non-NULL, the memory pointed to by inbuf is
 * used as the input file contents.  Otherwise, the contents are loaded from 
 * the file at path file_name.  Even if inbuf is non-NULL, a file_name 
 * parameter should be specified for error reporting.  Similarly, if outbuf is 
 * non-NULL, upon return, outbuf will point to a buffer containing the 
 * unprebound binary and outlen will point to the length of the output buffer.  
 * This buffer is vm_allocate'd and therefore should be vm_deallocate'd when it 
 * is no longer needed.  If outbuf is NULL, and output_file is not NULL the 
 * update file is written to output_file, if outbuf is NULL and output_file is 
 * NULL, it is written to file_name.  
 * If unprebind() is successful it returns REDO_PREBINDING_SUCCESS otherwise it
 * returns REDO_PREBINDING_FAILURE If the binary is already unprebound (i.e. it
 * has the MH_PREBINDABLE flag set) then REDO_PREBINDING_NOT_NEEDED is returned.
 * If the binary is not prebound and not prebindable, 
 * REDO_PREBINDING_NOT_PREBOUND is returned.  If zero_checksum is non-zero then
 * the cksum field the LC_PREBIND_CKSUM load command (if any) is set to zero on
 * output, otherwise it is left alone.
 * Unprebinding slides dynamic libraries to address zero, resets prebound 
 * symbols to address zero and type undefined, resets symbol pointers, removes 
 * LC_PREBOUND_DYLIB commands, resets library timestamps, resets two-level hints
 * and updates relocation entries if necessary.  Unprebound binaries have
 * the MH_PREBINDABLE flag set, but not MH_PREBOUND.  It will also set the the
 * MH_ALLMODSBOUND flag if all two-level libraries were used and all modules
 * were found to be bound in the LC_PREBOUND_DYLIB commands.
 * As unprebinding is intended to produce a canonical Mach-O
 * binary, bundles and non-prebound executables and dylibs are acceptable
 * as input.  For these files, the  unprebind operation will zero library 
 * time stamps and version numbers and zero entries in the two-level hints
 * table.  These files will not gain the MH_PREBINDABLE flag.
 * All resulting binaries successfully processed by unprebind() will have
 * the MH_CANONICAL flag.
 */
extern
enum redo_prebinding_retval
unprebind(
const char *file_name,
const char *output_file,
const char *program_name,
char **error_message,
int zero_checksum,
void *inbuf,
uint32_t inlen,
void **outbuf,
uint32_t *outlen);

enum object_file_type_retval {
    OFT_OTHER,
    OFT_EXECUTABLE,
    OFT_DYLIB,
    OFT_BUNDLE,
    OFT_ARCHIVE,
    OFT_INCONSISTENT,
    OFT_FILE_ERROR
};

/*
 * object_file_type() takes a file_name and determines what type of object
 * file it is.  If it is a fat file and the architectures are not of the same
 * type then OFT_INCONSISTENT is returned.  If the file_name can't be opened,
 * read or malformed then OFT_FILE_ERROR is returned.
 */
extern
enum object_file_type_retval
object_file_type(
const char *file_name,
const char *program_name,
char **error_message);

struct prebind_cksum_arch {
    cpu_type_t cputype;		/* cpu specifier */
    cpu_subtype_t cpusubtype;	/* machine specifier */
    uint32_t has_cksum;		/* 1 if the arch as an LC_PREBIND_CKSUM */
    uint32_t cksum;		/* value of the cksum in LC_PREBIND_CKSUM */
};

/*
 * get_prebind_cksums() takes a file_name that is a Mach-O file or fat file
 * containing Mach-O files and returns a malloc(3)'ed array of
 * prebind_cksum_arch structs indirectly through the cksums parameter.  The
 * number of prebind_cksum_arch structs is returned indirectly through the
 * ncksums parameter.  If successful it returns zero else it returns non-zero.
 */
extern
int
get_prebind_cksums(
const char *file_name,
struct prebind_cksum_arch **cksums,
uint32_t *ncksums,
const char *program_name,
char **error_message);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MACH_O_REDO_PREBINDING_H_ */
