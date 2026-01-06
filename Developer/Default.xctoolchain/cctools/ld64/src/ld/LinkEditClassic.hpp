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

#ifndef __LINKEDIT_CLASSIC_HPP__
#define __LINKEDIT_CLASSIC_HPP__

#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>

#include <vector>
#include <unordered_map>

#include "Options.h"
#include "ld.hpp"
#include "Architectures.hpp"
#include "MachOFileAbstraction.hpp"

namespace ld {
namespace tool {



class ClassicLinkEditAtom : public ld::Atom
{
public:

	// overrides of ld::Atom
	virtual ld::File*							file() const		{ return NULL; }
	virtual uint64_t							objectAddress() const { return 0; }

	virtual void								encode() = 0;
	virtual bool								hasStabs(uint32_t& ssos, uint32_t& ssoe, uint32_t& sos, uint32_t& soe) { return false; }

												ClassicLinkEditAtom(const Options& opts, ld::Internal& state, 
																OutputFile& writer, const ld::Section& sect,
																unsigned int pointerSize)
												: ld::Atom(sect, ld::Atom::definitionRegular,
															ld::Atom::combineNever, ld::Atom::scopeTranslationUnit,
															ld::Atom::typeUnclassified, ld::Atom::symbolTableNotIn,
															false, false, false, ld::Atom::Alignment(log2(pointerSize))), 
														_options(opts), _state(state), _writer(writer) { }
protected:	
	const Options&				_options;
	ld::Internal&				_state;
	OutputFile&					_writer;
};



class StringPoolAtom : public ClassicLinkEditAtom
{
public:
												StringPoolAtom(const Options& opts, ld::Internal& state, 
																OutputFile& writer, int pointerSize);

	// overrides of ld::Atom
	virtual const char*							name() const		{ return "string pool"; }
	virtual uint64_t							size() const;
	virtual void								copyRawContent(uint8_t buffer[]) const; 
	// overrides of ClassicLinkEditAtom
	virtual void								encode() { }

	int32_t										add(const char* name);
	int32_t										addUnique(const char* name);
	int32_t										emptyString()			{ return 1; }
	const char*									stringForIndex(int32_t) const;
	uint32_t									currentOffset();

private:
	enum { kBufferSize = 0x01000000 };
	typedef std::unordered_map<std::string, int32_t> StringToOffset;

	const uint32_t							_pointerSize;
	std::vector<char*>						_fullBuffers;
	char*									_currentBuffer;
	uint32_t								_currentBufferUsed;
	StringToOffset							_uniqueStrings;

	static ld::Section			_s_section;
};

ld::Section StringPoolAtom::_s_section("__LINKEDIT", "__string_pool", ld::Section::typeLinkEdit, true);


StringPoolAtom::StringPoolAtom(const Options& opts, ld::Internal& state, OutputFile& writer, int pointerSize)
	: ClassicLinkEditAtom(opts, state, writer, _s_section, pointerSize), 
	 _pointerSize(pointerSize), _currentBuffer(NULL), _currentBufferUsed(0)
{
	_currentBuffer = new char[kBufferSize];
	// burn first byte of string pool (so zero is never a valid string offset)
	_currentBuffer[_currentBufferUsed++] = ' ';
	// make offset 1 always point to an empty string
	_currentBuffer[_currentBufferUsed++] = '\0';
}

uint64_t StringPoolAtom::size() const
{
	// pointer size align size
	return (kBufferSize * _fullBuffers.size() + _currentBufferUsed + _pointerSize-1) & (-_pointerSize);
}

void StringPoolAtom::copyRawContent(uint8_t buffer[]) const
{
	uint64_t offset = 0;
	for (unsigned int i=0; i < _fullBuffers.size(); ++i) {
		memcpy(&buffer[offset], _fullBuffers[i], kBufferSize);
		offset += kBufferSize;
	}
	memcpy(&buffer[offset], _currentBuffer, _currentBufferUsed);
	// zero fill end to align
	offset += _currentBufferUsed;
	while ( (offset % _pointerSize) != 0 )
		buffer[offset++] = 0;
}

int32_t StringPoolAtom::add(const char* str)
{
	int32_t offset = kBufferSize * _fullBuffers.size() + _currentBufferUsed;
	int lenNeeded = strlcpy(&_currentBuffer[_currentBufferUsed], str, kBufferSize-_currentBufferUsed)+1;
	if ( (_currentBufferUsed+lenNeeded) < kBufferSize ) {
		_currentBufferUsed += lenNeeded;
	}
	else {
		int copied = kBufferSize-_currentBufferUsed-1;
		// change trailing '\0' that strlcpy added to real char
		_currentBuffer[kBufferSize-1] = str[copied];
		// alloc next buffer
		_fullBuffers.push_back(_currentBuffer);
		_currentBuffer = new char[kBufferSize];
		_currentBufferUsed = 0;
		// append rest of string
		this->add(&str[copied+1]);
	}
	return offset;
}

uint32_t StringPoolAtom::currentOffset()
{
	return kBufferSize * _fullBuffers.size() + _currentBufferUsed;
}


int32_t StringPoolAtom::addUnique(const char* str)
{
	StringToOffset::iterator pos = _uniqueStrings.find(str);
	if ( pos != _uniqueStrings.end() ) {
		return pos->second;
	}
	else {
		int32_t offset = this->add(str);
		_uniqueStrings[str] = offset;
		return offset;
	}
}


const char* StringPoolAtom::stringForIndex(int32_t index) const
{
	int32_t currentBufferStartIndex = kBufferSize * _fullBuffers.size();
	int32_t maxIndex = currentBufferStartIndex + _currentBufferUsed;
	// check for out of bounds
	if ( index > maxIndex )
		return "";
	// check for index in _currentBuffer
	if ( index > currentBufferStartIndex )
		return &_currentBuffer[index-currentBufferStartIndex];
	// otherwise index is in a full buffer
	uint32_t fullBufferIndex = index/kBufferSize;
	return &_fullBuffers[fullBufferIndex][index-(kBufferSize*fullBufferIndex)];
}



template <typename A>
class SymbolTableAtom : public ClassicLinkEditAtom
{
public:
												SymbolTableAtom(const Options& opts, ld::Internal& state, OutputFile& writer)
													: ClassicLinkEditAtom(opts, state, writer, _s_section, sizeof(pint_t)),
														_stabsStringsOffsetStart(0), _stabsStringsOffsetEnd(0),
														_stabsIndexStart(0), _stabsIndexEnd(0) { }

	// overrides of ld::Atom
	virtual const char*							name() const		{ return "symbol table"; }
	virtual uint64_t							size() const;
	virtual void								copyRawContent(uint8_t buffer[]) const; 
	// overrides of ClassicLinkEditAtom
	virtual void								encode();
	virtual bool								hasStabs(uint32_t& ssos, uint32_t& ssoe, uint32_t& sos, uint32_t& soe);

private:
	typedef typename A::P						P;
	typedef typename A::P::E					E;
	typedef typename A::P::uint_t				pint_t;

	bool							addLocal(const ld::Atom* atom, StringPoolAtom* pool);
	void							addGlobal(const ld::Atom* atom, StringPoolAtom* pool);
	void							addImport(const ld::Atom* atom, StringPoolAtom* pool);
	uint8_t							classicOrdinalForProxy(const ld::Atom* atom);
	uint32_t						stringOffsetForStab(const ld::relocatable::File::Stab& stab, StringPoolAtom* pool);
	uint64_t						valueForStab(const ld::relocatable::File::Stab& stab);
	uint8_t							sectionIndexForStab(const ld::relocatable::File::Stab& stab);
	bool							isAltEntry(const ld::Atom* atom);

	mutable std::vector<macho_nlist<P> >	_globals;
	mutable std::vector<macho_nlist<P> >	_locals;
	mutable std::vector<macho_nlist<P> >	_imports;
	
	uint32_t								_stabsStringsOffsetStart;
	uint32_t								_stabsStringsOffsetEnd;
	uint32_t								_stabsIndexStart;
	uint32_t								_stabsIndexEnd;

