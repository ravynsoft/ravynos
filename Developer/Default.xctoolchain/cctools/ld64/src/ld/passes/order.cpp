/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2009-2011 Apple Inc. All rights reserved.
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
#include <mach/machine.h>

#include <vector>
#include <map>
#include <set>
#include <unordered_map>

#include "ld.hpp"
#include "order.h"

namespace ld {
namespace passes {
namespace order {

//
// The purpose of this pass is to take the graph of all Atoms and produce an ordered
// sequence of atoms.  The constraints are that: 1) all Atoms of the same Segment must
// be contiguous, 2)  all Atoms of the same Section must be contigous, 3) Atoms specified
// in an order are sequenced as in the order file and before Atoms not specified,
// 4) Atoms in the same section from the same .o file should be contiguous and sequenced
// in the same order they were in the .o file, 5) Atoms in the same Section but which came
// from different .o files should be sequenced in the same order that the .o files
// were passed to the linker (i.e. command line order).
//
// The way this is implemented is that the linker passes a "base ordinal" to each File
// as it is constructed. Add each atom has an objectAddress() method. Then
// sorting is just sorting by section, then by file ordinal, then by object address.
//
// If an -order_file is specified, it gets more complicated.  First, an override-ordinal map
// is created.  It causes the sort routine to ignore the value returned by ordinal() and objectAddress() 
// and use the override value instead.  Next some Atoms must be laid out consecutively
// (e.g. hand written assembly that does not end with return, but rather falls into
// the next label).  This is modeled in via a kindNoneFollowOn fixup.  The use of
// kindNoneFollowOn fixups produces "clusters" of atoms that must stay together.
// If an order_file tries to move one atom, it may need to move a whole cluster.  The
// algorithm to do this models clusters using two maps.  The "starts" maps maps any
// atom in a cluster to the first Atom in the cluster.  The "nexts" maps an Atom in a
// cluster to the next Atom in the cluster.  With this in place, while processing an
// order_file, if any entry is in a cluster (in "starts" map), then the entire cluster is
// given ordinal overrides.
//

class Layout
{
public:
				Layout(const Options& opts, ld::Internal& state);
	void		doPass();
private:

	class Comparer {
	public:
					Comparer(const Layout& l, ld::Internal& s) : _layout(l), _state(s) {}
		bool		operator()(const ld::Atom* left, const ld::Atom* right);
	private:
		const Layout&	_layout;
		ld::Internal&	_state;
	};
				
	typedef std::unordered_map<std::string_view, const ld::Atom*> NameToAtom;
	
	typedef std::map<const ld::Atom*, const ld::Atom*> AtomToAtom;
	
	typedef std::map<const ld::Atom*, uint32_t> AtomToOrdinal;
	
	const ld::Atom*		findAtom(const Options::OrderedSymbol& orderedSymbol);
	void				buildNameTable();
	void				buildFollowOnTables();
	void				buildOrdinalOverrideMap();
	const ld::Atom*		follower(const ld::Atom* atom);
	static bool			matchesObjectFile(const ld::Atom* atom, const char* objectFileLeafName);
			bool		possibleToOrder(const ld::Internal::FinalSection*);
	
	const Options&						_options;
	ld::Internal&						_state;
	AtomToAtom							_followOnStarts;
	AtomToAtom							_followOnNexts;
	NameToAtom							_nameTable;
	std::vector<const ld::Atom*>		_nameCollisionAtoms;
	AtomToOrdinal						_ordinalOverrideMap;
	Comparer							_comparer;
	bool								_haveOrderFile;

