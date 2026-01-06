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

#ifndef __LINKEDIT_HPP__
#define __LINKEDIT_HPP__

#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <CommonCrypto/CommonDigest.h>
#include <CommonCrypto/CommonDigestSPI.h>

#include <vector>
#include <unordered_map>

#include "Options.h"
#include "ld.hpp"
#include "Architectures.hpp"
#include "MachOFileAbstraction.hpp"
#include "libcodedirectory.h"

#ifndef CS_LINKER_SIGNED
	#define CS_LINKER_SIGNED            0x00020000  /* Automatically signed by the linker */
#endif

namespace ld {
namespace tool {

class ByteStream {
private:
	std::vector<uint8_t>		_data;
public:
	std::vector<uint8_t>& bytes() { return _data; }
	unsigned long size() const { return _data.size(); }
	void reserve(unsigned long l) { _data.reserve(l); }
	const uint8_t* start() const { return &_data[0]; }

	void append_uleb128(uint64_t value) {
		uint8_t byte;
		do {
			byte = value & 0x7F;
			value &= ~0x7F;
			if ( value != 0 )
				byte |= 0x80;
			_data.push_back(byte);
			value = value >> 7;
		} while( byte >= 0x80 );
	}
	
	void append_sleb128(int64_t value) {
		bool isNeg = ( value < 0 );
		uint8_t byte;
		bool more;
		do {
			byte = value & 0x7F;
			value = value >> 7;
			if ( isNeg ) 
				more = ( (value != -1) || ((byte & 0x40) == 0) );
			else
				more = ( (value != 0) || ((byte & 0x40) != 0) );
			if ( more )
				byte |= 0x80;
			_data.push_back(byte);
		} 
		while( more );
	}
	
	void append_delta_encoded_uleb128_run(uint64_t start, const std::vector<uint64_t>& locations) {
		uint64_t lastAddr = start;
		for(std::vector<uint64_t>::const_iterator it = locations.begin(); it != locations.end(); ++it) {
			uint64_t nextAddr = *it;
			uint64_t delta = nextAddr - lastAddr;
			assert(delta != 0);
			append_uleb128(delta);
			lastAddr = nextAddr;
		}
	}

	void append_string(const char* str) {
		for (const char* s = str; *s != '\0'; ++s)
			_data.push_back(*s);
		_data.push_back('\0');
	}
	
	void append_byte(uint8_t byte) {
		_data.push_back(byte);
	}
	
	void append_mem(const void* mem, size_t len) {
		_data.insert(_data.end(), (uint8_t*)mem, (uint8_t*)mem + len);
	}

	// adds 'len' bytes to buffer and returns pointer to start of block
	uint8_t* alloc(size_t len) {
		size_t start = _data.size();
		for (size_t i=0; i < len; ++i)
			_data.push_back(0);
		return &_data[start];
	}

	static unsigned int	uleb128_size(uint64_t value) {
		uint32_t result = 0;
		do {
			value = value >> 7;
			++result;
		} while ( value != 0 );
		return result;
	}
	
	void pad_to_size(unsigned int alignment) {
		while ( (_data.size() % alignment) != 0 )
			_data.push_back(0);
	}
};


class LinkEditAtom : public ld::Atom
{
public:

	// overrides of ld::Atom
	virtual ld::File*							file() const		{ return NULL; }
	virtual uint64_t							objectAddress() const { return 0; }
	virtual uint64_t							size() const;
	virtual void								copyRawContent(uint8_t buffer[]) const; 

	virtual void								encode() const = 0;

	const uint8_t*								rawContent() const { return this->_encodedData.start(); }

												LinkEditAtom(const Options& opts, ld::Internal& state, 
																OutputFile& writer, const ld::Section& sect,
																unsigned int pointerSize)
												: ld::Atom(sect, ld::Atom::definitionRegular,
															ld::Atom::combineNever, ld::Atom::scopeTranslationUnit,
															ld::Atom::typeUnclassified, ld::Atom::symbolTableNotIn,
															false, false, false, ld::Atom::Alignment(log2(pointerSize))), 
														_options(opts), _state(state), _writer(writer), 
														_encoded(false) { }
protected:
	const Options&				_options;
	ld::Internal&				_state;
	OutputFile&					_writer;
	mutable ByteStream			_encodedData;
	mutable bool				_encoded;
};

uint64_t LinkEditAtom::size() const
{
	assert(_encoded);
	return _encodedData.size();
}

void LinkEditAtom::copyRawContent(uint8_t buffer[]) const
{
	assert(_encoded);
	memcpy(buffer, _encodedData.start(), _encodedData.size());
}




template <typename A>
class RebaseInfoAtom : public LinkEditAtom
{
public:
												RebaseInfoAtom(const Options& opts, ld::Internal& state, OutputFile& writer)
													: LinkEditAtom(opts, state, writer, _s_section, sizeof(pint_t)) { _encoded = true; }

	// overrides of ld::Atom
	virtual const char*							name() const		{ return "rebase info"; }
	// overrides of LinkEditAtom
	virtual void								encode() const;

private:
	void						encodeV1() const;

	struct rebase_tmp
	{
		rebase_tmp(uint8_t op, uint64_t p1, uint64_t p2=0) : opcode(op), operand1(p1), operand2(p2) {}
		uint8_t		opcode;
		uint64_t	operand1;
		uint64_t	operand2;
	};

	typedef typename A::P						P;
	typedef typename A::P::E					E;
	typedef typename A::P::uint_t				pint_t;
	
	static ld::Section			_s_section;
};

template <typename A>
ld::Section RebaseInfoAtom<A>::_s_section("__LINKEDIT", "__rebase", ld::Section::typeLinkEdit, true);


template <typename A>
void RebaseInfoAtom<A>::encode() const
{
	// omit relocs if this was supposed to be PIE but PIE not possible
	if ( _options.positionIndependentExecutable() && this->_writer.pieDisabled ) 
		return;

	// sort rebase info by type, then address
	std::vector<OutputFile::RebaseInfo>& info = this->_writer._rebaseInfo;
	if (info.empty())
		return;

	std::sort(info.begin(), info.end());
	
	// use encoding based on target minOS
	if ( _options.useLinkedListBinding() && !this->_writer._hasUnalignedFixup ) {
		if ( info.back()._type != REBASE_TYPE_POINTER )
			throw "unsupported rebase type with linked list opcodes";
		// As the binding and rebasing are both linked lists, just use the binds
		// to do everything.
	} else {
		encodeV1();
	}
}


template <typename A>
void RebaseInfoAtom<A>::encodeV1() const
{
	std::vector<OutputFile::RebaseInfo>& info = this->_writer._rebaseInfo;

	// convert to temp encoding that can be more easily optimized
	std::vector<rebase_tmp> mid;
	uint64_t curSegStart = 0;
	uint64_t curSegEnd = 0;
	uint32_t curSegIndex = 0;	
	uint8_t type = 0;
	uint64_t address = (uint64_t)(-1);
	for (std::vector<OutputFile::RebaseInfo>::iterator it = info.begin(); it != info.end(); ++it) {
		if ( type != it->_type ) {
			mid.push_back(rebase_tmp(REBASE_OPCODE_SET_TYPE_IMM, it->_type));
			type = it->_type;
		}
		if ( address != it->_address ) {
			if ( (it->_address < curSegStart) || ( it->_address >= curSegEnd) ) {
				if ( ! this->_writer.findSegment(this->_state, it->_address, &curSegStart, &curSegEnd, &curSegIndex) )
					throw "binding address outside range of any segment";
				mid.push_back(rebase_tmp(REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB, curSegIndex, it->_address - curSegStart));
			}
			else {
				mid.push_back(rebase_tmp(REBASE_OPCODE_ADD_ADDR_ULEB, it->_address-address));
			}
			address = it->_address;
		}
		mid.push_back(rebase_tmp(REBASE_OPCODE_DO_REBASE_ULEB_TIMES, 1));
		address += sizeof(pint_t);
		if ( address >= curSegEnd )
			address = 0;
	}
	mid.push_back(rebase_tmp(REBASE_OPCODE_DONE, 0));

	// optimize phase 1, compress packed runs of pointers
	rebase_tmp* dst = &mid[0];
	for (const rebase_tmp* src = &mid[0]; src->opcode != REBASE_OPCODE_DONE; ++src) {
		if ( (src->opcode == REBASE_OPCODE_DO_REBASE_ULEB_TIMES) && (src->operand1 == 1) ) {
			*dst = *src++;
			while (src->opcode == REBASE_OPCODE_DO_REBASE_ULEB_TIMES ) {
				dst->operand1 += src->operand1;
				++src;
			}
			--src;
			++dst;
		}
		else {
			*dst++ = *src;
		}
	}
	dst->opcode = REBASE_OPCODE_DONE;

	// optimize phase 2, combine rebase/add pairs
	dst = &mid[0];
	for (const rebase_tmp* src = &mid[0]; src->opcode != REBASE_OPCODE_DONE; ++src) {
		if ( (src->opcode == REBASE_OPCODE_DO_REBASE_ULEB_TIMES) 
				&& (src->operand1 == 1) 
				&& (src[1].opcode == REBASE_OPCODE_ADD_ADDR_ULEB)) {
			dst->opcode = REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB;
			dst->operand1 = src[1].operand1;
			++src;
			++dst;
		}
		else {
			*dst++ = *src;
		}
	}
	dst->opcode = REBASE_OPCODE_DONE;
	
	// optimize phase 3, compress packed runs of REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB with
	// same addr delta into one REBASE_OPCODE_DO_REBASE_ULEB_TIMES_SKIPPING_ULEB
	dst = &mid[0];
	for (const rebase_tmp* src = &mid[0]; src->opcode != REBASE_OPCODE_DONE; ++src) {
		uint64_t delta = src->operand1;
		if ( (src->opcode == REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB) 
				&& (src[1].opcode == REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB) 
				&& (src[2].opcode == REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB) 
				&& (src[1].operand1 == delta) 
				&& (src[2].operand1 == delta) ) {
			// found at least three in a row, this is worth compressing
			dst->opcode = REBASE_OPCODE_DO_REBASE_ULEB_TIMES_SKIPPING_ULEB;
			dst->operand1 = 1;
			dst->operand2 = delta;
			++src;
			while ( (src->opcode == REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB)
					&& (src->operand1 == delta) ) {
				dst->operand1++;
				++src;
			}
			--src;
			++dst;
		}
		else {
			*dst++ = *src;
		}
	}
	dst->opcode = REBASE_OPCODE_DONE;
	
	// optimize phase 4, use immediate encodings
	for (rebase_tmp* p = &mid[0]; p->opcode != REBASE_OPCODE_DONE; ++p) {
		if ( (p->opcode == REBASE_OPCODE_ADD_ADDR_ULEB) 
			&& (p->operand1 < (15*sizeof(pint_t)))
			&& ((p->operand1 % sizeof(pint_t)) == 0) ) {
			p->opcode = REBASE_OPCODE_ADD_ADDR_IMM_SCALED;
			p->operand1 = p->operand1/sizeof(pint_t);
		}
		else if ( (p->opcode == REBASE_OPCODE_DO_REBASE_ULEB_TIMES) && (p->operand1 < 15) ) {
			p->opcode = REBASE_OPCODE_DO_REBASE_IMM_TIMES;
		}
	}

	// convert to compressed encoding
	const static bool log = false;
	this->_encodedData.reserve(info.size()*2);
	bool done = false;
	for (typename std::vector<rebase_tmp>::iterator it = mid.begin(); !done && it != mid.end() ; ++it) {
		switch ( it->opcode ) {
			case REBASE_OPCODE_DONE:
				if ( log ) fprintf(stderr, "REBASE_OPCODE_DONE()\n");
				done = true;
				break;
			case REBASE_OPCODE_SET_TYPE_IMM:
				if ( log ) fprintf(stderr, "REBASE_OPCODE_SET_TYPE_IMM(%lld)\n", it->operand1);
				this->_encodedData.append_byte(REBASE_OPCODE_SET_TYPE_IMM | it->operand1);
				break;
			case REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
				if ( log ) fprintf(stderr, "REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB(%lld, 0x%llX)\n", it->operand1, it->operand2);
				this->_encodedData.append_byte(REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB | it->operand1);
				this->_encodedData.append_uleb128(it->operand2);
				break;
			case REBASE_OPCODE_ADD_ADDR_ULEB:
				if ( log ) fprintf(stderr, "REBASE_OPCODE_ADD_ADDR_ULEB(0x%llX)\n", it->operand1);
				this->_encodedData.append_byte(REBASE_OPCODE_ADD_ADDR_ULEB);
				this->_encodedData.append_uleb128(it->operand1);
				break;
			case REBASE_OPCODE_ADD_ADDR_IMM_SCALED:
				if ( log ) fprintf(stderr, "REBASE_OPCODE_ADD_ADDR_IMM_SCALED(%lld=0x%llX)\n", it->operand1, it->operand1*sizeof(pint_t));
				this->_encodedData.append_byte(REBASE_OPCODE_ADD_ADDR_IMM_SCALED | it->operand1 );
				break;
			case REBASE_OPCODE_DO_REBASE_IMM_TIMES:
				if ( log ) fprintf(stderr, "REBASE_OPCODE_DO_REBASE_IMM_TIMES(%lld)\n", it->operand1);
				this->_encodedData.append_byte(REBASE_OPCODE_DO_REBASE_IMM_TIMES | it->operand1);
				break;
			case REBASE_OPCODE_DO_REBASE_ULEB_TIMES:
				if ( log ) fprintf(stderr, "REBASE_OPCODE_DO_REBASE_ULEB_TIMES(%lld)\n", it->operand1);
				this->_encodedData.append_byte(REBASE_OPCODE_DO_REBASE_ULEB_TIMES);
				this->_encodedData.append_uleb128(it->operand1);
				break;
			case REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB:
				if ( log ) fprintf(stderr, "REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB(0x%llX)\n", it->operand1);
				this->_encodedData.append_byte(REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB);
				this->_encodedData.append_uleb128(it->operand1);
				break;
			case REBASE_OPCODE_DO_REBASE_ULEB_TIMES_SKIPPING_ULEB:
				if ( log ) fprintf(stderr, "REBASE_OPCODE_DO_REBASE_ULEB_TIMES_SKIPPING_ULEB(%lld, %lld)\n", it->operand1, it->operand2);
				this->_encodedData.append_byte(REBASE_OPCODE_DO_REBASE_ULEB_TIMES_SKIPPING_ULEB);
				this->_encodedData.append_uleb128(it->operand1);
				this->_encodedData.append_uleb128(it->operand2);
				break;
		}
	}
	
		
	// align to pointer size
	this->_encodedData.pad_to_size(sizeof(pint_t));

	this->_encoded = true;

	if (log) fprintf(stderr, "total rebase info size = %ld\n", this->_encodedData.size());
}


template <typename A>
class BindingInfoAtom : public LinkEditAtom
{
public:
												BindingInfoAtom(const Options& opts, ld::Internal& state, OutputFile& writer)
													: LinkEditAtom(opts, state, writer, _s_section, sizeof(pint_t)) { }

