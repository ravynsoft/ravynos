/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <libutil/MemoryFilesystem.h>
#include <libutil/FSUtil.h>

#include <algorithm>

#include <cassert>

using libutil::MemoryFilesystem;
using libutil::Filesystem;
using libutil::Permissions;
using libutil::FSUtil;

MemoryFilesystem::Entry::
Entry(std::string const &name, Type type) :
    _name(name),
    _type(type)
{
}

MemoryFilesystem::Entry *MemoryFilesystem::Entry::
child(std::string const &name)
{
    assert(_type == Type::Directory);

    for (MemoryFilesystem::Entry &entry : _children) {
        if (entry.name() == name) {
            return &entry;
        }
    }

    return nullptr;
}

MemoryFilesystem::Entry const *MemoryFilesystem::Entry::
child(std::string const &name) const
{
    assert(_type == Type::Directory);

    for (MemoryFilesystem::Entry const &entry : _children) {
        if (entry.name() == name) {
            return &entry;
        }
    }

    return nullptr;
}

MemoryFilesystem::Entry MemoryFilesystem::Entry::
File(std::string const &name, std::vector<uint8_t> const &contents)
{
    MemoryFilesystem::Entry entry = MemoryFilesystem::Entry(name, Type::File);
    entry.contents() = contents;
    return entry;
}

MemoryFilesystem::Entry MemoryFilesystem::Entry::
Directory(std::string const &name, std::vector<Entry> const &children)
{
    MemoryFilesystem::Entry entry = MemoryFilesystem::Entry(name, Type::Directory);
    entry.children() = children;
    return entry;
}

MemoryFilesystem::
MemoryFilesystem(std::vector<MemoryFilesystem::Entry> const &entries) :
#if _WIN32
    _root(MemoryFilesystem::Entry::Directory("C:", entries))
#else
    _root(MemoryFilesystem::Entry::Directory("", entries))
#endif
{
}

std::string MemoryFilesystem::
path(std::string const &path) const
{
#if _WIN32
    return "C:\\" + path;
#else
    return "/" + path;
#endif
}

template<typename T, typename U, typename V>
static bool
WalkPath(
    U filesystem,
    std::string const &path,
    bool all,
    V const &cb)
{
    /* All paths are expected to be absolute. */
    if (!FSUtil::IsAbsolutePath(path)) {
        return false;
    }

    std::string normalized = FSUtil::NormalizePath(path);
    if (normalized.empty()) {
        return false;
    }

    T *current = &filesystem->root();
    assert(current->type() == Filesystem::Type::Directory);

    std::string::size_type start = 0;
#if _WIN32
    std::string::size_type end = normalized.find('\\', start);
#else
    std::string::size_type end = normalized.find('/', start);
#endif

    std::string name = normalized.substr(start, end - start);

    // TODO: this assumes a root matching a path component
    if (name != current->name()) {
        return false;
    }

    start = end + 1;
#if _WIN32
    end = normalized.find('\\', start);
#else
    end = normalized.find('/', start);
#endif

    do {
        bool final = (end == std::string::npos);

        T *next = current;
        std::string name = normalized.substr(start, end - start);
        if (!name.empty()) {
            /* Get the next path component. */
            next = current->child(name);
        }

        if (all || final) {
            /* Call back with information. */
            next = cb(current, name, next);
        }

        if (final) {
            /* Nothing more. */
            return (next != nullptr);
        }

        /* Path component not found or not directory. */
        if (next == nullptr || next->type() != Filesystem::Type::Directory) {
            return false;
        }

        current = next;

        /* Move to next path component. */
        start = end + 1;
#if _WIN32
        end = normalized.find('\\', start);
#else
        end = normalized.find('/', start);
#endif
    } while (true);
}

bool MemoryFilesystem::
exists(std::string const &path) const
{
    return WalkPath<MemoryFilesystem::Entry const>(this, path, false, [](MemoryFilesystem::Entry const *parent, std::string const &name, MemoryFilesystem::Entry const *entry) {
        /* Nothing to do. */
        return entry;
    });
}

ext::optional<Filesystem::Type> MemoryFilesystem::
type(std::string const &path) const
{
    ext::optional<Type> type;

    if (!WalkPath<MemoryFilesystem::Entry const>(this, path, false, [&type](MemoryFilesystem::Entry const *parent, std::string const &name, MemoryFilesystem::Entry const *entry) -> MemoryFilesystem::Entry const * {
        if (entry != nullptr) {
            type = entry->type();
        }

        return entry;
    })) {
        return ext::nullopt;
    }

    return type;
}