	static ld::Section			_s_section;
	static int					_s_anonNameIndex;

};

template <typename A>
ld::Section SymbolTableAtom<A>::_s_section("__LINKEDIT", "__symbol_table", ld::Section::typeLinkEdit, true);

template <typename A>
int	 SymbolTableAtom<A>::_s_anonNameIndex = 1;

static bool chainLeadsTo(const ld::Atom* startAtom, const ld::Atom* targetAtom)
{
	if ( startAtom == targetAtom )
		return true;

	for (ld::Fixup::iterator fit = startAtom->fixupsBegin(); fit != startAtom->fixupsEnd(); ++fit) {
		if ( (fit->kind == ld::Fixup::kindNoneFollowOn) && (fit->binding == Fixup::bindingDirectlyBound) ) {
			const Atom* nextAtom = fit->u.target;
			assert(nextAtom != NULL);
			if ( chainLeadsTo(nextAtom, targetAtom) )
				return true;
		}
	}
	return false;
}

template <typename A>
bool SymbolTableAtom<A>::isAltEntry(const ld::Atom* atom) 
{
	// alt entries have a group subordinate reference to the previous atom
	for (ld::Fixup::iterator fit = atom->fixupsBegin(); fit != atom->fixupsEnd(); ++fit) {
		if ( (fit->kind == ld::Fixup::kindNoneGroupSubordinate) && (fit->binding == Fixup::bindingDirectlyBound) ) {
			const Atom* chainStart = fit->u.target;
			assert(chainStart != NULL);
			if ( chainLeadsTo(chainStart, atom) ) {
				return true;
			}
		}
	}
	return false;
}

template <typename A>
bool SymbolTableAtom<A>::addLocal(const ld::Atom* atom, StringPoolAtom* pool) 
{
	macho_nlist<P> entry;
	assert(atom->symbolTableInclusion() != ld::Atom::symbolTableNotIn);
	 
	// set n_strx
	std::string nameStr = std::string(atom->getUserVisibleName());
	const char* symbolName = nameStr.c_str();
	char anonName[32];
	if ( this->_options.outputKind() == Options::kObjectFile ) {
		if ( atom->contentType() == ld::Atom::typeCString ) {
			if ( atom->combine() == ld::Atom::combineByNameAndContent ) {
				// don't use 'l' labels for x86_64 strings
				// <rdar://problem/6605499> x86_64 obj-c runtime confused when static lib is stripped
				sprintf(anonName, "LC%u", _s_anonNameIndex++);
				symbolName = anonName;
			}
		}
		else if ( atom->contentType() == ld::Atom::typeCFI ) {
			if ( _options.removeEHLabels() )
				return false;
			// synthesize .eh name
			if ( strcmp(atom->name(), "CIE") == 0 )
				symbolName = "EH_Frame1";
			else
				symbolName = "func.eh";
		}
		else if ( atom->symbolTableInclusion() == ld::Atom::symbolTableInWithRandomAutoStripLabel ) {
			// make auto-strip anonymous name for symbol 
			sprintf(anonName, "l%03u", _s_anonNameIndex++);
			symbolName = anonName;
		}
	}

	// <rdar://problem/43388350> ER: Coalesce the string pools for the symbol table when linking objects together
	entry.set_n_strx(pool->addUnique(symbolName));

	// set n_type
	uint8_t type = N_SECT;
	if ( atom->definition() == ld::Atom::definitionAbsolute ) {
		type = N_ABS;
	}
	else if ( (atom->section().type() == ld::Section::typeObjC1Classes) 
				&& (this->_options.outputKind() == Options::kObjectFile) ) {
		// __OBJC __class has floating abs symbols for each class data structure
		type = N_ABS;
	}
	if ( atom->scope() == ld::Atom::scopeLinkageUnit )
		type |= N_PEXT;
	entry.set_n_type(type);

	// set n_sect (section number of implementation )
	if ( atom->definition() == ld::Atom::definitionAbsolute )
		entry.set_n_sect(0);
	else
		entry.set_n_sect(atom->machoSection());

	// set n_desc
	uint16_t desc = 0;
    if ( atom->symbolTableInclusion() == ld::Atom::symbolTableInAndNeverStrip )
        desc |= REFERENCED_DYNAMICALLY;
    if ( atom->dontDeadStrip() && (this->_options.outputKind() == Options::kObjectFile) )
        desc |= N_NO_DEAD_STRIP;
    if ( (atom->definition() == ld::Atom::definitionRegular) && (atom->combine() == ld::Atom::combineByName) )
		desc |= N_WEAK_DEF;
	if ( atom->isThumb() )
		desc |= N_ARM_THUMB_DEF;
    if ( (this->_options.outputKind() == Options::kObjectFile) && this->_state.allObjectFilesScatterable && isAltEntry(atom) )
        desc |= N_ALT_ENTRY;
    entry.set_n_desc(desc);

	// set n_value ( address this symbol will be at if this executable is loaded at it preferred address )
	if ( atom->definition() == ld::Atom::definitionAbsolute ) 
		entry.set_n_value(atom->objectAddress());
	else
		entry.set_n_value(atom->finalAddress());
	
	// add to array
	_locals.push_back(entry);
	return true;
}


template <typename A>
void SymbolTableAtom<A>::addGlobal(const ld::Atom* atom, StringPoolAtom* pool) 
{
	macho_nlist<P> entry;

	// set n_strx
	const char* symbolName = atom->name();
	char anonName[32];
	if ( this->_options.outputKind() == Options::kObjectFile ) {
		if ( atom->symbolTableInclusion() == ld::Atom::symbolTableInWithRandomAutoStripLabel ) {
			// make auto-strip anonymous name for symbol 
			sprintf(anonName, "l%03u", _s_anonNameIndex++);
			symbolName = anonName;
		}
	}
	entry.set_n_strx(pool->add(symbolName));

	// set n_type
	if ( atom->definition() == ld::Atom::definitionAbsolute ) {
		entry.set_n_type(N_EXT | N_ABS);
	}
	else if ( (atom->section().type() == ld::Section::typeObjC1Classes)
				&& (this->_options.outputKind() == Options::kObjectFile) ) {
		// __OBJC __class has floating abs symbols for each class data structure
		entry.set_n_type(N_EXT | N_ABS);
	}
	else if ( (atom->definition() == ld::Atom::definitionProxy) && (atom->scope() == ld::Atom::scopeGlobal) ) {
		entry.set_n_type(N_EXT | N_INDR);
	}
	else {
		entry.set_n_type(N_EXT | N_SECT);
		if ( (atom->scope() == ld::Atom::scopeLinkageUnit) && (this->_options.outputKind() == Options::kObjectFile) ) {
			if ( this->_options.keepPrivateExterns() )
				entry.set_n_type(N_EXT | N_SECT | N_PEXT);
		}
		else if ( (atom->symbolTableInclusion() == ld::Atom::symbolTableInAndNeverStrip)
					&& (atom->section().type() == ld::Section::typeMachHeader) 
					&& !_options.positionIndependentExecutable() ) {
			// the __mh_execute_header is historical magic in non-pie executabls and must be an absolute symbol
			entry.set_n_type(N_EXT | N_ABS);
		}
	}

	// set n_sect (section number of implementation)
	if ( atom->definition() == ld::Atom::definitionAbsolute )
		entry.set_n_sect(0);
	else if ( (atom->definition() == ld::Atom::definitionProxy) && (atom->scope() == ld::Atom::scopeGlobal) )
		entry.set_n_sect(0); 
	else
		entry.set_n_sect(atom->machoSection());

	// set n_desc
	uint16_t desc = 0;
    if ( atom->isThumb() )
        desc |= N_ARM_THUMB_DEF;
    if ( atom->symbolTableInclusion() == ld::Atom::symbolTableInAndNeverStrip )
        desc |= REFERENCED_DYNAMICALLY;
    if ( (atom->contentType() == ld::Atom::typeResolver) && (this->_options.outputKind() == Options::kObjectFile) )
        desc |= N_SYMBOL_RESOLVER;
    if ( atom->dontDeadStrip() && (this->_options.outputKind() == Options::kObjectFile) )
        desc |= N_NO_DEAD_STRIP;
    if ( (this->_options.outputKind() == Options::kObjectFile) && this->_state.allObjectFilesScatterable && isAltEntry(atom) )
        desc |= N_ALT_ENTRY;
    if ( (this->_options.outputKind() == Options::kObjectFile) && atom->cold() )
        desc |= N_COLD_FUNC;
    if ( (atom->definition() == ld::Atom::definitionRegular) && (atom->combine() == ld::Atom::combineByName) ) {
		desc |= N_WEAK_DEF;
		// <rdar://problem/6783167> support auto hidden weak symbols: .weak_def_can_be_hidden
		if ( (atom->scope() == ld::Atom::scopeGlobal) && atom->autoHide() && (this->_options.outputKind() == Options::kObjectFile) ) 
			desc |= N_WEAK_REF;
	}
	entry.set_n_desc(desc);

	// set n_value ( address this symbol will be at if this executable is loaded at it preferred address )
	if ( atom->definition() == ld::Atom::definitionAbsolute ) 
		entry.set_n_value(atom->objectAddress());
	else if ( (atom->definition() == ld::Atom::definitionProxy) && (atom->scope() == ld::Atom::scopeGlobal) ) {
		if ( atom->isAlias() ) {
			// this re-export also renames
			for (ld::Fixup::iterator fit = atom->fixupsBegin(); fit != atom->fixupsEnd(); ++fit) {
				if ( fit->kind == ld::Fixup::kindNoneFollowOn ) {
					assert(fit->binding == ld::Fixup::bindingDirectlyBound);
					entry.set_n_value(pool->add(fit->u.target->name()));
				}
			}
		}
		else
			entry.set_n_value(entry.n_strx());
	}
	else
		entry.set_n_value(atom->finalAddress());
		
	// add to array
	_globals.push_back(entry);
}

template <typename A>
uint8_t	SymbolTableAtom<A>::classicOrdinalForProxy(const ld::Atom* atom)
{
	assert(atom->definition() == ld::Atom::definitionProxy);
	// when linking for flat-namespace ordinals are always zero 
	if ( _options.nameSpace() != Options::kTwoLevelNameSpace )
		return 0;
	const ld::dylib::File* dylib = dynamic_cast<const ld::dylib::File*>(atom->file());
	// when linking -undefined dynamic_lookup, unbound symbols use DYNAMIC_LOOKUP_ORDINAL
	if ( dylib == NULL ) {
		if (_options.undefinedTreatment() == Options::kUndefinedDynamicLookup )
			return DYNAMIC_LOOKUP_ORDINAL;
		if (_options.allowedUndefined(atom->name()) )
			return DYNAMIC_LOOKUP_ORDINAL;
	}
	assert(dylib != NULL);
	int ord = this->_writer.dylibToOrdinal(dylib);
	if ( ord == BIND_SPECIAL_DYLIB_MAIN_EXECUTABLE )
		return EXECUTABLE_ORDINAL;
	return ord;
}


template <typename A>
void SymbolTableAtom<A>::addImport(const ld::Atom* atom, StringPoolAtom* pool) 
{
	macho_nlist<P> entry;

	// set n_strx
	entry.set_n_strx(pool->add(atom->name()));

	// set n_type
	if ( this->_options.outputKind() == Options::kObjectFile ) {
		if ( atom->section().type() == ld::Section::typeTempAlias ) {
			if ( atom->scope() == ld::Atom::scopeLinkageUnit )
				entry.set_n_type(N_INDR | N_EXT | N_PEXT);
			else
				entry.set_n_type(N_INDR | N_EXT);
		}
		else if ( (atom->scope() == ld::Atom::scopeLinkageUnit) 
				&& (atom->definition() == ld::Atom::definitionTentative) )
			entry.set_n_type(N_UNDF | N_EXT | N_PEXT);
		else 
			entry.set_n_type(N_UNDF | N_EXT);
	}
	else {
		entry.set_n_type(N_UNDF | N_EXT);
	}

	// set n_sect
	entry.set_n_sect(0);

	uint16_t desc = 0;
	if ( this->_options.outputKind() != Options::kObjectFile ) {
		uint8_t ordinal = this->classicOrdinalForProxy(atom);
		//fprintf(stderr, "ordinal=%u from reader=%p for symbol=%s\n", ordinal, atom->getFile(), atom->getName());
		SET_LIBRARY_ORDINAL(desc, ordinal);
		
#if 0
		// set n_desc ( high byte is library ordinal, low byte is reference type )
		std::map<const ObjectFile::Atom*,ObjectFile::Atom*>::iterator pos = fStubsMap.find(atom);
		if ( pos != fStubsMap.end() || ( strncmp(atom->getName(), ".objc_class_name_", 17) == 0) )
			desc |= REFERENCE_FLAG_UNDEFINED_LAZY;
		else
			desc |= REFERENCE_FLAG_UNDEFINED_NON_LAZY;
#endif
	}
	else if ( atom->definition() == ld::Atom::definitionTentative ) {
		uint8_t align = atom->alignment().powerOf2;
		// always record custom alignment of common symbols to match what compiler does
		SET_COMM_ALIGN(desc, align);
	}
 	if ( (this->_options.outputKind() != Options::kObjectFile)
		&& (atom->definition() == ld::Atom::definitionProxy) 
		&& (atom->combine() == ld::Atom::combineByName) ) {
			desc |= N_REF_TO_WEAK;
	}
	const ld::dylib::File* dylib = dynamic_cast<const ld::dylib::File*>(atom->file());
	if ( atom->weakImported() || ((dylib != NULL) && dylib->forcedWeakLinked()) )
		desc |= N_WEAK_REF;
	entry.set_n_desc(desc);

	// set n_value, zero for import proxy and size for tentative definition
	if ( atom->definition() == ld::Atom::definitionTentative )
		entry.set_n_value(atom->size());
	else if ( atom->section().type() != ld::Section::typeTempAlias )
		entry.set_n_value(0);
	else {
		assert(atom->fixupsBegin() != atom->fixupsEnd());
		for (ld::Fixup::iterator fit = atom->fixupsBegin(); fit != atom->fixupsEnd(); ++fit) {
			assert(fit->kind == ld::Fixup::kindNoneFollowOn);
			switch ( fit->binding ) {
				case ld::Fixup::bindingByNameUnbound:
					entry.set_n_value(pool->add(fit->u.name));
					break;
				case ld::Fixup::bindingsIndirectlyBound:
					entry.set_n_value(pool->add((_state.indirectBindingTable[fit->u.bindingIndex])->name()));
					break;
				default:
					assert(0 && "internal error: unexpected alias binding");
			}
		}
	}
	
	// add to array
	_imports.push_back(entry);
}

template <typename A>
uint8_t SymbolTableAtom<A>::sectionIndexForStab(const ld::relocatable::File::Stab& stab)
{
	// in FUN stabs, n_sect field is 0 for start FUN and 1 for end FUN
	if ( stab.type == N_FUN )
		return stab.other;
	else if ( stab.type == N_GSYM )	
		return 0;
	else if ( stab.atom != NULL ) 
		return stab.atom->machoSection();
	else
		return stab.other;
}


template <typename A>
uint64_t SymbolTableAtom<A>::valueForStab(const ld::relocatable::File::Stab& stab)
{
	switch ( stab.type ) {
		case N_FUN:
			if ( stab.atom == NULL ) {
				// <rdar://problem/5591394> Add support to ld64 for N_FUN stabs when used for symbolic constants
				return stab.value;
			}
			if ( (stab.string == NULL) || (strlen(stab.string) == 0) ) {
				// end of function N_FUN has size
				return stab.atom->size();
			}
			else {
				// start of function N_FUN has address
				return stab.atom->finalAddress();
			}
		case N_LBRAC:
		case N_RBRAC:
		case N_SLINE:
			if ( stab.atom == NULL )
				// some weird assembly files have slines not associated with a function
				return stab.value;
			else
				// all these stab types need their value changed from an offset in the atom to an address
				return stab.atom->finalAddress() + stab.value;
		case N_STSYM:
		case N_LCSYM:
		case N_BNSYM:
			// all these need address of atom
			if ( stab.atom != NULL )
				return stab.atom->finalAddress();
			else
				return 0;  // <rdar://problem/7811357> work around for mismatch N_BNSYM 
		case N_ENSYM:
			return stab.atom->size();
		case N_SO:
			if ( stab.atom == NULL ) {
				return 0;
			}
			else {
				if ( (stab.string == NULL) || (strlen(stab.string) == 0) ) {
					// end of translation unit N_SO has address of end of last atom
					return stab.atom->finalAddress() + stab.atom->size();
				}
				else {
					// start of translation unit N_SO has address of end of first atom
					return stab.atom->finalAddress();
				}
			}
			break;
		default:
			return stab.value;
	}
}

template <typename A>
uint32_t SymbolTableAtom<A>::stringOffsetForStab(const ld::relocatable::File::Stab& stab, StringPoolAtom* pool)
{
	switch (stab.type) {
		case N_SO:
			if ( (stab.string == NULL) || stab.string[0] == '\0' ) {
				return pool->emptyString();
				break;
			}
			// fall into uniquing case
		case N_SOL:
		case N_BINCL:
		case N_EXCL:
			return pool->addUnique(stab.string);
			break;
		default:
			if ( stab.string == NULL )
				return 0;
			else if ( stab.string[0] == '\0' )
				return pool->emptyString();
			else
				return pool->add(stab.string);
	}
	return 0;
}



template <typename A>
bool SymbolTableAtom<A>::hasStabs(uint32_t& ssos, uint32_t& ssoe, uint32_t& sos, uint32_t& soe)
{
	ssos = _stabsStringsOffsetStart;
	ssoe = _stabsStringsOffsetEnd;
	sos = _stabsIndexStart * sizeof(macho_nlist<P>);
	soe = _stabsIndexEnd * sizeof(macho_nlist<P>);
	return ( (_stabsIndexStart != _stabsIndexEnd) || (_stabsStringsOffsetStart != _stabsStringsOffsetEnd) );
}


template <typename A>
void SymbolTableAtom<A>::encode()
{
	// Note: We lay out the symbol table so that the strings for the stabs (local) symbols are at the
	// end of the string pool.  The stabs strings are not used when calculated the UUID for the image.
	// If the stabs strings were not last, the string offsets for all other symbols may very which would alter the UUID.

	// reserve space for local symbols
	uint32_t localsCount = _state.stabs.size() + this->_writer._localAtoms.size();

	// make nlist entries for all global symbols
	std::vector<const ld::Atom*>& globalAtoms = this->_writer._exportedAtoms;
	_globals.reserve(globalAtoms.size());
	uint32_t symbolIndex = localsCount;
	this->_writer._globalSymbolsStartIndex = localsCount;
	for (std::vector<const ld::Atom*>::const_iterator it=globalAtoms.begin(); it != globalAtoms.end(); ++it) {
		const ld::Atom* atom = *it;
		this->addGlobal(atom, this->_writer._stringPoolAtom);
		this->_writer._atomToSymbolIndex[atom] = symbolIndex++;
	}
	this->_writer._globalSymbolsCount = symbolIndex - this->_writer._globalSymbolsStartIndex;

	// make nlist entries for all undefined (imported) symbols
	std::vector<const ld::Atom*>& importAtoms = this->_writer._importedAtoms;
	_imports.reserve(importAtoms.size());
	this->_writer._importSymbolsStartIndex = symbolIndex;
	for (std::vector<const ld::Atom*>::const_iterator it=importAtoms.begin(); it != importAtoms.end(); ++it) {
		this->addImport(*it, this->_writer._stringPoolAtom);
		this->_writer._atomToSymbolIndex[*it] = symbolIndex++;
	}
	this->_writer._importSymbolsCount = symbolIndex - this->_writer._importSymbolsStartIndex;

	// go back to start and make nlist entries for all local symbols
	std::vector<const ld::Atom*>& localAtoms = this->_writer._localAtoms;
	this->_writer._localSymbolsStartIndex = 0;
	symbolIndex = 0;
	_locals.reserve(localsCount);
	for (const ld::Atom* atom : localAtoms) {
		if ( this->addLocal(atom, this->_writer._stringPoolAtom) )
			this->_writer._atomToSymbolIndex[atom] = symbolIndex++;
	}
	_stabsIndexStart = symbolIndex;
	_stabsStringsOffsetStart = this->_writer._stringPoolAtom->currentOffset();
	for (const ld::relocatable::File::Stab& stab : _state.stabs) {
		macho_nlist<P> entry;
		entry.set_n_type(stab.type);
		entry.set_n_sect(sectionIndexForStab(stab));
		entry.set_n_desc(stab.desc);
		entry.set_n_value(valueForStab(stab));
		entry.set_n_strx(stringOffsetForStab(stab, this->_writer._stringPoolAtom));
		_locals.push_back(entry);
		++symbolIndex;
	}
	_stabsIndexEnd = symbolIndex;
	_stabsStringsOffsetEnd = this->_writer._stringPoolAtom->currentOffset();
	this->_writer._localSymbolsCount = symbolIndex;
}

template <typename A>
uint64_t SymbolTableAtom<A>::size() const
{
	return sizeof(macho_nlist<P>) * (_locals.size() + _globals.size() + _imports.size());
}

template <typename A>
void SymbolTableAtom<A>::copyRawContent(uint8_t buffer[]) const
{
	memcpy(&buffer[this->_writer._localSymbolsStartIndex*sizeof(macho_nlist<P>)], &_locals[0], 
												this->_writer._localSymbolsCount*sizeof(macho_nlist<P>));
	memcpy(&buffer[this->_writer._globalSymbolsStartIndex*sizeof(macho_nlist<P>)], &_globals[0],
												this->_writer._globalSymbolsCount*sizeof(macho_nlist<P>));
	memcpy(&buffer[this->_writer._importSymbolsStartIndex *sizeof(macho_nlist<P>)], &_imports[0], 
												this->_writer._importSymbolsCount*sizeof(macho_nlist<P>));
}




class RelocationsAtomAbstract : public ClassicLinkEditAtom
{
public:
												RelocationsAtomAbstract(const Options& opts, ld::Internal& state, 
																OutputFile& writer, const ld::Section& sect,
																unsigned int pointerSize)
													: ClassicLinkEditAtom(opts, state, writer, sect, pointerSize),
													  _pointerSize(pointerSize) { }

