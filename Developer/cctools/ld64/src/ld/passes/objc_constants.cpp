/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2009 Apple Inc. All rights reserved.
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
#include <algorithm>
#include <string.h>
#endif

#include <stdint.h>
#include <math.h>
#include <unistd.h>
#include <dlfcn.h>

#include <vector>
#include <map>

#include "MachOFileAbstraction.hpp"
#include "ld.hpp"
#include "objc_constants.h"
#include "configure.h"

namespace ld {
namespace passes {
namespace objc_constants {

class File; // forward reference

class GOTEntryAtom : public ld::Atom {
public:
                                            GOTEntryAtom(ld::Internal& internal, const ld::Atom* target, bool is64)
                : ld::Atom(_s_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
                           ld::Atom::scopeLinkageUnit, ld::Atom::typeNonLazyPointer,
                           symbolTableNotIn, false, false, false, (is64 ? ld::Atom::Alignment(3) : ld::Atom::Alignment(2))),
                _fixup(0, ld::Fixup::k1of1, (is64 ? ld::Fixup::kindStoreTargetAddressLittleEndian64 : ld::Fixup::kindStoreTargetAddressLittleEndian32), target),
                _target(target),
                _is64(is64) { internal.addAtom(*this); }

    virtual const ld::File*                 file() const                            { return NULL; }
    virtual const char*                     name() const                            { return _target->name(); }
    virtual uint64_t                        size() const                            { return (_is64 ? 8 : 4); }
    virtual uint64_t                        objectAddress() const                   { return 0; }
    virtual void                            copyRawContent(uint8_t buffer[]) const  { }
    virtual void                            setScope(Scope)                         { }
    virtual ld::Fixup::iterator             fixupsBegin() const                     { return &_fixup; }
    virtual ld::Fixup::iterator             fixupsEnd()    const                    { return &((ld::Fixup*)&_fixup)[1]; }

private:
    mutable ld::Fixup                       _fixup;
    const ld::Atom*                         _target;
    bool                                    _is64;