bool MemoryFilesystem::
isReadable(std::string const &path) const
{
    return this->exists(path);
}

bool MemoryFilesystem::
isWritable(std::string const &path) const
{
    return this->exists(path);
}

bool MemoryFilesystem::
isExecutable(std::string const &path) const
{
    return this->exists(path);
}

static Permissions
AllPermissions()
{
    return Permissions(
        { Permissions::Permission::Read, Permissions::Permission::Write, Permissions::Permission::Execute },
        { Permissions::Permission::Read, Permissions::Permission::Write, Permissions::Permission::Execute },
        { Permissions::Permission::Read, Permissions::Permission::Write, Permissions::Permission::Execute });
}

ext::optional<Permissions> MemoryFilesystem::
readFilePermissions(std::string const &path) const
{
    if (this->type(path) != Type::File) {
        return ext::nullopt;
    }

    return AllPermissions();
}

ext::optional<Permissions> MemoryFilesystem::
readSymbolicLinkPermissions(std::string const &path) const
{
    return ext::nullopt;
}

ext::optional<Permissions> MemoryFilesystem::
readDirectoryPermissions(std::string const &path) const
{
    if (this->type(path) != Type::Directory) {
        return ext::nullopt;
    }

    return AllPermissions();
}

bool MemoryFilesystem::
writeFilePermissions(std::string const &path, Permissions::Operation operation, Permissions permissions)
{
    return this->type(path) == Type::File;
}

bool MemoryFilesystem::
writeSymbolicLinkPermissions(std::string const &path, Permissions::Operation operation, Permissions permissions)
{
    return this->type(path) == Type::SymbolicLink;
}

bool MemoryFilesystem::
writeDirectoryPermissions(std::string const &path, Permissions::Operation operation, Permissions permissions, bool recursive)
{
    return this->type(path) == Type::Directory;
}

bool MemoryFilesystem::
createFile(std::string const &path)
{
    return WalkPath<MemoryFilesystem::Entry>(this, path, false, [](MemoryFilesystem::Entry *parent, std::string const &name, MemoryFilesystem::Entry *entry) -> MemoryFilesystem::Entry * {
        if (entry != nullptr) {
            if (entry->type() == Type::File) {
                /* Exists as a file. */
                return entry;
            } else {
                /* Exists already, but not as a file. */
                return nullptr;
            }
        } else {
            /* Add empty file. */
            MemoryFilesystem::Entry file = MemoryFilesystem::Entry::File(name, std::vector<uint8_t>());
            std::vector<MemoryFilesystem::Entry> *children = &parent->children();
            children->emplace_back(std::move(file));
            return &children->back();
        }
    });
}

bool MemoryFilesystem::
read(std::vector<uint8_t> *contents, std::string const &path, size_t offset, ext::optional<size_t> length) const
{
    return WalkPath<MemoryFilesystem::Entry const>(this, path, false, [&](MemoryFilesystem::Entry const *parent, std::string const &name, MemoryFilesystem::Entry const *entry) -> MemoryFilesystem::Entry const * {
        if (entry == nullptr || entry->type() != Type::File) {
            return nullptr;
        }

        if (offset == 0 && !length) {
            *contents = entry->contents();
        } else {
            std::vector<uint8_t> const &from = entry->contents();
            size_t end = (length ? offset + *length : from.size() - offset);
            *contents = std::vector<uint8_t>(from.begin() + offset, from.begin() + end);
        }

        return entry;
    });
}

bool MemoryFilesystem::
write(std::vector<uint8_t> const &contents, std::string const &path)
{
    return WalkPath<MemoryFilesystem::Entry>(this, path, false, [&](MemoryFilesystem::Entry *parent, std::string const &name, MemoryFilesystem::Entry *entry) -> MemoryFilesystem::Entry * {
        if (entry != nullptr) {
            if (entry->type() == Type::File) {
                /* Exists as a file, replace contents. */
                entry->contents() = contents;
                return entry;
            } else {
                /* Exists already, but not as a file. */
                return nullptr;
            }
        } else {
            /* Add file. */
            MemoryFilesystem::Entry file = MemoryFilesystem::Entry::File(name, contents);
            std::vector<MemoryFilesystem::Entry> *children = &parent->children();
            children->emplace_back(std::move(file));
            return &children->back();
        }
    });
}

bool MemoryFilesystem::
copyFile(std::string const &from, std::string const &to)
{
    return Filesystem::copyFile(from, to);
}

