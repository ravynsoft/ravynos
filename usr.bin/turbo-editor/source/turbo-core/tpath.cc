#include <turbo/tpath.h>
#include <tvision/compat/borland/io.h> // 'access' on Windows

TStringView TPath::basename(TStringView path) noexcept
{
    size_t start = drivename(path).size();
    size_t end = start;
    for (size_t i = path.size(); i != start;)
    {
        --i;
        if (!isSep(path[i]))
        {
            end = i + 1;
            break;
        }
    }
    size_t beg = start;
    for (size_t j = end; j != start;)
    {
        --j;
        if (isSep(path[j]))
        {
            beg = j + 1;
            break;
        }
    }
    // Be aware that other functions (e.g. 'dirname') may rely on 'basename'
    // always retuning a substring of 'path'.
    return path.substr(beg, end - beg);
}

TStringView TPath::dirname(TStringView path) noexcept
{
    TStringView bn = basename(path);
    size_t end = &bn[0] - &path[0];
    for (size_t i = end; i != 0;)
    {
        --i;
        if (!isSep(path[i]))
        {
            end = i + 1;
            break;
        }
    }
    TStringView root = rootname(path);
    if (end && end >= root.size())
        return path.substr(0, end);
    else if (root.empty())
        return ".";
    else
        return root;
}

TStringView TPath::extname(TStringView path) noexcept
{
    TStringView bn = basename(path);
    size_t end = bn.size();
    for (size_t i = end; i != 0;)
    {
        --i;
        if (bn[i] == '.')
        {
            size_t begin = i;
            if (begin == 0 || (end == 2 && begin == 1 && bn[0] == '.'))
                return TStringView();
            return bn.substr(begin, end - begin);
        }
    }
    return TStringView();
}

TStringView TPath::rootname(TStringView path) noexcept
{
    switch (path.size())
    {
        case 0:
            break;
        case 1:
        case 2:
            if (isSep(path[0]))
                return path.substr(0, 1);
            break;
        default:
            if (isSep(path[0]))
                return path.substr(0, 1);
            if (isSep(path[2]) && path[1] == ':' && isDrive(path[0]))
                return path.substr(0, 3);
            break;
    }
    return path.substr(0, 0);
}

TStringView TPath::drivename(TStringView path) noexcept
{
    switch (path.size())
    {
        case 0:
            break;
        case 1:
            if (isSep(path[0]))
                return path.substr(0, 1);
            break;
        case 2:
            if (isSep(path[0]))
                return path.substr(0, 1);
            if (path[1] == ':' && isDrive(path[0]))
                return path.substr(0, 2);
            break;
        default:
            if (isSep(path[0]))
                return path.substr(0, 1);
            if (path[1] == ':' && isDrive(path[0]))
                return path.substr(0, isSep(path[2]) ? 3 : 2);
            break;
    }
    return path.substr(0, 0);
}

Boolean TPath::isAbsolute(TStringView path) noexcept
{
    switch (path.size())
    {
        case 0:
            return False;
        case 1:
        case 2:
            return isSep(path[0]);
        default:
            return isSep(path[0]) || (isSep(path[2]) && path[1] == ':' && isDrive(path[0]));
    }
}

Boolean TPath::exists(const char *path) noexcept
{
    return ::access(path, 0 /* F_OK */) == 0;
}

TStringView TPath::resolve(char abspath[MAXPATH], TStringView path) noexcept
{
    strnzcpy(abspath, path, MAXPATH);
    fexpand(abspath);
    return abspath;
}
