/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*- 
 *
 * Copyright (c) 2005-2009 Apple Inc. All rights reserved.
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
namespace {
	typedef long double max_align_t;
}
#endif

#include <vector>
#include <map>

#include "ld.hpp"
#include "opaque_section_file.h"


namespace opaque_section {



class Atom : public ld::Atom {
public:
	virtual ld::File*						file() const					{ return (ld::File*)&_file; }
	virtual const char*						name() const					{ return _name; }
	virtual uint64_t						size() const					{ return _size; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const
																			{ memcpy(buffer, _content, _size); }
	virtual void							setScope(Scope)					{ }

protected:
	friend class File;
											Atom(class File& f, const char* n,  const uint8_t* content, uint64_t sz);
	virtual									~Atom() {}
	
	class File&								_file;
	const char*								_name;
	const uint8_t*							_content;
	uint64_t								_size;
};


class File : public ld::File 
{
public:
								File(const char* segmentName, const char* sectionName, const char* pth, 
									const uint8_t fileContent[], uint64_t fileLength, 
									const char* symbolName="sect_create")
									: ld::File(pth, 0, ld::File::Ordinal::NullOrdinal(), Other),
									  _atom(*this, symbolName, fileContent, fileLength), 
									  _section(segmentName, sectionName, ld::Section::typeSectCreate) { }
	virtual						~File() { }
	
	virtual bool				forEachAtom(ld::File::AtomHandler& h) const { h.doAtom(_atom); return true; }
	virtual bool				justInTimeforEachAtom(const char* name, ld::File::AtomHandler&) const { return false; }

	ld::Atom*					atom() { return &_atom; }
private:
	friend class Atom;
	
	Atom						_atom;
	ld::Section					_section;
};

Atom::Atom(File& f, const char* n,  const uint8_t* content, uint64_t sz)
	: ld::Atom(f._section, ld::Atom::definitionRegular, ld::Atom::combineNever,
		ld::Atom::scopeTranslationUnit, ld::Atom::typeUnclassified, 
		symbolTableNotIn, true, false, false, ld::Atom::Alignment(0)), 
		_file(f), _name(n), _content(content), _size(sz) {}


//
// main function used by linker for -sectcreate
//
ld::File* parse(const char* segmentName, const char* sectionName, const char* path, 
				const uint8_t fileContent[], uint64_t fileLength, 
									const char* symbolName)
{
	return new File(segmentName, sectionName, path, fileContent, fileLength, symbolName);
}


} // namespace opaque_section






