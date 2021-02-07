#if     defined(GNUSTEP_BASE_LIBRARY)
#import <Foundation/Foundation.h>
#import <GNUstepBase/GSMime.h>
#import "Testing.h"

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];

  // Test charset conversions.
  PASS([GSMimeDocument encodingFromCharset: @"ansi_x3.4-1968"]
    == NSASCIIStringEncoding,
    "charset 'ansi_x3.4-1968' is NSASCIIStringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"ansi_x3.4-1986"]
    == NSASCIIStringEncoding,
    "charset 'ansi_x3.4-1986' is NSASCIIStringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"apple-roman"]
    == NSMacOSRomanStringEncoding,
    "charset 'apple-roman' is NSMacOSRomanStringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"ascii"]
    == NSASCIIStringEncoding,
    "charset 'ascii' is NSASCIIStringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"big5"]
    == NSBIG5StringEncoding,
    "charset 'big5' is NSBIG5StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"cp367"]
    == NSASCIIStringEncoding,
    "charset 'cp367' is NSASCIIStringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"cp819"]
    == NSISOLatin1StringEncoding,
    "charset 'cp819' is NSISOLatin1StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"csascii"]
    == NSASCIIStringEncoding,
    "charset 'csascii' is NSASCIIStringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"csisolatin1"]
    == NSISOLatin1StringEncoding,
    "charset 'csisolatin1' is NSISOLatin1StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"gb2312.1980"]
    == NSGB2312StringEncoding,
    "charset 'gb2312.1980' is NSGB2312StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"gsm0338"]
    == NSGSM0338StringEncoding,
    "charset 'gsm0338' is NSGSM0338StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"ia5"]
    == NSASCIIStringEncoding,
    "charset 'ia5' is NSASCIIStringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"ibm367"]
    == NSASCIIStringEncoding,
    "charset 'ibm367' is NSASCIIStringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"ibm819"]
    == NSISOLatin1StringEncoding,
    "charset 'ibm819' is NSISOLatin1StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso-10646-ucs-2"]
    == NSUnicodeStringEncoding,
    "charset 'iso-10646-ucs-2' is NSUnicodeStringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso10646-ucs-2"]
    == NSUnicodeStringEncoding,
    "charset 'iso10646-ucs-2' is NSUnicodeStringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso-8859-1"]
    == NSISOLatin1StringEncoding,
    "charset 'iso-8859-1' is NSISOLatin1StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso8859-1"]
    == NSISOLatin1StringEncoding,
    "charset 'iso8859-1' is NSISOLatin1StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso-8859-10"]
    == NSISOLatin6StringEncoding,
    "charset 'iso-8859-10' is NSISOLatin6StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso8859-10"]
    == NSISOLatin6StringEncoding,
    "charset 'iso8859-10' is NSISOLatin6StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso-8859-11"]
    == NSISOThaiStringEncoding,
    "charset 'iso-8859-11' is NSISOThaiStringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso8859-11"]
    == NSISOThaiStringEncoding,
    "charset 'iso8859-11' is NSISOThaiStringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso-8859-13"]
    == NSISOLatin7StringEncoding,
    "charset 'iso-8859-13' is NSISOLatin7StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso8859-13"]
    == NSISOLatin7StringEncoding,
    "charset 'iso8859-13' is NSISOLatin7StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso-8859-14"]
    == NSISOLatin8StringEncoding,
    "charset 'iso-8859-14' is NSISOLatin8StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso8859-14"]
    == NSISOLatin8StringEncoding,
    "charset 'iso8859-14' is NSISOLatin8StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso-8859-15"]
    == NSISOLatin9StringEncoding,
    "charset 'iso-8859-15' is NSISOLatin9StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso8859-15"]
    == NSISOLatin9StringEncoding,
    "charset 'iso8859-15' is NSISOLatin9StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso-8859-1:1987"]
    == NSISOLatin1StringEncoding,
    "charset 'iso-8859-1:1987' is NSISOLatin1StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso8859-1:1987"]
    == NSISOLatin1StringEncoding,
    "charset 'iso8859-1:1987' is NSISOLatin1StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso-8859-2"]
    == NSISOLatin2StringEncoding,
    "charset 'iso-8859-2' is NSISOLatin2StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso8859-2"]
    == NSISOLatin2StringEncoding,
    "charset 'iso8859-2' is NSISOLatin2StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso-8859-3"]
    == NSISOLatin3StringEncoding,
    "charset 'iso-8859-3' is NSISOLatin3StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso8859-3"]
    == NSISOLatin3StringEncoding,
    "charset 'iso8859-3' is NSISOLatin3StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso-8859-4"]
    == NSISOLatin4StringEncoding,
    "charset 'iso-8859-4' is NSISOLatin4StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso8859-4"]
    == NSISOLatin4StringEncoding,
    "charset 'iso8859-4' is NSISOLatin4StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso-8859-5"]
    == NSISOCyrillicStringEncoding,
    "charset 'iso-8859-5' is NSISOCyrillicStringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso8859-5"]
    == NSISOCyrillicStringEncoding,
    "charset 'iso8859-5' is NSISOCyrillicStringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso-8859-6"]
    == NSISOArabicStringEncoding,
    "charset 'iso-8859-6' is NSISOArabicStringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso8859-6"]
    == NSISOArabicStringEncoding,
    "charset 'iso8859-6' is NSISOArabicStringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso-8859-7"]
    == NSISOGreekStringEncoding,
    "charset 'iso-8859-7' is NSISOGreekStringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso8859-7"]
    == NSISOGreekStringEncoding,
    "charset 'iso8859-7' is NSISOGreekStringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso-8859-8"]
    == NSISOHebrewStringEncoding,
    "charset 'iso-8859-8' is NSISOHebrewStringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso8859-8"]
    == NSISOHebrewStringEncoding,
    "charset 'iso8859-8' is NSISOHebrewStringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso-8859-9"]
    == NSISOLatin5StringEncoding,
    "charset 'iso-8859-9' is NSISOLatin5StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso8859-9"]
    == NSISOLatin5StringEncoding,
    "charset 'iso8859-9' is NSISOLatin5StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso-ir-100"]
    == NSISOLatin1StringEncoding,
    "charset 'iso-ir-100' is NSISOLatin1StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso-ir-6"]
    == NSASCIIStringEncoding,
    "charset 'iso-ir-6' is NSASCIIStringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso-10646-1"]
    == NSUnicodeStringEncoding,
    "charset 'iso-10646-1' is NSUnicodeStringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso10646-1"]
    == NSUnicodeStringEncoding,
    "charset 'iso10646-1' is NSUnicodeStringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso646-us"]
    == NSASCIIStringEncoding,
    "charset 'iso646-us' is NSASCIIStringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso_646.991-irv"]
    == NSASCIIStringEncoding,
    "charset 'iso_646.991-irv' is NSASCIIStringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso_646.irv:1991"]
    == NSASCIIStringEncoding,
    "charset 'iso_646.irv:1991' is NSASCIIStringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"iso_8859-1"]
    == NSISOLatin1StringEncoding,
    "charset 'iso_8859-1' is NSISOLatin1StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"jisx0201.1976"]
    == NSShiftJISStringEncoding,
    "charset 'jisx0201.1976' is NSShiftJISStringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"koi8-r"]
    == NSKOI8RStringEncoding,
    "charset 'koi8-r' is NSKOI8RStringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"ksc5601.1987"]
    == NSKoreanEUCStringEncoding,
    "charset 'ksc5601.1987' is NSKoreanEUCStringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"ksc5601.1997"]
    == NSKoreanEUCStringEncoding,
    "charset 'ksc5601.1997' is NSKoreanEUCStringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"l1"]
    == NSISOLatin1StringEncoding,
    "charset 'l1' is NSISOLatin1StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"latin1"]
    == NSISOLatin1StringEncoding,
    "charset 'latin1' is NSISOLatin1StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"microsoft-cp1250"]
    == NSWindowsCP1250StringEncoding,
    "charset 'microsoft-cp1250' is NSWindowsCP1250StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"microsoft-cp1251"]
    == NSWindowsCP1251StringEncoding,
    "charset 'microsoft-cp1251' is NSWindowsCP1251StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"microsoft-cp1252"]
    == NSWindowsCP1252StringEncoding,
    "charset 'microsoft-cp1252' is NSWindowsCP1252StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"microsoft-cp1253"]
    == NSWindowsCP1253StringEncoding,
    "charset 'microsoft-cp1253' is NSWindowsCP1253StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"microsoft-cp1254"]
    == NSWindowsCP1254StringEncoding,
    "charset 'microsoft-cp1254' is NSWindowsCP1254StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"microsoft-symbol"]
    == NSSymbolStringEncoding,
    "charset 'microsoft-symbol' is NSSymbolStringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"shift_JIS"]
    == NSShiftJISStringEncoding,
    "charset 'shift_JIS' is NSShiftJISStringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"us"]
    == NSASCIIStringEncoding,
    "charset 'us' is NSASCIIStringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"us-ascii"]
    == NSASCIIStringEncoding,
    "charset 'us-ascii' is NSASCIIStringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"utf-16"]
    == NSUnicodeStringEncoding,
    "charset 'utf-16' is NSUnicodeStringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"utf16"]
    == NSUnicodeStringEncoding,
    "charset 'utf16' is NSUnicodeStringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"utf-7"]
    == NSUTF7StringEncoding,
    "charset 'utf-7' is NSUTF7StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"utf7"]
    == NSUTF7StringEncoding,
    "charset 'utf7' is NSUTF7StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"utf-8"]
    == NSUTF8StringEncoding,
    "charset 'utf-8' is NSUTF8StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"utf8"]
    == NSUTF8StringEncoding,
    "charset 'utf8' is NSUTF8StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"windows-1250"]
    == NSWindowsCP1250StringEncoding,
    "charset 'windows-1250' is NSWindowsCP1250StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"windows-1251"]
    == NSWindowsCP1251StringEncoding,
    "charset 'windows-1251' is NSWindowsCP1251StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"windows-1252"]
    == NSWindowsCP1252StringEncoding,
    "charset 'windows-1252' is NSWindowsCP1252StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"windows-1253"]
    == NSWindowsCP1253StringEncoding,
    "charset 'windows-1253' is NSWindowsCP1253StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"windows-1254"]
    == NSWindowsCP1254StringEncoding,
    "charset 'windows-1254' is NSWindowsCP1254StringEncoding");
  PASS([GSMimeDocument encodingFromCharset: @"windows-symbol"]
    == NSSymbolStringEncoding,
    "charset 'windows-symbol' is NSSymbolStringEncoding");

  // Test canonical charset names.

  PASS([[GSMimeDocument charsetFromEncoding: NSASCIIStringEncoding]
    isEqualToString: @"us-ascii"],
    "NSASCIIStringEncoding canonical charset is us-ascii");
  PASS([[GSMimeDocument charsetFromEncoding: NSBIG5StringEncoding]
    isEqualToString: @"big5"],
    "NSBIG5StringEncoding canonical charset is big5");
  PASS([[GSMimeDocument charsetFromEncoding: NSGB2312StringEncoding]
    isEqualToString: @"gb2312.1980"],
    "NSGB2312StringEncoding canonical charset is gb2312.1980");
  PASS([[GSMimeDocument charsetFromEncoding: NSGSM0338StringEncoding]
    isEqualToString: @"gsm0338"],
    "NSGSM0338StringEncoding canonical charset is gsm0338");
  PASS([[GSMimeDocument charsetFromEncoding: NSISOArabicStringEncoding]
    isEqualToString: @"iso-8859-6"],
    "NSISOArabicStringEncoding canonical charset is iso-8859-6");
  PASS([[GSMimeDocument charsetFromEncoding: NSISOCyrillicStringEncoding]
    isEqualToString: @"iso-8859-5"],
    "NSISOCyrillicStringEncoding canonical charset is iso-8859-5");
  PASS([[GSMimeDocument charsetFromEncoding: NSISOGreekStringEncoding]
    isEqualToString: @"iso-8859-7"],
    "NSISOGreekStringEncoding canonical charset is iso-8859-7");
  PASS([[GSMimeDocument charsetFromEncoding: NSISOHebrewStringEncoding]
    isEqualToString: @"iso-8859-8"],
    "NSISOHebrewStringEncoding canonical charset is iso-8859-8");
  PASS([[GSMimeDocument charsetFromEncoding: NSISOLatin1StringEncoding]
    isEqualToString: @"iso-8859-1"],
    "NSISOLatin1StringEncoding canonical charset is iso-8859-1");
  PASS([[GSMimeDocument charsetFromEncoding: NSISOLatin2StringEncoding]
    isEqualToString: @"iso-8859-2"],
    "NSISOLatin2StringEncoding canonical charset is iso-8859-2");
  PASS([[GSMimeDocument charsetFromEncoding: NSISOLatin3StringEncoding]
    isEqualToString: @"iso-8859-3"],
    "NSISOLatin3StringEncoding canonical charset is iso-8859-3");
  PASS([[GSMimeDocument charsetFromEncoding: NSISOLatin4StringEncoding]
    isEqualToString: @"iso-8859-4"],
    "NSISOLatin4StringEncoding canonical charset is iso-8859-4");
  PASS([[GSMimeDocument charsetFromEncoding: NSISOLatin5StringEncoding]
    isEqualToString: @"iso-8859-9"],
    "NSISOLatin5StringEncoding canonical charset is iso-8859-9");
  PASS([[GSMimeDocument charsetFromEncoding: NSISOLatin6StringEncoding]
    isEqualToString: @"iso-8859-10"],
    "NSISOLatin6StringEncoding canonical charset is iso-8859-10");
  PASS([[GSMimeDocument charsetFromEncoding: NSISOLatin7StringEncoding]
    isEqualToString: @"iso-8859-13"],
    "NSISOLatin7StringEncoding canonical charset is iso-8859-13");
  PASS([[GSMimeDocument charsetFromEncoding: NSISOLatin8StringEncoding]
    isEqualToString: @"iso-8859-14"],
    "NSISOLatin8StringEncoding canonical charset is iso-8859-14");
  PASS([[GSMimeDocument charsetFromEncoding: NSISOLatin9StringEncoding]
    isEqualToString: @"iso-8859-15"],
    "NSISOLatin9StringEncoding canonical charset is iso-8859-15");
  PASS([[GSMimeDocument charsetFromEncoding: NSISOThaiStringEncoding]
    isEqualToString: @"iso-8859-11"],
    "NSISOThaiStringEncoding canonical charset is iso-8859-11");
  PASS([[GSMimeDocument charsetFromEncoding: NSKOI8RStringEncoding]
    isEqualToString: @"koi8-r"],
    "NSKOI8RStringEncoding canonical charset is koi8-r");
  PASS([[GSMimeDocument charsetFromEncoding: NSKoreanEUCStringEncoding]
    isEqualToString: @"ksc5601.1987"],
    "NSKoreanEUCStringEncoding canonical charset is ksc5601.1987");
  PASS([[GSMimeDocument charsetFromEncoding: NSMacOSRomanStringEncoding]
    isEqualToString: @"apple-roman"],
    "NSMacOSRomanStringEncoding canonical charset is apple-roman");
  PASS([[GSMimeDocument charsetFromEncoding: NSShiftJISStringEncoding]
    isEqualToString: @"shift_JIS"],
    "NSShiftJISStringEncoding canonical charset is shift_JIS");
  PASS([[GSMimeDocument charsetFromEncoding: NSUTF7StringEncoding]
    isEqualToString: @"utf-7"],
    "NSUTF7StringEncoding canonical charset is utf-7");
  PASS([[GSMimeDocument charsetFromEncoding: NSUTF8StringEncoding]
    isEqualToString: @"utf-8"],
    "NSUTF8StringEncoding canonical charset is utf-8");
  PASS([[GSMimeDocument charsetFromEncoding: NSUnicodeStringEncoding]
    isEqualToString: @"utf-16"],
    "NSUnicodeStringEncoding canonical charset is utf-16");
  PASS([[GSMimeDocument charsetFromEncoding: NSWindowsCP1250StringEncoding]
    isEqualToString: @"windows-1250"],
    "NSWindowsCP1250StringEncoding canonical charset is windows-1250");
  PASS([[GSMimeDocument charsetFromEncoding: NSWindowsCP1251StringEncoding]
    isEqualToString: @"windows-1251"],
    "NSWindowsCP1251StringEncoding canonical charset is windows-1251");
  PASS([[GSMimeDocument charsetFromEncoding: NSWindowsCP1252StringEncoding]
    isEqualToString: @"windows-1252"],
    "NSWindowsCP1252StringEncoding canonical charset is windows-1252");
  PASS([[GSMimeDocument charsetFromEncoding: NSWindowsCP1253StringEncoding]
    isEqualToString: @"windows-1253"],
    "NSWindowsCP1253StringEncoding canonical charset is windows-1253");
  PASS([[GSMimeDocument charsetFromEncoding: NSWindowsCP1254StringEncoding]
    isEqualToString: @"windows-1254"],
    "NSWindowsCP1254StringEncoding canonical charset is windows-1254");
  [arp release]; arp = nil;
  return 0;
}
#else
int main(int argc,char **argv)
{
  return 0;
}
#endif
