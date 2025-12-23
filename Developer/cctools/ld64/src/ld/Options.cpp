/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2005-2011 Apple Inc. All rights reserved.
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
#include <mach/vm_prot.h>
#ifdef __linux__
#include <algorithm>
#include <limits.h>
#define PATH_MAX 1024
#define MAXPATHLEN PATH_MAX
#define CPU_SUBTYPE_X86_ALL 3
namespace {
	typedef long double max_align_t;
}
#else
#include <sys/sysctl.h>
#endif
#include <mach-o/dyld.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <spawn.h>
#include <cxxabi.h>
#include <Availability.h>
#include <tapi/tapi.h>

#include <vector>
#include <map>
#include <sstream>

#include "ld.hpp"
#include "Options.h"
#include "Architectures.hpp"
#include "MachOFileAbstraction.hpp"
#include "Snapshot.h"
#include "macho_relocatable_file.h"
#include "ResponseFiles.h"

// from FunctionNameDemangle.h
extern "C" size_t fnd_get_demangled_name(const char *mangledName, char *outputBuffer, size_t length);

#define VAL(x) #x
#define STRINGIFY(x) VAL(x)

// upward dependency on lto::version()
namespace lto {
	extern const char* version();
	extern unsigned static_api_version();
	extern unsigned runtime_api_version();
}

// magic to place command line in crash reports
const int crashreporterBufferSize = 2000;
static char crashreporterBuffer[crashreporterBufferSize];
#if 0 //__MAC_OS_X_VERSION_MIN_REQUIRED >= 1070
	#include <CrashReporterClient.h>
	// hack until ld does not need to build on 10.6 anymore
    struct crashreporter_annotations_t gCRAnnotations 
        __attribute__((section("__DATA," CRASHREPORTER_ANNOTATIONS_SECTION))) 
        = { CRASHREPORTER_ANNOTATIONS_VERSION, 0, 0, 0, 0, 0, 0 };
#else
	extern "C" char* __crashreporter_info__;
	__attribute__((used)) 
	char* __crashreporter_info__ = crashreporterBuffer;
#endif


static bool			sEmitWarnings = true;
static bool			sFatalWarnings = false;
static const char*	sWarningsSideFilePath = NULL;
static FILE*		sWarningsSideFile = NULL;
static int			sWarningsCount = 0;

void warning(const char* format, ...)
{
	++sWarningsCount;
	if ( sEmitWarnings ) {
		va_list	list;
		if ( sWarningsSideFilePath != NULL ) {
			if ( sWarningsSideFile == NULL )
				sWarningsSideFile = fopen(sWarningsSideFilePath, "a");
		}
		va_start(list, format);
		fprintf(stderr, "ld: warning: ");
		vfprintf(stderr, format, list);
		fprintf(stderr, "\n");
		if ( sWarningsSideFile != NULL ) {
			fprintf(sWarningsSideFile, "ld: warning: ");
			vfprintf(sWarningsSideFile, format, list);
			fprintf(sWarningsSideFile, "\n");
			fflush(sWarningsSideFile);
		}
		va_end(list);
	}
}

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


bool Options::FileInfo::checkFileExists(const Options& options, const char *p)
{
	if (isInlined) {
		modTime = 0;
		return true;
	}
	struct stat statBuffer;
	if (p == NULL) 
	  p = path;
	if ( stat(p, &statBuffer) == 0 ) {
		if (p != path) path = strdup(p);
		modTime = statBuffer.st_mtime;
		return true;
	}
	options.addDependency(Options::depNotFound, p);
//	fprintf(stderr, "not found: %s\n", p);
    return false;
}


Options::Options(int argc, const char* argv[])
	: fOutputFile("a.out"), fArchitecture(0), fSubArchitecture(0),
	  fFallbackArchitecture(0), fFallbackSubArchitecture(0), fArchitectureName("unknown"), fOutputKind(kDynamicExecutable),
	  fHasPreferredSubType(false), fArchSupportsThumb2(false), fBindAtLoad(false), fKeepPrivateExterns(false),
	  fIgnoreOtherArchFiles(false), fErrorOnOtherArchFiles(false), fForceSubtypeAll(false),
	  fInterposeMode(kInterposeNone), fDeadStrip(false), fNameSpace(kTwoLevelNameSpace),
	  fDylibCompatVersion(0), fDylibCurrentVersion(0), fDylibInstallName(NULL), fFinalName(NULL), fEntryName(NULL),
	  fBaseAddress(0), fMaxAddress(0xFFFFFFFFFFFFFFFFULL),
	  fBaseWritableAddress(0),
	  fExportMode(kExportDefault), fLibrarySearchMode(kSearchDylibAndArchiveInEachDir),
	  fUndefinedTreatment(kUndefinedError), fMessagesPrefixedWithArchitecture(true), 
	  fWeakReferenceMismatchTreatment(kWeakReferenceMismatchNonWeak),
	  fClientName(NULL),
	  fUmbrellaName(NULL), fInitFunctionName(NULL), fDotOutputFile(NULL), fExecutablePath(NULL),
	  fBundleLoader(NULL), fDtraceScriptName(NULL), fMapPath(NULL),
	  fDyldInstallPath("/usr/lib/dyld"), fLtoCachePath(NULL), fTempLtoObjectPath(NULL), fOverridePathlibLTO(NULL), fLtoCpu(NULL),
	  fKextObjectsEnable(-1),fKextObjectsDirPath(NULL),fToolchainPath(NULL),fOrderFilePath(NULL),
	  fZeroPageSize(ULLONG_MAX), fStackSize(0), fStackAddr(0), fSourceVersion(0), fSDKVersion(0), fExecutableStack(false), 
	  fNonExecutableHeap(false), fDisableNonExecutableHeap(false),
	  fMinimumHeaderPad(32), fSegmentAlignment(LD_PAGE_SIZE),
	  fCommonsMode(kCommonsIgnoreDylibs),  fUUIDMode(kUUIDContent), fLocalSymbolHandling(kLocalSymbolsAll), fWarnCommons(false), 
	  fVerbose(false), fKeepRelocations(false), fWarnStabs(false),
	  fTraceDylibSearching(false), fPause(false), fStatistics(false), fPrintOptions(false),
	  fSharedRegionEligible(false), fSharedRegionEligibleForceOff(false), fPrintOrderFileStatistics(false),
	  fReadOnlyx86Stubs(false), fPositionIndependentExecutable(false), fPIEOnCommandLine(false),
	  fDisablePositionIndependentExecutable(false), fMaxMinimumHeaderPad(false),
	  fDeadStripDylibs(false),  fAllowTextRelocs(false), fWarnTextRelocs(false), fKextsUseStubs(false),
	  fEncryptable(true), fEncryptableForceOn(false), fEncryptableForceOff(false),
	  fOrderData(true), fMarkDeadStrippableDylib(false),
	  fMakeCompressedDyldInfo(true), fMakeCompressedDyldInfoForceOff(false),
	  fMakeThreadedStartsSection(false), fNoEHLabels(false),
	  fAllowCpuSubtypeMismatches(false), fEnforceDylibSubtypesMatch(false),
	  fWarnOnSwiftABIVersionMismatches(false), fUseSimplifiedDylibReExports(false),
	  fObjCABIVersion2Override(false), fObjCABIVersion1Override(false), fCanUseUpwardDylib(false),
	  fFullyLoadArchives(false), fLoadAllObjcObjectsFromArchives(false), fFlatNamespace(false),
	  fLinkingMainExecutable(false), fForFinalLinkedImage(false), fForStatic(false),
	  fForDyld(false), fMakeTentativeDefinitionsReal(false), fWhyLoad(false), fRootSafe(false),
	  fSetuidSafe(false), fImplicitlyLinkPublicDylibs(true), fAddCompactUnwindEncoding(true),
	  fWarnCompactUnwind(false), fRemoveDwarfUnwindIfCompactExists(false),
	  fAutoOrderInitializers(true), fOptimizeZeroFill(true), fMergeZeroFill(false), fLogObjectFiles(false),
	  fLogAllFiles(false), fTraceDylibs(false), fTraceIndirectDylibs(false), fTraceArchives(false), fTraceEmitJSON(false),
	  fOutputSlidable(false), fWarnWeakExports(false), fNoWeakExports(false),
	  fObjcGcCompaction(false), fObjCGc(false), fObjCGcOnly(false), 
	  fDemangle(false), fTLVSupport(false), 
	  fVersionLoadCommand(false), fVersionLoadCommandForcedOn(false), 
	  fVersionLoadCommandForcedOff(false), fForceLegacyVersionLoadCommands(false), fFunctionStartsLoadCommand(false),
	  fFunctionStartsForcedOn(false), fFunctionStartsForcedOff(false),
	  fDataInCodeInfoLoadCommand(false), fDataInCodeInfoLoadCommandForcedOn(false), fDataInCodeInfoLoadCommandForcedOff(false),
	  fCanReExportSymbols(false), fObjcCategoryMerging(true), fPageAlignDataAtoms(false), 
	  fNeedsThreadLoadCommand(false), fEntryPointLoadCommand(false),
	  fSourceVersionLoadCommand(false),
	  fSourceVersionLoadCommandForceOn(false), fSourceVersionLoadCommandForceOff(false), 
	  fExportDynamic(false), fAbsoluteSymbols(false),
	  fAllowSimulatorToLinkWithMacOSX(false), fSimulatorSupportDylib(false), fKeepDwarfUnwind(true),
	  fKeepDwarfUnwindForcedOn(false), fKeepDwarfUnwindForcedOff(false),
	  fVerboseOptimizationHints(false), fIgnoreOptimizationHints(false),
	  fGenerateDtraceDOF(true), fAllowBranchIslands(true), fTraceSymbolLayout(false), 
	  fMarkAppExtensionSafe(false), fCheckAppExtensionSafe(false), fForceLoadSwiftLibs(false),
	  fSharedRegionEncodingV2(false), fUseDataConstSegment(false),
	  fUseDataConstSegmentForceOn(false), fUseDataConstSegmentForceOff(false), fUseTextExecSegment(false),
	  fBundleBitcode(false), fHideSymbols(false), fVerifyBitcode(false),
	  fReverseMapUUIDRename(false), fDeDupe(true), fVerboseDeDupe(false), fMakeInitializersIntoOffsets(false),
	  fUseLinkedListBinding(false), fMakeChainedFixups(false), fMakeChainedFixupsSection(false), fNoLazyBinding(false), fDebugVariant(false),
	  fReverseMapPath(NULL), fLTOCodegenOnly(false),
	  fIgnoreAutoLink(false), fAllowDeadDups(false), fAllowWeakImports(true), fInitializersTreatment(Options::kInvalid),
	  fZeroModTimeInDebugMap(false), fBitcodeKind(kBitcodeProcess),
	  fDebugInfoStripping(kDebugInfoMinimal), fTraceOutputFile(NULL), fPlatfromVersionCmdFound(false), fInternalSDK(false),
	  fWarnUnusedDylibs(false), fWarnUnusedDylibsForceOn(false), fWarnUnusedDylibsForceOff(false), fAdHocSign(false),
	  fPlatformMismatchesAreWarning(false),
	  fForceObjCRelativeMethodListsOn(false), fForceObjCRelativeMethodListsOff(false), fUseObjCRelativeMethodLists(false),
	  fSaveTempFiles(false), fLinkSnapshot(this), fSnapshotRequested(false), fPipelineFifo(NULL),
	  fDependencyInfoPath(NULL), fBuildContextName(NULL), fTraceFileDescriptor(-1), fMaxDefaultCommonAlign(0),
	  fUnalignedPointerTreatment(kUnalignedPointerIgnore), fPreferTAPIFile(false), fOSOPrefixPath(NULL)
{
	this->expandResponseFiles(argc, argv);
	this->checkForClassic(argc, argv);
	this->parsePreCommandLineEnvironmentSettings();
	this->parse(argc, argv);
	this->parsePostCommandLineEnvironmentSettings();
	this->reconfigureDefaults();
	this->checkIllegalOptionCombinations();
	
	this->addDependency(depOutputFile, fOutputFile);
	if ( fMapPath != NULL )
		this->addDependency(depOutputFile, fMapPath);
}

Options::~Options()
{
	if ( fTraceFileDescriptor != -1 )
		::close(fTraceFileDescriptor);
}

bool Options::errorBecauseOfWarnings() const
{
	return (sFatalWarnings && (sWarningsCount > 0));
}


const char*	Options::installPath() const
{
	if ( fDylibInstallName != NULL )
		return fDylibInstallName;
	else if ( fFinalName != NULL )
		return fFinalName;
	else
		return fOutputFile;
}


bool Options::interposable(const char* name) const
{
	switch ( fInterposeMode ) {
		case kInterposeNone:
			return false;
		case kInterposeAllExternal:
			return true;
		case kInterposeSome:
			return fInterposeList.contains(name);
	}
	throw "internal error";
}


bool Options::printWhyLive(const char* symbolName) const
{
	return fWhyLive.contains(symbolName);
}


const char*	Options::dotOutputFile()
{
	return fDotOutputFile;
}


bool Options::hasWildCardExportRestrictList() const
{
	// has -exported_symbols_list which contains some wildcards
	return ((fExportMode == kExportSome) && fExportSymbols.hasWildCards());
}

bool Options::hasWeakBitTweaks() const
{
	// has -exported_symbols_list which contains some wildcards
	return (!fForceWeakSymbols.empty() || !fForceNotWeakSymbols.empty());
}

bool Options::allGlobalsAreDeadStripRoots() const
{
	// -exported_symbols_list means globals are not exported by default
	if ( fExportMode == kExportSome )
		return false;
	//
	switch ( fOutputKind ) {
		case Options::kDynamicExecutable:
			// <rdar://problem/12839986> Add the -export_dynamic flag 
			return fExportDynamic;
		case Options::kStaticExecutable:
			// <rdar://problem/13361218> Support the -export_dynamic flag for xnu
			return fExportDynamic;
		case Options::kPreload:
			// by default unused globals in a main executable are stripped
			return false;
		case Options::kDynamicLibrary:
		case Options::kDynamicBundle:
		case Options::kObjectFile:
		case Options::kDyld:
		case Options::kKextBundle:
			return true;
	}
	return false;
}

bool Options::dyldLoadsOutput() const
{
	switch ( fOutputKind ) {
		case Options::kStaticExecutable:
		case Options::kPreload:
		case Options::kObjectFile:
		case Options::kKextBundle:
			return false;
		case Options::kDynamicExecutable:
		case Options::kDynamicLibrary:
		case Options::kDynamicBundle:
		case Options::kDyld:
			return true;
	}

}

bool Options::dyldOrKernelLoadsOutput() const
{
	switch ( fOutputKind ) {
		case Options::kStaticExecutable:
			return fKernel;
		case Options::kPreload:
		case Options::kObjectFile:
			return false;
		case Options::kDynamicExecutable:
		case Options::kDynamicLibrary:
		case Options::kDynamicBundle:
		case Options::kDyld:
		case Options::kKextBundle:
			return true;
	}

}

bool Options::keepRelocations()
{
	return fKeepRelocations;
}

bool Options::warnStabs()
{
	return fWarnStabs;
}

const char* Options::executablePath()
{
	return fExecutablePath;
}

uint32_t Options::initialSegProtection(const char* segName) const
{
	for(std::vector<Options::SegmentProtect>::const_iterator it = fCustomSegmentProtections.begin(); it != fCustomSegmentProtections.end(); ++it) {
		if ( strcmp(it->name, segName) == 0 ) {
			return it->init;
		}
	}
	if ( strcmp(segName, "__TEXT") == 0 ) {
		return ( fUseTextExecSegment ? VM_PROT_READ : (VM_PROT_READ | VM_PROT_EXECUTE) );
	}
	else if ( strcmp(segName, "__TEXT_EXEC") == 0 ) {
		return VM_PROT_READ | VM_PROT_EXECUTE;
	}
	else if ( strcmp(segName, "__PAGEZERO") == 0 ) {
		return 0;
	}
	else if ( strcmp(segName, "__LINKEDIT") == 0 ) {
		return VM_PROT_READ;
	}

	// all others default to read-write
	return VM_PROT_READ | VM_PROT_WRITE;
}

bool Options::readOnlyDataSegment(const char* name) const
{
	// set SG_READ_ONLY on segment
	// but not for shared cache dylibs (because __objc_const is not read-only)
	return ( fUseDataConstSegment && (strcmp(name, "__DATA_CONST") == 0) && (!fSharedRegionEligible || (fOutputKind == Options::kDyld)) );
}

uint32_t Options::maxSegProtection(const char* segName) const
{
	// allow macOS binaries to set alternate max-prot
	if ( fArchitecture == CPU_TYPE_X86_64 ) {
		for (const Options::SegmentProtect& info : fCustomSegmentProtections) {
			if ( strcmp(info.name, segName) == 0 ) {
				return info.max;
			}
		}
	}
	// i386 uses text-relocations so, need max prot to be rwx.  All others have max==init
	if ( fArchitecture != CPU_TYPE_I386 )
		return initialSegProtection(segName);

	if ( strcmp(segName, "__PAGEZERO") == 0 ) {
		return 0;
	}
	// all others default to all
	return VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE;
}
	
uint64_t Options::segPageSize(const char* segName) const
{
	for(std::vector<SegmentSize>::const_iterator it=fCustomSegmentSizes.begin(); it != fCustomSegmentSizes.end(); ++it) {
		if ( strcmp(it->name, segName) == 0 )
			return it->size;
	}
	return fSegmentAlignment;
}

uint64_t Options::customSegmentAddress(const char* segName) const
{
	for(std::vector<SegmentStart>::const_iterator it=fCustomSegmentAddresses.begin(); it != fCustomSegmentAddresses.end(); ++it) {
		if ( strcmp(it->name, segName) == 0 )
			return it->address;
	}
	// if custom stack in use, model as segment with custom address
	if ( (fStackSize != 0) && (strcmp("__UNIXSTACK", segName) == 0) )
		return fStackAddr - fStackSize;
	return 0;
}

bool Options::hasCustomSegmentAddress(const char* segName) const
{
	for(std::vector<SegmentStart>::const_iterator it=fCustomSegmentAddresses.begin(); it != fCustomSegmentAddresses.end(); ++it) {
		if ( strcmp(it->name, segName) == 0 )
			return true;
	}
	// if custom stack in use, model as segment with custom address
	if ( (fStackSize != 0) && (strcmp("__UNIXSTACK", segName) == 0) )
		return true;
	return false;
}

bool Options::hasCustomSectionAlignment(const char* segName, const char* sectName) const
{
	for (std::vector<SectionAlignment>::const_iterator it =	fSectionAlignments.begin(); it != fSectionAlignments.end(); ++it) {
		if ( (strcmp(it->segmentName, segName) == 0) && (strcmp(it->sectionName, sectName) == 0) )
			return true;
	}
	if ( fEncryptable && (strcmp(sectName, "__oslogstring") == 0) && (strcmp(segName, "__TEXT") == 0) )
		return true;

	return false;
}

uint8_t Options::customSectionAlignment(const char* segName, const char* sectName) const
{
	for (std::vector<SectionAlignment>::const_iterator it =	fSectionAlignments.begin(); it != fSectionAlignments.end(); ++it) {
		if ( (strcmp(it->segmentName, segName) == 0) && (strcmp(it->sectionName, sectName) == 0) )
			return it->alignment;
	}
	if ( fEncryptable && (strcmp(sectName, "__oslogstring") == 0) && (strcmp(segName, "__TEXT") == 0) )
		return __builtin_ctz(fSegmentAlignment);

	return 0;
}

bool Options::segmentOrderAfterFixedAddressSegment(const char* segName) const
{
	bool nowPinned = false;
	for (std::vector<const char*>::const_iterator it=fSegmentOrder.begin(); it != fSegmentOrder.end(); ++it) {
		if ( strcmp(*it, segName) == 0 )
			return nowPinned;
		if ( hasCustomSegmentAddress(*it) )
			nowPinned = true;
	}
	return false;
}

bool Options::hasExportedSymbolOrder()
{
	return (fExportSymbolsOrder.size() > 0);
}

bool Options::exportedSymbolOrder(const char* sym, unsigned int* order) const
{
	NameToOrder::const_iterator pos = fExportSymbolsOrder.find(sym);
	if ( pos != fExportSymbolsOrder.end() ) {
		*order = pos->second;
		return true;
	}
	else {
		*order = 0xFFFFFFFF;
		return false;
	}
}

void Options::loadSymbolOrderFile(const char* fileOfExports, NameToOrder& orderMapping)
{
	// read in whole file
	int fd = ::open(fileOfExports, O_RDONLY, 0);
	if ( fd == -1 )
		throwf("can't open -exported_symbols_order file: %s", fileOfExports);
	struct stat stat_buf;
	::fstat(fd, &stat_buf);
	char* p = (char*)malloc(stat_buf.st_size);
	if ( p == NULL )
		throwf("can't process -exported_symbols_order file: %s", fileOfExports);

	if ( read(fd, p, stat_buf.st_size) != stat_buf.st_size )
		throwf("can't read -exported_symbols_order file: %s", fileOfExports);

	::close(fd);

	// parse into symbols and add to unordered_set
	unsigned int count = 0;
	char * const end = &p[stat_buf.st_size];
	enum { lineStart, inSymbol, inComment } state = lineStart;
	char* symbolStart = NULL;
	for (char* s = p; s < end; ++s ) {
		switch ( state ) {
		case lineStart:
			if ( *s =='#' ) {
				state = inComment;
			}
			else if ( !isspace(*s) ) {
				state = inSymbol;
				symbolStart = s;
			}
			break;
		case inSymbol:
			if ( (*s == '\n') || (*s == '\r') ) {
				*s = '\0';
				// removing any trailing spaces
				char* last = s-1;
				while ( isspace(*last) ) {
					*last = '\0';
					--last;
				}
				orderMapping[symbolStart] = ++count;
				symbolStart = NULL;
				state = lineStart;
			}
			break;
		case inComment:
			if ( (*s == '\n') || (*s == '\r') )
				state = lineStart;
			break;
		}
	}
	if ( state == inSymbol ) {
		warning("missing line-end at end of file \"%s\"", fileOfExports);
		int len = end-symbolStart+1;
		char* temp = new char[len];
		strlcpy(temp, symbolStart, len);

		// remove any trailing spaces
		char* last = &temp[len-2];
		while ( isspace(*last) ) {
			*last = '\0';
			--last;
		}
		orderMapping[temp] = ++count;
	}

	// Note: we do not free() the malloc buffer, because the strings are used by the export-set hash table
}

void Options::loadImplictZipperFile(const char *path, std::vector<const char*>& paths)
{
	// read in whole file
	int fd = ::open(path, O_RDONLY, 0);
	if ( fd == -1 ) return; // Early exist if the file is not present
	struct stat stat_buf;
	::fstat(fd, &stat_buf);
	char* p = (char*)malloc(stat_buf.st_size);
	if ( p == NULL )
		throwf("can't process auto zipper file: %s", path);

	if ( read(fd, p, stat_buf.st_size) != stat_buf.st_size )
		throwf("can't process auto zipper file: %s", path);

	::close(fd);

	// parse into paths and add to a vector
	char * const end = &p[stat_buf.st_size];
	enum { lineStart, inInstallname, inComment } state = lineStart;
	char* installnameStart = NULL;
	for (char* s = p; s < end; ++s ) {
		switch ( state ) {
			case lineStart:
				if ( *s =='#' ) {
					state = inComment;
				}
				else if ( !isspace(*s) ) {
					state = inInstallname;
					installnameStart = s;
				}
				break;
			case inInstallname:
				if ( (*s == '\n') || (*s == '\r') ) {
					*s = '\0';
					// removing any trailing spaces
					char* last = s-1;
					while ( isspace(*last) ) {
						*last = '\0';
						--last;
					}
					paths.push_back(installnameStart);
					state = lineStart;
				}
				break;
			case inComment:
				if ( (*s == '\n') || (*s == '\r') )
					state = lineStart;
				break;
		}
	}
	if ( state == inInstallname ) {
		warning("missing line-end at end of file \"%s\"", path);
		int len = end-installnameStart+1;
		char* temp = new char[len];
		strlcpy(temp, installnameStart, len);

		// remove any trailing spaces
		char* last = &temp[len-2];
		while ( isspace(*last) ) {
			*last = '\0';
			--last;
		}
		paths.push_back(installnameStart);
	}

	// Note: we do not free() the malloc buffer, because the strings are returned out via paths
}

bool Options::forceWeak(const char* symbolName) const
{
	return fForceWeakSymbols.contains(symbolName);
}

bool Options::forceNotWeak(const char* symbolName) const
{
	return fForceNotWeakSymbols.contains(symbolName);
}

bool Options::forceWeakNonWildCard(const char* symbolName) const
{
	return fForceWeakSymbols.containsNonWildcard(symbolName);
}

bool Options::forceNotWeakNonWildcard(const char* symbolName) const
{
	return fForceNotWeakSymbols.containsNonWildcard(symbolName);
}

bool Options::forceCoalesce(const char* symbolName) const
{
	return fForceCoalesceSymbols.contains(symbolName);
}


bool Options::shouldExport(const char* symbolName) const
{
	switch (fExportMode) {
		case kExportSome:
			return fExportSymbols.contains(symbolName);
		case kDontExportSome:
			return ! fDontExportSymbols.contains(symbolName);
		case kExportDefault:
			return true;
	}
	throw "internal error";
}

bool Options::shouldReExport(const char* symbolName) const
{
	return fReExportSymbols.contains(symbolName);
}

bool Options::keepLocalSymbol(const char* symbolName) const
{
	switch (fLocalSymbolHandling) {
		case kLocalSymbolsAll:
			return true;
		case kLocalSymbolsNone:
			return false;
		case kLocalSymbolsSelectiveInclude:
			return fLocalSymbolsIncluded.contains(symbolName);
		case kLocalSymbolsSelectiveExclude:
			return ! fLocalSymbolsExcluded.contains(symbolName);
	}
	throw "internal error";
}

const std::vector<const char*>* Options::sectionOrder(const char* segName) const
{ 
	for (std::vector<SectionOrderList>::const_iterator it=fSectionOrder.begin(); it != fSectionOrder.end(); ++it) {
		if ( strcmp(it->segmentName, segName) == 0 )
			return &it->sectionOrder;
	}
	return NULL;
}

void Options::setInferredArch(cpu_type_t type, cpu_subtype_t subtype)
{
	assert(fArchitecture == 0);
	for (const ArchInfo* t=archInfoArray; t->archName != NULL; ++t) {
		if ( (type == t->cpuType) && (subtype == t->cpuSubType) ) {
			fArchitecture        = type;
			fSubArchitecture     = subtype;
			fArchitectureName    = t->archName;
			fHasPreferredSubType = t->isSubType;
			fArchSupportsThumb2  = t->supportsThumb2;
#if SUPPORT_ARCH_arm64e
			if ( (fArchitecture == CPU_TYPE_ARM64) && (fSubArchitecture == CPU_SUBTYPE_ARM64E) ) {
				fSupportsAuthenticatedPointers = true;
			}
#endif
			break;
		}
	}
	if ( fArchitecture == 0 ) {
		// some arch not in our table
		fArchitecture        = type;
		fSubArchitecture     = subtype;
		fArchitectureName    = "unknown";
	}

	fLinkSnapshot.recordArch(fArchitectureName);
}

bool Options::armUsesZeroCostExceptions() const
{
	return ( (fArchitecture == CPU_TYPE_ARM) && (fSubArchitecture == CPU_SUBTYPE_ARM_V7K) );
}

void Options::selectFallbackArch(const char *arch)
{
	// Should have the format "desired_arch:fallback_arch", for example "arm64_32:armv7k" to allow an armv7k
	// slice to substitute for arm64_32 if the latter isn't present.
	if (const char* fallbackEnv = getenv("LD_DYLIB_ARCH_FALLBACK") ) {
		std::string fallback(fallbackEnv);
		auto delimPos = fallback.find(':');

		// Check we've got a potentially valid fallback string and that it's this architecture we're falling back from.
		if ( delimPos == std::string::npos || fallback.substr(0, delimPos) != arch )
			return;

		std::string fallbackTo = fallback.substr(delimPos + 1);
		for (const ArchInfo *t = archInfoArray; t->archName != nullptr; ++t) {
			if ( fallbackTo == t->archName ) {
				fFallbackArchitecture = t->cpuType;
				fFallbackSubArchitecture = t->cpuSubType;
			}
		}
	}
	else {
		// <rdar://problem/39797337> let x86_64h fallback and use x86_64 slice
		if ( (fArchitecture == CPU_TYPE_X86_64) && (fSubArchitecture == CPU_SUBTYPE_X86_64_H) ) {
			fFallbackArchitecture    = CPU_TYPE_X86_64;
			fFallbackSubArchitecture = CPU_SUBTYPE_X86_ALL;
		}
	}
}

void Options::parseArch(const char* arch)
{
	if ( arch == NULL )
		throw "-arch must be followed by an architecture string";
	for (const ArchInfo* t=archInfoArray; t->archName != NULL; ++t) {
		if ( strcmp(t->archName,arch) == 0 ) {
			fArchitectureName = arch;
			fArchitecture = t->cpuType;
			fSubArchitecture = t->cpuSubType;
			fHasPreferredSubType = t->isSubType;
			fArchSupportsThumb2 = t->supportsThumb2;
			selectFallbackArch(arch);
			return;
		}
	}
	throwf("unknown/unsupported architecture name for: -arch %s", arch);
}

bool Options::checkForFile(const char* format, const char* dir, const char* rootName, FileInfo& result) const
{
	char possiblePath[strlen(dir)+strlen(rootName)+strlen(format)+8];
	sprintf(possiblePath, format,  dir, rootName);
	bool found = result.checkFileExists(*this, possiblePath);
	if ( fTraceDylibSearching )
		printf("[Logging for XBS]%sfound library: '%s'\n", (found ? " " : " not "), possiblePath);
	return found;
}


Options::FileInfo Options::findLibrary(const char* rootName, bool dylibsOnly) const
{
	FileInfo result;
	const int rootNameLen = strlen(rootName);
	// if rootName ends in .o there is no .a vs .dylib choice
	if ( (rootNameLen > 3) && (strcmp(&rootName[rootNameLen-2], ".o") == 0) ) {
		for (std::vector<const char*>::const_iterator it = fLibrarySearchPaths.begin();
			 it != fLibrarySearchPaths.end();
			 it++) {
			const char* dir = *it;
			if ( checkForFile("%s/%s", dir, rootName, result) )
				return result;
		}
	}
	else {
		bool lookForDylibs = false;
		switch ( fOutputKind ) {
			case Options::kDynamicExecutable:
			case Options::kDynamicLibrary:
			case Options::kDynamicBundle:
			case Options::kObjectFile:  // <rdar://problem/15914513> 
				lookForDylibs = true;
				break;
			case Options::kStaticExecutable:
			case Options::kDyld:
			case Options::kPreload:
			case Options::kKextBundle:
				lookForDylibs = false;
				break;
		}
		switch ( fLibrarySearchMode ) {
		case kSearchAllDirsForDylibsThenAllDirsForArchives:
				// first look in all directories for just for dylibs
				if ( lookForDylibs ) {
					for (std::vector<const char*>::const_iterator it = fLibrarySearchPaths.begin();
						 it != fLibrarySearchPaths.end();
						 it++) {
						const char* dir = *it;
						auto path = std::string(dir) + "/lib" + rootName + ".dylib";
						if ( findFile(path, {".tbd"}, result) )
							return result;
					}
					for (std::vector<const char*>::const_iterator it = fLibrarySearchPaths.begin();
						 it != fLibrarySearchPaths.end();
						 it++) {
						const char* dir = *it;
						if ( checkForFile("%s/lib%s.so", dir, rootName, result) )
							return result;
					}
				}
				// next look in all directories for just for archives
				if ( !dylibsOnly ) {
					for (std::vector<const char*>::const_iterator it = fLibrarySearchPaths.begin();
						 it != fLibrarySearchPaths.end();
						 it++) {
						const char* dir = *it;
						if ( checkForFile("%s/lib%s.a", dir, rootName, result) )
							return result;
					}
				}
				break;

			case kSearchDylibAndArchiveInEachDir:
				// look in each directory for just for a dylib then for an archive
				for (std::vector<const char*>::const_iterator it = fLibrarySearchPaths.begin();
					 it != fLibrarySearchPaths.end();
					 it++) {
					const char* dir = *it;
					auto path = std::string(dir) + "/lib" + rootName + ".dylib";
					if ( lookForDylibs && findFile(path, {".tbd"}, result) )
						return result;
					if ( lookForDylibs && checkForFile("%s/lib%s.so", dir, rootName, result) )
						return result;
					if ( !dylibsOnly && checkForFile("%s/lib%s.a", dir, rootName, result) )
						return result;
				}
				break;
		}
	}
	throwf("library not found for -l%s", rootName);
}

