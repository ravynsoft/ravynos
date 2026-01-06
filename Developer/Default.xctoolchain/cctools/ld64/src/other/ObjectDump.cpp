/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*- 
 *
 * Copyright (c) 2005-2010 Apple Inc. All rights reserved.
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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <mach-o/nlist.h>
#include <mach-o/stab.h>
#include <mach-o/fat.h>
#include <mach-o/loader.h>

#include "MachOFileAbstraction.hpp"
#include "parsers/macho_relocatable_file.h"
#include "parsers/lto_file.h"

const ld::VersionSet ld::File::_platforms;


static bool			sDumpContent= true;
static bool			sDumpStabs	= false;
static bool			sSort		= true;
static bool			sNMmode		= false;
static bool			sShowSection			= true;
static bool			sShowDefinitionKind		= true;
static bool			sShowCombineKind		= true;
static bool			sShowLineInfo			= true;

static cpu_type_t		sPreferredArch = 0xFFFFFFFF;
static cpu_subtype_t	sPreferredSubArch = 0xFFFFFFFF;
static const char* sMatchName = NULL;
static int sPrintRestrict;
static int sPrintAlign;
static int sPrintName;

 __attribute__((noreturn))
void throwf(const char* format, ...) 
{
	va_list	list;
	char*	p;
	va_start(list, format);
	vasprintf(&p, format, list);
	va_end(list);
	
	const char*	t = p;
	throw t;
}

void warning(const char* format, ...)
{
	va_list	list;
	fprintf(stderr, "warning: ");
	va_start(list, format);
	vfprintf(stderr, format, list);
	va_end(list);
	fprintf(stderr, "\n");
}

static void dumpStabs(const std::vector<ld::relocatable::File::Stab>* stabs)
{
	// debug info
	printf("stabs: (%lu)\n", stabs->size());
	for (std::vector<ld::relocatable::File::Stab>::const_iterator it = stabs->begin(); it != stabs->end(); ++it ) {
		const ld::relocatable::File::Stab& stab = *it;
		const char* code = "?????";
		switch (stab.type) {
			case N_GSYM:
				code = " GSYM";
				break;
			case N_FNAME:
				code = "FNAME";
				break;
			case N_FUN:
				code = "  FUN";
				break;
			case N_STSYM:
				code = "STSYM";
				break;
			case N_LCSYM:
				code = "LCSYM";
				break;
			case N_BNSYM:
				code = "BNSYM";
				break;
			case N_OPT:
				code = "  OPT";
				break;
			case N_RSYM:
				code = " RSYM";
				break;
			case N_SLINE:
				code = "SLINE";
				break;
			case N_ENSYM:
				code = "ENSYM";
				break;
			case N_SSYM:
				code = " SSYM";
				break;
			case N_SO:
				code = "   SO";
				break;
			case N_OSO:
				code = "  OSO";
				break;
			case N_LSYM:
				code = " LSYM";
				break;
			case N_BINCL:
				code = "BINCL";
				break;
			case N_SOL:
				code = "  SOL";
				break;
			case N_PARAMS:
				code = "PARMS";
				break;
			case N_VERSION:
				code = " VERS";
				break;
			case N_OLEVEL:
				code = "OLEVL";
				break;
			case N_PSYM:
				code = " PSYM";
				break;
			case N_EINCL:
				code = "EINCL";
				break;
			case N_ENTRY:
				code = "ENTRY";
				break;
			case N_LBRAC:
				code = "LBRAC";
				break;
			case N_EXCL:
				code = " EXCL";
				break;
			case N_RBRAC:
				code = "RBRAC";
				break;
			case N_BCOMM:
				code = "BCOMM";
				break;
			case N_ECOMM:
				code = "ECOMM";
				break;
			case N_LENG:
				code =  "LENG";
				break;
		}
		printf("  [atom=%20s] %02X %04X %s %s\n", ((stab.atom != NULL) ? stab.atom->name() : ""), stab.other, stab.desc, code, stab.string);
	}
}

#if 0
static void dumpAtomLikeNM(ld::Atom* atom)
{
	uint32_t size = atom->size();
	
	const char* visibility;
	switch ( atom->scope() ) {
		case ld::Atom::scopeTranslationUnit:
			visibility = "internal";
			break;
		case ld::Atom::scopeLinkageUnit:
			visibility = "hidden  ";
			break;
		case ld::Atom::scopeGlobal:
			visibility = "global  ";
			break;
		default:
			visibility = "        ";
			break;
	}

	const char* kind;
	switch ( atom->definitionKind() ) {
		case ld::Atom::kRegularDefinition:
			kind = "regular  ";
			break;
		case ld::Atom::kTentativeDefinition:
			kind = "tentative";
			break;
		case ld::Atom::kWeakDefinition:
			kind = "weak     ";
			break;
		case ld::Atom::kAbsoluteSymbol:
			kind = "absolute ";
			break;
		default:
			kind = "         ";
			break;
	}

	printf("0x%08X %s %s %s\n", size, visibility, kind, atom->name());
}


