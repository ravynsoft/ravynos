/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*- 
 *
 * Copyright (c) 2008 Apple Inc. All rights reserved.
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

#include <stdint.h>


#if __cplusplus
extern "C" {
#endif /* __cplusplus */


/*
 * prune_trie() is a C vended function that is used by strip(1) to prune out
 * defined exported symbols from the export trie.  It is passed a pointer to
 * the start of bytes of the the trie and the size.  The prune() funciton
 * passed is called with each symbol name in the trie to determine if it is
 * to be pruned (retuning 1) or not (returning 0).  It writes the new trie
 * back into the trie buffer and returns the new size in trie_new_size.
 * If the pruning succeeds, NULL is returned.  If there was an error processing
 * the trie (e.f. it is malformed), then an error message string is returned.
 * The error string can be freed.
 */
extern const char*
prune_trie(
	uint8_t*	trie_start,
	uint32_t	trie_start_size,
	int			(*prune)(const char *name),
	uint32_t*	trie_new_size);


#if __cplusplus
}
#endif /* __cplusplus */
