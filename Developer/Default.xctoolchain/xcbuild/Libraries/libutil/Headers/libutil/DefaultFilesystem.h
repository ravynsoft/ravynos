/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __libutil_DefaultFilesystem_h
#define __libutil_DefaultFilesystem_h

#include <libutil/Filesystem.h>

namespace libutil {

class DefaultFilesystem : public Filesystem {
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

#endif  // !__libutil_DefaultFilesystem_h
