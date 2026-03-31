/*
TEST_BUILD
    $C{COMPILE} $DIR/include-warnings.c -o include-warnings.exe -Wsystem-headers -Weverything -Wno-undef -Wno-old-style-cast -Wno-nullability-extension -Wno-c++98-compat 2>&1 | grep -v 'In file' | grep objc || true
END

TEST_RUN_OUTPUT
OK: includes.c
END
*/

// Detect warnings inside any header.
// The build command above filters out warnings inside non-objc headers 
// (which are noisy with -Weverything).
// -Wno-undef suppresses warnings about `#if __cplusplus` and the like.
// -Wno-old-style-cast is tough to avoid in mixed C/C++ code.
// -Wno-nullability-extension disables a warning about non-portable
//   _Nullable etc which we already handle correctly in objc-abi.h.
// -Wno-c++98-compat disables warnings about things that already
//   have guards against C++98.

#include "includes.c"
