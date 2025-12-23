/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-*
 *
 * Copyright (c) 2009-2010 Apple Inc. All rights reserved.
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
 

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#ifdef __linux__
#include <algorithm>
#include <string.h>
namespace {
	typedef long double max_align_t;
}
#define MAXPATHLEN 1024
#else
#include <sys/sysctl.h>
#endif
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <assert.h>

#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <algorithm>

#include "Options.h"

#include "ld.hpp"
#include "InputFiles.h"
#include "SymbolTable.h"



namespace ld {
namespace tool {


// HACK, I can't find a way to pass values in the compare classes (e.g. ContentFuncs)
// so use global variable to pass info.
static ld::IndirectBindingTable*	_s_indirectBindingTable = NULL;


SymbolTable::SymbolTable(const Options& opts, std::vector<const ld::Atom*>& ibt) 
	: _options(opts), _cstringTable(6151), _indirectBindingTable(ibt), _hasExternalTentativeDefinitions(false)
{  
	_s_indirectBindingTable = this;
}


size_t SymbolTable::ContentFuncs::operator()(const ld::Atom* atom) const
{
	return atom->contentHash(*_s_indirectBindingTable);
}

bool SymbolTable::ContentFuncs::operator()(const ld::Atom* left, const ld::Atom* right) const
{
	return (memcmp(left->rawContentPointer(), right->rawContentPointer(), left->size()) == 0);
}



size_t SymbolTable::CStringHashFuncs::operator()(const ld::Atom* atom) const
{
	return atom->contentHash(*_s_indirectBindingTable);
}

bool SymbolTable::CStringHashFuncs::operator()(const ld::Atom* left, const ld::Atom* right) const
{
	return (strcmp((char*)left->rawContentPointer(), (char*)right->rawContentPointer()) == 0);
}


size_t SymbolTable::UTF16StringHashFuncs::operator()(const ld::Atom* atom) const
{
	return atom->contentHash(*_s_indirectBindingTable);
}

bool SymbolTable::UTF16StringHashFuncs::operator()(const ld::Atom* left, const ld::Atom* right) const
{
	if ( left == right )
		return true;
	const void* leftContent = left->rawContentPointer();
	const void* rightContent = right->rawContentPointer();
	unsigned int amount = left->size()-2;
	bool result = (memcmp(leftContent, rightContent, amount) == 0);
	return result;
}


size_t SymbolTable::ReferencesHashFuncs::operator()(const ld::Atom* atom) const
{
	return atom->contentHash(*_s_indirectBindingTable);
}

bool SymbolTable::ReferencesHashFuncs::operator()(const ld::Atom* left, const ld::Atom* right) const
{
	return left->canCoalesceWith(*right, *_s_indirectBindingTable);
}


void SymbolTable::addDuplicateSymbolError(const char* name, const ld::Atom* atom)
{
	addDuplicateSymbol(_duplicateSymbolErrors, name, atom);
}

void SymbolTable::addDuplicateSymbolWarning(const char* name, const ld::Atom* atom)
{
	addDuplicateSymbol(_duplicateSymbolWarnings, name, atom);
}

void SymbolTable::addDuplicateSymbol(DuplicateSymbols& dups, const char *name, const ld::Atom *atom)
{
    // Look up or create the file list for name.
    DuplicateSymbols::iterator symbolsIterator = dups.find(name);
    DuplicatedSymbolAtomList *atoms = NULL;
    if (symbolsIterator != dups.end()) {
        atoms = symbolsIterator->second;
    }
    else {
        atoms = new std::vector<const ld::Atom *>;
        dups.insert(std::pair<const char *, DuplicatedSymbolAtomList *>(name, atoms));
    }

    // check if file is already in the list, add it if not
    bool found = false;
    for (DuplicatedSymbolAtomList::iterator it = atoms->begin(); !found && it != atoms->end(); it++)
        if (strcmp((*it)->safeFilePath(), atom->safeFilePath()) == 0)
            found = true;
    if (!found)
        atoms->push_back(atom);
}


void SymbolTable::checkDuplicateSymbols() const
{
	// print duplicate errors
    bool foundDuplicate = false;
    for (DuplicateSymbols::const_iterator symbolIt = _duplicateSymbolErrors.begin(); symbolIt != _duplicateSymbolErrors.end(); symbolIt++) {
        DuplicatedSymbolAtomList *atoms = symbolIt->second;
        bool reportDuplicate;
        if (_options.deadCodeStrip()) {
            // search for a live atom
            reportDuplicate = false;
            for (DuplicatedSymbolAtomList::iterator it = atoms->begin(); !reportDuplicate && it != atoms->end(); it++) {
                if ((*it)->live())
                    reportDuplicate = true;
            }
        } else {
            reportDuplicate = true;
        }
        if (reportDuplicate) {
            foundDuplicate = true;
            fprintf(stderr, "duplicate symbol '%s' in:\n", _options.demangleSymbol(symbolIt->first));
            for (DuplicatedSymbolAtomList::iterator atomIt = atoms->begin(); atomIt != atoms->end(); atomIt++) {
                fprintf(stderr, "    %s\n", (*atomIt)->safeFilePath());
            }
        }
    }
    if (foundDuplicate)
        throwf("%d duplicate symbol%s", (int)_duplicateSymbolErrors.size(), _duplicateSymbolErrors.size()==1?"":"s");

	// print duplicates warnings
    for (DuplicateSymbols::const_iterator symbolIt = _duplicateSymbolWarnings.begin(); symbolIt != _duplicateSymbolWarnings.end(); symbolIt++) {
        std::string msg = "duplicate symbol '" + std::string(_options.demangleSymbol(symbolIt->first)) + "' in:";
		for (const ld::Atom* atom : *symbolIt->second) {
			msg += "\n    " + std::string(atom->safeFilePath());
		}
		warning("%s", msg.c_str());
    }
}

// AtomPicker encapsulates the logic for picking which atom to use when adding an atom by name results in a collision
class NameCollisionResolution {
public:
	NameCollisionResolution(const ld::Atom& a, const ld::Atom& b, Options::Treatment duplicates, const Options& options) : _atomA(a), _atomB(b), _options(options), _reportDuplicate(false), _duplicates(duplicates) {
		pickAtom();
	}
	
