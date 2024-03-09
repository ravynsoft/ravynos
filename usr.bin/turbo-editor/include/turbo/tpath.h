#ifndef TURBO_TPATH_H
#define TURBO_TPATH_H

#include <tvision/tv.h>
#include <tvision/compat/borland/dir.h> // MAXPATH

class TPath
{
    // A namespace with methods for operating with filepaths.
    // Just like filepath-related methods in Turbo Vision:
    // * Both '/' and '\' are considered directory separators.
    // * Drive letters at the beginning of a path are considered regardless
    //   of the operating system.

public:

    static constexpr Boolean isSep(char c)
    {
        return c == '/' || c == '\\';
    }

    static constexpr Boolean isDrive(char c)
    {
        return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
    }

    // These return a substring of 'path'. They behave like the similarly-named
    // functions in Node.js's 'Path' module.
    static TStringView basename(TStringView path) noexcept;
    static TStringView dirname(TStringView path) noexcept;
    static TStringView extname(TStringView path) noexcept;
    static TStringView rootname(TStringView path) noexcept;
    static TStringView drivename(TStringView path) noexcept;

    static Boolean isAbsolute(TStringView path) noexcept;
    // Uses 'access' internally, hence the null-terminated parameter.
    static Boolean exists(const char *path) noexcept;
    // Stores the absolute version of 'path' into 'abspath' and returns
    // 'TStringView(abspath)'.
    // Uses Turbo Vision's 'fexpand' internally, hence the 'MAXPATH' limit.
    static TStringView resolve(char abspath[MAXPATH], TStringView path) noexcept;

};

#endif // TURBO_TPATH_H
