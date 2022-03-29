/*
 * Copyright (C) 2022 Zoe Knox <zoe@pixin.net>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifdef PLATFORM_IS_POSIX
#import "NSLocale_posix.h"
#import <Foundation/NSString.h>
#import <Foundation/NSNumber.h>
#import <Foundation/NSMutableDictionary.h>
#import <Foundation/NSMutableArray.h>
#import <Foundation/NSUserDefaults.h>
#import <langinfo.h>
#import <xlocale.h>

@implementation NSLocale(posix)

BOOL NSCurrentLocaleIsMetric(NSString *locale){
    if([locale hasSuffix:@"_US"] || [locale hasSuffix:@"_LR"] || [locale hasSuffix:@"_MM"])
        return NO;
    return YES;
}

+(NSDictionary *)_platformLocaleAdditionalDescriptionForIdentifier:(NSString *)identifier
{
    NSMutableDictionary *desc = [NSMutableDictionary dictionary];

    char *currentLocale = strdup(setlocale(LC_ALL, NULL));
    if(setlocale(LC_ALL, [[identifier stringByAppendingString:@".UTF-8"]
        UTF8String]) != NULL) {

        struct lconv *conv = localeconv();
        [desc setObject:[NSString stringWithUTF8String:conv->decimal_point]
            forKey:NSLocaleDecimalSeparator];
        [desc setObject:[NSString stringWithUTF8String:conv->thousands_sep]
            forKey:NSLocaleGroupingSeparator];
        [desc setObject:[NSString stringWithUTF8String:conv->currency_symbol]
            forKey:NSLocaleCurrencySymbol];
        [desc setObject:[NSString stringWithUTF8String:conv->int_curr_symbol]
            forKey:NSLocaleCurrencyCode];
        [desc setObject:[NSNumber numberWithBool:NSCurrentLocaleIsMetric(identifier)]
            forKey:NSLocaleUsesMetricSystem];

        // long day names
        NSMutableArray *arr = [NSMutableArray arrayWithCapacity:7];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(DAY_1)]];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(DAY_2)]];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(DAY_3)]];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(DAY_4)]];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(DAY_5)]];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(DAY_6)]];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(DAY_7)]];
        [desc setObject:arr forKey:NSWeekDayNameArray];

        // short day names
        arr = [NSMutableArray arrayWithCapacity:7];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(ABDAY_1)]];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(ABDAY_2)]];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(ABDAY_3)]];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(ABDAY_4)]];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(ABDAY_5)]];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(ABDAY_6)]];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(ABDAY_7)]];
        [desc setObject:arr forKey:NSShortWeekDayNameArray];

        // long month names
        arr = [NSMutableArray arrayWithCapacity:12];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(MON_1)]];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(MON_2)]];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(MON_3)]];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(MON_4)]];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(MON_5)]];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(MON_6)]];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(MON_7)]];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(MON_8)]];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(MON_9)]];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(MON_10)]];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(MON_11)]];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(MON_12)]];
        [desc setObject:arr forKey:NSMonthNameArray];

        // short month names
        arr = [NSMutableArray arrayWithCapacity:12];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(ABMON_1)]];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(ABMON_2)]];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(ABMON_3)]];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(ABMON_4)]];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(ABMON_5)]];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(ABMON_6)]];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(ABMON_7)]];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(ABMON_8)]];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(ABMON_9)]];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(ABMON_10)]];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(ABMON_11)]];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(ABMON_12)]];
        [desc setObject:arr forKey:NSShortMonthNameArray];

        [desc setObject:[NSString stringWithUTF8String:nl_langinfo(T_FMT)]
            forKey:NSTimeFormatString];
        [desc setObject:[NSString stringWithUTF8String:nl_langinfo(D_FMT)]
            forKey:NSDateFormatString];
        [desc setObject:[NSString stringWithUTF8String:nl_langinfo(D_T_FMT)]
            forKey:NSTimeDateFormatString];

        arr = [NSMutableArray arrayWithCapacity:2];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(AM_STR)]];
        [arr addObject:[NSString stringWithUTF8String:nl_langinfo(PM_STR)]];
        [desc setObject:arr forKey:NSAMPMDesignation];

        // Restore the initial locale
        setlocale(LC_ALL, currentLocale);
    }
    free(currentLocale);
    return desc;
}

+(NSString *)_platformCurrentLocaleIdentifier {
    if(getenv("LANG") != NULL) {
        NSString *s = [NSString stringWithCString:getenv("LANG")];
        if([s length] >= 5)
            return [s substringToIndex:5];
    }
    return @"en_US";
}

@end
#endif

