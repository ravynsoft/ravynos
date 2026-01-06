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
#if defined(__MWERKS__) && !defined(__private_extern__)
#define __private_extern__ __declspec(private_extern)
#endif

/*
 * execute() does an execvp using the argv passed to it.  If the parameter
 * verbose is non-zero the command is printed to stderr.  A non-zero return
 * value indicates success zero indicates failure.
 */
__private_extern__ int execute(
    char **argv,
    int verbose);

__private_extern__ void add_execute_list(
    char *str);

__private_extern__ void add_execute_list_with_prefix(
    char *str);

__private_extern__ char * cmd_with_prefix(
    char *str);

__private_extern__ void reset_execute_list(
    void);

__private_extern__ int execute_list(
    int verbose);