Options::FileInfo Options::findFramework(const char* frameworkName) const
{
	if ( frameworkName == NULL )
		throw "-framework missing next argument";
	char temp[strlen(frameworkName)+1];
	strcpy(temp, frameworkName);
	const char* name = temp;
	const char* suffix = NULL;
	char* comma = strchr(temp, ',');
	if ( comma != NULL ) {
		*comma = '\0';
		suffix = &comma[1];
	}
	return findFramework(name, suffix);
}

Options::FileInfo Options::findFramework(const char* rootName, const char* suffix) const
{
	for (const auto* path : fFrameworkSearchPaths) {
		auto possiblePath = std::string(path).append("/").append(rootName).append(".framework/").append(rootName);
		if ( suffix != nullptr ) {
			char realPath[PATH_MAX];
			// no symlink in framework to suffix variants, so follow main symlink
			if ( realpath(possiblePath.c_str(), realPath) != nullptr )
				possiblePath = std::string(realPath).append(suffix);
		}
        FileInfo result;
		if ( findFile(possiblePath, {".tbd"}, result) )
			return result;
	}
	// try without suffix
	if ( suffix != NULL )
		return findFramework(rootName, NULL);
	else
		throwf("framework not found %s", rootName);
}

static std::string replace_extension(const std::string &path, const std::string &ext)
{
	auto result = path;
	auto lastSlashIdx = result.find_last_of('/');
	auto lastDotIdx = result.find_last_of('.');
	if (lastDotIdx != std::string::npos && lastDotIdx > lastSlashIdx)
		result.erase(lastDotIdx, std::string::npos);
	if ( ext.size() > 0 && ext[0] == '.' )
		result.append(ext);
	else
		result.append('.' + ext);
	return result;
}

void Options::addTAPIInterface(tapi::LinkerInterfaceFile* interface, const char *path) const {
#if ((TAPI_API_VERSION_MAJOR == 1 &&  TAPI_API_VERSION_MINOR >= 3) || (TAPI_API_VERSION_MAJOR > 1))
	if (tapi::APIVersion::isAtLeast(1, 3)) {
		for (auto &name : interface->inlinedFrameworkNames()) {
			fTAPIFiles.emplace_back(interface, path, name.c_str());
		}
	}
#endif
}

bool Options::findFile(const std::string &path, const std::vector<std::string> &tbdExtensions, FileInfo& result) const
{
	FileInfo tbdInfo;
	for ( const auto &ext : tbdExtensions ) {
		auto newPath = replace_extension(path, ext);
		bool found = tbdInfo.checkFileExists(*this, newPath.c_str());
		if ( fTraceDylibSearching )
			printf("[Logging for XBS]%sfound library: '%s'\n", (found ? " " : " not "), newPath.c_str());
		if ( found )
			break;
	}

	FileInfo dylibInfo;
	{
		bool found = dylibInfo.checkFileExists(*this, path.c_str());
		if ( fTraceDylibSearching )
			printf("[Logging for XBS]%sfound library: '%s'\n", (found ? " " : " not "), path.c_str());
	}

	// There is only a text-based stub file or a dynamic library file.
	if ( tbdInfo.missing() != dylibInfo.missing() ) {
		result = tbdInfo.missing() ? dylibInfo : tbdInfo;
	}
	// There are both - a text-based stub file and a dynamic library file.
	else if ( !tbdInfo.missing() && !dylibInfo.missing() ) {
		// Check if we should prefer the text-based stub file (env var).
		if (fPreferTAPIFile) {
			result = tbdInfo;
		}
		// Check if we should prefer the text-based stub file (installapi).
		else if (tapi::LinkerInterfaceFile::shouldPreferTextBasedStubFile(tbdInfo.path)) {
			result = tbdInfo;
		}
		// If the files are still in sync we can use and should use the text-based stub file.
		else if (tapi::LinkerInterfaceFile::areEquivalent(tbdInfo.path, dylibInfo.path)) {
			result = tbdInfo;
		}
		// Otherwise issue a warning and fall-back to the dynamic library file.
		else {
			// <rdar://problem/48850374> Suppress warnings about iOSHostAdditions until mastering is fixed (rdar://46486148)
			if ( strstr(tbdInfo.path, "/SDKs/iOSHostAdditions") == NULL )
				warning("text-based stub file %s and library file %s are out of sync. Falling back to library file for linking.", tbdInfo.path, dylibInfo.path);
			result = dylibInfo;
		}
	} else {
		return false;
	}
	return true;
}

static bool startsWith(const std::string& str, const std::string& prefix)
{
	return (str.compare(0, prefix.length(), prefix) == 0);
}

static std::string getDirPath(const std::string& path)
{
	std::string::size_type lastSlashPos = path.find_last_of('/');
	if ( lastSlashPos == std::string::npos )
		return "./";
	else
		return path.substr(0, lastSlashPos+1);
}

Options::FileInfo Options::findFile(const std::string &path, const ld::dylib::File* fromDylib) const
{
	FileInfo result;

	// if absolute path and not a .o file, then use SDK prefix
	if ( (path[0] == '/') && (strcmp(&path[path.size()-2], ".o") != 0) ) {
		for (const auto* sdkPathDir : fSDKPaths) {
			auto possiblePath = std::string(sdkPathDir) + path;
			if ( findFile(possiblePath, {".tbd"}, result) )
				return result;
		}
	}

	// expand @ variables
	if ( path[0] == '@' ) {
		if ( startsWith(path, "@executable_path/") && (fExecutablePath != nullptr) ) {
			std::string exeBasedPath = getDirPath(fExecutablePath) + &path[17];
			if ( findFile(exeBasedPath, {".tbd"}, result) )
				return result;
		}
		else if ( startsWith(path, "@loader_path/") && (fromDylib != nullptr) ) {
			char absPath[PATH_MAX];
			if ( realpath(fromDylib->path(), absPath) != NULL ) {
				std::string loaderBasedPath = getDirPath(fromDylib->path()) + &path[13];
				if ( findFile(loaderBasedPath, {".tbd"}, result) )
					return result;
			}
		}
		else if ( startsWith(path, "@rpath/") ) {
			// first search any LC_RPATH supplied by dyld that re-exports dylib to be found
			if ( fromDylib != nullptr ) {
				for (const char* rp : fromDylib->rpaths() ) {
					std::string rpath = rp;
					// handle dylib that has LC_RPATH = @loader_path/blah
					if ( startsWith(rpath, "@loader_path/") ) {
						char absPath[PATH_MAX];
						if ( realpath(fromDylib->path(), absPath) != NULL )
							rpath = getDirPath(absPath) + &rpath[13];
						else
							rpath = getDirPath(fromDylib->path()) + &rpath[13];
					}
					std::string rpathBasedPath = rpath + "/" + &path[6];
					if ( findFile(rpathBasedPath, {".tbd"}, result) )
						return result;
				}
			}
		}
	}

	// find inlined TBD file before raw path.
	// rdar://problem/35864452
	if (hasInlinedTAPIFile(path)) {
		FileInfo inlinedFile(path.c_str());
		inlinedFile.isInlined = true;
		return inlinedFile;
	}

	// try raw path
	if ( findFile(path, {".tbd"}, result) )
		return result;

	// not found
	throwf("file not found: %s", path.c_str());
}

bool Options::hasInlinedTAPIFile(const std::string &path) const {
	for (const auto &dylib : fTAPIFiles) {
		if (dylib.getInstallName() == path)
			return true;
	}
	return false;
}

tapi::LinkerInterfaceFile* Options::findTAPIFile(const std::string &path) const
{
#if ((TAPI_API_VERSION_MAJOR == 1 &&  TAPI_API_VERSION_MINOR >= 3) || (TAPI_API_VERSION_MAJOR > 1))
	tapi::LinkerInterfaceFile* interface = nullptr;
	std::string TBDPath;
	
	// create parsing options.
	tapi::ParsingFlags flags = tapi::ParsingFlags::None;
	if (enforceDylibSubtypesMatch())
		flags |= tapi::ParsingFlags::ExactCpuSubType;
	
	if (!allowWeakImports())
		flags |= tapi::ParsingFlags::DisallowWeakImports;

	__block uint32_t linkMinOSVersion = 0;
	//FIXME handle this correctly once we have multi-platform TAPI.
	platforms().forEach(^(ld::Platform platform, uint32_t minVersion, uint32_t sdkVersion, bool &stop) {
		if (linkMinOSVersion == 0)
			linkMinOSVersion = minVersion;
		if (platform == ld::Platform::macOS)
			linkMinOSVersion = minVersion;
	});

	// Search through all the inlined framework.
	for (const auto &dylib : fTAPIFiles) {
		if (dylib.getInstallName() == path) {
			// If the install name matches, parse the framework.
			std::string errorMessage;
			auto file = dylib.getInterfaceFile()->getInlinedFramework(path.c_str(), architecture(), subArchitecture(),
																	  flags, tapi::PackedVersion32(linkMinOSVersion), errorMessage);
			if (!file)
				throw strdup(errorMessage.c_str());

			if (!interface) {
				// If this is the first inlined framework found, record the information.
				interface = file;
				TBDPath = dylib.getTAPIFilePath();
			} else {
				// If we found other inlined framework already, check to see if their versions are the same.
				// If not the same, emit an warning and record the newer one. Otherwise, just use the current one.
				if (interface->getCurrentVersion() == file->getCurrentVersion())
					continue;
				warning("Inlined framework/dylib mismatch: %s (%s and %s)", path.c_str(),
						TBDPath.c_str(), dylib.getTAPIFilePath().c_str());
				if (interface->getCurrentVersion() < file->getCurrentVersion()) {
					interface = file;
					TBDPath = dylib.getTAPIFilePath();
				}
			}
		}
	}
	return interface;
#else
	return nullptr;
#endif
}

// search for indirect dylib first using -F and -L paths first
Options::FileInfo Options::findIndirectDylib(const std::string& installName, const ld::dylib::File* fromDylib) const
{
	FileInfo result;

	auto lastSlashPos = installName.find_last_of('/');
	auto pos = ( lastSlashPos != std::string::npos ) ? lastSlashPos + 1 : 0;
	auto leafName = installName.substr(pos);

	// Is this in a framework?
	// /path/Foo.framework/Foo							==> true (Foo)
	// /path/Foo.framework/Frameworks/Bar.framework/Bar ==> true (Bar)
	// /path/Foo.framework/Resources/Bar				==> false
	bool isFramework = false;
	if ( lastSlashPos != std::string::npos ) {
		auto frameworkDir = std::string("/").append(leafName).append(".framework/");
		if ( installName.rfind(frameworkDir) != std::string::npos )
			isFramework = true;
	}
	
	// These are abbreviated versions of the routines findFramework and findLibrary above
	// because we already know the final name of the file that we're looking for and so
	// don't need to try variations, just paths. We do need to add the additional bits
	// onto the framework path though.
	if ( isFramework ) {
		auto endPos = installName.rfind(".framework");
		auto beginPos = installName.find_last_of('/', endPos);
		auto leafPath = installName.substr(beginPos);
		for (const auto* dir : fFrameworkSearchPaths) {
			auto possiblePath = dir + leafPath;
			if ( findFile(possiblePath, {".tbd"}, result) )
				return result;
		}
	} else {
		// if this is a .dylib inside a framework, do not search -L paths
		// <rdar://problem/5427952> ld64's re-export cycle detection logic prevents use of X11 libGL on Leopard
		bool embeddedDylib = ( (leafName.size() > 6)
					&& (leafName.find(".dylib", leafName.size()-6) != std::string::npos)
					&& (installName.find(".framework/") != std::string::npos) );
		if ( !embeddedDylib ) {
			for (const auto* dir : fLibrarySearchPaths) {
				//fprintf(stderr,"Finding Library: %s/%s\n", dir, leafName);
				std::string possiblePath = dir + std::string("/") + leafName;
				if ( findFile(possiblePath, {".tbd"}, result) )
					return result;
			}
		}
	}

	// If we didn't find it fall back to findFile.
	return findFile(installName, fromDylib);
}

void Options::loadFileList(const char* fileOfPaths, ld::File::Ordinal baseOrdinal)
{
	FILE* file;
	const char* comma = strrchr(fileOfPaths, ',');
	const char* prefix = NULL;
	if ( comma != NULL ) {
		// <rdar://problem/5907981> -filelist fails with comma in path
		file = fopen(fileOfPaths, "r");
		if ( file == NULL ) {
			prefix = comma+1;
			int realFileOfPathsLen = comma-fileOfPaths;
			char realFileOfPaths[realFileOfPathsLen+1];
			strncpy(realFileOfPaths,fileOfPaths, realFileOfPathsLen);
			realFileOfPaths[realFileOfPathsLen] = '\0';
			file = fopen(realFileOfPaths, "r");
			if ( file == NULL )
				throwf("-filelist file '%s' could not be opened, errno=%d (%s)\n", realFileOfPaths, errno, strerror(errno));
			this->addDependency(Options::depFileList, realFileOfPaths);
		}
	}
	else {
		file = fopen(fileOfPaths, "r");
		if ( file == NULL )
			throwf("-filelist file '%s' could not be opened, errno=%d (%s)\n", fileOfPaths, errno, strerror(errno));
		this->addDependency(Options::depFileList, fileOfPaths);
	}

	char path[PATH_MAX];
	ld::File::Ordinal previousOrdinal = baseOrdinal;
	while ( fgets(path, PATH_MAX, file) != NULL ) {
		path[PATH_MAX-1] = '\0';
		char* eol = strchr(path, '\n');
		if ( eol != NULL )
			*eol = '\0';
		if ( prefix != NULL ) {
			char builtPath[strlen(prefix)+strlen(path)+2];
			strcpy(builtPath, prefix);
			strcat(builtPath, "/");
			strcat(builtPath, path);
           if (fPipelineFifo != NULL) {
			   FileInfo info = FileInfo(builtPath);
			   info.ordinal = previousOrdinal.nextFileListOrdinal();
			   previousOrdinal = info.ordinal;
			   info.fromFileList = true;
              fInputFiles.push_back(info);
           } else {
			   FileInfo info = findFile(builtPath);
			   info.ordinal = previousOrdinal.nextFileListOrdinal();
			   previousOrdinal = info.ordinal;
			   info.fromFileList = true;
			   fInputFiles.push_back(info);
           }
		}
		else {
           if (fPipelineFifo != NULL) {
			   FileInfo info = FileInfo(path);
			   info.ordinal = previousOrdinal.nextFileListOrdinal();
			   previousOrdinal = info.ordinal;
			   info.fromFileList = true;
			   fInputFiles.push_back(info);
           } else {
			   FileInfo info = findFile(path);
			   info.ordinal = previousOrdinal.nextFileListOrdinal();
			   previousOrdinal = info.ordinal;
			   info.fromFileList = true;
			   fInputFiles.push_back(info);
           }
		}
	}
	fclose(file);
}


void Options::SetWithWildcards::remove(const NameSet& toBeRemoved)
{
	for(NameSet::const_iterator it=toBeRemoved.begin(); it != toBeRemoved.end(); ++it) {
		const char* symbolName = *it;
		NameSet::iterator pos = fRegular.find(symbolName);
		if ( pos != fRegular.end() )
			fRegular.erase(pos);
	}
}

bool Options::SetWithWildcards::hasWildCards(const char* symbol) 
{
	// an exported symbol name containing *, ?, or [ requires wildcard matching
	return ( strpbrk(symbol, "*?[") != NULL );
}

void Options::SetWithWildcards::insert(const char* symbol, SymbolMatchingMode match_mode)
{
	if ( match_mode == kAllowWildcards && hasWildCards(symbol) )
		fWildCard.push_back(symbol);
	else
		fRegular.insert(symbol);
}

bool Options::SetWithWildcards::contains(const char* symbol, bool* matchBecauseOfWildcard) const
{
	if ( matchBecauseOfWildcard != NULL )
		*matchBecauseOfWildcard = false;
	// first look at hash table on non-wildcard symbols
	if ( fRegular.find(symbol) != fRegular.end() )
		return true;
	// next walk list of wild card symbols looking for a match
	for(std::vector<const char*>::const_iterator it = fWildCard.begin(); it != fWildCard.end(); ++it) {
		if ( wildCardMatch(*it, symbol) ) {
			if ( matchBecauseOfWildcard != NULL )
				*matchBecauseOfWildcard = true;
			return true;
		}
	}
	return false;
}

// Support "foo.o:_bar" to mean symbol _bar in file foo.o
bool Options::SetWithWildcards::containsWithPrefix(const char* symbol, const char* file, bool& wildCardMatch) const
{
	wildCardMatch = false;
	if ( contains(symbol, &wildCardMatch) )
		return true;
	if ( file == NULL )
		return false;
	const char* s = strrchr(file, '/');
	if ( s != NULL )
		file = s+1;
	char buff[strlen(file)+strlen(symbol)+2];
	sprintf(buff, "%s:%s", file, symbol);
	return contains(buff, &wildCardMatch);
}

bool Options::SetWithWildcards::containsNonWildcard(const char* symbol) const
{
	// look at hash table on non-wildcard symbols
	return ( fRegular.find(symbol) != fRegular.end() );
}


std::vector<const char*> Options::exportsData() const
{
	return fExportSymbols.data();
}


std::vector<const char*> Options::SetWithWildcards::data() const
{
	std::vector<const char*> data;
	for (NameSet::const_iterator it=regularBegin(); it != regularEnd(); ++it) {
		data.push_back(*it);
	}
	for (std::vector<const char*>::const_iterator it=fWildCard.begin(); it != fWildCard.end(); ++it) {
		data.push_back(*it);
	}
	return data;
}

bool Options::SetWithWildcards::inCharRange(const char*& p, unsigned char c) const
{
	++p; // find end
	const char* b = p;
	while ( *p != '\0' ) {
		if ( *p == ']') {
			const char* e = p;
			// found beginining [ and ending ]
			unsigned char last = '\0';
			for ( const char* s = b; s < e; ++s ) {
				if ( *s == '-' ) {
					unsigned char next = *(++s);
					if ( (last <= c) && (c <= next) )
						return true;
					++s;
				}
				else {
					if ( *s == c )
						return true;
					last = *s;
				}
			}
			return false;
		}
		++p;
	}
	return false;
}

bool Options::SetWithWildcards::wildCardMatch(const char* pattern, const char* symbol) const
{
	const char* s = symbol;
	for (const char* p = pattern; *p != '\0'; ++p) {
		switch ( *p ) {
			case '*':
				if ( p[1] == '\0' )
					return true;
				for (const char* t = s; *t != '\0'; ++t) {
					if ( wildCardMatch(&p[1], t) )
						return true;
				}
				return false;
			case '?':
				if ( *s == '\0' )
					return false;
				++s;
				break;
			case '[':
				if ( ! inCharRange(p, *s) )
					return false;
				++s;
				break;
			default:
				if ( *s != *p )
					return false;
				++s;
		}
	}
	return (*s == '\0');
}


void Options::loadExportFile(const char* fileOfExports, const char* option, SetWithWildcards& set, SymbolMatchingMode match_mode)
{
	if ( fileOfExports == NULL )
		throwf("missing file after %s", option);
	// read in whole file
	int fd = ::open(fileOfExports, O_RDONLY, 0);
	if ( fd == -1 )
		throwf("can't open %s file: %s", option, fileOfExports);
	struct stat stat_buf;
	::fstat(fd, &stat_buf);
	char* p = (char*)malloc(stat_buf.st_size);
	if ( p == NULL )
		throwf("can't process %s file: %s", option, fileOfExports);

	if ( read(fd, p, stat_buf.st_size) != stat_buf.st_size )
		throwf("can't read %s file: %s", option, fileOfExports);

	this->addDependency(Options::depMisc, fileOfExports);

	::close(fd);

	// parse into symbols and add to unordered_set
	char * const end = &p[stat_buf.st_size];
	enum { lineStart, inSymbol, inComment } state = lineStart;
	char* symbolStart = NULL;
	for (char* s = p; s < end; ++s ) {
		switch ( state ) {
		case lineStart:
			if ( *s =='#' ) {
				state = inComment;
			}
			else if ( !isspace(*s) ) {
				state = inSymbol;
				symbolStart = s;
			}
			break;
		case inSymbol:
			if ( (*s == '\n') || (*s == '\r') ) {
				*s = '\0';
				// removing any trailing spaces
				char* last = s-1;
				while ( isspace(*last) ) {
					*last = '\0';
					--last;
				}
				set.insert(symbolStart, match_mode);
				symbolStart = NULL;
				state = lineStart;
			}
			break;
		case inComment:
			if ( (*s == '\n') || (*s == '\r') )
				state = lineStart;
			break;
		}
	}
	if ( state == inSymbol ) {
		warning("missing line-end at end of file \"%s\"", fileOfExports);
		int len = end-symbolStart+1;
		char* temp = new char[len];
		strlcpy(temp, symbolStart, len);

		// remove any trailing spaces
		char* last = &temp[len-2];
		while ( isspace(*last) ) {
			*last = '\0';
			--last;
		}
		set.insert(temp, match_mode);
	}

	// Note: we do not free() the malloc buffer, because the strings are used by the export-set hash table
}

void Options::parseAliasFile(const char* fileOfAliases)
{
	// read in whole file
	int fd = ::open(fileOfAliases, O_RDONLY, 0);
	if ( fd == -1 )
		throwf("can't open alias file: %s", fileOfAliases);
	struct stat stat_buf;
	::fstat(fd, &stat_buf);
	char* p = (char*)malloc(stat_buf.st_size+1);
	if ( p == NULL )
		throwf("can't process alias file: %s", fileOfAliases);

	if ( read(fd, p, stat_buf.st_size) != stat_buf.st_size )
		throwf("can't read alias file: %s", fileOfAliases);
	p[stat_buf.st_size] = '\n';
	::close(fd);
	this->addDependency(Options::depMisc, fileOfAliases);

	// parse into symbols and add to fAliases
	AliasPair pair;
	char * const end = &p[stat_buf.st_size+1];
	enum { lineStart, inRealName, inBetween, inAliasName, inComment } state = lineStart;
	int lineNumber = 1;
	for (char* s = p; s < end; ++s ) {
		switch ( state ) {
		case lineStart:
			if ( *s =='#' ) {
				state = inComment;
			}
			else if ( !isspace(*s) ) {
				state = inRealName;
				pair.realName = s;
			}
			break;
		case inRealName:
			if ( *s == '\n' ) {
				warning("line needs two symbols but has only one at line #%d in \"%s\"", lineNumber, fileOfAliases);
				++lineNumber;
				state = lineStart;
			}
			else if ( isspace(*s) ) {
				*s = '\0';
				state = inBetween;
			}
			break;
		case inBetween:
			if ( *s == '\n' ) {
				warning("line needs two symbols but has only one at line #%d in \"%s\"", lineNumber, fileOfAliases);
				++lineNumber;
				state = lineStart;
			}
			else if ( ! isspace(*s) ) {
				state = inAliasName;
				pair.alias = s;
			}
			break;
		case inAliasName:
			if ( *s =='#' ) {
				*s = '\0';
				// removing any trailing spaces
				char* last = s-1;
				while ( isspace(*last) ) {
					*last = '\0';
					--last;
				}
				fAliases.push_back(pair);
				state = inComment;
			}
			else if ( *s == '\n' ) {
				*s = '\0';
				// removing any trailing spaces
				char* last = s-1;
				while ( isspace(*last) ) {
					*last = '\0';
					--last;
				}
				fAliases.push_back(pair);
				state = lineStart;
			}
			break;
		case inComment:
			if ( *s == '\n' )
				state = lineStart;
			break;
		}
	}

	// Note: we do not free() the malloc buffer, because the strings therein are used by fAliases
}



void Options::setUndefinedTreatment(const char* treatment)
{
	if ( treatment == NULL )
		throw "-undefined missing [ warning | error | suppress | dynamic_lookup ]";

	if ( strcmp(treatment, "warning") == 0 )
		fUndefinedTreatment = kUndefinedWarning;
	else if ( strcmp(treatment, "error") == 0 )
		fUndefinedTreatment = kUndefinedError;
	else if ( strcmp(treatment, "suppress") == 0 )
		fUndefinedTreatment = kUndefinedSuppress;
	else if ( strcmp(treatment, "dynamic_lookup") == 0 )
		fUndefinedTreatment = kUndefinedDynamicLookup;
	else
		throw "invalid option to -undefined [ warning | error | suppress | dynamic_lookup ]";
}

Options::Treatment Options::parseTreatment(const char* treatment)
{
	if ( treatment == NULL )
		return kNULL;

	if ( strcmp(treatment, "warning") == 0 )
		return kWarning;
	else if ( strcmp(treatment, "error") == 0 )
		return kError;
	else if ( strcmp(treatment, "suppress") == 0 )
		return kSuppress;
	else
		return kInvalid;
}

void Options::setWeakReferenceMismatchTreatment(const char* treatment)
{
	if ( treatment == NULL )
		throw "-weak_reference_mismatches missing [ error | weak | non-weak ]";

	if ( strcmp(treatment, "error") == 0 )
		fWeakReferenceMismatchTreatment = kWeakReferenceMismatchError;
	else if ( strcmp(treatment, "weak") == 0 )
		fWeakReferenceMismatchTreatment = kWeakReferenceMismatchWeak;
	else if ( strcmp(treatment, "non-weak") == 0 )
		fWeakReferenceMismatchTreatment = kWeakReferenceMismatchNonWeak;
	else
		throw "invalid option to -weak_reference_mismatches [ error | weak | non-weak ]";
}

Options::CommonsMode Options::parseCommonsTreatment(const char* mode)
{
	if ( mode == NULL )
		throw "-commons missing [ ignore_dylibs | use_dylibs | error ]";

	if ( strcmp(mode, "ignore_dylibs") == 0 )
		return kCommonsIgnoreDylibs;
	else if ( strcmp(mode, "use_dylibs") == 0 )
		return kCommonsOverriddenByDylibs;
	else if ( strcmp(mode, "error") == 0 )
		return kCommonsConflictsDylibsError;
	else
		throw "invalid option to -commons [ ignore_dylibs | use_dylibs | error ]";
}

void Options::addDylibOverride(const char* paths)
{
	if ( paths == NULL )
		throw "-dylib_file must followed by two colon separated paths";
	const char* colon = strchr(paths, ':');
	if ( colon == NULL )
		throw "-dylib_file must followed by two colon separated paths";
	int len = colon-paths;
	char* target = new char[len+2];
	strncpy(target, paths, len);
	target[len] = '\0';
	DylibOverride entry;
	entry.installName = target;
	entry.useInstead = &colon[1];	
	fDylibOverrides.push_back(entry);
}

uint64_t Options::parseAddress(const char* addr)
{
	char* endptr;
	uint64_t result = strtoull(addr, &endptr, 16);
	return result;
}

uint32_t Options::parseProtection(const char* prot)
{
	uint32_t result = 0;
	for(const char* p = prot; *p != '\0'; ++p) {
		switch(tolower(*p)) {
			case 'r':
				result |= VM_PROT_READ;
				break;
			case 'w':
				result |= VM_PROT_WRITE;
				break;
			case 'x':
				result |= VM_PROT_EXECUTE;
				break;
			case '-':
				break;
			default:
				throwf("unknown -segprot letter in %s", prot);
		}
	}
	return result;
}


//
// Parses number of form A[.B[.B[.D[.E]]]] into a uint64_t where the bits are a24.b10.c10.d10.e10
//
uint64_t Options::parseVersionNumber64(const char* versionString)
{
	uint64_t a = 0;
	uint64_t b = 0;
	uint64_t c = 0;
	uint64_t d = 0;
	uint64_t e = 0;
	char* end;
	a = strtoul(versionString, &end, 10);
	if ( *end == '.' ) {
		b = strtoul(&end[1], &end, 10);
		if ( *end == '.' ) {
			c = strtoul(&end[1], &end, 10);
			if ( *end == '.' ) {
				d = strtoul(&end[1], &end, 10);
				if ( *end == '.' ) {
					e = strtoul(&end[1], &end, 10);
				}
			}
		}
	}
	if ( (*end != '\0') || (a > 0xFFFFFF) || (b > 0x3FF) || (c > 0x3FF) || (d > 0x3FF)  || (e > 0x3FF) )
		throwf("malformed 64-bit a.b.c.d.e version number: %s", versionString);

	return (a << 40) | ( b << 30 ) | ( c << 20 ) | ( d << 10 ) | e;
}


uint32_t Options::currentVersion32() const
{
	// warn if it does not fit into 32 bit vers number
	uint32_t a = (fDylibCurrentVersion >> 40) & 0xFFFF;
	uint32_t b = (fDylibCurrentVersion >> 30) & 0xFF;
	uint32_t c = (fDylibCurrentVersion >> 20) & 0xFF;
	uint64_t rep32 = ((uint64_t)a << 40) |  ((uint64_t)b << 30) | ((uint64_t)c << 20);
	if ( rep32 != fDylibCurrentVersion ) {
		warning("truncating -current_version to fit in 32-bit space used by old mach-o format");
		a = (fDylibCurrentVersion >> 40) & 0xFFFFFF;		
		if ( a > 0xFFFF )
			a = 0xFFFF;
		b = (fDylibCurrentVersion >> 30) & 0x3FF;
		if ( b > 0xFF )
			b = 0xFF;
		c = (fDylibCurrentVersion >> 20) & 0x3FF;
		if ( c > 0xFF )
			c = 0xFF;
	}
	return (a << 16) | ( b << 8 ) | c;
}

//
// Parses number of form X[.Y[.Z]] into a uint32_t where the nibbles are xxxx.yy.zz
//
uint32_t Options::parseVersionNumber32(const char* versionString)
{
	uint32_t x = 0;
	uint32_t y = 0;
	uint32_t z = 0;
	char* end;
	x = strtoul(versionString, &end, 10);
	if ( *end == '.' ) {
		y = strtoul(&end[1], &end, 10);
		if ( *end == '.' ) {
			z = strtoul(&end[1], &end, 10);
		}
	}
	if ( (*end != '\0') || (x > 0xffff) || (y > 0xff) || (z > 0xff) )
		throwf("malformed 32-bit x.y.z version number: %s", versionString);

	return (x << 16) | ( y << 8 ) | z;
}