bool MemoryFilesystem::
removeFile(std::string const &path)
{
    return WalkPath<MemoryFilesystem::Entry>(this, path, false, [](MemoryFilesystem::Entry *parent, std::string const &name, MemoryFilesystem::Entry *entry) -> MemoryFilesystem::Entry * {
        if (entry != nullptr) {
            if (entry->type() == Type::File) {
                /* Found, remove it. */
                std::vector<MemoryFilesystem::Entry> *children = &parent->children();
                children->erase(std::remove_if(children->begin(), children->end(), [&](MemoryFilesystem::Entry const &entry) {
                    return (entry.name() == name);
                }), children->end());
                return parent;
            } else {
                /* Can't remove directories. */
                return nullptr;
            }
        } else {
            /* Did not exist. */
            return nullptr;
        }
    });
}

ext::optional<std::string> MemoryFilesystem::
readSymbolicLink(std::string const &path, bool *directory) const
{
    return ext::nullopt;
}

bool MemoryFilesystem::
writeSymbolicLink(std::string const &target, std::string const &path, bool directory)
{
    return false;
}

bool MemoryFilesystem::
copySymbolicLink(std::string const &from, std::string const &to)
{
    return false;
}

bool MemoryFilesystem::
removeSymbolicLink(std::string const &path)
{
    return false;
}

bool MemoryFilesystem::
createDirectory(std::string const &path, bool recursive)
{
    return WalkPath<MemoryFilesystem::Entry>(this, path, recursive, [](MemoryFilesystem::Entry *parent, std::string const &name, MemoryFilesystem::Entry *entry) -> MemoryFilesystem::Entry * {
        if (entry != nullptr) {
            if (entry->type() == Type::Directory) {
                /* Intermediate directory already exists. */
                return entry;
            } else {
                /* Exists already, but not as a directory. */
                return nullptr;
            }
        } else {
            /* Add intermediate directory. */
            MemoryFilesystem::Entry directory = MemoryFilesystem::Entry::Directory(name, { });
            std::vector<MemoryFilesystem::Entry> *children = &parent->children();
            children->emplace_back(std::move(directory));
            return &children->back();
        }
    });
}

bool MemoryFilesystem::
copyDirectory(std::string const &from, std::string const &to, bool recursive)
{
    return Filesystem::copyDirectory(from, to, recursive);
}

bool MemoryFilesystem::
readDirectory(std::string const &path, bool recursive, std::function<void(std::string const &)> const &cb) const
{
    std::function<void(ext::optional<std::string> const &, MemoryFilesystem::Entry const *)> process =
        [&recursive, &cb, &process](ext::optional<std::string> const &subpath, MemoryFilesystem::Entry const *entry) {
        /* Report children. */
        for (MemoryFilesystem::Entry const &child : entry->children()) {
            std::string path = (subpath ? *subpath + "/" + child.name() : child.name());

            /* Process subdirectories first. */
            if (recursive) {
                process(path, &child);
            }

            cb(path);
        }
    };

    return WalkPath<MemoryFilesystem::Entry const>(this, path, false, [&process](MemoryFilesystem::Entry const *parent, std::string const &name, MemoryFilesystem::Entry const *entry) -> MemoryFilesystem::Entry const * {
        if (entry != nullptr && entry->type() == Type::Directory) {
            /* Found directory, process. */
            process(ext::nullopt, entry);
            return entry;
        } else {
            /* Did not exit or not a directory. */
            return nullptr;
        }
    });
}

bool MemoryFilesystem::
removeDirectory(std::string const &path, bool recursive)
{
    return WalkPath<MemoryFilesystem::Entry>(this, path, false, [&recursive](MemoryFilesystem::Entry *parent, std::string const &name, MemoryFilesystem::Entry *entry) -> MemoryFilesystem::Entry * {
        if (entry != nullptr && entry->type() == Type::Directory) {
            /* Only remove empty directories unless recursive. */
            if (!recursive && !entry->children().empty()) {
                return nullptr;
            }

            /* Remove directory. */
            std::vector<MemoryFilesystem::Entry> *children = &parent->children();
            children->erase(std::remove_if(children->begin(), children->end(), [&](MemoryFilesystem::Entry const &entry) {
                return (entry.name() == name);
            }), children->end());
            return parent;
        } else {
            /* Did not exist or not a directory. */
            return nullptr;
        }
    });
}

std::string MemoryFilesystem::
resolvePath(std::string const &path) const
{
    if (this->exists(path)) {
        return FSUtil::NormalizePath(path);
    } else {
        return std::string();
    }
}

