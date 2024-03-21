/* ------------------------------------------------------------------------*/
/*                                                                         */
/*   TSPAN.H                                                               */
/*                                                                         */
/*   Defines the class TSpan and its member functions.                     */
/*                                                                         */
/* ------------------------------------------------------------------------*/

#ifndef TVISION_TSPAN_H
#define TVISION_TSPAN_H

template <class T>
class TSpan {

    // This is actually a generalization of TStringView for any kind of element
    // type (and without enforcing the 'const' qualifier).
    // It exists for compatibility with Borland C++ and because std::span (C++ 20)
    // may not be widely available yet.

    T _FAR *ptr;
    size_t len;

public:

    // These are defined inline because otherwise they trigger
    // a bug in Borland C++ when T is const.

    constexpr TSpan() :
        ptr(0),
        len(0)
    {
    }

    constexpr TSpan(T _FAR *first, size_t n) :
        ptr(first),
        len(n)
    {
    }

#ifndef __BORLANDC__
    constexpr TSpan(decltype(nullptr)) :
        TSpan()
    {
    }

    template<size_t N>
    constexpr TSpan(T (&array)[N]) :
        TSpan(array, N)
    {
    }
#endif

    constexpr operator TSpan<const T>() const
    {
        return TSpan<const T>(ptr, len);
    }

    constexpr T _FAR * data() const
    {
        return ptr;
    }

    constexpr size_t size() const
    {
        return len;
    }

    constexpr size_t size_bytes() const
    {
        return size()*sizeof(T);
    }

    constexpr Boolean empty() const
    {
        return Boolean( size() == 0 );
    }

    constexpr T _FAR & operator[](size_t pos) const
    {
        return ptr[pos];
    }

    constexpr T _FAR & front() const
    {
        return ptr[0];
    }

    constexpr T _FAR & back() const
    {
        return ptr[len - 1];
    }

    constexpr TSpan subspan(size_t pos) const
    {
        return TSpan<T>(ptr + pos, len - pos);
    }

    constexpr TSpan subspan(size_t pos, size_t n) const
    {
        return TSpan<T>(ptr + pos, n <= len - pos ? n : len - pos);
    }

    constexpr T _FAR * begin() const
    {
        return &ptr[0];
    }

    constexpr const T _FAR * cbegin() const
    {
        return &ptr[0];
    }

    constexpr T _FAR * end() const
    {
        return &ptr[len];
    }

    constexpr const T _FAR * cend() const
    {
        return &ptr[len];
    }

};

#endif // TVISION_TSPAN_H
