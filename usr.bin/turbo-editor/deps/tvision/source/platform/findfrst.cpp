#include <internal/findfrst.h>
#include <internal/pathconv.h>

#define SPECIAL_BITS (_A_SUBDIR|_A_HIDDEN|_A_SYSTEM)

namespace tvision
{

FindFirstRec::RecList FindFirstRec::recList;

FindFirstRec* FindFirstRec::allocate(struct find_t *fileinfo, unsigned attrib,
                                     const char *pathname) noexcept
{
    // The findfirst interface based on DOS call 0x4E doesn't provide a
    // findclose function. The strategy here is the same as in Borland's RTL:
    // new directory streams are allocated unless fileinfo is an address
    // that has already been passed to us before.
    if (!fileinfo)
        return 0;
    std::lock_guard<std::mutex> lock(recList);
    FindFirstRec *r = 0;
    size_t index;

    for (index = 0; index < recList.size(); ++index)
        if (recList[index].finfo == fileinfo)
        {
            r = &recList[index];
            break;
        }
    // At this point, 'index' is either the position of the matching FindFirstRec
    // item or the size of recList, in which case a new item will be added
    // at the end of the list and 'index' will be a valid index pointing at it.
    if (r)
        r->close();
    else
    {
        recList.emplace_back();
        r = &recList[index];
        r->finfo = fileinfo;
    }
    // If pathname is a valid directory, make fileinfo point to the allocated
    // FindFirstRec. Otherwise, return NULL.
    if (r->setParameters(attrib, pathname))
    {
        fileinfo->reserved = (long) index;
        return r;
    }
    else
        return 0;
}

FindFirstRec* FindFirstRec::get(struct find_t *fileinfo) noexcept
{
    // Return the FindFirstRec instance pointed to by fileinfo.
    std::lock_guard<std::mutex> lock(recList);
    size_t pos = fileinfo->reserved;
    if (0 <= pos && pos < recList.size() && recList[pos].finfo == fileinfo)
        return &recList[pos];
    return 0;
}

#ifndef _WIN32

bool FindFirstRec::setParameters(unsigned attrib, const char *pathname) noexcept
{
    if (!dirStream)
    {
        searchAttr = attrib;
        if (setPath(pathname))
            return open();
    }
    return false;
}

bool FindFirstRec::next() noexcept
{
    struct dirent *e;
    bool found = false;
    while ((e = readdir(dirStream)) && !(found = matchEntry(e)));
    if (!e)
        close();
    return found;
}

bool FindFirstRec::open() noexcept
{
    if (!dirStream)
        return (dirStream = opendir(searchDir.c_str()));
    return false;
}

void FindFirstRec::close() noexcept
{
    if (dirStream)
        closedir(dirStream), dirStream = 0;
}

bool FindFirstRec::setPath(const char* pathname) noexcept
{
    // Reject NULL or empty pathnames.
    if (pathname && *pathname)
    {
        searchDir = pathname;
        path_dos2unix(searchDir);
        // Win32's FindFirst is designed to reject paths ending with a
        // separator. But legacy code unaware of Unix separators may be unable
        // to remove it and call findfirst with such a pathname. Therefore,
        // we handle this case gracefully.
        if (searchDir.back() == '/')
            wildcard = '.';
        else
        {
            auto lastSlash = searchDir.find_last_of('/');
            // When pathname doesn't contain a '/', wildcard keeps the whole
            // string and searchDir is set to the current directory.
            wildcard = searchDir.substr(lastSlash + 1);
            if (lastSlash == std::string::npos)
                searchDir = "./";
            else
                searchDir = searchDir.substr(0, lastSlash + 1);
            // '*.*' stands for 'any name, any extension'. We shouldn't expect
            // a dot in the filename in this case. In Borland C++, '*' yields
            // the same result as '*.*' in 32-bit builds, while it only returns
            // extensionless files in 16-bit builds.
            if (wildcard == "*.*")
                wildcard = '*';
        }
        // At this point, searchDir always ends with a '/'.
        return true;
    }
    return false;
}

bool FindFirstRec::matchEntry(struct dirent* e) noexcept
{
    struct stat st;
    if (wildcardMatch(wildcard.c_str(), e->d_name) &&
        stat((searchDir + e->d_name).c_str(), &st) == 0)
    {
        unsigned fileAttr = cvtAttr(&st, e->d_name);
        if (attrMatch(fileAttr))
        {
            // Match found, fill finfo.
            finfo->size = st.st_size;
            finfo->attrib = fileAttr;
            cvtTime(&st, finfo);
            strnzcpy(finfo->name, e->d_name, sizeof(find_t::name));
            return true;
        }
    }
    return false;
}

bool FindFirstRec::attrMatch(unsigned attrib) noexcept
{
    // Behaviour from the original _dos_findnext: 'if requested attribute
    // word includes hidden, system, or subdirectory bits, return
    // normal files AND those with any of the requested attributes'.
    return !(attrib & SPECIAL_BITS) || (searchAttr & attrib & SPECIAL_BITS);
}

bool FindFirstRec::wildcardMatch(char const *wildcard, char const *filename) noexcept
{
    // https://stackoverflow.com/a/3300547
    for (; *wildcard != '\0'; ++wildcard)
        switch (*wildcard)
        {
            case '?':
                if (*filename == '\0')
                    return false;
                ++filename;
                break;
            case '*':
                if (wildcard[1] == '\0')
                    return true;
                for (size_t i = 0; filename[i] != '\0'; ++i)
                    if (wildcardMatch(wildcard + 1, &filename[i]))
                        return true;
                return false;
            default:
                if (*filename != *wildcard)
                    return false;
                ++filename;
        }
    return *filename == '\0';
}

unsigned FindFirstRec::cvtAttr(const struct stat *st, const char* filename) noexcept
{
    // Returns file attributes in find_t format.
    unsigned attr = 0; // _A_NORMAL
    if (filename[0] == '.')
        attr |= _A_HIDDEN;
    if (st->st_mode & S_IFDIR)
        attr |= _A_SUBDIR;
    else if (!(st->st_mode & S_IFREG)) // If not a regular file
        attr |= _A_SYSTEM;
    else if (!(st->st_mode & S_IWUSR)) // If no write access, innacurate.
        attr |= _A_RDONLY;
    return attr;
}

void FindFirstRec::cvtTime(const struct stat *st, struct find_t *fileinfo) noexcept
{
    // Updates fileinfo with the times in st.
    struct FatDate {
        ushort  day     : 5, // Day of the month (1–31)
                month   : 4, // Month (1-12)
                year    : 7; // Year-1980
    } *wr_date = (FatDate *) &fileinfo->wr_date;
    struct FatTime {
        ushort  sec     : 5, // Seconds divided by 2
                min     : 6, // Minutes (0-59)
                hour    : 5; // Hour (0-23)
    } *wr_time = (FatTime *) &fileinfo->wr_time;

    struct tm *lt = localtime(&st->st_mtime);
    *wr_date = { ushort (lt->tm_mday),
                 ushort (lt->tm_mon + 1),
                 ushort (lt->tm_year - 80) };
    *wr_time = { ushort (lt->tm_sec/2),
                 ushort (lt->tm_min),
                 ushort (lt->tm_hour) };
}

#else

bool FindFirstRec::setParameters(unsigned attrib, const char *pathname) noexcept
{
    if (hFindFile != INVALID_HANDLE_VALUE)
        close();
    fileName.assign(pathname);
    searchAttr = attrib;
    return open();
}

bool FindFirstRec::next() noexcept
{
    WIN32_FIND_DATAW findData;
    while (true)
    {
        if (hFindFile == INVALID_HANDLE_VALUE)
        {
            MultiByteToWideChar(CP_UTF8, 0,
                                fileName.c_str(), -1,
                                findData.cFileName, sizeof(findData.cFileName)/sizeof(wchar_t));
            hFindFile = FindFirstFileW(findData.cFileName, &findData);
        }
        else if (!FindNextFileW(hFindFile, &findData))
        {
            close();
            return false;
        }

        if (hFindFile != INVALID_HANDLE_VALUE)
        {
            unsigned attr = cvtAttr(&findData, findData.cFileName);
            if (attrMatch(attr)) {
                // Match found, fill finfo.
                finfo->size = findData.nFileSizeLow;
                finfo->attrib = attr;
                cvtTime(&findData, finfo);
                WideCharToMultiByte(CP_UTF8, 0,
                                    findData.cFileName, -1,
                                    finfo->name, sizeof(finfo->name),
                                    nullptr, nullptr);
                return true;
            }
        }
        else
            return false;
    }
}

bool FindFirstRec::open() noexcept
{
    return true;
}

void FindFirstRec::close() noexcept
{
    if (hFindFile != INVALID_HANDLE_VALUE)
        FindClose(hFindFile), hFindFile = INVALID_HANDLE_VALUE;
}

bool FindFirstRec::attrMatch(unsigned attrib) noexcept
{
    return ((searchAttr & _A_VOLID) && (attrib & _A_VOLID))
        || !(attrib & SPECIAL_BITS)
        || (searchAttr & attrib & SPECIAL_BITS);
}

unsigned FindFirstRec::cvtAttr(const WIN32_FIND_DATAW *findData, const wchar_t* filename) noexcept
{
    unsigned attr = findData->dwFileAttributes;
    if (filename[0] == L'.')
        attr |= _A_HIDDEN;
    return attr;
}

void FindFirstRec::cvtTime(const WIN32_FIND_DATAW *findData, struct find_t *fileinfo) noexcept
{
    FILETIME localTime;
    FileTimeToLocalFileTime(&findData->ftLastWriteTime, &localTime);
    FileTimeToDosDateTime(&localTime, &fileinfo->wr_date, &fileinfo->wr_time);
}

#endif // _WIN32

} // namespace tvision
