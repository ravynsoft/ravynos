/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2005-2010 Apple  Inc. All rights reserved.
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

#ifndef __OPTIONS__
#define __OPTIONS__


#include <stdint.h>
#include <mach/machine.h>
#include <tapi/tapi.h>

#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "ld.hpp"
#include "Snapshot.h"
#include "MachOFileAbstraction.hpp"


extern void throwf (const char* format, ...) __attribute__ ((noreturn,format(printf, 1, 2)));
extern void warning(const char* format, ...) __attribute__((format(printf, 1, 2)));

class Snapshot;

class LibraryOptions
{
public:
	LibraryOptions() : fWeakImport(false), fReExport(false), fBundleLoader(false), 
						fUpward(false), fIndirectDylib(false), fNeeded(false),
						fForceLoad(false), fLoadHidden(false) {}
	// for dynamic libraries
	bool		fWeakImport;
	bool		fReExport;
	bool		fBundleLoader;
	bool		fUpward;
	bool		fIndirectDylib;
	bool		fNeeded;
	// for static libraries
	bool		fForceLoad;
	bool		fLoadHidden;
};


inline bool isCompilerSupportLib(const char* path) {
	const char* libName = strrchr(path, '/');
	if ( libName == nullptr )
		return false;
	if ( strncmp(libName, "/libclang_rt", 12) == 0 )
		return true;
	if ( strncmp(libName, "/libcompiler-rt", 15) == 0 )
		return true;
	return false;
}

//
// The public interface to the Options class is the abstract representation of what work the linker
// should do.
//
// This abstraction layer will make it easier to support a future where the linker is a shared library
// invoked directly from Xcode.  The target settings in Xcode would be used to directly construct an Options
// object (without building a command line which is then parsed).
//
//
class Options
{
public:
	Options(int argc, const char* argv[]);
	~Options();

	enum OutputKind { kDynamicExecutable, kStaticExecutable, kDynamicLibrary, kDynamicBundle, kObjectFile, kDyld, kPreload, kKextBundle };
	enum NameSpace { kTwoLevelNameSpace, kFlatNameSpace, kForceFlatNameSpace };
	// Standard treatment for many options.
	enum Treatment { kError, kWarning, kSuppress, kNULL, kInvalid };
	enum UndefinedTreatment { kUndefinedError, kUndefinedWarning, kUndefinedSuppress, kUndefinedDynamicLookup };
	enum WeakReferenceMismatchTreatment { kWeakReferenceMismatchError, kWeakReferenceMismatchWeak,
										  kWeakReferenceMismatchNonWeak };
	enum CommonsMode { kCommonsIgnoreDylibs, kCommonsOverriddenByDylibs, kCommonsConflictsDylibsError };
	enum UUIDMode { kUUIDNone, kUUIDRandom, kUUIDContent };
	enum LocalSymbolHandling { kLocalSymbolsAll, kLocalSymbolsNone, kLocalSymbolsSelectiveInclude, kLocalSymbolsSelectiveExclude };
	enum BitcodeMode { kBitcodeProcess, kBitcodeAsData, kBitcodeMarker, kBitcodeStrip };
	enum DebugInfoStripping { kDebugInfoNone, kDebugInfoMinimal, kDebugInfoFull };
	enum UnalignedPointerTreatment { kUnalignedPointerError, kUnalignedPointerWarning, kUnalignedPointerIgnore };

	static void userReadableSwiftVersion(uint8_t value, char versionString[64])
	{
		switch (value) {
			case 1:
				strcpy(versionString, "1.0");
				break;
			case 2:
				strcpy(versionString, "1.1");
				break;
			case 3:
				strcpy(versionString, "2.0");
				break;
			case 4:
				strcpy(versionString, "3.0");
				break;
			case 5:
				strcpy(versionString, "4.0");
				break;
			case 6:
				strcpy(versionString, "4.1/4.2");
				break;
			case 7:
				strcpy(versionString, "5 or later");
				break;
			default:
				sprintf(versionString, "unknown ABI version 0x%02X", value);
		}
	}


	class FileInfo {
    public:
		const char*				path;
		time_t					modTime;
		LibraryOptions			options;
		ld::File::Ordinal		ordinal;
		bool					fromFileList;
		bool					isInlined;

		// These are used by the threaded input file parsing engine.
		mutable int				inputFileSlot;	// The input file "slot" assigned to this particular file
		bool					readyToParse;

        // The use pattern for FileInfo is to create one on the stack in a leaf function and return
        // it to the calling frame by copy. Therefore the copy constructor steals the path string from
        // the source, which dies with the stack frame.
        FileInfo(FileInfo const &other) : path(other.path), modTime(other.modTime), options(other.options), ordinal(other.ordinal), fromFileList(other.fromFileList), isInlined(other.isInlined), inputFileSlot(-1) { ((FileInfo&)other).path = NULL; };

		FileInfo &operator=(FileInfo other) {
			std::swap(path, other.path);
			std::swap(modTime, other.modTime);
			std::swap(options, other.options);
			std::swap(ordinal, other.ordinal);
			std::swap(fromFileList, other.fromFileList);
			std::swap(isInlined, other.isInlined);
			std::swap(inputFileSlot, other.inputFileSlot);
			std::swap(readyToParse, other.readyToParse);
			return *this;
		}

        // Create an empty FileInfo. The path can be set implicitly by checkFileExists().
        FileInfo() : path(NULL), modTime(-1), options(), fromFileList(false), isInlined(false) {};
        
        // Create a FileInfo for a specific path, but does not stat the file.
        FileInfo(const char *_path) : path(strdup(_path)), modTime(-1), options(), fromFileList(false), isInlined(false) {};

        ~FileInfo() { if (path) ::free((void*)path); }
        