	// overrides of ld::Atom
	virtual const char*							name() const		{ return "binding info"; }
	// overrides of LinkEditAtom
	virtual void								encode() const;


private:
	void						encodeV1() const;
	void						encodeV2() const;

	typedef typename A::P						P;
	typedef typename A::P::E					E;
	typedef typename A::P::uint_t				pint_t;

	struct binding_tmp
	{
		binding_tmp(uint8_t op, uint64_t p1, uint64_t p2=0, const char* s=NULL) 
			: opcode(op), operand1(p1), operand2(p2), name(s) {}
		uint8_t		opcode;
		uint64_t	operand1;
		uint64_t	operand2;
		const char*	name;
	};

	static ld::Section			_s_section;
};

template <typename A>
ld::Section BindingInfoAtom<A>::_s_section("__LINKEDIT", "__binding", ld::Section::typeLinkEdit, true);


template <typename A>
void BindingInfoAtom<A>::encode() const
{
	// use encoding based on target minOS
	if ( _options.useLinkedListBinding() && !this->_writer._hasUnalignedFixup ) {
		encodeV2();
	} else {
		encodeV1();
	}
}


template <typename A>
void BindingInfoAtom<A>::encodeV1() const
{
	// sort by library, symbol, type, then address
	std::vector<OutputFile::BindingInfo>& info = this->_writer._bindingInfo;
	std::sort(info.begin(), info.end());

	// convert to temp encoding that can be more easily optimized
	std::vector<binding_tmp> mid;
	uint64_t curSegStart = 0;
	uint64_t curSegEnd = 0;
	uint32_t curSegIndex = 0;	
	int ordinal = 0x80000000;
	const char* symbolName = NULL;
	uint8_t type = 0;
	uint64_t address = (uint64_t)(-1);
	int64_t addend = 0;
	for (std::vector<OutputFile::BindingInfo>::const_iterator it = info.begin(); it != info.end(); ++it) {
		if ( ordinal != it->_libraryOrdinal ) {
			if ( it->_libraryOrdinal <= 0 ) {
				// special lookups are encoded as negative numbers in BindingInfo
				mid.push_back(binding_tmp(BIND_OPCODE_SET_DYLIB_SPECIAL_IMM, it->_libraryOrdinal));
			}
			else {
				mid.push_back(binding_tmp(BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB, it->_libraryOrdinal));
			}
			ordinal = it->_libraryOrdinal;
		}
		if ( symbolName != it->_symbolName ) {
			mid.push_back(binding_tmp(BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM, it->_flags, 0, it->_symbolName));
			symbolName = it->_symbolName;
		}
		if ( type != it->_type ) {
			mid.push_back(binding_tmp(BIND_OPCODE_SET_TYPE_IMM, it->_type));
			type = it->_type;
		}
		if ( address != it->_address ) {
			if ( (it->_address < curSegStart) || ( it->_address >= curSegEnd) ) {
				if ( ! this->_writer.findSegment(this->_state, it->_address, &curSegStart, &curSegEnd, &curSegIndex) )
					throw "binding address outside range of any segment";
				mid.push_back(binding_tmp(BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB, curSegIndex, it->_address - curSegStart));
			}
			else {
				mid.push_back(binding_tmp(BIND_OPCODE_ADD_ADDR_ULEB, it->_address-address));
			}
			address = it->_address;
		}
		if ( addend != it->_addend ) {
			mid.push_back(binding_tmp(BIND_OPCODE_SET_ADDEND_SLEB, it->_addend));
			addend = it->_addend;
		}
		mid.push_back(binding_tmp(BIND_OPCODE_DO_BIND, 0));
		address += sizeof(pint_t);
	}
	mid.push_back(binding_tmp(BIND_OPCODE_DONE, 0));


	// optimize phase 1, combine bind/add pairs
	binding_tmp* dst = &mid[0];
	for (const binding_tmp* src = &mid[0]; src->opcode != BIND_OPCODE_DONE; ++src) {
		if ( (src->opcode == BIND_OPCODE_DO_BIND) 
				&& (src[1].opcode == BIND_OPCODE_ADD_ADDR_ULEB) ) {
			dst->opcode = BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB;
			dst->operand1 = src[1].operand1;
			++src;
			++dst;
		}
		else {
			*dst++ = *src;
		}
	}
	dst->opcode = BIND_OPCODE_DONE;

	// optimize phase 2, compress packed runs of BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB with
	// same addr delta into one BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB
	dst = &mid[0];
	for (const binding_tmp* src = &mid[0]; src->opcode != BIND_OPCODE_DONE; ++src) {
		uint64_t delta = src->operand1;
		if ( (src->opcode == BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB) 
				&& (src[1].opcode == BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB) 
				&& (src[1].operand1 == delta) ) {
			// found at least two in a row, this is worth compressing
			dst->opcode = BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB;
			dst->operand1 = 1;
			dst->operand2 = delta;
			++src;
			while ( (src->opcode == BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB)
					&& (src->operand1 == delta) ) {
				dst->operand1++;
				++src;
			}
			--src;
			++dst;
		}
		else {
			*dst++ = *src;
		}
	}
	dst->opcode = BIND_OPCODE_DONE;
	
	// optimize phase 3, use immediate encodings
	for (binding_tmp* p = &mid[0]; p->opcode != REBASE_OPCODE_DONE; ++p) {
		if ( (p->opcode == BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB) 
			&& (p->operand1 < (15*sizeof(pint_t)))
			&& ((p->operand1 % sizeof(pint_t)) == 0) ) {
			p->opcode = BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED;
			p->operand1 = p->operand1/sizeof(pint_t);
		}
		else if ( (p->opcode == BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB) && (p->operand1 <= 15) ) {
			p->opcode = BIND_OPCODE_SET_DYLIB_ORDINAL_IMM;
		}
	}	
	dst->opcode = BIND_OPCODE_DONE;

	// convert to compressed encoding
	const static bool log = false;
	this->_encodedData.reserve(info.size()*2);
	bool done = false;
	for (typename std::vector<binding_tmp>::iterator it = mid.begin(); !done && it != mid.end() ; ++it) {
		switch ( it->opcode ) {
			case BIND_OPCODE_DONE:
				if ( log ) fprintf(stderr, "BIND_OPCODE_DONE()\n");
				done = true;
				break;
			case BIND_OPCODE_SET_DYLIB_ORDINAL_IMM:
				if ( log ) fprintf(stderr, "BIND_OPCODE_SET_DYLIB_ORDINAL_IMM(%lld)\n", it->operand1);
				this->_encodedData.append_byte(BIND_OPCODE_SET_DYLIB_ORDINAL_IMM | it->operand1);
				break;
			case BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB:
				if ( log ) fprintf(stderr, "BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB(%lld)\n", it->operand1);
				this->_encodedData.append_byte(BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB);
				this->_encodedData.append_uleb128(it->operand1);
				break;
			case BIND_OPCODE_SET_DYLIB_SPECIAL_IMM:
				if ( log ) fprintf(stderr, "BIND_OPCODE_SET_DYLIB_SPECIAL_IMM(%lld)\n", it->operand1);
				this->_encodedData.append_byte(BIND_OPCODE_SET_DYLIB_SPECIAL_IMM | (it->operand1 & BIND_IMMEDIATE_MASK));
				break;
			case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
				if ( log ) fprintf(stderr, "BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM(0x%0llX, %s)\n", it->operand1, it->name);
				this->_encodedData.append_byte(BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM | it->operand1);
				this->_encodedData.append_string(it->name);
				break;
			case BIND_OPCODE_SET_TYPE_IMM:
				if ( log ) fprintf(stderr, "BIND_OPCODE_SET_TYPE_IMM(%lld)\n", it->operand1);
				this->_encodedData.append_byte(BIND_OPCODE_SET_TYPE_IMM | it->operand1);
				break;
			case BIND_OPCODE_SET_ADDEND_SLEB:
				if ( log ) fprintf(stderr, "BIND_OPCODE_SET_ADDEND_SLEB(%lld)\n", it->operand1);
				this->_encodedData.append_byte(BIND_OPCODE_SET_ADDEND_SLEB);
				this->_encodedData.append_sleb128(it->operand1);
				break;
			case BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
				if ( log ) fprintf(stderr, "BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB(%lld, 0x%llX)\n", it->operand1, it->operand2);
				this->_encodedData.append_byte(BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB | it->operand1);
				this->_encodedData.append_uleb128(it->operand2);
				break;
			case BIND_OPCODE_ADD_ADDR_ULEB:
				if ( log ) fprintf(stderr, "BIND_OPCODE_ADD_ADDR_ULEB(0x%llX)\n", it->operand1);
				this->_encodedData.append_byte(BIND_OPCODE_ADD_ADDR_ULEB);
				this->_encodedData.append_uleb128(it->operand1);
				break;
			case BIND_OPCODE_DO_BIND:
				if ( log ) fprintf(stderr, "BIND_OPCODE_DO_BIND()\n");
				this->_encodedData.append_byte(BIND_OPCODE_DO_BIND);
				break;
			case BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
				if ( log ) fprintf(stderr, "BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB(0x%llX)\n", it->operand1);
				this->_encodedData.append_byte(BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB);
				this->_encodedData.append_uleb128(it->operand1);
				break;
			case BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
				if ( log ) fprintf(stderr, "BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED(%lld=0x%llX)\n", it->operand1, it->operand1*sizeof(pint_t));
				this->_encodedData.append_byte(BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED | it->operand1 );
				break;
			case BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB:
				if ( log ) fprintf(stderr, "BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB(%lld, %lld)\n", it->operand1, it->operand2);
				this->_encodedData.append_byte(BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB);
				this->_encodedData.append_uleb128(it->operand1);
				this->_encodedData.append_uleb128(it->operand2);
				break;
		}
	}
	
	// align to pointer size
	this->_encodedData.pad_to_size(sizeof(pint_t));

	this->_encoded = true;

	if (log) fprintf(stderr, "total binding info size = %ld\n", this->_encodedData.size());
}

template <typename A>
void BindingInfoAtom<A>::encodeV2() const
{
	std::vector<OutputFile::BindingInfo>& bindInfo = this->_writer._bindingInfo;
	std::vector<OutputFile::RebaseInfo>& rebaseInfo = this->_writer._rebaseInfo;
	const static bool log = false;

	std::sort(bindInfo.begin(), bindInfo.end());

	// convert to temp encoding that can be more easily optimized
	std::vector<binding_tmp> mid;
	uint64_t curSegStart = 0;
	uint64_t curSegEnd = 0;
	uint32_t curSegIndex = 0;
	int ordinal = 0x80000000;
	const char* symbolName = NULL;
	uint8_t type = 0;
	uint64_t address = (uint64_t)(-1);
	int64_t addend = 0;
	uint64_t numBinds = (uint64_t)(-1);
	for (std::vector<OutputFile::BindingInfo>::iterator it = bindInfo.begin(); it != bindInfo.end(); ++it) {
		bool madeChange = false;
		if ( ordinal != it->_libraryOrdinal ) {
			if ( it->_libraryOrdinal <= 0 ) {
				// special lookups are encoded as negative numbers in BindingInfo
				mid.push_back(binding_tmp(BIND_OPCODE_SET_DYLIB_SPECIAL_IMM, it->_libraryOrdinal));
			}
			else if ( it->_libraryOrdinal <= 15 ) {
				mid.push_back(binding_tmp(BIND_OPCODE_SET_DYLIB_ORDINAL_IMM, it->_libraryOrdinal));
			}
			else {
				mid.push_back(binding_tmp(BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB, it->_libraryOrdinal));
			}
			ordinal = it->_libraryOrdinal;
			madeChange = true;
		}
		if ( symbolName != it->_symbolName ) {
			mid.push_back(binding_tmp(BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM, it->_flags, 0, it->_symbolName));
			symbolName = it->_symbolName;
			madeChange = true;
		}
		if ( type != it->_type ) {
			if ( it->_type != BIND_TYPE_POINTER )
				throw "unsupported bind type with linked list opcodes";
			mid.push_back(binding_tmp(BIND_OPCODE_SET_TYPE_IMM, it->_type));
			type = it->_type;
			madeChange = true;
		}
		if ( address != it->_address ) {
			// Note, we don't push the addresses here.  That is all done later with the threaded chains
			if ( (it->_address < curSegStart) || ( it->_address >= curSegEnd) ) {
				if ( ! this->_writer.findSegment(this->_state, it->_address, &curSegStart, &curSegEnd, &curSegIndex) )
					throw "binding address outside range of any segment";
			}
			address = it->_address;
		}
		if ( addend != it->_addend ) {
			mid.push_back(binding_tmp(BIND_OPCODE_SET_ADDEND_SLEB, it->_addend));
			addend = it->_addend;
			madeChange = true;
		}

		if (madeChange) {
			++numBinds;
			mid.push_back(binding_tmp(BIND_OPCODE_DO_BIND, 0));
		}
		it->_threadedBindOrdinal = numBinds;
	}

	// We can only support 2^16 bind ordinals.
	if ( (numBinds > 0x10000) && (numBinds != (uint64_t)(-1)) )
		throwf("too many binds (%llu).  The limit is 65536", numBinds);

	// Now that we have the bind ordinal table populate, set the page starts.

	std::vector<int64_t>& threadedRebaseBindIndices = this->_writer._threadedRebaseBindIndices;
	threadedRebaseBindIndices.reserve(bindInfo.size() + rebaseInfo.size());

	for (int64_t i = 0, e = rebaseInfo.size(); i != e; ++i)
		threadedRebaseBindIndices.push_back(-i);

	for (int64_t i = 0, e = bindInfo.size(); i != e; ++i)
		threadedRebaseBindIndices.push_back(i + 1);

	// Now sort the entries by address.
	std::sort(threadedRebaseBindIndices.begin(), threadedRebaseBindIndices.end(),
			  [&rebaseInfo, &bindInfo](int64_t indexA, int64_t indexB) {
				  if (indexA == indexB)
					  return false;
				  uint64_t addressA = indexA <= 0 ? rebaseInfo[-indexA]._address : bindInfo[indexA - 1]._address;
				  uint64_t addressB = indexB <= 0 ? rebaseInfo[-indexB]._address : bindInfo[indexB - 1]._address;
				  assert(addressA != addressB);
				  return addressA < addressB;
			  });

	curSegStart = 0;
	curSegEnd = 0;
	curSegIndex = 0;
	uint64_t prevPageIndex = 0;
	for (int64_t entryIndex : threadedRebaseBindIndices) {
		OutputFile::RebaseInfo* rebase = nullptr;
		OutputFile::BindingInfo* bind = nullptr;
		uint64_t address = 0;
		if (entryIndex <= 0) {
			rebase = &rebaseInfo[-entryIndex];
			address = rebase->_address;
		} else {
			bind = &bindInfo[entryIndex - 1];
			address = bind->_address;
		}
		assert((address & 7) == 0);

		bool newSegment = false;
		if ( (address < curSegStart) || ( address >= curSegEnd) ) {
			// Start of a new segment.
			if ( ! this->_writer.findSegment(this->_state, address, &curSegStart, &curSegEnd, &curSegIndex) )
				throw "binding address outside range of any segment";
			newSegment = true;
		}

		// At this point we know we have the page starts array space reserved
		// so set the page start for this entry if we haven't got one already.
		uint64_t pageIndex = ( address - curSegStart ) / 4096;
		if ( newSegment || (pageIndex != prevPageIndex) ) {
			mid.push_back(binding_tmp(BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB, curSegIndex, address - curSegStart));
			mid.push_back(binding_tmp(BIND_OPCODE_THREADED | BIND_SUBOPCODE_THREADED_APPLY, 0));
		}
		prevPageIndex = pageIndex;
	}
	mid.push_back(binding_tmp(BIND_OPCODE_DONE, 0));

	// convert to compressed encoding
	this->_encodedData.reserve(bindInfo.size()*2);

	// First push the total number of binds so that we can allocate space for this in dyld.
	if ( log ) fprintf(stderr, "BIND_SUBOPCODE_THREADED_SET_BIND_ORDINAL_TABLE_SIZE_ULEB(%lld)\n", numBinds + 1);
	this->_encodedData.append_byte(BIND_OPCODE_THREADED | BIND_SUBOPCODE_THREADED_SET_BIND_ORDINAL_TABLE_SIZE_ULEB);
	this->_encodedData.append_uleb128(numBinds + 1);

	bool done = false;
	for (typename std::vector<binding_tmp>::iterator it = mid.begin(); !done && it != mid.end() ; ++it) {
		switch ( it->opcode ) {
			case BIND_OPCODE_DONE:
				if ( log ) fprintf(stderr, "BIND_OPCODE_DONE()\n");
				done = true;
				break;
			case BIND_OPCODE_SET_DYLIB_ORDINAL_IMM:
				if ( log ) fprintf(stderr, "BIND_OPCODE_SET_DYLIB_ORDINAL_IMM(%lld)\n", it->operand1);
				this->_encodedData.append_byte(BIND_OPCODE_SET_DYLIB_ORDINAL_IMM | it->operand1);
				break;
			case BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB:
				if ( log ) fprintf(stderr, "BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB(%lld)\n", it->operand1);
				this->_encodedData.append_byte(BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB);
				this->_encodedData.append_uleb128(it->operand1);
				break;
			case BIND_OPCODE_SET_DYLIB_SPECIAL_IMM:
				if ( log ) fprintf(stderr, "BIND_OPCODE_SET_DYLIB_SPECIAL_IMM(%lld)\n", it->operand1);
				this->_encodedData.append_byte(BIND_OPCODE_SET_DYLIB_SPECIAL_IMM | (it->operand1 & BIND_IMMEDIATE_MASK));
				break;
			case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
				if ( log ) fprintf(stderr, "BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM(0x%0llX, %s)\n", it->operand1, it->name);
				this->_encodedData.append_byte(BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM | it->operand1);
				this->_encodedData.append_string(it->name);
				break;
			case BIND_OPCODE_SET_TYPE_IMM:
				if ( log ) fprintf(stderr, "BIND_OPCODE_SET_TYPE_IMM(%lld)\n", it->operand1);
				this->_encodedData.append_byte(BIND_OPCODE_SET_TYPE_IMM | it->operand1);
				break;
			case BIND_OPCODE_SET_ADDEND_SLEB:
				if ( log ) fprintf(stderr, "BIND_OPCODE_SET_ADDEND_SLEB(%lld)\n", it->operand1);
				this->_encodedData.append_byte(BIND_OPCODE_SET_ADDEND_SLEB);
				this->_encodedData.append_sleb128(it->operand1);
				break;
			case BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
				if ( log ) fprintf(stderr, "BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB(%lld, 0x%llX)\n", it->operand1, it->operand2);
				this->_encodedData.append_byte(BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB | it->operand1);
				this->_encodedData.append_uleb128(it->operand2);
				break;
			case BIND_OPCODE_ADD_ADDR_ULEB:
				if ( log ) fprintf(stderr, "BIND_OPCODE_ADD_ADDR_ULEB(0x%llX)\n", it->operand1);
				this->_encodedData.append_byte(BIND_OPCODE_ADD_ADDR_ULEB);
				this->_encodedData.append_uleb128(it->operand1);
				break;
			case BIND_OPCODE_DO_BIND:
				if ( log ) fprintf(stderr, "BIND_OPCODE_DO_BIND()\n");
				this->_encodedData.append_byte(BIND_OPCODE_DO_BIND);
				break;
			case BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
				if ( log ) fprintf(stderr, "BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB(0x%llX)\n", it->operand1);
				this->_encodedData.append_byte(BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB);
				this->_encodedData.append_uleb128(it->operand1);
				break;
			case BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
				if ( log ) fprintf(stderr, "BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED(%lld=0x%llX)\n", it->operand1, it->operand1*sizeof(pint_t));
				this->_encodedData.append_byte(BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED | it->operand1 );
				break;
			case BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB:
				if ( log ) fprintf(stderr, "BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB(%lld, %lld)\n", it->operand1, it->operand2);
				this->_encodedData.append_byte(BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB);
				this->_encodedData.append_uleb128(it->operand1);
				this->_encodedData.append_uleb128(it->operand2);
				break;
			case BIND_OPCODE_THREADED | BIND_SUBOPCODE_THREADED_SET_BIND_ORDINAL_TABLE_SIZE_ULEB:
				if ( log ) fprintf(stderr, "BIND_SUBOPCODE_THREADED_SET_BIND_ORDINAL_TABLE_SIZE_ULEB(%lld)\n", it->operand1);
				this->_encodedData.append_byte(BIND_OPCODE_THREADED | BIND_SUBOPCODE_THREADED_SET_BIND_ORDINAL_TABLE_SIZE_ULEB);
				this->_encodedData.append_uleb128(it->operand1);
				break;
			case BIND_OPCODE_THREADED | BIND_SUBOPCODE_THREADED_APPLY:
				this->_encodedData.append_byte(BIND_OPCODE_THREADED | BIND_SUBOPCODE_THREADED_APPLY);
				if ( log ) fprintf(stderr, "BIND_SUBOPCODE_THREADED_APPLY()\n");
				break;
		}
	}

	// align to pointer size
	this->_encodedData.append_byte(BIND_OPCODE_DONE);
	this->_encodedData.pad_to_size(sizeof(pint_t));

	this->_encoded = true;

	if (log) fprintf(stderr, "total binding info size = %ld\n", this->_encodedData.size());
}



template <typename A>
class WeakBindingInfoAtom : public LinkEditAtom
{
public:
												WeakBindingInfoAtom(const Options& opts, ld::Internal& state, OutputFile& writer)
													: LinkEditAtom(opts, state, writer, _s_section, sizeof(pint_t)) { _encoded = true; }

