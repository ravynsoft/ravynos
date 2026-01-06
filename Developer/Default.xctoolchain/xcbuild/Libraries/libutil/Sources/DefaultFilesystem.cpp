/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <libutil/DefaultFilesystem.h>
#include <libutil/FSUtil.h>

#include <stack>
#include <climits>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno>

#if _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <libgen.h>
#include <dirent.h>
#include <sys/stat.h>
#if defined(__APPLE__) || defined(__FreeBSD__)
#include <copyfile.h>
#endif
#endif

using libutil::DefaultFilesystem;
using libutil::Filesystem;
using libutil::Permissions;

#if _WIN32
using WideString = std::basic_string<std::remove_const<std::remove_pointer<LPCWSTR>::type>::type>;

static std::string
WideStringToString(WideString const &str)
{
    int size = WideCharToMultiByte(CP_UTF8, 0, str.data(), (int)str.size(), NULL, 0, NULL, NULL);
    std::string multi = std::string();
    multi.resize(size);
    WideCharToMultiByte(CP_UTF8, 0, str.data(), (int)str.size(), &multi[0], size, NULL, NULL);
    return multi;
}

static WideString
StringToWideString(std::string const &str)
{
    int size = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), NULL, 0);
    WideString wide = WideString();
    wide.resize(size);
    MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), &wide[0], size);
    return wide;
}
#endif

#if _WIN32
#if _MSC_VER
#define __PACKED_STRUCT_BEGIN __pragma(pack(push, 1))
#define __PACKED_STRUCT_END __pragma(pack(pop))
#else
#define __PACKED_STRUCT_BEGIN
#define __PACKED_STRUCT_END __attribute__((packed))
#endif

#if _MSC_VER
__pragma(warning(push))
__pragma(warning(disable: 4201))
#endif

__PACKED_STRUCT_BEGIN struct _REPARSE_DATA_BUFFER {
    ULONG  ReparseTag;
    USHORT ReparseDataLength;
    USHORT Reserved;
    union {
        struct {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            ULONG  Flags;
            WCHAR  PathBuffer[1];
        } SymbolicLinkReparseBuffer;
        struct {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            WCHAR  PathBuffer[1];
        } MountPointReparseBuffer;
        struct {
            UCHAR DataBuffer[1];
        } GenericReparseBuffer;
    };
} __PACKED_STRUCT_END;

typedef _REPARSE_DATA_BUFFER REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;

#if _MSC_VER
__pragma(warning(pop))
#endif
#endif

bool DefaultFilesystem::
exists(std::string const &path) const
{
#if _WIN32
    WideString wide = StringToWideString(path);
    DWORD attributes = GetFileAttributesW(wide.data());
    return (attributes != INVALID_FILE_ATTRIBUTES);
#else
    return ::access(path.c_str(), F_OK) == 0;
#endif
}