static const char* cstringSymbolName(const char* orderFileString)
{
	char* result;
	asprintf(&result, "cstring=%s", orderFileString);
	// convert escaped characters
	char* d = result;
	for(const char* s=result; *s != '\0'; ++s, ++d) {
		if ( *s == '\\' ) {
			++s;
			switch ( *s ) {
				case 'n':
					*d = '\n';
					break;
				case 't':
					*d = '\t';
					break;
				case 'v':
					*d = '\v';
					break;
				case 'b':
					*d = '\b';
					break;
				case 'r':
					*d = '\r';
					break;
				case 'f':
					*d = '\f';
					break;
				case 'a':
					*d = '\a';
					break;
				case '\\':
					*d = '\\';
					break;
				case '?':
					*d = '\?';
					break;
				case '\'':
					*d = '\r';
					break;
				case '\"':
					*d = '\"';
					break;
				case 'x':
					// hexadecimal value of char
					{
						++s;
						char value = 0;
						while ( isxdigit(*s) ) {
							value *= 16;
							if ( isdigit(*s) )
								value += (*s-'0');
							else
								value += ((toupper(*s)-'A') + 10);
							++s;
						}
						*d = value;
					}
					break;
				default:
					if ( isdigit(*s) ) {
						// octal value of char
						char value = 0;
						while ( isdigit(*s) ) {
							value = (value << 3) + (*s-'0');
							++s;
						}
						*d = value;
					}
			}
		}
		else {
			*d = *s;
		}
	}
	*d = '\0';
	return result;
}

void Options::parseOrderFile(const char* path, bool cstring)
{
	// order files override auto-ordering
	fAutoOrderInitializers = false;

	// <rdar://problem/24856050> ld64 should prefer OrderFiles from the SDK over the ones in /
	for (const char* sdkPath : fSDKPaths) {
		char fullPath[PATH_MAX];
		strlcpy(fullPath, sdkPath, PATH_MAX);
		strlcat(fullPath, "/", PATH_MAX);
		strlcat(fullPath, path, PATH_MAX);
		struct stat statBuffer;
		if ( stat(fullPath, &statBuffer) == 0 ) {
			path = strdup(fullPath);
			break;
		}
	}

	// read in whole file
	int fd = ::open(path, O_RDONLY, 0);
	if ( fd == -1 )
		throwf("can't open order file: %s", path);
	struct stat stat_buf;
	::fstat(fd, &stat_buf);
	char* p = (char*)malloc(stat_buf.st_size+1);
	if ( p == NULL )
		throwf("can't process order file: %s", path);
	if ( read(fd, p, stat_buf.st_size) != stat_buf.st_size )
		throwf("can't read order file: %s", path);
	::close(fd);
	p[stat_buf.st_size] = '\n';
	this->addDependency(Options::depMisc, path);
	fOrderFilePath = strdup(path);

	// parse into vector of pairs
	char * const end = &p[stat_buf.st_size+1];
	enum { lineStart, inSymbol, inComment } state = lineStart;
	char* symbolStart = NULL;
	for (char* s = p; s < end; ++s ) {
		switch ( state ) {
		case lineStart:
			if ( *s =='#' ) {
				state = inComment;
			}
			else if ( !isspace(*s) || cstring ) {
				state = inSymbol;
				symbolStart = s;
			}
			break;
		case inSymbol:
			if ( (*s == '\n') || (!cstring && (*s == '#')) ) {
				bool wasComment = (*s == '#');
				*s = '\0';
				// removing any trailing spaces
				char* last = s-1;
				while ( isspace(*last) ) {
					*last = '\0';
					--last;
				}
				// if there is an architecture prefix, only use this symbol it if matches current arch
				if ( strncmp(symbolStart, "ppc:", 4) == 0 ) {
					symbolStart = NULL;
				}
				else if ( strncmp(symbolStart, "ppc64:", 6) == 0 ) {
					symbolStart = NULL;
				}
				else if ( strncmp(symbolStart, "i386:", 5) == 0 ) {
					if ( fArchitecture == CPU_TYPE_I386 )
						symbolStart = &symbolStart[5];
					else
						symbolStart = NULL;
				}
				else if ( strncmp(symbolStart, "x86_64:", 7) == 0 ) {
					if ( fArchitecture == CPU_TYPE_X86_64 )
						symbolStart = &symbolStart[7];
					else
						symbolStart = NULL;
				}
				else if ( strncmp(symbolStart, "arm:", 4) == 0 ) {
					if ( fArchitecture == CPU_TYPE_ARM )
						symbolStart = &symbolStart[4];
					else
						symbolStart = NULL;
				}
				else if ( strncmp(symbolStart, "arm64:", 6) == 0 ) {
					if ( fArchitecture == CPU_TYPE_ARM64 )
						symbolStart = &symbolStart[6];
					else
						symbolStart = NULL;
				}
				if ( symbolStart != NULL ) {
					char* objFileName = NULL;
					char* colon = strstr(symbolStart, ".o:");
					if ( colon != NULL ) {
						colon[2] = '\0';
						objFileName = symbolStart;
						symbolStart = &colon[3];
					}
					else {
						colon = strstr(symbolStart, ".o):");
						if ( colon != NULL ) {
							colon[3] = '\0';
							objFileName = symbolStart;
							symbolStart = &colon[4];
						}
					}
					// trim leading spaces
					while ( isspace(*symbolStart) ) 
						++symbolStart;
					Options::OrderedSymbol pair;
					if ( cstring )
						pair.symbolName = cstringSymbolName(symbolStart);
					else
						pair.symbolName = symbolStart;
					pair.objectFileName = objFileName;
					fOrderedSymbols.push_back(pair);
				}
				symbolStart = NULL;
				if ( wasComment )
					state = inComment;
				else
					state = lineStart;
			}
			break;
		case inComment:
			if ( *s == '\n' )
				state = lineStart;
			break;
		}
	}
	// Note: we do not free() the malloc buffer, because the strings are used by the fOrderedSymbols
}

void Options::parseSectionOrderFile(const char* segment, const char* section, const char* path)
{
	if ( (strcmp(section, "__cstring") == 0) && (strcmp(segment, "__TEXT") == 0) ) {
		parseOrderFile(path, true);
	}
	else if ( (strncmp(section, "__literal",9) == 0) && (strcmp(segment, "__TEXT") == 0) ) {
		warning("sorting of __literal[4,8,16] sections not supported");
	}
	else {
		// ignore section information and append all symbol names to global order file
		parseOrderFile(path, false);
	}
}

void Options::addSection(const char* segment, const char* section, const char* path)
{
	if ( strlen(segment) > 16 )
		throw "-seccreate segment name max 16 chars";
	if ( strlen(section) > 16 ) {
		char* tmp = strdup(section);
		tmp[16] = '\0';
		warning("-seccreate section name (%s) truncated to 16 chars (%s)\n", section, tmp);
		section = tmp;
	}

	// read in whole file
	int fd = ::open(path, O_RDONLY, 0);
	if ( fd == -1 )
		throwf("can't open -sectcreate file: %s", path);
	struct stat stat_buf;
	::fstat(fd, &stat_buf);
	char* p = (char*)malloc(stat_buf.st_size);
	if ( p == NULL )
		throwf("can't process -sectcreate file: %s", path);
	if ( read(fd, p, stat_buf.st_size) != stat_buf.st_size )
		throwf("can't read -sectcreate file: %s", path);
	::close(fd);

	// record section to create
	ExtraSection info = { segment, section, path, (uint8_t*)p, (uint64_t)stat_buf.st_size };
	fExtraSections.push_back(info);
}

void Options::addSectionRename(const char* srcSegment, const char* srcSection, const char* dstSegment, const char* dstSection)
{
	if ( strlen(srcSegment) > 16 )
		throw "-rename_section segment name max 16 chars";
	if ( strlen(srcSection) > 16 )
		throw "-rename_section section name max 16 chars";
	if ( strlen(dstSegment) > 16 )
		throw "-rename_section segment name max 16 chars";
	if ( strlen(dstSection) > 16 )
		throw "-rename_section section name max 16 chars";

	SectionRename info;
	info.fromSegment = srcSegment;
	info.fromSection = srcSection;
	info.toSegment = dstSegment;
	info.toSection = dstSection;

	fSectionRenames.push_back(info);
}


void Options::addSegmentRename(const char* srcSegment, const char* dstSegment)
{
	if ( strlen(srcSegment) > 16 )
		throw "-rename_segment segment name max 16 chars";
	if ( strlen(dstSegment) > 16 )
		throw "-rename_segment segment name max 16 chars";

	SegmentRename info;
	info.fromSegment = srcSegment;
	info.toSegment = dstSegment;

	fSegmentRenames.push_back(info);
}



void Options::addSymbolMove(const char* dstSegment, const char* symbolList,
							std::vector<SymbolsMove>& list, const char* optionName, SymbolMatchingMode match_mode)
{
	if ( strlen(dstSegment) > 16 )
		throwf("%s segment name max 16 chars", optionName);
	
	SymbolsMove tmp;
	list.push_back(tmp);
	SymbolsMove& info = list.back();
	info.toSegment = dstSegment;
	loadExportFile(symbolList, optionName, info.symbols, match_mode);
}
		
bool Options::moveRwSymbol(std::string_view symName, const char* filePath, const char*& seg, bool& wildCardMatch) const
{
	auto nameStr = std::string(symName);
	auto *name = nameStr.c_str();
	for (std::vector<SymbolsMove>::const_iterator it=fSymbolsMovesData.begin(); it != fSymbolsMovesData.end(); ++it) {
		const SymbolsMove& info = *it;
		if ( info.symbols.containsWithPrefix(name, filePath, wildCardMatch)) {
			seg  = info.toSegment;
			return true;
		}
	}
	return false;
}

bool Options::moveAXMethodList(const char* className) const
{
	for (const SymbolsMove& sm : fSymbolsMovesAXMethodLists) {
		bool wildcard;
		if ( sm.symbols.containsWithPrefix(className, NULL, wildcard) )
			return true;
	}
	return false;
}

bool Options::moveRoSymbol(std::string_view symName, const char* filePath, const char*& seg, bool& wildCardMatch) const
{
	auto nameStr = std::string(symName);
	auto *name = nameStr.c_str();
	for (std::vector<SymbolsMove>::const_iterator it=fSymbolsMovesCode.begin(); it != fSymbolsMovesCode.end(); ++it) {
		const SymbolsMove& info = *it;
		if ( info.symbols.containsWithPrefix(name, filePath, wildCardMatch)) {
			seg  = info.toSegment;
			return true;
		}
	}
	return false;
}

void Options::addSectionAlignment(const char* segment, const char* section, const char* alignmentStr)
{
	if ( strlen(segment) > 16 )
		throw "-sectalign segment name max 16 chars";
	if ( strlen(section) > 16 )
		throw "-sectalign section name max 16 chars";

	// argument to -sectalign is a hexadecimal number
	char* endptr;
	unsigned long value = strtoul(alignmentStr, &endptr, 16);
	if ( *endptr != '\0')
		throw "argument for -sectalign is not a hexadecimal number";
	if ( value > 0x8000 )
		throw "argument for -sectalign must be less than or equal to 0x8000";
	if ( value == 0 ) {
		warning("zero is not a valid -sectalign");
		value = 1;
	}

	// alignment is power of 2 (e.g. page alignment = 12)
	uint8_t alignment = (uint8_t)__builtin_ctz(value);
	if ( (unsigned long)(1 << alignment) != value ) {
		warning("alignment for -sectalign %s %s is not a power of two, using 0x%X",
			segment, section, 1 << alignment);
	}

	SectionAlignment info = { segment, section, alignment };
	fSectionAlignments.push_back(info);
}

void Options::addLibrary(const FileInfo& info)
{
	// if this library has already been added, don't add again (archives are automatically repeatedly searched)
	for (std::vector<Options::FileInfo>::iterator fit = fInputFiles.begin(); fit != fInputFiles.end(); fit++) {
		if ( strcmp(info.path, fit->path) == 0 ) {
			// if dylib is specified again but weak, record that it should be weak
			if ( info.options.fWeakImport )
				fit->options.fWeakImport = true;
			return;
		}
	}
	// add to list
	fInputFiles.push_back(info);
}

void Options::warnObsolete(const char* arg)
{
	warning("option %s is obsolete and being ignored", arg);
}


void Options::cannotBeUsedWithBitcode(const char* arg)
{
	if ( fBundleBitcode )
		throwf("%s and -bitcode_bundle (Xcode setting ENABLE_BITCODE=YES) cannot be used together", arg);
}

std::string Options::getVersionString32(uint32_t ver) const
{
	if (ver == 0 || ver >= 0x10000000)
		return "0.0.0";

	unsigned microVersion = ver & 0xFF;
	unsigned minorVersion = (ver >> 8) & 0xFF;
	unsigned majorVersion = (ver >> 16) & 0xFF;
	std::stringstream versionString;
	if ( microVersion == 0 )
		versionString << majorVersion << "." << minorVersion;
	else
		versionString << majorVersion << "." << minorVersion << "." << microVersion;
	return versionString.str();
}

std::string Options::getVersionString64(uint64_t ver) const
{
	uint64_t a = (ver >> 40) & 0xFFFFFF;
	uint64_t b = (ver >> 30) & 0x3FF;
	uint64_t c = (ver >> 20) & 0x3FF;
	uint64_t d = (ver >> 10) & 0x3FF;
	uint64_t e = ver & 0x3FF;
	std::stringstream versionString;
	versionString << a << "." << b << "." << c << "." << d << "." << e;
	return versionString.str();
}

// Convert X.Y[.Z] to 32-bit value xxxxyyzz
bool Options::parsePackedVersion32(const std::string& versionStr, uint32_t &result)
{
	result = 0;

	if ( versionStr.empty() )
		return false;

	size_t pos = versionStr.find('.');
	if ( pos == std::string::npos )
		return false;

	std::string majorStr = versionStr.substr(0, pos);
	std::string rest = versionStr.substr(pos+1);

	try {
		size_t majorEnd;
		int majorValue = std::stoi(majorStr, &majorEnd);
		if ( majorEnd != majorStr.size() )
			return false;
		if ( majorValue < 0 )
			return false;
		if ( majorValue > 65535 )
			return false;

		std::string minorStr;
		std::string microStr;
		pos = rest.find('.');
		if ( pos == std::string::npos ) {
			minorStr = rest;
		}
		else {
			minorStr = rest.substr(0, pos);
			microStr = rest.substr(pos+1);
		}

		size_t minorEnd;
		int minorValue = std::stoi(minorStr, &minorEnd);
		if ( minorEnd != minorStr.size() )
			return false;
		if ( minorValue < 0 )
			return false;
		if ( minorValue > 255 )
			return false;

		int microValue = 0;
		if ( !microStr.empty() ) {
			size_t microEnd;
			microValue = std::stoi(microStr, &microEnd);
			if ( microEnd != microStr.size() )
				return false;
			if ( microValue < 0 )
				return false;
			if ( microValue > 255 )
				return false;
		}

		result = (uint32_t)((majorValue << 16) | (minorValue << 8) | microValue);

		return true;
	}
	catch (...) {
		// std::stoi() throws exception on malformed input
		return false;
	}
}

std::string Options::getSDKVersionStr() const
{
	std::string retval;
	auto appendString = [&](const std::string& string) {
		if (retval.empty()) {
			retval = string;
		} else {
			retval += "/";
			retval += string;
		}
	};

	platforms().forEach(^(ld::Platform platform, uint32_t minVersion, uint32_t sdkVersion, bool &stop) {
		appendString(getVersionString32(sdkVersion));
	});

	return retval;
}

std::vector<std::string> Options::writeBitcodeLinkOptions() const
{
	__block std::vector<std::string> linkCommand;
	switch ( fOutputKind ) {
		case Options::kDynamicLibrary:
			linkCommand.push_back("-dylib");
			linkCommand.push_back("-compatibility_version");
			if ( fDylibCompatVersion != 0 ) {
				linkCommand.push_back(getVersionString32(fDylibCompatVersion));
			} else {
				linkCommand.push_back(getVersionString32(currentVersion32()));
			}
			if ( fDylibCurrentVersion != 0 ) {
				linkCommand.push_back("-current_version");
				linkCommand.push_back(getVersionString64(fDylibCurrentVersion));
			}
			linkCommand.push_back("-install_name");
			linkCommand.push_back(installPath());
			break;
		case Options::kDynamicExecutable:
			linkCommand.push_back("-execute");
			break;
		case Options::kObjectFile:
			linkCommand.push_back("-r");
			break;
		default:
			throwf("could not write bitcode options file output kind\n");
	}

	if (!fImplicitlyLinkPublicDylibs)
		linkCommand.push_back("-no_implicit_dylibs");

	// Add deployment target.
	// Platform is allowed to be unknown for "ld -r".

	platforms().forEach(^(ld::Platform platform, uint32_t minVersion, uint32_t sdkVersion, bool &stop) {
		std::string platName = nameFromPlatform(platform);
		std::transform(platName.begin(), platName.end(), platName.begin(), [](unsigned char c) -> unsigned char {
			if (c == ' ') return '-';
			return std::tolower(c);
		});
		linkCommand.push_back("-platform_version");
		linkCommand.push_back(platName.c_str());
		linkCommand.push_back(getVersionString32(minVersion));
		linkCommand.push_back(getVersionString32(sdkVersion));
	});


	// entry name
	if ( fEntryName ) {
		linkCommand.push_back("-e");
		linkCommand.push_back(fEntryName);
	}

	// Write rpaths
	if (!fRPaths.empty()) {
		for (std::vector<const char*>::const_iterator it=fRPaths.begin(); it != fRPaths.end(); ++it) {
			linkCommand.push_back("-rpath");
			linkCommand.push_back(*it);
		}
	}

	// Other bitcode compatiable options
	if ( fObjCABIVersion1Override ) {
		linkCommand.push_back("-objc_abi_version");
		linkCommand.push_back("1");
	} else if ( fObjCABIVersion2Override ) {
		linkCommand.push_back("-objc_abi_version");
		linkCommand.push_back("2");
	}
	if ( fExecutablePath ) {
		linkCommand.push_back("-executable_path");
		linkCommand.push_back(fExecutablePath);
	}
	if ( fDeadStrip )
		linkCommand.push_back("-dead_strip");
	if ( fExportDynamic )
		linkCommand.push_back("-export_dynamic");
	if ( fMarkAppExtensionSafe && fCheckAppExtensionSafe )
		linkCommand.push_back("-application_extension");

	if ( fSourceVersionLoadCommandForceOn )
		linkCommand.push_back("-add_source_version");
	if ( fSourceVersion != 0 ) {
		linkCommand.push_back("-source_version");
		linkCommand.push_back(getVersionString64(fSourceVersion));
	}

	for (const SectionAlignment &sectalign : fSectionAlignments)
		for (const char *arg : {"-sectalign", sectalign.segmentName, sectalign.sectionName})
			linkCommand.push_back(arg);

	// linker flag added by swift driver
	// rdar://problem/20108072
	if ( !fObjcCategoryMerging )
		linkCommand.push_back("-no_objc_category_merging");

	return linkCommand;
}

const char* Options::checkForNullArgument(const char* argument_name, const char* arg, bool allowDashArg) const
{
	if ( arg == NULL )
		throwf("missing argument for %s", argument_name);
	if ( !allowDashArg && (arg[0] == '-') )
		warning("argument missing after %s", argument_name);
	return arg;
}

const char* Options::checkForNullVersionArgument(const char* argument_name, const char* arg) const
{
	if ( arg == NULL )
		throwf("%s missing version argument", argument_name);
	return arg;
}

static const ld::PlatformInfo* isPlatformOption(const char* arg)
{
	__block const ld::PlatformInfo* result = nullptr;
	forEachSupportedPlatform(^(const ld::PlatformInfo& info, bool& stop) {
		if ( strcmp(arg, info.commandLineArg) == 0 ) {
			result = &info;
			stop = true;
		}
	});
	return result;
}

static const char* removeDoubleSlash(const char* path)
{
	if ( strstr(path, "//") == nullptr )
		return path;

	char fixedPath[strlen(path)+1];
	char* t = fixedPath;
	bool lastWasSlash = false;
	for (const char* s=path; *s != '\0'; ++s) {
		if ( *s == '/' ) {
			if ( !lastWasSlash )
				*t++ = *s;
			lastWasSlash = true;
		}
		else {
			*t++ = *s;
			lastWasSlash = false;
		}
	}
	*t = '\0';
	return strdup(fixedPath);
}