	// overrides of ld::Atom
	virtual const char*							name() const		{ return "weak binding info"; }
	// overrides of LinkEditAtom
	virtual void								encode() const;

private:
	typedef typename A::P						P;
	typedef typename A::P::E					E;
	typedef typename A::P::uint_t				pint_t;

	struct WeakBindingSorter
	{	
		 bool operator()(const OutputFile::BindingInfo& left, const OutputFile::BindingInfo& right)
		 {
			// sort by symbol, type, address
			if ( left._symbolName != right._symbolName )
				return ( strcmp(left._symbolName, right._symbolName) < 0 );
			if ( left._type != right._type )
				return  (left._type < right._type);
			return  (left._address < right._address);
		 }
	};
	
	struct binding_tmp
	{
		binding_tmp(uint8_t op, uint64_t p1, uint64_t p2=0, const char* s=NULL) 
			: opcode(op), operand1(p1), operand2(p2), name(s) {}
		uint8_t		opcode;
		uint64_t	operand1;
		uint64_t	operand2;
		const char*	name;
	};

	static ld::Section			_s_section;
};

template <typename A>
ld::Section WeakBindingInfoAtom<A>::_s_section("__LINKEDIT", "__weak_binding", ld::Section::typeLinkEdit, true);


template <typename A>
void WeakBindingInfoAtom<A>::encode() const
{
	// sort by symbol, type, address
	std::vector<OutputFile::BindingInfo>& info = this->_writer._weakBindingInfo;
	if ( info.size() == 0 ) {
		// short circuit if no weak binding needed
		this->_encoded = true;
		return;
	}
	std::sort(info.begin(), info.end(), WeakBindingSorter());
	
	// convert to temp encoding that can be more easily optimized
	std::vector<binding_tmp> mid;
	mid.reserve(info.size());
	uint64_t curSegStart = 0;
	uint64_t curSegEnd = 0;
	uint32_t curSegIndex = 0;	
	const char* symbolName = NULL;
	uint8_t type = 0;
	uint64_t address = (uint64_t)(-1);
	int64_t addend = 0;
	for (typename std::vector<OutputFile::BindingInfo>::const_iterator it = info.begin(); it != info.end(); ++it) {
		if ( symbolName != it->_symbolName ) {
			mid.push_back(binding_tmp(BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM, it->_flags, 0, it->_symbolName));
			symbolName = it->_symbolName;
		}
		// non-weak symbols just have BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM
		// weak symbols have SET_SEG, ADD_ADDR, SET_ADDED, DO_BIND
		if ( it->_type != BIND_TYPE_OVERRIDE_OF_WEAKDEF_IN_DYLIB ) {
			if ( type != it->_type ) {
				mid.push_back(binding_tmp(BIND_OPCODE_SET_TYPE_IMM, it->_type));
				type = it->_type;
			}
			if ( address != it->_address ) {
				if ( (it->_address < curSegStart) || ( it->_address >= curSegEnd) ) {
					if ( ! this->_writer.findSegment(this->_state, it->_address, &curSegStart, &curSegEnd, &curSegIndex) )
						throw "binding address outside range of any segment";
					mid.push_back(binding_tmp(BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB, curSegIndex, it->_address - curSegStart));
				}
				else {
					mid.push_back(binding_tmp(BIND_OPCODE_ADD_ADDR_ULEB, it->_address-address));
				}
				address = it->_address;
			}
			if ( addend != it->_addend ) {
				mid.push_back(binding_tmp(BIND_OPCODE_SET_ADDEND_SLEB, it->_addend));
				addend = it->_addend;
			}
			mid.push_back(binding_tmp(BIND_OPCODE_DO_BIND, 0));
			address += sizeof(pint_t);
		}
	}
	mid.push_back(binding_tmp(BIND_OPCODE_DONE, 0));


	// optimize phase 1, combine bind/add pairs
	binding_tmp* dst = &mid[0];
	for (const binding_tmp* src = &mid[0]; src->opcode != BIND_OPCODE_DONE; ++src) {
		if ( (src->opcode == BIND_OPCODE_DO_BIND) 
				&& (src[1].opcode == BIND_OPCODE_ADD_ADDR_ULEB) ) {
			dst->opcode = BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB;
			dst->operand1 = src[1].operand1;
			++src;
			++dst;
		}
		else {
			*dst++ = *src;
		}
	}
	dst->opcode = BIND_OPCODE_DONE;

	// optimize phase 2, compress packed runs of BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB with
	// same addr delta into one BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB
	dst = &mid[0];
	for (const binding_tmp* src = &mid[0]; src->opcode != BIND_OPCODE_DONE; ++src) {
		uint64_t delta = src->operand1;
		if ( (src->opcode == BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB) 
				&& (src[1].opcode == BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB) 
				&& (src[1].operand1 == delta) ) {
			// found at least two in a row, this is worth compressing
			dst->opcode = BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB;
			dst->operand1 = 1;
			dst->operand2 = delta;
			++src;
			while ( (src->opcode == BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB)
					&& (src->operand1 == delta) ) {
				dst->operand1++;
				++src;
			}
			--src;
			++dst;
		}
		else {
			*dst++ = *src;
		}
	}
	dst->opcode = BIND_OPCODE_DONE;
	
	// optimize phase 3, use immediate encodings
	for (binding_tmp* p = &mid[0]; p->opcode != REBASE_OPCODE_DONE; ++p) {
		if ( (p->opcode == BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB) 
			&& (p->operand1 < (15*sizeof(pint_t)))
			&& ((p->operand1 % sizeof(pint_t)) == 0) ) {
			p->opcode = BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED;
			p->operand1 = p->operand1/sizeof(pint_t);
		}
	}	
	dst->opcode = BIND_OPCODE_DONE;


	// convert to compressed encoding
	const static bool log = false;
	this->_encodedData.reserve(info.size()*2);
	bool done = false;
	for (typename std::vector<binding_tmp>::iterator it = mid.begin(); !done && it != mid.end() ; ++it) {
		switch ( it->opcode ) {
			case BIND_OPCODE_DONE:
				if ( log ) fprintf(stderr, "BIND_OPCODE_DONE()\n");
				this->_encodedData.append_byte(BIND_OPCODE_DONE);
				done = true;
				break;
			case BIND_OPCODE_SET_DYLIB_ORDINAL_IMM:
				if ( log ) fprintf(stderr, "BIND_OPCODE_SET_DYLIB_ORDINAL_IMM(%lld)\n", it->operand1);
				this->_encodedData.append_byte(BIND_OPCODE_SET_DYLIB_ORDINAL_IMM | it->operand1);
				break;
			case BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB:
				if ( log ) fprintf(stderr, "BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB(%lld)\n", it->operand1);
				this->_encodedData.append_byte(BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB);
				this->_encodedData.append_uleb128(it->operand1);
				break;
			case BIND_OPCODE_SET_DYLIB_SPECIAL_IMM:
				if ( log ) fprintf(stderr, "BIND_OPCODE_SET_DYLIB_SPECIAL_IMM(%lld)\n", it->operand1);
				this->_encodedData.append_byte(BIND_OPCODE_SET_DYLIB_SPECIAL_IMM | (it->operand1 & BIND_IMMEDIATE_MASK));
				break;
			case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
				if ( log ) fprintf(stderr, "BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM(0x%0llX, %s)\n", it->operand1, it->name);
				this->_encodedData.append_byte(BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM | it->operand1);
				this->_encodedData.append_string(it->name);
				break;
			case BIND_OPCODE_SET_TYPE_IMM:
				if ( log ) fprintf(stderr, "BIND_OPCODE_SET_TYPE_IMM(%lld)\n", it->operand1);
				this->_encodedData.append_byte(BIND_OPCODE_SET_TYPE_IMM | it->operand1);
				break;
			case BIND_OPCODE_SET_ADDEND_SLEB:
				if ( log ) fprintf(stderr, "BIND_OPCODE_SET_ADDEND_SLEB(%lld)\n", it->operand1);
				this->_encodedData.append_byte(BIND_OPCODE_SET_ADDEND_SLEB);
				this->_encodedData.append_sleb128(it->operand1);
				break;
			case BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
				if ( log ) fprintf(stderr, "BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB(%lld, 0x%llX)\n", it->operand1, it->operand2);
				this->_encodedData.append_byte(BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB | it->operand1);
				this->_encodedData.append_uleb128(it->operand2);
				break;
			case BIND_OPCODE_ADD_ADDR_ULEB:
				if ( log ) fprintf(stderr, "BIND_OPCODE_ADD_ADDR_ULEB(0x%llX)\n", it->operand1);
				this->_encodedData.append_byte(BIND_OPCODE_ADD_ADDR_ULEB);
				this->_encodedData.append_uleb128(it->operand1);
				break;
			case BIND_OPCODE_DO_BIND:
				if ( log ) fprintf(stderr, "BIND_OPCODE_DO_BIND()\n");
				this->_encodedData.append_byte(BIND_OPCODE_DO_BIND);
				break;
			case BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
				if ( log ) fprintf(stderr, "BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB(0x%llX)\n", it->operand1);
				this->_encodedData.append_byte(BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB);
				this->_encodedData.append_uleb128(it->operand1);
				break;
			case BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
				if ( log ) fprintf(stderr, "BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED(%lld=0x%llX)\n", it->operand1, it->operand1*sizeof(pint_t));
				this->_encodedData.append_byte(BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED | it->operand1 );
				break;
			case BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB:
				if ( log ) fprintf(stderr, "BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB(%lld, %lld)\n", it->operand1, it->operand2);
				this->_encodedData.append_byte(BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB);
				this->_encodedData.append_uleb128(it->operand1);
				this->_encodedData.append_uleb128(it->operand2);
				break;
		}
	}
	
	// align to pointer size
	this->_encodedData.pad_to_size(sizeof(pint_t));

	this->_encoded = true;

	if (log) fprintf(stderr, "total weak binding info size = %ld\n", this->_encodedData.size());
	
}



template <typename A>
class LazyBindingInfoAtom : public LinkEditAtom
{
public:
												LazyBindingInfoAtom(const Options& opts, ld::Internal& state, OutputFile& writer)
													: LinkEditAtom(opts, state, writer, _s_section, sizeof(pint_t)) {_encoded = true;  }

