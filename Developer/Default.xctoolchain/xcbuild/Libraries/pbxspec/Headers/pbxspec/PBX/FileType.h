/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxspec_PBX_FileType_h
#define __pbxspec_PBX_FileType_h

#include <pbxspec/PBX/Specification.h>
#include <pbxspec/PBX/BuildPhaseInjection.h>

#include <memory>
#include <string>
#include <vector>
#include <ext/optional>

namespace plist { class Array; }
namespace plist { class Dictionary; }

namespace pbxspec { namespace PBX {

class FileType : public Specification {
public:
    typedef std::shared_ptr <FileType> shared_ptr;
    typedef std::vector <shared_ptr> vector;

public:
    class ComponentPart {
    protected:
        std::string                             _identifier;
        ext::optional<std::string>              _type;
        ext::optional<std::string>              _location;
        ext::optional<std::vector<std::string>> _identifiers;
        ext::optional<std::string>              _reference;

    protected:
        friend class FileType;
        ComponentPart();

    public:
        inline std::string const &identifier() const
        { return _identifier; }

    public:
        inline ext::optional<std::string> const &type() const
        { return _type; }

    public:
        inline ext::optional<std::string> const &location() const
        { return _location; }

    public:
        inline ext::optional<std::vector<std::string>> const &identifiers() const
        { return _identifiers; }

    public:
        inline ext::optional<std::string> const &reference() const
        { return _reference; }