//
// Process all command line arguments.
//
// The only error checking done here is that each option is valid and if it has arguments
// that they too are valid.
//
// The general rule is "last option wins", i.e. if both -bundle and -dylib are specified,
// whichever was last on the command line is used.
//
// Error check for invalid combinations of options is done in checkIllegalOptionCombinations()
//
void Options::parse(int argc, const char* argv[])
{
    // Store the original args in the link snapshot.
    fLinkSnapshot.recordRawArgs(argc, argv);
    
	// pass one builds search list from -L and -F options
	this->buildSearchPaths(argc, argv);

	// reduce re-allocations
	fInputFiles.reserve(32);

	// pass two parse all other options
	for(int i=1; i < argc; ++i) {
		const char* arg = argv[i];

		if ( arg[0] == '-' ) {
            // by default, copy one arg to the snapshot link command, and do no file copying
            int snapshotArgIndex = i;
            int snapshotArgCount = -1; // -1 means compute count based on change in index
            int snapshotFileArgIndex = -1; // -1 means no data file parameter to arg

			// Since we don't care about the files passed, just the option names, we do this here.
			if (fPrintOptions)
				fprintf (stderr, "[Logging ld64 options]\t%s\n", arg);

			// grandfather in old arg name
			if ( strcmp(arg, "-iphoneos_version_min") == 0 )
				arg = "-ios_version_min";

			if ( strcmp(arg, "-macosx_version_min") == 0 )
				arg = "-macos_version_min";

			if ( (strcmp(arg, "-iosmac_version_min") == 0) || (strcmp(arg, "-uikitformac_version_min") == 0) )
				arg = "-maccatalyst_version_min";

			if ( (arg[1] == 'L') || (arg[1] == 'F') ) {
                snapshotArgCount = 0; // stripped out of link snapshot
				if (arg[2] == '\0')
					++i;
				// previously handled by buildSearchPaths()
			}
			// The one gnu style option we have to keep compatibility
			// with gcc. Might as well have the single hyphen one as well.
			else if ( (strcmp(arg, "--help") == 0)
					  || (strcmp(arg, "-help") == 0)) {
				fprintf (stdout, "ld64: For information on command line options please use 'man ld'.\n");
				exit (0);
			}
			else if ( strcmp(arg, "-arch") == 0 ) {
				const char* arch = argv[++i];
				parseArch(arch);
				fLinkSnapshot.recordArch(arch);
			}
			else if ( strcmp(arg, "-dynamic") == 0 ) {
				// default
			}
			else if ( strcmp(arg, "-static") == 0 ) {
				fForStatic = true;
				if ( (fOutputKind != kObjectFile) && (fOutputKind != kKextBundle) ) {
					fOutputKind = kStaticExecutable;
				}
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-dylib") == 0 ) {
				fOutputKind = kDynamicLibrary;
			}
			else if ( strcmp(arg, "-bundle") == 0 ) {
				fOutputKind = kDynamicBundle;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-dylinker") == 0 ) {
				fOutputKind = kDyld;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-execute") == 0 ) {
				if ( fOutputKind != kStaticExecutable )
					fOutputKind = kDynamicExecutable;
			}
			else if ( strcmp(arg, "-preload") == 0 ) {
				fOutputKind = kPreload;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-r") == 0 ) {
				fOutputKind = kObjectFile;
			}
			else if ( strcmp(arg, "-kext") == 0 ) {
				fOutputKind = kKextBundle;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-kernel") == 0 ) {
				fKernel = true;
			}
			else if ( strcmp(arg, "-kext_objects_dir") == 0 ) {
				fKextObjectsDirPath = argv[++i];
				if ( fKextObjectsDirPath == NULL )
					throw "missing argument to -kext_objects_dir";
				fKextObjectsEnable = 1;
			}
			else if ( strcmp(arg, "-no_kext_objects") == 0 ) {
				fKextObjectsEnable = 0;
			}
			else if ( strcmp(arg, "-o") == 0 ) {
				snapshotArgCount = 0;
				fOutputFile = checkForNullArgument(arg, argv[++i]);
				fLinkSnapshot.setOutputPath(fOutputFile);
			}
			else if ( strncmp(arg, "-lazy-l", 7) == 0 ) {
                snapshotArgCount = 0;
				FileInfo info = findLibrary(&arg[7], true);
				info.ordinal = ld::File::Ordinal::makeArgOrdinal((uint16_t)i);
				addLibrary(info);
				cannotBeUsedWithBitcode(arg);
				warning("-lazy-l is deprecated, changing to regular link");
			}
			else if ( strcmp(arg, "-lto_library") == 0 ) {
                snapshotFileArgIndex = 1;
				fOverridePathlibLTO = argv[++i];
				if ( fOverridePathlibLTO == NULL )
					throw "missing argument to -lto_library";
			}
			else if ( strcmp(arg, "-cache_path_lto") == 0 ) {
				fLtoCachePath = argv[++i];
				if ( fLtoCachePath == NULL )
					throw "missing argument to -cache_path_lto";
			}
			else if ( strcmp(arg, "-prune_interval_lto") == 0 ) {
				const char* value = argv[++i];
				if ( value == NULL )
					throw "missing argument to -prune_interval_lto";
				char* endptr;
				fLtoPruneIntervalOverwrite = true;
				fLtoPruneInterval = strtoul(value, &endptr, 10);
				if ( *endptr != '\0')
					throw "invalid argument for -prune_interval_lto";
			}
			else if ( strcmp(arg, "-prune_after_lto") == 0 ) {
				const char* value = argv[++i];
				if ( value == NULL )
					throw "missing argument to -prune_after_lto";
				char* endptr;
				fLtoPruneAfter = strtoul(value, &endptr, 10);
				if ( *endptr != '\0')
					throw "invalid argument for -prune_after_lto";
			}
			else if ( strcmp(arg, "-max_relative_cache_size_lto") == 0 ) {
				const char* value = argv[++i];
				if ( value == NULL )
					throw "missing argument to -max_relative_cache_size_lto";
				char* endptr;
				fLtoMaxCacheSize = strtoul(value, &endptr, 10);
				if ( *endptr != '\0')
					throw "invalid argument for -max_relative_cache_size_lto";
				if (fLtoMaxCacheSize > 100)
					throw "Expect a value between 0 and 100 for -max_relative_cache_size_lto";
			}
			else if ( (arg[1] == 'l') && (strncmp(arg,"-lazy_",6) != 0)  && (strcmp(arg,"-load_hidden") != 0) ) {
                snapshotArgCount = 0;
                try {
					FileInfo info = findLibrary(&arg[2]);
					info.ordinal = ld::File::Ordinal::makeArgOrdinal((uint16_t)i);
					addLibrary(info);
				}
				catch (const char* message) {
					// see if this is a Csu .o file only need for when targeting old OS versions
					__block bool 					isPreMinOS = false;
					__block ld::Platform 			thisPlatform = ld::Platform::unknown;
					__block uint32_t				thisPlatformMinOS = 0;
					fPlatforms.forEach(^(ld::Platform aPlatform, uint32_t minVersion, uint32_t sdkVersion, bool& stop) {
						const ld::PlatformInfo& info = platformInfo(aPlatform);
						if ( minVersion < info.minimumOsVersion ) {
							isPreMinOS 		  = true;
							thisPlatform 	  = aPlatform;
							thisPlatformMinOS = info.minimumOsVersion;
						}
					});
					if ( !isPreMinOS )
						throw;
					if ( strncmp(arg, "-l", 2) != 0 )
						throw;
					if ( strncmp(&arg[strlen(arg)-2], ".o", 2) != 0 )
						throw;
					// throw away this .o and use newer min OS vers
					warning("Csu support file %s not found, changing to target %s %s where it is not needed",
							arg, nameFromPlatform(thisPlatform), getVersionString32(thisPlatformMinOS).c_str());
					fPlatforms.updateMinVersion(thisPlatform, thisPlatformMinOS);
				}
			}
			// This causes a dylib to be weakly bound at
			// link time.  This corresponds to weak_import.
			else if ( strncmp(arg, "-weak-l", 7) == 0 ) {
				snapshotArgCount = 0;
				FileInfo info = findLibrary(&arg[7]);
				info.options.fWeakImport = true;
				info.ordinal = ld::File::Ordinal::makeArgOrdinal((uint16_t)i);
				addLibrary(info);
			}
			else if ( strncmp(arg, "-needed-l", 9) == 0 ) {
				snapshotArgCount = 0;
				FileInfo info = findLibrary(&arg[9]);
				info.options.fNeeded = true;
				info.ordinal = ld::File::Ordinal::makeArgOrdinal((uint16_t)i);
				addLibrary(info);
			}
			// Avoid lazy binding.
			else if ( strcmp(arg, "-bind_at_load") == 0 ) {
				fBindAtLoad = true;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-twolevel_namespace") == 0 ) {
				fNameSpace = kTwoLevelNameSpace;
			}
			else if ( strcmp(arg, "-flat_namespace") == 0 ) {
				fNameSpace = kFlatNameSpace;
				cannotBeUsedWithBitcode(arg);
			}
			// Also sets a bit to ensure dyld causes everything
			// in the namespace to be flat.
			// ??? Deprecate
			else if ( strcmp(arg, "-force_flat_namespace") == 0 ) {
				fNameSpace = kForceFlatNameSpace;
				cannotBeUsedWithBitcode(arg);
			}
			// Similar to --whole-archive.
			else if ( strcmp(arg, "-all_load") == 0 ) {
				fFullyLoadArchives = true;
			}
			else if ( strcmp(arg, "-noall_load") == 0) {
				warnObsolete(arg);
			}
			// Similar to -all_load
			else if ( strcmp(arg, "-ObjC") == 0 ) {
				fLoadAllObjcObjectsFromArchives = true;
			}
			// Similar to -all_load, but for the following archive only.
			else if ( strcmp(arg, "-force_load") == 0 ) {
				const char* path = checkForNullArgument(arg, argv[++i]);
				FileInfo info = findFile(path);
				info.options.fForceLoad = true;
				info.ordinal = ld::File::Ordinal::makeArgOrdinal((uint16_t)i);
				addLibrary(info);
			}
			else if ( strcmp(arg, "-load_hidden") == 0 ) {
				const char* path = checkForNullArgument(arg, argv[++i]);
				FileInfo info = findFile(path);
				info.options.fLoadHidden = true;
				info.ordinal = ld::File::Ordinal::makeArgOrdinal((uint16_t)i);
				addLibrary(info);
			}
			else if ( strncmp(arg, "-hidden-l", 9) == 0 ) {
				snapshotArgCount = 0;
				FileInfo info = findLibrary(&arg[9]);
				info.options.fLoadHidden = true;
				info.ordinal = ld::File::Ordinal::makeArgOrdinal((uint16_t)i);
				addLibrary(info);
			}
			// Library versioning.
			else if ( (strcmp(arg, "-dylib_compatibility_version") == 0)
					  || (strcmp(arg, "-compatibility_version") == 0)) {
				 const char* vers = argv[++i];
				 if ( vers == NULL )
					throw "-dylib_compatibility_version missing <version>";
				fDylibCompatVersion = parseVersionNumber32(vers);
			}
			else if ( (strcmp(arg, "-dylib_current_version") == 0)
					  || (strcmp(arg, "-current_version") == 0)) {
				 const char* vers = argv[++i];
				 if ( vers == NULL )
					throw "-dylib_current_version missing <version>";
				fDylibCurrentVersion = parseVersionNumber64(vers);
			}
			else if ( strcmp(arg, "-sectorder") == 0 ) {
				 if ( (argv[i+1]==NULL) || (argv[i+2]==NULL) || (argv[i+3]==NULL) )
					throw "-sectorder missing <segment> <section> <file-path>";
                snapshotFileArgIndex = 3;
				parseSectionOrderFile(argv[i+1], argv[i+2], argv[i+3]);
				i += 3;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-order_file") == 0 ) {
                snapshotFileArgIndex = 1;
				parseOrderFile(argv[++i], false);
			}
			else if ( strcmp(arg, "-order_file_statistics") == 0 ) {
				fPrintOrderFileStatistics = true;
				cannotBeUsedWithBitcode(arg);
			}
			// ??? Deprecate segcreate.
			// -sectcreate puts whole files into a section in the output.
			else if ( (strcmp(arg, "-sectcreate") == 0) || (strcmp(arg, "-segcreate") == 0) ) {
				 if ( (argv[i+1]==NULL) || (argv[i+2]==NULL) || (argv[i+3]==NULL) )
					throw "-sectcreate missing <segment> <section> <file-path>";
                snapshotFileArgIndex = 3;
				addSection(argv[i+1], argv[i+2], argv[i+3]);
				i += 3;
			}
			// Since we have a full path in binary/library names we need to be able to override it.
			else if ( (strcmp(arg, "-dylib_install_name") == 0)
					  || (strcmp(arg, "-dylinker_install_name") == 0)
					  || (strcmp(arg, "-install_name") == 0)) {
				const char* installName = argv[++i];
                if ( installName == nullptr )
					throw "-install_name missing <path>";
				fDylibInstallName = removeDoubleSlash(installName);
				if ( fDylibInstallName != installName )
					warning("double slash removed from -install_name (%s)", installName);
			}
			// Sets the base address of the output.
			else if ( (strcmp(arg, "-seg1addr") == 0) || (strcmp(arg, "-image_base") == 0) ) {
				 const char* address = argv[++i];
				 if ( address == NULL )
					throwf("%s missing <address>", arg);
				fBaseAddress = parseAddress(address);
				uint64_t temp = ((fBaseAddress+fSegmentAlignment-1) & (-fSegmentAlignment)); 
				if ( fBaseAddress != temp ) {
					warning("-seg1addr not %lld byte aligned, rounding up", fSegmentAlignment);
					fBaseAddress = temp;
				}
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-e") == 0 ) {
				fEntryName = argv[++i];
			}
			// Same as -@ from the FSF linker.
			else if ( strcmp(arg, "-filelist") == 0 ) {
                 snapshotArgCount = 0;
				 const char* path = argv[++i];
				 if ( (path == NULL) || (path[0] == '-') )
					throw "-filelist missing <path>";
				 ld::File::Ordinal baseOrdinal = ld::File::Ordinal::makeArgOrdinal((uint16_t)i);
				 loadFileList(path, baseOrdinal);
			}
			else if ( strcmp(arg, "-keep_private_externs") == 0 ) {
				cannotBeUsedWithBitcode(arg);
				fKeepPrivateExterns = true;
			}
			else if ( strcmp(arg, "-final_output") == 0 ) {
				// some projects end up with double slash in -final_output path
				fFinalName = removeDoubleSlash(argv[++i]);
			}
			// Ensure that all calls to exported symbols go through lazy pointers.  Multi-module
			// just ensures that this happens for cross object file boundaries.
			else if ( (strcmp(arg, "-interposable") == 0) || (strcmp(arg, "-multi_module") == 0)) {
				switch ( fInterposeMode ) {
					case kInterposeNone:
					case kInterposeAllExternal:
						fInterposeMode = kInterposeAllExternal;
						break;
					case kInterposeSome:
						// do nothing, -interposable_list overrides -interposable"
						break;
				}
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-interposable_list") == 0 ) {
                snapshotFileArgIndex = 1;
				fInterposeMode = kInterposeSome;
				loadExportFile(argv[++i], "-interposable_list", fInterposeList, kAllowWildcards);
				cannotBeUsedWithBitcode(arg);
			}
			// Default for -interposable/-multi_module/-single_module.
			else if ( strcmp(arg, "-single_module") == 0 ) {
				fInterposeMode = kInterposeNone;
			}
			else if ( strcmp(arg, "-exported_symbols_list") == 0 ) {
                snapshotFileArgIndex = 1;
				if ( fExportMode == kDontExportSome )
					throw "can't use -exported_symbols_list and -unexported_symbols_list";
				fExportMode = kExportSome;
				loadExportFile(argv[++i], "-exported_symbols_list", fExportSymbols, kAllowWildcards);
			}
			else if ( strcmp(arg, "-unexported_symbols_list") == 0 ) {
                snapshotFileArgIndex = 1;
				if ( fExportMode == kExportSome )
					throw "can't use -unexported_symbols_list and -exported_symbols_list";
				fExportMode = kDontExportSome;
				loadExportFile(argv[++i], "-unexported_symbols_list", fDontExportSymbols, kAllowWildcards);
			}
			else if ( strcmp(arg, "-exported_symbol") == 0 ) {
				if ( fExportMode == kDontExportSome )
					throw "can't use -exported_symbol and -unexported_symbols";
				fExportMode = kExportSome;
				const char* symbol = checkForNullArgument(arg, argv[++i]);
				fExportSymbols.insert(symbol, kAllowWildcards);
			}
			else if ( strcmp(arg, "-unexported_symbol") == 0 ) {
				if ( fExportMode == kExportSome )
					throw "can't use -unexported_symbol and -exported_symbol";
				fExportMode = kDontExportSome;
				const char* symbol = checkForNullArgument(arg, argv[++i]);
				fDontExportSymbols.insert(symbol, kAllowWildcards);
			}
			else if ( strcmp(arg, "-non_global_symbols_no_strip_list") == 0 ) {
                snapshotFileArgIndex = 1;
				if ( fLocalSymbolHandling == kLocalSymbolsSelectiveExclude )
					throw "can't use -non_global_symbols_no_strip_list and -non_global_symbols_strip_list";
				fLocalSymbolHandling = kLocalSymbolsSelectiveInclude;
				loadExportFile(argv[++i], "-non_global_symbols_no_strip_list", fLocalSymbolsIncluded, kAllowWildcards);
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-non_global_symbols_strip_list") == 0 ) {
                snapshotFileArgIndex = 1;
				if ( fLocalSymbolHandling == kLocalSymbolsSelectiveInclude )
					throw "can't use -non_global_symbols_no_strip_list and -non_global_symbols_strip_list";
				fLocalSymbolHandling = kLocalSymbolsSelectiveExclude;
				loadExportFile(argv[++i], "-non_global_symbols_strip_list", fLocalSymbolsExcluded, kAllowWildcards);
				cannotBeUsedWithBitcode(arg);
			}
			// ??? Deprecate
			else if ( strcmp(arg, "-no_arch_warnings") == 0 ) {
				 fIgnoreOtherArchFiles = true;
			}
			else if ( strcmp(arg, "-force_cpusubtype_ALL") == 0 ) {
				fForceSubtypeAll = true;
				fAllowCpuSubtypeMismatches = true;
				cannotBeUsedWithBitcode(arg);
			}
			// Similar to -weak-l but uses the absolute path name to the library.
			else if ( strcmp(arg, "-weak_library") == 0 ) {
				const char* path = checkForNullArgument(arg, argv[++i]);
				FileInfo info = findFile(path);
				info.options.fWeakImport = true;
				info.ordinal = ld::File::Ordinal::makeArgOrdinal((uint16_t)i);
				addLibrary(info);
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-lazy_library") == 0 ) {
				const char* path = checkForNullArgument(arg, argv[++i]);
				FileInfo info = findFile(path);
				info.ordinal = ld::File::Ordinal::makeArgOrdinal((uint16_t)i);
				addLibrary(info);
				cannotBeUsedWithBitcode(arg);
				warning("-lazy_library is deprecated, changing to regular link");
			}
			else if ( strcmp(arg, "-needed_library") == 0 ) {
                snapshotArgCount = 0;
				const char* path = checkForNullArgument(arg, argv[++i]);
				FileInfo info = findFile(path);
				info.options.fNeeded = true;
				info.ordinal = ld::File::Ordinal::makeArgOrdinal((uint16_t)i);
				addLibrary(info);
			}
			else if ( strcmp(arg, "-framework") == 0 ) {
				FileInfo info = findFramework(argv[++i]);
				info.ordinal = ld::File::Ordinal::makeArgOrdinal((uint16_t)i);
				addLibrary(info);
			}
			else if ( strcmp(arg, "-weak_framework") == 0 ) {
				FileInfo info = findFramework(argv[++i]);
				info.options.fWeakImport = true;
				info.ordinal = ld::File::Ordinal::makeArgOrdinal((uint16_t)i);
				addLibrary(info);
			}
			else if ( strcmp(arg, "-lazy_framework") == 0 ) {
				FileInfo info = findFramework(argv[++i]);
				info.ordinal = ld::File::Ordinal::makeArgOrdinal((uint16_t)i);
				addLibrary(info);
				cannotBeUsedWithBitcode(arg);
				warning("-lazy_framework is deprecated, changing to regular link");
			}
			else if ( strcmp(arg, "-needed_framework") == 0 ) {
				FileInfo info = findFramework(argv[++i]);
				info.options.fNeeded = true;
				info.ordinal = ld::File::Ordinal::makeArgOrdinal((uint16_t)i);
				addLibrary(info);
			}
			else if ( strcmp(arg, "-search_paths_first") == 0 ) {
				// previously handled by buildSearchPaths()
			}
			else if ( strcmp(arg, "-search_dylibs_first") == 0 ) {
				// previously handled by buildSearchPaths()
			}
			else if ( strcmp(arg, "-undefined") == 0 ) {
                setUndefinedTreatment(argv[++i]);
				cannotBeUsedWithBitcode(arg);
			}
			// Debugging output flag.
			else if ( strcmp(arg, "-arch_multiple") == 0 ) {
				 fMessagesPrefixedWithArchitecture = true;
			}
			// Specify what to do with relocations in read only
			// sections like .text.  Could be errors, warnings,
			// or suppressed.  Currently we do nothing with the
			// flag.
			else if ( strcmp(arg, "-read_only_relocs") == 0 ) {
				switch ( parseTreatment(argv[++i]) ) {
					case kNULL:
					case kInvalid:
						throw "-read_only_relocs missing [ warning | error | suppress ]";
					case kWarning:
						fWarnTextRelocs = true;
						fAllowTextRelocs = true;
						cannotBeUsedWithBitcode(arg);
						break;
					case kSuppress:
						fWarnTextRelocs = false;
						fAllowTextRelocs = true;
						cannotBeUsedWithBitcode(arg);
						break;
					case kError:
						fWarnTextRelocs = false;
						fAllowTextRelocs = false;
						break;
				}
			}
			else if ( strcmp(arg, "-unaligned_pointers") == 0 ) {
				switch ( parseTreatment(argv[++i]) ) {
					case kNULL:
					case kInvalid:
						throw "-unaligned_pointers missing [ warning | error | suppress ]";
					case kWarning:
						fUnalignedPointerTreatment = Options::kUnalignedPointerWarning;
						cannotBeUsedWithBitcode(arg);
						break;
					case kSuppress:
						fUnalignedPointerTreatment = Options::kUnalignedPointerIgnore;
						cannotBeUsedWithBitcode(arg);
						break;
					case kError:
						fUnalignedPointerTreatment = Options::kUnalignedPointerError;
						break;
				}
			}
			else if ( strcmp(arg, "-sect_diff_relocs") == 0 ) {
				warnObsolete(arg);
				++i;
			}
			// Warn, error or make strong a mismatch between weak
			// and non-weak references.
			else if ( strcmp(arg, "-weak_reference_mismatches") == 0 ) {
                setWeakReferenceMismatchTreatment(argv[++i]);
			}
			// For a deployment target of 10.3 and earlier ld64 will
			// prebind an executable with 0s in all addresses that
			// are prebound.  This can then be fixed up by update_prebinding
			// later.  Prebinding is less useful on 10.4 and greater.
			else if ( strcmp(arg, "-prebind") == 0 ) {
				warnObsolete(arg);
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-noprebind") == 0 ) {
				warnObsolete(arg);
			}
			else if ( strcmp(arg, "-prebind_allow_overlap") == 0 ) {
				warnObsolete(arg);
			}
			else if ( strcmp(arg, "-prebind_all_twolevel_modules") == 0 ) {
				warnObsolete(arg);
			}
			else if ( strcmp(arg, "-noprebind_all_twolevel_modules") == 0 ) {
				warnObsolete(arg);
			}
			else if ( strcmp(arg, "-nofixprebinding") == 0 ) {
				warnObsolete(arg);
			}
			// This should probably be deprecated when we respect -L and -F
			// when searching for libraries.
			else if ( strcmp(arg, "-dylib_file") == 0 ) {
                // ignore for snapshot because a stub dylib will be created in the snapshot
                 snapshotArgCount = 0;
				 addDylibOverride(argv[++i]);
				cannotBeUsedWithBitcode(arg);
			}
			// What to expand @executable_path to if found in dependent dylibs
			else if ( strcmp(arg, "-executable_path") == 0 ) {
				 fExecutablePath = argv[++i];
				 if ( (fExecutablePath == NULL) || (fExecutablePath[0] == '-') )
					throw "-executable_path missing <path>";
				// if a directory was passed, add / to end
				// <rdar://problem/5171880> ld64 can't find @executable _path relative dylibs from our umbrella frameworks
				struct stat statBuffer;
				if ( stat(fExecutablePath, &statBuffer) == 0 ) {
					if ( (statBuffer.st_mode & S_IFMT) == S_IFDIR ) {
						char* pathWithSlash = new char[strlen(fExecutablePath)+2];
						strcpy(pathWithSlash, fExecutablePath);
						strcat(pathWithSlash, "/");
						fExecutablePath = pathWithSlash;
					}
				}
			}
			// Aligns all segments to the power of 2 boundary specified.
			else if ( strcmp(arg, "-segalign") == 0 ) {
				const char* size = argv[++i];
				if ( size == NULL )
					throw "-segalign missing <size>";
				fSegmentAlignment = parseAddress(size);
				uint8_t alignment = (uint8_t)__builtin_ctz(fSegmentAlignment);
				uint32_t p2aligned = (1 << alignment);
				if ( p2aligned != fSegmentAlignment ) {
					warning("alignment for -segalign %s is not a power of two, using 0x%X", size, p2aligned);
					fSegmentAlignment = p2aligned;
				}
				cannotBeUsedWithBitcode(arg);
			}
			// Puts a specified segment at a particular address that must
			// be a multiple of the segment alignment.
			else if ( strcmp(arg, "-segaddr") == 0 ) {
				SegmentStart seg;
				seg.name = argv[++i];
				 if ( (seg.name == NULL) || (argv[i+1] == NULL) )
					throw "-segaddr missing segName Adddress";
				seg.address = parseAddress(argv[++i]);
				uint64_t temp = ((seg.address+fSegmentAlignment-1) & (-fSegmentAlignment)); 
				if ( seg.address != temp )
					warning("-segaddr %s not %lld byte aligned", seg.name, fSegmentAlignment);
				fCustomSegmentAddresses.push_back(seg);
				cannotBeUsedWithBitcode(arg);
			}
			// ??? Deprecate when we deprecate split-seg.
			else if ( strcmp(arg, "-segs_read_only_addr") == 0 ) {
				warnObsolete(arg);
				++i;
				cannotBeUsedWithBitcode(arg);
			}
			// ??? Deprecate when we deprecate split-seg.
			else if ( strcmp(arg, "-segs_read_write_addr") == 0 ) {
				warnObsolete(arg);
				++i;
				cannotBeUsedWithBitcode(arg);
			}
			// ??? Deprecate when we get rid of basing at build time.
			else if ( strcmp(arg, "-seg_addr_table") == 0 ) {
				warnObsolete(arg);
                snapshotFileArgIndex = 1;
				const char* name = argv[++i];
				if ( name == NULL )
					throw "-seg_addr_table missing argument";
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-seg_addr_table_filename") == 0 ) {
				warnObsolete(arg);
				++i;
			}
			else if ( strcmp(arg, "-segprot") == 0 ) {
				SegmentProtect seg;
				seg.name = argv[++i];
				 if ( (seg.name == NULL) || (argv[i+1] == NULL) || (argv[i+2] == NULL) )
					throw "-segprot missing segName max-prot init-prot";
				seg.max = parseProtection(argv[++i]);
				seg.init = parseProtection(argv[++i]);
				if ( strcmp(seg.name, "__LINKEDIT") == 0 )
					warning("-segprot cannot be used to modify __LINKEDIT protections");
				else
					fCustomSegmentProtections.push_back(seg);
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-pagezero_size") == 0 ) {
				 const char* size = argv[++i];
				 if ( size == NULL )
					throw "-pagezero_size missing <size>";
				fZeroPageSize = parseAddress(size);
				uint64_t temp = fZeroPageSize & (-4096); // page align
				if ( (fZeroPageSize != temp)  )
					warning("-pagezero_size not page aligned, rounding down");
				 fZeroPageSize = temp;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-stack_addr") == 0 ) {
				 const char* address = argv[++i];
				 if ( address == NULL )
					throw "-stack_addr missing <address>";
				fStackAddr = parseAddress(address);
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-stack_size") == 0 ) {
				 const char* size = argv[++i];
				 if ( size == NULL )
					throw "-stack_size missing <address>";
				fStackSize = parseAddress(size);
			}
			else if ( strcmp(arg, "-allow_stack_execute") == 0 ) {
				fExecutableStack = true;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-allow_heap_execute") == 0 ) {
				fDisableNonExecutableHeap = true;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-sectalign") == 0 ) {
				 if ( (argv[i+1]==NULL) || (argv[i+2]==NULL) || (argv[i+3]==NULL) )
					throw "-sectalign missing <segment> <section> <hexadecimal-number>";
				addSectionAlignment(argv[i+1], argv[i+2], argv[i+3]);
				i += 3;
			}
			else if ( strcmp(arg, "-sectorder_detail") == 0 ) {
				warnObsolete(arg);
			}
			else if ( strcmp(arg, "-sectobjectsymbols") == 0 ) {
				warnObsolete(arg);
				i += 2;
			}
			else if ( strcmp(arg, "-bundle_loader") == 0 ) {
                snapshotFileArgIndex = 1;
				fBundleLoader = argv[++i];
				if ( (fBundleLoader == NULL) || (fBundleLoader[0] == '-') )
					throw "-bundle_loader missing <path>";
				FileInfo info = findFile(fBundleLoader);
				info.ordinal = ld::File::Ordinal::makeArgOrdinal((uint16_t)i);
				info.options.fBundleLoader = true;
				fInputFiles.push_back(info);
			}
			else if ( strcmp(arg, "-private_bundle") == 0 ) {
				warnObsolete(arg);
			}
			else if ( strcmp(arg, "-twolevel_namespace_hints") == 0 ) {
				warnObsolete(arg);
			}
			else if ( strcmp(arg, "-platform_version") == 0 ) {
				const char* platformStr = argv[++i];
				if (platformStr == nullptr) {
					throwf("platform and versions must be specified");
				}
				ld::Platform plat = ld::platformFromName(platformStr);
				if ( plat == ld::Platform::unknown )
					throwf("%s passed unknown platform name '%s'", arg, platformStr);
				const char* versStr = checkForNullVersionArgument(arg, argv[++i]);
				if ( versStr == NULL )
					throwf("%s missing min version argument", arg);
				uint32_t minVersValue;
				if ( !parsePackedVersion32(versStr, minVersValue) ) {
					throwf("%s min version malformed: '%s'", arg, versStr);
				}
				const char* SDKVersStr = checkForNullVersionArgument(arg, argv[++i]);
				if ( SDKVersStr == NULL )
					throwf("%s missing sdk version argument", arg);
				uint32_t SDKValue;
				if ( !parsePackedVersion32(SDKVersStr, SDKValue) ) {
					throwf("%s sdk version malformed: '%s'", arg, SDKVersStr);
				}
				if ( fPlatforms.contains(plat) ) {
					std::string existingVersionStr = getVersionString32(fPlatforms.minOS(plat));
					warning("passed two min versions (%s, %s) for platform %s. Using %s.",
							existingVersionStr.c_str(), versStr, nameFromPlatform(plat), versStr);
					fPlatforms.updateMinVersion(plat, minVersValue);
					fPlatforms.updateSDKVersion(plat, SDKValue);
				}
				else {
					fPlatforms.insert(ld::PlatformVersion(plat, minVersValue, SDKValue));
				}
				fPlatfromVersionCmdFound = true;
			}
			else if ( const ld::PlatformInfo* info = isPlatformOption(arg) ) {
				const char* versStr = checkForNullVersionArgument(arg, argv[++i]);
				if ( versStr == NULL )
					throwf("%s missing version argument", arg);
				uint32_t value;
				if ( !parsePackedVersion32(versStr, value) ) {
					throwf("%s value malformed: '%s'", arg, versStr);
				}
				if ( value < info->minimumOsVersion ) {
					if ( info->platform == ld::Platform::iOSMac ) {
						value = info->minimumOsVersion;
						warning("changing %s minOS version from %s to %s", info->printName, versStr, getVersionString32(info->minimumOsVersion).c_str());
					}
					else {
						warning("building for %s %s is deprecated", info->printName, versStr);
					}
				}
				if (fPlatforms.contains(info->platform)) {
					std::string existingVersionStr = getVersionString32(fPlatforms.minOS(info->platform));
					warning("passed two min versions (%s, %s) for platform %s. Using %s.",
							existingVersionStr.c_str(), versStr, info->printName, versStr);
					fPlatforms.updateMinVersion(info->platform, value);
				} else {
					fPlatforms.insert(ld::PlatformVersion(info->platform, value));
				}
			}
			else if ( strcmp(arg, "-multiply_defined") == 0 ) {
				//warnObsolete(arg);
				++i;
			}
			else if ( strcmp(arg, "-multiply_defined_unused") == 0 ) {
				warnObsolete(arg);
				++i;
			}
			else if ( strcmp(arg, "-nomultidefs") == 0 ) {
				warnObsolete(arg);
			}
			// Display each file in which the argument symbol appears and whether
			// the file defines or references it.  This option takes an argument
			// as -y<symbol> note that there is no space.
			else if ( strncmp(arg, "-y", 2) == 0 ) {
				warnObsolete("-y");
			}
			// Same output as -y, but output <arg> number of undefined symbols only.
			else if ( strcmp(arg, "-Y") == 0 ) {
				//warnObsolete(arg);
				++i;
			}
			// This option affects all objects linked into the final result.
			else if ( strcmp(arg, "-m") == 0 ) {
				warnObsolete(arg);
			}
			else if ( (strcmp(arg, "-why_load") == 0) || (strcmp(arg, "-whyload") == 0) ) {
				 fWhyLoad = true;
			}
			else if ( strcmp(arg, "-why_live") == 0 ) {
				 const char* name = argv[++i];
				if ( name == NULL )
					throw "-why_live missing symbol name argument";
				fWhyLive.insert(name, kAllowWildcards);
			}
			else if ( strcmp(arg, "-u") == 0 ) {
				const char* name = argv[++i];
				if ( name == NULL )
					throw "-u missing argument";
				fInitialUndefines.push_back(name);
			}
			else if ( strcmp(arg, "-U") == 0 ) {
				const char* name = argv[++i];
				if ( name == NULL )
					throw "-U missing argument";
				fAllowedUndefined.insert(name);
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-s") == 0 ) {
				warnObsolete(arg);
				fLocalSymbolHandling = kLocalSymbolsNone;
				fDebugInfoStripping = Options::kDebugInfoNone;
			}
			else if ( strcmp(arg, "-x") == 0 ) {
				fLocalSymbolHandling = kLocalSymbolsNone;
			}
			else if ( strcmp(arg, "-S") == 0 ) {
				fDebugInfoStripping = Options::kDebugInfoNone;
			}
			else if ( strcmp(arg, "-X") == 0 ) {
				warnObsolete(arg);
			}
			else if ( strcmp(arg, "-Si") == 0 ) {
				warnObsolete(arg);
				fDebugInfoStripping = Options::kDebugInfoFull;
			}
			else if ( strcmp(arg, "-b") == 0 ) {
				warnObsolete(arg);
			}
			else if ( strcmp(arg, "-Sn") == 0 ) {
				warnObsolete(arg);
				fDebugInfoStripping = Options::kDebugInfoFull;
			}
			else if ( strcmp(arg, "-Sp") == 0 ) {
				warnObsolete(arg);
			}
			else if ( strcmp(arg, "-dead_strip") == 0 ) {
				fDeadStrip = true;
			}
			else if ( strcmp(arg, "-no_dead_strip_inits_and_terms") == 0 ) {
				fDeadStrip = true;
			}
			else if ( strcmp(arg, "-w") == 0 ) {
				// previously handled by buildSearchPaths()
			}
			else if ( strcmp(arg, "-fatal_warnings") == 0 ) {
				// previously handled by buildSearchPaths()
			}
			else if ( strcmp(arg, "-arch_errors_fatal") == 0 ) {
				fErrorOnOtherArchFiles = true;
			}
			else if ( strcmp(arg, "-M") == 0 ) {
				// FIX FIX
			}
			else if ( strcmp(arg, "-headerpad") == 0 ) {
				const char* size = argv[++i];
				if ( size == NULL )
					throw "-headerpad missing argument";
				 fMinimumHeaderPad = parseAddress(size);
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-headerpad_max_install_names") == 0 ) {
				// ignore -headerpad_max_install_names when compiling with bitcode
				// rdar://problem/20748962
				if ( fBundleBitcode )
					warning("-headerpad_max_install_names is ignored when used with -bitcode_bundle (Xcode setting ENABLE_BITCODE=YES)");
				else
					fMaxMinimumHeaderPad = true;
			}
			else if ( strcmp(arg, "-t") == 0 ) {
				fLogAllFiles = true;
			}
			else if ( strcmp(arg, "-whatsloaded") == 0 ) {
				fLogObjectFiles = true;
			}
			else if ( strcmp(arg, "-A") == 0 ) {
				warnObsolete(arg);
				 ++i;
			}
			else if ( strcmp(arg, "-umbrella") == 0 ) {
				const char* name = argv[++i];
				if ( name == NULL )
					throw "-umbrella missing argument";
				fUmbrellaName = name;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-allowable_client") == 0 ) {
				const char* name = argv[++i];

				if ( name == NULL )
					throw "-allowable_client missing argument";

				fAllowableClients.push_back(name);
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-client_name") == 0 ) {
				const char* name = argv[++i];

				if ( name == NULL )
					throw "-client_name missing argument";

				fClientName = name;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-sub_umbrella") == 0 ) {
				const char* name = argv[++i];
				if ( name == NULL )
					throw "-sub_umbrella missing argument";
				 fSubUmbellas.push_back(name);
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-sub_library") == 0 ) {
				const char* name = argv[++i];
				if ( name == NULL )
					throw "-sub_library missing argument";
				 fSubLibraries.push_back(name);
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-init") == 0 ) {
				const char* name = argv[++i];
				if ( name == NULL )
					throw "-init missing argument";
				fInitFunctionName = name;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-dot") == 0 ) {
				const char* name = argv[++i];
				if ( name == NULL )
					throw "-dot missing argument";
				fDotOutputFile = name;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-warn_commons") == 0 ) {
				fWarnCommons = true;
			}
			else if ( strcmp(arg, "-commons") == 0 ) {
				fCommonsMode = parseCommonsTreatment(argv[++i]);
			}
			else if ( strcmp(arg, "-keep_relocs") == 0 ) {
				fKeepRelocations = true;
			}
			else if ( strcmp(arg, "-warn_stabs") == 0 ) {
				fWarnStabs = true;
			}
			else if ( strcmp(arg, "-pause") == 0 ) {
				fPause = true;
			}
			else if ( strcmp(arg, "-print_statistics") == 0 ) {
				fStatistics = true;
			}
			else if ( strcmp(arg, "-d") == 0 ) {
				fMakeTentativeDefinitionsReal = true;
			}
			else if ( strcmp(arg, "-v") == 0 ) {
				// previously handled by buildSearchPaths()
			}
			else if ( strcmp(arg, "-Z") == 0 ) {
				// previously handled by buildSearchPaths()
			}
			else if ( strcmp(arg, "-syslibroot") == 0 ) {
                snapshotArgCount = 0;
				++i;
				// previously handled by buildSearchPaths()
			}
			else if ( strcmp(arg, "-bitcode_bundle") == 0 ) {
				snapshotArgCount = 0;
				// previously handled by buildSearchPaths()
			}
			else if ( strcmp(arg, "-no_uuid") == 0 ) {
				fUUIDMode = kUUIDNone;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-random_uuid") == 0 ) {
				fUUIDMode = kUUIDRandom;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-dtrace") == 0 ) {
                snapshotFileArgIndex = 1;
				const char* name = argv[++i];
				if ( name == NULL )
					throw "-dtrace missing argument";
				fDtraceScriptName = name;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-root_safe") == 0 ) {
				fRootSafe = true;
			}
			else if ( strcmp(arg, "-setuid_safe") == 0 ) {
				fSetuidSafe = true;
			}
			else if ( strcmp(arg, "-alias") == 0 ) {
				Options::AliasPair pair;
				pair.realName = checkForNullArgument(arg, argv[++i]);
				pair.alias = checkForNullArgument(arg, argv[++i]);
				fAliases.push_back(pair);
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-alias_list") == 0 ) {
                snapshotFileArgIndex = 1;
				parseAliasFile(argv[++i]);
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-save-temps") == 0 ) {
				fSaveTempFiles = true;
			}
			else if ( strcmp(arg, "-bitcode_hide_symbols") == 0 ) {
				fHideSymbols = true;
			}
			else if ( strcmp(arg, "-bitcode_verify") == 0 ) {
				fVerifyBitcode = true;
			}
			else if ( strcmp(arg, "-bitcode_symbol_map") == 0) {
				fReverseMapPath = checkForNullArgument(arg, argv[++i]);
				struct stat statbuf;
				int ret = ::stat(fReverseMapPath, &statbuf);
				if ( ret == 0 && S_ISDIR(statbuf.st_mode)) {
					char tempPath[PATH_MAX];
					sprintf(tempPath, "%s/XXXXXX", fReverseMapPath);
					int tempFile = ::mkstemp(tempPath);
					if (tempFile == -1)
						throwf("could not write file to symbol map directory: %s", fReverseMapPath);
					::close(tempFile);
					fReverseMapTempPath = std::string(tempPath);
					fReverseMapUUIDRename = true;
				} else
					fReverseMapTempPath = std::string(fReverseMapPath);
			}
			else if ( strcmp(arg, "-flto-codegen-only") == 0) {
				fLTOCodegenOnly = true;
			}
			else if ( strcmp(arg, "-ignore_auto_link") == 0) {
				fIgnoreAutoLink = true;
			}
			else if ( strcmp(arg, "-allow_dead_duplicates") == 0) {
				fAllowDeadDups = true;
			}
			else if ( strcmp(arg, "-bitcode_process_mode") == 0 ) {
				const char* bitcode_type = checkForNullArgument(arg, argv[++i]);
				if ( strcmp(bitcode_type, "strip") == 0 )
					fBitcodeKind = kBitcodeStrip;
				else if ( strcmp(bitcode_type, "marker") == 0 )
					fBitcodeKind = kBitcodeMarker;
				else if ( strcmp(bitcode_type, "data") == 0 )
					fBitcodeKind = kBitcodeAsData;
				else if ( strcmp(bitcode_type, "bitcode") == 0 )
					fBitcodeKind = kBitcodeProcess;
				else
					throw "unknown argument to -bitcode_process_mode {strip,marker,data,bitcode}";
			}
			else if ( strcmp(arg, "-rpath") == 0 ) {
				const char* path = checkForNullArgument(arg, argv[++i]);
				fRPaths.push_back(path);
			}
			else if ( strcmp(arg, "-read_only_stubs") == 0 ) {
				fReadOnlyx86Stubs = true;
			}
			else if ( strcmp(arg, "-slow_stubs") == 0 ) {
				warnObsolete(arg);
			}
			else if ( strcmp(arg, "-map") == 0 ) {
				fMapPath = checkForNullArgument(arg, argv[++i]);
			}
			else if ( strcmp(arg, "-pie") == 0 ) {
				fPositionIndependentExecutable = true;
				fPIEOnCommandLine = true;
			}
			else if ( strcmp(arg, "-no_pie") == 0 ) {
				fDisablePositionIndependentExecutable = true;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strncmp(arg, "-reexport-l", 11) == 0 ) {
				FileInfo info = findLibrary(&arg[11], true);
				info.options.fReExport = true;
				info.ordinal = ld::File::Ordinal::makeArgOrdinal((uint16_t)i);
				addLibrary(info);
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-reexport_library") == 0 ) {
				const char* path = checkForNullArgument(arg, argv[++i]);
				FileInfo info = findFile(path);
				info.options.fReExport = true;
				info.ordinal = ld::File::Ordinal::makeArgOrdinal((uint16_t)i);
				addLibrary(info);
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-reexport_framework") == 0 ) {
                // SNAPSHOT FIXME: what should we do for link snapshots? (ignore for now)
                snapshotArgCount = 0;
				FileInfo info = findFramework(argv[++i]);
				info.options.fReExport = true;
				info.ordinal = ld::File::Ordinal::makeArgOrdinal((uint16_t)i);
				addLibrary(info);
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strncmp(arg, "-upward-l", 9) == 0 ) {
				FileInfo info = findLibrary(&arg[9], true);
				info.options.fUpward = true;
				info.ordinal = ld::File::Ordinal::makeArgOrdinal((uint16_t)i);
				addLibrary(info);
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-upward_library") == 0 ) {
				const char* path = checkForNullArgument(arg, argv[++i]);
				FileInfo info = findFile(path);
				info.options.fUpward = true;
				info.ordinal = ld::File::Ordinal::makeArgOrdinal((uint16_t)i);
				addLibrary(info);
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-upward_framework") == 0 ) {
				FileInfo info = findFramework(argv[++i]);
				info.options.fUpward = true;
				info.ordinal = ld::File::Ordinal::makeArgOrdinal((uint16_t)i);
				addLibrary(info);
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-dead_strip_dylibs") == 0 ) {
				fDeadStripDylibs = true;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-no_implicit_dylibs") == 0 ) {
				fImplicitlyLinkPublicDylibs = false;
			}
			else if ( strcmp(arg, "-new_linker") == 0 ) {
				// ignore
			}
			else if ( strcmp(arg, "-no_encryption") == 0 ) {
				fEncryptableForceOff = true;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-encryptable") == 0 ) {
				fEncryptableForceOn = true;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-no_compact_unwind") == 0 ) {
				fAddCompactUnwindEncoding = false;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-mllvm") == 0 ) {
				const char* opts = checkForNullArgument(arg, argv[++i], true);
				fLLVMOptions.push_back(opts);
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-mcpu") == 0 ) {
				const char* cpu = checkForNullArgument(arg, argv[++i]);
				fLtoCpu = cpu;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-no_order_inits") == 0 ) {
				fAutoOrderInitializers = false;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-no_order_data") == 0 ) {
				fOrderData = false;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-seg_page_size") == 0 ) {
				SegmentSize seg;
				seg.name = argv[++i];
				 if ( (seg.name == NULL) || (argv[i+1] == NULL) )
					throw "-seg_page_size missing segName Adddress";
				seg.size = parseAddress(argv[++i]);
				uint64_t temp = seg.size & (-4096); // page align
				if ( (seg.size != temp)  )
					warning("-seg_page_size %s not 4K aligned, rounding down", seg.name);
				fCustomSegmentSizes.push_back(seg);
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-mark_dead_strippable_dylib") == 0 ) {
				fMarkDeadStrippableDylib = true;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-exported_symbols_order") == 0 ) {
                snapshotFileArgIndex = 1;
				loadSymbolOrderFile(argv[++i], fExportSymbolsOrder);
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-no_compact_linkedit") == 0 ) {
				warnObsolete("-no_compact_linkedit");
			}
			else if ( strcmp(arg, "-no_eh_labels") == 0 ) {
				fNoEHLabels = true;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-warn_compact_unwind") == 0 ) {
				fWarnCompactUnwind = true;
			}
			else if ( strcmp(arg, "-allow_sub_type_mismatches") == 0 ) {
				fAllowCpuSubtypeMismatches = true;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-no_zero_fill_sections") == 0 ) {
				fOptimizeZeroFill = false;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-merge_zero_fill_sections") == 0 ) {
				fMergeZeroFill = true;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-objc_abi_version") == 0 ) {
				const char* version = checkForNullVersionArgument(arg, argv[++i]);
				if ( strcmp(version, "2") == 0 ) {
					fObjCABIVersion1Override = false;
					fObjCABIVersion2Override = true;
				}
				else if ( strcmp(version, "1") == 0 ) {
					fObjCABIVersion1Override = true;
					fObjCABIVersion2Override = false;
				}
				else
					warning("ignoring unrecognized argument (%s) to -objc_abi_version", version);
			}
			else if ( strcmp(arg, "-warn_unused_dylibs") == 0 ) {
				fWarnUnusedDylibsForceOn = true;
			}
			else if ( strcmp(arg, "-no_warn_unused_dylibs") == 0 ) {
				fWarnUnusedDylibsForceOff = true;
			}
			else if ( strcmp(arg, "-objc_relative_method_lists") == 0 ) {
				fForceObjCRelativeMethodListsOn = true;
			}
			else if ( strcmp(arg, "-no_objc_relative_method_lists") == 0 ) {
				fForceObjCRelativeMethodListsOff = true;
			}
			else if ( strcmp(arg, "-warn_weak_exports") == 0 ) {
				fWarnWeakExports = true;
			}
			else if ( strcmp(arg, "-no_weak_exports") == 0 ) {
				fNoWeakExports = true;
			}
			else if ( strcmp(arg, "-objc_gc_compaction") == 0 ) {
				fObjcGcCompaction = true;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-objc_gc") == 0 ) {
				fObjCGc = true;
				if ( fObjCGcOnly ) {
					warning("-objc_gc overriding -objc_gc_only");
					fObjCGcOnly = false;
				}
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-objc_gc_only") == 0 ) {
				fObjCGcOnly = true;
				if ( fObjCGc ) {
					warning("-objc_gc_only overriding -objc_gc");
					fObjCGc = false;
				}
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-demangle") == 0 ) {
				fDemangle = true;
			}
			else if ( strcmp(arg, "-version_load_command") == 0 ) {
				fVersionLoadCommandForcedOn = true;
				fVersionLoadCommandForcedOff = false;
			}
			else if ( strcmp(arg, "-function_starts") == 0 ) {
				fFunctionStartsForcedOn = true;
				fFunctionStartsForcedOff = false;
			}
			else if ( strcmp(arg, "-no_function_starts") == 0 ) {
				fFunctionStartsForcedOff = true;
				fFunctionStartsForcedOn = false;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-no_data_in_code_info") == 0 ) {
				fDataInCodeInfoLoadCommandForcedOff = true;
				fDataInCodeInfoLoadCommandForcedOn = false;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-data_in_code_info") == 0 ) {
				fDataInCodeInfoLoadCommandForcedOn  = true;
				fDataInCodeInfoLoadCommandForcedOff = false;
			}
			else if ( strcmp(arg, "-object_path_lto") == 0 ) {
				fTempLtoObjectPath = checkForNullArgument(arg, argv[++i]);
			}
			else if ( strcmp(arg, "-no_objc_category_merging") == 0 ) {
				fObjcCategoryMerging = false;
			}
			else if ( strcmp(arg, "-force_symbols_weak_list") == 0 ) {
                snapshotFileArgIndex = 1;
				loadExportFile(argv[++i], "-force_symbols_weak_list", fForceWeakSymbols, kAllowWildcards);
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-force_symbols_not_weak_list") == 0 ) {
                snapshotFileArgIndex = 1;
				loadExportFile(argv[++i], "-force_symbols_not_weak_list", fForceNotWeakSymbols, kAllowWildcards);
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-force_symbol_weak") == 0 ) {
				const char* symbol = argv[++i];
				if ( symbol == NULL )
					throw "-force_symbol_weak missing <symbol>";
				fForceWeakSymbols.insert(symbol, kAllowWildcards);
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-force_symbol_not_weak") == 0 ) {
				const char* symbol = argv[++i];
				if ( symbol == NULL )
					throw "-force_symbol_not_weak missing <symbol>";
				fForceNotWeakSymbols.insert(symbol, kAllowWildcards);
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-reexported_symbols_list") == 0 ) {
                snapshotFileArgIndex = 1;
				if ( fExportMode == kExportSome )
					throw "can't use -exported_symbols_list and -reexported_symbols_list";
				loadExportFile(argv[++i], "-reexported_symbols_list", fReExportSymbols, kAllowWildcards);
			}
			else if ( strcmp(arg, "-dyld_env") == 0 ) {
				const char* envarg = argv[++i];
				if ( envarg == NULL )
					throw "-dyld_env missing ENV=VALUE";
				if ( strchr(envarg, '=') == NULL )
					throw "-dyld_env missing ENV=VALUE";
				fDyldEnvironExtras.push_back(envarg);
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-page_align_data_atoms") == 0 ) {
				fPageAlignDataAtoms = true;
				cannotBeUsedWithBitcode(arg);
			} 
			else if (strcmp(arg, "-debug_snapshot") == 0) {
                fLinkSnapshot.setSnapshotMode(Snapshot::SNAPSHOT_DEBUG);
                fSnapshotRequested = true;
				cannotBeUsedWithBitcode(arg);
            }
 			else if (strcmp(arg, "-snapshot_dir") == 0) {
				const char* path = argv[++i];
				if ( path == NULL )
					throw "-snapshot_dir missing path";
				fLinkSnapshot.setSnapshotMode(Snapshot::SNAPSHOT_DEBUG);
				fLinkSnapshot.setSnapshotPath(path);
				fSnapshotRequested = true;
				cannotBeUsedWithBitcode(arg);
            }
			else if ( strcmp(arg, "-source_version") == 0 ) {
				 const char* vers = argv[++i];
				 if ( vers == NULL )
					throw "-source_version missing <version>";
				fSourceVersion = parseVersionNumber64(vers);
			}
			else if ( strcmp(arg, "-add_source_version") == 0 ) {
				fSourceVersionLoadCommandForceOn = true;
			}
			else if ( strcmp(arg, "-no_source_version") == 0 ) {
				fSourceVersionLoadCommandForceOff = true;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-sdk_version") == 0 ) {
				 const char* vers = argv[++i];
				 if ( vers == NULL )
					throw "-sdk_version missing <version>";
				fSDKVersion = parseVersionNumber32(vers);
			}
			else if ( strcmp(arg, "-dependent_dr_info") == 0 ) {
				warnObsolete(arg);
			}
			else if ( strcmp(arg, "-no_dependent_dr_info") == 0 ) {
				warnObsolete(arg);
			}
			else if ( strcmp(arg, "-kexts_use_stubs") == 0 ) {
				fKextsUseStubs = true;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-dependency_info") == 0 ) {
                snapshotArgCount = 0;
				++i;
				// previously handled by buildSearchPaths()
			}
			else if ( strcmp(arg, "-export_dynamic") == 0 ) {
				fExportDynamic = true;
			}
			else if ( strcmp(arg, "-force_symbols_coalesce_list") == 0 ) {
                snapshotFileArgIndex = 1;
				loadExportFile(argv[++i], "-force_symbols_coalesce_list", fForceCoalesceSymbols, kAllowWildcards);
			}
			else if ( strcmp(arg, "-add_linker_option") == 0 ) {
				// ex: -add_linker_option '-framework Foundation'
				const char* optString = argv[++i];
				if ( optString == NULL )
					throw "-add_linker_option missing <option>";
				// break up into list of tokens at whitespace
				std::vector<const char*> opts;
				char* buffer = strdup(optString);
				char* start = buffer;
				for (char* s = buffer; ; ++s) {
					if ( isspace(*s)  ) {
						*s = '\0';
						opts.push_back(start);
						start = s+1;
					}
					else if ( *s == '\0' ) {
						opts.push_back(start);
						break;
					}
				}
				fLinkerOptions.push_back(opts);
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-allow_simulator_linking_to_macosx_dylibs") == 0 ) {
				fAllowSimulatorToLinkWithMacOSX = true;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-simulator_support") == 0 ) {
				fSimulatorSupportDylib = true;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-keep_dwarf_unwind") == 0 ) {
				fKeepDwarfUnwindForcedOn = true;
				fKeepDwarfUnwindForcedOff = false;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-no_keep_dwarf_unwind") == 0 ) {
				fKeepDwarfUnwindForcedOn = false;
				fKeepDwarfUnwindForcedOff = true;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-verbose_optimization_hints") == 0 ) {
				fVerboseOptimizationHints = true;
			}
			else if ( strcmp(arg, "-ignore_optimization_hints") == 0 ) {
				fIgnoreOptimizationHints = true;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-no_dtrace_dof") == 0 ) {
				fGenerateDtraceDOF = false;
			}
			else if ( strcmp(arg, "-rename_section") == 0 ) {
				 if ( (argv[i+1]==NULL) || (argv[i+2]==NULL) || (argv[i+3]==NULL) || (argv[i+4]==NULL) )
					throw "-rename_section missing <segment> <section> <segment> <section>";
				addSectionRename(argv[i+1], argv[i+2], argv[i+3], argv[i+4]);
				i += 4;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-rename_segment") == 0 ) {
				 if ( (argv[i+1]==NULL) || (argv[i+2]==NULL) )
					throw "-rename_segment missing <existing-segment> <new-segment>";
				addSegmentRename(argv[i+1], argv[i+2]);
				i += 2;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-move_to_ro_segment") == 0 ) {
				 if ( (argv[i+1]==NULL) || (argv[i+2]==NULL) )
					throw "-move_to_ro_segment missing <segment> <symbol-list-file>";
				addSymbolMove(argv[i+1], argv[i+2], fSymbolsMovesCode, "-move_to_ro_segment", kAllowWildcards);
				i += 2;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-move_to_rw_segment") == 0 ) {
				 if ( (argv[i+1]==NULL) || (argv[i+2]==NULL) )
					throw "-move_to_rw_segment missing <segment> <symbol-list-file>";
				addSymbolMove(argv[i+1], argv[i+2], fSymbolsMovesData, "-move_to_rw_segment", kAllowWildcards);
				i += 2;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-trace_symbol_layout") == 0 ) {
				fTraceSymbolLayout = true;
			}
			else if ( strcmp(arg, "-no_branch_islands") == 0 ) {
				fAllowBranchIslands = false;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-segment_order") == 0 ) {
				// ex: -segment_order __TEXT:__DATA:__JUNK
				const char* optString = argv[++i];
				if ( optString == NULL )
					throw "-segment_order missing colon separated <segment-list>";
				if ( !fSegmentOrder.empty() )
					throw "-segment_order used more than once";
				// break up into list of tokens at colon
				char* buffer = strdup(optString);
				char* start = buffer;
				for (char* s = buffer; ; ++s) {
					if ( *s == ':'  ) {
						*s = '\0';
						fSegmentOrder.push_back(start);
						start = s+1;
					}
					else if ( *s == '\0' ) {
						fSegmentOrder.push_back(start);
						break;
					}
				}
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-section_order") == 0 ) {
				// ex: -section_order __DATA  __data:__const:__nl_pointers
				 if ( (argv[i+1]==NULL) || (argv[i+2]==NULL) )
					throw "-section_order missing <segment> <section-list>";
				const char* segName = argv[++i];
				const char* optString = argv[++i];
				if ( sectionOrder(segName) != NULL )
					throwf("-section_order %s ... used more than once", segName);
				SectionOrderList dummy;
				fSectionOrder.push_back(dummy);
				SectionOrderList& entry = fSectionOrder.back();
				entry.segmentName = segName;
				// break up into list of tokens at colon
				char* buffer = strdup(optString);
				char* start = buffer;
				for (char* s = buffer; ; ++s) {
					if ( *s == ':'  ) {
						*s = '\0';
						entry.sectionOrder.push_back(start);
						start = s+1;
					}
					else if ( *s == '\0' ) {
						entry.sectionOrder.push_back(start);
						break;
					}
				}
				cannotBeUsedWithBitcode(arg);
			}			
			else if ( strcmp(arg, "-application_extension") == 0 ) {
				fMarkAppExtensionSafe = true;
				fCheckAppExtensionSafe = true;
			}
			else if ( strcmp(arg, "-no_application_extension") == 0 ) {
				fMarkAppExtensionSafe = false;
				fCheckAppExtensionSafe = false;
			}
			else if ( strcmp(arg, "-add_ast_path") == 0 ) {
				const char* path = argv[++i];
				if ( path == NULL )
					throw "-add_ast_path missing <option>";
				fASTFilePaths.push_back(path);
			}
			else if ( strcmp(arg, "-force_load_swift_libs") == 0 ) {
				fForceLoadSwiftLibs = true;
			}
			else if ( strcmp(arg, "-not_for_dyld_shared_cache") == 0 ) {
				fSharedRegionEligibleForceOff = true;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-dirty_data_list") == 0 ) {
				 if ( argv[i+1] == NULL )
					throw "-dirty_data_list missing <symbol-list-file>";
				addSymbolMove("__DATA_DIRTY", argv[i+1], fSymbolsMovesData, "-dirty_data_list", kAllowWildcards);
				++i;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-data_const") == 0 ) {
				fUseDataConstSegmentForceOn = true;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-no_data_const") == 0 ) {
				fUseDataConstSegmentForceOff = true;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-text_exec") == 0 ) {
				fUseTextExecSegment = true;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-add_split_seg_info") == 0) {
				fSharedRegionEligible = true;
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-no_auth_data") == 0 ) {
#if SUPPORT_ARCH_arm64e
				fUseAuthDataSegmentForceOff = true;
#endif
				cannotBeUsedWithBitcode(arg);
			}
			else if ( strcmp(arg, "-no_deduplicate") == 0 ) {
				fDeDupe = false;
			}
			else if ( strcmp(arg, "-verbose_deduplicate") == 0 ) {
				fVerboseDeDupe = true;
			}
			else if ( strcmp(arg, "-max_default_common_align") == 0 ) {
				const char* alignStr = argv[++i];
				if ( alignStr == NULL )
					throw "-max_default_common_align missing <align-value>";
				// argument is a hexadecimal number
				char* endptr;
				unsigned long value = strtoul(alignStr, &endptr, 16);
				if ( *endptr != '\0')
					throw "argument for -max_default_common_align is not a hexadecimal number";
				if ( value > 0x8000 )
					throw "argument for -max_default_common_align must be less than or equal to 0x8000";
				if ( value == 0 ) {
					warning("zero is not a valid -max_default_common_align");
					value = 1;
				}
				// alignment is power of 2 
				uint8_t alignment = (uint8_t)__builtin_ctz(value);
				if ( (unsigned long)(1 << alignment) != value ) {
					warning("alignment for -max_default_common_align is not a power of two, using 0x%X", 1 << alignment);
				}
				fMaxDefaultCommonAlign = alignment;
			}
			else if ( strcmp(arg, "-no_weak_imports") == 0 ) {
				fAllowWeakImports = false;
			}
			else if ( strcmp(arg, "-no_inits") == 0 ) {
				fInitializersTreatment = Options::kError;
			}
			else if ( strcmp(arg, "-no_warn_inits") == 0 ) {
				fInitializersTreatment = Options::kSuppress;
			}
			else if ( strcmp(arg, "-threaded_starts_section") == 0 ) {
				fMakeThreadedStartsSection = true;
			}
			else if ( strcmp(arg, "-fixup_chains_section") == 0 ) {
				fMakeChainedFixups = true;
				fMakeChainedFixupsSection = true;
			}
			else if ( strcmp(arg, "-fixup_chains") == 0 ) {
				fMakeChainedFixups = true;
			}
			else if (strcmp(arg, "-debug_variant") == 0) {
			    fDebugVariant = true;
            }
			else if (strcmp(arg, "-no_new_main") == 0) {
				// HACK until 39514191 is fixed
			}
			else if (strcmp(arg, "-init_offsets") == 0) {
				fMakeInitializersIntoOffsets = true;
			}
			else if (strcmp(arg, "-adhoc_codesign") == 0) {
				fAdHocSign = true;
			}
			else if (strcmp(arg, "-oso_prefix") == 0) {
				const char* path = argv[++i];
				if ( path == NULL )
					throw "-oso_prefix missing <path>";
				fOSOPrefixPath = path;
			}
			// put this last so that it does not interfer with other options starting with 'i'
			else if ( strncmp(arg, "-i", 2) == 0 ) {
				const char* colon = strchr(arg, ':');
				if ( colon == NULL )
					throwf("unknown option: %s", arg);
				Options::AliasPair pair;
				char* temp = new char[colon-arg];
				strlcpy(temp, &arg[2], colon-arg-1);
				pair.realName = &colon[1];
				pair.alias = temp;
				fAliases.push_back(pair);
			}
			else {
				throwf("unknown option: %s", arg);
			}
            
            if (snapshotArgCount == -1)
                snapshotArgCount = i-snapshotArgIndex+1;
            if (snapshotArgCount > 0)
                fLinkSnapshot.addSnapshotLinkArg(snapshotArgIndex, snapshotArgCount, snapshotFileArgIndex);
		}
		else {
			FileInfo info = findFile(arg);
			info.ordinal = ld::File::Ordinal::makeArgOrdinal((uint16_t)i);
			if ( strcmp(&info.path[strlen(info.path)-2], ".a") == 0 )
				addLibrary(info);
			else
				fInputFiles.push_back(info);
		}
	}
	    
	if (fSnapshotRequested)
		fLinkSnapshot.createSnapshot();

	if ( (fOutputKind == kKextBundle) && !platforms().contains(ld::Platform::macOS) ) {
		if ( fKextObjectsEnable < 0 )
			fKextObjectsEnable = ((fArchitecture == CPU_TYPE_ARM64) || (fArchitecture == CPU_TYPE_ARM));

		if (fKextObjectsEnable > 0) {
			if ( !fKextObjectsDirPath ) {
				const char* dstroot;
				const char* objdir = getenv("LD_KEXT_OBJECTS_DIR");
				if ( objdir )
					fKextObjectsDirPath = strdup(objdir);
				else if ( (dstroot = getenv("DSTROOT")) )
					asprintf((char **)&fKextObjectsDirPath, "%s/AppleInternal/KextObjects", dstroot);
			}
			fLinkSnapshot.setSnapshotMode(Snapshot::SNAPSHOT_KEXT);
			fLinkSnapshot.createSnapshot();
		}
	}
}