static void dumpAtom(ld::Atom* atom)
{
	if(sMatchName && strcmp(sMatchName, atom->name()))
		return;

	//printf("atom:    %p\n", atom);
	
	// name
	if(!sPrintRestrict || sPrintName)
		printf("name:    %s\n",  atom->name());
	
	// scope
	if(!sPrintRestrict)
		switch ( atom->scope() ) {
		case ld::Atom::scopeTranslationUnit:
			printf("scope:   translation unit\n");
			break;
		case ld::Atom::scopeLinkageUnit:
			printf("scope:   linkage unit\n");
			break;
		case ld::Atom::scopeGlobal:
			printf("scope:   global\n");
			break;
		default:
			printf("scope:   unknown\n");
		}
	
	// kind
	if(!sPrintRestrict)
		switch ( atom->definitionKind() ) {
		case ld::Atom::kRegularDefinition:
			printf("kind:     regular\n");
			break;
		case ld::Atom::kWeakDefinition:
			printf("kind:     weak\n");
			break;
		case ld::Atom::kTentativeDefinition:
			printf("kind:     tentative\n");
			break;
		case ld::Atom::kExternalDefinition:
			printf("kind:     import\n");
			break;
		case ld::Atom::kExternalWeakDefinition:
			printf("kind:     weak import\n");
			break;
		case ld::Atom::kAbsoluteSymbol:
			printf("kind:     absolute symbol\n");
			break;
		default:
			printf("kind:   unknown\n");
		}

	// segment and section
	if(!sPrintRestrict && (atom->section().sectionName() != NULL) )
		printf("section: %s,%s\n", atom->section().segmentName(), atom->section().sectionName());

	// attributes
	if(!sPrintRestrict) {
		printf("attrs:   ");
		if ( atom->dontDeadStrip() )
			printf("dont-dead-strip ");
		if ( atom->isThumb() )
			printf("thumb ");
		printf("\n");
	}
	
	// size
	if(!sPrintRestrict)
		printf("size:    0x%012llX\n", atom->size());
	
	// alignment
	if(!sPrintRestrict || sPrintAlign)
		printf("align:    %u mod %u\n", atom->alignment().modulus, (1 << atom->alignment().powerOf2) );

	// content
	if (!sPrintRestrict && sDumpContent ) { 
		uint64_t size = atom->size();
		if ( size < 4096 ) {
			uint8_t content[size];
			atom->copyRawContent(content);
			printf("content: ");
			if ( atom->contentType() == ld::Atom::typeCString ) {
				printf("\"");
				for (unsigned int i=0; i < size; ++i) {
					if(content[i]<'!' || content[i]>=127)
						printf("\\%o", content[i]);
					else
						printf("%c", content[i]);
				}
				printf("\"");
			}
			else {
				for (unsigned int i=0; i < size; ++i)
					printf("%02X ", content[i]);
			}
		}
		printf("\n");
	}
	
	// unwind info
	if(!sPrintRestrict) {
		if ( atom->beginUnwind() != atom->endUnwind() ) {
			printf("unwind encodings:\n");
			for (ld::Atom::UnwindInfo::iterator it = atom->beginUnwind(); it != atom->endUnwind(); ++it) {
				printf("\t 0x%04X  0x%08X\n", it->startOffset, it->unwindInfo);
			}
		}
	}
#if 0	
	// references
	if(!sPrintRestrict) {
		std::vector<ObjectFile::Reference*>&  references = atom->getReferences();
		const int refCount = references.size();
		printf("references: (%u)\n", refCount);
		for (int i=0; i < refCount; ++i) {
			ObjectFile::Reference* ref = references[i];
			printf("   %s\n", ref->getDescription());
		}
	}
#endif	
	// line info
	if(!sPrintRestrict) {
		if ( atom->beginLineInfo() != atom->endLineInfo() ) {
			printf("line info:\n");
			for (ld::Atom::LineInfo::iterator it = atom->beginLineInfo(); it != atom->endLineInfo(); ++it) {
				printf("   offset 0x%04X, line %d, file %s\n", it->atomOffset, it->lineNumber, it->fileName);
			}
		}
	}

	if(!sPrintRestrict)
		printf("\n");
}
#endif
struct AtomSorter
{
     bool operator()(const ld::Atom* left, const ld::Atom* right)
     {
		if ( left == right )
			return false;
		// first sort by segment name
		int diff = strcmp(left->section().segmentName(), right->section().segmentName());
		if ( diff != 0 )
			return (diff > 0);
		
		// then sort by section name
		diff = strcmp(left->section().sectionName(), right->section().sectionName());
		if ( diff != 0 )
			return (diff < 0);
		
		// then sort by atom name
		diff = strcmp(left->name(), right->name());
		if ( diff != 0 )
			return (diff < 0);
		
		// if cstring, sort by content
		if ( left->contentType() == ld::Atom::typeCString ) {
			diff = strcmp((char*)left->rawContentPointer(), (char*)right->rawContentPointer());
			if ( diff != 0 )
				return (diff < 0);
		}
		else if ( left->section().type() == ld::Section::typeCStringPointer ) {
			// if pointer to c-string sort by name
			const char* leftString = NULL;
			assert(left->fixupsBegin() != left->fixupsEnd());
			for (ld::Fixup::iterator fit = left->fixupsBegin(); fit != left->fixupsEnd(); ++fit) {
				if ( fit->binding == ld::Fixup::bindingByContentBound ) {
					const ld::Atom* cstringAtom = fit->u.target;
					assert(cstringAtom->contentType() == ld::Atom::typeCString);
					leftString = (char*)cstringAtom->rawContentPointer();
				}
			}
			const char* rightString = NULL;
			assert(right->fixupsBegin() != right->fixupsEnd());
			for (ld::Fixup::iterator fit = right->fixupsBegin(); fit != right->fixupsEnd(); ++fit) {
				if ( fit->binding == ld::Fixup::bindingByContentBound ) {
					const ld::Atom* cstringAtom = fit->u.target;
					assert(cstringAtom->contentType() == ld::Atom::typeCString);
					rightString = (char*)cstringAtom->rawContentPointer();
				}
			}
			if ( leftString != rightString ) {
				assert(leftString != NULL);
				assert(rightString != NULL);
				diff = strcmp(leftString, rightString);
				if ( diff != 0 )
					return (diff < 0);
			}
		}
		else if ( left->section().type() == ld::Section::typeLiteral4 ) {
			// if literal sort by content
			uint32_t leftValue  = *(uint32_t*)left->rawContentPointer();
			uint32_t rightValue = *(uint32_t*)right->rawContentPointer();
			diff = (leftValue - rightValue);
			if ( diff != 0 )
				return (diff < 0);
		}
		else if ( left->section().type() == ld::Section::typeCFI ) {
			// if __he_frame sort by address
			diff = (left->objectAddress() - right->objectAddress());
			if ( diff != 0 )
				return (diff < 0);
		}
		else if ( left->section().type() == ld::Section::typeNonLazyPointer ) {
			// if non-lazy-pointer sort by name
			const char* leftString = NULL;
			assert(left->fixupsBegin() != left->fixupsEnd());
			for (ld::Fixup::iterator fit = left->fixupsBegin(); fit != left->fixupsEnd(); ++fit) {
				if ( fit->binding == ld::Fixup::bindingByNameUnbound ) {
					leftString = fit->u.name;
				}
				else if ( fit->binding == ld::Fixup::bindingDirectlyBound ) {
					leftString = fit->u.target->name();
				}
			}
			const char* rightString = NULL;
			assert(right->fixupsBegin() != right->fixupsEnd());
			for (ld::Fixup::iterator fit = right->fixupsBegin(); fit != right->fixupsEnd(); ++fit) {
				if ( fit->binding == ld::Fixup::bindingByNameUnbound ) {
					rightString = fit->u.name;
				}
				else if ( fit->binding == ld::Fixup::bindingDirectlyBound ) {
					rightString = fit->u.target->name();
				}
			}
			assert(leftString != NULL);
			assert(rightString != NULL);
			diff = strcmp(leftString, rightString);
			if ( diff != 0 )
				return (diff < 0);
		}
		
		// else sort by size
		return (left->size() < right->size());
     }
};