	// Returns which atom to use
	const ld::Atom& chosen() { return *_chosen; }
	bool choseAtom(const ld::Atom& atom) { return _chosen == &atom; }

	// Returns true if the two atoms should be reported as a duplicate symbol
	bool reportDuplicateError()    { return _reportDuplicate && (_duplicates == Options::Treatment::kError); }
	bool reportDuplicateWarning()  { return _reportDuplicate && (_duplicates == Options::Treatment::kWarning); }

private:
	const ld::Atom& _atomA;
	const ld::Atom& _atomB;
	const Options& _options;
	const ld::Atom* _chosen;
	bool _reportDuplicate;
	Options::Treatment _duplicates;

	void pickAtom(const ld::Atom& atom) { _chosen = &atom; } // primitive to set which atom is picked
	void pickAtomA() { pickAtom(_atomA); }	// primitive to pick atom A
	void pickAtomB() { pickAtom(_atomB); }	// primitive to pick atom B
	
	// use atom A if pickA, otherwise use atom B
	void pickAOrB(bool pickA) { if (pickA) pickAtomA(); else pickAtomB(); }
	
	void pickHigherOrdinal() {
		pickAOrB(_atomA.file()->ordinal() < _atomB.file()->ordinal());
	}
	
	void pickLowerOrdinal() {
		pickAOrB(_atomA.file()->ordinal() > _atomB.file()->ordinal());
	}
	
	void pickLargerSize() {
		if (_atomA.size() == _atomB.size())
			pickLowerOrdinal();
		else
			pickAOrB(_atomA.size() > _atomB.size());
	}
	
	void pickGreaterAlignment() {
		pickAOrB(_atomA.alignment().trailingZeros() > _atomB.alignment().trailingZeros());
	}
	
