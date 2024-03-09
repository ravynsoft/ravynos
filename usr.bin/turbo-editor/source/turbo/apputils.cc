#include "apputils.h"

active_counter &FileCounter::operator[](std::string_view file) noexcept
{
    // We need to keep at least one copy of 'file' alive.
    // This is because I don't want to use a map of std::string.
    auto it = map.find(file);
    if (it == map.end())
    {
        // Allocate string for the filename. We use a forward_list to avoid
        // reference invalidation.
        const auto &s = strings.emplace_front(file);
        it = map.emplace(s, active_counter()).first;
    }
    return it->second;
}