    static ld::Section                      _s_section;
};

ld::Section GOTEntryAtom::_s_section("__DATA_CONST", "__objc_ptr", ld::Section::typeNonLazyPointer);

ld::Section _s_section("__DATA_CONST", "__objc_ptr", ld::Section::typeNonLazyPointer);

class NullAtom
{
public:
    bool operator()(const ld::Atom* atom) const {
        return (atom == NULL);
    }
};

void doPass(const Options& opts, ld::Internal& internal)
{
    const bool log = false;

    // only make got section in final linked images
    if ( opts.outputKind() == Options::kObjectFile )
        return;

    bool runPass = false;
#if SUPPORT_ARCH_arm64e
    runPass = opts.supportsAuthenticatedPointers() & opts.sharedRegionEligible();
#endif
    if ( !runPass )
        return;

    const bool is64 = true;

    // Find the objc constants, ie, CFString's
    std::unordered_map<const ld::Atom*, std::vector<ld::Fixup*>> objcMap;
    std::unordered_map<const ld::Atom*, uint64_t> atomPositions;
    std::vector<ld::Internal::FinalSection*> objcSections;
    for (ld::Internal::FinalSection* sect : internal.sections) {
        if ( sect->type() != ld::Section::typeCFString )
            continue;
        for (const ld::Atom* atom : sect->atoms) {
            objcMap[atom] = { };
            atomPositions[atom] = objcMap.size();
        }
        objcSections.push_back(sect);
    }

    // Find all references to the objc atoms and make sure they are either
    // pointers or adrp/add pairs that we can update
    for (ld::Internal::FinalSection* sect : internal.sections) {
        for (const ld::Atom* atom : sect->atoms) {
            const ld::Atom* target = NULL;
            for (ld::Fixup::iterator fit = atom->fixupsBegin(), end=atom->fixupsEnd(); fit != end; ++fit) {
                switch ( fit->binding ) {
                    case ld::Fixup::bindingsIndirectlyBound:
                        target = internal.indirectBindingTable[fit->u.bindingIndex];
                        break;
                    case ld::Fixup::bindingDirectlyBound:
                        target = fit->u.target;
                        break;
                    default:
                        break;
                }
                if ( target == NULL )
                    continue;

                auto mapEntryIt = objcMap.find(target);
                if ( mapEntryIt == objcMap.end() )
                    continue;

                switch (fit->kind) {
                    case ld::Fixup::kindStoreTargetAddressLittleEndian64:
                        // This is supported, but we don't need to change it if we
                        // create a GOT, so skip it
                        break;
#if SUPPORT_ARCH_arm64
                    case ld::Fixup::kindStoreTargetAddressARM64Page21:
                        // ADRP fixups are supported, so add them to the map
                        mapEntryIt->second.push_back(fit);
                        break;
                    case ld::Fixup::kindStoreTargetAddressARM64PageOff12: {
                        // We can only support off12 add fixups as we need to
                        // convert them to ldr later
                        const uint8_t* loc = atom->rawContentPointer() + fit->offsetInAtom;
                        uint32_t instruction = LittleEndian::get32(*(uint32_t*)loc);
                        if ( (instruction & 0xFFC00000) != 0x91000000 ) {
                            // This fixup is not supported, so we can't move this objc atom
                            objcMap.erase(mapEntryIt);
                        } else {
                            mapEntryIt->second.push_back(fit);
                        }
                        break;
                    }
#endif
                    case ld::Fixup::kindLinkerOptimizationHint:
                        // Skip LOHs.  These are fine
                        break;
                    default:
                        // This fixup is not supported, so we can't move this objc atom
                        objcMap.erase(mapEntryIt);
                        break;
                }
            }
        }
    }

    // Sort the atoms by their ordinal to get reproducible builds
    std::vector<const Atom*> sortedAtoms;
    for (const auto& mapEntry : objcMap) {
        sortedAtoms.push_back(mapEntry.first);
    }

    struct AtomByOrdinalSorter
    {
        bool operator()(const ld::Atom* left, const ld::Atom* right) const
        {
            // first sort by reader
            ld::File::Ordinal leftFileOrdinal  = left->file()->ordinal();
            ld::File::Ordinal rightFileOrdinal = right->file()->ordinal();
            if ( leftFileOrdinal != rightFileOrdinal)
                return (leftFileOrdinal < rightFileOrdinal);

            // then sort by atom objectAddress
            uint64_t leftAddr  = left->objectAddress();
            uint64_t rightAddr = right->objectAddress();
            return leftAddr < rightAddr;
        }
    };

    AtomByOrdinalSorter sorter;
    std::sort(sortedAtoms.begin(), sortedAtoms.end(), sorter);

    // update atoms to use GOTs
    for (const ld::Atom* objcAtom : sortedAtoms) {
        const std::vector<ld::Fixup*>& fixups = objcMap[objcAtom];

        // If we have no fixups, the this objc constant was only referenced by 64-bit
        // pointer slots, not adrp/add, so we can skip it
        if ( fixups.empty() )
            continue;

        const Atom* gotAtom = new GOTEntryAtom(internal, objcAtom, is64);

        for (ld::Fixup* fixup : fixups) {
            fixup->binding = ld::Fixup::bindingDirectlyBound;
            fixup->u.target = gotAtom;

            switch (fixup->kind) {
#if SUPPORT_ARCH_arm64
                case ld::Fixup::kindStoreTargetAddressARM64Page21:
                    // We don't have to update the adrp instruction bytes.
                    // Just setting the target was enough here
                    break;
                case ld::Fixup::kindStoreTargetAddressARM64PageOff12: {
                    // We need to update the add to an ldr
                    // We can't write the instruction here, so instead change the fixup kind
                    // and it'll get rewritten later
                    fixup->kind = Fixup::kindStoreTargetAddressARM64PageOff12ConvertAddToLoad;
                    break;
                }
#endif
                default:
                    assert(0 && "Invalid fixup kind");
            }
        }
    }

    // Move all eligile CF/objc atoms to a new segment
    for (ld::Internal::FinalSection* sect : objcSections) {
        const char* sectionName = sect->sectionName();
        Section::Type sectionType = sect->type();
        ld::Internal::FinalSection* newSection = internal.getFinalSection(*new ld::Section("__OBJC_CONST", sectionName, sectionType));
        for (std::vector<const ld::Atom*>::iterator ait = sect->atoms.begin(); ait != sect->atoms.end(); ++ait) {
            const ld::Atom* atom = *ait;
            if ( objcMap.count(atom) != 0 ) {
                newSection->atoms.push_back(atom);
                internal.atomToSection[atom] = newSection;
                if (log) fprintf(stderr, "moved to __OBJC_CONST: %s, size=%llu\n", atom->name(), atom->size());
                *ait = NULL;  // change atom to NULL for later bulk removal
            }
        }
        sect->atoms.erase(std::remove_if(sect->atoms.begin(), sect->atoms.end(), NullAtom()), sect->atoms.end());
        if ( sect->atoms.empty() )
            internal.sections.erase(std::remove(internal.sections.begin(), internal.sections.end(), sect), internal.sections.end());
    }
}


} // namespace objc_constants
} // namespace passes
} // namespace ld