	// overrides of ld::Atom
	virtual const char*							name() const		{ return "lazy binding info"; }
	// overrides of LinkEditAtom
	virtual void								encode() const;

private:
	typedef typename A::P						P;
	typedef typename A::P::E					E;
	typedef typename A::P::uint_t				pint_t;
	
	static ld::Section			_s_section;
};

template <typename A>
ld::Section LazyBindingInfoAtom<A>::_s_section("__LINKEDIT", "__lazy_binding", ld::Section::typeLinkEdit, true);



template <typename A>
void LazyBindingInfoAtom<A>::encode() const
{
	// stream all lazy bindings and record start offsets
	std::vector<OutputFile::BindingInfo>& info = this->_writer._lazyBindingInfo;
	for (std::vector<OutputFile::BindingInfo>::const_iterator it = info.begin(); it != info.end(); ++it) {
		// record start offset for use by stub helper
		this->_writer.setLazyBindingInfoOffset(it->_address, this->_encodedData.size());

		// write address to bind
		uint64_t segStart = 0;
		uint64_t segEnd = 0;
		uint32_t segIndex = 0;	
		if ( ! this->_writer.findSegment(this->_state, it->_address, &segStart, &segEnd, &segIndex) )
			throw "lazy binding address outside range of any segment";
		this->_encodedData.append_byte(BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB | segIndex);
		this->_encodedData.append_uleb128(it->_address - segStart);
		
		// write ordinal
		if ( it->_libraryOrdinal <= 0 ) {
			// special lookups are encoded as negative numbers in BindingInfo
			this->_encodedData.append_byte(BIND_OPCODE_SET_DYLIB_SPECIAL_IMM | (it->_libraryOrdinal & BIND_IMMEDIATE_MASK) );
		}
		else if ( it->_libraryOrdinal <= 15 ) {
			// small ordinals are encoded in opcode
			this->_encodedData.append_byte(BIND_OPCODE_SET_DYLIB_ORDINAL_IMM | it->_libraryOrdinal);
		}
		else {
			this->_encodedData.append_byte(BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB);
			this->_encodedData.append_uleb128(it->_libraryOrdinal);
		}
		// write symbol name
		this->_encodedData.append_byte(BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM | it->_flags);
		this->_encodedData.append_string(it->_symbolName);
		// write do bind
		this->_encodedData.append_byte(BIND_OPCODE_DO_BIND);
		this->_encodedData.append_byte(BIND_OPCODE_DONE);
	}
	
	// align to pointer size
	this->_encodedData.pad_to_size(sizeof(pint_t));
	
	this->_encoded = true;
	//fprintf(stderr, "lazy binding info size = %ld, for %ld entries\n", _encodedData.size(), allLazys.size());
}




template <typename A>
class ChainedInfoAtom : public LinkEditAtom
{
public:
												ChainedInfoAtom(const Options& opts, ld::Internal& state, OutputFile& writer)
													: LinkEditAtom(opts, state, writer, _s_section, sizeof(pint_t)) { _encoded = true; }