    protected:
        bool parse(std::string const &identifier, plist::Array const *array);
    };

protected:
    friend class ComponentPart;

protected:
    ext::optional<std::string>              _uti;
    ext::optional<std::string>              _language;
    ext::optional<std::string>              _computerLanguage;
    ext::optional<std::string>              _gccDialectName;
    ext::optional<std::string>              _plistStructureDefinition;
    ext::optional<std::string>              _permissions;
    ext::optional<std::vector<std::string>> _extensions;
    ext::optional<std::vector<std::string>> _mimeTypes;
    ext::optional<std::vector<std::string>> _typeCodes;
    ext::optional<std::vector<std::string>> _filenamePatterns;
    ext::optional<std::vector<std::vector<uint8_t>>> _magicWords;
    ext::optional<std::vector<std::string>> _extraPropertyNames;
    ext::optional<std::vector<std::string>> _prefix;
    ext::optional<std::vector<ComponentPart>>       _componentParts;
    ext::optional<std::vector<BuildPhaseInjection>> _buildPhaseInjectionsWhenEmbedding;
    ext::optional<bool>                     _isTextFile;
    ext::optional<bool>                     _isBuildPropertiesFile;
    ext::optional<bool>                     _isSourceCode;
    ext::optional<bool>                     _isSwiftSourceCode;
    ext::optional<bool>                     _isPreprocessed;
    ext::optional<bool>                     _isTransparent;
    ext::optional<bool>                     _isDocumentation;
    ext::optional<bool>                     _isEmbeddable;
    ext::optional<bool>                     _isExecutable;
    ext::optional<bool>                     _isExecutableWithGUI;
    ext::optional<bool>                     _isApplication;
    ext::optional<bool>                     _isBundle;
    ext::optional<bool>                     _isLibrary;
    ext::optional<bool>                     _isDynamicLibrary;
    ext::optional<bool>                     _isStaticLibrary;
    ext::optional<bool>                     _isFolder;
    ext::optional<bool>                     _isWrappedFolder;
    ext::optional<bool>                     _isFrameworkWrapper;
    ext::optional<bool>                     _isStaticFrameworkWrapper;
    ext::optional<bool>                     _isProjectWrapper;
    ext::optional<bool>                     _isTargetWrapper;
    ext::optional<bool>                     _isScannedForIncludes;
    ext::optional<bool>                     _includeInIndex;
    ext::optional<bool>                     _canSetIncludeInIndex;
    ext::optional<bool>                     _requiresHardTabs;
    ext::optional<bool>                     _containsNativeCode;
    ext::optional<bool>                     _appliesToBuildRules;
    ext::optional<bool>                     _changesCauseDependencyGraphInvalidation;
    ext::optional<std::string>              _fallbackAutoroutingBuildPhase;
    ext::optional<bool>                     _codeSignOnCopy;
    ext::optional<bool>                     _removeHeadersOnCopy;
    ext::optional<bool>                     _validateOnCopy;

protected:
    FileType();

public:
    virtual ~FileType();

public:
    inline SpecificationType type() const override
    { return FileType::Type(); }

public:
    inline FileType::shared_ptr base() const
    { return std::static_pointer_cast<FileType>(Specification::base()); }

public:
    inline ext::optional<std::string> const &UTI() const
    { return _uti; }

public:
    inline ext::optional<std::vector<std::string>> const &extensions() const
    { return _extensions; }
    inline ext::optional<std::vector<std::string>> const &MIMETypes() const
    { return _mimeTypes; }
    inline ext::optional<std::vector<std::string>> const &typeCodes() const
    { return _typeCodes; }
    inline ext::optional<std::vector<std::string>> const &filenamePatterns() const
    { return _filenamePatterns; }
    inline ext::optional<std::vector<std::vector<uint8_t>>> const &magicWords() const
    { return _magicWords; }

public:
    inline ext::optional<std::string> const &language() const
    { return _language; }
    inline ext::optional<std::string> const &computerLanguage() const
    { return _computerLanguage; }

public:
    inline ext::optional<std::string> const &GCCDialectName() const
    { return _gccDialectName; }

public:
    inline ext::optional<std::string> const &plistStructureDefinition() const
    { return _plistStructureDefinition; }

public:
    inline bool isTextFile() const
    { return _isTextFile.value_or(false); }
    inline ext::optional<bool> isTextFileOptional() const
    { return _isTextFile; }
    inline bool isTransparent() const
    { return _isTransparent.value_or(false); }
    inline ext::optional<bool> isTransparentOptional() const
    { return _isTransparent; }

public:
    inline bool isBuildPropertiesFile() const
    { return _isBuildPropertiesFile.value_or(false); }
    inline ext::optional<bool> isBuildPropertiesFileOptional() const
    { return _isBuildPropertiesFile; }

public:
    inline bool isEmbeddable() const
    { return _isEmbeddable.value_or(false); }
    inline ext::optional<bool> isEmbeddableOptional() const
    { return _isEmbeddable; }

public:
    inline bool isExecutable() const
    { return _isExecutable.value_or(false); }
    inline ext::optional<bool> isExecutableOptional() const
    { return _isExecutable; }
    inline bool isExecutableWithGUI() const
    { return _isExecutableWithGUI.value_or(false); }
    inline ext::optional<bool> isExecutableWithGUIOptional() const
    { return _isExecutableWithGUI; }
    inline bool containsNativeCode() const
    { return _containsNativeCode.value_or(false); }
    inline ext::optional<bool> containsNativeCodeOptional() const
    { return _containsNativeCode; }

public:
    inline bool isApplication() const
    { return _isApplication.value_or(false); }
    inline ext::optional<bool> isApplicationOptional() const
    { return _isApplication; }

public:
    inline bool isBundle() const
    { return _isBundle.value_or(false); }
    inline ext::optional<bool> isBundleOptional() const
    { return _isBundle; }

public:
    inline bool isLibrary() const
    { return _isLibrary.value_or(false); }
    inline ext::optional<bool> isLibraryOptional() const
    { return _isLibrary; }
    inline bool isDynamicLibrary() const
    { return _isDynamicLibrary.value_or(false); }
    inline ext::optional<bool> isDynamicLibraryOptional() const
    { return _isDynamicLibrary; }
    inline bool isStaticLibrary() const
    { return _isStaticLibrary.value_or(false); }
    inline ext::optional<bool> isStaticLibraryOptional() const
    { return _isStaticLibrary; }

public:
    inline bool isFolder() const
    { return _isFolder.value_or(false); }
    inline ext::optional<bool> isFolderOptional() const
    { return _isFolder; }
    inline bool isWrappedFolder() const
    { return _isWrappedFolder.value_or(false); }
    inline ext::optional<bool> isWrappedFolderOptional() const
    { return _isWrappedFolder; }

public:
    inline bool isFrameworkWrapper() const
    { return _isFrameworkWrapper.value_or(false); }
    inline ext::optional<bool> isFrameworkWrapperOptional() const
    { return _isFrameworkWrapper; }
    inline bool isStaticFrameworkWrapper() const
    { return _isStaticFrameworkWrapper.value_or(false); }
    inline ext::optional<bool> isStaticFrameworkWrapperOptional() const
    { return _isStaticFrameworkWrapper; }

public:
    inline bool isProjectWrapper() const
    { return _isProjectWrapper.value_or(false); }
    inline ext::optional<bool> isProjectWrapperOptional() const
    { return _isProjectWrapper; }

public:
    inline bool isTargetWrapper() const
    { return _isTargetWrapper.value_or(false); }
    inline ext::optional<bool> isTargetWrapperOptional() const
    { return _isTargetWrapper; }

public:
    inline ext::optional<std::vector<std::string>> const &extraPropertyNames() const
    { return _extraPropertyNames; }
    inline ext::optional<std::vector<ComponentPart>> const &componentParts() const
    { return _componentParts; }

public:
    inline ext::optional<std::vector<BuildPhaseInjection>> const &buildPhaseInjectionsWhenEmbedding() const
    { return _buildPhaseInjectionsWhenEmbedding; }

public:
    inline bool isSourceCode() const
    { return _isSourceCode.value_or(false); }
    inline ext::optional<bool> isSourceCodeOptional() const
    { return _isSourceCode; }
    inline bool isSwiftSourceCode() const
    { return _isSwiftSourceCode.value_or(false); }
    inline ext::optional<bool> isSwiftSourceCodeOptional() const
    { return _isSwiftSourceCode; }
    inline bool isPreprocessed() const
    { return _isPreprocessed.value_or(false); }
    inline ext::optional<bool> isPreprocessedOptional() const
    { return _isPreprocessed; }
    inline bool isScannedForIncludes() const
    { return _isScannedForIncludes.value_or(false); }
    inline ext::optional<bool> isScannedForIncludesOptional() const
    { return _isScannedForIncludes; }
    inline bool includeInIndex() const
    { return _includeInIndex.value_or(false); }
    inline ext::optional<bool> includeInIndexOptional() const
    { return _includeInIndex; }
    inline bool canSetIncludeInIndex() const
    { return _canSetIncludeInIndex.value_or(false); }
    inline ext::optional<bool> canSetIncludeInIndexOptional() const
    { return _canSetIncludeInIndex; }

public:
    inline bool isDocumentation() const
    { return _isDocumentation.value_or(false); }
    inline ext::optional<bool> isDocumentationOptional() const
    { return _isDocumentation; }

public:
    inline bool requiresHardTabs() const
    { return _requiresHardTabs.value_or(false); }
    inline ext::optional<bool> requiresHardTabsOptional() const
    { return _requiresHardTabs; }

public:
    inline ext::optional<std::string> const &permissions() const
    { return _permissions; }

public:
    inline ext::optional<std::vector<std::string>> const &prefix() const
    { return _prefix; }

public:
    inline bool appliesToBuildRules() const
    { return _appliesToBuildRules.value_or(false); }
    inline ext::optional<bool> appliesToBuildRulesOptional() const
    { return _appliesToBuildRules; }
    inline bool changesCauseDependencyGraphInvalidation() const
    { return _changesCauseDependencyGraphInvalidation.value_or(false); }
    inline ext::optional<bool> changesCauseDependencyGraphInvalidationOptional() const
    { return _changesCauseDependencyGraphInvalidation; }
    inline ext::optional<std::string> const &fallbackAutoroutingBuildPhase() const
    { return _fallbackAutoroutingBuildPhase; }

public:
    inline bool codeSignOnCopy() const
    { return _codeSignOnCopy.value_or(false); }
    inline ext::optional<bool> codeSignOnCopyOptional() const
    { return _codeSignOnCopy; }
    inline bool removeHeadersOnCopy() const
    { return _removeHeadersOnCopy.value_or(false); }
    inline ext::optional<bool> removeHeadersOnCopyOptional() const
    { return _removeHeadersOnCopy; }
    inline bool validateOnCopy() const
    { return _validateOnCopy.value_or(false); }
    inline ext::optional<bool> validateOnCopyOptional() const
    { return _validateOnCopy; }

protected:
    friend class Specification;
    bool parse(Context *context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check) override;

protected:
    bool inherit(Specification::shared_ptr const &base) override;
    virtual bool inherit(FileType::shared_ptr const &base);

protected:
    static FileType::shared_ptr Parse(Context *context, plist::Dictionary const *dict);

public:
    static inline SpecificationType Type()
    { return SpecificationType::FileType; }
};

} }

#endif  // !__pbxspec_PBX_FileType_h