bool Options::shouldUseBuildVersion(ld::Platform plat, uint32_t minOSvers) const
{
	// Allow B&I to force old load commands
	if ( forceLegacyVersionLoadCommands() )
		return false;

#if SUPPORT_ARCH_arm64 || SUPPORT_ARCH_arm64e
	// all arm64 variants are new and use LC_BUILD_VERSION
	if ( fArchitecture == CPU_TYPE_ARM64 ) {
		// except for pre-12.0 iOS and tvOS devices
		if ( ((plat == ld::Platform::iOS) || (plat == ld::Platform::tvOS)) && (minOSvers < 0x000C0000) )
			return false;
		return true;
	}
#endif

	// three libsystem simulator support dylibs need to use old load commands to work with old simulators
	if ( isSimulatorSupportDylib() && (plat != ld::Platform::iOSMac) )
		return false;

	// otherwise, use LC_BUILD_VERSION if minOS is new enough
	return (minOSvers >= platformInfo(plat).firstOsVersionUsingBuildVersionLC);
}

//
// -syslibroot <path> is used for SDK support.
// The rule is that all search paths (both explicit and default) are
// checked to see if they exist in the SDK.  If so, that path is
// replaced with the sdk prefixed path.  If not, that search path
// is used as is.  If multiple -syslibroot options are specified
// their directory structures are logically overlayed and files
// from sdks specified earlier on the command line used before later ones.

void Options::buildSearchPaths(int argc, const char* argv[])
{
	bool addStandardLibraryDirectories = true;
	ld::Platform platform = ld::Platform::unknown;
	std::vector<const char*> libraryPaths;
	std::vector<const char*> frameworkPaths;
	libraryPaths.reserve(10);
	frameworkPaths.reserve(10);
	// scan through argv looking for -L, -F, -Z, and -syslibroot options
	for(int i=0; i < argc; ++i) {
		if ( (argv[i][0] == '-') && (argv[i][1] == 'L') ) {
			const char* libSearchDir = &argv[i][2];
			// Allow either "-L{path}" or "-L {path}".
			if (argv[i][2] == '\0') {
				// -L {path}.  Make sure there is an argument following this.
				const char* path = argv[++i];
				if ( path == NULL )
					throw "-L missing argument";
				libSearchDir = path;
			}
			if ( libSearchDir[0] == '\0' ) 
				throw "-L must be immediately followed by a directory path (no space)";
			libraryPaths.push_back(libSearchDir);
		}
		else if ( (argv[i][0] == '-') && (argv[i][1] == 'F') ) {
			const char* frameworkSearchDir = &argv[i][2];
			// Allow either "-F{path}" or "-F {path}".
			if (argv[i][2] == '\0') {
				// -F {path}.  Make sure there is an argument following this.
				const char* path = argv[++i];
				if ( path == NULL )
					throw "-F missing argument";
				frameworkSearchDir = path;
			}
			if ( frameworkSearchDir[0] == '\0' ) 
				throw "-F must be immediately followed by a directory path (no space)";
			frameworkPaths.push_back(frameworkSearchDir);
		}
		else if ( strcmp(argv[i], "-Z") == 0 )
			addStandardLibraryDirectories = false;
		else if ( strcmp(argv[i], "-v") == 0 ) {
			fVerbose = true;
			extern const char ldVersionString[];
			fprintf(stderr, "%s", ldVersionString);
			fprintf(stderr, "BUILD "  __TIME__ " "  __DATE__"\n");
			fprintf(stderr, "configured to support archs: %s\n", ALL_SUPPORTED_ARCHS);
			 // if only -v specified, exit cleanly
			 if ( argc == 2 ) {
				const char* ltoVers = lto::version();
				if ( ltoVers != NULL )
					fprintf(stderr, "LTO support using: %s (static support for %d, runtime is %d)\n",
							ltoVers, lto::static_api_version(), lto::runtime_api_version());
				fprintf(stderr, "TAPI support using: %s\n", tapi::Version::getFullVersionAsString().c_str());
				exit(0);
			}
		}
		else if ( strcmp(argv[i], "-version_details") == 0 ) {
			fVerbose = true;
			extern const char ldVersionString[];
			fprintf(stdout, "{\n");
			fprintf(stdout, "\t\"version\": \"%s\",\n", STRINGIFY(LD64_VERSION_NUM));
			fprintf(stdout, "\t\"architectures\": [\n");
			char all_archs[] = ALL_SUPPORTED_ARCHS;
			char* p = strtok(all_archs, " ");
			int i = 0;
			while (p) {
				if (i++ > 0) {
					fprintf(stdout, ",\n");
				}
				fprintf(stdout, "\t\t\"%s\"", p);
				p = strtok(NULL, " ");
			}
			fprintf(stdout, "\n\t],\n");

			const char* ltoVers = lto::version();
			if ( ltoVers != NULL ) {
				fprintf(stdout, "\t\"lto\": {\n");
				fprintf(stdout, "\t\t\"runtime_api_version\": %d,\n", lto::runtime_api_version());
				fprintf(stdout, "\t\t\"static_api_version\": %d,\n", lto::static_api_version());
				fprintf(stdout, "\t\t\"version_string\": \"%s\"\n", ltoVers);
				fprintf(stdout, "\t},\n");
			}

			fprintf(stdout, "\t\"tapi\": {\n");
			fprintf(stdout, "\t\t\"version\": \"%s\",\n", tapi::Version::getAsString().c_str());
			fprintf(stdout, "\t\t\"version_string\": \"%s\"\n", tapi::Version::getFullVersionAsString().c_str());
			fprintf(stdout, "\t}\n");
			fprintf(stdout, "}\n");
			// if only -version_json specified, exit cleanly
			if ( argc == 2 ) {
				exit(0);
			}
		}
		else if ( strcmp(argv[i], "-syslibroot") == 0 ) {
			const char* path = argv[++i];
			if ( path == NULL )
				throw "-syslibroot missing argument";
			fSDKPaths.push_back(path);
		}
		else if ( strcmp(argv[i], "-search_paths_first") == 0 ) {
			fLibrarySearchMode = kSearchDylibAndArchiveInEachDir;
		}
		else if ( strcmp(argv[i], "-search_dylibs_first") == 0 ) {
			fLibrarySearchMode = kSearchAllDirsForDylibsThenAllDirsForArchives;
		}
		else if ( strcmp(argv[i], "-w") == 0 ) {
			sEmitWarnings = false;
		}
		else if ( strcmp(argv[i], "-fatal_warnings") == 0 ) {
			sFatalWarnings = true;
		}
		else if ( strcmp(argv[i], "-dependency_info") == 0 ) {
			 const char* path = argv[++i];
			 if ( path == NULL )
				throw "-dependency_info missing <path>";
			fDependencyInfoPath = path;
		}
		else if ( strcmp(argv[i], "-bitcode_bundle") == 0 ) {
			fBundleBitcode = true;
		}
		else if ( strcmp(argv[i], "-platform_version") == 0 ) {
			const char* platformStr = argv[++i];
			if (platformStr == nullptr) {
				throwf("platform and versions must be specified");
			}
			platform = ld::platformFromName(platformStr);
			++i;
		}
	}
	int standardLibraryPathsStartIndex = libraryPaths.size();
	int standardFrameworkPathsStartIndex = frameworkPaths.size();
	if ( addStandardLibraryDirectories ) {
		if ( platform == ld::Platform::driverKit ) {
			// <rdar://problem/60663707> ld64 should use different default search paths for driverkit programs, so clang does not have to add them
			libraryPaths.push_back("/System/DriverKit/usr/lib");
			frameworkPaths.push_back("/System/DriverKit/System/Library/Frameworks/");
		}
		else {
			libraryPaths.push_back("/usr/lib");
			libraryPaths.push_back("/usr/local/lib");

			frameworkPaths.push_back("/Library/Frameworks/");
			frameworkPaths.push_back("/System/Library/Frameworks/");
		}
	}

	// <rdar://problem/5829579> Support for configure based hacks 
	// if last -syslibroot is /, then ignore all syslibroots
	if ( fSDKPaths.size() > 0 ) {
		if ( strcmp(fSDKPaths.back(), "/") == 0 ) {
			fSDKPaths.clear();
		}
	}

	// now merge sdk and library paths to make real search paths
	fLibrarySearchPaths.reserve(libraryPaths.size()*(fSDKPaths.size()+1));
	int libIndex = 0;
	for (std::vector<const char*>::iterator it = libraryPaths.begin(); it != libraryPaths.end(); ++it, ++libIndex) {
		const char* libDir = *it;
		bool sdkOverride = false;
		if ( libDir[0] == '/' ) {
			char betterLibDir[PATH_MAX];
			if ( strstr(libDir, "/..") != NULL ) {
				if ( realpath(libDir, betterLibDir) != NULL )
					libDir = strdup(betterLibDir);
			}
			const int libDirLen = strlen(libDir);
			for (std::vector<const char*>::iterator sdkit = fSDKPaths.begin(); sdkit != fSDKPaths.end(); sdkit++) {
				const char* sdkDir = *sdkit;
				const int sdkDirLen = strlen(sdkDir);
				char newPath[libDirLen + sdkDirLen+4];
				strcpy(newPath, sdkDir);
				if ( newPath[sdkDirLen-1] == '/' )
					newPath[sdkDirLen-1] = '\0';
				strcat(newPath, libDir);
				struct stat statBuffer;
				if ( stat(newPath, &statBuffer) == 0 ) {
					if ( (statBuffer.st_mode & S_IFDIR) == 0 ) {
						warning("-syslibroot and -L combined path '%s' is not a directory", newPath);
					}
					else {
						fLibrarySearchPaths.push_back(strdup(newPath));
						sdkOverride = true;
					}
				}
			}
		}
		if ( !sdkOverride ) {
			if ( (libIndex >= standardLibraryPathsStartIndex) && (fSDKPaths.size() == 1) ) {
				// <rdar://problem/6438270> -syslibroot should skip standard search paths not in the SDK
				// if one SDK is specified and a standard library path is not in the SDK, don't use it
			}
			else {
				struct stat statBuffer;
				if ( stat(libDir, &statBuffer) == 0 ) {
					if ( (statBuffer.st_mode & S_IFDIR) == 0 )
						warning("-L path '%s' is not a directory", libDir);
					else
						fLibrarySearchPaths.push_back(libDir);
				}
				else if ( !addStandardLibraryDirectories || (strcmp(libDir, "/usr/local/lib") != 0) ) {
					warning("directory not found for option '-L%s'", libDir);
				}
			}
		}
	}


	// now merge sdk and framework paths to make real search paths
	fFrameworkSearchPaths.reserve(frameworkPaths.size()*(fSDKPaths.size()+1));
	int frameIndex = 0;
	for (std::vector<const char*>::iterator it = frameworkPaths.begin(); it != frameworkPaths.end(); ++it, ++frameIndex) {
		const char* frameworkDir = *it;
		bool sdkOverride = false;
		if ( frameworkDir[0] == '/' ) {
			char betterFrameworkDir[PATH_MAX];
			if ( strstr(frameworkDir, "/..") != NULL ) {
				if ( realpath(frameworkDir, betterFrameworkDir) != NULL )
					frameworkDir = strdup(betterFrameworkDir);
			}
			const int frameworkDirLen = strlen(frameworkDir);
			for (std::vector<const char*>::iterator sdkit = fSDKPaths.begin(); sdkit != fSDKPaths.end(); sdkit++) {
				const char* sdkDir = *sdkit;
				const int sdkDirLen = strlen(sdkDir);
				char newPath[frameworkDirLen + sdkDirLen+4];
				strcpy(newPath, sdkDir);
				if ( newPath[sdkDirLen-1] == '/' )
					newPath[sdkDirLen-1] = '\0';
				strcat(newPath, frameworkDir);
				struct stat statBuffer;
				if ( stat(newPath, &statBuffer) == 0 ) {
					if ( (statBuffer.st_mode & S_IFDIR) == 0 ) {
						warning("-syslibroot and -F combined path '%s' is not a directory", newPath);
					}
					else {
						fFrameworkSearchPaths.push_back(strdup(newPath));
						sdkOverride = true;
					}
				}
			}
		}
		if ( !sdkOverride ) {
			if ( (frameIndex >= standardFrameworkPathsStartIndex) && (fSDKPaths.size() == 1) ) {
				// <rdar://problem/6438270> -syslibroot should skip standard search paths not in the SDK
				// if one SDK is specified and a standard library path is not in the SDK, don't use it
			}
			else {
				struct stat statBuffer;
				if ( stat(frameworkDir, &statBuffer) == 0 ) {
					if ( (statBuffer.st_mode & S_IFDIR) == 0 )
						warning("-F path '%s' is not a directory", frameworkDir);
					else
						fFrameworkSearchPaths.push_back(frameworkDir);
				}
				else if ( !addStandardLibraryDirectories || (strcmp(frameworkDir, "/Library/Frameworks/") != 0) ) {
					warning("directory not found for option '-F%s'", frameworkDir);
				}
			}
		}
	}

	if ( fVerbose ) {
		fprintf(stderr,"Library search paths:\n");
		for (std::vector<const char*>::iterator it = fLibrarySearchPaths.begin();
			 it != fLibrarySearchPaths.end();
			 it++)
			fprintf(stderr,"\t%s\n", *it);
		fprintf(stderr,"Framework search paths:\n");
		for (std::vector<const char*>::iterator it = fFrameworkSearchPaths.begin();
			 it != fFrameworkSearchPaths.end();
			 it++)
			fprintf(stderr,"\t%s\n", *it);
	}
}