	virtual void							addPointerReloc(uint64_t addr, uint32_t symNum, uint32_t length) = 0;
	virtual void							addTextReloc(uint64_t addr, ld::Fixup::Kind k, uint64_t targetAddr, uint32_t symNum) = 0;
	virtual void							addExternalPointerReloc(uint64_t addr, const ld::Atom*) = 0;
	virtual void							addExternalCallSiteReloc(uint64_t addr, const ld::Atom*) = 0;
	virtual uint64_t						relocBaseAddress(ld::Internal& state) = 0;
	virtual	void							addSectionReloc(ld::Internal::FinalSection*	sect, ld::Fixup::Kind, 
															const ld::Atom* inAtom, uint32_t offsetInAtom, 
															bool toTargetUsesExternalReloc ,bool fromTargetExternalReloc,
#if SUPPORT_ARCH_arm64e
															ld::Fixup* fixupWithAuthData,
#endif
															const ld::Atom* toTarget, uint64_t toAddend, 
															const ld::Atom* fromTarget, uint64_t fromAddend) = 0;

	uint32_t								pointerSize() const {
		return _pointerSize;
	}

protected:
	uint32_t								symbolIndex(const ld::Atom* atom) const;

private:
	uint32_t 								_pointerSize;
};



uint32_t RelocationsAtomAbstract::symbolIndex(const ld::Atom* atom) const
{
	std::map<const ld::Atom*, uint32_t>::iterator pos = this->_writer._atomToSymbolIndex.find(atom);
	if ( pos != this->_writer._atomToSymbolIndex.end() )
		return pos->second;
	fprintf(stderr, "_atomToSymbolIndex content:\n");
	for(std::map<const ld::Atom*, uint32_t>::iterator it = this->_writer._atomToSymbolIndex.begin(); it != this->_writer._atomToSymbolIndex.end(); ++it) {
			fprintf(stderr, "%p(%s) => %d\n", it->first, it->first->name(), it->second);
	}
	throwf("internal error: atom not found in symbolIndex(%s)", atom->name());
}


template <typename A>
class LocalRelocationsAtom : public RelocationsAtomAbstract
{
public:
												LocalRelocationsAtom(const Options& opts, ld::Internal& state, OutputFile& writer)
													: RelocationsAtomAbstract(opts, state, writer, _s_section, sizeof(pint_t)) { }

	// overrides of ld::Atom
	virtual const char*							name() const		{ return "local relocations"; }
	virtual uint64_t							size() const;
	virtual void								copyRawContent(uint8_t buffer[]) const; 
	// overrides of ClassicLinkEditAtom
	virtual void								encode() {}
	// overrides of RelocationsAtomAbstract
	virtual void								addPointerReloc(uint64_t addr, uint32_t symNum, uint32_t length);
	virtual void								addExternalPointerReloc(uint64_t addr, const ld::Atom*) {}
	virtual void								addExternalCallSiteReloc(uint64_t addr, const ld::Atom*) {}
	virtual uint64_t							relocBaseAddress(ld::Internal& state);
	virtual void								addTextReloc(uint64_t addr, ld::Fixup::Kind k, uint64_t targetAddr, uint32_t symNum);
	virtual	void								addSectionReloc(ld::Internal::FinalSection*	sect, ld::Fixup::Kind, 
															const ld::Atom* inAtom, uint32_t offsetInAtom, 
															bool toTargetUsesExternalReloc ,bool fromTargetExternalReloc,
#if SUPPORT_ARCH_arm64e
																ld::Fixup* fixupWithAuthData,
#endif
															const ld::Atom* toTarget, uint64_t toAddend, 
															const ld::Atom* fromTarget, uint64_t fromAddend) { }

private:
	typedef typename A::P						P;
	typedef typename A::P::E					E;
	typedef typename A::P::uint_t				pint_t;
	
	std::vector<macho_relocation_info<P> >		_relocs;

	static ld::Section			_s_section;
};

template <typename A>
ld::Section LocalRelocationsAtom<A>::_s_section("__LINKEDIT", "__local_relocs", ld::Section::typeLinkEdit, true);


template <>
uint64_t LocalRelocationsAtom<x86_64>::relocBaseAddress(ld::Internal& state)
{
	if ( _options.outputKind() == Options::kKextBundle ) {
		// for kext bundles the reloc base address starts at __TEXT segment
		return _options.baseAddress();
	}
	// for all other kinds, the x86_64 reloc base address starts at first writable segment (usually __DATA)
	for (std::vector<ld::Internal::FinalSection*>::iterator sit = state.sections.begin(); sit != state.sections.end(); ++sit) {
		ld::Internal::FinalSection* sect = *sit;
		if ( !sect->isSectionHidden() && _options.initialSegProtection(sect->segmentName()) & VM_PROT_WRITE )
			return sect->address;
	}
	throw "writable (__DATA) segment not found";
}

template <typename A>
uint64_t LocalRelocationsAtom<A>::relocBaseAddress(ld::Internal& state)
{
	// <rdar://35164406> in -preload, aligned atoms can cause baseAddress and first section address to be different
	if ( _options.outputKind() == Options::kPreload ) {
		for (ld::Internal::FinalSection* sect : state.sections) {
			if ( !sect->isSectionHidden() )
				return sect->address;
		}
	}
	
	return _options.baseAddress();
}

template <typename A>
void LocalRelocationsAtom<A>::addPointerReloc(uint64_t addr, uint32_t symNum, uint32_t length)
{
	assert(length == 4 || length == 8);
	macho_relocation_info<P> reloc;
	reloc.set_r_address(addr);
	reloc.set_r_symbolnum(symNum);
	reloc.set_r_pcrel(false);
	reloc.set_r_length(length == 4 ? 2 : 3);
	reloc.set_r_extern(false);
	reloc.set_r_type(GENERIC_RELOC_VANILLA);
	_relocs.push_back(reloc);
}

template <typename A>
void LocalRelocationsAtom<A>::addTextReloc(uint64_t addr, ld::Fixup::Kind kind, uint64_t targetAddr, uint32_t symNum)
{
}


template <typename A>
uint64_t LocalRelocationsAtom<A>::size() const
{
	return _relocs.size() * sizeof(macho_relocation_info<P>);
}

template <typename A>
void LocalRelocationsAtom<A>::copyRawContent(uint8_t buffer[]) const
{
	memcpy(buffer, &_relocs[0], _relocs.size()*sizeof(macho_relocation_info<P>));
}






template <typename A>
class ExternalRelocationsAtom : public RelocationsAtomAbstract
{
public:
												ExternalRelocationsAtom(const Options& opts, ld::Internal& state, OutputFile& writer)
													: RelocationsAtomAbstract(opts, state, writer, _s_section, sizeof(pint_t)) { }

	// overrides of ld::Atom
	virtual const char*							name() const		{ return "external relocations"; }
	virtual uint64_t							size() const;
	virtual void								copyRawContent(uint8_t buffer[]) const; 
	// overrides of ClassicLinkEditAtom
	virtual void								encode() {}
	// overrides of RelocationsAtomAbstract
	virtual void								addPointerReloc(uint64_t addr, uint32_t symNum, uint32_t length) {}
	virtual void								addTextReloc(uint64_t addr, ld::Fixup::Kind k, uint64_t targetAddr, uint32_t symNum) {}
	virtual void								addExternalPointerReloc(uint64_t addr, const ld::Atom*);
	virtual void								addExternalCallSiteReloc(uint64_t addr, const ld::Atom*);
	virtual uint64_t							relocBaseAddress(ld::Internal& state);
	virtual	void								addSectionReloc(ld::Internal::FinalSection*	sect, ld::Fixup::Kind, 
															const ld::Atom* inAtom, uint32_t offsetInAtom, 
															bool toTargetUsesExternalReloc ,bool fromTargetExternalReloc,
#if SUPPORT_ARCH_arm64e
																ld::Fixup* fixupWithAuthData,
#endif
															const ld::Atom* toTarget, uint64_t toAddend, 
															const ld::Atom* fromTarget, uint64_t fromAddend) { }
	

private:
	typedef typename A::P						P;
	typedef typename A::P::E					E;
	typedef typename A::P::uint_t				pint_t;
	
	struct LocAndAtom { 
							LocAndAtom(uint64_t l, const ld::Atom* a) : loc(l), atom(a), symbolIndex(0) {}

		uint64_t			loc; 
		const ld::Atom*		atom; 
		uint32_t			symbolIndex;
		
		bool operator<(const LocAndAtom& rhs) const {
			// sort first by symbol number
			if ( this->symbolIndex != rhs.symbolIndex )
				return (this->symbolIndex < rhs.symbolIndex);
			// then sort all uses of the same symbol by address
			return (this->loc < rhs.loc);
		}
		
	};

	static uint32_t		pointerReloc();
	static uint32_t		callReloc();

	mutable std::vector<LocAndAtom>			_pointerLocations;
	mutable std::vector<LocAndAtom>			_callSiteLocations;

