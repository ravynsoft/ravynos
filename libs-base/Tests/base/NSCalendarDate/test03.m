/*

 * DateFormatTest.m - demonstrate [NSCalendarDate descriptionWithCalendarFormat] hanging behaviour with some values.
 *
 *  Created on: Mar 15, 2016
 */

#import <Foundation/Foundation.h>
#import "Testing.h"

#define ONE_SECOND	((double)1 / (24*60*60))

static NSString *
formattedDaysSince1970(double daysSince1970)
{
  NSCalendarDate *calendarDate;
  NSTimeZone *tz;
  NSString *formattedDate;
  double secondsSinceReference;

  // Convert offset in "days from 1970" to offset in seconds
  // from Reference date (from 01-Jan-2001).
  secondsSinceReference
    = floor (daysSince1970 / ONE_SECOND - NSTimeIntervalSince1970 + 0.5);

  printf ("daysSince1970: %.18g. secondsSinceReference: %.18g.\n",
    daysSince1970, secondsSinceReference);

  calendarDate = [[NSCalendarDate alloc]
    initWithTimeIntervalSinceReferenceDate: secondsSinceReference];
  tz = [NSTimeZone timeZoneWithName: @"GMT"];
  [calendarDate setTimeZone: tz];

  formattedDate = [calendarDate descriptionWithCalendarFormat: @"%d-%m-%Y"];
  RELEASE(calendarDate);
  return formattedDate;
}

int main(void)
{
  CREATE_AUTORELEASE_POOL(arp);

  if (sizeof(NSInteger) == 4)
    {
      PASS_EQUAL(formattedDaysSince1970(8640000000), @"02-01-4001",
       "format date for 8640000000");

      PASS_EQUAL(formattedDaysSince1970(2147483651), @"02-01-4001",
       "format date for 2147483651");
    }
  else
    {
      PASS_EQUAL(formattedDaysSince1970(8640000000), @"17-07-23657486",
       "format date for 8640000000");

      PASS_EQUAL(formattedDaysSince1970(2147483651), @"15-07-5881580",
       "format date for 2147483651");
    }
  DESTROY(arp);
  return 0;
}

