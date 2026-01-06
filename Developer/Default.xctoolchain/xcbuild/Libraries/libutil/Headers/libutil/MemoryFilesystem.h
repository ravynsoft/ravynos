/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __libutil_MemoryFilesystem_h
#define __libutil_MemoryFilesystem_h

#include <libutil/Filesystem.h>

namespace libutil {

class MemoryFilesystem : public Filesystem {
public:
    class Entry {
    public:
        std::string          _name;

    public:
        Type                 _type;
        std::vector<uint8_t> _contents;
        std::vector<Entry>   _children;

    private:
        Entry(std::string const &name, Type type);

    public:
        std::string &name()
        { return _name; }
        std::string const &name() const
        { return _name; }

    public:
        Type type() const
        { return _type; }
        std::vector<uint8_t> &contents()
        { return _contents; }
        std::vector<uint8_t> const &contents() const
        { return _contents; }
        std::vector<Entry> &children()
        { return _children; }
        std::vector<Entry> const &children() const
        { return _children; }

    public:
        MemoryFilesystem::Entry *child(std::string const &name);
        MemoryFilesystem::Entry const *child(std::string const &name) const;

    public:
        static Entry File(std::string const &name, std::vector<uint8_t> const &contents);
        static Entry Directory(std::string const &name, std::vector<Entry> const &children);
    };

private:
    Entry _root;

public:
    MemoryFilesystem(std::vector<Entry> const &entries);

public:
    Entry &root()
    { return _root; }
    Entry const &root() const
    { return _root; }

public:
    std::string path(std::string const &path) const;

public:
    virtual bool exists(std::string const &path) const;
    virtual ext::optional<Type> type(std::string const &path) const;

public:
    virtual bool isReadable(std::string const &path) const;
    virtual bool isWritable(std::string const &path) const;
    virtual bool isExecutable(std::string const &path) const;

public:
    virtual ext::optional<Permissions> readFilePermissions(std::string const &path) const;
    virtual bool writeFilePermissions(std::string const &path, Permissions::Operation operation, Permissions permissions);
    virtual bool createFile(std::string const &path);
    virtual bool read(std::vector<uint8_t> *contents, std::string const &path, size_t offset = 0, ext::optional<size_t> length = ext::nullopt) const;
    virtual bool write(std::vector<uint8_t> const &contents, std::string const &path);
    virtual bool copyFile(std::string const &from, std::string const &to);
    virtual bool removeFile(std::string const &path);

public:
    virtual ext::optional<Permissions> readSymbolicLinkPermissions(std::string const &path) const;
    virtual bool writeSymbolicLinkPermissions(std::string const &path, Permissions::Operation operation, Permissions permissions);
    virtual ext::optional<std::string> readSymbolicLink(std::string const &path, bool *directory = nullptr) const;
    virtual bool writeSymbolicLink(std::string const &target, std::string const &path, bool directory);
    virtual bool copySymbolicLink(std::string const &from, std::string const &to);
    virtual bool removeSymbolicLink(std::string const &path);

public:
    virtual ext::optional<Permissions> readDirectoryPermissions(std::string const &path) const;
    virtual bool writeDirectoryPermissions(std::string const &path, Permissions::Operation operation, Permissions permissions, bool recursive);
    virtual bool createDirectory(std::string const &path, bool recursive);
    virtual bool readDirectory(std::string const &path, bool recursive, std::function<void(std::string const &)> const &cb) const;
    virtual bool copyDirectory(std::string const &from, std::string const &to, bool recursive);
    virtual bool removeDirectory(std::string const &path, bool recursive);

public:
    virtual std::string resolvePath(std::string const &path) const;
};

}

#endif  // !__libutil_MemoryFilesystem_h