	static ld::Section			_s_section;
};

template <typename A>
ld::Section ExternalRelocationsAtom<A>::_s_section("__LINKEDIT", "__extrn_relocs", ld::Section::typeLinkEdit, true);

template <>
uint64_t ExternalRelocationsAtom<x86_64>::relocBaseAddress(ld::Internal& state)
{
	// for x86_64 the reloc base address starts at __DATA segment
	for (std::vector<ld::Internal::FinalSection*>::iterator sit = state.sections.begin(); sit != state.sections.end(); ++sit) {
		ld::Internal::FinalSection* sect = *sit;
		if ( !sect->isSectionHidden() && _options.initialSegProtection(sect->segmentName()) & VM_PROT_WRITE )
			return sect->address;
	}
	throw "writable (__DATA) segment not found";
}

template <typename A>
uint64_t ExternalRelocationsAtom<A>::relocBaseAddress(ld::Internal& state)
{
	return 0;
}

template <typename A>
void ExternalRelocationsAtom<A>::addExternalPointerReloc(uint64_t addr, const ld::Atom* target)
{
	_pointerLocations.push_back(LocAndAtom(addr, target));
}

template <typename A>
void ExternalRelocationsAtom<A>::addExternalCallSiteReloc(uint64_t addr, const ld::Atom* target)
{
	_callSiteLocations.push_back(LocAndAtom(addr, target));
}


template <typename A>
uint64_t ExternalRelocationsAtom<A>::size() const
{
	if ( _options.outputKind() == Options::kStaticExecutable ) {
		assert(_pointerLocations.size() == 0);
		assert(_callSiteLocations.size() == 0);
	}
	return (_pointerLocations.size() + _callSiteLocations.size()) * sizeof(macho_relocation_info<P>);
}

#if SUPPORT_ARCH_arm64
template <> uint32_t ExternalRelocationsAtom<arm64>::pointerReloc() { return ARM64_RELOC_UNSIGNED; }
#endif
#if SUPPORT_ARCH_arm64_32
template <> uint32_t ExternalRelocationsAtom<arm64_32>::pointerReloc() { return ARM64_RELOC_UNSIGNED; }
#endif
#if SUPPORT_ARCH_arm_any
template <> uint32_t ExternalRelocationsAtom<arm>::pointerReloc() { return ARM_RELOC_VANILLA; }
#endif
template <> uint32_t ExternalRelocationsAtom<x86>::pointerReloc() { return GENERIC_RELOC_VANILLA; }
template <> uint32_t ExternalRelocationsAtom<x86_64>::pointerReloc() { return X86_64_RELOC_UNSIGNED; }


template <> uint32_t ExternalRelocationsAtom<x86_64>::callReloc() { return X86_64_RELOC_BRANCH; }
template <> uint32_t ExternalRelocationsAtom<x86>::callReloc() { return GENERIC_RELOC_VANILLA; }
#if SUPPORT_ARCH_arm64
template <> uint32_t ExternalRelocationsAtom<arm64>::callReloc() { return ARM64_RELOC_BRANCH26; }
#endif
#if SUPPORT_ARCH_arm64_32
template <> uint32_t ExternalRelocationsAtom<arm64_32>::callReloc() { return ARM64_RELOC_BRANCH26; }
#endif

template <typename A> 
uint32_t ExternalRelocationsAtom<A>::callReloc() 
{ 
	assert(0 && "external call relocs not implemented");
	return 0; 
}


template <typename A>
void ExternalRelocationsAtom<A>::copyRawContent(uint8_t buffer[]) const
{
	macho_relocation_info<P>* r = (macho_relocation_info<P>*)buffer;
	
	// assign symbol index, now that symbol table is built
	for (typename std::vector<LocAndAtom>::iterator it = _pointerLocations.begin(); it != _pointerLocations.end(); ++it) {
		it->symbolIndex = symbolIndex(it->atom);
	}
	std::sort(_pointerLocations.begin(), _pointerLocations.end());
	for (typename std::vector<LocAndAtom>::const_iterator it = _pointerLocations.begin(); it != _pointerLocations.end(); ++it, ++r) {
		r->set_r_address(it->loc);
		r->set_r_symbolnum(it->symbolIndex);
		r->set_r_pcrel(false);
		r->set_r_length();
		r->set_r_extern(true);
		r->set_r_type(this->pointerReloc());
	}
	
	for (typename std::vector<LocAndAtom>::iterator it = _callSiteLocations.begin(); it != _callSiteLocations.end(); ++it) {
		it->symbolIndex = symbolIndex(it->atom);
	}
	std::sort(_callSiteLocations.begin(), _callSiteLocations.end());
	for (typename std::vector<LocAndAtom>::const_iterator it = _callSiteLocations.begin(); it != _callSiteLocations.end(); ++it, ++r) {
		r->set_r_address(it->loc);
		r->set_r_symbolnum(it->symbolIndex);
		r->set_r_pcrel(true);
		r->set_r_length(2);
		r->set_r_extern(true);
		r->set_r_type(this->callReloc());
	}
}


template <typename A>
class SectionRelocationsAtom : public RelocationsAtomAbstract
{
public:
												SectionRelocationsAtom(const Options& opts, ld::Internal& state, OutputFile& writer)
													: RelocationsAtomAbstract(opts, state, writer, _s_section, sizeof(pint_t)) { }

	// overrides of ld::Atom
	virtual const char*							name() const		{ return "section relocations"; }
	virtual uint64_t							size() const;
	virtual void								copyRawContent(uint8_t buffer[]) const; 
	// overrides of ClassicLinkEditAtom
	virtual void								encode();
	// overrides of RelocationsAtomAbstract
	virtual void								addPointerReloc(uint64_t addr, uint32_t symNum, uint32_t length) {}
	virtual void								addTextReloc(uint64_t addr, ld::Fixup::Kind k, uint64_t targetAddr, uint32_t symNum) {}
	virtual void								addExternalPointerReloc(uint64_t addr, const ld::Atom*) {}
	virtual void								addExternalCallSiteReloc(uint64_t addr, const ld::Atom*) {}
	virtual uint64_t							relocBaseAddress(ld::Internal& state) { return 0; }
	virtual	void								addSectionReloc(ld::Internal::FinalSection*	sect, ld::Fixup::Kind, 
															const ld::Atom* inAtom, uint32_t offsetInAtom, 
															bool toTargetUsesExternalReloc ,bool fromTargetExternalReloc,
#if SUPPORT_ARCH_arm64e
																ld::Fixup* fixupWithAuthData,
#endif
															const ld::Atom* toTarget, uint64_t toAddend, 
															const ld::Atom* fromTarget, uint64_t fromAddend);
		
private:
	typedef typename A::P						P;
	typedef typename A::P::E					E;
	typedef typename A::P::uint_t				pint_t;
	

	struct Entry {
		ld::Fixup::Kind				kind;
		bool						toTargetUsesExternalReloc;
		bool						fromTargetUsesExternalReloc;
		const ld::Atom* 			inAtom;
		uint32_t					offsetInAtom;
		const ld::Atom* 			toTarget; 
		uint64_t					toAddend; 
		const ld::Atom* 			fromTarget; 
		uint64_t					fromAddend;
#if SUPPORT_ARCH_arm64e
		bool 						hasAuthData;
		ld::Fixup::AuthData			authData;
#endif
	};
	uint32_t									sectSymNum(bool external, const ld::Atom* target);
	void										encodeSectionReloc(ld::Internal::FinalSection* sect, 
																const Entry& entry, std::vector<macho_relocation_info<P> >& relocs);
	
	struct SectionAndEntries {
		ld::Internal::FinalSection*				sect;
		std::vector<Entry>						entries;
		std::vector<macho_relocation_info<P> >	relocs;
	};
	
	std::vector<SectionAndEntries>				_entriesBySection;

