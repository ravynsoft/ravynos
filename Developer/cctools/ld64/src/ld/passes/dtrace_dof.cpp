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
#include <unordered_map>
#include <unordered_set>

#include "ld.hpp"
#include "MachOFileAbstraction.hpp"
#include "dtrace_dof.h"

// prototype for entry point in libdtrace.dylib
typedef uint8_t* (*createdof_func_t)(cpu_type_t, unsigned int, const char*[], unsigned int, const char*[], const char*[], uint64_t offsetsInDOF[], size_t* size);


namespace ld {
namespace passes {
namespace dtrace {

class File; // forward reference

class Atom : public ld::Atom {
public:
											Atom(class File& f, const char* n,  const uint8_t* content, uint64_t sz);

	virtual ld::File*						file() const					{ return (ld::File*)&_file; }
	virtual const char*						name() const					{ return _name; }
	virtual uint64_t						size() const					{ return _size; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const
																			{ memcpy(buffer, _content, _size); }
	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return &_fixups[0]; }
	virtual ld::Fixup::iterator				fixupsEnd() const				{ return &_fixups[_fixups.size()]; }

protected:
	friend class File;
	virtual									~Atom() {}
	
	class File&								_file;
	const char*								_name;
	const uint8_t*							_content;
	uint64_t								_size;
	mutable std::vector<ld::Fixup>			_fixups;
};


class File : public ld::File 
{
public:
								File(const char* segmentName, const char* sectionName, const char* pth, 
									const uint8_t fileContent[], uint64_t fileLength, Ordinal ord, 
									const char* symbolName="dof")
									: ld::File(pth, 0, ord, Other),
									  _atom(*this, symbolName, fileContent, fileLength), 
									  _section(segmentName, sectionName, ld::Section::typeDtraceDOF) { }
	virtual						~File() {}
	
	virtual bool				forEachAtom(AtomHandler& h) const { h.doAtom(_atom); return true; }
	virtual bool				justInTimeforEachAtom(const char* name, AtomHandler&) const { return false; }

	void						reserveFixups(unsigned int count) { _atom._fixups.reserve(count); }
	void						addSectionFixup(const ld::Fixup& f) { _atom._fixups.push_back(f); }
	ld::Atom&					atom() { return _atom; }
private:
	friend class Atom;
	
