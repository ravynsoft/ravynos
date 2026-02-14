#include <TargetConditionals.h>
#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
#include <wchar.h>
#include <wctype.h>
#include <xlocale.h>

#include <darwintest.h>

#if TARGET_OS_OSX
T_DECL(locale_PR_23679075, "converts a cyrillic a to uppercase")
{
	locale_t loc = newlocale(LC_COLLATE_MASK|LC_CTYPE_MASK, "ru_RU", 0);
	T_ASSERT_NOTNULL(loc, "newlocale(LC_COLLATE_MASK|LC_CTYPE_MASK, \"ru_RU\", 0) should return a locale");

	T_ASSERT_EQ(towupper_l(0x0430, loc), 0x0410, NULL);
	freelocale(loc);
}

T_DECL(locale_PR_24165555, "swprintf with Russian chars")
{
    setlocale(LC_ALL, "ru_RU.UTF-8");

    wchar_t buffer[256];
    T_EXPECT_POSIX_SUCCESS(swprintf(buffer, 256, L"%ls", L"English: Hello World"), "English");
    T_EXPECT_POSIX_SUCCESS(swprintf(buffer, 256, L"%ls", L"Russian: ру́сский язы́к"), "Russian");

    setlocale(LC_ALL, "");
}

T_DECL(locale_PR_28774201, "return code on bad locale")
{
    T_EXPECT_NULL(newlocale(LC_COLLATE_MASK | LC_CTYPE_MASK, "foobar", NULL), NULL);
    T_EXPECT_EQ(errno, ENOENT, NULL);
}
#endif