        // Stat the file and update fileLen and modTime.
        // If the object already has a path the p must be NULL.
        // If the object does not have a path then p can be any candidate path, and if the file exists the object permanently remembers the path.
        // Returns true if the file exists, false if not.
        bool checkFileExists(const Options& options, const char *p=NULL);
        
        // Returns true if a previous call to checkFileExists() succeeded.
        // Returns false if the file does not exist of checkFileExists() has never been called.
        bool missing() const { return modTime == -1; }
	};

	class TAPIInterface {
	public:
		TAPIInterface(tapi::LinkerInterfaceFile* file, const char* path, const char *installName) :
			_file(file), _tbdPath(path), _installName(installName) {}
		tapi::LinkerInterfaceFile* getInterfaceFile() const { return _file; }
		std::string getTAPIFilePath() const { return _tbdPath; }
		const std::string &getInstallName() const { return _installName; }
	private:
		tapi::LinkerInterfaceFile* _file;
		std::string _tbdPath;
		std::string _installName;
	};
	
	struct ExtraSection {
		const char*				segmentName;
		const char*				sectionName;
		const char*				path;
		const uint8_t*			data;
		uint64_t				dataLen;
		typedef ExtraSection* iterator;
		typedef const ExtraSection* const_iterator;
	};

	struct SectionAlignment {
		const char*				segmentName;
		const char*				sectionName;
		uint8_t					alignment;
	};

	struct SectionOrderList {
		const char*					segmentName;
		std::vector<const char*>	sectionOrder;
	};

	struct OrderedSymbol {
		const char*				symbolName;
		const char*				objectFileName;
	};
	typedef const OrderedSymbol*	OrderedSymbolsIterator;

	struct SegmentStart {
		const char*				name;
		uint64_t				address;
	};
	
	struct SegmentSize {
		const char*				name;
		uint64_t				size;
	};
	
	struct SegmentProtect {
		const char*				name;
		uint32_t				max;
		uint32_t				init;
	};
	
	struct DylibOverride {
		const char*				installName;
		const char*				useInstead;
	};

	struct AliasPair {
		const char*			realName;
		const char*			alias;
	};

	struct SectionRename {
		const char*			fromSegment;
		const char*			fromSection;
		const char*			toSegment;
		const char*			toSection;
	};

	struct SegmentRename {
		const char*			fromSegment;
		const char*			toSegment;
	};

	enum { depLinkerVersion=0x00, depObjectFile=0x10, depDirectDylib=0x10, depIndirectDylib=0x10, 
		  depUpwardDirectDylib=0x10, depUpwardIndirectDylib=0x10, depArchive=0x10,
		  depFileList=0x10, depSection=0x10, depBundleLoader=0x10, depMisc=0x10, depNotFound=0x11,
		  depOutputFile = 0x40 };
	
	void						addDependency(uint8_t, const char* path) const;
	
	typedef const char* const*	UndefinesIterator;

//	const ObjectFile::ReaderOptions&	readerOptions();
	const char*							outputFilePath() const { return fOutputFile; }
	const std::vector<FileInfo>&		getInputFiles() const { return fInputFiles; }

