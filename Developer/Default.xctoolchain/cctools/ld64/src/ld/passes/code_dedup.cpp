/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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

#ifdef __linux__
#include <string.h>
#endif
#include <stdint.h>
#include <math.h>
#include <unistd.h>
#include <dlfcn.h>
#include <mach/machine.h>

#include <vector>
#include <map>
#include <algorithm>
#include <unordered_map>

#include "ld.hpp"
#include "code_dedup.h"

namespace ld {
namespace passes {
namespace dedup {



class DeDupAliasAtom : public ld::Atom
{
public:
										DeDupAliasAtom(const ld::Atom* dupOf, const ld::Atom* replacement) :
											ld::Atom(dupOf->section(), ld::Atom::definitionRegular, ld::Atom::combineNever,
													dupOf->scope(), dupOf->contentType(), ld::Atom::symbolTableIn,
													false, false, true, replacement->alignment()),
											_dedupOf(dupOf),
											_fixup(0, ld::Fixup::k1of1, ld::Fixup::kindNoneFollowOn, ld::Fixup::bindingDirectlyBound, replacement) {
                                                if ( dupOf->autoHide() )
                                                    setAutoHide();
                                            }

	virtual const ld::File*				file() const		{ return _dedupOf->file(); }
	virtual const char*					translationUnitSource() const
															{ return NULL; }
	virtual const char*					name() const		{ return _dedupOf->name(); }
	virtual uint64_t					size() const		{ return 0; }
	virtual uint64_t					objectAddress() const { return 0; }
	virtual void						copyRawContent(uint8_t buffer[]) const { }
	virtual ld::Fixup::iterator			fixupsBegin() const	{ return &((ld::Fixup*)&_fixup)[0]; }
	virtual ld::Fixup::iterator			fixupsEnd()	const	{ return &((ld::Fixup*)&_fixup)[1]; }

private:
	const ld::Atom*                     _dedupOf;
	ld::Fixup							_fixup;
};


namespace {
    typedef std::unordered_map<const ld::Atom*, unsigned long> CachedHashes;

    ld::Internal*   sState = nullptr;
    CachedHashes    sSavedHashes;
    unsigned long   sHashCount = 0;
    unsigned long   sFixupCompareCount = 0;
};


// A helper for std::unordered_map<> that hashes the instructions of a function
struct atom_hashing {

    static unsigned long hash(const ld::Atom* atom) {
        auto pos = sSavedHashes.find(atom);
        if ( pos != sSavedHashes.end() )
            return pos->second;

        const unsigned instructionBytes = atom->size();
        const uint8_t*	instructions = atom->rawContentPointer();
        unsigned long hash = instructionBytes;
        for (unsigned i=0; i < instructionBytes; ++i) {
            hash = (hash * 33) + instructions[i];
        }
		for (ld::Fixup::iterator fit = atom->fixupsBegin(), end=atom->fixupsEnd(); fit != end; ++fit) {
            const Atom* target = NULL;
            switch ( fit->binding ) {
                case ld::Fixup::bindingDirectlyBound:
                    target = fit->u.target;
                    break;
                case ld::Fixup::bindingsIndirectlyBound:
                    target = sState->indirectBindingTable[fit->u.bindingIndex];
                    break;
                default:
                    break;
            }
            // don't include calls to auto-hide functions in hash because they might be de-dup'ed
            switch ( fit->kind ) {
#if SUPPORT_ARCH_arm64
                case ld::Fixup::kindStoreTargetAddressARM64Branch26:
#endif
                case ld::Fixup::kindStoreTargetAddressX86BranchPCRel32:
                    if ( target && target->autoHide() )
                        continue; // don't include
                    break;
                default:
                    break;
            }
            if ( target != NULL ) {
                const char* name = target->name();
                if ( target->contentType() == ld::Atom::typeCString )
                    name = (const char*)target->rawContentPointer();
                for (const char* s = name; *s != '\0'; ++s)
                    hash = (hash * 33) + *s;
            }
        }
        ++sHashCount;
        sSavedHashes[atom] = hash;
        return hash;
    }

    unsigned long operator()(const ld::Atom* atom) const {
        return hash(atom);
    }
};


// A helper for std::unordered_map<> that compares functions
struct atom_equal {

    struct BackChain {
        BackChain*      prev;
        const Atom*     inCallChain1;
        const Atom*     inCallChain2;

        bool inCallChain(const Atom* target) {
            for (BackChain* p = this; p->prev != NULL; p = p->prev) {
                if ( (p->inCallChain1 == target) || (p->inCallChain2 == target) )
                     return true;
            }
            return false;
        }
    };