class dumper : public ld::File::AtomHandler
{
public:
			void dump();
	virtual void doAtom(const ld::Atom&);
	virtual void doFile(const ld::File&) {} 
private:
	void			dumpAtom(const ld::Atom& atom);
	const char*		scopeString(const ld::Atom&);
	const char*		definitionString(const ld::Atom&);
	const char*		combineString(const ld::Atom&);
	const char*		inclusionString(const ld::Atom&);
	const char*		attributeString(const ld::Atom&);
	const char*		makeName(const ld::Atom& atom);
	const char*		referenceTargetAtomName(const ld::Fixup* ref);
	void			dumpFixup(const ld::Fixup* ref);
	
	uint64_t		addressOfFirstAtomInSection(const ld::Section&);
	
	std::vector<const ld::Atom*> _atoms;
};

const char*	dumper::scopeString(const ld::Atom& atom)
{
	switch ( (ld::Atom::Scope)atom.scope() ) {
		case ld::Atom::scopeTranslationUnit:
			return "translation-unit";
		case ld::Atom::scopeLinkageUnit:
			return "hidden";
		case ld::Atom::scopeGlobal:
			if ( atom.autoHide() )
				return "global but automatically hidden";
			else
				return "global";
	}
	return "UNKNOWN";	
}

const char*	dumper::definitionString(const ld::Atom& atom)
{
	switch ( (ld::Atom::Definition)atom.definition() ) {
		case ld::Atom::definitionRegular:
			return "regular";
		case ld::Atom::definitionTentative:
			return "tentative";
		case ld::Atom::definitionAbsolute:
			return "absolute";
		case ld::Atom::definitionProxy:
			return "proxy";
		}
	return "UNKNOWN";	
}

const char*	dumper::combineString(const ld::Atom& atom)
{
	switch ( (ld::Atom::Combine)atom.combine() ) {
		case ld::Atom::combineNever:
			return "never";
		case ld::Atom::combineByName:
			return "by-name";
		case ld::Atom::combineByNameAndContent:
			return "by-name-and-content";
		case ld::Atom::combineByNameAndReferences:
			return "by-name-and-references";
		}
	return "UNKNOWN";	
}

const char*	dumper::inclusionString(const ld::Atom& atom)
{
	switch ( (ld::Atom::SymbolTableInclusion)atom.symbolTableInclusion() ) {
		case ld::Atom::symbolTableNotIn:
			return "not in";
		case ld::Atom::symbolTableNotInFinalLinkedImages:
			return "not in final linked images";
		case ld::Atom::symbolTableIn:
			return "in";
		case ld::Atom::symbolTableInAndNeverStrip:
			return "in and never strip";
		case ld::Atom::symbolTableInAsAbsolute:
			return "in as absolute";
		case ld::Atom::symbolTableInWithRandomAutoStripLabel:
			return "in as random auto-strip label";
		}
	return "UNKNOWN";	
}



const char*	dumper::attributeString(const ld::Atom& atom)
{
	static char buffer[256];
	buffer[0] = '\0';
	
	if ( atom.dontDeadStrip() )
		strcat(buffer, "dont-dead-strip ");

	if ( atom.isThumb() )
		strcat(buffer, "thumb ");
		
	if ( atom.isAlias() )
		strcat(buffer, "alias ");
		
	if ( atom.cold() )
		strcat(buffer, "cold ");

	if ( atom.contentType() == ld::Atom::typeResolver )
		strcat(buffer, "resolver ");
		
	return buffer;
}

const char* dumper::makeName(const ld::Atom& atom)
{
	static char buffer[4096];
	strcpy(buffer, "???");
	switch ( atom.symbolTableInclusion() ) {
		case ld::Atom::symbolTableNotIn:
			if ( atom.contentType() == ld::Atom::typeCString ) {
				strcpy(buffer, "cstring=");
				strlcat(buffer, (char*)atom.rawContentPointer(), 4096);
			}
			else if ( atom.section().type() == ld::Section::typeLiteral4 ) {
				char temp[16];
				strcpy(buffer, "literal4=");
				uint32_t value = *(uint32_t*)atom.rawContentPointer();
				sprintf(temp, "0x%08X", value);
				strcat(buffer, temp);
			}
			else if ( atom.section().type() == ld::Section::typeLiteral8 ) {
				char temp[32];
				strcpy(buffer, "literal8=");
				uint32_t value1 = *(uint32_t*)atom.rawContentPointer();
				uint32_t value2 = ((uint32_t*)atom.rawContentPointer())[1];
				sprintf(temp, "0x%08X%08X", value1, value2);
				strcat(buffer, temp);
			}
			else if ( atom.section().type() == ld::Section::typeLiteral16 ) {
				char temp[64];
				strcpy(buffer, "literal16=");
				uint32_t value1 = *(uint32_t*)atom.rawContentPointer();
				uint32_t value2 = ((uint32_t*)atom.rawContentPointer())[1];
				uint32_t value3 = ((uint32_t*)atom.rawContentPointer())[2];
				uint32_t value4 = ((uint32_t*)atom.rawContentPointer())[3];
				sprintf(temp, "0x%08X%08X%08X%08X", value1, value2, value3, value4);
				strcat(buffer, temp);
			}
			else if ( atom.section().type() == ld::Section::typeCStringPointer ) {
				assert(atom.fixupsBegin() != atom.fixupsEnd());
				for (ld::Fixup::iterator fit = atom.fixupsBegin(); fit != atom.fixupsEnd(); ++fit) {
					if ( fit->binding == ld::Fixup::bindingByContentBound ) {
						const ld::Atom* cstringAtom = fit->u.target;
						if ( (cstringAtom != NULL) && (cstringAtom->contentType() == ld::Atom::typeCString) ) {
							strlcpy(buffer, atom.name(), 4096);
							strlcat(buffer, "=", 4096);
							strlcat(buffer, (char*)cstringAtom->rawContentPointer(), 4096);
						}
					}
				}
			}
			else if ( atom.section().type() == ld::Section::typeNonLazyPointer ) {
				assert(atom.fixupsBegin() != atom.fixupsEnd());
				for (ld::Fixup::iterator fit = atom.fixupsBegin(); fit != atom.fixupsEnd(); ++fit) {
					if ( fit->binding == ld::Fixup::bindingByNameUnbound ) {
						strcpy(buffer, "non-lazy-pointer-to:");
						strlcat(buffer, fit->u.name, 4096);
						return buffer;
					}
					else if ( fit->binding == ld::Fixup::bindingDirectlyBound ) {
						strcpy(buffer, "non-lazy-pointer-to-local:");
						strlcat(buffer, fit->u.target->name(), 4096);
						return buffer;
					}
				}
				strlcpy(buffer, atom.name(), 4096);
			}
			else {
				uint64_t sectAddr = addressOfFirstAtomInSection(atom.section());
				sprintf(buffer, "%s@%s+0x%08llX", atom.name(), atom.section().sectionName(), atom.objectAddress()-sectAddr);
			}
			break;
		case ld::Atom::symbolTableNotInFinalLinkedImages:
		case ld::Atom::symbolTableIn:
		case ld::Atom::symbolTableInAndNeverStrip:
		case ld::Atom::symbolTableInAsAbsolute:
		case ld::Atom::symbolTableInWithRandomAutoStripLabel:
			strlcpy(buffer, atom.name(), 4096);
			break;
	}
	return buffer;
}