	// overrides of ld::Atom
	virtual const char*							name() const		{ return "fixup chain info"; }
	// overrides of LinkEditAtom
	virtual void								encode() const;

private:
	typedef typename A::P						P;
	typedef typename A::P::E					E;
	typedef typename A::P::uint_t				pint_t;


	static ld::Section			_s_section;
};

template <typename A>
ld::Section ChainedInfoAtom<A>::_s_section("__LINKEDIT", "__chainfixups", ld::Section::typeLinkEdit, true);



template <typename A>
void ChainedInfoAtom<A>::encode() const
{
	this->_encodedData.bytes().reserve(1024);

	uint16_t format = DYLD_CHAINED_IMPORT;
	if ( _writer._chainedFixupBinds.hasHugeSymbolStrings() )
		format = DYLD_CHAINED_IMPORT_ADDEND64;
	else if ( _writer._chainedFixupBinds.hasHugeAddends() )
		format = DYLD_CHAINED_IMPORT_ADDEND64;
	else if ( _writer._chainedFixupBinds.hasLargeAddends() )
		format = DYLD_CHAINED_IMPORT_ADDEND;
	dyld_chained_fixups_header header;
	header.fixups_version = 0;
	header.starts_offset  = (sizeof(dyld_chained_fixups_header)+7 & -8); // 8-byte align
	header.imports_offset = 0;	// fixed up later
	header.symbols_offset = 0;	// fixed up later
	header.imports_count  = _writer._chainedFixupBinds.count();
	header.imports_format = format;
	header.symbols_format = 0;
	this->_encodedData.append_mem(&header, sizeof(dyld_chained_fixups_header));
	this->_encodedData.pad_to_size(8);
	const unsigned segsHeaderOffset = this->_encodedData.size();

	// write starts table
	dyld_chained_starts_in_image segs;
	segs.seg_count 	        = _writer._chainedFixupSegments.size();
	segs.seg_info_offset[0] = 0;
	this->_encodedData.append_mem(&segs, sizeof(dyld_chained_starts_in_image));
	uint32_t emptyOffset = 0;
	for (unsigned i=1; i < _writer._chainedFixupSegments.size(); ++i)
		this->_encodedData.append_mem(&emptyOffset, sizeof(uint32_t)); // fixed up later if segment used
	unsigned segIndex         = 0;
	uint64_t textStartAddress = 0;
	uint64_t maxRebaseAddress = 0;
	for (OutputFile::ChainedFixupSegInfo& segInfo : _writer._chainedFixupSegments) {
		if ( strcmp(segInfo.name, "__TEXT") == 0 ) {
			textStartAddress = segInfo.startAddr;
		}
		else if ( strcmp(segInfo.name, "__LINKEDIT") == 0 ) {
			uint64_t baseAddress = textStartAddress;
			if ( (segInfo.pointerFormat == DYLD_CHAINED_PTR_32) && (baseAddress == 0x4000) )
				baseAddress = 0; // 32-bit main executables have rebase targets that are zero based
			maxRebaseAddress = (segInfo.startAddr - baseAddress + 0x00100000-1) & -0x00100000; // align to 1MB
		}
	}
	_writer._chainedFixupBinds.setMaxRebase(maxRebaseAddress);
	for (OutputFile::ChainedFixupSegInfo& segInfo : _writer._chainedFixupSegments) {
		if ( !segInfo.pages.empty() ) {
			uint32_t startBytesPerPage = sizeof(uint16_t);
			if ( segInfo.pointerFormat == DYLD_CHAINED_PTR_32 ) {
				if ( _options.sharedRegionEligible() )
					startBytesPerPage = 256; // can't reuse non-pointers for dylibs in cache, so alloc worst case space
				else
					startBytesPerPage = 40 ; // guesstimate 32-bit chains go ~0.5K before needing a new start
			}
			dyld_chained_starts_in_segment aSeg;
			aSeg.size 			   = offsetof(dyld_chained_starts_in_segment, page_start) + segInfo.pages.size()*startBytesPerPage;
			aSeg.page_size 		   = segInfo.pageSize;
			aSeg.pointer_format    = segInfo.pointerFormat;
			aSeg.segment_offset    = segInfo.startAddr - textStartAddress;
			aSeg.max_valid_pointer = (segInfo.pointerFormat == DYLD_CHAINED_PTR_32) ? maxRebaseAddress : 0;
			aSeg.page_count 	   = segInfo.pages.size();
			// pad so that dyld_chained_starts_in_segment will be 64-bit aligned
			this->_encodedData.pad_to_size(8);
			dyld_chained_starts_in_image* segHeader = (dyld_chained_starts_in_image*)(this->_encodedData.start()+segsHeaderOffset);
			segHeader->seg_info_offset[segIndex] = this->_encodedData.size() - segsHeaderOffset;
			this->_encodedData.append_mem(&aSeg, offsetof(dyld_chained_starts_in_segment, page_start) );
			std::vector<uint16_t> 	segChainOverflows;
			for (OutputFile::ChainedFixupPageInfo& pageInfo : segInfo.pages) {
				uint16_t startOffset = pageInfo.fixupOffsets.empty() ? DYLD_CHAINED_PTR_START_NONE : pageInfo.fixupOffsets.front();
				this->_encodedData.append_mem(&startOffset, sizeof(startOffset));
			}
			if ( segInfo.pointerFormat == DYLD_CHAINED_PTR_32 ) {
				// zero out chain overflow area
				long padBytes = (startBytesPerPage-2) * segInfo.pages.size();
				for (long i=0; i < padBytes; ++i)
					this->_encodedData.append_byte(0);
			}
		}
		++segIndex;
	}

	// build imports and symbol table
	__block std::vector<dyld_chained_import>  		  imports;
	__block std::vector<dyld_chained_import_addend>   importsAddend;
	__block std::vector<dyld_chained_import_addend64> importsAddend64;
	__block std::vector<char>                		  stringPool;
	stringPool.push_back('\0');
	_writer._chainedFixupBinds.forEachBind(^(unsigned int bindOrdinal, const ld::Atom* importAtom, uint64_t addend) {
		uint32_t libOrdinal;
		uint64_t tempAddend = addend;
		bool isBind = _writer.needsBind(importAtom, false, &tempAddend, nullptr, nullptr, &libOrdinal);
		assert(isBind);
		const char* symName    = importAtom->name();
		bool 		weakImport = importAtom->weakImported();
		if ( const ld::dylib::File* fromDylib = (ld::dylib::File*)importAtom->file() ) {
			// if command line forced symbols from this dylib to be weak
			if ( fromDylib->forcedWeakLinked() )
				weakImport = true;
		}
		uint32_t nameOffset = stringPool.size();
		if ( header.imports_format == DYLD_CHAINED_IMPORT ) {
			dyld_chained_import   anImport;
			anImport.lib_ordinal = libOrdinal;
			anImport.weak_import = weakImport;
			anImport.name_offset = nameOffset;
			assert(anImport.name_offset == nameOffset);
			imports.push_back(anImport);
		}
		else if ( header.imports_format == DYLD_CHAINED_IMPORT_ADDEND ) {
			dyld_chained_import_addend  anImportA;
			anImportA.lib_ordinal = libOrdinal;
			anImportA.weak_import = weakImport;
			anImportA.name_offset = nameOffset;
			anImportA.addend      = addend;
			assert((uint64_t)anImportA.addend == addend);
			assert(anImportA.name_offset == nameOffset);
			importsAddend.push_back(anImportA);
		}
		else {
			dyld_chained_import_addend64  anImportA64;
			anImportA64.lib_ordinal = libOrdinal;
			anImportA64.weak_import = weakImport;
			anImportA64.name_offset = nameOffset;
			anImportA64.addend      = addend;
			importsAddend64.push_back(anImportA64);
		}
		stringPool.insert(stringPool.end(), symName, &symName[strlen(symName)+1]);
	});

	// write imports and symbol table
	dyld_chained_fixups_header* chainHeader = (dyld_chained_fixups_header*)(this->_encodedData.start());
	switch ( header.imports_format ) {
		case DYLD_CHAINED_IMPORT:
			this->_encodedData.pad_to_size(4);
			chainHeader = (dyld_chained_fixups_header*)(this->_encodedData.start());
			chainHeader->imports_offset = this->_encodedData.size();
			this->_encodedData.append_mem(&imports[0], sizeof(dyld_chained_import)*imports.size());
			break;
		case DYLD_CHAINED_IMPORT_ADDEND:
			this->_encodedData.pad_to_size(4);
			chainHeader = (dyld_chained_fixups_header*)(this->_encodedData.start());
			chainHeader->imports_offset = this->_encodedData.size();
			this->_encodedData.append_mem(&importsAddend[0], sizeof(dyld_chained_import_addend)*importsAddend.size());
			break;
		case DYLD_CHAINED_IMPORT_ADDEND64:
			this->_encodedData.pad_to_size(8);
			chainHeader = (dyld_chained_fixups_header*)(this->_encodedData.start());
			chainHeader->imports_offset = this->_encodedData.size();
			this->_encodedData.append_mem(&importsAddend64[0], sizeof(dyld_chained_import_addend64)*importsAddend64.size());
			break;
	}
	chainHeader = (dyld_chained_fixups_header*)(this->_encodedData.start());
	chainHeader->symbols_offset = this->_encodedData.size();
	this->_encodedData.append_mem(&stringPool[0], stringPool.size());

	// align to pointer size
	this->_encodedData.pad_to_size(sizeof(pint_t));

	this->_encoded = true;
}


template <typename A>
class ExportInfoAtom : public LinkEditAtom
{
public:
												ExportInfoAtom(const Options& opts, ld::Internal& state, OutputFile& writer)
													: LinkEditAtom(opts, state, writer, _s_section, sizeof(pint_t)) { _encoded = true; }

	// overrides of ld::Atom
	virtual const char*							name() const		{ return "export info"; }
	// overrides of LinkEditAtom
	virtual void								encode() const;

private:
	typedef typename A::P						P;
	typedef typename A::P::E					E;
	typedef typename A::P::uint_t				pint_t;

	const ld::Atom*								stubForResolverFunction(const ld::Atom* resolver) const;

	struct TrieEntriesSorter
	{
		TrieEntriesSorter(const Options& o) : _options(o) {}
		
		 bool operator()(const mach_o::trie::Entry& left, const mach_o::trie::Entry& right)
		 {
			unsigned int leftOrder;
			unsigned int rightOrder;
			_options.exportedSymbolOrder(left.name, &leftOrder);
			_options.exportedSymbolOrder(right.name, &rightOrder);
			if ( leftOrder != rightOrder ) 
				return (leftOrder < rightOrder);
			else
				return (left.address < right.address);
		 }
	private:
		const Options&	_options;
	};
	