ext::optional<Filesystem::Type> DefaultFilesystem::
type(std::string const &path) const
{
#if _WIN32
    WideString wide = StringToWideString(path);

    static DWORD const share = (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE);
    static DWORD const flags = (FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS);
    HANDLE handle = CreateFileW(wide.c_str(), 0, share, nullptr, OPEN_EXISTING, flags, nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        return ext::nullopt;
    }

    BY_HANDLE_FILE_INFORMATION information;
    if (!GetFileInformationByHandle(handle, &information)) {
        CloseHandle(handle);
        return ext::nullopt;
    }

    if ((information.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0) {
        BYTE buffer[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
        REPARSE_DATA_BUFFER *data = reinterpret_cast<REPARSE_DATA_BUFFER *>(&buffer);

        DWORD ignored = 0;
        if (!DeviceIoControl(handle, FSCTL_GET_REPARSE_POINT, nullptr, 0, data, MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &ignored, nullptr)) {
            CloseHandle(handle);
            return ext::nullopt;
        }

        if ((data->ReparseTag & IO_REPARSE_TAG_SYMLINK) != 0) {
            CloseHandle(handle);
            return Type::SymbolicLink;
        } else {
            /* Some other type of reparse point. */
            CloseHandle(handle);
            return ext::nullopt;
        }
    } else if ((information.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
        CloseHandle(handle);
        return Type::Directory;
    } else {
        /* No flag for files; assume as default. */
        CloseHandle(handle);
        return Type::File;
    }
#else
    struct stat st;
    if (::lstat(path.c_str(), &st) < 0) {
        return ext::nullopt;
    }

    if (S_ISREG(st.st_mode)) {
        return Type::File;
    } else if (S_ISLNK(st.st_mode)) {
        return Type::SymbolicLink;
    } else if (S_ISDIR(st.st_mode)) {
        return Type::Directory;
    } else {
        /* Unsupported file type, e.g. character or block device. */
        return ext::nullopt;
    }
#endif
}

bool DefaultFilesystem::
isReadable(std::string const &path) const
{
#if _WIN32
    WideString wide = StringToWideString(path);

    static DWORD const share = (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE);
    static DWORD const flags = (FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS);
    HANDLE handle = CreateFileW(wide.c_str(), GENERIC_READ, share, nullptr, OPEN_EXISTING, flags, nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        return false;
    }

    CloseHandle(handle);
    return true;
#else
    return ::access(path.c_str(), R_OK) == 0;
#endif
}

bool DefaultFilesystem::
isWritable(std::string const &path) const
{
#if _WIN32
    WideString wide = StringToWideString(path);

    static DWORD const share = (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE);
    static DWORD const flags = (FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS);
    HANDLE handle = CreateFileW(wide.c_str(), GENERIC_WRITE, share, nullptr, OPEN_EXISTING, flags, nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        return false;
    }

    CloseHandle(handle);
    return true;
#else
    return ::access(path.c_str(), W_OK) == 0;
#endif
}

bool DefaultFilesystem::
isExecutable(std::string const &path) const
{
#if _WIN32
    WideString wide = StringToWideString(path);

    DWORD type = 0;
    if (!GetBinaryTypeW(wide.c_str(), &type)) {
        return false;
    }

    return true;
#else
    return ::access(path.c_str(), X_OK) == 0;
#endif
}

#if !_WIN32
static Permissions
ModePermissions(mode_t mode)
{
    Permissions permissions;
    permissions.flag(Permissions::Flag::Sticky, (mode & S_ISVTX) != 0);
    permissions.flag(Permissions::Flag::SetUserID, (mode & S_ISUID) != 0);
    permissions.flag(Permissions::Flag::SetGroupID, (mode & S_ISGID) != 0);
    permissions.user(Permissions::Permission::Read, (mode & S_IRUSR) != 0);
    permissions.user(Permissions::Permission::Write, (mode & S_IWUSR) != 0);
    permissions.user(Permissions::Permission::Execute, (mode & S_IXUSR) != 0);
    permissions.group(Permissions::Permission::Read, (mode & S_IRGRP) != 0);
    permissions.group(Permissions::Permission::Write, (mode & S_IWGRP) != 0);
    permissions.group(Permissions::Permission::Execute, (mode & S_IXGRP) != 0);
    permissions.other(Permissions::Permission::Read, (mode & S_IROTH) != 0);
    permissions.other(Permissions::Permission::Write, (mode & S_IWOTH) != 0);
    permissions.other(Permissions::Permission::Execute, (mode & S_IXOTH) != 0);
    return permissions;
}
#endif

ext::optional<Permissions> DefaultFilesystem::
readFilePermissions(std::string const &path) const
{
#if _WIN32
    // TODO: Support file permssions on Windows.
    return ext::nullopt;
#else
    struct stat st;
    if (::stat(path.c_str(), &st) < 0) {
        return ext::nullopt;
    }

    return ModePermissions(st.st_mode);
#endif
}

ext::optional<Permissions> DefaultFilesystem::
readSymbolicLinkPermissions(std::string const &path) const
{
#if _WIN32
    // TODO: Support file permssions on Windows.
    return ext::nullopt;
#else
    struct stat st;
    if (::lstat(path.c_str(), &st) < 0) {
        return ext::nullopt;
    }

    return ModePermissions(st.st_mode);
#endif
}

ext::optional<Permissions> DefaultFilesystem::
readDirectoryPermissions(std::string const &path) const
{
#if _WIN32
    // TODO: Support file permssions on Windows.
    return ext::nullopt;
#else
    struct stat st;
    if (::stat(path.c_str(), &st) < 0) {
        return ext::nullopt;
    }

    return ModePermissions(st.st_mode);
#endif
}

#if !_WIN32
static mode_t
PermissionsMode(Permissions permissions)
{
    mode_t mode = 0;
    mode |= (permissions.flag(Permissions::Flag::Sticky) ? S_ISVTX : 0);
    mode |= (permissions.flag(Permissions::Flag::SetUserID) ? S_ISUID : 0);
    mode |= (permissions.flag(Permissions::Flag::SetGroupID) ? S_ISUID : 0);
    mode |= (permissions.user(Permissions::Permission::Read) ? S_IRUSR : 0);
    mode |= (permissions.user(Permissions::Permission::Write) ? S_IWUSR : 0);
    mode |= (permissions.user(Permissions::Permission::Execute) ? S_IXUSR : 0);
    mode |= (permissions.group(Permissions::Permission::Read) ? S_IRUSR : 0);
    mode |= (permissions.group(Permissions::Permission::Write) ? S_IWUSR : 0);
    mode |= (permissions.group(Permissions::Permission::Execute) ? S_IXUSR : 0);
    mode |= (permissions.other(Permissions::Permission::Read) ? S_IRUSR : 0);
    mode |= (permissions.other(Permissions::Permission::Write) ? S_IWUSR : 0);
    mode |= (permissions.other(Permissions::Permission::Execute) ? S_IXUSR : 0);
    return mode;
}
#endif

bool DefaultFilesystem::
writeFilePermissions(std::string const &path, Permissions::Operation operation, Permissions permissions)
{
#if _WIN32
    // TODO: Support file permssions on Windows.
    return true;
#else
    ext::optional<Permissions> updated = this->readFilePermissions(path);
    if (!updated) {
        return false;
    }

    updated->combine(operation, permissions);
    mode_t mode = PermissionsMode(*updated);

    if (::chmod(path.c_str(), mode) != 0) {
        return false;
    }

    return true;
#endif
}

bool DefaultFilesystem::
writeSymbolicLinkPermissions(std::string const &path, Permissions::Operation operation, Permissions permissions)
{
#if _WIN32
    // TODO: Support file permssions on Windows.
    return true;
#elif defined(__APPLE__) || defined(__FreeBSD__)
    ext::optional<Permissions> updated = this->readSymbolicLinkPermissions(path);
    if (!updated) {
        return false;
    }

    updated->combine(operation, permissions);
    mode_t mode = PermissionsMode(*updated);

    if (::lchmod(path.c_str(), mode) != 0) {
        return false;
    }

    return true;
#else
    /* Symbolic links have no permissions on most filesystems. */
    return true;
#endif
}

bool DefaultFilesystem::
writeDirectoryPermissions(std::string const &path, Permissions::Operation operation, Permissions permissions, bool recursive)
{
    ext::optional<Permissions> updated = this->readDirectoryPermissions(path);
    if (!updated) {
        return false;
    }

    updated->combine(operation, permissions);

#if _WIN32
    // TODO: Support file permssions on Windows.
    return true;
#else
    mode_t mode = PermissionsMode(*updated);

    if (::chmod(path.c_str(), mode) != 0) {
        return false;
    }
#endif

    if (recursive) {
        bool success = true;

        success &= this->readDirectory(path, recursive, [this, &path, &operation, &permissions, &success](std::string const &name) {
            std::string full = path + "/" + name;

            ext::optional<Type> type = this->type(full);
            if (!type) {
                return false;
            }

            switch (*type) {
                case Type::File:
                    if (!this->writeFilePermissions(full, operation, permissions)) {
                        success = false;
                        return false;
                    }
                    break;
                case Type::Directory:
                    /* Already recursing directory; don't need to recurse again. */
                    if (!this->writeDirectoryPermissions(full, operation, permissions, false)) {
                        success = false;
                        return false;
                    }
                    break;
                case Type::SymbolicLink:
                    if (!this->writeSymbolicLinkPermissions(full, operation, permissions)) {
                        success = false;
                        return false;
                    }
                    break;
            }

            return true;
        });

        if (!success) {
            return false;
        }
    }

    return true;
}

bool DefaultFilesystem::
createFile(std::string const &path)
{
#if _WIN32
    WideString wide = StringToWideString(path);

    static DWORD const share = (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE);
    HANDLE handle = CreateFileW(wide.c_str(), 0, share, nullptr, OPEN_ALWAYS, 0, nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        return false;
    }


    CloseHandle(handle);
    return true;
#else
    if (this->isWritable(path)) {
        return true;
    }

    FILE *fp = std::fopen(path.c_str(), "w");
    if (fp == nullptr) {
        return false;
    }

    std::fclose(fp);
    return true;
#endif
}

bool DefaultFilesystem::
read(std::vector<uint8_t> *contents, std::string const &path, size_t offset, ext::optional<size_t> length) const
{
#if _WIN32
    WideString wide = StringToWideString(path);

    static DWORD const share = (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE);
    HANDLE handle = CreateFileW(wide.c_str(), GENERIC_READ, share, nullptr, OPEN_EXISTING, 0, nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD size = GetFileSize(handle, nullptr);
    if (size == INVALID_FILE_SIZE) {
        CloseHandle(handle);
        return false;
    }

    if (SetFilePointer(handle, offset, nullptr, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
        CloseHandle(handle);
        return false;
    }

    if (length) {
        if (offset + *length > static_cast<size_t>(size)) {
            CloseHandle(handle);
            return false;
        }

        size = *length;
    }

    *contents = std::vector<uint8_t>(size);

    DWORD bytesRead;
    if (!ReadFile(handle, contents->data(), size, &bytesRead, nullptr)) {
        CloseHandle(handle);
        return false;
    }

    CloseHandle(handle);
    return true;
#else
    FILE *fp = std::fopen(path.c_str(), "rb");
    if (fp == nullptr) {
        return false;
    }

    if (std::fseek(fp, 0, SEEK_END) != 0) {
        std::fclose(fp);
        return false;
    }

    long size = std::ftell(fp);
    if (size == (long)-1) {
        std::fclose(fp);
        return false;
    }

    if (length) {
        if (offset + *length > static_cast<size_t>(size)) {
            std::fclose(fp);
            return false;
        }

        size = *length;
    }

    if (std::fseek(fp, offset, SEEK_SET) != 0) {
        std::fclose(fp);
        return false;
    }

    *contents = std::vector<uint8_t>(size);

    if (size > 0) {
        if (std::fread(contents->data(), size, 1, fp) != 1) {
            std::fclose(fp);
            return false;
        }
    }

    std::fclose(fp);

    return true;
#endif
}

bool DefaultFilesystem::
write(std::vector<uint8_t> const &contents, std::string const &path)
{
#if _WIN32
    WideString wide = StringToWideString(path);

    static DWORD const share = (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE);
    HANDLE handle = CreateFileW(wide.c_str(), GENERIC_WRITE, share, nullptr, CREATE_ALWAYS, 0, nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD bytesWritten;
    if (!WriteFile(handle, contents.data(), contents.size(), &bytesWritten, nullptr)) {
        CloseHandle(handle);
        return false;
    }

    CloseHandle(handle);
    return true;
#else
    FILE *fp = std::fopen(path.c_str(), "wb");
    if (fp == nullptr) {
        return false;
    }

    size_t size = contents.size();

    if (size > 0) {
        if (std::fwrite(contents.data(), size, 1, fp) != 1) {
            std::fclose(fp);
            return false;
        }
    }

    std::fclose(fp);

    return true;
#endif
}

bool DefaultFilesystem::
copyFile(std::string const &from, std::string const &to)
{
#if _WIN32
    ext::optional<Type> fromType = this->type(from);
    if (fromType != Type::File && fromType != Type::SymbolicLink) {
        return false;
    }

    ext::optional<Type> toType = this->type(to);
    if (toType == Type::SymbolicLink) {
        /* CopyFile() will otherwise copy to the target. */
        if (!this->removeSymbolicLink(to)) {
            return false;
        }
    }

    WideString fwide = StringToWideString(from);
    WideString twide = StringToWideString(to);
    if (!CopyFileW(fwide.c_str(), twide.c_str(), FALSE)) {
        return false;
    }

    return true;
#elif defined(__APPLE__) || defined(__FreeBSD__)
    ext::optional<Type> fromType = this->type(from);
    if (fromType != Type::File && fromType != Type::SymbolicLink) {
        return false;
    }

    ext::optional<Type> toType = this->type(to);
    if (toType) {
        switch (*toType) {
            case Type::File:
                if (!this->removeFile(to)) {
                    return false;
                }
                break;
            case Type::SymbolicLink:
                if (!this->removeSymbolicLink(to)) {
                    return false;
                }
                break;
            case Type::Directory:
                return false;
        }
    }

    copyfile_state_t state = ::copyfile_state_alloc();
    copyfile_flags_t flags = COPYFILE_ALL | COPYFILE_NOFOLLOW;
    if (::copyfile(from.c_str(), to.c_str(), state, flags)) {
        return false;
    }
    ::copyfile_state_free(state);

    return true;
#else
    return Filesystem::copyFile(from, to);
#endif
}

bool DefaultFilesystem::
removeFile(std::string const &path)
{
#if _WIN32
    WideString wide = StringToWideString(path);
    if (!DeleteFileW(wide.c_str())) {
        return false;
    }

    return true;
#else
    if (::unlink(path.c_str()) < 0) {
        return false;
    }

    return true;
#endif
}

ext::optional<std::string> DefaultFilesystem::
readSymbolicLink(std::string const &path, bool *directory) const
{
#if _WIN32
    WideString wide = StringToWideString(path);

    static DWORD const share = (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE);
    static DWORD const flags = (FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS);
    HANDLE handle = CreateFileW(wide.c_str(), 0, share, nullptr, OPEN_EXISTING, flags, nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        return ext::nullopt;
    }

    BY_HANDLE_FILE_INFORMATION information;
    if (!GetFileInformationByHandle(handle, &information)) {
        CloseHandle(handle);
        return ext::nullopt;
    }

    if ((information.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) == 0) {
        CloseHandle(handle);
        return ext::nullopt;
    }

    BYTE buffer[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
    REPARSE_DATA_BUFFER *data = reinterpret_cast<REPARSE_DATA_BUFFER *>(&buffer);

    DWORD ignored = 0;
    if (!DeviceIoControl(handle, FSCTL_GET_REPARSE_POINT, nullptr, 0, data, MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &ignored, nullptr)) {
        CloseHandle(handle);
        return ext::nullopt;
    }

    if ((data->ReparseTag & IO_REPARSE_TAG_SYMLINK) == 0) {
        CloseHandle(handle);
        return ext::nullopt;
    }

    CloseHandle(handle);

    if (directory) {
        *directory = (information.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    }

    size_t offset = (data->SymbolicLinkReparseBuffer.SubstituteNameOffset / sizeof(WCHAR));
    size_t length = (data->SymbolicLinkReparseBuffer.SubstituteNameLength / sizeof(WCHAR));
    WCHAR *pathBuffer = data->SymbolicLinkReparseBuffer.PathBuffer;
    WideString target = WideString(&pathBuffer[offset], &pathBuffer[offset + length]);
    return WideStringToString(target);
#else
    for (size_t size = PATH_MAX; true; size *= 2) {
        std::string current = std::string();
        current.resize(size);

        ssize_t len = ::readlink(path.c_str(), &current[0], current.size());
        if (len == current.size()) {
            /* May need more space. */
        } else if (len != -1) {
            /* Success. */
            if (directory) {
                *directory = (this->type(this->resolvePath(path)) == Type::Directory);
            }
            return std::string(current.c_str());
        } else {
            return ext::nullopt;
        }
    }
#endif
}

bool DefaultFilesystem::
writeSymbolicLink(std::string const &target, std::string const &path, bool directory)
{
#if _WIN32
    WideString twide = StringToWideString(target);
    WideString pwide = StringToWideString(path);

#if __MINGW32__
#if !defined(CreateSymbolicLink)
    static BOOL(*CreateSymbolicLinkW)(LPCWSTR, LPCWSTR, DWORD) = nullptr;
    if (CreateSymbolicLinkW == nullptr) {
        CreateSymbolicLinkW = reinterpret_cast<decltype(CreateSymbolicLinkW)>(GetProcAddress(GetModuleHandle("kernel32.dll"), "CreateSymbolicLinkW"));
    }
#endif
#if !defined(SYMBOLIC_LINK_FLAG_DIRECTORY)
#define SYMBOLIC_LINK_FLAG_DIRECTORY 0x1
#endif
#endif

    DWORD flags = (directory ? SYMBOLIC_LINK_FLAG_DIRECTORY : 0);
    if (!CreateSymbolicLinkW(pwide.c_str(), twide.c_str(), flags)) {
        return false;
    }

    return true;
#else
    if (::symlink(target.c_str(), path.c_str()) != 0) {
        return false;
    }

    return true;
#endif
}

bool DefaultFilesystem::
copySymbolicLink(std::string const &from, std::string const &to)
{
#if _WIN32
    ext::optional<Type> fromType = this->type(from);
    if (fromType != Type::SymbolicLink) {
        return false;
    }

#if __MINGW32__
#if !defined(COPY_FILE_COPY_SYMLINK)
#define COPY_FILE_COPY_SYMLINK 0x00000800
#endif
#endif

    WideString fwide = StringToWideString(from);
    WideString twide = StringToWideString(to);
    if (!CopyFileExW(fwide.c_str(), twide.c_str(), nullptr, nullptr, nullptr, COPY_FILE_COPY_SYMLINK)) {
        return false;
    }

    return true;
#elif defined(__APPLE__) || defined(__FreeBSD__)
    if (this->type(from) != Type::SymbolicLink) {
        return false;
    }

    if (this->type(to) == Type::SymbolicLink) {
        if (!this->removeSymbolicLink(to)) {
            return false;
        }
    }

    copyfile_state_t state = ::copyfile_state_alloc();
    copyfile_flags_t flags = COPYFILE_ALL | COPYFILE_NOFOLLOW;
    if (::copyfile(from.c_str(), to.c_str(), state, flags)) {
        return false;
    }
    ::copyfile_state_free(state);

    return true;
#else
    return Filesystem::copySymbolicLink(from, to);
#endif
}

bool DefaultFilesystem::
removeSymbolicLink(std::string const &path)
{
    if (this->type(path) == Type::SymbolicLink) {
#if _WIN32
        WideString wide = StringToWideString(path);
        if (!DeleteFileW(wide.c_str())) {
            return false;
        }

        return true;
#else
        if (::unlink(path.c_str()) != 0) {
            return false;
        }
#endif
    }

    return true;
}

bool DefaultFilesystem::
createDirectory(std::string const &path, bool recursive)
{
#if !_WIN32
    /* Get and re-set current mode mask. */
    mode_t mask = ::umask(0);
    ::umask(mask);

    /* Mode is most allowed by mask. */
    mode_t mode = (S_IRWXU | S_IRWXG | S_IRWXO) & ~mask;
#endif

    if (recursive) {
        std::string current = path;
        std::stack<std::string> create;

        /* Build up list of directories to create. */
        while (this->type(current) != Type::Directory && current != "") {
            create.push(current);
            current = FSUtil::GetDirectoryName(current);
        }

        /* Create intermediate directories. */
        while (!create.empty()) {
            std::string const &directory = create.top();

#if _WIN32
            WideString wide = StringToWideString(directory);
            if (!CreateDirectoryW(wide.c_str(), nullptr)) {
                return false;
            }
#else
            if (::mkdir(directory.c_str(), mode) != 0) {
                return false;
            }
#endif

            create.pop();
        }
    } else {
#if _WIN32
            WideString wide = StringToWideString(path);
            if (!CreateDirectoryW(wide.c_str(), nullptr)) {
                return false;
            }
#else
            if (::mkdir(path.c_str(), mode) != 0) {
                return false;
            }
#endif
    }

    return true;
}

bool DefaultFilesystem::
readDirectory(std::string const &path, bool recursive, std::function<void(std::string const &)> const &cb) const
{
    std::function<bool(std::string const &, ext::optional<std::string> const &)> process =
        [this, &recursive, &cb, &process](std::string const &absolute, ext::optional<std::string> const &relative) -> bool {
#if _WIN32
        WideString wide = StringToWideString(absolute);
        wide += static_cast<wchar_t>('\\');
        wide += static_cast<wchar_t>('*');

        WIN32_FIND_DATAW data;
        HANDLE handle = FindFirstFileW(wide.c_str(), &data);
        if (handle == INVALID_HANDLE_VALUE) {
            return false;
        }
#else
        DIR *dp = ::opendir(absolute.c_str());
        if (dp == nullptr) {
            return false;
        }
#endif

        /* Report children. */
#if _WIN32
        do {
            std::string name_ = WideStringToString(WideString(data.cFileName));
            char const *name = name_.c_str();
#else
        while (struct dirent *entry = ::readdir(dp)) {
            char const *name = entry->d_name;
#endif
            if (::strcmp(name, ".") == 0 || ::strcmp(name, "..") == 0) {
                continue;
            }

            std::string path = (relative ? *relative + "/" + name : name);

            cb(path);
        }
#if _WIN32
        while (FindNextFileW(handle, &data));
        if (GetLastError() != ERROR_NO_MORE_FILES) {
            return false;
        }
#endif

        /* Process subdirectories. */
        if (recursive) {
#if _WIN32
            FindClose(handle);
            handle = FindFirstFileW(wide.c_str(), &data);
            if (handle == INVALID_HANDLE_VALUE) {
                return false;
            }
#else
            ::rewinddir(dp);
#endif

#if _WIN32
            do {
                std::string name_ = WideStringToString(WideString(data.cFileName));
                char const *name = name_.c_str();
#else
            while (struct dirent *entry = ::readdir(dp)) {
                char const *name = entry->d_name;
#endif
                if (::strcmp(name, ".") == 0 || ::strcmp(name, "..") == 0) {
                    continue;
                }

                std::string full = absolute + "/" + name;

                if (this->type(full) == Type::Directory) {
                    std::string path = (relative ? *relative + "/" + name : name);
                    if (!process(full, path)) {
#if _WIN32
                        FindClose(handle);
#else
                        ::closedir(dp);
#endif
                        return false;
                    }
                }
            }

#if _WIN32
            while (FindNextFileW(handle, &data));
            if (GetLastError() != ERROR_NO_MORE_FILES) {
                return false;
            }
#endif
        }

#if _WIN32
        FindClose(handle);
#else
        ::closedir(dp);
#endif
        return true;
    };

    return process(path, ext::nullopt);
}

bool DefaultFilesystem::
copyDirectory(std::string const &from, std::string const &to, bool recursive)
{
#if defined(__APPLE__) || defined(__FreeBSD__)
    if (this->type(from) != Type::Directory) {
        return false;
    }

    if (this->type(to) == Type::Directory) {
        if (!this->removeDirectory(to, recursive)) {
            return false;
        }
    }

    copyfile_state_t state = ::copyfile_state_alloc();
    copyfile_flags_t flags = COPYFILE_ALL | COPYFILE_NOFOLLOW | (recursive ? COPYFILE_RECURSIVE : 0);
    if (::copyfile(from.c_str(), to.c_str(), state, flags)) {
        return false;
    }
    ::copyfile_state_free(state);

    return true;
#else
    return Filesystem::copyDirectory(from, to, recursive);
#endif
}

bool DefaultFilesystem::
removeDirectory(std::string const &path, bool recursive)
{
    if (recursive) {
        bool success = true;

        success &= this->readDirectory(path, recursive, [this, &path, &success](std::string const &name) {
            std::string full = path + "/" + name;

            ext::optional<Type> type = this->type(full);
            if (!type) {
                return false;
            }

            switch (*type) {
                case Type::File:
                    if (!this->removeFile(full)) {
                        success = false;
                        return false;
                    }
                    break;
                case Type::SymbolicLink:
                    if (!this->removeSymbolicLink(full)) {
                        success = false;
                        return false;
                    }
                    break;
                case Type::Directory:
                    if (!this->removeDirectory(full, false)) {
                        success = false;
                        return false;
                    }
                    break;
            }

            return true;
        });

        if (!success) {
            return false;
        }
    }

#if _WIN32
    WideString wide = StringToWideString(path);
    if (!RemoveDirectoryW(wide.c_str())) {
        return false;
    }
#else
    if (::rmdir(path.c_str()) != 0) {
        return false;
    }
#endif

    return true;
}

std::string DefaultFilesystem::
resolvePath(std::string const &path) const
{
#if _WIN32
    WideString wide = StringToWideString(path);

    static DWORD const flags = FILE_FLAG_BACKUP_SEMANTICS;
    static DWORD const share = (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE);
    HANDLE handle = CreateFileW(wide.c_str(), 0, share, NULL, OPEN_EXISTING, flags, NULL);
    if (handle == INVALID_HANDLE_VALUE) {
        return std::string();
    }

#if __MINGW32__
#if !defined(GetFinalPathNameByHandle)
    static DWORD(*GetFinalPathNameByHandleW)(HANDLE, LPWSTR, DWORD, DWORD) = nullptr;
    if (GetFinalPathNameByHandleW == nullptr) {
        GetFinalPathNameByHandleW = reinterpret_cast<decltype(GetFinalPathNameByHandleW)>(GetProcAddress(GetModuleHandle("kernel32.dll"), "GetFinalPathNameByHandleW"));
    }
#endif
#endif

    static DWORD const options = (FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
    DWORD size = GetFinalPathNameByHandleW(handle, nullptr, 0, options);
    if (size == 0) {
        CloseHandle(handle);
        return std::string();
    }

    WideString buffer;
    buffer.resize(size);
    if (GetFinalPathNameByHandleW(handle, &buffer[0], buffer.size(), options) == 0) {
        CloseHandle(handle);
        return std::string();
    }

    /*
     * The size returned from `GetFinalPathNameByHandleW()` is one too big on some versions.
     * Rather than try and detect those versions, just detect a too-big buffer and truncate.
     */
    size_t length = wcslen(buffer.c_str());
    if (size > length) {
        buffer.resize(length);
    }

    CloseHandle(handle);

    std::string result = WideStringToString(buffer);

    /*
     * Convert from NT path to DOS path. Otherwise, anything using this path will lose
     * normalization, which will break with any forward slashes that end up in the path.
     */
    if (result.size() >= 7 &&
        (result[0] == '\\' && result[1] == '\\' && result[2] == '?' && result[3] == '\\') &&
        (::isalpha(result[4]) && result[5] == ':' && result[6] == '\\')) {
        /* Trim the NT path designator. */
        result = result.substr(4);
    }

    return result;
#else
    char realPath[PATH_MAX + 1];
    if (::realpath(path.c_str(), realPath) == nullptr) {
        return std::string();
    } else {
        return realPath;
    }
#endif
}