const char* dumper::referenceTargetAtomName(const ld::Fixup* ref)
{
	static char buffer[4096];
	switch ( ref->binding ) {
		case ld::Fixup::bindingNone:
			return "NO BINDING";
		case ld::Fixup::bindingByNameUnbound:
			strcpy(buffer, "by-name(");
			strlcat(buffer, ref->u.name, 4096);
			strlcat(buffer, ")", 4096);
			return buffer;
			//return ref->u.name;
		case ld::Fixup::bindingByContentBound:
			strcpy(buffer, "by-content(");
			strlcat(buffer, makeName(*ref->u.target), 4096);
			strlcat(buffer, ")", 4096);
			return buffer;
		case ld::Fixup::bindingDirectlyBound:
			strcpy(buffer, "direct(");
			strlcat(buffer, makeName(*ref->u.target), 4096);
			strlcat(buffer, ")", 4096);
			return buffer;
		case ld::Fixup::bindingsIndirectlyBound:
			return "BOUND INDIRECTLY";
	}
	return "BAD BINDING";
}


void dumper::dumpFixup(const ld::Fixup* ref)
{
	if ( ref->weakImport ) {
		printf("weak_import ");
	}
	switch ( (ld::Fixup::Kind)(ref->kind) ) {
		case ld::Fixup::kindNone:
			printf("none");
			break;
		case ld::Fixup::kindNoneFollowOn:
			printf("followed by %s", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindNoneGroupSubordinate:
			printf("group subordinate %s", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindNoneGroupSubordinateFDE:
			printf("group subordinate FDE %s", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindNoneGroupSubordinateLSDA:
			printf("group subordinate LSDA %s", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindNoneGroupSubordinatePersonality:
			printf("group subordinate personality %s", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindSetTargetAddress:
			printf("%s", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindSubtractTargetAddress:
			printf(" - %s", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindAddAddend:
			printf(" + 0x%llX", ref->u.addend);
			break;
		case ld::Fixup::kindSubtractAddend:
			printf(" - 0x%llX", ref->u.addend);
			break;
		case ld::Fixup::kindSetTargetImageOffset:
			printf("imageOffset(%s)", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindSetTargetSectionOffset:
			printf("sectionOffset(%s)", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindStore8:
			printf(", then store byte");
			break;
		case ld::Fixup::kindStoreLittleEndian16:
			printf(", then store 16-bit little endian");
			break;
		case ld::Fixup::kindStoreLittleEndianLow24of32:
			printf(", then store low 24-bit little endian");
			break;
		case ld::Fixup::kindStoreLittleEndian32:
			printf(", then store 32-bit little endian");
			break;
		case ld::Fixup::kindStoreLittleEndian64:
			printf(", then store 64-bit little endian");
			break;
		case ld::Fixup::kindStoreBigEndian16:
			printf(", then store 16-bit big endian");
			break;
		case ld::Fixup::kindStoreBigEndianLow24of32:
			printf(", then store low 24-bit big endian");
			break;
		case ld::Fixup::kindStoreBigEndian32:
			printf(", then store 32-bit big endian");
			break;
		case ld::Fixup::kindStoreBigEndian64:
			printf(", then store 64-bit big endian");
			break;
		case ld::Fixup::kindStoreX86BranchPCRel8:
			printf(", then store as x86 8-bit pcrel branch");
			break;
		case ld::Fixup::kindStoreX86BranchPCRel32:
			printf(", then store as x86 32-bit pcrel branch");
			break;
		case ld::Fixup::kindStoreX86PCRel8:
			printf(", then store as x86 8-bit pcrel");
			break;
		case ld::Fixup::kindStoreX86PCRel16:
			printf(", then store as x86 16-bit pcrel");
			break;
		case ld::Fixup::kindStoreX86PCRel32:
			printf(", then store as x86 32-bit pcrel");
			break;
		case ld::Fixup::kindStoreX86PCRel32_1:
			printf(", then store as x86 32-bit pcrel from +1");
			break;
		case ld::Fixup::kindStoreX86PCRel32_2:
			printf(", then store as x86 32-bit pcrel from +2");
			break;
		case ld::Fixup::kindStoreX86PCRel32_4:
			printf(", then store as x86 32-bit pcrel from +4");
			break;
		case ld::Fixup::kindStoreX86PCRel32GOTLoad:
			printf(", then store as x86 32-bit pcrel GOT load");
			break;
		case ld::Fixup::kindStoreX86PCRel32GOTLoadNowLEA:
			printf(", then store as x86 32-bit pcrel GOT load -> LEA");
			break;
		case ld::Fixup::kindStoreX86PCRel32GOT:
			printf(", then store as x86 32-bit pcrel GOT access");
			break;
		case ld::Fixup::kindStoreX86PCRel32TLVLoad:
			printf(", then store as x86 32-bit pcrel TLV load");
			break;
		case ld::Fixup::kindStoreX86PCRel32TLVLoadNowLEA:
			printf(", then store as x86 32-bit pcrel TLV load");
			break;
		case ld::Fixup::kindStoreX86Abs32TLVLoad:
			printf(", then store as x86 32-bit absolute TLV load");
			break;
		case ld::Fixup::kindStoreX86Abs32TLVLoadNowLEA:
			printf(", then store as x86 32-bit absolute TLV load -> LEA");
			break;
		case ld::Fixup::kindStoreARMBranch24:
			printf(", then store as ARM 24-bit pcrel branch");
			break;
		case ld::Fixup::kindStoreThumbBranch22:
			printf(", then store as Thumb 22-bit pcrel branch");
			break;
		case ld::Fixup::kindStoreARMLoad12:
			printf(", then store as ARM 12-bit pcrel load");
			break;
		case ld::Fixup::kindStoreARMLow16:
			printf(", then store low-16 in ARM movw");
			break;
		case ld::Fixup::kindStoreARMHigh16:
			printf(", then store high-16 in ARM movt");
			break;
		case ld::Fixup::kindStoreThumbLow16:
			printf(", then store low-16 in Thumb movw");
			break;
		case ld::Fixup::kindStoreThumbHigh16:
			printf(", then store high-16 in Thumb movt");
			break;
#if SUPPORT_ARCH_arm64
		case ld::Fixup::kindStoreARM64Branch26:
			printf(", then store as ARM64 26-bit pcrel branch");
			break;
		case ld::Fixup::kindStoreARM64Page21:
			printf(", then store as ARM64 21-bit pcrel ADRP");
			break;
		case ld::Fixup::kindStoreARM64PageOff12:
			printf(", then store as ARM64 12-bit offset");
			break;
		case ld::Fixup::kindStoreARM64GOTLoadPage21:
			printf(", then store as ARM64 21-bit pcrel ADRP of GOT");
			break;
		case ld::Fixup::kindStoreARM64GOTLoadPageOff12:
			printf(", then store as ARM64 12-bit page offset of GOT");
			break;
		case ld::Fixup::kindStoreARM64GOTLeaPage21:
			printf(", then store as ARM64 21-bit pcrel ADRP of GOT lea");
			break;
		case ld::Fixup::kindStoreARM64GOTLeaPageOff12:
			printf(", then store as ARM64 12-bit page offset of GOT lea");
			break;
		case ld::Fixup::kindStoreARM64TLVPLoadPage21:
			printf(", then store as ARM64 21-bit pcrel ADRP of TLVP");
			break;
		case ld::Fixup::kindStoreARM64TLVPLoadPageOff12:
			printf(", then store as ARM64 12-bit page offset of TLVP");
			break;
		case ld::Fixup::kindStoreARM64TLVPLoadNowLeaPage21:
			printf(", then store as ARM64 21-bit pcrel ADRP of lea of TLVP");
			break;
		case ld::Fixup::kindStoreARM64TLVPLoadNowLeaPageOff12:
			printf(", then store as ARM64 12-bit page offset of lea of TLVP");
			break;
		case ld::Fixup::kindStoreARM64PointerToGOT:
			printf(", then store as 64-bit pointer to GOT entry");
			break;
		case ld::Fixup::kindStoreARM64PCRelToGOT:
			printf(", then store as 32-bit delta to GOT entry");
			break;
#endif
#if SUPPORT_ARCH_arm64_32
		case ld::Fixup::kindStoreARM64PointerToGOT32:
			printf(", then store as 32-bit pointer to GOT entry");
			break;
#endif
		case ld::Fixup::kindDtraceExtra:
			printf("dtrace static probe extra info");
			break;
		case ld::Fixup::kindStoreX86DtraceCallSiteNop:
			printf("x86 dtrace static probe site");
			break;
		case ld::Fixup::kindStoreX86DtraceIsEnableSiteClear:
			printf("x86 dtrace static is-enabled site");
			break;
		case ld::Fixup::kindStoreARMDtraceCallSiteNop:
			printf("ARM dtrace static probe site");
			break;
		case ld::Fixup::kindStoreARMDtraceIsEnableSiteClear:
			printf("ARM dtrace static is-enabled site");
			break;
		case ld::Fixup::kindStoreThumbDtraceCallSiteNop:
			printf("Thumb dtrace static probe site");
			break;
		case ld::Fixup::kindStoreThumbDtraceIsEnableSiteClear:
			printf("Thumb dtrace static is-enabled site");
			break;
#if SUPPORT_ARCH_arm64
		case ld::Fixup::kindStoreARM64DtraceCallSiteNop:
			printf("ARM64 dtrace static probe site");
			break;
		case ld::Fixup::kindStoreARM64DtraceIsEnableSiteClear:
			printf("ARM64 dtrace static is-enabled site");
			break;
#endif
		case ld::Fixup::kindLazyTarget:
			printf("lazy reference to external symbol %s", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindSetLazyOffset:
			printf("offset of lazy binding info for %s", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindIslandTarget:
			printf("ultimate target of island %s", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindDataInCodeStartData:
			printf("start of data in code");
			break;
		case ld::Fixup::kindDataInCodeStartJT8:
			printf("start of jump table 8 data in code");
			break;
		case ld::Fixup::kindDataInCodeStartJT16:
			printf("start of jump table 16 data in code");
			break;
		case ld::Fixup::kindDataInCodeStartJT32:
			printf("start of jump table 32 data in code");
			break;
		case ld::Fixup::kindDataInCodeStartJTA32:
			printf("start of jump table absolute 32 data in code");
			break;
		case ld::Fixup::kindDataInCodeEnd:
			printf("end of data in code");
			break;
		case ld::Fixup::kindLinkerOptimizationHint:
#if SUPPORT_ARCH_arm64
			ld::Fixup::LOH_arm64 extra;
			extra.addend = ref->u.addend;
			printf("ARM64 hint: ");
			switch(extra.info.kind) {
				case LOH_ARM64_ADRP_ADRP:
					printf("ADRP-ADRP");
					break;
				case LOH_ARM64_ADRP_LDR:
					printf("ADRP-LDR");
					break;
				case LOH_ARM64_ADRP_ADD_LDR:
					printf("ADRP-ADD-LDR");
					break;
				case LOH_ARM64_ADRP_LDR_GOT_LDR:
					printf("ADRP-LDR-GOT-LDR");
					break;
				case LOH_ARM64_ADRP_ADD_STR:
					printf("ADRP-ADD-STR");
					break;
				case LOH_ARM64_ADRP_LDR_GOT_STR:
					printf("ADRP-LDR-GOT-STR");
					break;
				case LOH_ARM64_ADRP_ADD:
					printf("ADRP-ADD");
					break;
				default:
					printf("kind=%d", extra.info.kind);
					break;
			}
			printf(", offset1=0x%X", (extra.info.delta1 << 2)  + ref->offsetInAtom);
			if ( extra.info.count > 0 )
				printf(", offset2=0x%X", (extra.info.delta2 << 2) + ref->offsetInAtom);
			if ( extra.info.count > 1 )
				printf(", offset3=0x%X", (extra.info.delta3 << 2)  + ref->offsetInAtom);
			if ( extra.info.count > 2 )
				printf(", offset4=0x%X", (extra.info.delta4 << 2)  + ref->offsetInAtom);
#endif			
			break;
		case ld::Fixup::kindStoreTargetAddressLittleEndian32:
			printf("store 32-bit little endian address of %s", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindStoreTargetAddressLittleEndian64:
			printf("store 64-bit little endian address of %s", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindStoreTargetAddressBigEndian32:
			printf("store 32-bit big endian address of %s", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindStoreTargetAddressBigEndian64:
			printf("store 64-bit big endian address of %s", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindStoreTargetAddressX86PCRel32:
			printf("x86 store 32-bit pc-rel address of %s", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindStoreTargetAddressX86BranchPCRel32:
			printf("x86 store 32-bit pc-rel branch to %s", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindStoreTargetAddressX86PCRel32GOTLoad:
			printf("x86 store 32-bit pc-rel GOT load of %s", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindStoreTargetAddressX86PCRel32GOTLoadNowLEA:
			printf("x86 store 32-bit pc-rel lea of %s", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindStoreTargetAddressX86PCRel32TLVLoad:
			printf("x86 store 32-bit pc-rel TLV load of %s", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindStoreTargetAddressX86PCRel32TLVLoadNowLEA:
			printf("x86 store 32-bit pc-rel TLV lea of %s", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindStoreTargetAddressX86Abs32TLVLoad:
			printf("x86 store 32-bit absolute TLV load of %s", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindStoreTargetAddressX86Abs32TLVLoadNowLEA:
			printf("x86 store 32-bit absolute TLV lea of %s", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindStoreTargetAddressARMBranch24:
			printf("ARM store 24-bit pc-rel branch to %s", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindStoreTargetAddressThumbBranch22:
			printf("Thumb store 22-bit pc-rel branch to %s", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindStoreTargetAddressARMLoad12:
			printf("ARM store 12-bit pc-rel branch to %s", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindSetTargetTLVTemplateOffset:
		case ld::Fixup::kindSetTargetTLVTemplateOffsetLittleEndian32:
		case ld::Fixup::kindSetTargetTLVTemplateOffsetLittleEndian64:
			printf("tlv template offset of %s", referenceTargetAtomName(ref));
			break;
#if SUPPORT_ARCH_arm64
		case ld::Fixup::kindStoreTargetAddressARM64Branch26:
			printf("ARM64 store 26-bit pcrel branch to %s", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindStoreTargetAddressARM64Page21:
			printf("ARM64 store 21-bit pcrel ADRP to %s", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindStoreTargetAddressARM64PageOff12:
			printf("ARM64 store 12-bit page offset of %s", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindStoreTargetAddressARM64PageOff12ConvertAddToLoad:
			printf("ARM64 store 12-bit page offset of %s, then convert add to load", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindStoreTargetAddressARM64GOTLoadPage21:
			printf("ARM64 store 21-bit pcrel ADRP to GOT for %s", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindStoreTargetAddressARM64GOTLoadPageOff12:
			printf("ARM64 store 12-bit page offset of GOT of %s", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindStoreTargetAddressARM64GOTLeaPage21:
			printf("ARM64 store 21-bit pcrel ADRP to GOT lea for %s", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindStoreTargetAddressARM64GOTLeaPageOff12:
			printf("ARM64 store 12-bit page offset of GOT lea of %s", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindStoreTargetAddressARM64TLVPLoadPage21:
			printf("ARM64 store 21-bit pcrel ADRP to TLV for %s", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindStoreTargetAddressARM64TLVPLoadPageOff12:
			printf("ARM64 store 12-bit page offset of TLV of %s", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindStoreTargetAddressARM64TLVPLoadNowLeaPage21:
			printf("ARM64 store 21-bit pcrel ADRP to lea for TLV for %s", referenceTargetAtomName(ref));
			break;
		case ld::Fixup::kindStoreTargetAddressARM64TLVPLoadNowLeaPageOff12:
			printf("ARM64 store 12-bit page offset of lea for TLV of %s", referenceTargetAtomName(ref));
			break;
#endif
#if SUPPORT_ARCH_arm64e
		case ld::Fixup::kindSetAuthData:
			printf("(addrDiv=%d, diversity=0X%04X, key=%d) ", ref->u.authData.hasAddressDiversity, ref->u.authData.discriminator, ref->u.authData.key);
			break;
		case ld::Fixup::kindStoreLittleEndianAuth64:
			printf(", then store auth 64-bit little endian");
			break;
		case ld::Fixup::kindStoreTargetAddressLittleEndianAuth64:
			printf("store auth 64-bit little endian address of %s", referenceTargetAtomName(ref));
			break;
#endif
		//default:
		//	printf("unknown fixup");
		//	break;
	}
}

uint64_t dumper::addressOfFirstAtomInSection(const ld::Section& sect)
{
	uint64_t lowestAddr = (uint64_t)(-1);
	for (std::vector<const ld::Atom*>::iterator it=_atoms.begin(); it != _atoms.end(); ++it) {
		const ld::Atom* atom = *it;
		if ( &atom->section() == &sect ) {
			if ( atom->objectAddress() < lowestAddr )
				lowestAddr = atom->objectAddress();
		}
	}
	return lowestAddr;
}

void dumper::doAtom(const ld::Atom& atom)
{
	if ( (sMatchName != NULL) && (strcmp(sMatchName, atom.name()) != 0) )
		return;
	_atoms.push_back(&atom);
}

void dumper::dump()
{	
	if ( sSort ) 
		std::sort(_atoms.begin(), _atoms.end(), AtomSorter());

	for (std::vector<const ld::Atom*>::iterator it=_atoms.begin(); it != _atoms.end(); ++it) {
		this->dumpAtom(**it);
	}
}	

void dumper::dumpAtom(const ld::Atom& atom)
{		
 	printf("name:     %s\n", makeName(atom)); 
	printf("size:     0x%0llX\n", atom.size());
	printf("align:    %u mod %u\n", atom.alignment().modulus, (1 << atom.alignment().powerOf2) );
	printf("scope:    %s\n", scopeString(atom));
	if ( sShowDefinitionKind ) 
		printf("def:      %s\n", definitionString(atom));
	if ( sShowCombineKind )
		printf("combine:  %s\n", combineString(atom));
	printf("symbol:   %s\n", inclusionString(atom));
	printf("attrs:    %s\n", attributeString(atom));
	if ( sShowSection )
		printf("section:  %s,%s\n", atom.section().segmentName(), atom.section().sectionName());
	if ( atom.beginUnwind() != atom.endUnwind() ) {
		uint32_t lastOffset = 0;
		uint32_t lastCUE = 0;
		bool first = true;
		const char* label = "unwind:";
		for (ld::Atom::UnwindInfo::iterator it=atom.beginUnwind(); it != atom.endUnwind(); ++it) {
			if ( !first ) {
				printf("%s   0x%08X -> 0x%08X: 0x%08X\n", label, lastOffset, it->startOffset, lastCUE);
				label = "       ";
			}
			lastOffset = it->startOffset;
			lastCUE = it->unwindInfo;
			first = false;
		}
		printf("%s   0x%08X -> 0x%08X: 0x%08X\n", label, lastOffset, (uint32_t)atom.size(), lastCUE);
	}
	if ( atom.contentType() == ld::Atom::typeCString ) {
		uint8_t buffer[atom.size()+2];
		atom.copyRawContent(buffer);
		buffer[atom.size()] = '\0';
		printf("content:  \"%s\"\n", buffer);
	}
	if ( atom.fixupsBegin() != atom.fixupsEnd() ) {
		printf("fixups:\n");
		for (unsigned int off=0; off < atom.size()+1; ++off) {
			for (ld::Fixup::iterator it = atom.fixupsBegin(); it != atom.fixupsEnd(); ++it) {
				if ( it->offsetInAtom == off ) {
					switch ( it->clusterSize ) {
						case ld::Fixup::k1of1:
							printf("    0x%04X ", it->offsetInAtom);
							dumpFixup(it);
							break;
						case ld::Fixup::k1of2:
							printf("    0x%04X ", it->offsetInAtom);
							dumpFixup(it);
							++it;
							dumpFixup(it);
							break;
						case ld::Fixup::k1of3:
							printf("    0x%04X ", it->offsetInAtom);
							dumpFixup(it);
							++it;
							dumpFixup(it);
							++it;
							dumpFixup(it);
							break;
						case ld::Fixup::k1of4:
							printf("    0x%04X ", it->offsetInAtom);
							dumpFixup(it);
							++it;
							dumpFixup(it);
							++it;
							dumpFixup(it);
							++it;
							dumpFixup(it);
							break;
						case ld::Fixup::k1of5:
							printf("    0x%04X ", it->offsetInAtom);
							dumpFixup(it);
							++it;
							dumpFixup(it);
							++it;
							dumpFixup(it);
							++it;
							dumpFixup(it);
							++it;
							dumpFixup(it);
							break;
						default:
							printf("   BAD CLUSTER SIZE: cluster=%d\n", it->clusterSize);
					}
					printf("\n");
				}
			}
		}
	}
	if ( sShowLineInfo ) {
		if ( atom.beginLineInfo() != atom.endLineInfo() ) {
			printf("line info:\n");
			for (ld::Atom::LineInfo::iterator it = atom.beginLineInfo(); it != atom.endLineInfo(); ++it) {
				printf("   offset 0x%04X, line %d, file %s\n", it->atomOffset, it->lineNumber, it->fileName);
			}
		}
	}
	
	printf("\n");
}

static void dumpFile(ld::relocatable::File* file)
{
	// stabs debug info
	if ( sDumpStabs && (file->debugInfo() == ld::relocatable::File::kDebugInfoStabs) ) {
		const std::vector<ld::relocatable::File::Stab>* stabs = file->stabs();
		if ( stabs != NULL )
			dumpStabs(stabs);
	}
	// dump atoms
	dumper d;
	file->forEachAtom(d);
	d.dump();

#if 0	
	// get all atoms
	std::vector<ObjectFile::Atom*> atoms = reader->getAtoms();
	
	// make copy of vector and sort (so output is canonical)
	std::vector<ObjectFile::Atom*> sortedAtoms(atoms);
	if ( sSort )
		std::sort(sortedAtoms.begin(), sortedAtoms.end(), AtomSorter());
	
	for(std::vector<ObjectFile::Atom*>::iterator it=sortedAtoms.begin(); it != sortedAtoms.end(); ++it) {
		if ( sNMmode )
			dumpAtomLikeNM(*it);
		else
			dumpAtom(*it);
	}
#endif
}


static ld::relocatable::File* createReader(const char* path)
{
	struct stat stat_buf;
	
	int fd = ::open(path, O_RDONLY, 0);
	if ( fd == -1 )
		throwf("cannot open file: %s", path);
	::fstat(fd, &stat_buf);
	uint8_t* p = (uint8_t*)::mmap(NULL, stat_buf.st_size, PROT_READ, MAP_FILE | MAP_PRIVATE, fd, 0);
	::close(fd);
	if ( p == (uint8_t*)(-1) )
		throwf("cannot mmap file: %s", path);
	const mach_header* mh = (mach_header*)p;
	uint64_t fileLen = stat_buf.st_size;
	bool foundFatSlice = false;
	if ( mh->magic == OSSwapBigToHostInt32(FAT_MAGIC) ) {
		const struct fat_header* fh = (struct fat_header*)p;
		const struct fat_arch* archs = (struct fat_arch*)(p + sizeof(struct fat_header));
		if ( (uint32_t)sPreferredArch ==  0xFFFFFFFF ) {
			// just dump first slice of fat .o file
			if ( OSSwapBigToHostInt32(fh->nfat_arch) > 0 )  {
				p = p + OSSwapBigToHostInt32(archs[0].offset);
				mh = (struct mach_header*)p;
				fileLen = OSSwapBigToHostInt32(archs[0].size);
				sPreferredArch = OSSwapBigToHostInt32(archs[0].cputype);
				sPreferredSubArch = OSSwapBigToHostInt32(archs[0].cpusubtype);
				foundFatSlice = true;
			}
		}
		else {
			for (unsigned long i=0; i < OSSwapBigToHostInt32(fh->nfat_arch); ++i) {
				if ( OSSwapBigToHostInt32(archs[i].cputype) == (uint32_t)sPreferredArch ) {
					if ( ((uint32_t)sPreferredSubArch == 0xFFFFFFFF) || ((uint32_t)sPreferredSubArch == OSSwapBigToHostInt32(archs[i].cpusubtype)) ) {
						p = p + OSSwapBigToHostInt32(archs[i].offset);
						mh = (struct mach_header*)p;
						fileLen = OSSwapBigToHostInt32(archs[i].size);
						foundFatSlice = true;
						break;
					}
				}
			}
		}
	}

	mach_o::relocatable::ParserOptions objOpts;
	objOpts.architecture		= sPreferredArch;
	objOpts.objSubtypeMustMatch = false;
	objOpts.logAllFiles			= false;
	objOpts.warnUnwindConversionProblems	= true;
	objOpts.keepDwarfUnwind		= false;
	objOpts.forceDwarfConversion = false;
	objOpts.verboseOptimizationHints = true;
	objOpts.armUsesZeroCostExceptions = true;
#if SUPPORT_ARCH_arm64e
	objOpts.supportsAuthenticatedPointers = true;
#endif
	objOpts.subType				= sPreferredSubArch;
	objOpts.treateBitcodeAsData = false;
	objOpts.usingBitcode		= true;
	objOpts.forceHidden			= false;
#if 1
	if ( ! foundFatSlice ) {
		cpu_type_t archOfObj;
		cpu_subtype_t subArchOfObj;
		ld::VersionSet platformsFound;
		if ( mach_o::relocatable::isObjectFile(p, fileLen, &archOfObj, &subArchOfObj, platformsFound) ) {
			objOpts.architecture = archOfObj;
			objOpts.subType = subArchOfObj;
		}
	}

	ld::relocatable::File* objResult = mach_o::relocatable::parse(p, fileLen, path, stat_buf.st_mtime, ld::File::Ordinal::NullOrdinal(), objOpts);
	if ( objResult != NULL )
		return objResult;

	// see if it is an llvm object file
	objResult = lto::parse(p, fileLen, path, stat_buf.st_mtime, ld::File::Ordinal::NullOrdinal(), sPreferredArch, sPreferredSubArch, false, true);
	if ( objResult != NULL ) 
		return objResult;

	throwf("not a mach-o object file: %s", path);
#else
	// for peformance testing
	for (int i=0; i < 500; ++i ) {
		ld::relocatable::File* objResult = mach_o::relocatable::parse(p, fileLen, path, stat_buf.st_mtime, 0, objOpts);
		delete objResult;
	}
	exit(0);
#endif
}

static
void
usage()
{
	fprintf(stderr, "ObjectDump options:\n"
			"\t-no_content\tdon't dump contents\n"
			"\t-no_section\tdon't dump section name\n"
			"\t-no_definition\tdon't dump definition kind\n"
			"\t-no_combine\tdon't dump combine mode\n"
			"\t-stabs\t\tdump stabs\n"
			"\t-arch aaa\tonly dump info about arch aaa\n"
			"\t-only sym\tonly dump info about sym\n"
			"\t-align\t\tonly print alignment info\n"
			"\t-name\t\tonly print symbol names\n"
		);
}

int main(int argc, const char* argv[])
{
	if(argc<2) {
		usage();
		return 0;
	}

	try {
		for(int i=1; i < argc; ++i) {
			const char* arg = argv[i];
			if ( arg[0] == '-' ) {
				if ( strcmp(arg, "-no_content") == 0 ) {
					sDumpContent = false;
				}
				else if ( strcmp(arg, "-nm") == 0 ) {
					sNMmode = true;
				}
				else if ( strcmp(arg, "-stabs") == 0 ) {
					sDumpStabs = true;
				}
				else if ( strcmp(arg, "-no_sort") == 0 ) {
					sSort = false;
				}
				else if ( strcmp(arg, "-no_section") == 0 ) {
					sShowSection = false;
				}
				else if ( strcmp(arg, "-no_definition") == 0 ) {
					sShowDefinitionKind = false;
				}
				else if ( strcmp(arg, "-no_combine") == 0 ) {
					sShowCombineKind = false;
				}
				else if ( strcmp(arg, "-no_line_info") == 0 ) {
					sShowLineInfo = false;
				}
				else if ( strcmp(arg, "-arch") == 0 ) {
					const char* archName = argv[++i];
					if ( archName == NULL )
						throw "-arch missing architecture name";
					bool found = false;
					for (const ArchInfo* t=archInfoArray; t->archName != NULL; ++t) {
						if ( strcmp(t->archName,archName) == 0 ) {
							sPreferredArch = t->cpuType;
							if ( t->isSubType )
								sPreferredSubArch = t->cpuSubType;
							found = true;
						}
					}
					if ( !found )
						throwf("unknown architecture %s", archName);
				}
				else if ( strcmp(arg, "-only") == 0 ) {
					sMatchName = ++i<argc? argv[i]: NULL;
				}
				else if ( strcmp(arg, "-align") == 0 ) {
					sPrintRestrict = true;
					sPrintAlign = true;
				}
				else if ( strcmp(arg, "-name") == 0 ) {
					sPrintRestrict = true;
					sPrintName = true;
				}
				else {
					usage();
					throwf("unknown option: %s\n", arg);
				}
			}
			else {
				ld::relocatable::File* reader = createReader(arg);
				dumpFile(reader);
			}
		}
	}
	catch (const char* msg) {
		fprintf(stderr, "ObjDump failed: %s\n", msg);
		return 1;
	}
	
	return 0;
}