	static ld::Section			_s_section;
};

template <typename A>
ld::Section ExportInfoAtom<A>::_s_section("__LINKEDIT", "__export", ld::Section::typeLinkEdit, true);

template <typename A>
const ld::Atom* ExportInfoAtom<A>::stubForResolverFunction(const ld::Atom* resolver) const
{
	for (std::vector<ld::Internal::FinalSection*>::iterator sit = _state.sections.begin(); sit != _state.sections.end(); ++sit) {
		ld::Internal::FinalSection* sect = *sit;
		if ( (sect->type() == ld::Section::typeStub) || (sect->type() == ld::Section::typeStubClose) ) {
			for (std::vector<const ld::Atom*>::iterator ait = sect->atoms.begin(); ait != sect->atoms.end(); ++ait) {
				const ld::Atom* atom = *ait;
				if ( strcmp(atom->name(), resolver->name()) == 0 )
					return atom;
			}
		}
	}
	assert(0 && "no stub for resolver function");
	return NULL;
}


template <typename A>
void ExportInfoAtom<A>::encode() const
{
	// make vector of mach_o::trie::Entry for all exported symbols
	std::vector<const ld::Atom*>& exports = this->_writer._exportedAtoms;
	uint64_t imageBaseAddress = this->_writer.headerAndLoadCommandsSection->address;
	std::vector<mach_o::trie::Entry> entries;
	unsigned int padding = 0;
	entries.reserve(exports.size());
	for (std::vector<const ld::Atom*>::const_iterator it = exports.begin(); it != exports.end(); ++it) {
		const ld::Atom* atom = *it;
		mach_o::trie::Entry entry;
		uint64_t flags = (atom->contentType() == ld::Atom::typeTLV) ? EXPORT_SYMBOL_FLAGS_KIND_THREAD_LOCAL : EXPORT_SYMBOL_FLAGS_KIND_REGULAR;
		uint64_t other = 0;
		uint64_t address = atom->finalAddress() - imageBaseAddress;
		if ( atom->definition() == ld::Atom::definitionProxy ) {
			entry.name = atom->name();
			entry.flags = flags | EXPORT_SYMBOL_FLAGS_REEXPORT;
			if ( atom->combine() == ld::Atom::combineByName )
				entry.flags |= EXPORT_SYMBOL_FLAGS_WEAK_DEFINITION;
			entry.other = this->_writer.compressedOrdinalForAtom(atom);
			if ( entry.other == BIND_SPECIAL_DYLIB_SELF ) {
				warning("not adding explict export for symbol %s because it is already re-exported from dylib %s", entry.name, atom->safeFilePath());
				continue;
			}
			if ( atom->isAlias() ) {
				// alias proxy means symbol was re-exported with a name change
				const ld::Atom* aliasOf = NULL;
				for (ld::Fixup::iterator fit = atom->fixupsBegin(); fit != atom->fixupsEnd(); ++fit) {
					if ( fit->kind == ld::Fixup::kindNoneFollowOn ) {
						assert(fit->binding == ld::Fixup::bindingDirectlyBound);
						aliasOf = fit->u.target;
					}
				}
				assert(aliasOf != NULL);
				entry.importName = aliasOf->name();
			}
			else {
				// symbol name stays same as re-export
				entry.importName = atom->name();
			}
			entries.push_back(entry);
			//fprintf(stderr, "re-export %s from lib %llu as %s\n", entry.importName, entry.other, entry.name);
		}
		else if ( atom->definition() == ld::Atom::definitionAbsolute ) {
			entry.name = atom->name();
			entry.flags = _options.canUseAbsoluteSymbols() ? EXPORT_SYMBOL_FLAGS_KIND_ABSOLUTE : EXPORT_SYMBOL_FLAGS_KIND_REGULAR;
			entry.address = address;
			entry.other = other;
			entry.importName = NULL;
			entries.push_back(entry);
		}
		else {
			if ( (atom->definition() == ld::Atom::definitionRegular) && (atom->combine() == ld::Atom::combineByName) )
				flags |= EXPORT_SYMBOL_FLAGS_WEAK_DEFINITION;
			if ( atom->isThumb() )
				address |= 1;
			if ( atom->contentType() == ld::Atom::typeResolver ) {
				flags |= EXPORT_SYMBOL_FLAGS_STUB_AND_RESOLVER;
				// set normal lookup to return stub address
				// and add resolver function in new location that newer dyld's can access
				other = address;
				const ld::Atom* stub = stubForResolverFunction(atom);
				address = stub->finalAddress() - imageBaseAddress;
				if ( stub->isThumb() )
					address |= 1;
			}
			entry.name = atom->name();
			entry.flags = flags;
			entry.address = address; 
			entry.other = other; 
			entry.importName = NULL;
			entries.push_back(entry);
		}

		if (_options.sharedRegionEligible() && strncmp(atom->section().segmentName(), "__DATA", 6) == 0) {
			// Maximum address is 64bit which is 10 bytes as a uleb128. Minimum is 1 byte
			// Pad the section out so we can deal with addresses getting larger when __DATA segment
			// is moved before __TEXT in dyld shared cache.
			padding += 9;
		}
	}

	// sort vector by -exported_symbols_order, and any others by address
	std::sort(entries.begin(), entries.end(), TrieEntriesSorter(_options));
	
	// create trie
	mach_o::trie::makeTrie(entries, this->_encodedData.bytes());

	//Add additional data padding for the unoptimized shared cache
	for (unsigned int i = 0; i < padding; ++i)
		this->_encodedData.append_byte(0);

	// align to pointer size
	this->_encodedData.pad_to_size(sizeof(pint_t));
	
	this->_encoded = true;
}


template <typename A>
class SplitSegInfoV1Atom : public LinkEditAtom
{
public:
												SplitSegInfoV1Atom(const Options& opts, ld::Internal& state, OutputFile& writer)
													: LinkEditAtom(opts, state, writer, _s_section, sizeof(pint_t)) { }

	// overrides of ld::Atom
	virtual const char*							name() const		{ return "split seg info"; }
	// overrides of LinkEditAtom
	virtual void								encode() const;

private:
	typedef typename A::P						P;
	typedef typename A::P::E					E;
	typedef typename A::P::uint_t				pint_t;

	void										addSplitSegInfo(uint64_t address, ld::Fixup::Kind k, uint32_t) const;
	void										uleb128EncodeAddresses(const std::vector<uint64_t>& locations) const;

	mutable std::vector<uint64_t>				_32bitPointerLocations;
	mutable std::vector<uint64_t>				_64bitPointerLocations;
	mutable std::vector<uint64_t>				_thumbLo16Locations;
	mutable std::vector<uint64_t>				_thumbHi16Locations[16];
	mutable std::vector<uint64_t>				_armLo16Locations;
	mutable std::vector<uint64_t>				_armHi16Locations[16];
	mutable std::vector<uint64_t>				_adrpLocations;


