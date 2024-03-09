#define Uses_TScreenCell
#include <tvision/tv.h>

#include <type_traits>

#include <test.h>

#define check_trivial_size_align(T, size, align) \
    EXPECT_EQ(std::is_trivial<T>(), true); \
    EXPECT_EQ(sizeof(T), size); \
    EXPECT_EQ(alignof(T), align);

TEST(Scrncell, StructsShouldBeTrivialAndHaveTheExpectedAlignmentAndSize)
{
    // Use 'alignof(type)' rather than hardcoding the alignment values,
    // given that they may vary among systems.
    check_trivial_size_align(TColorDesired, 4,  alignof(uint32_t));
    check_trivial_size_align(TColorAttr,    8,  alignof(uint64_t));
    check_trivial_size_align(TAttrPair,     16, alignof(uint64_t));
    check_trivial_size_align(TCellChar,     16, alignof(uint8_t));
    check_trivial_size_align(TScreenCell,   24, alignof(uint64_t));
    check_trivial_size_align(TColorBIOS,    1,  alignof(uint8_t));
    check_trivial_size_align(TColorRGB,     4,  alignof(uint32_t));
    check_trivial_size_align(TColorXTerm,   1,  alignof(uint8_t));
}
