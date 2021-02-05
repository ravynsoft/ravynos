#ifdef WINDOWS
#import "NSLocale+windows.h"
#import <Foundation/NSString.h>
#import <Foundation/NSNumber.h>
#import <Foundation/NSMutableDictionary.h>

#include <windows.h>

@implementation NSLocale(windows)

static NSMutableDictionary *sLocales = nil;

static BOOL CALLBACK enumLocalesProc(LPTSTR lpLocaleString)
{
	// Someone at MS thought that passing a int as a string is a good idea
	LCID lcid = 0;
	sscanf(lpLocaleString, "%x", &lcid); 
	
	// Gets the ISO lang and coutry for that locale and store the ISO<->LCID association
	uint8_t langISO[9];
	GetLocaleInfo(lcid,LOCALE_SISO639LANGNAME,langISO,sizeof(langISO)/sizeof(langISO[0]));
	uint8_t countryISO[9];
	GetLocaleInfo(lcid,LOCALE_SISO3166CTRYNAME,countryISO,sizeof(countryISO)/sizeof(countryISO[0]));
	
	[sLocales setObject:[NSNumber numberWithUnsignedLong:lcid] forKey: [NSString stringWithFormat:@"%s_%s", langISO, countryISO]];
	return YES;
}

BOOL NSCurrentLocaleIsMetric(){
   uint16_t buffer[2];
   int       size=GetLocaleInfoW(LOCALE_USER_DEFAULT,LOCALE_IMEASURE,buffer,2);

   if(buffer[0]=='0')
    return YES;
   
   return NO;
}

+(NSDictionary *)_platformLocaleAdditionalDescriptionForIdentifier:(NSString *)string
{
	@synchronized(self) {
		if (sLocales == nil) {
			// Get all of the installed locales ISO names & LCID
			sLocales = [[NSMutableDictionary alloc] initWithCapacity:50];
			EnumSystemLocales((LOCALE_ENUMPROC)enumLocalesProc, LCID_INSTALLED);
		}
	}
	NSMutableDictionary *desc = [NSMutableDictionary dictionary];
	
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
	return desc;
}

+(NSString *)_platformCurrentLocaleIdentifier {
	uint8_t langISO[9];
	GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_SISO639LANGNAME,langISO,sizeof(langISO)/sizeof(langISO[0]));
	uint8_t countryISO[9];
	GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_SISO3166CTRYNAME,countryISO,sizeof(countryISO)/sizeof(countryISO[0]));
	
	return [NSString stringWithFormat:@"%s_%s", langISO, countryISO];
}

@end
#endif