// this is run before the command line is parsed
void Options::parsePreCommandLineEnvironmentSettings()
{
	if (getenv("LD_FORCE_LEGACY_VERSION_LOAD_CMDS") != NULL) {
		fForceLegacyVersionLoadCommands = true;
	}

	if ((getenv("LD_TRACE_ARCHIVES") != NULL)
		|| (getenv("RC_TRACE_ARCHIVES") != NULL))
	    fTraceArchives = true;

	if ((getenv("LD_TRACE_DYLIBS") != NULL)
		|| (getenv("RC_TRACE_DYLIBS") != NULL)) {
	    fTraceDylibs = true;
		fTraceIndirectDylibs = true;
	}
	
	if ((getenv("LD_TRACE_DEPENDENTS") != NULL)) {
		fTraceEmitJSON 		 = true;
		// <rdar://problem/43652680> ld64 should ignore LD_TRACE_ARCHIVES and LD_TRACE_DYLIBS if LD_TRACE_DEPENDENTS is set in the environment
		fTraceArchives 		 = false;
	    fTraceDylibs  		 = false;
		fTraceIndirectDylibs = false;
	}

	if (getenv("RC_TRACE_DYLIB_SEARCHING") != NULL) {
	    fTraceDylibSearching = true;
	}

	if (getenv("LD_PRINT_OPTIONS") != NULL)
		fPrintOptions = true;

	if (fTraceDylibs || fTraceArchives || fTraceEmitJSON)
		fTraceOutputFile = getenv("LD_TRACE_FILE");

	if (getenv("LD_PRINT_ORDER_FILE_STATISTICS") != NULL)
		fPrintOrderFileStatistics = true;

	if (getenv("LD_NO_ENCRYPT") != NULL) {
		fEncryptable = false;
		fMarkAppExtensionSafe = true; // temporary
		fCheckAppExtensionSafe = false;
	}

	if (getenv("LD_APPLICATION_EXTENSION_SAFE") != NULL) {
		fMarkAppExtensionSafe = true;
		fCheckAppExtensionSafe = false;
	}
	
	if (getenv("LD_ALLOW_CPU_SUBTYPE_MISMATCHES") != NULL)
		fAllowCpuSubtypeMismatches = true;
	
	if (getenv("LD_DYLIB_CPU_SUBTYPES_MUST_MATCH") != NULL)
		fEnforceDylibSubtypesMatch = true;

	if (getenv("LD_WARN_ON_SWIFT_ABI_VERSION_MISMATCHES") != NULL)
		fWarnOnSwiftABIVersionMismatches = true;
	
	sWarningsSideFilePath = getenv("LD_WARN_FILE");
	
	const char* customDyldPath = getenv("LD_DYLD_PATH");
	if ( customDyldPath != NULL ) 
		fDyldInstallPath = customDyldPath;
    
    const char* debugArchivePath = getenv("LD_DEBUG_SNAPSHOT");
    if (debugArchivePath != NULL) {
        fLinkSnapshot.setSnapshotMode(Snapshot::SNAPSHOT_DEBUG);
        if (strlen(debugArchivePath) > 0)
            fLinkSnapshot.setSnapshotPath(debugArchivePath);
        fSnapshotRequested = true;
    }

    const char* pipeFdString = getenv("LD_PIPELINE_FIFO");
    if (pipeFdString != NULL) {
		fPipelineFifo = pipeFdString;
    }

	// <rdar://problem/30746905> [Reproducible Builds] If env ZERO_AR_DATE is set, zero out timestamp in N_OSO stab
	if ( getenv("ZERO_AR_DATE") != NULL )
		fZeroModTimeInDebugMap = true;

	char rawPath[PATH_MAX];
	char path[PATH_MAX];
	char *base;
	uint32_t bufSize = PATH_MAX;
	if ( _NSGetExecutablePath(rawPath, &bufSize) != -1 ) {
		if ( realpath(rawPath, path) != NULL ) {
#define TOOLCHAINBASEPATH "/Developer/Toolchains/"
			if ( (base = strstr(path, TOOLCHAINBASEPATH)) )
				fToolchainPath = strndup(path, base - path + strlen(TOOLCHAINBASEPATH));
		}
	}

	// <rdar://problem/38679559> ld64 should consider RC_RELEASE when calculating a binary's UUID
	fBuildContextName = getenv("RC_RELEASE");
	
	if (getenv("LD_PREFER_TAPI_FILE") != NULL)
		fPreferTAPIFile = true;

}


// this is run after the command line is parsed
void Options::parsePostCommandLineEnvironmentSettings()
{
	// when building a dynamic main executable, default any use of @executable_path to output path
	if ( fExecutablePath == NULL && (fOutputKind == kDynamicExecutable) ) {
		fExecutablePath = fOutputFile;
	}

	// allow build system to force on dead-code-stripping
	if ( !fDeadStrip ) {
		if ( getenv("LD_DEAD_STRIP") != NULL ) {
			switch (fOutputKind) {
				case Options::kDynamicLibrary:
				case Options::kDynamicExecutable:
				case Options::kDynamicBundle:
					fDeadStrip = true;
					break;
				case Options::kPreload:
				case Options::kObjectFile:
				case Options::kDyld:
				case Options::kStaticExecutable:
				case Options::kKextBundle:
					break;
			}
		}
	}
	
	// allow build system to force on -warn_commons
	if ( getenv("LD_WARN_COMMONS") != NULL )
		fWarnCommons = true;
	
	// allow B&I to set default -source_version
	if ( fSourceVersion == 0 ) {
		const char* vers = getenv("RC_ProjectSourceVersion");
		if ( vers != NULL )
			fSourceVersion = parseVersionNumber64(vers);
	}
		
}


bool Options::sharedCacheEligiblePath(const char* path) const
{
	if ( (strncmp(path, "/usr/lib/", 9) == 0) || (strncmp(path, "/System/Library/", 16) == 0) )
		return true;

	// <rdar://problem/48183961> Dylibs with install_names in /Library/Apple/ should be eligible for dyld shared cache
	if ( platforms().contains(ld::Platform::macOS) ) {
		if ( (strncmp(path, "/Library/Apple/usr/lib/", 23) == 0) || (strncmp(path, "/Library/Apple/System/Library/", 30) == 0) )
			return true;
	}

	uint32_t iosMacVers = platforms().minOS(ld::Platform::iOS);
	if ( iosMacVers >= 0x000D0000 ) {
		if ( (strncmp(path, "/System/iOSSupport/System/Library/", 34) == 0) || (strncmp(path, "/System/iOSSupport/usr/lib/", 27) == 0) )
			return true;
	}
	return false;
}