	void pickBetweenRegularAtoms() {
		if ( _atomA.combine() == ld::Atom::combineByName ) {
			if ( _atomB.combine() == ld::Atom::combineByName ) {
				// <rdar://problem/9183821> always choose mach-o over llvm bit code, otherwise LTO may eliminate the llvm atom
				const bool aIsLTO = (_atomA.contentType() == ld::Atom::typeLTOtemporary);
				const bool bIsLTO = (_atomB.contentType() == ld::Atom::typeLTOtemporary);
				// <rdar://problem/9183821> always choose mach-o over llvm bit code, otherwise LTO may eliminate the llvm atom
				if ( aIsLTO != bIsLTO ) {
					pickAOrB(!aIsLTO);
				}
				else {
					// both weak, prefer non-auto-hide one
					if ( _atomA.autoHide() != _atomB.autoHide() ) {
						// <rdar://problem/6783167> support auto hidden weak symbols: .weak_def_can_be_hidden
						pickAOrB(!_atomA.autoHide());
					}
					else if ( _atomA.autoHide() && _atomB.autoHide() ) {
						// both have auto-hide, so use one with greater alignment
						pickGreaterAlignment();
					}
					else {
						// neither auto-hide, check visibility
						if ( _atomA.scope() != _atomB.scope() ) {
							// <rdar://problem/8304984> use more visible weak def symbol
							pickAOrB(_atomA.scope() == ld::Atom::scopeGlobal);
						}
						else {
							// both have same visibility, use one with greater alignment
							pickGreaterAlignment();
						}
					}
				}
			}
			else {
				pickAtomB(); // pick not-weak

			}
		}
		else {
			if ( _atomB.combine() == ld::Atom::combineByName ) {
				pickAtomA(); // pick not-weak

			}
			else {
				// both are not-weak
				if ( _atomA.section().type() == ld::Section::typeMachHeader ) {
					pickAtomA();
				} 
				else if ( _atomB.section().type() == ld::Section::typeMachHeader ) {
					pickAtomB();
				} 
				else {
					switch (_duplicates) {
						case Options::Treatment::kError:
							_reportDuplicate = true;
							break;
						case Options::Treatment::kWarning:
							pickLowerOrdinal();
							_reportDuplicate = true;
							break;
						case Options::Treatment::kSuppress:
							pickLowerOrdinal();
							break;
						default:
							break;
					}
				}
			}
		}
	}
	
	void pickCommonsMode(const ld::Atom& dylib, const ld::Atom& proxy) {
		assert(dylib.definition() == ld::Atom::definitionTentative);
		assert(proxy.definition() == ld::Atom::definitionProxy);
		switch ( _options.commonsMode() ) {
			case Options::kCommonsIgnoreDylibs:
				if ( _options.warnCommons() )
					warning("using common symbol %s from %s and ignoring definition from dylib %s",
							proxy.name(), proxy.safeFilePath(), dylib.safeFilePath());
				pickAtom(dylib);
				break;
			case Options::kCommonsOverriddenByDylibs:
				if ( _options.warnCommons() )
					warning("replacing common symbol %s from %s with true definition from dylib %s",
							proxy.name(), proxy.safeFilePath(), dylib.safeFilePath());
				pickAtom(proxy);
				break;
			case Options::kCommonsConflictsDylibsError:
				throwf("common symbol %s from %s conflicts with definition from dylib %s",
					   proxy.name(), proxy.safeFilePath(), dylib.safeFilePath());
		}
	}
	
	void pickProxyAtom() {
		// both atoms are definitionProxy
		// <rdar://problem/5137732> ld should keep looking when it finds a weak definition in a dylib
		if ( _atomA.combine() == ld::Atom::combineByName ) {
			pickAtomB();
		} else if ( _atomB.combine() == ld::Atom::combineByName ) {
			pickAtomA();
		} else {
				throwf("symbol %s exported from both %s and %s\n", _atomA.name(), _atomA.safeFilePath(), _atomB.safeFilePath());
		}
	}
	