	static ld::Section							_s_section;
};

template <typename A>
ld::Section SectionRelocationsAtom<A>::_s_section("__LINKEDIT", "__sect_relocs", ld::Section::typeLinkEdit, true);




template <typename A>
uint64_t SectionRelocationsAtom<A>::size() const
{
	uint32_t count = 0;
	for(typename std::vector<SectionAndEntries>::const_iterator it=_entriesBySection.begin(); it != _entriesBySection.end(); ++it) {
		const SectionAndEntries& se = *it;
		count += se.relocs.size();
	}
	return count * sizeof(macho_relocation_info<P>);
}

template <typename A>
void SectionRelocationsAtom<A>::copyRawContent(uint8_t buffer[]) const
{
	uint32_t offset = 0;
	for(typename std::vector<SectionAndEntries>::const_iterator it=_entriesBySection.begin(); it != _entriesBySection.end(); ++it) {
		const SectionAndEntries& se = *it;
		memcpy(&buffer[offset], &se.relocs[0], se.relocs.size()*sizeof(macho_relocation_info<P>));
		offset += (se.relocs.size() * sizeof(macho_relocation_info<P>));
	}
}


template <>
void SectionRelocationsAtom<x86_64>::encodeSectionReloc(ld::Internal::FinalSection* sect, 
													const Entry& entry, std::vector<macho_relocation_info<P> >& relocs)
{
	macho_relocation_info<P> reloc1;
	macho_relocation_info<P> reloc2;
	uint64_t address = entry.inAtom->finalAddress()+entry.offsetInAtom - sect->address;
	bool external = entry.toTargetUsesExternalReloc;
	uint32_t symbolNum = sectSymNum(external, entry.toTarget);
	bool fromExternal = false;
	uint32_t fromSymbolNum = 0;
	if ( entry.fromTarget != NULL ) {
		fromExternal = entry.fromTargetUsesExternalReloc;
		fromSymbolNum = sectSymNum(fromExternal, entry.fromTarget);
	}
	
	
	switch ( entry.kind ) {
		case ld::Fixup::kindStoreX86BranchPCRel32:
		case ld::Fixup::kindStoreTargetAddressX86BranchPCRel32:
		case ld::Fixup::kindStoreX86DtraceCallSiteNop:
		case ld::Fixup::kindStoreX86DtraceIsEnableSiteClear:
			reloc1.set_r_address(address);
			reloc1.set_r_symbolnum(symbolNum);
			reloc1.set_r_pcrel(true);
			reloc1.set_r_length(2);
			reloc1.set_r_extern(external);
			reloc1.set_r_type(X86_64_RELOC_BRANCH);
			relocs.push_back(reloc1);
			break;
			
		case ld::Fixup::kindStoreX86BranchPCRel8:
			reloc1.set_r_address(address);
			reloc1.set_r_symbolnum(symbolNum);
			reloc1.set_r_pcrel(true);
			reloc1.set_r_length(0);
			reloc1.set_r_extern(external);
			reloc1.set_r_type(X86_64_RELOC_BRANCH);
			relocs.push_back(reloc1);
			break;
			
		case ld::Fixup::kindStoreX86PCRel32:
		case ld::Fixup::kindStoreTargetAddressX86PCRel32:
			reloc1.set_r_address(address);
			reloc1.set_r_symbolnum(symbolNum);
			reloc1.set_r_pcrel(true);
			reloc1.set_r_length(2);
			reloc1.set_r_extern(external);
			reloc1.set_r_type(X86_64_RELOC_SIGNED);
			relocs.push_back(reloc1);
			break;
			
		case ld::Fixup::kindStoreX86PCRel32_1:
			reloc1.set_r_address(address);
			reloc1.set_r_symbolnum(symbolNum);
			reloc1.set_r_pcrel(true);
			reloc1.set_r_length(2);
			reloc1.set_r_extern(external);
			reloc1.set_r_type(X86_64_RELOC_SIGNED_1);
			relocs.push_back(reloc1);
			break;
			
		case ld::Fixup::kindStoreX86PCRel32_2:
			reloc1.set_r_address(address);
			reloc1.set_r_symbolnum(symbolNum);
			reloc1.set_r_pcrel(true);
			reloc1.set_r_length(2);
			reloc1.set_r_extern(external);
			reloc1.set_r_type(X86_64_RELOC_SIGNED_2);
			relocs.push_back(reloc1);
			break;
			
		case ld::Fixup::kindStoreX86PCRel32_4:
			reloc1.set_r_address(address);
			reloc1.set_r_symbolnum(symbolNum);
			reloc1.set_r_pcrel(true);
			reloc1.set_r_length(2);
			reloc1.set_r_extern(external);
			reloc1.set_r_type(X86_64_RELOC_SIGNED_4);
			relocs.push_back(reloc1);
			break;
		
		case ld::Fixup::kindStoreX86PCRel32GOTLoad:
		case ld::Fixup::kindStoreTargetAddressX86PCRel32GOTLoad:
			reloc1.set_r_address(address);
			reloc1.set_r_symbolnum(symbolNum);
			reloc1.set_r_pcrel(true);
			reloc1.set_r_length(2);
			reloc1.set_r_extern(external);
			reloc1.set_r_type(X86_64_RELOC_GOT_LOAD);
			relocs.push_back(reloc1);
			break;

		case ld::Fixup::kindStoreX86PCRel32GOT:
			reloc1.set_r_address(address);
			reloc1.set_r_symbolnum(symbolNum);
			reloc1.set_r_pcrel(true);
			reloc1.set_r_length(2);
			reloc1.set_r_extern(external);
			reloc1.set_r_type(X86_64_RELOC_GOT);
			relocs.push_back(reloc1);
			break;

		case ld::Fixup::kindStoreLittleEndian64:
		case ld::Fixup::kindStoreTargetAddressLittleEndian64:
			if ( entry.fromTarget != NULL ) {
				// this is a pointer-diff
				reloc1.set_r_address(address);
				reloc1.set_r_symbolnum(symbolNum);
				reloc1.set_r_pcrel(false);
				reloc1.set_r_length(3);
				reloc1.set_r_extern(external);
				reloc1.set_r_type(X86_64_RELOC_UNSIGNED);
				reloc2.set_r_address(address);
				reloc2.set_r_symbolnum(fromSymbolNum);
				reloc2.set_r_pcrel(false);
				reloc2.set_r_length(3);
				reloc2.set_r_extern(fromExternal);
				reloc2.set_r_type(X86_64_RELOC_SUBTRACTOR);
				relocs.push_back(reloc2);
				relocs.push_back(reloc1);
			}
			else {
				// regular pointer
				reloc1.set_r_address(address);
				reloc1.set_r_symbolnum(symbolNum);
				reloc1.set_r_pcrel(false);
				reloc1.set_r_length(3);
				reloc1.set_r_extern(external);
				reloc1.set_r_type(X86_64_RELOC_UNSIGNED);
				relocs.push_back(reloc1);
			}
			break;

		case ld::Fixup::kindStoreLittleEndian32:
		case ld::Fixup::kindStoreTargetAddressLittleEndian32:
			if ( entry.fromTarget != NULL ) {
				// this is a pointer-diff
				reloc1.set_r_address(address);
				reloc1.set_r_symbolnum(symbolNum);
				reloc1.set_r_pcrel(false);
				reloc1.set_r_length(2);
				reloc1.set_r_extern(external);
				reloc1.set_r_type(X86_64_RELOC_UNSIGNED);
				reloc2.set_r_address(address);
				reloc2.set_r_symbolnum(fromSymbolNum);
				reloc2.set_r_pcrel(false);
				reloc2.set_r_length(2);
				reloc2.set_r_extern(fromExternal);
				reloc2.set_r_type(X86_64_RELOC_SUBTRACTOR);
				relocs.push_back(reloc2);
				relocs.push_back(reloc1);
			}
			else {
				// regular pointer
				reloc1.set_r_address(address);
				reloc1.set_r_symbolnum(symbolNum);
				reloc1.set_r_pcrel(false);
				reloc1.set_r_length(2);
				reloc1.set_r_extern(external);
				reloc1.set_r_type(X86_64_RELOC_UNSIGNED);
				relocs.push_back(reloc1);
			}
			break;
		case ld::Fixup::kindStoreTargetAddressX86PCRel32TLVLoad:
			reloc1.set_r_address(address);
			reloc1.set_r_symbolnum(symbolNum);
			reloc1.set_r_pcrel(true);
			reloc1.set_r_length(2);
			reloc1.set_r_extern(external);
			reloc1.set_r_type(X86_64_RELOC_TLV);
			relocs.push_back(reloc1);
			break;
		default:
			assert(0 && "need to handle -r reloc");
		
	}

}



template <typename A>
uint32_t SectionRelocationsAtom<A>::sectSymNum(bool external, const ld::Atom* target)
{
	if ( target->definition() == ld::Atom::definitionAbsolute ) 
		return R_ABS;
	if ( external )
		return this->symbolIndex(target);	// in external relocations, r_symbolnum field is symbol index
	else
		return target->machoSection();		// in non-extern relocations, r_symbolnum is mach-o section index of target
}

template <>
void SectionRelocationsAtom<x86>::encodeSectionReloc(ld::Internal::FinalSection* sect,
													const Entry& entry, std::vector<macho_relocation_info<P> >& relocs)
{
	macho_relocation_info<P> reloc1;
	macho_relocation_info<P> reloc2;
	macho_scattered_relocation_info<P>* sreloc1 = (macho_scattered_relocation_info<P>*)&reloc1;
	macho_scattered_relocation_info<P>* sreloc2 = (macho_scattered_relocation_info<P>*)&reloc2;
	uint64_t address = entry.inAtom->finalAddress()+entry.offsetInAtom - sect->address;
	bool external = entry.toTargetUsesExternalReloc;
	uint32_t symbolNum = sectSymNum(external, entry.toTarget);
	bool fromExternal = false;
	uint32_t fromSymbolNum = 0;
	if ( entry.fromTarget != NULL ) {
		fromExternal = entry.fromTargetUsesExternalReloc;
		fromSymbolNum = sectSymNum(fromExternal, entry.fromTarget);
	}

	switch ( entry.kind ) {
		case ld::Fixup::kindStoreX86PCRel32:
		case ld::Fixup::kindStoreX86BranchPCRel32:
		case ld::Fixup::kindStoreTargetAddressX86BranchPCRel32:
		case ld::Fixup::kindStoreX86DtraceCallSiteNop:
		case ld::Fixup::kindStoreX86DtraceIsEnableSiteClear:
			if ( !external && (entry.toAddend != 0) ) {
				// use scattered reloc is target offset is non-zero
				sreloc1->set_r_scattered(true);
				sreloc1->set_r_pcrel(true);
				sreloc1->set_r_length(2);
				sreloc1->set_r_type(GENERIC_RELOC_VANILLA);
				sreloc1->set_r_address(address);
				sreloc1->set_r_value(entry.toTarget->finalAddress());
			}
			else {
				reloc1.set_r_address(address);
				reloc1.set_r_symbolnum(symbolNum);
				reloc1.set_r_pcrel(true);
				reloc1.set_r_length(2);
				reloc1.set_r_extern(external);
				reloc1.set_r_type(GENERIC_RELOC_VANILLA);
			}
			relocs.push_back(reloc1);
			break;
	
		case ld::Fixup::kindStoreX86BranchPCRel8:
			if ( !external && (entry.toAddend != 0) ) {
				// use scattered reloc is target offset is non-zero
				sreloc1->set_r_scattered(true);
				sreloc1->set_r_pcrel(true);
				sreloc1->set_r_length(0);
				sreloc1->set_r_type(GENERIC_RELOC_VANILLA);
				sreloc1->set_r_address(address);
				sreloc1->set_r_value(entry.toTarget->finalAddress());
			}
			else {
				reloc1.set_r_address(address);
				reloc1.set_r_symbolnum(symbolNum);
				reloc1.set_r_pcrel(true);
				reloc1.set_r_length(0);
				reloc1.set_r_extern(external);
				reloc1.set_r_type(GENERIC_RELOC_VANILLA);
			}
			relocs.push_back(reloc1);
			break;

		case ld::Fixup::kindStoreX86PCRel16:
			if ( !external && (entry.toAddend != 0) ) {
				// use scattered reloc is target offset is non-zero
				sreloc1->set_r_scattered(true);
				sreloc1->set_r_pcrel(true);
				sreloc1->set_r_length(1);
				sreloc1->set_r_type(GENERIC_RELOC_VANILLA);
				sreloc1->set_r_address(address);
				sreloc1->set_r_value(entry.toTarget->finalAddress());
			}
			else {
				reloc1.set_r_address(address);
				reloc1.set_r_symbolnum(symbolNum);
				reloc1.set_r_pcrel(true);
				reloc1.set_r_length(1);
				reloc1.set_r_extern(external);
				reloc1.set_r_type(GENERIC_RELOC_VANILLA);
			}
			relocs.push_back(reloc1);
			break;

		case ld::Fixup::kindStoreLittleEndian32:
		case ld::Fixup::kindStoreTargetAddressLittleEndian32:
			if ( entry.fromTarget != NULL ) {
				// this is a pointer-diff
				sreloc1->set_r_scattered(true);
				sreloc1->set_r_pcrel(false);
				sreloc1->set_r_length(2);
				if ( entry.toTarget->scope() == ld::Atom::scopeTranslationUnit )
					sreloc1->set_r_type(GENERIC_RELOC_LOCAL_SECTDIFF);
				else
					sreloc1->set_r_type(GENERIC_RELOC_SECTDIFF);
				sreloc1->set_r_address(address);
				if ( entry.toTarget == entry.inAtom ) {
					if ( entry.toAddend > entry.toTarget->size() ) 
						sreloc1->set_r_value(entry.toTarget->finalAddress()+entry.offsetInAtom);
					else
						sreloc1->set_r_value(entry.toTarget->finalAddress()+entry.toAddend);
				}
				else
					sreloc1->set_r_value(entry.toTarget->finalAddress());
				sreloc2->set_r_scattered(true);
				sreloc2->set_r_pcrel(false);
				sreloc2->set_r_length(2);
				sreloc2->set_r_type(GENERIC_RELOC_PAIR);
				sreloc2->set_r_address(0);
				if ( entry.fromTarget == entry.inAtom ) {
					if ( entry.fromAddend > entry.fromTarget->size() )
						sreloc2->set_r_value(entry.fromTarget->finalAddress()+entry.offsetInAtom);
					else
						sreloc2->set_r_value(entry.fromTarget->finalAddress()+entry.fromAddend);
				}
				else
					sreloc2->set_r_value(entry.fromTarget->finalAddress());
				relocs.push_back(reloc1);
				relocs.push_back(reloc2);
			}
			else {
				// regular pointer
				if ( !external && (entry.toAddend != 0) && (entry.toTarget->symbolTableInclusion() != ld::Atom::symbolTableNotIn) ) {
					// use scattered reloc if target offset is non-zero into named atom (5658046)
					sreloc1->set_r_scattered(true);
					sreloc1->set_r_pcrel(false);
					sreloc1->set_r_length(2);
					sreloc1->set_r_type(GENERIC_RELOC_VANILLA);
					sreloc1->set_r_address(address);
					sreloc1->set_r_value(entry.toTarget->finalAddress());
				}
				else {
					reloc1.set_r_address(address);
					reloc1.set_r_symbolnum(symbolNum);
					reloc1.set_r_pcrel(false);
					reloc1.set_r_length(2);
					reloc1.set_r_extern(external);
					reloc1.set_r_type(GENERIC_RELOC_VANILLA);
				}
				relocs.push_back(reloc1);
			}
			break;
		case ld::Fixup::kindStoreX86PCRel32TLVLoad:
		case ld::Fixup::kindStoreX86Abs32TLVLoad:
		case ld::Fixup::kindStoreTargetAddressX86Abs32TLVLoad:
			reloc1.set_r_address(address);
			reloc1.set_r_symbolnum(symbolNum);
			reloc1.set_r_pcrel(entry.kind == ld::Fixup::kindStoreX86PCRel32TLVLoad);
			reloc1.set_r_length(2);
			reloc1.set_r_extern(external);
			reloc1.set_r_type(GENERIC_RLEOC_TLV);
			relocs.push_back(reloc1);
			break;
		default:
			assert(0 && "need to handle -r reloc");
		
	}
}



#if SUPPORT_ARCH_arm_any
template <>
void SectionRelocationsAtom<arm>::encodeSectionReloc(ld::Internal::FinalSection* sect, 
													const Entry& entry, std::vector<macho_relocation_info<P> >& relocs)
{
	macho_relocation_info<P> reloc1;
	macho_relocation_info<P> reloc2;
	macho_scattered_relocation_info<P>* sreloc1 = (macho_scattered_relocation_info<P>*)&reloc1;
	macho_scattered_relocation_info<P>* sreloc2 = (macho_scattered_relocation_info<P>*)&reloc2;
	uint64_t address = entry.inAtom->finalAddress()+entry.offsetInAtom - sect->address;
	bool external = entry.toTargetUsesExternalReloc;
	uint32_t symbolNum = sectSymNum(external, entry.toTarget);
	bool fromExternal = false;
	uint32_t fromSymbolNum = 0;
	if ( entry.fromTarget != NULL ) {
		fromExternal = entry.fromTargetUsesExternalReloc;
		fromSymbolNum = sectSymNum(fromExternal, entry.fromTarget);
	}
	

	switch ( entry.kind ) {
		case ld::Fixup::kindStoreTargetAddressARMBranch24:
		case ld::Fixup::kindStoreARMBranch24:
		case ld::Fixup::kindStoreARMDtraceCallSiteNop:
		case ld::Fixup::kindStoreARMDtraceIsEnableSiteClear:
			if ( !external && (entry.toAddend != 0) ) {
				// use scattered reloc is target offset is non-zero
				sreloc1->set_r_scattered(true);
				sreloc1->set_r_pcrel(true);
				sreloc1->set_r_length(2);
				sreloc1->set_r_type(ARM_RELOC_BR24);
				sreloc1->set_r_address(address);
				sreloc1->set_r_value(entry.toTarget->finalAddress());
			}
			else {
				reloc1.set_r_address(address);
				reloc1.set_r_symbolnum(symbolNum);
				reloc1.set_r_pcrel(true);
				reloc1.set_r_length(2);
				reloc1.set_r_extern(external);
				reloc1.set_r_type(ARM_RELOC_BR24);
			}
			relocs.push_back(reloc1);
			break;
	
		case ld::Fixup::kindStoreTargetAddressThumbBranch22:
		case ld::Fixup::kindStoreThumbBranch22:
		case ld::Fixup::kindStoreThumbDtraceCallSiteNop:
		case ld::Fixup::kindStoreThumbDtraceIsEnableSiteClear:
			if ( !external && (entry.toAddend != 0) ) {
				// use scattered reloc is target offset is non-zero
				sreloc1->set_r_scattered(true);
				sreloc1->set_r_pcrel(true);
				sreloc1->set_r_length(2);
				sreloc1->set_r_type(ARM_THUMB_RELOC_BR22);
				sreloc1->set_r_address(address);
				sreloc1->set_r_value(entry.toTarget->finalAddress());
			}
			else {
				reloc1.set_r_address(address);
				reloc1.set_r_symbolnum(symbolNum);
				reloc1.set_r_pcrel(true);
				reloc1.set_r_length(2);
				reloc1.set_r_extern(external);
				reloc1.set_r_type(ARM_THUMB_RELOC_BR22);
			}
			relocs.push_back(reloc1);
			break;

		case ld::Fixup::kindStoreLittleEndian32:
		case ld::Fixup::kindStoreTargetAddressLittleEndian32:
			if ( entry.fromTarget != NULL ) {
				// this is a pointer-diff
				sreloc1->set_r_scattered(true);
				sreloc1->set_r_pcrel(false);
				sreloc1->set_r_length(2);
				if ( entry.toTarget->scope() == ld::Atom::scopeTranslationUnit )
					sreloc1->set_r_type(ARM_RELOC_LOCAL_SECTDIFF);
				else
					sreloc1->set_r_type(ARM_RELOC_SECTDIFF);
				sreloc1->set_r_address(address);
				if ( entry.toTarget == entry.inAtom ) {
					if ( entry.toAddend > entry.toTarget->size() ) 
						sreloc1->set_r_value(entry.toTarget->finalAddress()+entry.offsetInAtom);
					else
						sreloc1->set_r_value(entry.toTarget->finalAddress()+entry.toAddend);
				}
				else {
					sreloc1->set_r_value(entry.toTarget->finalAddress());
				}
				sreloc2->set_r_scattered(true);
				sreloc2->set_r_pcrel(false);
				sreloc2->set_r_length(2);
				sreloc2->set_r_type(ARM_RELOC_PAIR);
				sreloc2->set_r_address(0);
				if ( entry.fromTarget == entry.inAtom ) {
					//unsigned int pcBaseOffset = entry.inAtom->isThumb() ? 4 : 8;
					//if ( entry.fromAddend > pcBaseOffset )
					//	sreloc2->set_r_value(entry.fromTarget->finalAddress()+entry.fromAddend-pcBaseOffset);
					//else
						sreloc2->set_r_value(entry.fromTarget->finalAddress()+entry.fromAddend);
				}
				else {
					sreloc2->set_r_value(entry.fromTarget->finalAddress());
				}
				relocs.push_back(reloc1);
				relocs.push_back(reloc2);
			}
			else {
				// regular pointer
				if ( !external && (entry.toAddend != 0) ) {
					// use scattered reloc is target offset is non-zero
					sreloc1->set_r_scattered(true);
					sreloc1->set_r_pcrel(false);
					sreloc1->set_r_length(2);
					sreloc1->set_r_type(ARM_RELOC_VANILLA);
					sreloc1->set_r_address(address);
					sreloc1->set_r_value(entry.toTarget->finalAddress());
				}
				else {
					reloc1.set_r_address(address);
					reloc1.set_r_symbolnum(symbolNum);
					reloc1.set_r_pcrel(false);
					reloc1.set_r_length(2);
					reloc1.set_r_extern(external);
					reloc1.set_r_type(ARM_RELOC_VANILLA);
				}
				relocs.push_back(reloc1);
			}
			break;
			
		case ld::Fixup::kindStoreARMLow16:
		case ld::Fixup::kindStoreARMHigh16:
		case ld::Fixup::kindStoreThumbLow16:
		case ld::Fixup::kindStoreThumbHigh16:
			{
				int len = 0;
				uint32_t otherHalf = 0;
				uint32_t value;
				if ( entry.fromTarget != NULL )  {
				  // this is a sect-diff
				  value = (entry.toTarget->finalAddress()+entry.toAddend) - (entry.fromTarget->finalAddress()+entry.fromAddend);
				}
				else {
					// this is an absolute address
					value = entry.toAddend;
					if ( !external )
						value += entry.toTarget->finalAddress();
				}
				switch ( entry.kind ) {
					case ld::Fixup::kindStoreARMLow16:
						len = 0;
						otherHalf = value >> 16;
						break;
					case ld::Fixup::kindStoreARMHigh16:
						len = 1;
						otherHalf = value & 0xFFFF;
						break;
					case ld::Fixup::kindStoreThumbLow16:
						len = 2;
						otherHalf = value >> 16;
						break;
					case ld::Fixup::kindStoreThumbHigh16:
						len = 3;
						otherHalf = value & 0xFFFF;
						break;
					default:
						break;
				}
				if ( entry.fromTarget != NULL ) {
					// this is a sect-diff
					sreloc1->set_r_scattered(true);
					sreloc1->set_r_pcrel(false);
					sreloc1->set_r_length(len);
					sreloc1->set_r_type(ARM_RELOC_HALF_SECTDIFF);
					sreloc1->set_r_address(address);
					sreloc1->set_r_value(entry.toTarget->finalAddress());
					sreloc2->set_r_scattered(true);
					sreloc2->set_r_pcrel(false);
					sreloc2->set_r_length(len);
					sreloc2->set_r_type(ARM_RELOC_PAIR);
					sreloc2->set_r_address(otherHalf);
					if ( entry.fromTarget == entry.inAtom ) 
						sreloc2->set_r_value(entry.fromTarget->finalAddress()+entry.fromAddend);
					else 
						sreloc2->set_r_value(entry.fromTarget->finalAddress());
					relocs.push_back(reloc1);
					relocs.push_back(reloc2);
				}
				else {
					// this is absolute address
					if ( !external && (entry.toAddend != 0) ) {
						// use scattered reloc is target offset is non-zero
						sreloc1->set_r_scattered(true);
						sreloc1->set_r_pcrel(false);
						sreloc1->set_r_length(len); 
						sreloc1->set_r_type(ARM_RELOC_HALF);
						sreloc1->set_r_address(address);
						sreloc1->set_r_value(entry.toTarget->finalAddress());
						reloc2.set_r_address(otherHalf);
						reloc2.set_r_symbolnum(0);
						reloc2.set_r_pcrel(false);
						reloc2.set_r_length(len); 
						reloc2.set_r_extern(false);
						reloc2.set_r_type(ARM_RELOC_PAIR);
						relocs.push_back(reloc1);
						relocs.push_back(reloc2);
					}
					else {
						reloc1.set_r_address(address);
						reloc1.set_r_symbolnum(symbolNum);
						reloc1.set_r_pcrel(false);
						reloc1.set_r_length(len);
						reloc1.set_r_extern(external);
						reloc1.set_r_type(ARM_RELOC_HALF);
						reloc2.set_r_address(otherHalf);  // other half
						reloc2.set_r_symbolnum(0);
						reloc2.set_r_pcrel(false);
						reloc2.set_r_length(len); 
						reloc2.set_r_extern(false);
						reloc2.set_r_type(ARM_RELOC_PAIR);
						relocs.push_back(reloc1);
						relocs.push_back(reloc2);
					}
				}
			}
			break;
					
		default:
			assert(0 && "need to handle -r reloc");
		
	}
}
#endif

#if SUPPORT_ARCH_arm64
template <>
void SectionRelocationsAtom<arm64>::encodeSectionReloc(ld::Internal::FinalSection* sect, 
													const Entry& entry, std::vector<macho_relocation_info<P> >& relocs)
{
	macho_relocation_info<P> reloc1;
	macho_relocation_info<P> reloc2;
	uint64_t address = entry.inAtom->finalAddress()+entry.offsetInAtom - sect->address;
	bool external = entry.toTargetUsesExternalReloc;
	uint32_t symbolNum = sectSymNum(external, entry.toTarget);
	bool fromExternal = false;
	uint32_t fromSymbolNum = 0;
	if ( entry.fromTarget != NULL ) {
		fromExternal = entry.fromTargetUsesExternalReloc;
		fromSymbolNum = sectSymNum(fromExternal, entry.fromTarget);
	}
	
	
	switch ( entry.kind ) {
		case ld::Fixup::kindStoreARM64Branch26:
			if ( entry.toAddend != 0 ) {
				assert(entry.toAddend < 0x400000);
				reloc2.set_r_address(address);
				reloc2.set_r_symbolnum(entry.toAddend);
				reloc2.set_r_pcrel(false);
				reloc2.set_r_length(2);
				reloc2.set_r_extern(false);
				reloc2.set_r_type(ARM64_RELOC_ADDEND);
				relocs.push_back(reloc2);
			}
			// fall into next case
		case ld::Fixup::kindStoreTargetAddressARM64Branch26:
		case ld::Fixup::kindStoreARM64DtraceCallSiteNop:
		case ld::Fixup::kindStoreARM64DtraceIsEnableSiteClear:
			reloc1.set_r_address(address);
			reloc1.set_r_symbolnum(symbolNum);
			reloc1.set_r_pcrel(true);
			reloc1.set_r_length(2);
			reloc1.set_r_extern(external);
			reloc1.set_r_type(ARM64_RELOC_BRANCH26);
			relocs.push_back(reloc1);
			break;
			
		case ld::Fixup::kindStoreARM64Page21:
			if ( entry.toAddend != 0 ) {
				assert(entry.toAddend < 0x400000);
				reloc2.set_r_address(address);
				reloc2.set_r_symbolnum(entry.toAddend);
				reloc2.set_r_pcrel(false);
				reloc2.set_r_length(2);
				reloc2.set_r_extern(false);
				reloc2.set_r_type(ARM64_RELOC_ADDEND);
				relocs.push_back(reloc2);
			}
			// fall into next case
		case ld::Fixup::kindStoreTargetAddressARM64Page21:
			reloc1.set_r_address(address);
			reloc1.set_r_symbolnum(symbolNum);
			reloc1.set_r_pcrel(true);
			reloc1.set_r_length(2);
			reloc1.set_r_extern(external);
			reloc1.set_r_type(ARM64_RELOC_PAGE21);
			relocs.push_back(reloc1);
			break;

		case ld::Fixup::kindStoreARM64PageOff12:
			if ( entry.toAddend != 0 ) {
				assert(entry.toAddend < 0x400000);
				reloc2.set_r_address(address);
				reloc2.set_r_symbolnum(entry.toAddend);
				reloc2.set_r_pcrel(false);
				reloc2.set_r_length(2);
				reloc2.set_r_extern(false);
				reloc2.set_r_type(ARM64_RELOC_ADDEND);
				relocs.push_back(reloc2);
			}
			// fall into next case
		case ld::Fixup::kindStoreTargetAddressARM64PageOff12:
		case ld::Fixup::kindStoreTargetAddressARM64PageOff12ConvertAddToLoad:
			reloc1.set_r_address(address);
			reloc1.set_r_symbolnum(symbolNum);
			reloc1.set_r_pcrel(false);
			reloc1.set_r_length(2);
			reloc1.set_r_extern(external);
			reloc1.set_r_type(ARM64_RELOC_PAGEOFF12);
			relocs.push_back(reloc1);
			break;

		case ld::Fixup::kindStoreTargetAddressARM64GOTLoadPage21:
		case ld::Fixup::kindStoreARM64GOTLoadPage21:
			reloc1.set_r_address(address);
			reloc1.set_r_symbolnum(symbolNum);
			reloc1.set_r_pcrel(true);
			reloc1.set_r_length(2);
			reloc1.set_r_extern(external);
			reloc1.set_r_type(ARM64_RELOC_GOT_LOAD_PAGE21);
			relocs.push_back(reloc1);
			break;

		case ld::Fixup::kindStoreTargetAddressARM64GOTLoadPageOff12:
		case ld::Fixup::kindStoreARM64GOTLoadPageOff12:
			reloc1.set_r_address(address);
			reloc1.set_r_symbolnum(symbolNum);
			reloc1.set_r_pcrel(false);
			reloc1.set_r_length(2);
			reloc1.set_r_extern(external);
			reloc1.set_r_type(ARM64_RELOC_GOT_LOAD_PAGEOFF12);
			relocs.push_back(reloc1);
			break;

		case ld::Fixup::kindStoreARM64TLVPLoadPageOff12:
		case ld::Fixup::kindStoreTargetAddressARM64TLVPLoadPageOff12:
			reloc1.set_r_address(address);
			reloc1.set_r_symbolnum(symbolNum);
			reloc1.set_r_pcrel(false);
			reloc1.set_r_length(2);
			reloc1.set_r_extern(external);
			reloc1.set_r_type(ARM64_RELOC_TLVP_LOAD_PAGEOFF12);
			relocs.push_back(reloc1);
			break;

		case ld::Fixup::kindStoreARM64TLVPLoadPage21:
		case ld::Fixup::kindStoreTargetAddressARM64TLVPLoadPage21:
			reloc1.set_r_address(address);
			reloc1.set_r_symbolnum(symbolNum);
			reloc1.set_r_pcrel(true);
			reloc1.set_r_length(2);
			reloc1.set_r_extern(external);
			reloc1.set_r_type(ARM64_RELOC_TLVP_LOAD_PAGE21);
			relocs.push_back(reloc1);
			break;

		case ld::Fixup::kindStoreLittleEndian64:
		case ld::Fixup::kindStoreTargetAddressLittleEndian64:
			if ( entry.fromTarget != NULL ) {
				// this is a pointer-diff
				reloc1.set_r_address(address);
				reloc1.set_r_symbolnum(symbolNum);
				reloc1.set_r_pcrel(false);
				reloc1.set_r_length(3);
				reloc1.set_r_extern(external);
				reloc1.set_r_type(ARM64_RELOC_UNSIGNED);
				reloc2.set_r_address(address);
				reloc2.set_r_symbolnum(fromSymbolNum);
				reloc2.set_r_pcrel(false);
				reloc2.set_r_length(3);
				reloc2.set_r_extern(fromExternal);
				reloc2.set_r_type(ARM64_RELOC_SUBTRACTOR);
				relocs.push_back(reloc2);
				relocs.push_back(reloc1);
			}
			else {
				// regular pointer
				reloc1.set_r_address(address);
				reloc1.set_r_symbolnum(symbolNum);
				reloc1.set_r_pcrel(false);
				reloc1.set_r_length(3);
				reloc1.set_r_extern(external);
				reloc1.set_r_type(ARM64_RELOC_UNSIGNED);
				relocs.push_back(reloc1);
			}
			break;

		case ld::Fixup::kindStoreLittleEndian32:
		case ld::Fixup::kindStoreTargetAddressLittleEndian32:
			if ( entry.fromTarget != NULL ) {
				// this is a pointer-diff
				reloc1.set_r_address(address);
				reloc1.set_r_symbolnum(symbolNum);
				reloc1.set_r_pcrel(false);
				reloc1.set_r_length(2);
				reloc1.set_r_extern(external);
				reloc1.set_r_type(ARM64_RELOC_UNSIGNED);
				reloc2.set_r_address(address);
				reloc2.set_r_symbolnum(fromSymbolNum);
				reloc2.set_r_pcrel(false);
				reloc2.set_r_length(2);
				reloc2.set_r_extern(fromExternal);
				reloc2.set_r_type(ARM64_RELOC_SUBTRACTOR);
				relocs.push_back(reloc2);
				relocs.push_back(reloc1);
			}
			else {
				// regular pointer
				reloc1.set_r_address(address);
				reloc1.set_r_symbolnum(symbolNum);
				reloc1.set_r_pcrel(false);
				reloc1.set_r_length(2);
				reloc1.set_r_extern(external);
				reloc1.set_r_type(ARM64_RELOC_UNSIGNED);
				relocs.push_back(reloc1);
			}
			break;

		case ld::Fixup::kindStoreARM64PointerToGOT:
            reloc1.set_r_address(address);
            reloc1.set_r_symbolnum(symbolNum);
            reloc1.set_r_pcrel(false);
            reloc1.set_r_length(3);
            reloc1.set_r_extern(external);
            reloc1.set_r_type(ARM64_RELOC_POINTER_TO_GOT);
            relocs.push_back(reloc1);
            break;

		case ld::Fixup::kindStoreARM64PCRelToGOT:
            reloc1.set_r_address(address);
            reloc1.set_r_symbolnum(symbolNum);
            reloc1.set_r_pcrel(true);
            reloc1.set_r_length(2);
            reloc1.set_r_extern(external);
            reloc1.set_r_type(ARM64_RELOC_POINTER_TO_GOT);
            relocs.push_back(reloc1);
            break;

#if SUPPORT_ARCH_arm64e
		case ld::Fixup::kindStoreLittleEndianAuth64:
		case ld::Fixup::kindStoreTargetAddressLittleEndianAuth64: {
			assert(entry.fromTarget == NULL);
			assert(entry.hasAuthData);

			// An authenticated pointer is:
			// {
			//	 int32_t addend;
			//	 uint16_t diversityData;
			//	 uint16_t hasAddressDiversity : 1;
			//	 uint16_t key : 2;
			//	 uint16_t zeroes : 11;
			//	 uint16_t zero : 1;
			//	 uint16_t authenticated : 1;
			// }
			reloc1.set_r_address(address);
			reloc1.set_r_symbolnum(symbolNum);
			reloc1.set_r_pcrel(false);
			reloc1.set_r_length(3);
			reloc1.set_r_extern(external);
			reloc1.set_r_type(ARM64_RELOC_AUTHENTICATED_POINTER);
			relocs.push_back(reloc1);
		}
		break;
#endif

		default:
			assert(0 && "need to handle arm64 -r reloc");
		
	}

}
#endif // SUPPORT_ARCH_arm64

#if SUPPORT_ARCH_arm64_32
template <>
void SectionRelocationsAtom<arm64_32>::encodeSectionReloc(ld::Internal::FinalSection* sect, 
													const Entry& entry, std::vector<macho_relocation_info<P> >& relocs)
{
	macho_relocation_info<P> reloc1;
	macho_relocation_info<P> reloc2;
	uint64_t address = entry.inAtom->finalAddress()+entry.offsetInAtom - sect->address;
	bool external = entry.toTargetUsesExternalReloc;
	uint32_t symbolNum = sectSymNum(external, entry.toTarget);
	bool fromExternal = false;
	uint32_t fromSymbolNum = 0;
	if ( entry.fromTarget != NULL ) {
		fromExternal = entry.fromTargetUsesExternalReloc;
		fromSymbolNum = sectSymNum(fromExternal, entry.fromTarget);
	}
	
	
	switch ( entry.kind ) {
		case ld::Fixup::kindStoreARM64Branch26:
			if ( entry.toAddend != 0 ) {
				assert(entry.toAddend < 0x400000);
				reloc2.set_r_address(address);
				reloc2.set_r_symbolnum(entry.toAddend);
				reloc2.set_r_pcrel(false);
				reloc2.set_r_length(2);
				reloc2.set_r_extern(false);
				reloc2.set_r_type(ARM64_RELOC_ADDEND);
				relocs.push_back(reloc2);
			}
			// fall into next case
		case ld::Fixup::kindStoreTargetAddressARM64Branch26:
		case ld::Fixup::kindStoreARM64DtraceCallSiteNop:
		case ld::Fixup::kindStoreARM64DtraceIsEnableSiteClear:
			reloc1.set_r_address(address);
			reloc1.set_r_symbolnum(symbolNum);
			reloc1.set_r_pcrel(true);
			reloc1.set_r_length(2);
			reloc1.set_r_extern(external);
			reloc1.set_r_type(ARM64_RELOC_BRANCH26);
			relocs.push_back(reloc1);
			break;
			
		case ld::Fixup::kindStoreARM64Page21:
			if ( entry.toAddend != 0 ) {
				assert(entry.toAddend < 0x400000);
				reloc2.set_r_address(address);
				reloc2.set_r_symbolnum(entry.toAddend);
				reloc2.set_r_pcrel(false);
				reloc2.set_r_length(2);
				reloc2.set_r_extern(false);
				reloc2.set_r_type(ARM64_RELOC_ADDEND);
				relocs.push_back(reloc2);
			}
			// fall into next case
		case ld::Fixup::kindStoreTargetAddressARM64Page21:
			reloc1.set_r_address(address);
			reloc1.set_r_symbolnum(symbolNum);
			reloc1.set_r_pcrel(true);
			reloc1.set_r_length(2);
			reloc1.set_r_extern(external);
			reloc1.set_r_type(ARM64_RELOC_PAGE21);
			relocs.push_back(reloc1);
			break;

		case ld::Fixup::kindStoreARM64PageOff12:
			if ( entry.toAddend != 0 ) {
				assert(entry.toAddend < 0x400000);
				reloc2.set_r_address(address);
				reloc2.set_r_symbolnum(entry.toAddend);
				reloc2.set_r_pcrel(false);
				reloc2.set_r_length(2);
				reloc2.set_r_extern(false);
				reloc2.set_r_type(ARM64_RELOC_ADDEND);
				relocs.push_back(reloc2);
			}
			// fall into next case
		case ld::Fixup::kindStoreTargetAddressARM64PageOff12:
		case ld::Fixup::kindStoreTargetAddressARM64PageOff12ConvertAddToLoad:
			reloc1.set_r_address(address);
			reloc1.set_r_symbolnum(symbolNum);
			reloc1.set_r_pcrel(false);
			reloc1.set_r_length(2);
			reloc1.set_r_extern(external);
			reloc1.set_r_type(ARM64_RELOC_PAGEOFF12);
			relocs.push_back(reloc1);
			break;

		case ld::Fixup::kindStoreTargetAddressARM64GOTLoadPage21:
		case ld::Fixup::kindStoreARM64GOTLoadPage21:
			reloc1.set_r_address(address);
			reloc1.set_r_symbolnum(symbolNum);
			reloc1.set_r_pcrel(true);
			reloc1.set_r_length(2);
			reloc1.set_r_extern(external);
			reloc1.set_r_type(ARM64_RELOC_GOT_LOAD_PAGE21);
			relocs.push_back(reloc1);
			break;

		case ld::Fixup::kindStoreTargetAddressARM64GOTLoadPageOff12:
		case ld::Fixup::kindStoreARM64GOTLoadPageOff12:
			reloc1.set_r_address(address);
			reloc1.set_r_symbolnum(symbolNum);
			reloc1.set_r_pcrel(false);
			reloc1.set_r_length(2);
			reloc1.set_r_extern(external);
			reloc1.set_r_type(ARM64_RELOC_GOT_LOAD_PAGEOFF12);
			relocs.push_back(reloc1);
			break;

		case ld::Fixup::kindStoreARM64TLVPLoadPageOff12:
		case ld::Fixup::kindStoreTargetAddressARM64TLVPLoadPageOff12:
			reloc1.set_r_address(address);
			reloc1.set_r_symbolnum(symbolNum);
			reloc1.set_r_pcrel(false);
			reloc1.set_r_length(2);
			reloc1.set_r_extern(external);
			reloc1.set_r_type(ARM64_RELOC_TLVP_LOAD_PAGEOFF12);
			relocs.push_back(reloc1);
			break;

		case ld::Fixup::kindStoreARM64TLVPLoadPage21:
		case ld::Fixup::kindStoreTargetAddressARM64TLVPLoadPage21:
			reloc1.set_r_address(address);
			reloc1.set_r_symbolnum(symbolNum);
			reloc1.set_r_pcrel(true);
			reloc1.set_r_length(2);
			reloc1.set_r_extern(external);
			reloc1.set_r_type(ARM64_RELOC_TLVP_LOAD_PAGE21);
			relocs.push_back(reloc1);
			break;

		case ld::Fixup::kindStoreLittleEndian64:
		case ld::Fixup::kindStoreTargetAddressLittleEndian64:
			if ( entry.fromTarget != NULL ) {
				// this is a pointer-diff
				reloc1.set_r_address(address);
				reloc1.set_r_symbolnum(symbolNum);
				reloc1.set_r_pcrel(false);
				reloc1.set_r_length(3);
				reloc1.set_r_extern(external);
				reloc1.set_r_type(ARM64_RELOC_UNSIGNED);
				reloc2.set_r_address(address);
				reloc2.set_r_symbolnum(fromSymbolNum);
				reloc2.set_r_pcrel(false);
				reloc2.set_r_length(3);
				reloc2.set_r_extern(fromExternal);
				reloc2.set_r_type(ARM64_RELOC_SUBTRACTOR);
				relocs.push_back(reloc2);
				relocs.push_back(reloc1);
			}
			else {
				// regular pointer
				reloc1.set_r_address(address);
				reloc1.set_r_symbolnum(symbolNum);
				reloc1.set_r_pcrel(false);
				reloc1.set_r_length(3);
				reloc1.set_r_extern(external);
				reloc1.set_r_type(ARM64_RELOC_UNSIGNED);
				relocs.push_back(reloc1);
			}
			break;

		case ld::Fixup::kindStoreLittleEndian32:
		case ld::Fixup::kindStoreTargetAddressLittleEndian32:
			if ( entry.fromTarget != NULL ) {
				// this is a pointer-diff
				reloc1.set_r_address(address);
				reloc1.set_r_symbolnum(symbolNum);
				reloc1.set_r_pcrel(false);
				reloc1.set_r_length(2);
				reloc1.set_r_extern(external);
				reloc1.set_r_type(ARM64_RELOC_UNSIGNED);
				reloc2.set_r_address(address);
				reloc2.set_r_symbolnum(fromSymbolNum);
				reloc2.set_r_pcrel(false);
				reloc2.set_r_length(2);
				reloc2.set_r_extern(fromExternal);
				reloc2.set_r_type(ARM64_RELOC_SUBTRACTOR);
				relocs.push_back(reloc2);
				relocs.push_back(reloc1);
			}
			else {
				// regular pointer
				reloc1.set_r_address(address);
				reloc1.set_r_symbolnum(symbolNum);
				reloc1.set_r_pcrel(false);
				reloc1.set_r_length(2);
				reloc1.set_r_extern(external);
				reloc1.set_r_type(ARM64_RELOC_UNSIGNED);
				relocs.push_back(reloc1);
			}
			break;

		case ld::Fixup::kindStoreARM64PointerToGOT32:
            reloc1.set_r_address(address);
            reloc1.set_r_symbolnum(symbolNum);
            reloc1.set_r_pcrel(false);
            reloc1.set_r_length(2);
            reloc1.set_r_extern(external);
            reloc1.set_r_type(ARM64_RELOC_POINTER_TO_GOT);
            relocs.push_back(reloc1);
            break;

		case ld::Fixup::kindStoreARM64PCRelToGOT:
            reloc1.set_r_address(address);
            reloc1.set_r_symbolnum(symbolNum);
            reloc1.set_r_pcrel(true);
            reloc1.set_r_length(2);
            reloc1.set_r_extern(external);
            reloc1.set_r_type(ARM64_RELOC_POINTER_TO_GOT);
            relocs.push_back(reloc1);
            break;

		default:
			assert(0 && "need to handle arm64 -r reloc");
		
	}

}
#endif // SUPPORT_ARCH_arm64_32

template <typename A>
void SectionRelocationsAtom<A>::addSectionReloc(ld::Internal::FinalSection*	sect, ld::Fixup::Kind kind, 
												const ld::Atom* inAtom, uint32_t offsetInAtom,  
												bool toTargetUsesExternalReloc ,bool fromTargetExternalReloc,
#if SUPPORT_ARCH_arm64e
												ld::Fixup* fixupWithAuthData,
#endif
												const ld::Atom* toTarget, uint64_t toAddend, 
												const ld::Atom* fromTarget, uint64_t fromAddend)
{
	Entry entry;
	entry.kind = kind;
	entry.toTargetUsesExternalReloc = toTargetUsesExternalReloc;
	entry.fromTargetUsesExternalReloc = fromTargetExternalReloc;
	entry.inAtom = inAtom;
	entry.offsetInAtom = offsetInAtom;
	entry.toTarget = toTarget;
	entry.toAddend = toAddend;
	entry.fromTarget = fromTarget;
	entry.fromAddend = fromAddend;
#if SUPPORT_ARCH_arm64e
	if (fixupWithAuthData) {
		entry.hasAuthData = true;
		entry.authData = fixupWithAuthData->u.authData;
	} else {
		entry.hasAuthData = false;
	}
#endif
	
	static ld::Internal::FinalSection* lastSection = NULL;
	static SectionAndEntries* lastSectionAndEntries = NULL;
		
	if ( sect != lastSection ) {
		for(typename std::vector<SectionAndEntries>::iterator it=_entriesBySection.begin(); it != _entriesBySection.end(); ++it) {
			if ( sect == it->sect ) {
				lastSection = sect;
				lastSectionAndEntries = &*it;
				break;
			}
		}
		if ( sect != lastSection ) {
			SectionAndEntries tmp;
			tmp.sect = sect;
			_entriesBySection.push_back(tmp);
			lastSection = sect;
			lastSectionAndEntries = &_entriesBySection.back();
		}
	}
	lastSectionAndEntries->entries.push_back(entry);
}

template <typename A>
void SectionRelocationsAtom<A>::encode()
{
	// convert each Entry record to one or two reloc records
	for(typename std::vector<SectionAndEntries>::iterator it=_entriesBySection.begin(); it != _entriesBySection.end(); ++it) {
		SectionAndEntries& se = *it;
		for(typename std::vector<Entry>::iterator eit=se.entries.begin(); eit != se.entries.end(); ++eit) {
			encodeSectionReloc(se.sect, *eit, se.relocs);
		}
	}
	
	// update sections with start and count or relocs
	uint32_t index = 0;
	for(typename std::vector<SectionAndEntries>::iterator it=_entriesBySection.begin(); it != _entriesBySection.end(); ++it) {
		SectionAndEntries& se = *it;
		se.sect->relocStart = index;
		se.sect->relocCount = se.relocs.size();
		index += se.sect->relocCount;
	}

}



template <typename A>
class IndirectSymbolTableAtom : public ClassicLinkEditAtom
{
public:
												IndirectSymbolTableAtom(const Options& opts, ld::Internal& state, OutputFile& writer)
													: ClassicLinkEditAtom(opts, state, writer, _s_section, sizeof(pint_t)) { }

