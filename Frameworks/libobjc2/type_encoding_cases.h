/** 
 * type_encoding_cases.h -  expects the APPLY_TYPE macro to be defined.  This
 * macro is invoked once for every type and its Objective-C name.  Use this
 * file when implementing things like the -unsignedIntValue family of methods.
 * For this case, the macro will be invoked with unsigned int as the type and
 * unsignedInt as the name.
 */
#ifndef APPLY_TYPE
#error Define APPLY_TYPE(type, name, capitalizedName, encodingChar) before including this file
#endif
APPLY_TYPE(long double, long double, LongDouble, 'D')
APPLY_TYPE(double, double, Double, 'd')
APPLY_TYPE(float, float, Float, 'f')
APPLY_TYPE(signed char, char, Char, 'c')
APPLY_TYPE(int, int, Int, 'i')
APPLY_TYPE(short, short, Short, 's')
APPLY_TYPE(long, long, Long, 'l')
APPLY_TYPE(long long, longLong, LongLong, 'q')
//APPLY_TYPE(__int128, int128, Int128, 't')
APPLY_TYPE(unsigned char, unsignedChar, UnsignedChar, 'C')
APPLY_TYPE(unsigned short, unsignedShort, UnsignedShort, 'S')
APPLY_TYPE(unsigned int, unsignedInt, UnsignedInt, 'I')
APPLY_TYPE(unsigned long, unsignedLong, UnsignedLong, 'L')
APPLY_TYPE(unsigned long long, unsignedLongLong, UnsignedLongLong, 'Q')
//APPLY_TYPE(unsigned __int128, unsignedInt128, UnsignedInt128, 'T')
#ifdef NON_INTEGER_TYPES
#undef NON_INTEGER_TYPES
APPLY_TYPE(_Bool, bool, Bool, 'B')
#ifndef SKIP_ID
APPLY_TYPE(id, object, Object, '@')
#endif
APPLY_TYPE(Class, class, Class, '#')
APPLY_TYPE(SEL, selector, Selector, ':')
APPLY_TYPE(char*, cString, CString, '*')
#endif
#undef APPLY_TYPE