	static bool							_s_log;
};

bool Layout::_s_log = false;

Layout::Layout(const Options& opts, ld::Internal& state)
	: _options(opts), _state(state), _comparer(*this, state), _haveOrderFile(opts.orderedSymbolsCount() != 0)
{
}


bool Layout::Comparer::operator()(const ld::Atom* left, const ld::Atom* right)
{
	if ( left == right )
		return false;

	// magic section$start symbol always sorts to the start of its section
	if ( left->contentType() == ld::Atom::typeSectionStart )
		return true;
	if ( right->contentType() == ld::Atom::typeSectionStart )
		return false;

	// if an -order_file is specified, then sorting is altered to sort those symbols first
	if ( _layout._haveOrderFile ) {
		AtomToOrdinal::const_iterator leftPos  = _layout._ordinalOverrideMap.find(left);
		AtomToOrdinal::const_iterator rightPos = _layout._ordinalOverrideMap.find(right);
		AtomToOrdinal::const_iterator end = _layout._ordinalOverrideMap.end();
		if ( leftPos != end ) {
			if ( rightPos != end ) {
				// both left and right are overridden, so compare overridden ordinals
				return leftPos->second < rightPos->second;
			}
			else {
				// left is overridden and right is not, so left < right
				return true;
			}
		}
		else {
			if ( rightPos != end ) {
				// right is overridden and left is not, so right < left
				return false;
			}
			else {
				// neither are overridden, 
				// fall into default sorting below
			}
		}
	}

	// magic section$end symbol always sorts to the end of its section
	if ( left->contentType() == ld::Atom::typeSectionEnd )
		return false;
	if ( right->contentType() == ld::Atom::typeSectionEnd )
		return true;

	// aliases sort before their target
	bool leftIsAlias = left->isAlias();
	if ( leftIsAlias ) {
		for (ld::Fixup::iterator fit=left->fixupsBegin(); fit != left->fixupsEnd(); ++fit) {
			const ld::Atom* target = NULL;
			if ( fit->kind == ld::Fixup::kindNoneFollowOn ) {
				switch ( fit->binding ) {
					case ld::Fixup::bindingsIndirectlyBound:
						target = _state.indirectBindingTable[fit->u.bindingIndex];
						break;
					case ld::Fixup::bindingDirectlyBound:
						target = fit->u.target;
						break;
                    default:
                        break;   
				}
			    if ( target == right )
					return true; // left already before right
				left = target; // sort as if alias was its target
				break;
		    }
		}
	}
	bool rightIsAlias = right->isAlias();
    if ( rightIsAlias ) {
        for (ld::Fixup::iterator fit=right->fixupsBegin(); fit != right->fixupsEnd(); ++fit) {
			const ld::Atom* target = NULL;
			if ( fit->kind == ld::Fixup::kindNoneFollowOn ) {
				switch ( fit->binding ) {
					case ld::Fixup::bindingsIndirectlyBound:
						target = _state.indirectBindingTable[fit->u.bindingIndex];
						break;
					case ld::Fixup::bindingDirectlyBound:
						target = fit->u.target;
						break;
                    default:
                        break;   
				}
			    if ( target == left )
                    return false; // need to swap, alias is after target
				right = target; // continue with sort as if right was target
                break;
			}       
		}
    }

	// the __common section can have real or tentative definitions
	// we want the real ones to sort before tentative ones
	bool leftIsTent  =  (left->definition() == ld::Atom::definitionTentative);
	bool rightIsTent =  (right->definition() == ld::Atom::definitionTentative);
	if ( leftIsTent != rightIsTent )
		return rightIsTent; 

#if 0	
	// initializers are auto sorted to start of section
	if ( !fInitializerSet.empty() ) {
		bool leftFirst  = (fInitializerSet.count(left) != 0);
		bool rightFirst = (fInitializerSet.count(right) != 0);
		if ( leftFirst != rightFirst ) 
			return leftFirst;
	}

	// terminators are auto sorted to end of section
	if ( !fTerminatorSet.empty() ) {
		bool leftLast  = (fTerminatorSet.count(left) != 0);
		bool rightLast = (fTerminatorSet.count(right) != 0);
		if ( leftLast != rightLast ) 
			return rightLast;
	}
#endif

	// sort cold functions to end
	bool leftCold  = left->cold();
	bool rightCold = right->cold();
	if ( leftCold != rightCold )
		return rightCold;
	
	// sort by .o order
	const ld::File* leftFile = left->file();
	const ld::File* rightFile = right->file();
	// <rdar://problem/10830126> properly sort if on file is NULL and the other is not
	ld::File::Ordinal leftFileOrdinal = (leftFile != NULL) ? leftFile->ordinal() : ld::File::Ordinal::NullOrdinal();
	ld::File::Ordinal rightFileOrdinal = (rightFile != NULL) ? rightFile->ordinal() : ld::File::Ordinal::NullOrdinal();
	if ( leftFileOrdinal != rightFileOrdinal )
		return leftFileOrdinal< rightFileOrdinal;

	// tentative definitions have no address in .o file, they are traditionally laid out by name
	if ( leftIsTent && rightIsTent )
		return left->getUserVisibleName() < right->getUserVisibleName();

	// lastly sort by atom address
	int64_t addrDiff = left->objectAddress() - right->objectAddress();
	if ( addrDiff == 0 ) {
		// have same address so one might be an alias, and aliases need to sort before target
		if ( leftIsAlias != rightIsAlias )
			return leftIsAlias;

		// both at same address, sort by name
		return left->getUserVisibleName() < right->getUserVisibleName();
	}
	return (addrDiff < 0);
}

bool Layout::matchesObjectFile(const ld::Atom* atom, const char* objectFileLeafName)
{
	if ( objectFileLeafName == NULL )
		return true;
	const char* atomFullPath = atom->file()->path();
	const char* lastSlash = strrchr(atomFullPath, '/');
	if ( lastSlash != NULL ) {
		if ( strcmp(&lastSlash[1], objectFileLeafName) == 0 )
			return true;
	}
	else {
		if ( strcmp(atomFullPath, objectFileLeafName) == 0 )
			return true;
	}
	return false;
}


bool Layout::possibleToOrder(const ld::Internal::FinalSection* sect)
{
	// atoms in only some sections can have order_file applied
	switch ( sect->type() ) {
		case ld::Section::typeUnclassified:
		case ld::Section::typeCode:
		case ld::Section::typeZeroFill:
			return true;
		case ld::Section::typeImportProxies:
			return false;
		default:
			// if section has command line aliases, then we must apply ordering so aliases layout before targets
			if ( _options.haveCmdLineAliases() ) {
				for (std::vector<const ld::Atom*>::const_iterator ait=sect->atoms.begin(); ait != sect->atoms.end(); ++ait) {
					const ld::Atom* atom = *ait;
					if ( atom->isAlias() )
						return true;
				}
			}
			break;
	}
	return false;
}

void Layout::buildNameTable()
{
	for (std::vector<ld::Internal::FinalSection*>::iterator sit=_state.sections.begin(); sit != _state.sections.end(); ++sit) {
		ld::Internal::FinalSection* sect = *sit;
		// some sections are not worth scanning for names
		if ( ! possibleToOrder(sect) )
			continue;
		for (std::vector<const ld::Atom*>::iterator ait=sect->atoms.begin(); ait != sect->atoms.end(); ++ait) {
			const ld::Atom* atom = *ait;
			if ( atom->symbolTableInclusion() == ld::Atom::symbolTableIn ) {
				auto name = atom->getUserVisibleName();
				if ( name.size() != 0 ) {
					// static function or data
					NameToAtom::iterator pos = _nameTable.find(name);
					if ( pos == _nameTable.end() )
						_nameTable[name] = atom;
					else {
						const ld::Atom* existing = _nameTable[name];
						if ( existing != NULL ) {
							_nameCollisionAtoms.push_back(existing);
							_nameTable[name] = NULL;	// collision, denote with NULL
						}
						_nameCollisionAtoms.push_back(atom);
					}
				}
			}
		}
	}
	if ( _s_log ) {
		fprintf(stderr, "buildNameTable() _nameTable:\n");
		for(NameToAtom::iterator it=_nameTable.begin(); it != _nameTable.end(); ++it)
			fprintf(stderr, "  %p <- %s\n", it->second, std::string(it->first).c_str());
		fprintf(stderr, "buildNameTable() _nameCollisionAtoms:\n");
		for(std::vector<const ld::Atom*>::iterator it=_nameCollisionAtoms.begin(); it != _nameCollisionAtoms.end(); ++it)
			fprintf(stderr, "  %p, %s\n", *it, std::string((*it)->getUserVisibleName()).c_str());
	}
}


const ld::Atom* Layout::findAtom(const Options::OrderedSymbol& orderedSymbol)
{
	// look for name in _nameTable
	NameToAtom::iterator pos = _nameTable.find(orderedSymbol.symbolName);
	if ( pos != _nameTable.end() ) {
		if ( (pos->second != NULL) && matchesObjectFile(pos->second, orderedSymbol.objectFileName) ) {
			//fprintf(stderr, "found %s in hash table\n", orderedSymbol.symbolName);
			return pos->second;
		}
		if ( pos->second == NULL ) {
			// name is in hash table, but atom is NULL, so that means there are duplicates, so we use super slow way
			if ( ( orderedSymbol.objectFileName == NULL) && _options.printOrderFileStatistics() ) {
				warning("%s specified in order_file but it exists in multiple .o files. "
						"Prefix symbol with .o filename in order_file to disambiguate", orderedSymbol.symbolName);
			}
			for (std::vector<const ld::Atom*>::iterator it=_nameCollisionAtoms.begin(); it != _nameCollisionAtoms.end(); it++) {
				const ld::Atom* atom = *it;
				if ( atom->getUserVisibleName() == std::string_view(orderedSymbol.symbolName) ) {
					if ( matchesObjectFile(atom, orderedSymbol.objectFileName) ) {
						return atom;
					}
				}
			}
		}
	}
		
	return NULL;
}

const ld::Atom* Layout::follower(const ld::Atom* atom)
{
	for (const ld::Atom* a = _followOnStarts[atom]; a != NULL; a = _followOnNexts[a]) {
		assert(a != NULL);
		if ( _followOnNexts[a] == atom ) {
			return a;
		}
	}
	// no follower, first in chain
	return NULL;
}

void Layout::buildFollowOnTables()
{
	// if no -order_file, then skip building follow on table
	if ( ! _haveOrderFile )
		return;

	// first make a pass to find all follow-on references and build start/next maps
	// which are a way to represent clusters of atoms that must layout together
	for (std::vector<ld::Internal::FinalSection*>::iterator sit=_state.sections.begin(); sit != _state.sections.end(); ++sit) {
		ld::Internal::FinalSection* sect = *sit;
		if ( !possibleToOrder(sect) ) 
			continue;
		for (std::vector<const ld::Atom*>::iterator ait=sect->atoms.begin(); ait != sect->atoms.end(); ++ait) {
			const ld::Atom* atom = *ait;
			for (ld::Fixup::iterator fit = atom->fixupsBegin(), end=atom->fixupsEnd(); fit != end; ++fit) {
				if ( fit->kind == ld::Fixup::kindNoneFollowOn ) {
					assert(fit->binding == ld::Fixup::bindingDirectlyBound);
					const ld::Atom* followOnAtom = fit->u.target;
					if ( _s_log ) fprintf(stderr, "ref %p %s -> %p %s\n", atom, atom->name(), followOnAtom, followOnAtom->name());
					assert(_followOnNexts.count(atom) == 0);
					_followOnNexts[atom] = followOnAtom;
					if ( _followOnStarts.count(atom) == 0 ) {
						// first time atom has been seen, make it start of chain
						_followOnStarts[atom] = atom;
						if ( _s_log ) fprintf(stderr, "  start %s -> %s\n", atom->name(), atom->name());
					}
					if ( _followOnStarts.count(followOnAtom) == 0 ) {
						// first time followOnAtom has been seen, make atom start of chain
						_followOnStarts[followOnAtom] = _followOnStarts[atom];
						if ( _s_log ) fprintf(stderr, "  start %s -> %s\n", followOnAtom->name(), _followOnStarts[atom]->name());
					}
					else {
						if ( _followOnStarts[followOnAtom] == followOnAtom ) {
							// followOnAtom atom already start of another chain, hook together 
							// and change all to use atom as start
							const ld::Atom* a = followOnAtom;
							while ( true ) {
								assert(_followOnStarts[a] == followOnAtom);
								_followOnStarts[a] = _followOnStarts[atom];
								if ( _s_log ) fprintf(stderr, "  adjust start for %s -> %s\n", a->name(), _followOnStarts[atom]->name());
								AtomToAtom::iterator pos = _followOnNexts.find(a);
								if ( pos != _followOnNexts.end() )
									a = pos->second;
								else
									break;
							}
						}
						else {
							// attempt to insert atom into existing followOn chain
							const ld::Atom* curPrevToFollowOnAtom = this->follower(followOnAtom);
							assert(curPrevToFollowOnAtom != NULL);
							assert((atom->size() == 0) || (curPrevToFollowOnAtom->size() == 0));
							if ( atom->size() == 0 ) {
								// insert alias into existing chain right before followOnAtom
								_followOnNexts[curPrevToFollowOnAtom] = atom;
								_followOnNexts[atom] = followOnAtom;
								_followOnStarts[atom] = _followOnStarts[followOnAtom];
							}
							else {
								// insert real atom into existing chain right before alias of followOnAtom
								const ld::Atom* curPrevPrevToFollowOn = this->follower(curPrevToFollowOnAtom);
								if ( curPrevPrevToFollowOn == NULL ) {
									// nothing previous, so make this a start of a new chain
									_followOnNexts[atom] = curPrevToFollowOnAtom;
									for (const ld::Atom* a = atom; a != NULL; a = _followOnNexts[a]) {
										if ( _s_log ) fprintf(stderr, "  adjust start for %s -> %s\n", a->name(), atom->name());
										_followOnStarts[a] = atom;
									}
								}
								else {
									// is previous, insert into existing chain before previous
									_followOnNexts[curPrevPrevToFollowOn] = atom;
									_followOnNexts[atom] = curPrevToFollowOnAtom;
									_followOnStarts[atom] = _followOnStarts[curPrevToFollowOnAtom];
								}
							}
						}
					}
				}
			}
		}
	}

	if ( _s_log ) {
		for(AtomToAtom::iterator it = _followOnStarts.begin(); it != _followOnStarts.end(); ++it)
			fprintf(stderr, "start %s -> %s\n", it->first->name(), it->second->name());

		for(AtomToAtom::iterator it = _followOnNexts.begin(); it != _followOnNexts.end(); ++it)
			fprintf(stderr, "next %s -> %s\n", it->first->name(), (it->second != NULL) ? it->second->name() : "null");
	}
}


class InSet
{
public:
	InSet(const std::set<const ld::Atom*>& theSet) : _set(theSet)  {}

