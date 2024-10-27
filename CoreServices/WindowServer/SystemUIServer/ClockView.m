/*
 * Copyright (C) 2022-2024 Zoe Knox <zoe@pixin.net>
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

#include <pthread.h>
#include <unistd.h>
#include <locale.h>
#import <AppKit/AppKit.h>
#import "desktop.h"

const NSString *PrefsDateFormatStringKey = @"DateFormatString";
const NSString *defaultFormatEN = @"%a %b %d  %I:%M %p";
pthread_mutex_t mtx;

@implementation ClockView
- initWithFrame:(NSRect)frame {
    NSUserDefaults *prefs = [NSUserDefaults standardUserDefaults];
    dateFormat = [prefs stringForKey:PrefsDateFormatStringKey];
    if(dateFormat == nil || [dateFormat length] == 0) {
        NSString *locale = [[NSLocale currentLocale] localeIdentifier];
        if([locale hasPrefix:@"en"])
            dateFormat = defaultFormatEN;
        else if([locale hasPrefix:@"C"])
            dateFormat = @"%Y-%m-%d %R";
        else
            dateFormat = [prefs objectForKey:NSTimeDateFormatString];
    }
    dateFormatter = [[NSDateFormatter alloc] initWithDateFormat:dateFormat 
        allowNaturalLanguage:YES locale:[NSLocale currentLocale]];

    NSFont *font = [NSFont systemFontOfSize:15];
    attributes = [NSDictionary
        dictionaryWithObjects:@[font, [NSColor blackColor]]
                      forKeys:@[NSFontAttributeName, NSForegroundColorAttributeName]];

    dateString = [[NSAttributedString alloc]
        initWithString:[self currentDateValue] attributes:attributes];

    NSSize sz = [dateString size];
    sz.width += menuBarHPad;
    self = [super initWithFrame:NSMakeRect(frame.size.width - sz.width, menuBarVPad,
            sz.width + menuBarHPad, sz.height)];
    sz.width += menuBarHPad;

    pthread_mutex_init(&mtx, NULL);
    [NSTimer scheduledTimerWithTimeInterval:1
                                     target:self
                                   selector:@selector(notifyTick:)
                                   userInfo:nil
                                    repeats:YES];

    return self;
}

- (NSString *)currentDateValue {
    return [dateFormatter stringForObjectValue:[NSDate date]];
}

- (void)notifyTick:(id)arg {
    static NSString *_dateValue = NULL;
    dateString = [[NSAttributedString alloc] initWithString:[self currentDateValue] attributes:attributes];
    if([_dateValue isEqualToString:[self currentDateValue]] == NO) {
        [[NSColor windowBackgroundColor] set];
        NSRectFill(_frame);
        [dateString drawInRect:_frame];
        _dateValue = [self currentDateValue];
    }
}

- (NSSize)size {
    return _frame.size;
}

- (BOOL)refusesFirstResponder {
	return YES;
}

@end