void Options::reconfigureDefaults()
{
	// -preload maps to "freestanding" platform
	if ( fOutputKind == Options::kPreload ) {
		// Some EFI builds set -macosx_version_min, even though they are -preload
		if ( !platforms().empty() )
			fPlatforms.clear();
		// set platform to freestanding
		fPlatforms.insert(ld::PlatformVersion(ld::Platform::freestanding, 0));
	}
	// -static maps to "freestanding" platform, unless one already set
	else if ( (fOutputKind == Options::kStaticExecutable) && platforms().empty() ) {
		fPlatforms.insert(ld::PlatformVersion(ld::Platform::freestanding, 0));
	}

	// if -arch or platform not specified, look at .o files and infer
	if ( platforms().empty() || (fArchitecture == 0) )
		inferArchAndPlatform();

	// if no platform specified in .o files, look for env vars
	if ( platforms().empty() ) {
		if ( fOutputKind != Options::kObjectFile )
			warning("No platform min-version specified on command line");

		// check for *_DEPLOYMENT_TARGET env var
		ld::forEachSupportedPlatform(^(const ld::PlatformInfo& info, bool& stop) {
			if ( info.fallbackEnvVarName != NULL ) {
				if ( const char* versStr = getenv(info.fallbackEnvVarName) ) {
					stop = true;
					uint32_t value;
					if ( parsePackedVersion32(versStr, value) ) {
						fPlatforms.insert(ld::PlatformVersion(info.platform, value));
					}
					else {
						warning("%s env var malformed: '%s'", info.fallbackEnvVarName, versStr);
					}
				}
			}
		});
	}

	// sync reader options
	switch ( fOutputKind ) {
		case Options::kObjectFile:
			fForFinalLinkedImage = false;
			break;
		case Options::kDyld:
			fForDyld = true;
			fForFinalLinkedImage = true;
			fNoEHLabels = true;
			break;
		case Options::kDynamicLibrary:
		case Options::kDynamicBundle:
		case Options::kKextBundle:
			fForFinalLinkedImage = true;
			fNoEHLabels = true;
			break;
		case Options::kDynamicExecutable:
		case Options::kStaticExecutable:
		case Options::kPreload:
			fLinkingMainExecutable = true;
			fForFinalLinkedImage = true;
			fNoEHLabels = true;
			break;
	}


	// default to adding functions start for dynamic code, static code must opt-in
	switch ( fOutputKind ) {
		case Options::kPreload:
		case Options::kStaticExecutable:
		case Options::kKextBundle:
			if ( fDataInCodeInfoLoadCommandForcedOn )
				fDataInCodeInfoLoadCommand = true;
			if ( fFunctionStartsForcedOn )
				fFunctionStartsLoadCommand = true;
			break;
		case Options::kObjectFile:
			if ( !fDataInCodeInfoLoadCommandForcedOff )
				fDataInCodeInfoLoadCommand = true;
			if ( fFunctionStartsForcedOn )
				fFunctionStartsLoadCommand = true;
			break;
		case Options::kDynamicExecutable:
		case Options::kDyld:
		case Options::kDynamicLibrary:
		case Options::kDynamicBundle:
			if ( !fDataInCodeInfoLoadCommandForcedOff )
				fDataInCodeInfoLoadCommand = true;
			if ( !fFunctionStartsForcedOff )
				fFunctionStartsLoadCommand = true;
			break;
	}
		
	// adjust kext type based on architecture
	if ( fOutputKind == kKextBundle ) {
		switch ( fArchitecture ) {
			case CPU_TYPE_X86_64:
				// x86_64 uses new MH_KEXT_BUNDLE type
				fMakeCompressedDyldInfo = false;
				fMakeCompressedDyldInfoForceOff = true;
				fAllowTextRelocs = true;
				fUndefinedTreatment = kUndefinedDynamicLookup;
				break;
			case CPU_TYPE_ARM64:
#if SUPPORT_ARCH_arm64_32
			case CPU_TYPE_ARM64_32:
#endif
				// arm64 uses new MH_KEXT_BUNDLE type
				fMakeCompressedDyldInfo = false;
				fMakeCompressedDyldInfoForceOff = true;
				fAllowTextRelocs = false; 
				fKextsUseStubs = true;
				fUndefinedTreatment = kUndefinedDynamicLookup;
				break;
			case CPU_TYPE_ARM:
				fMakeCompressedDyldInfo = false;
				fMakeCompressedDyldInfoForceOff = true;
				fAllowTextRelocs = false;
				fKextsUseStubs = !fAllowTextRelocs;
				fUndefinedTreatment = kUndefinedDynamicLookup;
				break;
			case CPU_TYPE_I386:
				// use .o files
				fOutputKind = kObjectFile;
				break;
		}
	}

	// disable implicit dylibs when targeting 10.3
	// <rdar://problem/5451987> add option to disable implicit load commands for indirectly used public dylibs
	if ( !platforms().minOS(ld::version2007) )
		fImplicitlyLinkPublicDylibs = false;

	// set too-large size
	switch ( fArchitecture ) {
		case CPU_TYPE_I386:
			fMaxAddress = 0xFFFFFFFF;
			break;
		case CPU_TYPE_X86_64:
			break;
		case CPU_TYPE_ARM:
			switch ( fOutputKind ) {
				case Options::kDynamicExecutable:
				case Options::kDynamicLibrary:
				case Options::kDynamicBundle:
					// user land code is limited to low 1GB
					fMaxAddress = 0x2FFFFFFF;
					break;
				case Options::kStaticExecutable:
				case Options::kObjectFile:
				case Options::kDyld:
				case Options::kPreload:
				case Options::kKextBundle:
					fMaxAddress = 0xFFFFFFFF;
					break;
			}
			// range check -seg1addr for ARM
			if ( fBaseAddress > fMaxAddress ) {
				warning("ignoring -seg1addr 0x%08llX.  Address out of range.", fBaseAddress);
				fBaseAddress = 0;
			}
			break;
	}

	// determine if info for shared region should be added
	if ( fOutputKind == Options::kDynamicLibrary ) {
		if ( platforms().minOS(ld::version2008Fall) )
			if ( !fSharedRegionEligibleForceOff )
				if ( sharedCacheEligiblePath(this->installPath()) )
					fSharedRegionEligible = true;
	}
	else if ( fOutputKind == Options::kDyld ) {
        // <rdar://problem/10111122> Enable dyld to be put into the dyld shared cache
        fSharedRegionEligible = true;
	}

	// A -kext for iOS 10 ==>  -data_const, -text_exec, -add_split_seg_info
	if ( (fOutputKind == Options::kKextBundle) && (fArchitecture == CPU_TYPE_ARM64) ) {
		fUseDataConstSegment = true;
		fUseTextExecSegment = true;
		fSharedRegionEligible = true;
	}

	// by default, use __DATA_CONST for all userland binaries targeting newer OS
	if ( !fSharedRegionEligible && !fUseDataConstSegmentForceOff && platforms().minOS(ld::version2019Fall) ) {
		switch ( fOutputKind ) {
			case Options::kDynamicExecutable:
				// don't auto use __DATA_CONST for -no_pie programs (like emacs)
				fUseDataConstSegment = !fDisablePositionIndependentExecutable;
				break;
			case Options::kDynamicBundle:
				// <rdar://problem/47933622> Bundles located at: /System/Library/UserEventPlugins/*plugins need be opted out of __DATA_CONST
				if ( (fFinalName != NULL) && (strncmp(fFinalName, "/System/Library/UserEventPlugins/", 33) == 0) )
					fUseDataConstSegment = false;
				else
					fUseDataConstSegment = true;
				break;
			case Options::kDynamicLibrary:
			case Options::kDyld:
				fUseDataConstSegment = true;
				break;
			case Options::kStaticExecutable:
			case Options::kObjectFile:
			case Options::kPreload:
			case Options::kKextBundle:
				break;
		}
	}

	// If we are going to be shared cache eligible, work out if we have dirty data as that requires V2
	if ( fSharedRegionEligible && (fArchitecture != CPU_TYPE_I386)
		&& (platforms().minOS(ld::supportsSplitSegV2) || (fArchitecture == CPU_TYPE_ARM64)) ) {
		// If -dirty_data_list not specified, look in $SDKROOT/AppleInternal/DirtyDataFiles/<dylib>.dirty for dirty data list
		if ( fSymbolsMovesData.empty() && ( fDylibInstallName != NULL) && !fSDKPaths.empty() ) {
			const char* dylibLeaf = strrchr(fDylibInstallName, '/');
			if ( dylibLeaf ) {
				char path[PATH_MAX];
				strlcpy(path , fSDKPaths.front(), sizeof(path));
				strlcat(path , "/AppleInternal/DirtyDataFiles", sizeof(path));
				strlcat(path , dylibLeaf, sizeof(path));
				strlcat(path , ".dirty", sizeof(path));
				FileInfo info;
				if ( info.checkFileExists(*this, path) )
					// <rdar://problem/54355096> ld: Disable wildcard matching in dirty data lists sourced from /AppleInternal/DirtyDataFiles
					addSymbolMove("__DATA_DIRTY", path, fSymbolsMovesData, "-dirty_data_list", kDisallowWildcards);
			}
		}

		// Look in $SDKROOT/AppleInternal/AccessibilityLinkerSymbols/<dylib>.axsymbols for objc-class names
		if ( (fDylibInstallName != NULL) && !fSDKPaths.empty() ) {
			const char* dylibLeaf = strrchr(fDylibInstallName, '/');
			if ( dylibLeaf ) {
				char path[PATH_MAX];
				strlcpy(path , fSDKPaths.front(), sizeof(path));
				strlcat(path , "/AppleInternal/AccessibilityLinkerSymbols", sizeof(path));
				strlcat(path , dylibLeaf, sizeof(path));
				strlcat(path , ".axsymbols", sizeof(path));
				FileInfo info;
				if ( info.checkFileExists(*this, path) ) {
					SymbolsMove tmp;
					fSymbolsMovesAXMethodLists.push_back(tmp);
					loadExportFile(path, ".axsymbols", fSymbolsMovesAXMethodLists.back().symbols, kAllowWildcards);
				}
			}
		}
	}

	// <rdar://problem/48979517> if aliases are used with dirty data, allow alias to do the move
	if ( !fSymbolsMovesData.empty() && !fAliases.empty() ) {
		for (const AliasPair& pair : fAliases) {
			for (SymbolsMove& moveInfo : fSymbolsMovesData) {
				bool matchBecauseOfWildcard;
				if ( moveInfo.symbols.contains(pair.alias, &matchBecauseOfWildcard) ) {
					moveInfo.symbols.insert(pair.realName, kAllowWildcards);
				}
			}
		}
	}

	// <rdar://problem/32138080> Automatically use OrderFiles found in the AppleInternal SDK
	if ( (fFinalName != NULL) && fOrderedSymbols.empty() && !fSDKPaths.empty() ) {
		char path[PATH_MAX];
		strlcpy(path , fSDKPaths.front(), sizeof(path));
		strlcat(path , "/AppleInternal/OrderFiles/", sizeof(path));
		strlcat(path , fFinalName, sizeof(path));
		strlcat(path , ".order", sizeof(path));
		FileInfo info;
		if ( info.checkFileExists(*this, path) )
			parseOrderFile(path, false);
	}

	// Use V2 shared cache info when targeting newer OSs and archs
	if ( fSharedRegionEligible && (platforms().minOS(ld::supportsSplitSegV2) || (fArchitecture == CPU_TYPE_ARM64)) ) {
		fSharedRegionEncodingV2 = true;
		if ( fSharedRegionEncodingV2 && (fArchitecture == CPU_TYPE_I386) ) {
			// Disable V2 on i386 as its not qualififed yet.
			fSharedRegionEncodingV2 = false;
		} else if ( platforms().contains(ld::Platform::macOS) && (fArchitecture == CPU_TYPE_X86_64) && !fKernel ) {
			fSharedRegionEncodingV2 = false;
			// <rdar://problem/40600799> use v2 for stable Swift dylibs in /usr/lib/swift/
			if ( strncmp(this->installPath(), "/usr/lib/swift/", 15) == 0 )
				fSharedRegionEncodingV2 = true;
			// <rdar://problem/31428120> any other OS frameworks that use swift need v2
			for (const char* searchPath  : fLibrarySearchPaths ) {
				if ( strstr(searchPath, "/usr/lib/swift") != NULL ) {
					fSharedRegionEncodingV2 = true;
					break;
				}
			}
			// <rdar://problem/61357465> Enable split seg V2 in macOS when using dirty data
			if ( !fSymbolsMovesData.empty() || !fSymbolsMovesAXMethodLists.empty() )
				fSharedRegionEncodingV2 = true;
		}
		fIgnoreOptimizationHints = true;
	}
	
	if ( fUseDataConstSegmentForceOn ) {
		fUseDataConstSegment = true;
	}
	else if ( fUseDataConstSegmentForceOff ) {
		fUseDataConstSegment = false;
	}
	else if ( fSharedRegionEncodingV2 ) {
		// automatically use __DATA_CONST in shared dylibs when V2 is in use
		fUseDataConstSegment = true;
	}

#if SUPPORT_ARCH_arm64e
	if ( (fArchitecture == CPU_TYPE_ARM64) && (fSubArchitecture == CPU_SUBTYPE_ARM64E) ) {
		// use __AUTH_DATA when targetting the arm64e shared cache
		if ( !fUseAuthDataSegmentForceOff && fSharedRegionEligible && (fOutputKind == Options::kDynamicLibrary) ) {
			fUseAuthDataSegment = true;
			if ( (fOutputKind == Options::kDynamicLibrary) && (fDylibInstallName != NULL) ) {
				// Don't add __AUTH to libxpc for now until their range check is fixed
				if ( strncmp(fDylibInstallName, "/usr/lib/system/libxpc", 22) == 0 )
					fUseAuthDataSegment = false;
				// Don't add __AUTH to liblibdispatch for now until their range check is fixed
				if ( strncmp(fDylibInstallName, "/usr/lib/system/libdispatch", 27) == 0 )
					fUseAuthDataSegment = false;
				if ( strncmp(fDylibInstallName, "/usr/lib/system/introspection/libdispatch", 41) == 0 )
					fUseAuthDataSegment = false;
			}
		}
	}
#endif

	// TEMP until <rdar://problem/45656620&45656620> are fixed
	if ( (fFinalName != NULL) && ((strcmp(fFinalName, "/usr/libexec/locationd") == 0) || (strcmp(fFinalName, "/usr/libexec/terminusd") == 0)) ) {
		fUseDataConstSegment = false;
	}

	if ( fUseDataConstSegment ) {
		addSectionRename("__DATA", "__got",				"__DATA_CONST", "__got");
#if SUPPORT_ARCH_arm64e
		addSectionRename("__DATA", "__auth_got",		"__DATA_CONST", "__auth_got");
		addSectionRename("__DATA", "__auth_ptr",		"__DATA_CONST", "__auth_ptr");
#endif
		addSectionRename("__DATA", "__nl_symbol_ptr",	"__DATA_CONST", "__nl_symbol_ptr");
		addSectionRename("__DATA", "__const",			"__DATA_CONST", "__const");
		addSectionRename("__DATA", "__cfstring",		"__DATA_CONST", "__cfstring");
		addSectionRename("__DATA", "__mod_init_func",   "__DATA_CONST", "__mod_init_func");
		addSectionRename("__DATA", "__mod_term_func",   "__DATA_CONST", "__mod_term_func");
		addSectionRename("__DATA", "__objc_classlist",  "__DATA_CONST", "__objc_classlist");
		addSectionRename("__DATA", "__objc_nlclslist",	"__DATA_CONST", "__objc_nlclslist");
		addSectionRename("__DATA", "__objc_catlist",	"__DATA_CONST", "__objc_catlist");
		addSectionRename("__DATA", "__objc_nlcatlist",	"__DATA_CONST", "__objc_nlcatlist");
		addSectionRename("__DATA", "__objc_protolist",	"__DATA_CONST", "__objc_protolist");
		addSectionRename("__DATA", "__objc_imageinfo",	"__DATA_CONST", "__objc_imageinfo");
		if ( fSharedRegionEligible ) {
			// these sections are not really read-only, except in the dyld cache (for now)
			addSectionRename("__DATA", "__la_symbol_ptr",	"__DATA_CONST", "__la_symbol_ptr");
			addSectionRename("__DATA", "__objc_const",	    "__DATA_CONST", "__objc_const");
		}
	}
	if ( fUseTextExecSegment ) {
		addSectionRename("__TEXT", "__text",				"__TEXT_EXEC", "__text");
		addSectionRename("__TEXT", "__stubs",				"__TEXT_EXEC", "__stubs");
	}
	// <rdar://problem/49907181> automatically move thread local data to __DATA_DIRTY if dirty data is in use
	if ( fSharedRegionEncodingV2 && !fSymbolsMovesData.empty() ) {
		addSectionRename("__DATA", "__thread_vars",	        "__DATA_DIRTY", "__thread_vars");
	}

	// <rdar://problem/5366363> -r -x implies -S
	if ( (fOutputKind == Options::kObjectFile) && (fLocalSymbolHandling == kLocalSymbolsNone) )
		fDebugInfoStripping = Options::kDebugInfoNone;			

	// <rdar://problem/15252891> -r implies -no_uuid
	if ( fOutputKind == Options::kObjectFile )
		fUUIDMode = kUUIDNone;

	// choose how to process unwind info
	switch ( fArchitecture ) {
		case CPU_TYPE_I386:		
		case CPU_TYPE_X86_64:		
		case CPU_TYPE_ARM64:		
#if SUPPORT_ARCH_arm64_32
		case CPU_TYPE_ARM64_32:		
#endif
			switch ( fOutputKind ) {
				case Options::kObjectFile:
				case Options::kStaticExecutable:
				case Options::kPreload:
				case Options::kKextBundle:
					fAddCompactUnwindEncoding = false;
					break;
				case Options::kDyld:
				case Options::kDynamicLibrary:
				case Options::kDynamicBundle:
				case Options::kDynamicExecutable:
					//if ( fAddCompactUnwindEncoding && (fVersionMin >= ld::mac10_6) )
					//	fRemoveDwarfUnwindIfCompactExists = true;
					break;
			}
			break;
		case CPU_TYPE_ARM:
			if ( armUsesZeroCostExceptions() )  {
				switch ( fOutputKind ) {
					case Options::kObjectFile:
					case Options::kStaticExecutable:
					case Options::kPreload:
					case Options::kKextBundle:
						fAddCompactUnwindEncoding = false;
						break;
					case Options::kDyld:
					case Options::kDynamicLibrary:
					case Options::kDynamicBundle:
					case Options::kDynamicExecutable:
						fAddCompactUnwindEncoding = true;
						break;
				}
			}
			else {
				fAddCompactUnwindEncoding = false;
				fRemoveDwarfUnwindIfCompactExists = false;
			}
			break;
		case 0:
			// if -arch is missing, assume we don't want compact unwind info
			fAddCompactUnwindEncoding = false;
			break;
	}
		
	// only iOS/tvOS/watchOS executables should be encryptable
	switch ( fOutputKind ) {
		case Options::kObjectFile:
		case Options::kDyld:
		case Options::kStaticExecutable:
		case Options::kPreload:
		case Options::kKextBundle:
			fEncryptable = false;
			break;
		case Options::kDynamicExecutable:
			break;
		case Options::kDynamicLibrary:
		case Options::kDynamicBundle:
			// <rdar://problem/16293398> Add LC_ENCRYPTION_INFO load command to bundled frameworks
			if ( !platforms().minOS(ld::version2013) )
				fEncryptable = false;
			break;
	}
	if ( !platforms().contains(ld::Platform::iOS) && !platforms().contains(ld::Platform::tvOS) && !platforms().contains(ld::Platform::watchOS) )
		fEncryptable = false;
	if ( fEncryptableForceOn )
		fEncryptable = true;
	else if ( fEncryptableForceOff )
		fEncryptable = false;

	// don't move inits in dyld because dyld wants certain
	// entries point at stable locations at the start of __text
	if ( fOutputKind == Options::kDyld ) 
		fAutoOrderInitializers = false;
		
		
	// disable __data ordering for some output kinds
	switch ( fOutputKind ) {
		case Options::kObjectFile:
		case Options::kDyld:
		case Options::kStaticExecutable:
		case Options::kPreload:
		case Options::kKextBundle:
			fOrderData = false;
			break;
		case Options::kDynamicExecutable:
		case Options::kDynamicLibrary:
		case Options::kDynamicBundle:
			break;
	}
	
	// only use compressed LINKEDIT for final linked images
	switch ( fOutputKind ) {
		case Options::kDynamicExecutable:
		case Options::kDynamicLibrary:
		case Options::kDynamicBundle:
			break;
		case Options::kDyld:
			// arm64e has support for compressed LINKEDIT.
			if ( (fArchitecture == CPU_TYPE_ARM64) && (fSubArchitecture == CPU_SUBTYPE_ARM64E) )
				break;
		case Options::kPreload:
		case Options::kStaticExecutable:
		case Options::kObjectFile:
		case Options::kKextBundle:
			fMakeCompressedDyldInfoForceOff = true;
			break;
	}

	// only use legacy LINKEDIT if compressed LINKEDIT is forced off of:
	//			macOS before 10.6
	//			iOS before 3.1
	if ( fMakeCompressedDyldInfoForceOff || !platforms().minOS(ld::version2009) )
		fMakeCompressedDyldInfo = false;

	// only ARM and x86_64 enforces that cpu-sub-types must match
	switch ( fArchitecture ) {
		case CPU_TYPE_ARM:
		case CPU_TYPE_ARM64:
			break;
		case CPU_TYPE_X86_64:
			fEnforceDylibSubtypesMatch = false;
			break;
		case CPU_TYPE_I386:
#if SUPPORT_ARCH_arm64_32
		case CPU_TYPE_ARM64_32:
#endif
			fEnforceDylibSubtypesMatch = false;
			break;
	}
		
		
	// only final linked images can not optimize zero fill sections
	if ( fOutputKind == Options::kObjectFile )
		fOptimizeZeroFill = true;

	// all undefines in -r mode
//	if ( fOutputKind == Options::kObjectFile )
//		fUndefinedTreatment = kUndefinedSuppress;

	// only dynamic final linked images should warn about use of commmons
	if ( fWarnCommons ) {
		switch ( fOutputKind ) {
			case Options::kDynamicExecutable:
			case Options::kDynamicLibrary:
			case Options::kDynamicBundle:
				break;
			case Options::kPreload:
			case Options::kStaticExecutable:
			case Options::kObjectFile:
			case Options::kDyld:
			case Options::kKextBundle:
				fWarnCommons = false;
				break;
		}
	}
	
	// Mac OS X 10.5 and iPhoneOS 2.0 support LC_REEXPORT_DYLIB
	if ( platforms().minOS(ld::version2008) )
		fUseSimplifiedDylibReExports = true;
	
	// Mac OS X 10.7 and iOS 4.2 support LC_LOAD_UPWARD_DYLIB
	if ( platforms().minOS(ld::version2010) && (fOutputKind == kDynamicLibrary) )
		fCanUseUpwardDylib = true;

	if (fArchitecture == CPU_TYPE_ARM64) {
#if SUPPORT_ARCH_arm64e
		if (fSubArchitecture == CPU_SUBTYPE_ARM64E)
		{
			// FIXME: Move some of these to arm64
			fNoLazyBinding = true;
			switch ( fOutputKind ) {
				case Options::kDynamicExecutable:
				case Options::kDynamicLibrary:
				case Options::kDynamicBundle:
				case Options::kDyld:
					fUseLinkedListBinding = true;
					fUseAuthenticatedStubs = true;
					break;
				case Options::kPreload:
				case Options::kStaticExecutable:
				case Options::kObjectFile:
				case Options::kKextBundle:
					break;
			}
			switch ( fOutputKind ) {
				case Options::kDynamicExecutable:
				case Options::kDyld:
				case Options::kDynamicLibrary:
				case Options::kObjectFile:
				case Options::kDynamicBundle:
					fSupportsAuthenticatedPointers = true;
					break;
				case Options::kKextBundle:
					fSupportsAuthenticatedPointers = true;
					fSupportPackingText = true;
					break;
				case Options::kStaticExecutable:
					fSupportsAuthenticatedPointers = fKernel;
					fSupportPackingText = fKernel;
					break;
				case Options::kPreload:
					fSupportsAuthenticatedPointers = false;
					break;
			}
		}
#endif
	}

	if ( fMakeThreadedStartsSection && (fArchitecture != CPU_TYPE_ARM64) ) {
		// Threaded starts isn't valid here so ignore it.
		warning("-threaded_starts_section ignored ignored for non-arm64");
		fMakeThreadedStartsSection = false;
	}

	if (fMakeThreadedStartsSection) {
		switch ( fOutputKind ) {
			case Options::kDynamicExecutable:
			case Options::kDyld:
			case Options::kDynamicLibrary:
			case Options::kObjectFile:
			case Options::kDynamicBundle:
			case Options::kKextBundle:
				// Threaded starts isn't valid here so ignore it.
				warning("-threaded_starts_section ignored for binaries other than -static or -preload");
				fMakeThreadedStartsSection = false;
				break;
			case Options::kStaticExecutable:
			case Options::kPreload:
				fUseLinkedListBinding = true;
				fNoLazyBinding = true;
#if SUPPORT_ARCH_arm64e
				if ( (fArchitecture == CPU_TYPE_ARM64) && (fSubArchitecture == CPU_SUBTYPE_ARM64E) )
					fSupportsAuthenticatedPointers = true;
#endif
				break;
		}
	}

	switch ( fOutputKind ) {
		case Options::kDynamicExecutable:
			// don't use chained fixups for macOS main executables because they might be tools just run during build
			if ( platforms().minOS(ld::supportsChainedFixups) && (fArchitecture != CPU_TYPE_I386) && !targetIOSSimulator() && !platforms().contains(ld::Platform::macOS) ) {
				// first arm64e, later all arches
				if ( (fArchitecture == CPU_TYPE_ARM64) && (fSubArchitecture == CPU_SUBTYPE_ARM64E) )
					fMakeChainedFixups = true;
			}
			break;
		case Options::kDynamicLibrary:
		case Options::kDynamicBundle:
			if ( platforms().minOS(ld::supportsChainedFixups) && (fArchitecture != CPU_TYPE_I386) && !targetIOSSimulator() && !fSimulatorSupportDylib ) {
				// first arm64e, later all arches
				if ( (fArchitecture == CPU_TYPE_ARM64) && (fSubArchitecture == CPU_SUBTYPE_ARM64E) )
					fMakeChainedFixups = true;
			}
			break;
		case Options::kStaticExecutable:
			if ( fKernel && (fArchitecture == CPU_TYPE_ARM64) && (fSubArchitecture == CPU_SUBTYPE_ARM64E) ) {
				if ( platforms().minOS(ld::version2020Fall) )
					fMakeChainedFixups = true;
			}
			break;
		case Options::kDyld:
		case Options::kPreload:
			break;
		case Options::kObjectFile:
			break;
		case Options::kKextBundle:
			if ( (fArchitecture == CPU_TYPE_ARM64) && (fSubArchitecture == CPU_SUBTYPE_ARM64E) ) {
				if ( platforms().minOS(ld::version2020Fall) )
					fMakeChainedFixups = true;
			}
			break;
	}
	if ( fMakeChainedFixups ) {
		fMakeCompressedDyldInfo = false;
		fNoLazyBinding = true;
		fMakeCompressedDyldInfo = false;
		fMakeCompressedDyldInfoForceOff = true;
	}

	// <rdar://problem/54324757> Mark all 2020 binaries are no longer using lazy bind
	if ( (fArchitecture == CPU_TYPE_I386) && !targetIOSSimulator() && platforms().minOS(ld::version2020Fall) )
		fNoLazyBinding = true;

	// <rdar://problem/56786313> Also mark all simulator support binaries as no lazy bind
	if ( fSimulatorSupportDylib )
		fNoLazyBinding = true;

	// <rdar://problem/29241917> transform __DATA, __mod_init_funcs to __TEXT offsets
	if ( fMakeChainedFixups )
		fMakeInitializersIntoOffsets = true;


	// Weak binding requires that if we want to use linked list binding, we must
	// also be using no lazy binding.
	if ( fUseLinkedListBinding )
		assert(fNoLazyBinding);
	
	// MacOSX 10.7 defaults to PIE
	if ( (fArchitecture == CPU_TYPE_I386)
		&& (fOutputKind == kDynamicExecutable)
		&& platforms().minOS(ld::mac10_7) ) {
			fPositionIndependentExecutable = true;
	}

	// armv7 for iOS4.3 defaults to PIE
	if ( (fArchitecture == CPU_TYPE_ARM) 
		&& fArchSupportsThumb2
		&& (fOutputKind == kDynamicExecutable)
		&& (platforms().contains(ld::Platform::watchOS) || platforms().minOS(ld::iOS_4_3)) ) {
			fPositionIndependentExecutable = true;
	}

	// <rdar://problem/24535196> x86_64 defaults PIE (for 10.6 and later)
	if ( (fArchitecture == CPU_TYPE_X86_64) && (fOutputKind == kDynamicExecutable) && (platforms().minOS(ld::mac10_6) || platforms().contains(ld::Platform::iOSMac)) )
		fPositionIndependentExecutable = true;

	// <rdar://problem/55348074> dext processes should be PIE
	if ( (fOutputKind == kDynamicExecutable) && platforms().contains(ld::Platform::driverKit) )
		fPositionIndependentExecutable = true;

	// Simulator defaults to PIE
	if ( targetIOSSimulator() && (fOutputKind == kDynamicExecutable) )
		fPositionIndependentExecutable = true;

	// -no_pie anywhere on command line disable PIE
	if ( fDisablePositionIndependentExecutable )
		fPositionIndependentExecutable = false;

	// arm64 is always PIE
	if ( ((fArchitecture == CPU_TYPE_ARM64)
#if SUPPORT_ARCH_arm64_32
          || (fArchitecture == CPU_TYPE_ARM64_32)
#endif
          )
        && (fOutputKind == kDynamicExecutable) ) {
		fPositionIndependentExecutable = true;
		if ( fDisablePositionIndependentExecutable )
			warning("-no_pie ignored for arm64");
	}

	// set fOutputSlidable
	switch ( fOutputKind ) {
		case Options::kObjectFile:
			fOutputSlidable = false;
			break;
		case Options::kStaticExecutable:
		case Options::kDynamicExecutable:
			fOutputSlidable = fPositionIndependentExecutable;
			break;
		case Options::kPreload:
			fOutputSlidable = fPIEOnCommandLine;
			break;
		case Options::kDyld:
		case Options::kDynamicLibrary:
		case Options::kDynamicBundle:
		case Options::kKextBundle:
			fOutputSlidable = true;
			break;
	}

	// Let linker know if thread local variables are supported
	// This is more complex than normal version checks since
	// runtime support varies by architecture
	if (platforms().minOS(ld::supportsTLV)
			|| ((fArchitecture == CPU_TYPE_ARM64) && platforms().minOS(ld::iOS_8_0))
			|| ((fArchitecture == CPU_TYPE_X86_64) && platforms().minOS(ld::iOS_9_0))) {
		fTLVSupport = true;
	}

	// default to adding version load command for dynamic code, static code must opt-in
	switch ( fOutputKind ) {
		case Options::kObjectFile:
			fVersionLoadCommand = false;
			break;
		case Options::kStaticExecutable:
		case Options::kPreload:
		case Options::kKextBundle:
			if ( fVersionLoadCommandForcedOn )
				fVersionLoadCommand = true;
			break;
		case Options::kDynamicExecutable:
		case Options::kDyld:
		case Options::kDynamicLibrary:
		case Options::kDynamicBundle:
			if ( !fVersionLoadCommandForcedOff )
				fVersionLoadCommand = true;
			break;
	}
	
	// support re-export of individual symbols in MacOSX 10.7 and iOS 4.2
	if ( (fOutputKind == kDynamicLibrary) && platforms().minOS(ld::version2010) )
		fCanReExportSymbols = true;
	
	// ObjC optimization is only in dynamic final linked images
	switch ( fOutputKind ) {
		case Options::kObjectFile:
		case Options::kStaticExecutable:
		case Options::kPreload:
		case Options::kKextBundle:
		case Options::kDyld:
			fObjcCategoryMerging = false;
			break;
		case Options::kDynamicExecutable:
		case Options::kDynamicLibrary:
		case Options::kDynamicBundle:
			break;
	}

	// i386 main executables linked on Mac OS X 10.7 default to NX heap
	// regardless of target unless overriden with -allow_heap_execute anywhere
	// on the command line
	if ( (fArchitecture == CPU_TYPE_I386) && (fOutputKind == kDynamicExecutable) && !fDisableNonExecutableHeap)
		fNonExecutableHeap = true;
		
	// Use LC_MAIN instead of LC_UNIXTHREAD for newer OSs
	switch ( fOutputKind ) {
		case Options::kDynamicExecutable:
			if ( platforms().contains(ld::Platform::driverKit) )
				break;
			// <rdar://problem/16310363> Look for "_main" not "start" when building for all recent OS versions and architectures
			if ( platforms().minOS(ld::version2012) || (fArchitecture == CPU_TYPE_ARM64)
		#if SUPPORT_ARCH_arm64_32
													|| (fArchitecture == CPU_TYPE_ARM64_32)
		#endif
			) {
				fEntryPointLoadCommand = true;
				if ( fEntryName == NULL )
					fEntryName = "_main";
				if ( strcmp(fEntryName, "start") == 0 ) {
					warning("Ignoring '-e start' because entry point 'start' is not used for the targeted OS version");
					fEntryName = "_main";
				}
			}
			else {
				fNeedsThreadLoadCommand = true;
				if ( fEntryName == NULL )
					fEntryName = "start";
			}
			break;
		case Options::kObjectFile:
		case Options::kKextBundle:
		case Options::kDynamicLibrary:
		case Options::kDynamicBundle:
			break;
			
		case Options::kStaticExecutable:
		case Options::kPreload:
		case Options::kDyld:
			fNeedsThreadLoadCommand = true;
			if ( fEntryName == NULL ) 
				fEntryName = "start";  // Perhaps these should have no default and require -e
			break;
	}
	
	// add LC_SOURCE_VERSION
	switch ( fOutputKind ) {
		case Options::kDynamicExecutable:
		case Options::kKextBundle:
		case Options::kDynamicLibrary:
		case Options::kDynamicBundle:
		case Options::kDyld:
		case Options::kStaticExecutable:
			if ( fSourceVersionLoadCommandForceOn ) {
				fSourceVersionLoadCommand = true;
			}
			else if ( fSourceVersionLoadCommandForceOff ) {
				fSourceVersionLoadCommand = false;
			}
			else {
				if ( platforms().minOS(ld::version2012) ) {
					fSourceVersionLoadCommand = true;
				}
				else
					fSourceVersionLoadCommand = false;
			}
			break;
		case Options::kObjectFile:
		case Options::kPreload:
			fSourceVersionLoadCommand = false;
			break;
	}

	if (fSDKVersion != 0) {
		if (platforms().count() > 1) {
			throw "-sdk_version may not be used for zippered binaries";
		}
		if (fPlatfromVersionCmdFound) {
			warning("-sdk_version and -platform_version are not compatible, ignoring -sdk_version");
		} else {
			// This is kind of gross, but we will remove it once -platform_version is adopted, so
			// there is no point in adding a nicer interface to VersionSet
			__block ld::Platform currentPlatform;
			fPlatforms.forEach(^(ld::Platform platform, uint32_t minVersion, uint32_t sdkVersion, bool &stop) {
				currentPlatform = platform;
			});
			fPlatforms.updateSDKVersion(currentPlatform, fSDKVersion);
			//warning("-sdk_version is deprecated, please switch to -platform_version");
		}
	} else {
		bool inferredFromSDKpath = false;
		// if -sdk_version not on command line, infer from -syslibroot
		if ( !fPlatfromVersionCmdFound && (fSDKPaths.size() > 0) ) {
			const char* sdkPath = fSDKPaths.front();
			const char* end = &sdkPath[strlen(sdkPath)-1];
			while ( !isdigit(*end) && (end > sdkPath) )
				--end;
			const char* start = end-1;
			while ( (isdigit(*start) || (*start == '.')) && (start > sdkPath))
				--start;
			char sdkVersionStr[32];
			int len = end-start+1;
			if ( len > 2 ) {
				strlcpy(sdkVersionStr, start+1, len);
				__block ld::Platform currentPlatform;
				fPlatforms.forEach(^(ld::Platform platform, uint32_t minVersion, uint32_t sdkVersion, bool &stop) {
					if ( platform != ld::Platform::iOSMac )
						currentPlatform = platform;
				});
				fPlatforms.updateSDKVersion(currentPlatform, parseVersionNumber32(sdkVersionStr));
				inferredFromSDKpath = true;
			}
		}

		// if no version was explicitly set on the commandline or by an environment variable, and the
		// the platform is macOS then set the min version to the current
#ifdef __APPLE__
		if ( !fPlatfromVersionCmdFound && !inferredFromSDKpath && (fSDKVersion == 0) && platforms().contains(ld::Platform::macOS)
			&& !(getenv("RC_ProjectName") && getenv("MACOSX_DEPLOYMENT_TARGET")) && (fOutputKind != Options::kObjectFile) ) {
			int mib[2] = { CTL_KERN, KERN_OSRELEASE };
			char kernVersStr[100];
			size_t strlen = sizeof(kernVersStr);
			if ( sysctl(mib, 2, kernVersStr, &strlen, NULL, 0) != -1 ) {
				uint32_t kernVers = parseVersionNumber32(kernVersStr);
				int minor = (kernVers >> 16) - 4;  // kernel major version is 4 ahead of x in 10.
				fPlatforms.updateSDKVersion(ld::Platform::macOS, (0x000A0000 + (minor << 8)));
			}
		}
#endif
	}

	// allow trie based absolute symbols if targeting new enough OS
	if ( fMakeCompressedDyldInfo || fMakeChainedFixups ) {
		if ( platforms().minOS(ld::version2013) ) {
			fAbsoluteSymbols = true;
		}
	}

	// <rdar://problem/13070042> Only third party apps should have 16KB page segments by default
	if ( fEncryptable ) {
		if ( fSegmentAlignment == 4096 )
			fSegmentAlignment = 4096*4;
	}
  
	// <rdar://problem/12258065> ARM64 needs 16KB page size for user land code
	// <rdar://problem/15974532> make armv7[s] use 16KB pages in user land code for iOS 8 or later
	if ( fSegmentAlignment == LD_PAGE_SIZE ) {
		switch ( fOutputKind ) {
			case Options::kDynamicExecutable:
			case Options::kDynamicLibrary:
			case Options::kDynamicBundle:
			case Options::kDyld:
				// <rdar://problem/14676611> 16KB segments for arm64 kexts
				if ( (fArchitecture == CPU_TYPE_ARM64)
#if SUPPORT_ARCH_arm64_32
                      || (fArchitecture == CPU_TYPE_ARM64_32)
#endif
                     || (fArchitecture == CPU_TYPE_ARM) ) {
					fSegmentAlignment = 4096*4;
				}
				break;
			case Options::kStaticExecutable:
			case Options::kKextBundle:
				// <rdar://problem/66078724> Kernel images built for x86_64 have 16k aligned segments
				if ( fArchitecture == CPU_TYPE_X86_64 )
					fSegmentAlignment = 4096;
				// <rdar://problem/14676611> 16KB segments for arm64 kexts
				if ( fArchitecture == CPU_TYPE_ARM64 )
					fSegmentAlignment = 4096*4;
#if SUPPORT_ARCH_arm64_32
				if ( fArchitecture == CPU_TYPE_ARM64_32 )
					fSegmentAlignment = 4096*4;
#endif
				break;
			case Options::kObjectFile:
				break;
			case Options::kPreload:
				if ( fArchitecture == CPU_TYPE_ARM )
					fSegmentAlignment = 4096;
				break;
		}
	}



	// <rdar://problem/13624134> linker should not convert dwarf unwind if .o file has compact unwind section
	switch ( fOutputKind ) {
		case Options::kDynamicExecutable:
		case Options::kDynamicLibrary:
		case Options::kDynamicBundle:
		case Options::kDyld:
			if ( fKeepDwarfUnwindForcedOn ) {
				fKeepDwarfUnwind = true;
			}
			else if ( fKeepDwarfUnwindForcedOff ) {
				fKeepDwarfUnwind = false;
			}
			else {
				if ( platforms().minOS(ld::version2013) )
					fKeepDwarfUnwind = false;
				else
					fKeepDwarfUnwind = true;
			}
			break;
		case Options::kKextBundle:
		case Options::kStaticExecutable:
		case Options::kObjectFile:
		case Options::kPreload:
			fKeepDwarfUnwind = true;
			break;
	}
	
	// Make sure -image_base matches alignment
	uint64_t alignedBaseAddress = (fBaseAddress+fSegmentAlignment-1) & (-fSegmentAlignment);
	if ( alignedBaseAddress != fBaseAddress ) {
		warning("base address 0x%llX is not properly aligned. Changing it to 0x%llX", fBaseAddress, alignedBaseAddress);
		fBaseAddress = alignedBaseAddress;
	}

	// <rdar://problem/20503811> Reduce the default alignment of structures/arrays to save memory in embedded systems
	if ( fMaxDefaultCommonAlign == 0 ) {
		if ( fOutputKind == Options::kPreload )
			fMaxDefaultCommonAlign = 8;
		else
			fMaxDefaultCommonAlign = 15;
	}

	// Add warnings for issues likely to cause OS verification issues
	if ( fSharedRegionEligible && !fRPaths.empty() && !fDebugVariant ) {
		// <rdar://problem/18719327> warn if -rpath is used with OS dylibs
		warning("OS dylibs should not add rpaths (linker option: -rpath) (Xcode build setting: LD_RUNPATH_SEARCH_PATHS)");
	}
	if ( (fOutputKind == Options::kDynamicLibrary) && (fDylibInstallName != NULL) && (fFinalName != NULL) && sharedCacheEligiblePath(fFinalName) && !fDebugVariant ) {
		if ( strncmp(fDylibInstallName, "@rpath", 6) == 0 )
			warning("OS dylibs should not use @rpath for -install_name.  Use absolute path instead");
		if ( strcmp(fDylibInstallName, fFinalName) != 0 )
			warning("OS dylibs -install_name should match its real absolute path");
	}

#if SUPPORT_ARCH_arm64e
	// arm64e chained fixups do not support misaligned pointers
	if ( (fArchitecture == CPU_TYPE_ARM64) && (fSubArchitecture == CPU_SUBTYPE_ARM64E) ) {
		if ( fUnalignedPointerTreatment == Options::kUnalignedPointerWarning )
			warning("for arm64e unaligned pointer errors are fatal");
		fUnalignedPointerTreatment = Options::kUnalignedPointerError;
	}
#endif
	// set if unaligned pointers are warnings or errors
	if ( fUnalignedPointerTreatment == Options::kUnalignedPointerIgnore ) {
		if ( platforms().minOS(ld::mac10_12) && fSharedRegionEligible ) {
			fUnalignedPointerTreatment = Options::kUnalignedPointerWarning;
		}
		else if ( platforms().minOS(ld::iOS_10_0) ) {
			fUnalignedPointerTreatment = Options::kUnalignedPointerWarning;
		}
	}

	// warn by default for OS dylibs
	if ( fInitializersTreatment == Options::kInvalid ) {
		if ( fSharedRegionEligible && (fOutputKind == Options::kDynamicLibrary) && !fDebugVariant ) {
			fInitializersTreatment = Options::kWarning;
		}
		else
			fInitializersTreatment = Options::kSuppress;
	}
	
	// OS dylibs get unused-dylib-warning by default
	if ( fWarnUnusedDylibsForceOn ) {
		fWarnUnusedDylibs = true;
	}
	else if ( fWarnUnusedDylibsForceOff ) {
		fWarnUnusedDylibs = false;
	}
	else {
		fWarnUnusedDylibs = fSharedRegionEligible;
	}


	// targeting newer OSs gets you relative objc method lists
	if ( fForceObjCRelativeMethodListsOn ) {
		fUseObjCRelativeMethodLists = true;
	}
	else if ( fForceObjCRelativeMethodListsOff ) {
		fUseObjCRelativeMethodLists = false;
	}
	else if ( !dyldLoadsOutput() ) {
		// don't even look for objc in non-dynamic output
		fUseObjCRelativeMethodLists = false;
	}
	else if ( (fOutputKind == Options::kDynamicExecutable) && (fArchitecture == CPU_TYPE_X86_64) ) {
		// main executables might be tools and might need to run on older builders
		fUseObjCRelativeMethodLists = false;
	}
	else {
		fUseObjCRelativeMethodLists = platforms().minOS(ld::version2020Fall);
	}

	// <rdar://problem/51911409> codesign all userland arm64 macOS binaries
	if ( dyldLoadsOutput() && (fArchitecture == CPU_TYPE_ARM64) && platforms().contains(ld::Platform::macOS) )
		fAdHocSign = true;

}

