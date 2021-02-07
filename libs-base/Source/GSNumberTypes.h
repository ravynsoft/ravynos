/**
 * GSNumberTypes expects the INTEGER_MACRO macro to be defined.  This macro is
 * invoked once for every type and its Objective-C name.  Use this file when
 * implementing things like the -unsignedIntValue family of methods.  For this
 * case, the macro will be invoked with unsigned int as the type and
 * unsignedInt as the name.
 *
 */
#ifndef INTEGER_MACRO
#error Define INTEGER_MACRO(encoding, type, name, capitalizedName) before including GSNumberTypes.h
#endif
INTEGER_MACRO('d', double, double, Double)
INTEGER_MACRO('f', float, float, Float)
INTEGER_MACRO('c', signed char, char, Char)
INTEGER_MACRO('i', int, int, Int)
INTEGER_MACRO('s', short, short, Short)
INTEGER_MACRO('l', long, long, Long)
#ifndef NO_NSNUMBER
#	if SIZEOF_VOIDP == 4
INTEGER_MACRO('i', NSInteger, integer, Integer)
INTEGER_MACRO('I', NSUInteger, unsignedInteger, UnsignedInteger)
#	undef NO_NSNUMBER
#	else
INTEGER_MACRO('q', NSInteger, integer, Integer)
INTEGER_MACRO('Q', NSUInteger, unsignedInteger, UnsignedInteger)
#	endif
#endif
INTEGER_MACRO('q', long long, longLong, LongLong)
INTEGER_MACRO('C', unsigned char, unsignedChar, UnsignedChar)
INTEGER_MACRO('S', unsigned short, unsignedShort, UnsignedShort)
INTEGER_MACRO('I', unsigned int, unsignedInt, UnsignedInt)
INTEGER_MACRO('L', unsigned long, unsignedLong, UnsignedLong)
INTEGER_MACRO('Q', unsigned long long, unsignedLongLong, UnsignedLongLong) 
#undef INTEGER_MACRO