    static bool sameFixups(const ld::Atom* atom1, const ld::Atom* atom2, BackChain& backChain) {
        ++sFixupCompareCount;
        //fprintf(stderr, "sameFixups(%s,%s)\n", atom1->name(), atom2->name());
        Fixup::iterator	f1   = atom1->fixupsBegin();
        Fixup::iterator	end1 = atom1->fixupsEnd();
        Fixup::iterator	f2   = atom2->fixupsBegin();
        Fixup::iterator	end2 = atom2->fixupsEnd();
        // two atoms must have same number of fixups
        if ( (end1 - f1) != (end2 - f2) )
            return false;
        // if no fixups, fixups are equal
        if ( f1 == end1 )
            return true;
        // all fixups must be the same
        do {
            if ( f1->offsetInAtom != f2->offsetInAtom )
                return false;
            if ( f1->kind != f2->kind )
                return false;
            if ( (f1->kind == ld::Fixup::kindAddAddend) || (f1->kind == ld::Fixup::kindSubtractAddend) ) {
                if ( f1->u.addend != f2->u.addend )
                    return false;
            }
            if ( f1->clusterSize != f2->clusterSize )
                return false;
            if ( f1->binding != f2->binding )
                return false;
            const Atom* target1 = NULL;
            const Atom* target2 = NULL;
            switch ( f1->binding ) {
                case ld::Fixup::bindingDirectlyBound:
                    target1 = f1->u.target;
                    target2 = f2->u.target;
                    break;
                case ld::Fixup::bindingsIndirectlyBound:
                    target1 = sState->indirectBindingTable[f1->u.bindingIndex];
                    target2 = sState->indirectBindingTable[f2->u.bindingIndex];
                    break;
                 case ld::Fixup::bindingNone:
                    break;
               default:
                    return false;
            }
            if ( target1 != target2 ) {
                // targets must match unless they are both calls to functions that will de-dup together
    #if SUPPORT_ARCH_arm64
                if ( (f1->kind != ld::Fixup::kindStoreTargetAddressARM64Branch26) && (f1->kind != ld::Fixup::kindStoreTargetAddressX86BranchPCRel32) )
    #else
                if ( f1->kind != ld::Fixup::kindStoreTargetAddressX86BranchPCRel32 )
    #endif
                    return false;
                if ( target1->section().type() != target2->section().type() )
                    return false;
                if ( target1->section().type() != ld::Section::typeCode )
                    return false;
                // to support co-recursive functions, don't recurse into equals() for targets already in the back chain
                if ( !backChain.inCallChain(target1) || !backChain.inCallChain(target2) ) {
                    BackChain nextBackChain;
                    nextBackChain.prev = &backChain;
                    nextBackChain.inCallChain1 = target1;
                    nextBackChain.inCallChain2 = target2;
                    if ( !equal(target1, target2, nextBackChain) )
                        return false;
                }
            }

            ++f1;
            ++f2;
        } while (f1 != end1);

        return true;
    }

    static bool equal(const ld::Atom* atom1, const ld::Atom* atom2, BackChain& backChain) {
        if ( atom1->size() != atom2->size() )
            return false;
        if ( atom_hashing::hash(atom1) != atom_hashing::hash(atom2) )
            return false;
        if ( memcmp(atom1->rawContentPointer(), atom2->rawContentPointer(), atom1->size()) != 0 )
            return false;
        bool result = sameFixups(atom1, atom2, backChain);
        //fprintf(stderr, "sameFixups(%s,%s) = %d\n", atom1->name(), atom2->name(), result);
        return result;
    }

