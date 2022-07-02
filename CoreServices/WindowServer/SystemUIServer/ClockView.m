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

#import <AppKit/AppKit.h>
#import "desktop.h"

const NSString *PrefsDateFormatStringKey = @"DateFormatString";
const NSString *defaultFormatEN = @"%a %b %d  %I:%M %p";

@implementation ClockView
- initWithFrame:(NSRect)frame {
    NSUserDefaults *prefs = [NSUserDefaults standardUserDefaults];
    dateFormat = [prefs stringForKey:PrefsDateFormatStringKey];
    if(dateFormat == nil || [dateFormat length] == 0) {
        NSString *locale = [[NSLocale currentLocale] localeIdentifier];
        if([locale hasPrefix:@"en"])
            dateFormat = defaultFormatEN;
        else
            dateFormat = [prefs objectForKey:NSTimeDateFormatString];
    }
    dateFormatter = [[NSDateFormatter alloc] initWithDateFormat:dateFormat 
        allowNaturalLanguage:YES locale:[NSLocale currentLocale]];

    NSFont *font = [NSFont systemFontOfSize:15];
    attributes = [NSDictionary dictionaryWithObject:font forKey:NSFontAttributeName];

    NSAttributedString *dateString = [[NSAttributedString alloc]
        initWithString:[dateFormatter stringForObjectValue:[NSDate date]]
        attributes:attributes];

    NSSize sz = [dateString size];
    self = [super initWithText:dateString
        atPoint:NSMakePoint(frame.size.width - sz.width - menuBarHPad, menuBarVPad)
        withMaxWidth:300];
    [self setFont:font];

    [NSThread detachNewThreadSelector:@selector(notifyTick:) toTarget:self withObject:nil];

    return self;
}

- (NSString *)currentDateValue {
    return [dateFormatter stringForObjectValue:[NSDate date]];
}

- (void)notifyTick:(id)arg {
    while(1) {
        [[NSNotificationCenter defaultCenter] postNotificationName:@"ClockTick" object:nil userInfo:NULL];
        usleep(400000);
    }
}

@end