	cpu_type_t					architecture() const { return fArchitecture; }
	bool						preferSubArchitecture() const { return fHasPreferredSubType; }
	cpu_subtype_t				subArchitecture() const { return fSubArchitecture & ~CPU_SUBTYPE_MASK; }
	uint8_t						subArchitectureFlags() const { return (fSubArchitecture & CPU_SUBTYPE_MASK) >> 24; }
	cpu_type_t                  fallbackArchitecture() const { return fFallbackArchitecture; }
	cpu_subtype_t				fallbackSubArchitecture() const { return fFallbackSubArchitecture; }
	bool						allowSubArchitectureMismatches() const { return fAllowCpuSubtypeMismatches; }
	bool						enforceDylibSubtypesMatch() const { return fEnforceDylibSubtypesMatch; }
	bool						warnOnSwiftABIVersionMismatches() const { return fWarnOnSwiftABIVersionMismatches; }
	bool						forceCpuSubtypeAll() const { return fForceSubtypeAll; }
	const char*					architectureName() const { return fArchitectureName; }
	void						setInferredArch(cpu_type_t, cpu_subtype_t subtype);
	void						setInferredPlatform(ld::Platform platform, uint32_t minOsVers);
	bool						archSupportsThumb2() const { return fArchSupportsThumb2; }
	OutputKind					outputKind() const { return fOutputKind; }
	bool						dyldLoadsOutput() const;
	bool						dyldOrKernelLoadsOutput() const;
	bool						bindAtLoad() const { return fBindAtLoad; }
	NameSpace					nameSpace() const { return fNameSpace; }
	const char*					installPath() const;			// only for kDynamicLibrary
	uint64_t					currentVersion() const { return fDylibCurrentVersion; }		// only for kDynamicLibrary
	uint32_t					currentVersion32() const;		// only for kDynamicLibrary
	uint32_t					compatibilityVersion() const { return fDylibCompatVersion; }	// only for kDynamicLibrary
	const char*					entryName() const { return fEntryName; }		// only for kDynamicExecutable or kStaticExecutable
	const char*					executablePath();
	uint64_t					baseAddress() const { return fBaseAddress; }
	uint64_t					maxAddress() const { return fMaxAddress; }
	bool						keepPrivateExterns() const { return fKeepPrivateExterns; }		// only for kObjectFile
	bool						interposable(const char* name) const;
	bool						hasExportRestrictList() const { return (fExportMode != kExportDefault); }	// -exported_symbol or -unexported_symbol
	bool						hasExportMaskList() const { return (fExportMode == kExportSome); }		// just -exported_symbol
	bool						hasWildCardExportRestrictList() const;
	bool						hasReExportList() const { return ! fReExportSymbols.empty(); }
	bool						wasRemovedExport(const char* sym) const { return ( fRemovedExports.find(sym) != fRemovedExports.end() ); }
	bool						allGlobalsAreDeadStripRoots() const;
	bool						shouldExport(const char*) const;
	bool						shouldReExport(const char*) const;
	std::vector<const char*>	exportsData() const;
	bool						ignoreOtherArchInputFiles() const { return fIgnoreOtherArchFiles; }
	bool						traceDylibs() const	{ return fTraceDylibs; }
	bool						traceArchives() const { return fTraceArchives; }
	bool						traceEmitJSON() const { return fTraceEmitJSON; }
	bool						deadCodeStrip()	const	{ return fDeadStrip; }
	UndefinedTreatment			undefinedTreatment() const { return fUndefinedTreatment; }
	uint32_t 					minOSversion(const ld::Platform& platform) const;
	bool						messagesPrefixedWithArchitecture();
	Treatment					picTreatment();
	WeakReferenceMismatchTreatment	weakReferenceMismatchTreatment() const { return fWeakReferenceMismatchTreatment; }
	const char*					umbrellaName() const { return fUmbrellaName; }
	const std::vector<const char*>&	allowableClients() const { return fAllowableClients; }
	const char*					clientName() const { return fClientName; }
	const char*					initFunctionName() const { return fInitFunctionName; }			// only for kDynamicLibrary
	const char*					dotOutputFile();
	uint64_t					pageZeroSize() const { return fZeroPageSize; }
	bool						hasCustomStack() const { return (fStackSize != 0); }
	uint64_t					customStackSize() const { return fStackSize; }
	uint64_t					customStackAddr() const { return fStackAddr; }
	bool						hasExecutableStack() const { return fExecutableStack; }
	bool						hasNonExecutableHeap() const { return fNonExecutableHeap; }
	UndefinesIterator			initialUndefinesBegin() const { return &fInitialUndefines[0]; }
	UndefinesIterator			initialUndefinesEnd() const { return &fInitialUndefines[fInitialUndefines.size()]; }
	const std::vector<const char*>&	initialUndefines() const { return fInitialUndefines; }
	bool						printWhyLive(const char* name) const;
	uint32_t					minimumHeaderPad() const { return fMinimumHeaderPad; }
	bool						maxMminimumHeaderPad() const { return fMaxMinimumHeaderPad; }
	ExtraSection::const_iterator	extraSectionsBegin() const { return &fExtraSections[0]; }
	ExtraSection::const_iterator	extraSectionsEnd() const { return &fExtraSections[fExtraSections.size()]; }
	CommonsMode					commonsMode() const { return fCommonsMode; }
	bool						warnCommons() const { return fWarnCommons; }
	bool						keepRelocations();
	FileInfo					findFile(const std::string &path, const ld::dylib::File* fromDylib=nullptr) const;
	bool						findFile(const std::string &path, const std::vector<std::string> &tbdExtensions, FileInfo& result) const;
	bool						hasInlinedTAPIFile(const std::string &path) const;
	tapi::LinkerInterfaceFile*	findTAPIFile(const std::string &path) const;
	UUIDMode					UUIDMode() const { return fUUIDMode; }
	bool						warnStabs();
	bool						pauseAtEnd() { return fPause; }
	bool						printStatistics() const { return fStatistics; }
	bool						printArchPrefix() const { return fMessagesPrefixedWithArchitecture; }
	void						gotoClassicLinker(int argc, const char* argv[]);
	bool						sharedRegionEligible() const { return fSharedRegionEligible; }
	bool						printOrderFileStatistics() const { return fPrintOrderFileStatistics; }
	const char*					orderFilePath() const { return fOrderFilePath; }
	const char*					dTraceScriptName() { return fDtraceScriptName; }
	bool						dTrace() { return (fDtraceScriptName != NULL); }
	unsigned long				orderedSymbolsCount() const { return fOrderedSymbols.size(); }
	OrderedSymbolsIterator		orderedSymbolsBegin() const { return &fOrderedSymbols[0]; }
	OrderedSymbolsIterator		orderedSymbolsEnd() const { return &fOrderedSymbols[fOrderedSymbols.size()]; }
	uint64_t					baseWritableAddress() { return fBaseWritableAddress; }
	uint64_t					segmentAlignment() const { return fSegmentAlignment; }
	uint64_t					segPageSize(const char* segName) const;
	uint64_t					customSegmentAddress(const char* segName) const;
	uint64_t					machHeaderVmAddr() const;
	bool						hasCustomSegmentAddress(const char* segName) const;
	bool						hasCustomSectionAlignment(const char* segName, const char* sectName) const;
	uint8_t						customSectionAlignment(const char* segName, const char* sectName) const;
	uint32_t					initialSegProtection(const char*) const;
	bool						readOnlyDataSegment(const char*) const;
	uint32_t					maxSegProtection(const char*) const; 
	bool						saveTempFiles() const { return fSaveTempFiles; }
	const std::vector<const char*>&   rpaths() const { return fRPaths; }
	bool						readOnlyx86Stubs() { return fReadOnlyx86Stubs; }
	const std::vector<DylibOverride>&	dylibOverrides() const { return fDylibOverrides; }
	const char*					generatedMapPath() const { return fMapPath; }
	bool						positionIndependentExecutable() const { return fPositionIndependentExecutable; }
	Options::FileInfo			findIndirectDylib(const std::string& installName, const ld::dylib::File* fromDylib) const;
	bool						deadStripDylibs() const { return fDeadStripDylibs; }
	bool						allowedUndefined(const char* name) const { return ( fAllowedUndefined.find(name) != fAllowedUndefined.end() ); }
	bool						someAllowedUndefines() const { return (fAllowedUndefined.size() != 0); }
	LocalSymbolHandling			localSymbolHandling() { return fLocalSymbolHandling; }
	bool						keepLocalSymbol(const char* symbolName) const;
	bool						allowTextRelocs() const { return fAllowTextRelocs; }
	bool						warnAboutTextRelocs() const { return fWarnTextRelocs; }
	bool						kextsUseStubs() const { return fKextsUseStubs; }
	bool						verbose() const { return fVerbose; }
	bool						makeEncryptable() const { return fEncryptable; }
	bool						needsUnwindInfoSection() const { return fAddCompactUnwindEncoding; }
	const std::vector<const char*>&	llvmOptions() const{ return fLLVMOptions; }
	const std::vector<const char*>&	segmentOrder() const{ return fSegmentOrder; }
	bool						segmentOrderAfterFixedAddressSegment(const char* segName) const;
	const std::vector<const char*>* sectionOrder(const char* segName) const;
	const std::vector<const char*>&	dyldEnvironExtras() const{ return fDyldEnvironExtras; }
	const std::vector<const char*>&	astFilePaths() const{ return fASTFilePaths; }
	bool						makeCompressedDyldInfo() const { return fMakeCompressedDyldInfo; }
	bool						makeThreadedStartsSection() const { return fMakeThreadedStartsSection; }
	bool						hasExportedSymbolOrder();
	bool						exportedSymbolOrder(const char* sym, unsigned int* order) const;
	bool						orderData() { return fOrderData; }
	bool						errorOnOtherArchFiles() const { return fErrorOnOtherArchFiles; }
	bool						markAutoDeadStripDylib() const { return fMarkDeadStrippableDylib; }
	bool						removeEHLabels() const { return fNoEHLabels; }
	bool						useSimplifiedDylibReExports() const { return fUseSimplifiedDylibReExports; }
	bool						objCABIVersion2POverride() const { return fObjCABIVersion2Override; }
	bool						useUpwardDylibs() const { return fCanUseUpwardDylib; }
	bool						fullyLoadArchives() const { return fFullyLoadArchives; }
	bool						loadAllObjcObjectsFromArchives() const { return fLoadAllObjcObjectsFromArchives; }
	bool						autoOrderInitializers() const { return fAutoOrderInitializers; }
	bool						optimizeZeroFill() const { return fOptimizeZeroFill; }
	bool						mergeZeroFill() const { return fMergeZeroFill; }
	bool						logAllFiles() const { return fLogAllFiles; }
	DebugInfoStripping			debugInfoStripping() const { return fDebugInfoStripping; }
	bool						flatNamespace() const { return fFlatNamespace; }
	bool						linkingMainExecutable() const { return fLinkingMainExecutable; }
	bool						implicitlyLinkIndirectPublicDylibs() const { return fImplicitlyLinkPublicDylibs; }
	bool						whyLoad() const { return fWhyLoad; }
	const char*					traceOutputFile() const { return fTraceOutputFile; }
	bool						outputSlidable() const { return fOutputSlidable; }
	bool						haveCmdLineAliases() const { return (fAliases.size() != 0); }
	const std::vector<AliasPair>& cmdLineAliases() const { return fAliases; }
	bool						makeTentativeDefinitionsReal() const { return fMakeTentativeDefinitionsReal; }
	const char*					dyldInstallPath() const { return fDyldInstallPath; }
	bool						warnWeakExports() const { return fWarnWeakExports; }
	bool						noWeakExports() const { return fNoWeakExports; }
	bool						objcGcCompaction() const { return fObjcGcCompaction; }
	bool						objcGc() const { return fObjCGc; }
	bool						objcGcOnly() const { return fObjCGcOnly; }
	bool						canUseThreadLocalVariables() const { return fTLVSupport; }
	bool						forceLegacyVersionLoadCommands() const { return fForceLegacyVersionLoadCommands; }
	bool						shouldUseBuildVersion(ld::Platform plat, uint32_t minOSvers) const;
	bool						addVersionLoadCommand() const { return fVersionLoadCommand && (platforms().count() != 0) && !platforms().contains(ld::Platform::freestanding); }
	bool						addFunctionStarts() const { return fFunctionStartsLoadCommand; }
	bool						addDataInCodeInfo() const { return fDataInCodeInfoLoadCommand; }
	bool						canReExportSymbols() const { return fCanReExportSymbols; }
	const char*					ltoCachePath() const { return fLtoCachePath; }
	bool						ltoPruneIntervalOverwrite() const { return fLtoPruneIntervalOverwrite; }
	int							ltoPruneInterval() const { return fLtoPruneInterval; }
	int							ltoPruneAfter() const { return fLtoPruneAfter; }
	unsigned					ltoMaxCacheSize() const { return fLtoMaxCacheSize; }
	const char*					tempLtoObjectPath() const { return fTempLtoObjectPath; }
	const char*					overridePathlibLTO() const { return fOverridePathlibLTO; }
	const char*					mcpuLTO() const { return fLtoCpu; }
	const char*					kextObjectsPath() const { return fKextObjectsDirPath; }
	int							kextObjectsEnable() const { return fKextObjectsEnable; }
	const char*					toolchainPath() const { return fToolchainPath; }
	bool						objcCategoryMerging() const { return fObjcCategoryMerging; }
	bool						pageAlignDataAtoms() const { return fPageAlignDataAtoms; }
	bool						keepDwarfUnwind() const { return fKeepDwarfUnwind; }
	bool						verboseOptimizationHints() const { return fVerboseOptimizationHints; }
	bool						ignoreOptimizationHints() const { return fIgnoreOptimizationHints; }
	bool						generateDtraceDOF() const { return fGenerateDtraceDOF; }
	bool						allowBranchIslands() const { return fAllowBranchIslands; }
	bool						traceSymbolLayout() const { return fTraceSymbolLayout; }
	bool						markAppExtensionSafe() const { return fMarkAppExtensionSafe; }
	bool						checkDylibsAreAppExtensionSafe() const { return fCheckAppExtensionSafe; }
	bool						forceLoadSwiftLibs() const { return fForceLoadSwiftLibs; }
	bool						bundleBitcode() const { return fBundleBitcode; }
	bool						hideSymbols() const { return fHideSymbols; }
	bool						verifyBitcode() const { return fVerifyBitcode; }
	bool						renameReverseSymbolMap() const { return fReverseMapUUIDRename; }
	bool						deduplicateFunctions() const { return fDeDupe; }
	bool						verboseDeduplicate() const { return fVerboseDeDupe; }
	bool						makeInitializersIntoOffsets() const { return fMakeInitializersIntoOffsets; }
	bool						useLinkedListBinding() const { return fUseLinkedListBinding; }
	bool						makeChainedFixups() const { return fMakeChainedFixups; }
#if SUPPORT_ARCH_arm64e
	bool						useAuthenticatedStubs() const { return fUseAuthenticatedStubs; }
	bool						supportsAuthenticatedPointers() const { return fSupportsAuthenticatedPointers; }
	bool						useAuthDataSegment() const { return fUseAuthDataSegment; }
#endif
	bool						supportPackingText() const { return fSupportPackingText; }
	bool						noLazyBinding() const { return fNoLazyBinding; }
	bool						debugVariant() const { return fDebugVariant; }
	bool						isKernel() const { return fKernel; }
	const char*					reverseSymbolMapPath() const { return fReverseMapPath; }
	std::string					reverseMapTempPath() const { return fReverseMapTempPath; }
	bool						ltoCodegenOnly() const { return fLTOCodegenOnly; }
	bool						ignoreAutoLink() const { return fIgnoreAutoLink; }
	bool						allowDeadDuplicates() const { return fAllowDeadDups; }
	bool						allowWeakImports() const { return fAllowWeakImports; }
	Treatment					initializersTreatment() const { return fInitializersTreatment; }
	BitcodeMode					bitcodeKind() const { return fBitcodeKind; }
	bool						sharedRegionEncodingV2() const { return fSharedRegionEncodingV2; }
	bool						useDataConstSegment() const { return fUseDataConstSegment; }
	bool						useTextExecSegment() const { return fUseTextExecSegment; }
	bool						hasWeakBitTweaks() const;
	bool						forceWeak(const char* symbolName) const;
	bool						forceNotWeak(const char* symbolName) const;
	bool						forceWeakNonWildCard(const char* symbolName) const;
	bool						forceNotWeakNonWildcard(const char* symbolName) const;
	bool						forceCoalesce(const char* symbolName) const;
    Snapshot&                   snapshot() const { return fLinkSnapshot; }
	bool						errorBecauseOfWarnings() const;
	bool						needsThreadLoadCommand() const { return fNeedsThreadLoadCommand; }
	bool						needsEntryPointLoadCommand() const { return fEntryPointLoadCommand; }
	bool						needsSourceVersionLoadCommand() const { return fSourceVersionLoadCommand; }
	bool						canUseAbsoluteSymbols() const { return fAbsoluteSymbols; }
	bool						allowSimulatorToLinkWithMacOSX() const { return fAllowSimulatorToLinkWithMacOSX; }
	bool						isSimulatorSupportDylib() const { return fSimulatorSupportDylib; }
	uint64_t					sourceVersion() const { return fSourceVersion; }
	const char*					demangleSymbol(const char* sym) const;
    bool						pipelineEnabled() const { return fPipelineFifo != NULL; }
    const char*					pipelineFifo() const { return fPipelineFifo; }
	bool						dumpDependencyInfo() const { return (fDependencyInfoPath != NULL); }
	const char*					dependencyInfoPath() const { return fDependencyInfoPath; }
	bool						targetIOSSimulator() const { return platforms().contains(ld::simulatorPlatforms); }
	ld::relocatable::File::LinkerOptionsList&
								linkerOptions() const { return fLinkerOptions; }
	FileInfo					findFramework(const char* frameworkName) const;
	FileInfo					findLibrary(const char* rootName, bool dylibsOnly=false) const;
	bool						armUsesZeroCostExceptions() const;
	const std::vector<SectionRename>& sectionRenames() const { return fSectionRenames; }
	const std::vector<SegmentRename>& segmentRenames() const { return fSegmentRenames; }
	bool						moveRoSymbol(std::string_view symName, const char* filePath, const char*& seg, bool& wildCardMatch) const;
	bool						moveRwSymbol(std::string_view symName, const char* filePath, const char*& seg, bool& wildCardMatch) const;
	bool						moveAXMethodList(const char* className) const;
	const ld::VersionSet& 		platforms() const { return fPlatforms; }
	const std::vector<const char*>&	sdkPaths() const { return fSDKPaths; }
	bool						internalSDK() const { return fInternalSDK; }
	bool						adHocSign() const { return fAdHocSign; }
	bool						platformMismatchesAreWarning() const { return fPlatformMismatchesAreWarning; }
	bool						warnUnusedDylibs() const { return fWarnUnusedDylibs; }
	bool						useObjCRelativeMethodLists() const { return fUseObjCRelativeMethodLists; }
	std::vector<std::string>	writeBitcodeLinkOptions() const;
	std::string					getSDKVersionStr() const;
	uint8_t						maxDefaultCommonAlign() const { return fMaxDefaultCommonAlign; }
	bool						hasDataSymbolMoves() const { return !fSymbolsMovesData.empty(); }
	bool						hasCodeSymbolMoves() const { return !fSymbolsMovesCode.empty(); }
	void						writeToTraceFile(const char* buffer, size_t len) const;
	UnalignedPointerTreatment	unalignedPointerTreatment() const { return fUnalignedPointerTreatment; }
	bool						zeroModTimeInDebugMap() const { return fZeroModTimeInDebugMap; }
	void						writeDependencyInfo() const;
	std::vector<TAPIInterface>	&TAPIFiles() { return fTAPIFiles; }
	void						addTAPIInterface(tapi::LinkerInterfaceFile* interface, const char *path) const;
	const char*					buildContextName() const { return fBuildContextName; }
	bool 						sharedCacheEligiblePath(const char* path) const;
	const char* 				debugMapObjectPrefixPath() const { return fOSOPrefixPath; }
	bool						fromSDK(const char* path) const;