	// overrides of ld::Atom
	virtual const char*							name() const		{ return "indirect symbol table"; }
	virtual uint64_t							size() const;
	virtual void								copyRawContent(uint8_t buffer[]) const; 
	// overrides of ClassicLinkEditAtom
	virtual void								encode();

private:
	typedef typename A::P						P;
	typedef typename A::P::E					E;
	typedef typename A::P::uint_t				pint_t;
	
	void										encodeStubSection(ld::Internal::FinalSection* sect);
	void										encodeLazyPointerSection(ld::Internal::FinalSection* sect);
	void										encodeNonLazyPointerSection(ld::Internal::FinalSection* sect);
	void										encodeTLVPointerSection(ld::Internal::FinalSection* sect);
	uint32_t									symIndexOfStubAtom(const ld::Atom*);
	uint32_t									symIndexOfLazyPointerAtom(const ld::Atom*);
	uint32_t									symIndexOfNonLazyPointerAtom(const ld::Atom*);
	uint32_t									symbolIndex(const ld::Atom*);


	std::vector<uint32_t>						_entries;
	
	static ld::Section			_s_section;
};

template <typename A>
ld::Section IndirectSymbolTableAtom<A>::_s_section("__LINKEDIT", "__ind_sym_tab", ld::Section::typeLinkEdit, true);




template <typename A>
uint32_t IndirectSymbolTableAtom<A>::symbolIndex(const ld::Atom* atom)
{
	std::map<const ld::Atom*, uint32_t>::iterator pos = this->_writer._atomToSymbolIndex.find(atom);
	if ( pos != this->_writer._atomToSymbolIndex.end() )
		return pos->second;
	//fprintf(stderr, "_atomToSymbolIndex content:\n");
	//for(std::map<const ld::Atom*, uint32_t>::iterator it = this->_writer._atomToSymbolIndex.begin(); it != this->_writer._atomToSymbolIndex.end(); ++it) {
	//		fprintf(stderr, "%p(%s) => %d\n", it->first, it->first->name(), it->second);
	//}
	throwf("internal error: atom not found in symbolIndex(%s)", atom->name());
}

template <typename A>
uint32_t IndirectSymbolTableAtom<A>::symIndexOfStubAtom(const ld::Atom* stubAtom)
{
	for (ld::Fixup::iterator fit = stubAtom->fixupsBegin(); fit != stubAtom->fixupsEnd(); ++fit) {
		if ( fit->binding == ld::Fixup::bindingDirectlyBound ) {
			ld::Atom::ContentType type = fit->u.target->contentType();
			if (( type == ld::Atom::typeLazyPointer) || (type == ld::Atom::typeLazyDylibPointer) )
				return symIndexOfLazyPointerAtom(fit->u.target);
			if ( type == ld::Atom::typeNonLazyPointer )
				return symIndexOfNonLazyPointerAtom(fit->u.target);
		}
	}
	throw "internal error: stub missing fixup to lazy pointer";
}


template <typename A>
uint32_t IndirectSymbolTableAtom<A>::symIndexOfLazyPointerAtom(const ld::Atom* lpAtom)
{
	if ( lpAtom->contentType() == ld::Atom::typeLazyPointer || lpAtom->contentType() == ld::Atom::typeLazyDylibPointer ) {
		for (ld::Fixup::iterator fit = lpAtom->fixupsBegin(); fit != lpAtom->fixupsEnd(); ++fit) {
			if ( fit->kind == ld::Fixup::kindLazyTarget ) {
				assert(fit->binding == ld::Fixup::bindingDirectlyBound);
				return symbolIndex(fit->u.target);
			}
		}
	}
	else if ( lpAtom->contentType() == ld::Atom::typeNonLazyPointer ) {
		for (ld::Fixup::iterator fit = lpAtom->fixupsBegin(); fit != lpAtom->fixupsEnd(); ++fit) {
			if ( (fit->kind == ld::Fixup::kindStoreTargetAddressLittleEndian32) || (fit->kind == ld::Fixup::kindStoreTargetAddressLittleEndian64) ) {
				assert(fit->binding == ld::Fixup::bindingDirectlyBound);
				return symbolIndex(fit->u.target);
			}
		}
	}
	throw "internal error: lazy pointer missing fixupLazyTarget fixup";
}

template <typename A>
uint32_t IndirectSymbolTableAtom<A>::symIndexOfNonLazyPointerAtom(const ld::Atom* nlpAtom)
{
	//fprintf(stderr, "symIndexOfNonLazyPointerAtom(%p) %s\n", nlpAtom, nlpAtom->name());
	for (ld::Fixup::iterator fit = nlpAtom->fixupsBegin(); fit != nlpAtom->fixupsEnd(); ++fit) {
#if SUPPORT_ARCH_arm64e
		// Skip authentication fixups
		if ( fit->clusterSize == ld::Fixup::k1of2 ) {
			if ( fit->kind != ld::Fixup::kindSetAuthData )
				break;
			++fit;
		} else
#endif
		{
			if ( fit->clusterSize != ld::Fixup::k1of1 )
				return INDIRECT_SYMBOL_LOCAL;
		}
		const ld::Atom* target;
		switch ( fit->binding ) {
			case ld::Fixup::bindingDirectlyBound:
				target = fit->u.target;
				break;
			case ld::Fixup::bindingsIndirectlyBound:
				target = _state.indirectBindingTable[fit->u.bindingIndex];
				break;
			default:
				throw "internal error: unexpected non-lazy pointer binding";
		}
		bool targetIsGlobal = (target->scope() == ld::Atom::scopeGlobal);
		switch ( target->definition() ) {
			case ld::Atom::definitionRegular:
				if ( targetIsGlobal ) {
					if ( _options.outputKind() == Options::kObjectFile ) {
						// nlpointer to global symbol uses indirect symbol table in .o files
						return symbolIndex(target);
					} 
					else if ( target->combine() == ld::Atom::combineByName ) {
						// dyld needs to bind nlpointer to global weak def
						return symbolIndex(target);
					}
					else if ( _options.nameSpace() != Options::kTwoLevelNameSpace ) {
						// dyld needs to bind nlpointer to global def linked for flat namespace
						return symbolIndex(target);
					}
				}
				break;
			case ld::Atom::definitionTentative:
			case ld::Atom::definitionAbsolute:
				if ( _options.outputKind() == Options::kObjectFile ) {
					// tentative def in .o file always uses symbol index
					return symbolIndex(target);
				}
				// dyld needs to bind nlpointer to global def linked for flat namespace
				if ( targetIsGlobal && _options.nameSpace() != Options::kTwoLevelNameSpace ) 
					return symbolIndex(target);
				break;
			case ld::Atom::definitionProxy:
				// dyld needs to bind nlpointer to something in another dylib
				return symbolIndex(target);
		}
	}
	if ( nlpAtom->fixupsBegin() == nlpAtom->fixupsEnd() ) {
		// no fixups means this is the ImageLoader cache slot
		return INDIRECT_SYMBOL_ABS;
	}
	
	// The magic index INDIRECT_SYMBOL_LOCAL tells dyld it should does not need to bind
	// this non-lazy pointer.
	return INDIRECT_SYMBOL_LOCAL;
}



template <typename A>
void IndirectSymbolTableAtom<A>::encodeStubSection(ld::Internal::FinalSection* sect)
{
	sect->indirectSymTabStartIndex = _entries.size();
	sect->indirectSymTabElementSize = sect->atoms[0]->size();
	for (std::vector<const ld::Atom*>::iterator ait = sect->atoms.begin(); ait != sect->atoms.end(); ++ait) {
		_entries.push_back(symIndexOfStubAtom(*ait));
	}
}

template <typename A>
void IndirectSymbolTableAtom<A>::encodeLazyPointerSection(ld::Internal::FinalSection* sect)
{
	sect->indirectSymTabStartIndex = _entries.size();
	for (std::vector<const ld::Atom*>::iterator ait = sect->atoms.begin(); ait != sect->atoms.end(); ++ait) {
		_entries.push_back(symIndexOfLazyPointerAtom(*ait));
	}
}

template <typename A>
void IndirectSymbolTableAtom<A>::encodeNonLazyPointerSection(ld::Internal::FinalSection* sect)
{
	sect->indirectSymTabStartIndex = _entries.size();
	for (std::vector<const ld::Atom*>::iterator ait = sect->atoms.begin(); ait != sect->atoms.end(); ++ait) {
		_entries.push_back(symIndexOfNonLazyPointerAtom(*ait));
	}
}

template <typename A>
void IndirectSymbolTableAtom<A>::encodeTLVPointerSection(ld::Internal::FinalSection* sect)
{
	sect->indirectSymTabStartIndex = _entries.size();
	for (std::vector<const ld::Atom*>::iterator ait = sect->atoms.begin(); ait != sect->atoms.end(); ++ait) {
		_entries.push_back(symIndexOfNonLazyPointerAtom(*ait));
	}
}


template <typename A>
void IndirectSymbolTableAtom<A>::encode()
{
	// static executables should not have an indirect symbol table, unless PIE
	if ( (this->_options.outputKind() == Options::kStaticExecutable) && !_options.positionIndependentExecutable() ) 
		return;

	// x86_64 kext bundles should not have an indirect symbol table unless using stubs
	if ( (this->_options.outputKind() == Options::kKextBundle) && !this->_options.kextsUseStubs() )
		return;

	// slidable static executables (-static -pie) should not have an indirect symbol table
	if ( (this->_options.outputKind() == Options::kStaticExecutable) && this->_options.positionIndependentExecutable() ) 
		return;

	// find all special sections that need a range of the indirect symbol table section
	for (std::vector<ld::Internal::FinalSection*>::iterator sit = this->_state.sections.begin(); sit != this->_state.sections.end(); ++sit) {
		ld::Internal::FinalSection* sect = *sit;
		switch ( sect->type() ) {
			case ld::Section::typeStub:
			case ld::Section::typeStubClose:
				this->encodeStubSection(sect);
				break;
			case ld::Section::typeLazyPointerClose:
			case ld::Section::typeLazyPointer:
			case ld::Section::typeLazyDylibPointer:
				this->encodeLazyPointerSection(sect);
				break;
			case ld::Section::typeNonLazyPointer:
				this->encodeNonLazyPointerSection(sect);
				break;
			case ld::Section::typeTLVPointers:
				this->encodeTLVPointerSection(sect);
				break;
			default:
				break;
		}
	}
}

template <typename A>
uint64_t IndirectSymbolTableAtom<A>::size() const
{
	return _entries.size() * sizeof(uint32_t);
}

template <typename A>
void IndirectSymbolTableAtom<A>::copyRawContent(uint8_t buffer[]) const
{
	uint32_t* array = (uint32_t*)buffer;
	for(unsigned long i=0; i < _entries.size(); ++i) {
		E::set32(array[i], _entries[i]);
	}
}








} // namespace tool 
} // namespace ld 

#endif // __LINKEDIT_CLASSIC_HPP__
