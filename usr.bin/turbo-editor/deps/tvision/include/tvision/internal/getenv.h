#ifndef TVISION_GETENV_H
#define TVISION_GETENV_H

#include <stdlib.h>

namespace tvision
{

template<typename T>
inline T getEnv(const char* name, T def = T{}) noexcept
{
    const char* body = getenv(name);
    return body ? body : def;
}

template<>
inline int getEnv<int>(const char* name, int def) noexcept
{
    const char* body = getenv(name);
    if (body) {
        char* end;
        auto i = strtol(body, &end, 0);
        if (body != end)
            return i;
    }
    return def;
}

} // namespace tvision

#endif // TVISION_GETENV_H