	void pickAtom() {
		//fprintf(stderr, "pickAtom(), a=%p, def=%d, b=%p, def=%d\n", &_atomA, _atomA.definition(), &_atomB, _atomB.definition());
		// First, discriminate by definition
		switch (_atomA.definition()) {
			case ld::Atom::definitionRegular:
				switch (_atomB.definition()) {
					case ld::Atom::definitionRegular:
						pickBetweenRegularAtoms();
						break;
					case ld::Atom::definitionTentative:
						if ( _atomB.size() > _atomA.size() ) {
							warning("tentative definition of '%s' with size %llu from '%s' is being replaced by real definition of smaller size %llu from '%s'",
									_atomA.name(), _atomB.size(), _atomB.safeFilePath(), _atomA.size(), _atomA.safeFilePath());
						}
						pickAtomA();
						break;
					case ld::Atom::definitionAbsolute:
						_reportDuplicate = true;
						pickHigherOrdinal();
						break;
					case ld::Atom::definitionProxy:
						pickAtomA();
						break;
				}
				break;
			case ld::Atom::definitionTentative:
				switch (_atomB.definition()) {
					case ld::Atom::definitionRegular:
						if ( _atomA.size() > _atomB.size() ) {
							warning("tentative definition of '%s' with size %llu from '%s' is being replaced by real definition of smaller size %llu from '%s'",
									_atomA.name(), _atomA.size(),_atomA.safeFilePath(), _atomB.size(), _atomB.safeFilePath());
						}
						pickAtomB();
						break;
					case ld::Atom::definitionTentative:
						pickLargerSize();
						break;
					case ld::Atom::definitionAbsolute:
						pickHigherOrdinal();
						break;
					case ld::Atom::definitionProxy:
						pickCommonsMode(_atomA, _atomB);
						break;
				}
				break;
			case ld::Atom::definitionAbsolute:
				switch (_atomB.definition()) {
					case ld::Atom::definitionRegular:
						_reportDuplicate = true;
						pickHigherOrdinal();
						break;
					case ld::Atom::definitionTentative:
						pickAtomA();
						break;
					case ld::Atom::definitionAbsolute:
						_reportDuplicate = true;
						pickHigherOrdinal();
						break;
					case ld::Atom::definitionProxy:
						pickAtomA();
						break;
				}
				break;
			case ld::Atom::definitionProxy:
				switch (_atomB.definition()) {
					case ld::Atom::definitionRegular:
						pickAtomB();
						break;
					case ld::Atom::definitionTentative:
						pickCommonsMode(_atomB, _atomA);
						break;
					case ld::Atom::definitionAbsolute:
						pickAtomB();
						break;
					case ld::Atom::definitionProxy:
						pickProxyAtom();
						break;
				}
				break;
		}
	}
};

bool SymbolTable::addByName(const ld::Atom& newAtom, Options::Treatment duplicates)
{
	bool useNew = true;
	assert(newAtom.name() != NULL);
	const char* name = newAtom.name();
	IndirectBindingSlot slot = this->findSlotForName(name);
	const ld::Atom* existingAtom = _indirectBindingTable[slot];
	//fprintf(stderr, "addByName(%p) name=%s, slot=%u, existing=%p\n", &newAtom, newAtom.name(), slot, existingAtom);
	if ( existingAtom != NULL ) {
		assert(&newAtom != existingAtom);
		NameCollisionResolution picker(newAtom, *existingAtom, duplicates, _options);
		if ( picker.reportDuplicateError() ) {
			addDuplicateSymbolError(name, existingAtom);
			addDuplicateSymbolError(name, &newAtom);
		}
		else if ( picker.reportDuplicateWarning() ) {
			addDuplicateSymbolWarning(name, existingAtom);
			addDuplicateSymbolWarning(name, &newAtom);
		}
		useNew = picker.choseAtom(newAtom);
	}
	if ( useNew ) {
		_indirectBindingTable[slot] = &newAtom;
		if ( existingAtom != NULL ) {
			markCoalescedAway(existingAtom);
		}
		if ( newAtom.scope() == ld::Atom::scopeGlobal ) {
			if ( newAtom.definition() == ld::Atom::definitionTentative ) {
				_hasExternalTentativeDefinitions = true;
			}
		}
	}
	else {
		markCoalescedAway(&newAtom);
	}
	// return if existing atom in symbol table was replaced
	return useNew && (existingAtom != NULL);
}


bool SymbolTable::addByContent(const ld::Atom& newAtom)
{
	bool useNew = true;
	const ld::Atom* existingAtom;
	IndirectBindingSlot slot = this->findSlotForContent(&newAtom, &existingAtom);
	//fprintf(stderr, "addByContent(%p) name=%s, slot=%u, existing=%p\n", &newAtom, newAtom.name(), slot, existingAtom);
	if ( existingAtom != NULL ) {
		// use existing unless new one has greater alignment requirements
		useNew = ( newAtom.alignment().trailingZeros() > existingAtom->alignment().trailingZeros() );
	}
	if ( useNew ) {
		_indirectBindingTable[slot] = &newAtom;
		if ( existingAtom != NULL ) 
			markCoalescedAway(existingAtom);
	}
	else {
		_indirectBindingTable[slot] = existingAtom;
		if ( existingAtom != &newAtom )
			markCoalescedAway(&newAtom);
	}
	// return if existing atom in symbol table was replaced
	return useNew && (existingAtom != NULL);
}

bool SymbolTable::addByReferences(const ld::Atom& newAtom)
{
	bool useNew = true;
	const ld::Atom* existingAtom;
	IndirectBindingSlot slot = this->findSlotForReferences(&newAtom, &existingAtom);
	//fprintf(stderr, "addByReferences(%p) name=%s, slot=%u, existing=%p\n", &newAtom, newAtom.name(), slot, existingAtom);
	if ( existingAtom != NULL ) {
		// use existing unless new one has greater alignment requirements
		useNew = ( newAtom.alignment().trailingZeros() > existingAtom->alignment().trailingZeros() );
	}
	if ( useNew ) {
		_indirectBindingTable[slot] = &newAtom;
		if ( existingAtom != NULL ) 
			markCoalescedAway(existingAtom);
	}
	else {
		if ( existingAtom != &newAtom )
			markCoalescedAway(&newAtom);
	}
	// return if existing atom in symbol table was replaced
	return useNew && (existingAtom != NULL);
}


bool SymbolTable::add(const ld::Atom& atom, Options::Treatment duplicates)
{
	//fprintf(stderr, "SymbolTable::add(%p), name=%s\n", &atom, atom.name());
	assert(atom.scope() != ld::Atom::scopeTranslationUnit);
	switch ( atom.combine() ) {
		case ld::Atom::combineNever:
		case ld::Atom::combineByName:
			return this->addByName(atom, duplicates);
			break;
		case ld::Atom::combineByNameAndContent:
			return this->addByContent(atom);
			break;
		case ld::Atom::combineByNameAndReferences:
			return this->addByReferences(atom);
			break;
	}

	return false;
}

void SymbolTable::markCoalescedAway(const ld::Atom* atom)
{
	// remove this from list of all atoms used
	//fprintf(stderr, "markCoalescedAway(%p) from %s\n", atom, atom->safeFilePath());
	(const_cast<ld::Atom*>(atom))->setCoalescedAway();
	
	//
	// The fixupNoneGroupSubordinate* fixup kind is used to model group comdat.  
	// The "signature" atom in the group has a fixupNoneGroupSubordinate* fixup to
	// all other members of the group.  So, if the signature atom is 
	// coalesced away, all other atoms in the group should also be removed.  
	//
	for (ld::Fixup::iterator fit=atom->fixupsBegin(), fend=atom->fixupsEnd(); fit != fend; ++fit) {	
		switch ( fit->kind ) {
			case ld::Fixup::kindNoneGroupSubordinate:
			case ld::Fixup::kindNoneGroupSubordinateFDE:
			case ld::Fixup::kindNoneGroupSubordinateLSDA:
				assert(fit->binding == ld::Fixup::bindingDirectlyBound);
				this->markCoalescedAway(fit->u.target);
				break;
			default:
				break;
		}
	}

}


struct StrcmpSorter {
		bool operator() (const char* i,const char* j) {
			if (i==NULL)
				return true;
			if (j==NULL)
				return false;
			return strcmp(i, j)<0;}
};

void SymbolTable::undefines(std::vector<const char*>& undefs)
{
	// return all names in _byNameTable that have no associated atom
	for (NameToSlot::iterator it=_byNameTable.begin(); it != _byNameTable.end(); ++it) {
		//fprintf(stderr, "  _byNameTable[%s] = slot %d which has atom %p\n", it->first, it->second, _indirectBindingTable[it->second]);
		if ( _indirectBindingTable[it->second] == NULL )
			undefs.push_back(it->first);
	}
	// sort so that undefines are in a stable order (not dependent on hashing functions)
	struct StrcmpSorter strcmpSorter;
	std::sort(undefs.begin(), undefs.end(), strcmpSorter);
}


void SymbolTable::tentativeDefs(std::vector<const char*>& tents)
{
	// return all names in _byNameTable that have no associated atom
	for (NameToSlot::iterator it=_byNameTable.begin(); it != _byNameTable.end(); ++it) {
		const char* name = it->first;
		const ld::Atom* atom = _indirectBindingTable[it->second];
		if ( (atom != NULL) && (atom->definition() == ld::Atom::definitionTentative) )
			tents.push_back(name);
	}
	std::sort(tents.begin(), tents.end());
}


void SymbolTable::mustPreserveForBitcode(std::unordered_set<const char*>& syms)
{
	// return all names in _byNameTable that have no associated atom
	for (const auto &entry: _byNameTable) {
		const char* name = entry.first;
		const ld::Atom* atom = _indirectBindingTable[entry.second];
		if ( (atom == NULL) || (atom->definition() == ld::Atom::definitionProxy) )
			syms.insert(name);
	}
}


bool SymbolTable::hasName(const char* name)			
{ 
	NameToSlot::iterator pos = _byNameTable.find(name);
	if ( pos == _byNameTable.end() ) 
		return false;
	return (_indirectBindingTable[pos->second] != NULL); 
}

// find existing or create new slot
SymbolTable::IndirectBindingSlot SymbolTable::findSlotForName(const char* name)
{
	NameToSlot::iterator pos = _byNameTable.find(name);
	if ( pos != _byNameTable.end() ) 
		return pos->second;
	// create new slot for this name
	SymbolTable::IndirectBindingSlot slot = _indirectBindingTable.size();
	_indirectBindingTable.push_back(NULL);
	_byNameTable[name] = slot;
	_byNameReverseTable[slot] = name;
	return slot;
}

void SymbolTable::removeDeadAtoms()
{
	// remove dead atoms from: _byNameTable, _byNameReverseTable, and _indirectBindingTable
	std::vector<const char*> namesToRemove;
	for (NameToSlot::iterator it=_byNameTable.begin(); it != _byNameTable.end(); ++it) {
		IndirectBindingSlot slot = it->second;
		const ld::Atom* atom = _indirectBindingTable[slot];
		if ( atom != NULL ) {
			if ( !atom->live() && !atom->dontDeadStrip() ) {
				//fprintf(stderr, "removing from symbolTable[%u] %s\n", slot, atom->name());
				_indirectBindingTable[slot] = NULL;
				// <rdar://problem/16025786> need to completely remove dead atoms from symbol table
				_byNameReverseTable.erase(slot);
				// can't remove while iterating, do it after iteration
				namesToRemove.push_back(it->first);
			}
		}
	}
	for (std::vector<const char*>::iterator it = namesToRemove.begin(); it != namesToRemove.end(); ++it) {
		_byNameTable.erase(*it);
	}

	// remove dead atoms from _nonLazyPointerTable
	for (ReferencesToSlot::iterator it=_nonLazyPointerTable.begin(); it != _nonLazyPointerTable.end(); ) {
		const ld::Atom* atom = it->first;
		assert(atom != NULL);
		if ( !atom->live() && !atom->dontDeadStrip() )
			it = _nonLazyPointerTable.erase(it);
		else
			++it;
	}

	// remove dead atoms from _cstringTable
	for (CStringToSlot::iterator it=_cstringTable.begin(); it != _cstringTable.end(); ) {
		const ld::Atom* atom = it->first;
		assert(atom != NULL);
		if ( !atom->live() && !atom->dontDeadStrip() )
			it = _cstringTable.erase(it);
		else
			++it;
	}

	// remove dead atoms from _utf16Table
	for (UTF16StringToSlot::iterator it=_utf16Table.begin(); it != _utf16Table.end(); ) {
		const ld::Atom* atom = it->first;
		assert(atom != NULL);
		if ( !atom->live() && !atom->dontDeadStrip() )
			it = _utf16Table.erase(it);
		else
			++it;
	}

	// remove dead atoms from _cfStringTable
	for (ReferencesToSlot::iterator it=_cfStringTable.begin(); it != _cfStringTable.end(); ) {
		const ld::Atom* atom = it->first;
		assert(atom != NULL);
		if ( !atom->live() && !atom->dontDeadStrip() )
			it = _cfStringTable.erase(it);
		else
			++it;
	}

	// remove dead atoms from _literal4Table
	for (ContentToSlot::iterator it=_literal4Table.begin(); it != _literal4Table.end(); ) {
		const ld::Atom* atom = it->first;
		assert(atom != NULL);
		if ( !atom->live() && !atom->dontDeadStrip() )
			it = _literal4Table.erase(it);
		else
			++it;
	}

	// remove dead atoms from _literal8Table
	for (ContentToSlot::iterator it=_literal8Table.begin(); it != _literal8Table.end(); ) {
		const ld::Atom* atom = it->first;
		assert(atom != NULL);
		if ( !atom->live() && !atom->dontDeadStrip() )
			it = _literal8Table.erase(it);
		else
			++it;
	}

	// remove dead atoms from _literal16Table
	for (ContentToSlot::iterator it=_literal16Table.begin(); it != _literal16Table.end(); ) {
		const ld::Atom* atom = it->first;
		assert(atom != NULL);
		if ( !atom->live() && !atom->dontDeadStrip() )
			it = _literal16Table.erase(it);
		else
			++it;
	}
}


// find existing or create new slot
SymbolTable::IndirectBindingSlot SymbolTable::findSlotForContent(const ld::Atom* atom, const ld::Atom** existingAtom)
{
	//fprintf(stderr, "findSlotForContent(%p)\n", atom);
	SymbolTable::IndirectBindingSlot slot = 0;
	UTF16StringToSlot::iterator upos;
	CStringToSlot::iterator cspos;
	ContentToSlot::iterator pos;
	switch ( atom->section().type() ) {
		case ld::Section::typeCString:
			cspos = _cstringTable.find(atom);
			if ( cspos != _cstringTable.end() ) {
				*existingAtom = _indirectBindingTable[cspos->second];
				return cspos->second;
			}
			slot = _indirectBindingTable.size();
			_cstringTable[atom] = slot;
			break;
		case ld::Section::typeNonStdCString:
			{
				// use seg/sect name is key to map to avoid coalescing across segments and sections
				char segsect[64];
				sprintf(segsect, "%s/%s", atom->section().segmentName(), atom->section().sectionName());
				NameToMap::iterator mpos = _nonStdCStringSectionToMap.find(segsect);
				CStringToSlot* map = NULL;
				if ( mpos == _nonStdCStringSectionToMap.end() ) {
					map = new CStringToSlot();
					_nonStdCStringSectionToMap[strdup(segsect)] = map;
				}
				else {
					map = mpos->second;
				}
				cspos = map->find(atom);
				if ( cspos != map->end() ) {
					*existingAtom = _indirectBindingTable[cspos->second];
					return cspos->second;
				}
				slot = _indirectBindingTable.size();
				map->operator[](atom) = slot;
			}
			break;
		case ld::Section::typeUTF16Strings:
			upos = _utf16Table.find(atom);
			if ( upos != _utf16Table.end() ) {
				*existingAtom = _indirectBindingTable[upos->second];
				return upos->second;
			}
			slot = _indirectBindingTable.size();
			_utf16Table[atom] = slot;
			break;
		case ld::Section::typeLiteral4:
			pos = _literal4Table.find(atom);
			if ( pos != _literal4Table.end() ) {
				*existingAtom = _indirectBindingTable[pos->second];
				return pos->second;
			}
			slot = _indirectBindingTable.size();
			_literal4Table[atom] = slot;
			break;
		case ld::Section::typeLiteral8:
			pos = _literal8Table.find(atom);
			if ( pos != _literal8Table.end() ) {
				*existingAtom = _indirectBindingTable[pos->second];
				return pos->second;
			}
			slot = _indirectBindingTable.size();
			_literal8Table[atom] = slot;
			break;
		case ld::Section::typeLiteral16:
			pos = _literal16Table.find(atom);
			if ( pos != _literal16Table.end() ) {
				*existingAtom = _indirectBindingTable[pos->second];
				return pos->second;
			}
			slot = _indirectBindingTable.size();
			_literal16Table[atom] = slot;
			break;
		default:
			assert(0 && "section type does not support coalescing by content");
	}
	_indirectBindingTable.push_back(atom); 
	*existingAtom = NULL;
	return slot;
}



// find existing or create new slot
SymbolTable::IndirectBindingSlot SymbolTable::findSlotForReferences(const ld::Atom* atom, const ld::Atom** existingAtom)
{
	//fprintf(stderr, "findSlotForReferences(%p)\n", atom);
	
	SymbolTable::IndirectBindingSlot slot = 0;
	ReferencesToSlot::iterator pos;
	switch ( atom->section().type() ) {
		case ld::Section::typeNonLazyPointer:		
			pos = _nonLazyPointerTable.find(atom);
			if ( pos != _nonLazyPointerTable.end() ) {
				*existingAtom = _indirectBindingTable[pos->second];
				return pos->second;
			}
			slot = _indirectBindingTable.size();
			_nonLazyPointerTable[atom] = slot;
			break;
		case ld::Section::typeCFString:
			pos = _cfStringTable.find(atom);
			if ( pos != _cfStringTable.end() ) {
				*existingAtom = _indirectBindingTable[pos->second];
				return pos->second;
			}
			slot = _indirectBindingTable.size();
			_cfStringTable[atom] = slot;
			break;
		case ld::Section::typeObjCClassRefs:
			pos = _objc2ClassRefTable.find(atom);
			if ( pos != _objc2ClassRefTable.end() ) {
				*existingAtom = _indirectBindingTable[pos->second];
				return pos->second;
			}
			slot = _indirectBindingTable.size();
			_objc2ClassRefTable[atom] = slot;
			break;
		case ld::Section::typeCStringPointer:
			pos = _pointerToCStringTable.find(atom);
			if ( pos != _pointerToCStringTable.end() ) {
				*existingAtom = _indirectBindingTable[pos->second];
				return pos->second;
			}
			slot = _indirectBindingTable.size();
			_pointerToCStringTable[atom] = slot;
			break;
		case ld::Section::typeTLVPointers:
			pos = _threadPointerTable.find(atom);
			if ( pos != _threadPointerTable.end() ) {
				*existingAtom = _indirectBindingTable[pos->second];
				return pos->second;
			}
			slot = _indirectBindingTable.size();
			_threadPointerTable[atom] = slot;
			break;
		default:
			assert(0 && "section type does not support coalescing by references");
	}
	_indirectBindingTable.push_back(atom);
	*existingAtom = NULL;
	return slot;
}


const char*	SymbolTable::indirectName(IndirectBindingSlot slot) const
{
	assert(slot < _indirectBindingTable.size());
	const ld::Atom* target = _indirectBindingTable[slot];
	if ( target != NULL )  {
		return target->name();
	}
	// handle case when by-name reference is indirected and no atom yet in _byNameTable
	SlotToName::const_iterator pos = _byNameReverseTable.find(slot);
	if ( pos != _byNameReverseTable.end() )
		return pos->second;
	assert(0);
	return NULL;
}

const ld::Atom* SymbolTable::indirectAtom(IndirectBindingSlot slot) const
{
	assert(slot < _indirectBindingTable.size());
	return _indirectBindingTable[slot];
}


void SymbolTable::removeDeadUndefs(std::vector<const ld::Atom*>& allAtoms, const std::unordered_set<const ld::Atom*>& keep)
{
	// mark the indirect entries in use
	std::vector<bool> indirectUsed;
	for (size_t i=0; i < _indirectBindingTable.size(); ++i)
		indirectUsed.push_back(false);
	for (const ld::Atom* atom : allAtoms) {
		for (auto it = atom->fixupsBegin(); it != atom->fixupsEnd(); ++it) {
			switch (it->binding) {
				case ld::Fixup::bindingsIndirectlyBound:
					indirectUsed[it->u.bindingIndex] = true;
					break;
				default:
					break;
			}
		}
	}

	// any indirect entry not in use which points to an undefined proxy can be removed
	for (size_t slot=0; slot < indirectUsed.size(); ++slot) {
		if ( !indirectUsed[slot] ) {
			const ld::Atom* atom = _indirectBindingTable[slot];
			if ( (atom != nullptr) && (atom->definition() == ld::Atom::definitionProxy) && (keep.count(atom) == 0) ) {
				const char* name = atom->name();
				_indirectBindingTable[slot] = NULL;
				_byNameReverseTable.erase(slot);
				_byNameTable.erase(name);
				allAtoms.erase(std::remove(allAtoms.begin(), allAtoms.end(), atom), allAtoms.end());
			}
			else if ( atom == nullptr ) {
				if ( const char* undefName = _byNameReverseTable[slot] ) {
					// <rdar://problem/55544746> Remove unused undef symbols from symbol table after LTO before doing final resolve
					_byNameReverseTable.erase(slot);
					_byNameTable.erase(undefName);
				}
			}
		}
	}

}

void SymbolTable::printStatistics()
{
//	fprintf(stderr, "cstring table size: %lu, bucket count: %lu, hash func called %u times\n", 
//				_cstringTable.size(), _cstringTable.bucket_count(), cstringHashCount);
	int count[11];
	for(unsigned int b=0; b < 11; ++b) {
		count[b] = 0;
	}
	for(unsigned int i=0; i < _cstringTable.bucket_count(); ++i) {
		unsigned int n = _cstringTable.bucket_size(i);
		if ( n < 10 ) 
			count[n] += 1;
		else
			count[10] += 1;
	}
	fprintf(stderr, "cstring table distribution\n");
	for(unsigned int b=0; b < 11; ++b) {
		fprintf(stderr, "%u buckets have %u elements\n", count[b], b);
	}
	fprintf(stderr, "indirect table size: %lu\n", _indirectBindingTable.size());
	fprintf(stderr, "by-name table size: %lu\n", _byNameTable.size());
//	fprintf(stderr, "by-content table size: %lu, hash count: %u, equals count: %u, lookup count: %u\n", 
//						_byContentTable.size(), contentHashCount, contentEqualCount, contentLookupCount);
//	fprintf(stderr, "by-ref table size: %lu, hashed count: %u, equals count: %u, lookup count: %u, insert count: %u\n", 
//						_byReferencesTable.size(), refHashCount, refEqualsCount, refLookupCount, refInsertCount);

	//ReferencesHash obj;
	//for(ReferencesHashToSlot::iterator it=_byReferencesTable.begin(); it != _byReferencesTable.end(); ++it) {
	//	if ( obj.operator()(it->first) == 0x2F3AC0EAC744EA70 ) {
	//		fprintf(stderr, "hash=0x2F3AC0EAC744EA70 for %p %s from %s\n", it->first, it->first->name(), it->first->safeFilePath());
	//	
	//	}
	//}
	
}

} // namespace tool 
} // namespace ld 

