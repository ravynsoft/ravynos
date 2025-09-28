#ifndef TVISION_CONSTARR_H
#define TVISION_CONSTARR_H

#include <stddef.h>

// std::array is not constexpr until C++17. So we make our own, which costs
// nothing.

namespace tvision
{

template <class T, size_t N>
struct constarray
{
    T elems[N];

    constexpr T& operator[](size_t i) noexcept
    {
        return elems[i];
    }

    constexpr const T& operator[](size_t i) const noexcept
    {
        return elems[i];
    }

};

} // namespace tvision

#endif // TVISION_CONSTARR_H
