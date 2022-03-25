#ifdef PLATFORM_IS_POSIX
#import "NSLocale_posix.h"
#import <Foundation/NSString.h>
#import <Foundation/NSNumber.h>
#import <Foundation/NSMutableDictionary.h>


@implementation NSLocale(posix)

BOOL NSCurrentLocaleIsMetric(){
   // FIXME
   return NO;
}

+(NSDictionary *)_platformLocaleAdditionalDescriptionForIdentifier:(NSString *)string
{
	NSMutableDictionary *desc = [NSMutableDictionary dictionary];
	
#if 0 // FIXME:
	NSNumber *n = [sLocales objectForKey:string];
	if (n) {
		LCID lcid = [n unsignedLongValue];
		// Gets the metrics info
		uint16_t metrics[2];
		int size=GetLocaleInfoW(lcid,LOCALE_IMEASURE,metrics,sizeof(metrics)/sizeof(metrics[0]));
		if (size) {
			[desc setObject:[NSNumber numberWithBool:metrics[0] == '0'] forKey:NSLocaleUsesMetricSystem];
		}
		
		// Get the decimal separator
		uint16_t decimal[4];
		size=GetLocaleInfoW(lcid,LOCALE_SDECIMAL,decimal,sizeof(decimal)/sizeof(decimal[0]));
		if (size) {
			[desc setObject:[NSString stringWithFormat:@"%S", decimal] forKey:NSLocaleDecimalSeparator];
		}
		
		// Get the currency symbol
		uint16_t currency[19];
		size=GetLocaleInfoW(lcid,LOCALE_SCURRENCY,currency,sizeof(currency)/sizeof(currency[0]));
		if (size) {
			[desc setObject:[NSString stringWithFormat:@"%S", currency] forKey:NSLocaleCurrencySymbol];
		}
		// TODO - gets date format, etc.
	}
#endif
	return desc;
}

+(NSString *)_platformCurrentLocaleIdentifier {
    if(getenv("LANG") != NULL) {
        NSString *s = [NSString stringWithCString:getenv("LANG")];
        if([s length] >= 5)
            return [s substringToIndex:5];
    }
    return [NSString stringWithString:@"de_DE"];
}

@end
#endif