void Options::checkIllegalOptionCombinations()
{
	// check -undefined setting
	switch ( fUndefinedTreatment ) {
		case kUndefinedError:
			// always legal
			break;
		case kUndefinedDynamicLookup: {
			platforms().forEach(^(ld::Platform platform, uint32_t minVersion, uint32_t sdkVersion, bool &stop) {
				switch (platform) {
					case ld::Platform::macOS:
						if ( fSharedRegionEligible && dyldLoadsOutput() )
							warning("-undefined dynamic_lookup is incompatible with dyld share cache");
						break;
					case ld::Platform::iOSMac:
					case ld::Platform::unknown:
						break;
					case ld::Platform::iOS:
					case ld::Platform::iOS_simulator:
					case ld::Platform::watchOS:
					case ld::Platform::watchOS_simulator:
					case ld::Platform::bridgeOS:
					case ld::Platform::tvOS:
					case ld::Platform::tvOS_simulator:
					case ld::Platform::freestanding:
					case ld::Platform::driverKit:
						if ( fOutputKind != kKextBundle )
							warning("-undefined dynamic_lookup is deprecated on %s", nameFromPlatform(platform));
						break;
				}
			});
		} break;
		case kUndefinedWarning:
		case kUndefinedSuppress:
			// requires flat namespace
			if ( fNameSpace == kTwoLevelNameSpace )
				throw "can't use -undefined warning or suppress with -twolevel_namespace";
			break;
	}

	// unify -sub_umbrella with dylibs
	for (std::vector<const char*>::iterator it = fSubUmbellas.begin(); it != fSubUmbellas.end(); it++) {
		const char* subUmbrella = *it;
		bool found = false;
		for (std::vector<Options::FileInfo>::iterator fit = fInputFiles.begin(); fit != fInputFiles.end(); fit++) {
			Options::FileInfo& info = *fit;
			const char* lastSlash = strrchr(info.path, '/');
			if ( lastSlash == NULL )
				lastSlash = info.path - 1;
			std::string path(&lastSlash[1]);
			auto idx = path.find(".tbd", path.size() - 4);
			if (idx != std::string::npos)
				path.erase(idx);
			if ( path == subUmbrella ) {
				info.options.fReExport = true;
				found = true;
                fLinkSnapshot.recordSubUmbrella(info.path);
				break;
			}
		}
		if ( ! found  )
			warning("-sub_umbrella %s does not match a supplied dylib", subUmbrella);
	}

	// unify -sub_library with dylibs
	for (std::vector<const char*>::iterator it = fSubLibraries.begin(); it != fSubLibraries.end(); it++) {
		const char* subLibrary = *it;
		bool found = false;
		for (std::vector<Options::FileInfo>::iterator fit = fInputFiles.begin(); fit != fInputFiles.end(); fit++) {
			Options::FileInfo& info = *fit;
			const char* lastSlash = strrchr(info.path, '/');
			if ( lastSlash == NULL )
				lastSlash = info.path - 1;
			const char* dot = strchr(&lastSlash[1], '.');
			if ( dot == NULL )
				dot = &lastSlash[strlen(lastSlash)];
			if ( strncmp(&lastSlash[1], subLibrary, dot-lastSlash-1) == 0 ) {
				info.options.fReExport = true;
				found = true;
                fLinkSnapshot.recordSubLibrary(info.path);
				break;
			}
		}
		if ( ! found  )
			warning("-sub_library %s does not match a supplied dylib", subLibrary);
	}

	// sync reader options
	if ( fNameSpace != kTwoLevelNameSpace ) {
		fFlatNamespace = true;
		platforms().forEach(^(ld::Platform platform, uint32_t minVersion, uint32_t sdkVersion, bool &stop) {
			switch (platform) {
				case ld::Platform::unknown:
				case ld::Platform::macOS:
				case ld::Platform::iOSMac:
					break;
				case ld::Platform::iOS:
				case ld::Platform::iOS_simulator:
				case ld::Platform::watchOS:
				case ld::Platform::watchOS_simulator:
				case ld::Platform::bridgeOS:
				case ld::Platform::tvOS:
				case ld::Platform::tvOS_simulator:
				case ld::Platform::driverKit:
				case ld::Platform::freestanding:
					warning("-flat_namespace is deprecated on %s", nameFromPlatform(platform));
					break;
			}
		});
	}


	// check -stack_addr
	if ( fStackAddr != 0 ) {
		switch (fArchitecture) {
			case CPU_TYPE_I386:
            case CPU_TYPE_ARM:
#if SUPPORT_ARCH_arm64_32
			case CPU_TYPE_ARM64_32:
#endif
				if ( fStackAddr > 0xFFFFFFFF )
					throw "-stack_addr must be < 4G for 32-bit processes";
				break;
			case CPU_TYPE_X86_64:
			case CPU_TYPE_ARM64:
				break;
		}
		if ( (fStackAddr & -4096) != fStackAddr )
			throw "-stack_addr must be multiples of 4K";
		if ( fStackSize  == 0 )
			throw "-stack_addr must be used with -stack_size";
	}

	// check -stack_size
	if ( fStackSize != 0 ) {
		switch (fArchitecture) {
			case CPU_TYPE_I386:
				if ( platforms().contains(ld::Platform::macOS) ) {
					if ( fStackSize > 0xFFFFFFFF )
						throw "-stack_size must be < 4GB for 32-bit processes";
					if ( fStackAddr == 0 )
						fStackAddr = 0xC0000000;
					if ( (fStackAddr > 0xB0000000) && ((fStackAddr-fStackSize) < 0xB0000000)  )
						warning("custom stack placement overlaps and will disable shared region");
				}
				else {
					if ( fStackSize > 0x1F000000 )
						throw "-stack_size must be < 496MB";
					if ( fStackAddr == 0 )
						fStackAddr = 0xC0000000;
				}
				break;
            case CPU_TYPE_ARM:
				if ( fStackSize > 0x1F000000 )
					throw "-stack_size must be < 496MB";
				if ( fStackAddr == 0 )
					fStackAddr = 0x1F000000;
                if ( fStackAddr > 0x20000000)
                    throw "-stack_addr must be < 0x20000000 for arm";
				break;
			case CPU_TYPE_X86_64:
				if ( platforms().contains(ld::Platform::macOS) ) {
					if ( fStackSize > 0x10000000000 )
						throw "-stack_size must be <= 1TB";
					if ( fStackAddr == 0 ) {
						fStackAddr = 0x00007FFF5C000000LL;
					}
				}
				else {
					if ( fStackSize > 0x20000000 )
						throw "-stack_size must be <= 512MB";
					if ( fStackAddr == 0 ) {
						fStackAddr = 0x120000000;
				}
				break;
			case CPU_TYPE_ARM64:
				if ( fStackSize > 0x20000000 )
					throw "-stack_size must be <= 512MB";
				if ( fStackAddr == 0 )
					fStackAddr = 0x120000000;
				}
				break;
#if SUPPORT_ARCH_arm64_32
			case CPU_TYPE_ARM64_32:
				if ( fStackSize > 0x20000000 )
					throw "-stack_size must be < 512MB";
				if ( fStackAddr == 0 )
					fStackAddr = 0x2F000000;
				break;
#endif
		}
		if ( (fStackSize & (-fSegmentAlignment)) != fStackSize )
			throwf("-stack_size must be multiple of segment alignment (%lldKB)", fSegmentAlignment/1024);
		switch ( fOutputKind ) {
			case Options::kDynamicExecutable:
			case Options::kStaticExecutable:
				// custom stack size only legal when building main executable
				break;
			case Options::kDynamicLibrary:
			case Options::kDynamicBundle:
			case Options::kObjectFile:
			case Options::kDyld:
			case Options::kPreload:
			case Options::kKextBundle:
				throw "-stack_size option can only be used when linking a main executable";
		}
		if ( fStackSize > fStackAddr )
			throwf("-stack_size (0x%08llX) must be smaller than -stack_addr (0x%08llX)", fStackSize, fStackAddr);
	}

	// check that -allow_stack_execute is only used with main executables
	if ( fExecutableStack ) {
		switch ( fOutputKind ) {
			case Options::kDynamicExecutable:
			case Options::kStaticExecutable:
				// -allow_stack_execute size only legal when building main executable
				break;
			case Options::kDynamicLibrary:
			case Options::kDynamicBundle:
			case Options::kObjectFile:
			case Options::kDyld:
			case Options::kPreload:
			case Options::kKextBundle:
				throw "-allow_stack_execute option can only be used when linking a main executable";
		}
	}

	// check that -allow_heap_execute is only used with i386 main executables
	if ( fDisableNonExecutableHeap ) {
		if ( fArchitecture != CPU_TYPE_I386 )
			throw "-allow_heap_execute option can only be used when linking for i386";
		switch ( fOutputKind ) {
			case Options::kDynamicExecutable:
				// -allow_heap_execute only legal when building main executable
				break;
			case Options::kStaticExecutable:
			case Options::kDynamicLibrary:
			case Options::kDynamicBundle:
			case Options::kObjectFile:
			case Options::kDyld:
			case Options::kPreload:
			case Options::kKextBundle:
				throw "-allow_heap_execute option can only be used when linking a main executable";
		}
	}

	// check -client_name is only used when making a bundle or main executable
	if ( fClientName != NULL ) {
		switch ( fOutputKind ) {
			case Options::kDynamicExecutable:
			case Options::kDynamicBundle:
				break;
			case Options::kStaticExecutable:
			case Options::kDynamicLibrary:
			case Options::kObjectFile:
			case Options::kDyld:
			case Options::kPreload:
			case Options::kKextBundle:
				throw "-client_name can only be used with -bundle";
		}
	}
	
	// check -init is only used when building a dylib
	if ( (fInitFunctionName != NULL) && (fOutputKind != Options::kDynamicLibrary) )
		throw "-init can only be used with -dynamiclib";

	// check -bundle_loader only used with -bundle
	if ( (fBundleLoader != NULL) && (fOutputKind != Options::kDynamicBundle) )
		throw "-bundle_loader can only be used with -bundle";

	// check -dtrace not used with -r
	if ( (fDtraceScriptName != NULL) && (fOutputKind == Options::kObjectFile) )
		throw "-dtrace can only be used when creating final linked images";

	// check -d can only be used with -r
	if ( fMakeTentativeDefinitionsReal && (fOutputKind != Options::kObjectFile) )
		throw "-d can only be used with -r";

	// check that -root_safe is not used with -r
	if ( fRootSafe && (fOutputKind == Options::kObjectFile) )
		throw "-root_safe cannot be used with -r";

	// check that -setuid_safe is not used with -r
	if ( fSetuidSafe && (fOutputKind == Options::kObjectFile) )
		throw "-setuid_safe cannot be used with -r";

	// <rdar://problem/12781832> compiler driver no longer uses -objc_abi_version, it uses -ios_simulator_version_min instead
	if ( !fObjCABIVersion1Override && !fObjCABIVersion2Override && targetIOSSimulator() )
		fObjCABIVersion2Override = true;

	// rdar://problem/4718189 map ObjC class names to new runtime names
	bool alterObjC1ClassNamesToObjC2 = false;
	switch (fArchitecture) {
		case CPU_TYPE_I386:
			// i386 only uses new symbols when using objc2 ABI
			if ( fObjCABIVersion2Override )
				alterObjC1ClassNamesToObjC2 = true;
			break;
		case CPU_TYPE_X86_64:
		case CPU_TYPE_ARM:
		case CPU_TYPE_ARM64:
#if SUPPORT_ARCH_arm64_32
		case CPU_TYPE_ARM64_32:
#endif
			alterObjC1ClassNamesToObjC2 = true;
			break;
	}

	// make sure all required exported symbols exist
	std::vector<const char*> impliedExports;
	for (NameSet::const_iterator it=fExportSymbols.regularBegin(); it != fExportSymbols.regularEnd(); ++it) {
		const char* name = *it;
		const int len = strlen(name);
		if ( (strcmp(&name[len-3], ".eh") == 0) || (strncmp(name, ".objc_category_name_", 20) == 0) ) {
			// never export .eh symbols
			warning("ignoring %s in export list", name);
		}
		else if ( (fArchitecture == CPU_TYPE_I386) && !fObjCABIVersion2Override && (strncmp(name, "_OBJC_CLASS_$", 13) == 0) ) {
			warning("ignoring Objc2 Class symbol %s in i386 export list", name);
			fRemovedExports.insert(name);
		}
		else if ( alterObjC1ClassNamesToObjC2 && (strncmp(name, ".objc_class_name_", 17) == 0) ) {
			// linking ObjC2 ABI, but have ObjC1 ABI name in export list.  Change it to intended name
			fRemovedExports.insert(name);
			char* temp;
			asprintf(&temp, "_OBJC_CLASS_$_%s", &name[17]);
			impliedExports.push_back(temp);
			asprintf(&temp, "_OBJC_METACLASS_$_%s", &name[17]);
			impliedExports.push_back(temp);
		}
		else {
			fInitialUndefines.push_back(name);
		}
	}
	fExportSymbols.remove(fRemovedExports);
	for (std::vector<const char*>::iterator it=impliedExports.begin(); it != impliedExports.end(); ++it) {
		const char* name = *it;
		fExportSymbols.insert(name, kAllowWildcards);
		fInitialUndefines.push_back(name);
	}

	// make sure all required re-exported symbols exist
	for (NameSet::const_iterator it=fReExportSymbols.regularBegin(); it != fReExportSymbols.regularEnd(); ++it) {
		fInitialUndefines.push_back(*it);
	}
	
	// make sure that -init symbol exists
	if ( fInitFunctionName != NULL )
		fInitialUndefines.push_back(fInitFunctionName);

	// make sure that entry symbol exists
	switch ( fOutputKind ) {
		case Options::kDynamicExecutable:
		case Options::kStaticExecutable:
		case Options::kDyld:
		case Options::kPreload:
			if ( !platforms().contains(ld::Platform::driverKit) )
				fInitialUndefines.push_back(fEntryName);
			break;
		case Options::kDynamicLibrary:
		case Options::kDynamicBundle:
		case Options::kObjectFile:
		case Options::kKextBundle:
			break;
	}

	// make sure every alias base exists
	for (std::vector<AliasPair>::iterator it=fAliases.begin(); it != fAliases.end(); ++it) {
		fInitialUndefines.push_back(it->realName);
	}

	// check custom segments
	if ( fCustomSegmentAddresses.size() != 0 ) {
		// verify no segment is in zero page
		if ( fZeroPageSize != ULLONG_MAX ) {
			for (std::vector<SegmentStart>::iterator it = fCustomSegmentAddresses.begin(); it != fCustomSegmentAddresses.end(); ++it) {
				if ( it->address < fZeroPageSize )
					throwf("-segaddr %s 0x%llX conflicts with -pagezero_size", it->name, it->address);
			}
		}
		// verify no duplicates
		for (std::vector<SegmentStart>::iterator it = fCustomSegmentAddresses.begin(); it != fCustomSegmentAddresses.end(); ++it) {
			for (std::vector<SegmentStart>::iterator it2 = fCustomSegmentAddresses.begin(); it2 != fCustomSegmentAddresses.end(); ++it2) {
				if ( (it->address == it2->address) && (it != it2) )
					throwf("duplicate -segaddr addresses for %s and %s", it->name, it2->name);
			}
			// a custom segment address of zero will disable the use of a zero page
			if ( it->address == 0 )
				fZeroPageSize = 0;
		}
	}

	if ( fKernel ) {
		if ( fOutputKind != Options::kStaticExecutable )
			throwf("-kernel must be used with -static");
	}

	// <rdar://problem/47805298> building chained fixups should not allow a custom load address
	if ( fMakeChainedFixups ) {
		switch ( fOutputKind ) {
			case Options::kDynamicExecutable:
				fZeroPageSize = ULLONG_MAX; // reset page-zero size
				[[clang::fallthrough]];
			case Options::kDynamicLibrary:
			case Options::kDynamicBundle:
			case Options::kDyld:
				if ( fBaseAddress != 0 ) {
					warning("prefered load addresses (-seg1addr) are disabled with chained fixups");
					fBaseAddress = 0;
				}
				break;
			case Options::kPreload:
			case Options::kKextBundle:
				break;
			case Options::kStaticExecutable:
				if ( !fKernel && !fMakeChainedFixupsSection )
					throwf("to use chained fixups with static executables, use -fixup_chains_section");
				break;
			case Options::kObjectFile:
				throwf("chained fixups cannot be used with relocatable object files");
		}
	}

	if ( fZeroPageSize == ULLONG_MAX ) {
		// zero page size not specified on command line, set default
		switch (fArchitecture) {
			case CPU_TYPE_I386:
			case CPU_TYPE_ARM:
#if SUPPORT_ARCH_arm64_32
			case CPU_TYPE_ARM64_32:
#endif
				// first 4KB for 32-bit architectures
				fZeroPageSize = 0x1000;
				break;
			case CPU_TYPE_ARM64:
			case CPU_TYPE_X86_64:
				// first 4GB for x86_64 on all OS's
				fZeroPageSize = 0x100000000ULL;
				break;
			default:
				// if -arch not used, default to 4K zero-page
				fZeroPageSize = 0x1000;
		}
	}
	else {
		switch ( fOutputKind ) {
			case Options::kDynamicExecutable:
			case Options::kStaticExecutable:
				// -pagezero_size size only legal when building main executable
				break;
			case Options::kDynamicLibrary:
			case Options::kDynamicBundle:
			case Options::kObjectFile:
			case Options::kDyld:
			case Options::kPreload:
			case Options::kKextBundle:
				if ( fZeroPageSize != 0 )
					throw "-pagezero_size option can only be used when linking a main executable";
		}
	}

	// if main executable with custom base address, model zero page as custom segment
	if ( (fOutputKind == Options::kDynamicExecutable) && (fBaseAddress != 0) && (fZeroPageSize != 0) ) {
		SegmentStart seg;
		seg.name = "__PAGEZERO";
		seg.address = 0;;
		fCustomSegmentAddresses.push_back(seg);
	}

	// -dead_strip and -r are incompatible
	if ( fDeadStrip && (fOutputKind == Options::kObjectFile) )
		throw "-r and -dead_strip cannot be used together";

	// can't use -rpath unless targeting 10.5 or later
	if ( fRPaths.size() > 0 ) {
		if ( !platforms().minOS(ld::version2008) )
			throw "-rpath can only be used when targeting Mac OS X 10.5 or later";
		switch ( fOutputKind ) {
			case Options::kDynamicExecutable:
			case Options::kDynamicLibrary:
			case Options::kDynamicBundle:
				break;
			case Options::kStaticExecutable:
			case Options::kObjectFile:
			case Options::kDyld:
			case Options::kPreload:
			case Options::kKextBundle:
				throw "-rpath can only be used when creating a dynamic final linked image";
		}
	}
	
	if ( fPositionIndependentExecutable ) {
		switch ( fOutputKind ) {
			case Options::kDynamicExecutable:
				// check -pie is only used when building a dynamic main executable for 10.5
				if ( !platforms().minOS(ld::supportsPIE) ) {
					throw "-pie requires targetting a newer minimum version";
				}
				break;
			case Options::kStaticExecutable:
			case Options::kPreload:
				// -pie is ok with -static or -preload
				break;
			case Options::kDynamicLibrary:
			case Options::kDynamicBundle:
				warning("-pie being ignored. It is only used when linking a main executable");
				fPositionIndependentExecutable = false;
				break;
			case Options::kObjectFile:
			case Options::kDyld:
			case Options::kKextBundle:
				throw "-pie can only be used when linking a main executable";
		}
	}
	
	// check -read_only_relocs is not used with x86_64 or arm64
	if ( fAllowTextRelocs && dyldOrKernelLoadsOutput() ) {
		if ( (fArchitecture == CPU_TYPE_X86_64) && (fOutputKind != kKextBundle) && !fKernel ) {
			warning("-read_only_relocs cannot be used with x86_64");
			fAllowTextRelocs = false;
		}
		else if ( fArchitecture == CPU_TYPE_ARM64 ) {
			warning("-read_only_relocs cannot be used with arm64");
		}
#if SUPPORT_ARCH_arm64_32
		else if ( fArchitecture == CPU_TYPE_ARM64_32 ) {
			warning("-read_only_relocs cannot be used with arm64_32");
		}
#endif
	}
	
	// check -mark_auto_dead_strip is only used with dylibs
	if ( fMarkDeadStrippableDylib ) {
		if ( fOutputKind != Options::kDynamicLibrary ) {
			warning("-mark_auto_dead_strip can only be used when creating a dylib");
			fMarkDeadStrippableDylib = false;
		}
	}
	
	// -force_cpusubtype_ALL is not supported for ARM
	if ( fForceSubtypeAll ) {
		if ( fArchitecture == CPU_TYPE_ARM ) {
			warning("-force_cpusubtype_ALL will become unsupported for ARM architectures");
		}
	}
	
	// -reexported_symbols_list can only be used with -dynamiclib
	if ( !fReExportSymbols.empty() ) {
		if ( fOutputKind != Options::kDynamicLibrary )
			throw "-reexported_symbols_list can only used used when created dynamic libraries";
		if ( !platforms().minOS(ld::version2010) )
			throw "targeted OS version does not support -reexported_symbols_list";
	}
	
	// -dyld_env can only be used with main executables
	if ( (fOutputKind != Options::kDynamicExecutable) && (fDyldEnvironExtras.size() != 0) )
		throw "-dyld_env can only used used when created main executables";

	// -segment_order can only be used with -preload or -static
	if ( !fSegmentOrder.empty() && ((fOutputKind != Options::kPreload) && (fOutputKind != kStaticExecutable)) )
		throw "-segment_order can only used used with -preload output";

	// warn about bitcode option combinations
	if ( !fBundleBitcode ) {
		if ( fVerifyBitcode )
			warning("-bitcode_verify is ignored without -bitcode_bundle");
		else if ( fHideSymbols )
			warning("-bitcode_hide_symbols is ignored without -bitcode_bundle");
	}
	if ( fReverseMapPath != NULL && !fHideSymbols ) {
		throw "-bitcode_symbol_map can only be used with -bitcode_hide_symbols";
	}
	// auto fix up the process type for strip -S.
	// when there is only one input and output type is object file, downgrade kBitcodeProcess to kBitcodeAsData.
	if ( fOutputKind == Options::kObjectFile && fInputFiles.size() == 1 && fBitcodeKind == Options::kBitcodeProcess )
		fBitcodeKind = Options::kBitcodeAsData;

	// <rdar://problem/17598404> warn if building an embedded iOS dylib for pre-iOS 8
	// <rdar://problem/18935714> How can we suppress "ld: warning: embedded dylibs/frameworks only run on iOS 8 or later" when building XCTest?
	if ( (fOutputKind == Options::kDynamicLibrary) && (fDylibInstallName != NULL) ) {
		if ( platforms().contains(ld::Platform::iOS) && !platforms().minOS(ld::iOS_8_0) && (fDylibInstallName[0] == '@') && !fEncryptableForceOff )
			warning("embedded dylibs/frameworks only run on iOS 8 or later");
	}

	// produce nicer error when no input
	if ( fInputFiles.empty() ) {
		throw "no object files specified";
	}

	// If the platform is only macOS check to see if the dylib is on the implicit zipper list
	if (fDylibInstallName && platforms().count() == 1 && platforms().contains(ld::Platform::macOS)) {
		std::vector<const char*> implictZipperPaths;
		for (const auto& sdkPath : fSDKPaths) {
			char autoZipperPath[MAXPATHLEN];
			strlcpy(autoZipperPath, sdkPath, MAXPATHLEN);
			strlcat(autoZipperPath, "/AppleInternal/LinkerAutoZipperList.txt", MAXPATHLEN);
			loadImplictZipperFile(autoZipperPath, implictZipperPaths);
		}
		for (const auto& zipperPath : implictZipperPaths) {
			if (strcmp(zipperPath, fDylibInstallName) == 0) {
				// FIXME: once we are reading the SDK version out of the actual SDK set this dynamically
				// For now hardcode to 13.0
				fPlatforms.insert(ld::PlatformVersion(ld::Platform::iOSMac, 0x000D0000));
				break;
			}
		}
	}

	// Check zippered combinations.
	if (platforms().count() > 2) {
		throw "Illegal platform count.  Only 2 platforms at a maximum can be specified";
	}

	if (platforms().contains(ld::Platform::iOSMac) && !platforms().minOS(ld::iOS_13_0)) {
		throwf("%s platform version must be at least 13.0", nameFromPlatform(ld::Platform::iOSMac));
	}

	// <rdar://problem/38155581> ld64 shouldn't allow final executables to have more than one version load command
	if ( platforms().contains(ld::Platform::iOSMac) && platforms().contains(ld::Platform::macOS) ) {
		if ( (fOutputKind != Options::kDynamicLibrary) && (fOutputKind != Options::kDynamicBundle) && (fOutputKind != Options::kObjectFile) ) {
			warning("Only dylibs and bundles can be zippered, changing output to be macOS only.");
			fPlatforms.erase(ld::Platform::iOSMac);
		}
	}

	for (const char* sdkPath : fSDKPaths) {
		std::string possiblePath = std::string(sdkPath) + "/AppleInternal/";
		struct stat statBuffer;
		if ( stat(possiblePath.c_str(), &statBuffer) == 0 ) {
			fInternalSDK = true;
			break;
		}
	}

	// <rdar://problem/39095109> Linker warning for i386 macOS binaries
	if ( (architecture() == CPU_TYPE_I386) && platforms().contains(ld::Platform::macOS) ) {
		if ( !fInternalSDK )
			warning("The i386 architecture is deprecated for macOS (remove from the Xcode build setting: ARCHS)");
	}
}

void Options::inferArchAndPlatform()
{
	// scan all input files, looking for a thin .o file.
	// the first one found is presumably the architecture to link
	uint8_t buffer[0x4000];
	for (const Options::FileInfo& fInfo : fInputFiles) {
		int fd = ::open(fInfo.path, O_RDONLY, 0);
		if ( fd != -1 ) {
			struct stat stat_buf;
			if ( fstat(fd, &stat_buf) != -1) {
				ssize_t readAmount = stat_buf.st_size;
				if ( 0x4000 < readAmount )
					readAmount = 0x4000;
				ssize_t amount = read(fd, buffer, readAmount);
				::close(fd);
				try {
					if ( amount >= readAmount ) {
						cpu_type_t    	type;
						cpu_subtype_t 	subtype;
						ld::VersionSet	platformsFound;
						if ( mach_o::relocatable::isObjectFile(buffer, amount, &type, &subtype, platformsFound) ) {
							if ( fArchitecture == 0 )
								setInferredArch(type, subtype);
							if ( platforms().empty() ) {
								if ( platformsFound.empty() ) {
									fPlatforms.insert(ld::PlatformVersion(ld::Platform::freestanding, 0, 0));
								}
								else {
									fPlatforms = platformsFound;
								}
								// only use compressed LINKEDIT for:
								//			Mac OS X 10.6 or later
								//			iOS 3.1 or later
								if ( !fMakeCompressedDyldInfo && platforms().minOS(ld::version2009) && !fMakeCompressedDyldInfoForceOff )
									fMakeCompressedDyldInfo = true;
								// Mac OS X 10.5 and iPhoneOS 2.0 support LC_REEXPORT_DYLIB
								if ( platforms().minOS(ld::version2008) )
									fUseSimplifiedDylibReExports = true;
							}
							return;
						}
					}
				}
				catch (const char* msg) {
					throwf("%s in %s", msg, fInfo.path);
				}
			}
		}
	}

	if ( platforms().empty() && (fOutputKind != Options::kObjectFile) )
		warning("platform not specified");

	if ( fArchitecture == 0 )
		warning("-arch not specified");
}

void Options::expandResponseFiles(int& argc, const char**& argv)
{
	int newargc = argc;
	char** newargv = (char**)argv;
	if (0 == ExpandResponseFiles(&newargc, &newargv)) {
		argc = newargc;
		argv = (const char**)newargv;
	}
}

void Options::checkForClassic(int argc, const char* argv[])
{
	// scan options
	bool archFound = false;
	bool staticFound = false;
	bool dtraceFound = false;
	bool kextFound = false;
	bool rFound = false;
	bool creatingMachKernel = false;
	bool newLinker = false;
	
	// build command line buffer in case ld crashes
#if 0 //__MAC_OS_X_VERSION_MIN_REQUIRED >= 1070
	CRSetCrashLogMessage(crashreporterBuffer);
#endif
	const char* srcRoot = getenv("SRCROOT");
	if ( srcRoot != NULL ) {
		strlcpy(crashreporterBuffer, "SRCROOT=", crashreporterBufferSize);
		strlcat(crashreporterBuffer, srcRoot, crashreporterBufferSize);
		strlcat(crashreporterBuffer, "\n", crashreporterBufferSize);
	}
#ifdef LD_VERS
	strlcat(crashreporterBuffer, LD_VERS, crashreporterBufferSize);
	strlcat(crashreporterBuffer, "\n", crashreporterBufferSize);
#endif
	strlcat(crashreporterBuffer, "ld ", crashreporterBufferSize);
	for(int i=1; i < argc; ++i) {
		strlcat(crashreporterBuffer, argv[i], crashreporterBufferSize);
		strlcat(crashreporterBuffer, " ", crashreporterBufferSize);
	}

	for(int i=0; i < argc; ++i) {
		const char* arg = argv[i];
		if ( arg[0] == '-' ) {
			if ( strcmp(arg, "-arch") == 0 ) {
				parseArch(argv[++i]);
				archFound = true;
			}
			else if ( strcmp(arg, "-static") == 0 ) {
				staticFound = true;
			}
			else if ( strcmp(arg, "-kext") == 0 ) {
				kextFound = true;
			}
			else if ( strcmp(arg, "-dtrace") == 0 ) {
				dtraceFound = true;
			}
			else if ( strcmp(arg, "-r") == 0 ) {
				rFound = true;
			}
			else if ( strcmp(arg, "-new_linker") == 0 ) {
				newLinker = true;
			}
			else if ( strcmp(arg, "-classic_linker") == 0 ) {
				// ld_classic does not understand this option, so remove it
				for(int j=i; j < argc; ++j)
					argv[j] = argv[j+1];
				warning("using ld_classic");
				this->gotoClassicLinker(argc-1, argv);
			}
			else if ( strcmp(arg, "-o") == 0 ) {
				const char* outfile = argv[++i];
				if ( (outfile != NULL) && (strstr(outfile, "/mach_kernel") != NULL) )
					creatingMachKernel = true;
			}
		}
	}
}

void Options::gotoClassicLinker(int argc, const char* argv[])
{
	argv[0] = "ld_classic";
	// ld_classic does not support -iphoneos_version_min, so change
	for(int j=0; j < argc; ++j) {
		if ( (strcmp(argv[j], "-iphoneos_version_min") == 0) || (strcmp(argv[j], "-ios_version_min") == 0) ) {
			argv[j] = "-macosx_version_min";
			if ( j < argc-1 )
				argv[j+1] = "10.5";
			break;
		}
	}
	// ld classic does not understand -kext (change to -static -r)
	for(int j=0; j < argc; ++j) {
		if ( strcmp(argv[j], "-kext") == 0) 
			argv[j] = "-r";
		else if ( strcmp(argv[j], "-dynamic") == 0) 
			argv[j] = "-static";
	}
	// ld classic does not understand -demangle 
	for(int j=0; j < argc; ++j) {
		if ( strcmp(argv[j], "-demangle") == 0) 
			argv[j] = "-noprebind";
	}
	// in -v mode, print command line passed to ld_classic
	for(int i=0; i < argc; ++i) {
		if ( strcmp(argv[i], "-v") == 0 ) {
			for(int j=0; j < argc; ++j)
				printf("%s ", argv[j]);
			printf("\n");
			break;
		}
	}
	char rawPath[PATH_MAX];
	char path[PATH_MAX];
	uint32_t bufSize = PATH_MAX;
	if ( _NSGetExecutablePath(rawPath, &bufSize) != -1 ) {
		if ( realpath(rawPath, path) != NULL ) {
			char* lastSlash = strrchr(path, '/');
			if ( lastSlash != NULL ) {
				strcpy(lastSlash+1, "ld_classic");
				argv[0] = path;
				execvp(path, (char**)argv);
			}
		}
	}
	// in case of error in above, try searching for ld_classic via PATH
	execvp(argv[0], (char**)argv);
	fprintf(stderr, "can't exec ld_classic\n");
	exit(1);
}


// Note, returned string buffer is own by this function.
// It should not be freed
// It will be reused, so clients need to strdup() if they want
// to use it long term.
const char* Options::demangleSymbol(const char* sym) const
{
	// only try to demangle symbols if -demangle on command line
	if ( !fDemangle )
		return sym;

	static size_t size = 1024;
	static char* buff = (char*)malloc(size);

#if DEMANGLE_SWIFT
	// only try to demangle symbols that look like Swift symbols
	if ( strncmp(sym, "_$", 2) == 0 ) {
		size_t demangledSize = fnd_get_demangled_name(&sym[1], buff, size);
		if ( demangledSize > size ) {
			size = demangledSize+2;
			buff = (char*)realloc(buff, size);
			demangledSize = fnd_get_demangled_name(&sym[1], buff, size);
		}
		if ( demangledSize != 0 )
			return buff;
	}
#endif

	// only try to demangle symbols that look like C++ symbols
	if ( strncmp(sym, "__Z", 3) != 0 )
		return sym;

	int status;
	char* result = abi::__cxa_demangle(&sym[1], buff, &size, &status); 
	if ( result != NULL ) {
		// if demangling successful, keep buffer for next demangle
		buff = result;
		return buff;
	}
	return sym;
}


void Options::writeDependencyInfo() const
{
	// do nothing if -dependency_info not used
	if ( !dumpDependencyInfo() )
		return;

	// <rdar://problem/30750137> sort entries for build reproducibility
	std::sort(fDependencies.begin(), fDependencies.end(), [](const DependencyEntry& a, const DependencyEntry& b) -> bool {
		if ( a.opcode != b.opcode )
			return (a.opcode < b.opcode);
		return (a.path < b.path);
	});

	// one time open() of -dependency_info file
	int fd = open(this->dependencyInfoPath(), O_WRONLY | O_TRUNC | O_CREAT, 0666);
	if ( fd == -1 )
		throwf("Could not open or create -dependency_info file: %s", this->dependencyInfoPath());

	// write header
	uint8_t version = depLinkerVersion;
	if ( write(fd, &version, 1) == -1 )
		throwf("write() to -dependency_info failed, errno=%d", errno);
	extern const char ldVersionString[];
	if ( write(fd, ldVersionString, strlen(ldVersionString)+1) == -1 )
		throwf("write() to -dependency_info failed, errno=%d", errno);

	// write each dependency
	for (const auto& entry: fDependencies) {
		//printf("%d %s\n", entry.opcode, entry.path.c_str());
		if ( write(fd, &entry.opcode, 1) == -1 )
			throwf("write() to -dependency_info failed, errno=%d", errno);
		if ( write(fd, entry.path.c_str(), entry.path.size()+1) == -1 )
			throwf("write() to -dependency_info failed, errno=%d", errno);
	}

	::close(fd);
}


void Options::addDependency(uint8_t opcode, const char* path) const
{
	if ( !this->dumpDependencyInfo() ) 
		return;

	char realPath[PATH_MAX];
	if ( path[0] != '/' ) {
		if ( realpath(path, realPath) != NULL ) {
			path = realPath;
		}
	}

	DependencyEntry entry;
	entry.opcode = opcode;
	entry.path   = path;
	fDependencies.push_back(entry);
}


void Options::writeToTraceFile(const char* buffer, size_t len) const
{
	// one time open() of custom LD_TRACE_FILE
	if ( fTraceFileDescriptor == -1 ) {
		if ( fTraceOutputFile != NULL ) {
			fTraceFileDescriptor = open(fTraceOutputFile, O_WRONLY | O_APPEND | O_CREAT, 0666);
			if ( fTraceFileDescriptor == -1 )
				throwf("Could not open or create trace file (errno=%d): %s", errno, fTraceOutputFile);
		}
		else {
			fTraceFileDescriptor = fileno(stderr);
		}
	}

	while (len > 0) {
		ssize_t amountWritten = write(fTraceFileDescriptor, buffer, len);
		if ( amountWritten == -1 )
			/* Failure to write shouldn't fail the build. */
			return;
		buffer += amountWritten;
		len -= amountWritten;
	}
}


uint64_t Options::machHeaderVmAddr() const
{
	if ( fOutputKind == Options::kDynamicExecutable )
		return fBaseAddress + fZeroPageSize;

	return fBaseAddress;
}

bool Options::fromSDK(const char* path) const
{
	for (const char* sdkPath : fSDKPaths) {
		if ( strncmp(path, sdkPath, strlen(sdkPath)) == 0 )
			return true;
	}
	return false;
}