	static ld::Section			_s_section;
};

template <typename A>
ld::Section SplitSegInfoV1Atom<A>::_s_section("__LINKEDIT", "__splitSegInfo", ld::Section::typeLinkEdit, true);

template <>
void SplitSegInfoV1Atom<x86_64>::addSplitSegInfo(uint64_t address, ld::Fixup::Kind kind, uint32_t extra) const
{
	switch (kind) {
		case ld::Fixup::kindStoreX86PCRel32:
		case ld::Fixup::kindStoreX86PCRel32_1:
		case ld::Fixup::kindStoreX86PCRel32_2:
		case ld::Fixup::kindStoreX86PCRel32_4:
		case ld::Fixup::kindStoreX86PCRel32GOTLoad:
		case ld::Fixup::kindStoreX86PCRel32GOTLoadNowLEA:
		case ld::Fixup::kindStoreX86PCRel32GOT:
		case ld::Fixup::kindStoreLittleEndian32:
		case ld::Fixup::kindStoreTargetAddressLittleEndian32:
		case ld::Fixup::kindStoreTargetAddressX86PCRel32:
		case ld::Fixup::kindStoreTargetAddressX86PCRel32GOTLoad:
		case ld::Fixup::kindStoreTargetAddressX86PCRel32GOTLoadNowLEA:
		case ld::Fixup::kindStoreTargetAddressX86PCRel32TLVLoad:
		case ld::Fixup::kindStoreTargetAddressX86PCRel32TLVLoadNowLEA:
			_32bitPointerLocations.push_back(address);
			break;
		case ld::Fixup::kindStoreLittleEndian64:
		case ld::Fixup::kindStoreTargetAddressLittleEndian64:
			_64bitPointerLocations.push_back(address);
			break;
#if SUPPORT_ARCH_arm64e
		case ld::Fixup::kindStoreLittleEndianAuth64:
		case ld::Fixup::kindStoreTargetAddressLittleEndianAuth64:
			assert(false);
			break;
#endif
		default:
			warning("codegen at address 0x%08llX prevents image from working in dyld shared cache", address);
			break;
	}
}

template <>
void SplitSegInfoV1Atom<x86>::addSplitSegInfo(uint64_t address, ld::Fixup::Kind kind, uint32_t extra) const
{
	switch (kind) {
		case ld::Fixup::kindStoreLittleEndian32:
		case ld::Fixup::kindStoreTargetAddressLittleEndian32:
		case ld::Fixup::kindStoreX86PCRel32TLVLoad:
		case ld::Fixup::kindStoreX86PCRel32TLVLoadNowLEA:
			_32bitPointerLocations.push_back(address);
			break;
		default:
			warning("codegen at address 0x%08llX prevents image from working in dyld shared cache", address);
			break;
	}
}

template <>
void SplitSegInfoV1Atom<arm>::addSplitSegInfo(uint64_t address, ld::Fixup::Kind kind, uint32_t extra) const
{
	switch (kind) {
		case ld::Fixup::kindStoreLittleEndian32:
			_32bitPointerLocations.push_back(address);
			break;
        case ld::Fixup::kindStoreARMLow16:
 			_armLo16Locations.push_back(address);
			break;
       case ld::Fixup::kindStoreThumbLow16: 
			_thumbLo16Locations.push_back(address);
			break;
        case ld::Fixup::kindStoreARMHigh16: 
            assert(extra < 16);
			_armHi16Locations[extra].push_back(address);
			break;
        case ld::Fixup::kindStoreThumbHigh16: 
            assert(extra < 16);
			_thumbHi16Locations[extra].push_back(address);
			break;
		default:
			warning("codegen at address 0x%08llX prevents image from working in dyld shared cache", address);
			break;
	}
}

#if SUPPORT_ARCH_arm64
template <>
void SplitSegInfoV1Atom<arm64>::addSplitSegInfo(uint64_t address, ld::Fixup::Kind kind, uint32_t extra) const
{
	switch (kind) {
		case ld::Fixup::kindStoreARM64Page21:
		case ld::Fixup::kindStoreARM64GOTLoadPage21:
		case ld::Fixup::kindStoreARM64GOTLeaPage21:
		case ld::Fixup::kindStoreARM64TLVPLoadPage21:
		case ld::Fixup::kindStoreTargetAddressARM64Page21:
		case ld::Fixup::kindStoreTargetAddressARM64GOTLoadPage21:
		case ld::Fixup::kindStoreTargetAddressARM64GOTLeaPage21:
			_adrpLocations.push_back(address);
			break;
		case ld::Fixup::kindStoreLittleEndian32:
		case ld::Fixup::kindStoreARM64PCRelToGOT:
			_32bitPointerLocations.push_back(address);
			break;
 		case ld::Fixup::kindStoreLittleEndian64:
		case ld::Fixup::kindStoreTargetAddressLittleEndian64:
			_64bitPointerLocations.push_back(address);
			break;
#if SUPPORT_ARCH_arm64e
		case ld::Fixup::kindStoreLittleEndianAuth64:
		case ld::Fixup::kindStoreTargetAddressLittleEndianAuth64:
			warning("authenticated pointer at address 0x%08llX prevents image from working in dyld shared cache", address);
			break;
#endif
		default:
			warning("codegen at address 0x%08llX prevents image from working in dyld shared cache", address);
			break;
	}
}
#endif

#if SUPPORT_ARCH_arm64_32
template <>
void SplitSegInfoV1Atom<arm64_32>::addSplitSegInfo(uint64_t address, ld::Fixup::Kind kind, uint32_t extra) const
{
	switch (kind) {
		case ld::Fixup::kindStoreARM64Page21:
		case ld::Fixup::kindStoreARM64GOTLoadPage21:
		case ld::Fixup::kindStoreARM64GOTLeaPage21:
		case ld::Fixup::kindStoreARM64TLVPLoadPage21:
		case ld::Fixup::kindStoreTargetAddressARM64Page21:
		case ld::Fixup::kindStoreTargetAddressARM64GOTLoadPage21:
		case ld::Fixup::kindStoreTargetAddressARM64GOTLeaPage21:
			_adrpLocations.push_back(address);
			break;
		case ld::Fixup::kindStoreLittleEndian32:
		case ld::Fixup::kindStoreARM64PCRelToGOT:
			_32bitPointerLocations.push_back(address);
			break;
 		case ld::Fixup::kindStoreLittleEndian64:
		case ld::Fixup::kindStoreTargetAddressLittleEndian64:
			_64bitPointerLocations.push_back(address);
			break;
		default:
			warning("codegen at address 0x%08llX prevents image from working in dyld shared cache", address);
			break;
	}
}
#endif

template <typename A>
void SplitSegInfoV1Atom<A>::uleb128EncodeAddresses(const std::vector<uint64_t>& locations) const
{
	pint_t addr = this->_options.baseAddress();
	for(typename std::vector<uint64_t>::const_iterator it = locations.begin(); it != locations.end(); ++it) {
		pint_t nextAddr = *it;
		//fprintf(stderr, "nextAddr=0x%0llX\n", (uint64_t)nextAddr);
		uint64_t delta = nextAddr - addr;
		//fprintf(stderr, "delta=0x%0llX\n", delta);
		if ( delta == 0 ) 
			throw "double split seg info for same address";
		// uleb128 encode
		uint8_t byte;
		do {
			byte = delta & 0x7F;
			delta &= ~0x7F;
			if ( delta != 0 )
				byte |= 0x80;
			this->_encodedData.append_byte(byte);
			delta = delta >> 7;
		} 
		while( byte >= 0x80 );
		addr = nextAddr;
	}
}


template <typename A>
void SplitSegInfoV1Atom<A>::encode() const
{
	// sort into group by pointer adjustment kind
	std::vector<OutputFile::SplitSegInfoEntry>& info = this->_writer._splitSegInfos;
	for (std::vector<OutputFile::SplitSegInfoEntry>::const_iterator it = info.begin(); it != info.end(); ++it) {
		this->addSplitSegInfo(it->fixupAddress, it->kind, it->extra);
	}

	// delta compress runs of addresses
	this->_encodedData.reserve(8192);
	if ( _32bitPointerLocations.size() != 0 ) {
		this->_encodedData.append_byte(1);
		//fprintf(stderr, "type 1:\n");
		std::sort(_32bitPointerLocations.begin(), _32bitPointerLocations.end());
		this->uleb128EncodeAddresses(_32bitPointerLocations);
		this->_encodedData.append_byte(0); // terminator
	}
	
	if ( _64bitPointerLocations.size() != 0 ) {
		this->_encodedData.append_byte(2);
		//fprintf(stderr, "type 2:\n");
		std::sort(_64bitPointerLocations.begin(), _64bitPointerLocations.end());
		this->uleb128EncodeAddresses(_64bitPointerLocations);
		this->_encodedData.append_byte(0); // terminator
	}

	if ( _adrpLocations.size() != 0 ) {
		this->_encodedData.append_byte(3);
		//fprintf(stderr, "type 3:\n");
		std::sort(_adrpLocations.begin(), _adrpLocations.end());
		this->uleb128EncodeAddresses(_adrpLocations);
		this->_encodedData.append_byte(0); // terminator
	}

	if ( _thumbLo16Locations.size() != 0 ) {
		this->_encodedData.append_byte(5);
		//fprintf(stderr, "type 5:\n");
		std::sort(_thumbLo16Locations.begin(), _thumbLo16Locations.end());
		this->uleb128EncodeAddresses(_thumbLo16Locations);
		this->_encodedData.append_byte(0); // terminator
	}

	if ( _armLo16Locations.size() != 0 ) {
		this->_encodedData.append_byte(6);
		//fprintf(stderr, "type 6:\n");
		std::sort(_armLo16Locations.begin(), _armLo16Locations.end());
		this->uleb128EncodeAddresses(_armLo16Locations);
		this->_encodedData.append_byte(0); // terminator
	}

	for (uint32_t i=0; i < 16; ++i) {
		if ( _thumbHi16Locations[i].size() != 0 ) {
			this->_encodedData.append_byte(16+i);
			//fprintf(stderr, "type 16+%d:\n", i);
			std::sort(_thumbHi16Locations[i].begin(), _thumbHi16Locations[i].end());
			this->uleb128EncodeAddresses(_thumbHi16Locations[i]);
			this->_encodedData.append_byte(0); // terminator
		}
	}

	for (uint32_t i=0; i < 16; ++i) {
		if ( _armHi16Locations[i].size() != 0 ) {
			this->_encodedData.append_byte(32+i);
			//fprintf(stderr, "type 32+%d:\n", i);
			std::sort(_armHi16Locations[i].begin(), _armHi16Locations[i].end());
			this->uleb128EncodeAddresses(_armHi16Locations[i]);
			this->_encodedData.append_byte(0); // terminator
		}
	}

	// always add zero byte to mark end
	this->_encodedData.append_byte(0);

	// align to pointer size
	this->_encodedData.pad_to_size(sizeof(pint_t));
	
	this->_encoded = true;
	
	// clean up temporaries
	_32bitPointerLocations.clear();
	_64bitPointerLocations.clear();
}


template <typename A>
class SplitSegInfoV2Atom : public LinkEditAtom
{
public:
												SplitSegInfoV2Atom(const Options& opts, ld::Internal& state, OutputFile& writer)
													: LinkEditAtom(opts, state, writer, _s_section, sizeof(pint_t)) { }

	// overrides of ld::Atom
	virtual const char*							name() const		{ return "split seg info"; }
	// overrides of LinkEditAtom
	virtual void								encode() const;

private:
	typedef typename A::P						P;
	typedef typename A::P::E					E;
	typedef typename A::P::uint_t				pint_t;

	// Whole		 :== <count> FromToSection+
	// FromToSection :== <from-sect-index> <to-sect-index> <count> ToOffset+
	// ToOffset		 :== <to-sect-offset-delta> <count> FromOffset+
	// FromOffset	 :== <kind> <count> <from-sect-offset-delta>

	typedef uint32_t SectionIndexes;
	typedef std::map<uint8_t, std::vector<uint64_t> > FromOffsetMap;
	typedef std::map<uint64_t, FromOffsetMap> ToOffsetMap;
	typedef std::map<SectionIndexes, ToOffsetMap> WholeMap;


	static ld::Section			_s_section;
};

template <typename A>
ld::Section SplitSegInfoV2Atom<A>::_s_section("__LINKEDIT", "__splitSegInfo", ld::Section::typeLinkEdit, true);


template <typename A>
void SplitSegInfoV2Atom<A>::encode() const
{
	// sort into group by adjustment kind
	//fprintf(stderr, "_splitSegV2Infos.size=%lu\n", this->_writer._splitSegV2Infos.size());
	WholeMap whole;
	for (const OutputFile::SplitSegInfoV2Entry&  entry : this->_writer._splitSegV2Infos) {
		//fprintf(stderr, "from=%d, to=%d\n", entry.fixupSectionIndex, entry.targetSectionIndex);
		SectionIndexes index = entry.fixupSectionIndex << 16 | entry.targetSectionIndex;
		ToOffsetMap& toOffsets = whole[index];
		FromOffsetMap& fromOffsets = toOffsets[entry.targetSectionOffset];
		fromOffsets[entry.referenceKind].push_back(entry.fixupSectionOffset);
	}

	// Add marker that this is V2 data
	this->_encodedData.reserve(8192);
	this->_encodedData.append_byte(DYLD_CACHE_ADJ_V2_FORMAT); 

	// stream out
	// Whole :== <count> FromToSection+
	this->_encodedData.append_uleb128(whole.size());
	for (auto& fromToSection : whole) {
		uint8_t fromSectionIndex = fromToSection.first >> 16;
		uint8_t toSectionIndex   = fromToSection.first & 0xFFFF;
		ToOffsetMap& toOffsets   = fromToSection.second;
		// FromToSection :== <from-sect-index> <to-sect-index> <count> ToOffset+
		this->_encodedData.append_uleb128(fromSectionIndex);
		this->_encodedData.append_uleb128(toSectionIndex);
		this->_encodedData.append_uleb128(toOffsets.size());
		//fprintf(stderr, "from sect=%d, to sect=%d, count=%lu\n", fromSectionIndex, toSectionIndex, toOffsets.size());
		uint64_t lastToOffset = 0;
		for (auto& fromToOffsets : toOffsets) {
			uint64_t toSectionOffset = fromToOffsets.first;
			FromOffsetMap& fromOffsets = fromToOffsets.second;
			// ToOffset	:== <to-sect-offset-delta> <count> FromOffset+
			this->_encodedData.append_uleb128(toSectionOffset - lastToOffset);
			this->_encodedData.append_uleb128(fromOffsets.size());
			for (auto& kindAndOffsets : fromOffsets) {
				uint8_t kind = kindAndOffsets.first;
				std::vector<uint64_t>& fromOffsets = kindAndOffsets.second;
				// FromOffset :== <kind> <count> <from-sect-offset-delta>
				this->_encodedData.append_uleb128(kind);
				this->_encodedData.append_uleb128(fromOffsets.size());
				std::sort(fromOffsets.begin(), fromOffsets.end());
				uint64_t lastFromOffset = 0;
				for (uint64_t offset : fromOffsets) {
					this->_encodedData.append_uleb128(offset - lastFromOffset);
					lastFromOffset = offset;
				}
			}
			lastToOffset = toSectionOffset;
		}
	}


	// always add zero byte to mark end
	this->_encodedData.append_byte(0);

	// align to pointer size
	this->_encodedData.pad_to_size(sizeof(pint_t));
	
	this->_encoded = true;
}



template <typename A>
class FunctionStartsAtom : public LinkEditAtom
{
public:
												FunctionStartsAtom(const Options& opts, ld::Internal& state, OutputFile& writer)
													: LinkEditAtom(opts, state, writer, _s_section, sizeof(pint_t)) { }

	// overrides of ld::Atom
	virtual const char*							name() const		{ return "function starts"; }
	// overrides of LinkEditAtom
	virtual void								encode() const;

private:
	typedef typename A::P						P;
	typedef typename A::P::E					E;
	typedef typename A::P::uint_t				pint_t;

	static ld::Section			_s_section;
};

template <typename A>
ld::Section FunctionStartsAtom<A>::_s_section("__LINKEDIT", "__funcStarts", ld::Section::typeLinkEdit, true);


template <typename A>
void FunctionStartsAtom<A>::encode() const
{
	this->_encodedData.reserve(8192);
	const uint64_t badAddress = 0xFFFFFFFFFFFFFFFF;
	uint64_t addr = badAddress;
	// delta compress all function addresses
	for (std::vector<ld::Internal::FinalSection*>::iterator it = this->_state.sections.begin(); it != this->_state.sections.end(); ++it) {
		ld::Internal::FinalSection* sect = *it;
		if ( sect->type() == ld::Section::typeMachHeader ) {
			// start with delta from start of __TEXT
			addr = sect->address;
		}
		else if ( sect->type() == ld::Section::typeCode ) {
			assert(addr != badAddress);
			std::vector<const ld::Atom*>& atoms = sect->atoms;
			for (std::vector<const ld::Atom*>::iterator ait = atoms.begin(); ait != atoms.end(); ++ait) {
				const ld::Atom* atom = *ait;
				// <rdar://problem/10422823> filter out zero-length atoms, so LC_FUNCTION_STARTS address can't spill into next section
				if ( atom->size() == 0 )
					continue;
				uint64_t nextAddr = atom->finalAddress();
				if ( atom->isThumb() )
					nextAddr |= 1; 
				uint64_t delta = nextAddr - addr;
				if ( delta != 0 )
					this->_encodedData.append_uleb128(delta);
				addr = nextAddr;
			}
		}
	}
	
	// terminator
	this->_encodedData.append_byte(0); 
	
	// align to pointer size
	this->_encodedData.pad_to_size(sizeof(pint_t));		

	this->_encoded = true;
}


// <rdar://problem/9218847> Need way to formalize data in code
template <typename A>
class DataInCodeAtom : public LinkEditAtom
{
public:
												DataInCodeAtom(const Options& opts, ld::Internal& state, OutputFile& writer)
													: LinkEditAtom(opts, state, writer, _s_section, sizeof(pint_t)) { }