	static uint32_t				parseVersionNumber32(const char*);

private:
	typedef std::unordered_map<const char*, unsigned int, ld::CStringHash, ld::CStringEquals> NameToOrder;
	typedef std::unordered_set<const char*, ld::CStringHash, ld::CStringEquals>  NameSet;
	enum ExportMode { kExportDefault, kExportSome, kDontExportSome };
	enum LibrarySearchMode { kSearchDylibAndArchiveInEachDir, kSearchAllDirsForDylibsThenAllDirsForArchives };
	enum InterposeMode { kInterposeNone, kInterposeAllExternal, kInterposeSome };
	enum SymbolMatchingMode { kAllowWildcards, kDisallowWildcards };

	class SetWithWildcards {
	public:
		void					insert(const char*, SymbolMatchingMode);
		bool					contains(const char*, bool* wildCardMatch=NULL) const;
		bool					containsWithPrefix(const char* symbol, const char* file, bool& wildCardMatch) const;
		bool					containsNonWildcard(const char*) const;
		bool					empty() const			{ return fRegular.empty() && fWildCard.empty(); }
		bool					hasWildCards() const	{ return !fWildCard.empty(); }
		NameSet::const_iterator		regularBegin() const	{ return fRegular.begin(); }
		NameSet::const_iterator		regularEnd() const		{ return fRegular.end(); }
		void					remove(const NameSet&); 
		std::vector<const char*>		data() const;
	private:
		static bool				hasWildCards(const char*);
		bool					wildCardMatch(const char* pattern, const char* candidate) const;
		bool					inCharRange(const char*& range, unsigned char c) const;