	Atom						_atom;
	ld::Section					_section;
};

Atom::Atom(File& f, const char* n,  const uint8_t* content, uint64_t sz)
	: ld::Atom(f._section, ld::Atom::definitionRegular, ld::Atom::combineNever,
		ld::Atom::scopeTranslationUnit, ld::Atom::typeUnclassified, 
		symbolTableNotIn, false, false, false, ld::Atom::Alignment(0)), 
		_file(f), _name(strdup(n)), _content(content), _size(sz) {}



struct DTraceProbeInfo {
	DTraceProbeInfo(const ld::Atom* a, uint32_t o, const char* n) : atom(a), offset(o), probeName(n) {}
	const ld::Atom*					atom;
	uint32_t						offset;
	const char*						probeName;
};
typedef std::unordered_map<const char*, std::vector<DTraceProbeInfo>, CStringHash, CStringEquals>	ProviderToProbes;
typedef	std::unordered_set<const char*, CStringHash, CStringEquals>  CStringSet;



void doPass(const Options& opts, ld::Internal& internal)
{
	static bool log = false;
	
	// only make __dof section in final linked images
	if ( opts.outputKind() == Options::kObjectFile )
		return;
	
	// skip making __dof section if command line option said not to
	if ( ! opts.generateDtraceDOF() )
		return;

	// scan all atoms looking for dtrace probes
	std::vector<DTraceProbeInfo>					probeSites;
	std::vector<DTraceProbeInfo>					isEnabledSites;
	std::map<const ld::Atom*,CStringSet>			atomToDtraceTypes;
	for (std::vector<ld::Internal::FinalSection*>::iterator sit=internal.sections.begin(); sit != internal.sections.end(); ++sit) {
		ld::Internal::FinalSection* sect = *sit;
		if ( sect->type() != ld::Section::typeCode ) 
			continue;
		for (std::vector<const ld::Atom*>::iterator ait=sect->atoms.begin();  ait != sect->atoms.end(); ++ait) {
			const ld::Atom* atom = *ait;
			for (ld::Fixup::iterator fit = atom->fixupsBegin(), end=atom->fixupsEnd(); fit != end; ++fit) {
				switch ( fit->kind ) {
					case ld::Fixup::kindStoreX86DtraceCallSiteNop:
					case ld::Fixup::kindStoreARMDtraceCallSiteNop:
					case ld::Fixup::kindStoreThumbDtraceCallSiteNop:
					case ld::Fixup::kindStoreARM64DtraceCallSiteNop:
						probeSites.push_back(DTraceProbeInfo(atom, fit->offsetInAtom, fit->u.name));
						break;
					case ld::Fixup::kindStoreX86DtraceIsEnableSiteClear:
					case ld::Fixup::kindStoreARMDtraceIsEnableSiteClear:
					case ld::Fixup::kindStoreThumbDtraceIsEnableSiteClear:
					case ld::Fixup::kindStoreARM64DtraceIsEnableSiteClear:
						isEnabledSites.push_back(DTraceProbeInfo(atom, fit->offsetInAtom, fit->u.name));
						break;
					case ld::Fixup::kindDtraceExtra:
						atomToDtraceTypes[atom].insert(fit->u.name);
						break;
					default:
						break;
				}
			}
		}
	}

	// if no probes, we're done
	if ( (probeSites.size() == 0) && (isEnabledSites.size() == 0) ) 
		return;
	
	ld::Fixup::Kind storeKind = ld::Fixup::kindNone;
	switch ( opts.architecture() ) {
		case CPU_TYPE_I386:
		case CPU_TYPE_X86_64:
		case CPU_TYPE_ARM:
		case CPU_TYPE_ARM64:
#if SUPPORT_ARCH_arm64_32
		case CPU_TYPE_ARM64_32:
#endif
			storeKind = ld::Fixup::kindStoreLittleEndian32;
			break;
		default:
			throw "unsupported arch for DOF";
	}
	
	// partition probes by provider name
	// The symbol names looks like:
	//	"___dtrace_probe$" provider-name "$" probe-name [ "$"... ]
	//	"___dtrace_isenabled$" provider-name "$" probe-name [ "$"... ]
	ProviderToProbes providerToProbes;
	std::vector<DTraceProbeInfo> emptyList;
	for(std::vector<DTraceProbeInfo>::iterator it = probeSites.begin(); it != probeSites.end(); ++it) {
		// ignore probes in functions that were coalesed away rdar://problem/5628149
		if ( it->atom->coalescedAway() )
			continue;
		const char* providerStart = &it->probeName[16];
		const char* providerEnd = strchr(providerStart, '$');
		if ( providerEnd != NULL ) {
			char providerName[providerEnd-providerStart+1];
			strlcpy(providerName, providerStart, providerEnd-providerStart+1);
			ProviderToProbes::iterator pos = providerToProbes.find(providerName);
			if ( pos == providerToProbes.end() ) {
				const char* dup = strdup(providerName);
				providerToProbes[dup] = emptyList;
			}
			providerToProbes[providerName].push_back(*it);
		}
	}
	for(std::vector<DTraceProbeInfo>::iterator it = isEnabledSites.begin(); it != isEnabledSites.end(); ++it) {
		// ignore probes in functions that were coalesed away rdar://problem/5628149
		if ( it->atom->coalescedAway() )
			continue;
		const char* providerStart = &it->probeName[20];
		const char* providerEnd = strchr(providerStart, '$');
		if ( providerEnd != NULL ) {
			char providerName[providerEnd-providerStart+1];
			strlcpy(providerName, providerStart, providerEnd-providerStart+1);
			ProviderToProbes::iterator pos = providerToProbes.find(providerName);
			if ( pos == providerToProbes.end() ) {
				const char* dup = strdup(providerName);
				providerToProbes[dup] = emptyList;
			}
			providerToProbes[providerName].push_back(*it);
		}
	}
	
	// create a DOF section for each provider
	int dofIndex=1;
	CStringSet sectionNamesUsed;
	for(ProviderToProbes::iterator pit = providerToProbes.begin(); pit != providerToProbes.end(); ++pit, ++dofIndex) {
		const char* providerName = pit->first;
		const std::vector<DTraceProbeInfo>& probes = pit->second;

		// open library and find dtrace_create_dof()
		void* handle = dlopen("/usr/lib/libdtrace.dylib", RTLD_LAZY);
		if ( handle == NULL )
			throwf("couldn't dlopen() /usr/lib/libdtrace.dylib: %s", dlerror());
		createdof_func_t pCreateDOF = (createdof_func_t)dlsym(handle, "dtrace_ld_create_dof");
		if ( pCreateDOF == NULL )
			throwf("couldn't find \"dtrace_ld_create_dof\" in /usr/lib/libdtrace.dylib: %s", dlerror());
		// build list of typedefs/stability infos for this provider
		CStringSet types;
		for(std::vector<DTraceProbeInfo>::const_iterator it = probes.begin(); it != probes.end(); ++it) {
			std::map<const ld::Atom*,CStringSet>::iterator pos = atomToDtraceTypes.find(it->atom);
			if ( pos != atomToDtraceTypes.end() ) {
				for(CStringSet::iterator sit = pos->second.begin(); sit != pos->second.end(); ++sit) {
					const char* providerStart = strchr(*sit, '$')+1;
					const char* providerEnd = strchr(providerStart, '$');
					if ( providerEnd != NULL ) {
						char aProviderName[providerEnd-providerStart+1];
						strlcpy(aProviderName, providerStart, providerEnd-providerStart+1);
						if ( strcmp(aProviderName, providerName) == 0 )
							types.insert(*sit);
					}
				}
			}
		}
		int typeCount = types.size();
		const char* typeNames[typeCount];
		//fprintf(stderr, "types for %s:\n", providerName);
		uint32_t index = 0;
		for(CStringSet::iterator it = types.begin(); it != types.end(); ++it) {
			typeNames[index] = *it;
			//fprintf(stderr, "\t%s\n", *it);
			++index;
		}
		
		// build list of probe/isenabled sites
		const uint32_t probeCount = probes.size();
		const char* probeNames[probeCount];
		const char* funtionNames[probeCount];
		uint64_t offsetsInDOF[probeCount];
		index = 0;
		for(std::vector<DTraceProbeInfo>::const_iterator it = probes.begin(); it != probes.end(); ++it) {
			probeNames[index] = it->probeName;
			funtionNames[index] = it->atom->name();
			offsetsInDOF[index] = 0;
			++index;
		}
		if ( log ) {
			fprintf(stderr, "calling libtrace to create DOF:\n");
			fprintf(stderr, "   types::\n");
			for(int i=0; i < typeCount; ++i) 
				fprintf(stderr, "     [%u]\t %s\n", i, typeNames[i]);
			fprintf(stderr, "   probes::\n");
			for(uint32_t i=0; i < probeCount; ++i) 
				fprintf(stderr, "     [%u]\t %s in %s\n", i, probeNames[i], funtionNames[i]);
		}
		
		// call dtrace library to create DOF section
		size_t dofSectionSize;
		uint8_t* p = (*pCreateDOF)(opts.architecture(), typeCount, typeNames, probeCount, probeNames, funtionNames, offsetsInDOF, &dofSectionSize);
		if ( p != NULL ) {
			char* sectionName = new char[18]; // alloc new string, pass ownership to File()
			strcpy(sectionName, "__dof_");
			strlcpy(&sectionName[6], providerName, 10);
			// create unique section name so each DOF is in its own section
			if ( sectionNamesUsed.count(sectionName) != 0 ) {
				sectionName[15] = '0';
				sectionName[16] = '\0';
				while ( sectionNamesUsed.count(sectionName) != 0 ) {
					++sectionName[15];
				}
			}
			sectionNamesUsed.insert(sectionName);
			char symbolName[strlen(providerName)+64];
			sprintf(symbolName, "__dtrace_dof_for_provider_%s", providerName);
			File* f = new File("__TEXT", sectionName, "dtrace", p, dofSectionSize, ld::File::Ordinal::NullOrdinal(), symbolName);
			if ( log ) {
				fprintf(stderr, "libdtrace created DOF of size %ld\n", dofSectionSize);
			}
			// add references
			f->reserveFixups(3*probeCount);
			for (uint32_t i=0; i < probeCount; ++i) {
				uint64_t offset = offsetsInDOF[i];
				//fprintf(stderr, "%s offset[%d]=0x%08llX\n", providerName, i, offset);
				if ( offset > dofSectionSize )
					throwf("offsetsInDOF[%d]=%0llX > dofSectionSize=%0lX\n", i, offset, dofSectionSize);
				f->addSectionFixup(ld::Fixup(offset, ld::Fixup::k1of4, ld::Fixup::kindSetTargetAddress, probes[i].atom));
				f->addSectionFixup(ld::Fixup(offset, ld::Fixup::k2of4, ld::Fixup::kindAddAddend, probes[i].offset));
				f->addSectionFixup(ld::Fixup(offset, ld::Fixup::k3of4, ld::Fixup::kindSubtractTargetAddress, &f->atom()));
				f->addSectionFixup(ld::Fixup(offset, ld::Fixup::k4of4, storeKind));
			}
			// insert new section
			internal.addAtom(f->atom());
		}
		else {
			throw "error creating dtrace DOF section";
		}
	}
	


}


} // namespace dtrace
} // namespace passes 
} // namespace ld 
