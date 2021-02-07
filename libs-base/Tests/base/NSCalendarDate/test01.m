#import "Testing.h"
#import <Foundation/NSCalendarDate.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSString.h>

#include "./western.h"

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSTimeInterval time1, time2, time3, time4, time5, time6, time7, time8, time9;
  NSCalendarDate *date1;
  NSDictionary *locale;

  locale = westernLocale();

  time1 = [[NSCalendarDate dateWithString: @"Nov 20 02 01:54:22"
                           calendarFormat: @"%b %d %y %H:%M:%S"
				   locale: locale] 
			          timeIntervalSinceReferenceDate]; 
  time2 = [[NSCalendarDate dateWithString: @"Nov 20 02 02:54:22"
                           calendarFormat: @"%b %d %y %H:%M:%S"
				   locale: locale] 
			          timeIntervalSinceReferenceDate]; 
  time3 = [[NSCalendarDate dateWithString: @"Nov 20 02 03:54:22"
                           calendarFormat: @"%b %d %y %H:%M:%S"
				   locale: locale]
			          timeIntervalSinceReferenceDate]; 
  time4 = [[NSCalendarDate dateWithString: @"Nov 20 02 04:54:22"
                           calendarFormat: @"%b %d %y %H:%M:%S"
				   locale: locale]
			          timeIntervalSinceReferenceDate]; 
  time5 = [[NSCalendarDate dateWithString: @"Nov 20 02 05:54:22"
                           calendarFormat: @"%b %d %y %H:%M:%S"
				   locale: locale] 
			          timeIntervalSinceReferenceDate]; 
  time6 = [[NSCalendarDate dateWithString: @"Nov 20 02 06:54:22"
                           calendarFormat: @"%b %d %y %H:%M:%S"
				   locale: locale] 
			          timeIntervalSinceReferenceDate]; 
  time7 = [[NSCalendarDate dateWithString: @"Nov 20 02 07:54:22"
                           calendarFormat: @"%b %d %y %H:%M:%S"
				   locale: locale] 
			          timeIntervalSinceReferenceDate]; 
  time8 = [[NSCalendarDate dateWithString: @"Nov 20 02 08:54:22"
                           calendarFormat: @"%b %d %y %H:%M:%S"
				   locale: locale]
			          timeIntervalSinceReferenceDate]; 
  time9 = [[NSCalendarDate dateWithString: @"Nov 20 02 09:54:22"
                           calendarFormat: @"%b %d %y %H:%M:%S"
				   locale: locale] 
			          timeIntervalSinceReferenceDate]; 
 
  PASS ((time1 < time2 && time2 < time3 && time3 < time4 && time4 < time5
    && time5 < time6 && time6 < time7 && time7 < time8 && time8 < time9),
    "+dateWithString:calendarFormat: works if no time zone is specified");
  
  date1 = [NSCalendarDate dateWithString: @"Nov 29 06 12:00am" 
                          calendarFormat: @"%b %d %y %H:%M%p"
				  locale: locale]; 
  PASS(date1 != nil && [date1 hourOfDay] == 0, "12:00am is midnight");

  date1 = [NSCalendarDate dateWithString: @"Nov 29 06 12:00pm" 
                          calendarFormat: @"%b %d %y %H:%M%p"
				  locale: locale]; 
  PASS(date1 != nil && [date1 hourOfDay] == 12, "12:00pm is noon");

  date1 = [NSCalendarDate dateWithString: @"Nov 29 06 01:25:38" 
                          calendarFormat: @"%b %d %y %H:%M:%S"
				  locale: locale]; 
  PASS([date1 timeIntervalSinceReferenceDate] + 1 == [[date1 addTimeInterval:1]
  						timeIntervalSinceReferenceDate],
       "-addTimeInterval: works on a NSCalendarDate parsed with no timezone");

  {
    NSString *fmt = @"%Y-%m-%d %H:%M:%S:%F";
    NSString *fmt2 = @"%Y-%m-%e %H:%M:%S:%F";
    NSString *dateString = @"2006-04-22 22:22:22:901";
    NSString *dateString2 = @"2006-04-2 22:22:22:901";
    NSCalendarDate *date = [NSCalendarDate 
      dateWithString:dateString calendarFormat:fmt locale:locale];
    NSCalendarDate *date2 = [NSCalendarDate 
      dateWithString:dateString2 calendarFormat:fmt2 locale:locale];
    NSLog(@"%@\n%@", dateString, [date descriptionWithCalendarFormat:fmt]);
    PASS([dateString isEqual: [date descriptionWithCalendarFormat:fmt]],
      "formatting milliseconds works");
    PASS([dateString2 isEqual: [date2 descriptionWithCalendarFormat:fmt2]],
      "formatting with %%e works");
  }

  [arp release]; arp = nil;
  return 0;
}