		NameSet							fRegular;
		std::vector<const char*>		fWildCard;
	};

	struct SymbolsMove {
		const char*			toSegment;
		SetWithWildcards	symbols;
	};

	struct DependencyEntry {
		uint8_t				opcode;
		std::string			path;
	};

	const char*					checkForNullArgument(const char* argument_name, const char* arg, bool allowDashArg=false) const;
	const char*					checkForNullVersionArgument(const char* argument_name, const char* arg) const;
	void						parse(int argc, const char* argv[]);
	void						checkIllegalOptionCombinations();
	void						buildSearchPaths(int argc, const char* argv[]);
	void						parseArch(const char* architecture);
	void						selectFallbackArch(const char *architecture);
	FileInfo					findFramework(const char* rootName, const char* suffix) const;
	bool						checkForFile(const char* format, const char* dir, const char* rootName,
											 FileInfo& result) const;
	uint64_t					parseVersionNumber64(const char*);
	std::string					getVersionString32(uint32_t ver) const;
	std::string					getVersionString64(uint64_t ver) const;
	bool						parsePackedVersion32(const std::string& versionStr, uint32_t &result);
	void						parseSectionOrderFile(const char* segment, const char* section, const char* path);
	void						parseOrderFile(const char* path, bool cstring);
	void						addSection(const char* segment, const char* section, const char* path);
	void						addSubLibrary(const char* name);
	void						loadFileList(const char* fileOfPaths, ld::File::Ordinal baseOrdinal);
	uint64_t					parseAddress(const char* addr);
	void						loadExportFile(const char* fileOfExports, const char* option, SetWithWildcards& set, SymbolMatchingMode match_mode);
	void						parseAliasFile(const char* fileOfAliases);
	void						parsePreCommandLineEnvironmentSettings();
	void						parsePostCommandLineEnvironmentSettings();
	void						setUndefinedTreatment(const char* treatment);
	void						setWeakReferenceMismatchTreatment(const char* treatment);
	void						addDylibOverride(const char* paths);
	void						addSectionAlignment(const char* segment, const char* section, const char* alignment);
	CommonsMode					parseCommonsTreatment(const char* mode);
	Treatment					parseTreatment(const char* treatment);
	void						reconfigureDefaults();
	void						expandResponseFiles(int& argc, const char**& argv);
	void						checkForClassic(int argc, const char* argv[]);
	void						parseSegAddrTable(const char* segAddrPath, const char* installPath);
	void						addLibrary(const FileInfo& info);
	void						warnObsolete(const char* arg);
	uint32_t					parseProtection(const char* prot);
	void						loadSymbolOrderFile(const char* fileOfExports, NameToOrder& orderMapping);
	void						addSectionRename(const char* srcSegment, const char* srcSection, const char* dstSegment, const char* dstSection);
	void						addSegmentRename(const char* srcSegment, const char* dstSegment);
	void						addSymbolMove(const char* dstSegment, const char* symbolList, std::vector<SymbolsMove>& list, const char* optionName, SymbolMatchingMode);
	void						cannotBeUsedWithBitcode(const char* arg);
	void						loadImplictZipperFile(const char *path,std::vector<const char*>& paths);
	void 						inferArchAndPlatform();


//	ObjectFile::ReaderOptions			fReaderOptions;
	const char*							fOutputFile;
	std::vector<Options::FileInfo>		fInputFiles;
	cpu_type_t							fArchitecture;
	cpu_subtype_t						fSubArchitecture;
	cpu_type_t							fFallbackArchitecture;
	cpu_subtype_t						fFallbackSubArchitecture;
	const char*							fArchitectureName;
	OutputKind							fOutputKind;
	bool								fHasPreferredSubType;
	bool								fArchSupportsThumb2;
	bool								fBindAtLoad;
	bool								fKeepPrivateExterns;
	bool								fIgnoreOtherArchFiles;
	bool								fErrorOnOtherArchFiles;
	bool								fForceSubtypeAll;
	InterposeMode						fInterposeMode;
	bool								fDeadStrip;
	NameSpace							fNameSpace;
	uint32_t							fDylibCompatVersion;
	uint64_t							fDylibCurrentVersion;
	const char*							fDylibInstallName;
	const char*							fFinalName;
	const char*							fEntryName;
	uint64_t							fBaseAddress;
	uint64_t							fMaxAddress;
	uint64_t							fBaseWritableAddress;
	SetWithWildcards					fExportSymbols;
	SetWithWildcards					fDontExportSymbols;
	SetWithWildcards					fInterposeList;
	SetWithWildcards					fForceWeakSymbols;
	SetWithWildcards					fForceNotWeakSymbols;
	SetWithWildcards					fReExportSymbols;
	SetWithWildcards					fForceCoalesceSymbols;
	NameSet								fRemovedExports;
	NameToOrder							fExportSymbolsOrder;
	ExportMode							fExportMode;
	LibrarySearchMode					fLibrarySearchMode;
	UndefinedTreatment					fUndefinedTreatment;
	bool								fMessagesPrefixedWithArchitecture;
	WeakReferenceMismatchTreatment		fWeakReferenceMismatchTreatment;
	std::vector<const char*>			fSubUmbellas;
	std::vector<const char*>			fSubLibraries;
	std::vector<const char*>			fAllowableClients;
	std::vector<const char*>			fRPaths;
	const char*							fClientName;
	const char*							fUmbrellaName;
	const char*							fInitFunctionName;
	const char*							fDotOutputFile;
	const char*							fExecutablePath;
	const char*							fBundleLoader;
	const char*							fDtraceScriptName;
	const char*							fMapPath;
	const char*							fDyldInstallPath;
	const char*							fLtoCachePath;
	bool								fLtoPruneIntervalOverwrite;
	int									fLtoPruneInterval;
	int									fLtoPruneAfter;
	unsigned							fLtoMaxCacheSize;
	const char*							fTempLtoObjectPath;
	const char*							fOverridePathlibLTO;
	const char*							fLtoCpu;
	int     							fKextObjectsEnable;
	const char*							fKextObjectsDirPath;
	const char*							fToolchainPath;
	const char*							fOrderFilePath;
	uint64_t							fZeroPageSize;
	uint64_t							fStackSize;
	uint64_t							fStackAddr;
	uint64_t							fSourceVersion;
	uint32_t							fSDKVersion;
	bool								fExecutableStack;
	bool								fNonExecutableHeap;
	bool								fDisableNonExecutableHeap;
	uint32_t							fMinimumHeaderPad;
	uint64_t							fSegmentAlignment;
	CommonsMode							fCommonsMode;
	enum UUIDMode						fUUIDMode;
	SetWithWildcards					fLocalSymbolsIncluded;
	SetWithWildcards					fLocalSymbolsExcluded;
	LocalSymbolHandling					fLocalSymbolHandling;
	bool								fWarnCommons;
	bool								fVerbose;
	bool								fKeepRelocations;
	bool								fWarnStabs;
	bool								fTraceDylibSearching;
	bool								fPause;
	bool								fStatistics;
	bool								fPrintOptions;
	bool								fSharedRegionEligible;
	bool								fSharedRegionEligibleForceOff;
	bool								fPrintOrderFileStatistics;
	bool								fReadOnlyx86Stubs;
	bool								fPositionIndependentExecutable;
	bool								fPIEOnCommandLine;
	bool								fDisablePositionIndependentExecutable;
	bool								fMaxMinimumHeaderPad;
	bool								fDeadStripDylibs;
	bool								fAllowTextRelocs;
	bool								fWarnTextRelocs;
	bool								fKextsUseStubs;
	bool								fEncryptable;
	bool								fEncryptableForceOn;
	bool								fEncryptableForceOff;
	bool								fOrderData;
	bool								fMarkDeadStrippableDylib;
	bool								fMakeCompressedDyldInfo;
	bool								fMakeCompressedDyldInfoForceOff;
	bool								fMakeThreadedStartsSection;
	bool								fNoEHLabels;
	bool								fAllowCpuSubtypeMismatches;
	bool								fEnforceDylibSubtypesMatch;
	bool								fWarnOnSwiftABIVersionMismatches;
	bool								fUseSimplifiedDylibReExports;
	bool								fObjCABIVersion2Override;
	bool								fObjCABIVersion1Override;
	bool								fCanUseUpwardDylib;
	bool								fFullyLoadArchives;
	bool								fLoadAllObjcObjectsFromArchives;
	bool								fFlatNamespace;
	bool								fLinkingMainExecutable;
	bool								fForFinalLinkedImage;
	bool								fForStatic;
	bool								fForDyld;
	bool								fMakeTentativeDefinitionsReal;
	bool								fWhyLoad;
	bool								fRootSafe;
	bool								fSetuidSafe;
	bool								fImplicitlyLinkPublicDylibs;
	bool								fAddCompactUnwindEncoding;
	bool								fWarnCompactUnwind;
	bool								fRemoveDwarfUnwindIfCompactExists;
	bool								fAutoOrderInitializers;
	bool								fOptimizeZeroFill;
	bool								fMergeZeroFill;
	bool								fLogObjectFiles;
	bool								fLogAllFiles;
	bool								fTraceDylibs;
	bool								fTraceIndirectDylibs;
	bool								fTraceArchives;
	bool								fTraceEmitJSON;
	bool								fOutputSlidable;
	bool								fWarnWeakExports;
	bool								fNoWeakExports;
	bool								fObjcGcCompaction;
	bool								fObjCGc;
	bool								fObjCGcOnly;
	bool								fDemangle;
	bool								fTLVSupport;
	bool								fVersionLoadCommand;
	bool								fVersionLoadCommandForcedOn;
	bool								fVersionLoadCommandForcedOff;
	bool								fForceLegacyVersionLoadCommands;
	bool								fFunctionStartsLoadCommand;
	bool								fFunctionStartsForcedOn;
	bool								fFunctionStartsForcedOff;
	bool								fDataInCodeInfoLoadCommand;
	bool								fDataInCodeInfoLoadCommandForcedOn;
	bool								fDataInCodeInfoLoadCommandForcedOff;
	bool								fCanReExportSymbols;
	bool								fObjcCategoryMerging;
	bool								fPageAlignDataAtoms;
	bool								fNeedsThreadLoadCommand;
	bool								fEntryPointLoadCommand;
	bool								fSourceVersionLoadCommand;
	bool								fSourceVersionLoadCommandForceOn;
	bool								fSourceVersionLoadCommandForceOff;	
	bool								fExportDynamic;
	bool								fAbsoluteSymbols;
	bool								fAllowSimulatorToLinkWithMacOSX;
	bool								fSimulatorSupportDylib;
	bool								fKeepDwarfUnwind;
	bool								fKeepDwarfUnwindForcedOn;
	bool								fKeepDwarfUnwindForcedOff;
	bool								fVerboseOptimizationHints;
	bool								fIgnoreOptimizationHints;
	bool								fGenerateDtraceDOF;
	bool								fAllowBranchIslands;
	bool								fTraceSymbolLayout;
	bool								fMarkAppExtensionSafe;
	bool								fCheckAppExtensionSafe;
	bool								fForceLoadSwiftLibs;
	bool								fSharedRegionEncodingV2;
	bool								fUseDataConstSegment;
	bool								fUseDataConstSegmentForceOn;
	bool								fUseDataConstSegmentForceOff;
	bool								fUseTextExecSegment;
	bool								fBundleBitcode;
	bool								fHideSymbols;
	bool								fVerifyBitcode;
	bool								fReverseMapUUIDRename;
	bool								fDeDupe;
	bool								fVerboseDeDupe;
	bool								fMakeInitializersIntoOffsets;
	bool								fUseLinkedListBinding;
	bool								fMakeChainedFixups;
	bool								fMakeChainedFixupsSection;
#if SUPPORT_ARCH_arm64e
	bool								fUseAuthenticatedStubs 			= false;
	bool								fSupportsAuthenticatedPointers 	= false;
	bool								fUseAuthDataSegment				= false;
	bool								fUseAuthDataSegmentForceOff		= false;
#endif
	bool								fSupportPackingText 			= false;
	bool								fNoLazyBinding;
	bool								fDebugVariant;
	bool								fKernel							= false;
	const char*							fReverseMapPath;
	std::string							fReverseMapTempPath;
	bool								fLTOCodegenOnly;
	bool								fIgnoreAutoLink;
	bool								fAllowDeadDups;
	bool								fAllowWeakImports;
	Treatment							fInitializersTreatment;
	bool								fZeroModTimeInDebugMap;
	BitcodeMode							fBitcodeKind;
	DebugInfoStripping					fDebugInfoStripping;
	const char*							fTraceOutputFile;
	ld::VersionSet						fPlatforms;
	bool								fPlatfromVersionCmdFound;
	bool								fInternalSDK;
	bool								fWarnUnusedDylibs;
	bool								fWarnUnusedDylibsForceOn;
	bool								fWarnUnusedDylibsForceOff;
	bool								fAdHocSign;
	bool								fPlatformMismatchesAreWarning;
	bool								fForceObjCRelativeMethodListsOn;
	bool								fForceObjCRelativeMethodListsOff;
	bool								fUseObjCRelativeMethodLists;
	std::vector<AliasPair>				fAliases;
	std::vector<const char*>			fInitialUndefines;
	NameSet								fAllowedUndefined;
	SetWithWildcards					fWhyLive;
	std::vector<ExtraSection>			fExtraSections;
	std::vector<SectionAlignment>		fSectionAlignments;
	std::vector<OrderedSymbol>			fOrderedSymbols;
	std::vector<SegmentStart>			fCustomSegmentAddresses;
	std::vector<SegmentSize>			fCustomSegmentSizes;
	std::vector<SegmentProtect>			fCustomSegmentProtections;
	std::vector<DylibOverride>			fDylibOverrides; 
	std::vector<const char*>			fLLVMOptions;
	std::vector<const char*>			fLibrarySearchPaths;
	std::vector<const char*>			fFrameworkSearchPaths;
	std::vector<const char*>			fSDKPaths;
	std::vector<const char*>			fDyldEnvironExtras;
	std::vector<const char*>			fSegmentOrder;
	std::vector<const char*>			fASTFilePaths;
	std::vector<SectionOrderList>		fSectionOrder;
	std::vector< std::vector<const char*> > fLinkerOptions;
	std::vector<SectionRename>			fSectionRenames;
	std::vector<SegmentRename>			fSegmentRenames;
	std::vector<SymbolsMove>			fSymbolsMovesData;
	std::vector<SymbolsMove>			fSymbolsMovesCode;
	std::vector<SymbolsMove>			fSymbolsMovesAXMethodLists;
	bool								fSaveTempFiles;
    mutable Snapshot					fLinkSnapshot;
    bool								fSnapshotRequested;
    const char*							fPipelineFifo;
	const char*							fDependencyInfoPath;
	const char*							fBuildContextName;
	mutable int							fTraceFileDescriptor;
	uint8_t								fMaxDefaultCommonAlign;
	UnalignedPointerTreatment			fUnalignedPointerTreatment;
	mutable std::vector<DependencyEntry> fDependencies;
	mutable std::vector<Options::TAPIInterface> fTAPIFiles;
	bool								fPreferTAPIFile;
	const char*							fOSOPrefixPath;
};



#endif // __OPTIONS__
