#ifndef __pbxproj_PBX_Project_h
#define __pbxproj_PBX_Project_h

#include <pbxproj/PBX/Object.h>
#include <pbxproj/PBX/Group.h>
#include <pbxproj/PBX/Target.h>
#include <pbxproj/XC/ConfigurationList.h>

namespace libutil { class Filesystem; }

namespace pbxproj { namespace PBX {

class Project : public Object {
public:
    class ProjectReference {
    private:
        Group::shared_ptr         _productGroup;
        FileReference::shared_ptr _projectReference;

    public:
        ProjectReference();

    public:
        inline Group::shared_ptr const &productGroup() const
        { return _productGroup; }
        inline FileReference::shared_ptr const &projectReference() const
        { return _projectReference; }

    protected:
        friend class pbxproj::PBX::Project;
        bool parse(Context &context, plist::Dictionary const *dict);
    };

public:
    typedef std::shared_ptr <Project> shared_ptr;
    typedef std::vector <shared_ptr> vector;

private:
    std::string                        _projectFile;
    std::string                        _dataFile;
    std::string                        _basePath;
    std::string                        _name;
    std::unordered_map<std::string, Object::shared_ptr> _blueprints;

private:
    XC::ConfigurationList::shared_ptr  _buildConfigurationList;
    std::string                        _compatibilityVersion;
    std::string                        _developmentRegion;
    bool                               _hasScannedForEncodings;
    std::vector<std::string>             _knownRegions;
    Group::shared_ptr                  _mainGroup;
    Group::shared_ptr                  _productRefGroup;
    std::string                        _projectDirPath;
    std::string                        _projectRoot;
    std::vector<ProjectReference>      _projectReferences;
    Target::vector                     _targets;
    FileReference::vector              _fileReferences;

public:
    Project();

public:
    static shared_ptr Open(libutil::Filesystem const *filesystem, std::string const &path);

public:
    inline XC::ConfigurationList::shared_ptr const &buildConfigurationList() const
    { return _buildConfigurationList; }

public:
    inline std::string const &compatibilityVersion() const
    { return _compatibilityVersion; }

public:
    inline std::string const &developmentRegion() const
    { return _developmentRegion; }

public:
    inline bool hasScannedForEncodings() const
    { return _hasScannedForEncodings; }

public:
    inline std::vector<std::string> const &knownRegions() const
    { return _knownRegions; }

public:
    inline Group::shared_ptr const &mainGroup() const
    { return _mainGroup; }

public:
    inline Group::shared_ptr const &productRefGroup() const
    { return _productRefGroup; }

public:
    inline std::string const &projectDirPath() const
    { return _projectDirPath; }
    inline std::string const &projectRoot() const
    { return _projectRoot; }

public:
    inline std::vector<ProjectReference> const &projectReferences() const
    { return _projectReferences; }

public:
    inline Target::vector const &targets() const
    { return _targets; }

public:
    inline std::string const &name() const
    { return _name; }
    inline std::string const &dataFile() const
    { return _dataFile; }
    inline std::string const &projectFile() const
    { return _projectFile; }
    inline std::string const &basePath() const
    { return _basePath; }

public:
    std::string sourceRoot() const;

protected:
    friend class pbxproj::Context;
    inline void cacheObject(Object::shared_ptr const &object)
    { _blueprints[object->blueprintIdentifier()] = object; }

public:
    inline FileReference::vector const &fileReferences() const
    { return _fileReferences; }

public:
    inline Object::shared_ptr resolveBuildableReference(std::string const &blueprintIdentifier) const
    {
        if (blueprintIdentifier.empty())
            return Object::shared_ptr();

        auto I = _blueprints.find(blueprintIdentifier);
        if (I == _blueprints.end())
            return Object::shared_ptr();
        else
            return I->second;
    }

public:
    pbxsetting::Level settings(void) const;

protected:
    bool parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check) override;

public:
    static inline char const *Isa()
    { return ISA::PBXProject; }
};

} }

#endif  // !__pbxproj_PBX_Project_h
