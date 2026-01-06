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
#include <vector>

#include "MachOFileAbstraction.hpp"
#include "MachOTrie.hpp"
#include "prune_trie.h"




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
const char*
prune_trie(
	uint8_t*	trie_start,
	uint32_t	trie_start_size,
	int			(*prune)(const char *name),
	uint32_t*	trie_new_size)
{
	// convert trie to vector of entries
	std::vector<mach_o::trie::Entry> originalExports;
	try {
		parseTrie(trie_start, trie_start+trie_start_size, originalExports);
	}
	catch (const char* msg) {
		return strdup(msg);
	}
	catch (...) {
		return strdup("unexpected exception processing trie");
	}
	
	// prune entries into new vector of entries
	std::vector<mach_o::trie::Entry> newExports;
	newExports.reserve(originalExports.size());
	for(std::vector<mach_o::trie::Entry>::iterator it = originalExports.begin(); it != originalExports.end(); ++it) {
		if ( prune(it->name) == 0 ) 
			newExports.push_back(*it);
	}

	// create new export trie
	std::vector<uint8_t> newExportTrieBytes;
	newExportTrieBytes.reserve(trie_start_size);
	mach_o::trie::makeTrie(newExports, newExportTrieBytes);
	// Need to align trie to 8 or 4 bytes.  We don't know the arch, but if the incoming trie
	// was not 8-byte aligned, then it can't be a 64-bit arch, so use 4-byte alignement.
	if ( (trie_start_size % 8) != 0 ) {
		// 4-byte align 
		while ( (newExportTrieBytes.size() % 4 ) != 0)
			newExportTrieBytes.push_back(0);
	}
	else {
		// 8-byte align 
		while ( (newExportTrieBytes.size() % 8 ) != 0)
			newExportTrieBytes.push_back(0);
	}
	
	// copy into place, zero pad
	*trie_new_size = newExportTrieBytes.size();
	if ( *trie_new_size > trie_start_size ) {
		char* msg;
		asprintf(&msg, "new trie is larger (%d) than original (%d)", *trie_new_size, trie_start_size);
		return msg;
	}
	memcpy(trie_start, &newExportTrieBytes[0], *trie_new_size);
	bzero(trie_start+*trie_new_size, trie_start_size - *trie_new_size);
	
	// success
	return NULL;
}