	// overrides of ld::Atom
	virtual const char*							name() const		{ return "data-in-code info"; }
	// overrides of LinkEditAtom
	virtual void								encode() const;

private:
	typedef typename A::P						P;
	typedef typename A::P::E					E;
	typedef typename A::P::uint_t				pint_t;

	struct FixupByAddressSorter
	{
		 bool operator()(const ld::Fixup* left, const ld::Fixup* right)
		 {
			if (left->offsetInAtom != right->offsetInAtom)
				return (left->offsetInAtom < right->offsetInAtom);
			return (left->kind > right->kind);
		 }
	};

	void encodeEntry(uint32_t startImageOffset, int len, ld::Fixup::Kind kind) const {
		//fprintf(stderr, "encodeEntry(start=0x%08X, len=0x%04X, kind=%04X\n", startImageOffset, len, kind);
		do {
			macho_data_in_code_entry<P> entry;
			entry.set_offset(startImageOffset);
			entry.set_length(len);
			switch ( kind ) {
				case ld::Fixup::kindDataInCodeStartData:
					entry.set_kind(DICE_KIND_DATA);
					break;
				case ld::Fixup::kindDataInCodeStartJT8:
					entry.set_kind(DICE_KIND_JUMP_TABLE8);
					break;
				case ld::Fixup::kindDataInCodeStartJT16:
					entry.set_kind(DICE_KIND_JUMP_TABLE16);
					break;
				case ld::Fixup::kindDataInCodeStartJT32:
					entry.set_kind(DICE_KIND_JUMP_TABLE32);
					break;
				case ld::Fixup::kindDataInCodeStartJTA32:
					entry.set_kind(DICE_KIND_ABS_JUMP_TABLE32);
					break;
				default:
					assert(0 && "bad L$start$ label to encode");
			}
			uint8_t* bp = (uint8_t*)&entry;
			this->_encodedData.append_byte(bp[0]);
			this->_encodedData.append_byte(bp[1]);
			this->_encodedData.append_byte(bp[2]);
			this->_encodedData.append_byte(bp[3]);
			this->_encodedData.append_byte(bp[4]);
			this->_encodedData.append_byte(bp[5]);
			this->_encodedData.append_byte(bp[6]);
			this->_encodedData.append_byte(bp[7]);
			// in rare case data range is huge, create multiple entries
			len -= 0xFFF8;
			startImageOffset += 0xFFF8;
		} while ( len > 0 );
	}

	static ld::Section			_s_section;
};

template <typename A>
ld::Section DataInCodeAtom<A>::_s_section("__LINKEDIT", "__dataInCode", ld::Section::typeLinkEdit, true);


template <typename A>
void DataInCodeAtom<A>::encode() const
{
	if ( this->_writer.hasDataInCode ) {
		uint64_t mhAddress = 0;
		for (std::vector<ld::Internal::FinalSection*>::iterator sit = _state.sections.begin(); sit != _state.sections.end(); ++sit) {
			ld::Internal::FinalSection* sect = *sit;
			if ( sect->type() == ld::Section::typeMachHeader )
				mhAddress = sect->address;
			if ( sect->type() != ld::Section::typeCode )
				continue;
			for (std::vector<const ld::Atom*>::iterator ait = sect->atoms.begin(); ait != sect->atoms.end(); ++ait) {
				const ld::Atom*	atom = *ait;
				// gather all code-in-data labels
				std::vector<const ld::Fixup*> dataInCodeLabels;
				for (ld::Fixup::iterator fit = atom->fixupsBegin(); fit != atom->fixupsEnd(); ++fit) {
					switch ( fit->kind ) {
						case ld::Fixup::kindDataInCodeStartData:
						case ld::Fixup::kindDataInCodeStartJT8:
						case ld::Fixup::kindDataInCodeStartJT16:
						case ld::Fixup::kindDataInCodeStartJT32:
						case ld::Fixup::kindDataInCodeStartJTA32:
						case ld::Fixup::kindDataInCodeEnd:
							dataInCodeLabels.push_back(fit);
							break;
						default:
							break;
					}
				}
				// to do: sort labels by address
				std::sort(dataInCodeLabels.begin(), dataInCodeLabels.end(), FixupByAddressSorter());
				
				// convert to array of struct data_in_code_entry
				ld::Fixup::Kind prevKind = ld::Fixup::kindDataInCodeEnd;
				uint32_t prevOffset = 0;
				for ( std::vector<const ld::Fixup*>::iterator sfit = dataInCodeLabels.begin(); sfit != dataInCodeLabels.end(); ++sfit) {
					if ( ((*sfit)->kind != prevKind) && (prevKind != ld::Fixup::kindDataInCodeEnd) ) {
						int len = (*sfit)->offsetInAtom - prevOffset;
						if ( len == 0 )
							warning("overlapping data-in-code in '%s' at offset 0x%04X", atom->name(), prevOffset);
						this->encodeEntry(atom->finalAddress()+prevOffset-mhAddress, (*sfit)->offsetInAtom - prevOffset, prevKind);
					}
					prevKind = (*sfit)->kind;
					prevOffset = (*sfit)->offsetInAtom;
				}
				if ( prevKind != ld::Fixup::kindDataInCodeEnd ) {
					// add entry if function ends with data
					this->encodeEntry(atom->finalAddress()+prevOffset-mhAddress, atom->size() - prevOffset, prevKind);
				}
			}
		}
	}
	
	this->_encoded = true;
}






template <typename A>
class OptimizationHintsAtom : public LinkEditAtom
{
public:
												OptimizationHintsAtom(const Options& opts, ld::Internal& state, OutputFile& writer)
													: LinkEditAtom(opts, state, writer, _s_section, sizeof(pint_t)) { 
														assert(opts.outputKind() == Options::kObjectFile);
													}

	// overrides of ld::Atom
	virtual const char*							name() const		{ return "linker optimization hints"; }
	// overrides of LinkEditAtom
	virtual void								encode() const;

private:
	typedef typename A::P						P;
	typedef typename A::P::E					E;
	typedef typename A::P::uint_t				pint_t;

	static ld::Section			_s_section;

};

template <typename A>
ld::Section OptimizationHintsAtom<A>::_s_section("__LINKEDIT", "__opt_hints", ld::Section::typeLinkEdit, true);

template <typename A>
void OptimizationHintsAtom<A>::encode() const
{
	if ( _state.someObjectHasOptimizationHints ) {
		for (std::vector<ld::Internal::FinalSection*>::iterator sit = _state.sections.begin(); sit != _state.sections.end(); ++sit) {
			ld::Internal::FinalSection* sect = *sit;
			if ( sect->type() != ld::Section::typeCode )
				continue;
			for (std::vector<const ld::Atom*>::iterator ait = sect->atoms.begin(); ait != sect->atoms.end(); ++ait) {
				const ld::Atom*	atom = *ait;
				uint64_t address = atom->finalAddress();
				for (ld::Fixup::iterator fit = atom->fixupsBegin(); fit != atom->fixupsEnd(); ++fit) {
					if ( fit->kind != ld::Fixup::kindLinkerOptimizationHint) 
						continue;
					ld::Fixup::LOH_arm64 extra;
					extra.addend = fit->u.addend;
					_encodedData.append_uleb128(extra.info.kind);
					_encodedData.append_uleb128(extra.info.count+1);
					_encodedData.append_uleb128((extra.info.delta1 << 2) + fit->offsetInAtom + address);
					if ( extra.info.count > 0 )
						_encodedData.append_uleb128((extra.info.delta2 << 2) + fit->offsetInAtom + address);
					if ( extra.info.count > 1 )
						_encodedData.append_uleb128((extra.info.delta3 << 2) + fit->offsetInAtom + address);
					if ( extra.info.count > 2 )
						_encodedData.append_uleb128((extra.info.delta4 << 2) + fit->offsetInAtom + address);
				}
			}
		}
			
		this->_encodedData.pad_to_size(sizeof(pint_t));
	}
	
	this->_encoded = true;
}

class CodeSignatureAtom : public LinkEditAtom
{
public:
												CodeSignatureAtom(const Options& opts, ld::Internal& state, OutputFile& writer)
													: LinkEditAtom(opts, state, writer, _s_section, 16), _opts(opts) {
														assert(opts.outputKind() != Options::kObjectFile);
														this->_encoded = true;  // only works because this is last
													}

	// overrides of ld::Atom
	virtual const char*							name() const		{ return "code signature"; }
	// overrides of LinkEditAtom
	virtual void								encode() const;

			void								hash(uint8_t* wholeFileBuffer) const;

private:
	const Options& 				_opts;
	mutable libcd*				_sigRef = nullptr;

	static ld::Section			_s_section;

};

ld::Section CodeSignatureAtom::_s_section("__LINKEDIT", "__code_sign", ld::Section::typeLinkEdit, true);

void CodeSignatureAtom::encode() const
{
	// calculate pages in binary to know how big codesignature needs to be
	Internal::FinalSection* codeSignSect = _state.sections.back();
	assert(codeSignSect->atoms[0] == this);
	const size_t inBbufferSize = codeSignSect->fileOffset;

	// create code signing object
	_sigRef = libcd_create(inBbufferSize);

	// figure out which hashes to use
	__block ld::Platform sig_platform = ld::Platform::unknown;
	__block uint32_t     sig_min_version;
	_opts.platforms().forEach(^(ld::Platform platform, uint32_t minVersion, uint32_t sdkVersion, bool& stop) {
		switch ( platform ) {
		case Platform::unknown:
		case Platform::freestanding:
		case Platform::bridgeOS:
			break;
		case ld::Platform::macOS:
		case ld::Platform::iOS:
		case ld::Platform::tvOS:
		case ld::Platform::iOS_simulator:
		case ld::Platform::tvOS_simulator:
		case ld::Platform::watchOS_simulator:
		case ld::Platform::driverKit:
		case ld::Platform::watchOS:
			sig_platform = platform;
			sig_min_version = minVersion;
			break;
		case ld::Platform::iOSMac:
			// if this is zippered, don't overwrite macOS info
			if ( sig_platform == ld::Platform::unknown ) {
				sig_platform = platform;
				sig_min_version = minVersion;
			}
			break;
		}
	});
	if ( libcd_set_hash_types_for_platform_version(_sigRef, (int)sig_platform, (int)sig_min_version) != LIBCD_SET_HASH_TYPE_SUCCESS )
		throw "can't determine codesign hash for output platform";

	// set identifier
	const char* lastSlash = strrchr(_opts.outputFilePath(), '/');
	const char* leafName = (lastSlash != nullptr) ? lastSlash+1 : _opts.outputFilePath();
	libcd_set_signing_id(_sigRef, leafName);

	// add flags
	libcd_set_flags(_sigRef, CS_ADHOC | CS_LINKER_SIGNED);

	// update section size now that code signature size is known
	codeSignSect->size = libcd_superblob_size(_sigRef);

	// allocate space for code-signature (never used, sign is written directly to output buffer in hash())
	this->_encodedData.alloc(codeSignSect->size);

	// align to pointer size
	this->_encodedData.pad_to_size(8);
	this->_encoded = true;
}

void CodeSignatureAtom::hash(uint8_t* wholeFileBuffer) const
{
	Internal::FinalSection* codeSignSect = _state.sections.back();
	assert(codeSignSect->atoms[0] == this);
	uint8_t* codeSignBuffer = &wholeFileBuffer[codeSignSect->fileOffset];

	libcd_set_input_mem(_sigRef, wholeFileBuffer);
	libcd_set_output_mem(_sigRef, codeSignBuffer, codeSignSect->size);
	if ( libcd_serialize(_sigRef) != 0 )
		throw "error code signing";
}


} // namespace tool 
} // namespace ld 

#endif // __LINKEDIT_HPP__