	bool operator()(const ld::Atom* atom) const {
		return ( _set.count(atom) != 0 );
	}
private:
	const std::set<const ld::Atom*>&  _set;
};


void Layout::buildOrdinalOverrideMap()
{
	// if no -order_file, then skip building override map
	if ( ! _haveOrderFile )
		return;

	// build fast name->atom table
	this->buildNameTable();

	// handle .o files that cannot have their atoms rearranged
	// with the start/next maps of follow-on atoms we can process the order file and produce override ordinals
	uint32_t index = 0;
	uint32_t matchCount = 0;
	std::set<const ld::Atom*> moveToData;
	for(Options::OrderedSymbolsIterator it = _options.orderedSymbolsBegin(); it != _options.orderedSymbolsEnd(); ++it) {
		const ld::Atom* atom = this->findAtom(*it);
		if ( atom != NULL ) {
			// <rdar://problem/8612550> When order file used on data, turn ordered zero fill symbols into zero data
			switch ( atom->section().type() ) {
				case ld::Section::typeZeroFill:
				case ld::Section::typeTentativeDefs:
					if ( atom->size() <= 512 ) {
						const char* dstSeg;
						bool wildCardMatch;
						const ld::File* f = atom->file();
						const char* path = (f != NULL) ? f->path() : NULL;
						if ( !_options.moveRwSymbol(atom->getUserVisibleName(), path, dstSeg, wildCardMatch) )
							moveToData.insert(atom);
					}
					break;
				default:
					break;
			}
		
			AtomToAtom::iterator start = _followOnStarts.find(atom);
			if ( start != _followOnStarts.end() ) {
				// this symbol for the order file corresponds to an atom that is in a cluster that must lay out together
				for(const ld::Atom* nextAtom = start->second; nextAtom != NULL; nextAtom = _followOnNexts[nextAtom]) {
					AtomToOrdinal::iterator pos = _ordinalOverrideMap.find(nextAtom);
					if ( pos == _ordinalOverrideMap.end() ) {
						_ordinalOverrideMap[nextAtom] = index++;
						if (_s_log ) fprintf(stderr, "override ordinal %u assigned to %s in cluster from %s\n", index, nextAtom->name(), nextAtom->safeFilePath());
					}
					else {
						if (_s_log ) fprintf(stderr, "could not order %s as %u because it was already laid out earlier by %s as %u\n",
										atom->name(), index, _followOnStarts[atom]->name(), _ordinalOverrideMap[atom] );
					}
				}
			}
			else {
				_ordinalOverrideMap[atom] = index;
				if (_s_log ) fprintf(stderr, "override ordinal %u assigned to %s from %s\n", index, atom->name(), atom->safeFilePath());
			}
			++matchCount;
		}
		else {
			if ( _options.printOrderFileStatistics() ) {
				if ( it->objectFileName == NULL )
					warning("can't find match for order_file entry: %s", it->symbolName);
				else
					warning("can't find match for order_file entry: %s/%s", it->objectFileName, it->symbolName);
			}
		}
		 ++index;
	}
	if ( _options.printOrderFileStatistics() && (_options.orderedSymbolsCount() != matchCount) ) {
		warning("only %u out of %lu order_file symbols were applicable", matchCount, _options.orderedSymbolsCount() );
	}

	// <rdar://problem/8612550> When order file used on data, turn ordered zero fill symbols into zeroed data
	if ( ! moveToData.empty() ) {
		// <rdar://problem/14919139> only move zero fill symbols to __data if there is a __data section
		ld::Internal::FinalSection* dataSect = NULL;
		for (std::vector<ld::Internal::FinalSection*>::iterator sit=_state.sections.begin(); sit != _state.sections.end(); ++sit) {
			ld::Internal::FinalSection* sect = *sit;
			if ( sect->type() == ld::Section::typeUnclassified ) {
				if ( (strcmp(sect->sectionName(), "__data") == 0) && (strcmp(sect->segmentName(), "__DATA") == 0) )
					dataSect = sect;
			}
		}

		if ( dataSect != NULL ) {
			// add atoms to __data
			dataSect->atoms.insert(dataSect->atoms.end(), moveToData.begin(), moveToData.end());
			// remove atoms from original sections
			for (std::vector<ld::Internal::FinalSection*>::iterator sit=_state.sections.begin(); sit != _state.sections.end(); ++sit) {
				ld::Internal::FinalSection* sect = *sit;
				switch ( sect->type() ) {
					case ld::Section::typeZeroFill:
					case ld::Section::typeTentativeDefs:
						sect->atoms.erase(std::remove_if(sect->atoms.begin(), sect->atoms.end(), InSet(moveToData)), sect->atoms.end());
						break;
					default:
						break;
				}
			}
			// update atom-to-section map
			for (std::set<const ld::Atom*>::iterator it=moveToData.begin(); it != moveToData.end(); ++it) {
				_state.atomToSection[*it] = dataSect;
			}
		}
	}

}

void Layout::doPass()
{
	const bool log = false;
	if ( log ) {
		fprintf(stderr, "Unordered atoms:\n");
		for (std::vector<ld::Internal::FinalSection*>::iterator sit=_state.sections.begin(); sit != _state.sections.end(); ++sit) {
			ld::Internal::FinalSection* sect = *sit;
			for (std::vector<const ld::Atom*>::iterator ait=sect->atoms.begin(); ait != sect->atoms.end(); ++ait) {
				const ld::Atom* atom = *ait;
				fprintf(stderr, "\t%p\t%s\t%s\n", atom, sect->sectionName(), atom->name());
			}
		}
	}
	
	// handle .o files that cannot have their atoms rearranged
	this->buildFollowOnTables();

	// assign new ordinal value to all ordered atoms
	this->buildOrdinalOverrideMap();

	// sort atoms in each section
	for (std::vector<ld::Internal::FinalSection*>::iterator sit=_state.sections.begin(); sit != _state.sections.end(); ++sit) {
		ld::Internal::FinalSection* sect = *sit;
		switch ( sect->type() ) {
			case ld::Section::typeTempAlias:
			case ld::Section::typeStub:
			case ld::Section::typeLazyPointer:
			case ld::Section::typeLazyPointerClose:
			case ld::Section::typeStubClose:
			case ld::Section::typeNonLazyPointer:
				// these sections are already sorted by pass that created them
				break;
			default:
				if ( log ) fprintf(stderr, "sorting section %s\n", sect->sectionName());
				std::sort(sect->atoms.begin(), sect->atoms.end(), _comparer);
				break;
		}
	}

	if ( log ) {
		fprintf(stderr, "Sorted atoms:\n");
		for (std::vector<ld::Internal::FinalSection*>::iterator sit=_state.sections.begin(); sit != _state.sections.end(); ++sit) {
			ld::Internal::FinalSection* sect = *sit;
			for (std::vector<const ld::Atom*>::iterator ait=sect->atoms.begin(); ait != sect->atoms.end(); ++ait) {
				const ld::Atom* atom = *ait;
				fprintf(stderr, "\t%p\t%s\t%s\n", atom, sect->sectionName(), atom->name());
			}
		}
	}
}


void doPass(const Options& opts, ld::Internal& state)
{
	Layout layout(opts, state);
	layout.doPass();
}


} // namespace order_file
} // namespace passes 
} // namespace ld 