    bool operator()(const ld::Atom* atom1, const ld::Atom* atom2) const {
        BackChain backChain = { NULL, atom1, atom2 };
        return equal(atom1, atom2, backChain);
    }
};



void doPass(const Options& opts, ld::Internal& state)
{
	const bool log = false;
	
	// only de-duplicate in final linked images
	if ( opts.outputKind() == Options::kObjectFile )
		return;

	// only de-duplicate for architectures that use relocations that don't store bits in instructions
	if ( (opts.architecture() != CPU_TYPE_ARM64) && (opts.architecture() != CPU_TYPE_X86_64) )
		return;

    // support -no_deduplicate to suppress this pass
    if ( ! opts.deduplicateFunctions() )
        return;

    const bool verbose = opts.verboseDeduplicate();

    // find __text section
    ld::Internal::FinalSection* textSection = NULL;
    for (ld::Internal::FinalSection* sect : state.sections) {
        if ( (sect->type() == ld::Section::typeCode) && (strcmp(sect->sectionName(), "__text") == 0) ) {
            textSection = sect;
            break;
        }
    }
    if ( textSection == NULL )
        return;

    // build map of auto-hide functions and their duplicates
    // the key for the map is always the first element in the value vector
    // the key is always earlier in the atoms list then matching other atoms
    sState = &state;
    std::unordered_map<const ld::Atom*, std::vector<const ld::Atom*>, atom_hashing, atom_equal> map;
    std::unordered_set<const ld::Atom*> masterAtoms;
    for (const ld::Atom* atom : textSection->atoms) {
        // ignore empty (alias) atoms
        if ( atom->size() == 0 )
            continue;
        if ( atom->autoHide() )
            map[atom].push_back(atom);
	}

    if ( log ) {
        for (auto& entry : map) {
            if ( entry.second.size() > 1 ) {
                printf("Found following matching functions:\n");
                for (const ld::Atom* atom : entry.second) {
                    printf("  %p %s\n", atom, atom->name());
                }
            }
        }
        fprintf(stderr, "duplicate sets count:\n");
        for (auto& entry : map)
            fprintf(stderr, "  %p -> %lu\n", entry.first, entry.second.size());
    }

    // construct alias atoms to replace atoms found to be duplicates
    unsigned atomsBeingComparedCount = 0;
    uint64_t dedupSavings = 0;
    std::vector<const ld::Atom*>& textAtoms = textSection->atoms;
    std::unordered_map<const ld::Atom*, const ld::Atom*> replacementMap;
    for (auto& entry : map) {
        const ld::Atom* masterAtom = entry.first;
        std::vector<const ld::Atom*>& dups = entry.second;
        atomsBeingComparedCount += dups.size();
        if ( dups.size() == 1 )
            continue;
        if ( verbose )  {
            dedupSavings += ((dups.size() - 1) * masterAtom->size());
            fprintf(stderr, "deduplicate the following %lu functions (%llu bytes apiece):\n", dups.size(), masterAtom->size());
        }
        for (const ld::Atom* dupAtom : dups) {
            if ( verbose )
                fprintf(stderr, "    %s\n", dupAtom->name());
            if ( dupAtom == masterAtom )
                continue;
            const ld::Atom* aliasAtom = new DeDupAliasAtom(dupAtom, masterAtom);
            auto pos = std::find(textAtoms.begin(), textAtoms.end(), masterAtom);
            if ( pos != textAtoms.end() ) {
                textAtoms.insert(pos, aliasAtom);
                state.atomToSection[aliasAtom] = textSection;
                replacementMap[dupAtom] = aliasAtom;
                (const_cast<ld::Atom*>(dupAtom))->setCoalescedAway();
            }
        }
    }
    if ( verbose )  {
        fprintf(stderr, "deduplication saved %llu bytes of __text\n", dedupSavings);
    }

    if ( log ) {
        fprintf(stderr, "replacement map:\n");
        for (auto& entry : replacementMap)
            fprintf(stderr, "  %p -> %p\n", entry.first, entry.second);
    }

    // walk all atoms and replace references to dups with references to alias
    for (ld::Internal::FinalSection* sect : state.sections) {
        for (const ld::Atom* atom : sect->atoms) {
			for (ld::Fixup::iterator fit = atom->fixupsBegin(), end=atom->fixupsEnd(); fit != end; ++fit) {
                std::unordered_map<const ld::Atom*, const ld::Atom*>::iterator pos;
                switch ( fit->binding ) {
                    case ld::Fixup::bindingsIndirectlyBound:
                        pos = replacementMap.find(state.indirectBindingTable[fit->u.bindingIndex]);
                        if ( pos != replacementMap.end() )
                            state.indirectBindingTable[fit->u.bindingIndex] = pos->second;
                        break;
                    case ld::Fixup::bindingDirectlyBound:
                        pos = replacementMap.find(fit->u.target);
                        if ( pos != replacementMap.end() )
                            fit->u.target = pos->second;
                        break;
                    default:
                        break;
                }
			}
        }
    }

    if ( log ) {
        fprintf(stderr, "atoms before pruning:\n");
        for (const ld::Atom* atom : textSection->atoms)
            fprintf(stderr, "  %p (size=%llu) %s\n", atom, atom->size(), atom->name());
    }
    // remove replaced atoms from section
	textSection->atoms.erase(std::remove_if(textSection->atoms.begin(), textSection->atoms.end(),
                [&](const ld::Atom* atom) {
                    return (replacementMap.count(atom) != 0);
                }),
                textSection->atoms.end());

   for (auto& entry : replacementMap)
        state.atomToSection.erase(entry.first);

    if ( log ) {
        fprintf(stderr, "atoms after pruning:\n");
        for (const ld::Atom* atom : textSection->atoms)
            fprintf(stderr, "  %p (size=%llu) %s\n", atom, atom->size(), atom->name());
    }

   //fprintf(stderr, "hash-count=%lu, fixup-compares=%lu, atom-count=%u\n", sHashCount, sFixupCompareCount, atomsBeingComparedCount);
}


} // namespace dedup
} // namespace passes 
} // namespace ld 
