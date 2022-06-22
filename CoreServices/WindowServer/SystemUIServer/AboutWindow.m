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
#import <Foundation/NSPlatform.h>
#import "desktop.h"
#import "AboutWindow.h"

#define LOGO_WIDTH 140
#define LOGO_HEIGHT 140
#define WIN_WIDTH 550
#define WIN_HEIGHT 300
#define V_SPACER 20
#define H_SPACER 15

static NSString *_formatAsGB(unsigned long bytes)
{
    double gb = (double)bytes;
    gb /=  (1024.0 * 1024.0 * 1024.0);
    return [NSString stringWithFormat:@"%.1f GB", gb];
}


@implementation AboutWindow
- (AboutWindow *)init {
    self = [super initWithContentRect:NSMakeRect(0,0,WIN_WIDTH,WIN_HEIGHT)
        styleMask:NSTitledWindowMask backing:NSBackingStoreBuffered defer:NO];
    [self setTitle:@"About This Computer"];

    NSImageView *iv = [[NSImageView alloc] initWithFrame:NSMakeRect(0, WIN_HEIGHT/2 - LOGO_HEIGHT/2,
        LOGO_WIDTH, LOGO_HEIGHT)];
    NSString *releaseLogo = [[NSBundle mainBundle] pathForResource:@"ReleaseLogo" ofType:@"tiff"];
    NSImage *img = [[NSImage alloc] initWithContentsOfFile:releaseLogo];
    [img setScalesWhenResized:YES];
    [img setSize:NSMakeSize(LOGO_WIDTH, LOGO_HEIGHT)];
    [iv setImage:img];
    [[self contentView] addSubview:iv];

    NSDictionary *osVersionDictionary = [NSDictionary
        dictionaryWithContentsOfFile:@"/System/Library/CoreServices/SystemVersion.plist"];

    NSFontManager *fontmgr = [NSFontManager sharedFontManager];
    NSFont *fontBigBold = [fontmgr convertFont:[NSFont systemFontOfSize:32] toHaveTrait:NSBoldFontMask];
    NSFont *fontMed = [NSFont systemFontOfSize:20];
    NSFont *fontNormal = [NSFont systemFontOfSize:14];
    NSFont *fontBold = [fontmgr convertFont:fontNormal toHaveTrait:NSBoldFontMask];

    float ypos = WIN_HEIGHT - (WIN_HEIGHT / 4) - V_SPACER;
    float xpos = LOGO_WIDTH + H_SPACER;
    float width = WIN_WIDTH - LOGO_WIDTH - (2*H_SPACER);
    NSTextView *tv = [[NSTextView alloc] initWithFrame:NSMakeRect(xpos, ypos, width, WIN_HEIGHT / 4)];
    [tv setDrawsBackground:NO];
    NSTextStorage *ts = [tv textStorage];

    [ts beginEditing];

    NSAttributedString *s = [[NSAttributedString alloc] initWithString:
        [osVersionDictionary objectForKey:@"ProductName"]
        attributes:[NSDictionary dictionaryWithObject:fontBigBold forKey:NSFontAttributeName]];
    [ts appendAttributedString:s];

    s = [[NSAttributedString alloc] initWithString:[NSString stringWithFormat:@" %@\n",
        [osVersionDictionary objectForKey:@"ProductFamily"]]
        attributes:[NSDictionary dictionaryWithObject:fontMed forKey:NSFontAttributeName]];
    [ts appendAttributedString:s];

    s = [[NSAttributedString alloc] initWithString:[NSString stringWithFormat:@"Version: %@",
        [osVersionDictionary objectForKey:@"ProductUserVisibleVersion"]]
        attributes:[NSDictionary dictionaryWithObject:fontNormal forKey:NSFontAttributeName]];
    [ts appendAttributedString:s];

    [ts endEditing];
    [tv setEditable:NO];
    [[self contentView] addSubview:tv];

    FILE *fp = popen("/System/Library/CoreServices/DMIHelper", "r");
    if(fp) {
        char buf1[80];
        char buf2[80];
        memset(buf1, 0, sizeof(buf1));
        memset(buf2, 0, sizeof(buf2));
        if(fgets(buf1, sizeof(buf1), fp) != NULL)
            buf1[strlen(buf1) - 1] = 0;
        if(fgets(buf2, sizeof(buf2), fp) != NULL)
            buf2[strlen(buf2) - 1] = 0;
        pclose(fp);
        s = [[NSAttributedString alloc] initWithString:[NSString 
            stringWithFormat:@"%s %s", buf1, buf2]
            attributes:[NSDictionary dictionaryWithObject:fontBold forKey:NSFontAttributeName]];
        ypos -= [s size].height;

        [[self contentView] addSubview:[Label labelWithText:s atPoint:NSMakePoint(xpos, ypos)
            withMaxWidth:width]];
        ypos -= ([s size].height * 2);
    }

    NSPlatform *platform = [NSPlatform currentPlatform];
    s = [[NSAttributedString alloc] initWithString:@"Processor:"
        attributes:[NSDictionary dictionaryWithObject:fontBold forKey:NSFontAttributeName]];
    
    [[self contentView] addSubview:[Label labelWithText:s atPoint:NSMakePoint(xpos, ypos)
        withMaxWidth:width / 4]];

    s = [[NSAttributedString alloc] initWithString:[NSString stringWithFormat:@"%u-core %@",
            [platform processorCount], [platform CPUModel]]
            attributes:[NSDictionary dictionaryWithObject:fontNormal forKey:NSFontAttributeName]];
    Label *label = [Label labelWithText:s atPoint:NSMakePoint(xpos + width / 4, ypos) 
        withMaxWidth:width - (width / 4)];
    [[self contentView] addSubview:label];

    ypos -= [label size].height;

    s = [[NSAttributedString alloc] initWithString:@"Memory:"
        attributes:[NSDictionary dictionaryWithObject:fontBold forKey:NSFontAttributeName]];
    ypos -= [s size].height;
    [[self contentView] addSubview:[Label labelWithText:s atPoint:NSMakePoint(xpos, ypos)
        withMaxWidth:width / 4]];

    s = [[NSAttributedString alloc] initWithString:_formatAsGB([platform physicalMemory])
            attributes:[NSDictionary dictionaryWithObject:fontNormal forKey:NSFontAttributeName]];
    label = [Label labelWithText:s atPoint:NSMakePoint(xpos + width / 4, ypos)
        withMaxWidth:width - (width / 4)];
    [[self contentView] addSubview:label];

    ypos -= [label size].height;

    s = [[NSAttributedString alloc] initWithString:@"Graphics:"
        attributes:[NSDictionary dictionaryWithObject:fontBold forKey:NSFontAttributeName]];
    ypos -= [s size].height;
    
    [[self contentView] addSubview:[Label labelWithText:s atPoint:NSMakePoint(xpos, ypos)
        withMaxWidth:width / 4]];

    char *cards[] = {"vgapci0", "vgapci1", NULL};
    char buf[256];
    for(int i = 0; cards[i] != NULL; ++i) {
        char adaptor[300];

        memset(adaptor, 0, sizeof(adaptor));
        snprintf(buf, sizeof(buf), "/usr/sbin/pciconf -lv %s", cards[i]);
        FILE *fp = popen(buf, "r");
        if(fp == NULL)
            continue;
        
        while(fgets(buf, sizeof(buf), fp) != NULL) {
            NSString *line = [[NSString alloc] initWithUTF8String:buf];
        
            if([line rangeOfString:@"vendor"].location != NSNotFound) {
                for(int start = 0; start < [line length]; ++start) {
                    if([line characterAtIndex:start] == '\'') {
                        strncat(adaptor, 
                            [[line substringWithRange:NSMakeRange(start + 1,
                            [line length] - start - 3)] UTF8String],
                            sizeof(adaptor) - strlen(adaptor) - 1);
                        strcat(adaptor, " ");
                        break;
                    }
                }
            } else if([line rangeOfString:@"device"].location != NSNotFound) {
                for(int start = 0; start < [line length]; ++start) {
                    if([line characterAtIndex:start] == '\'') {
                        strncat(adaptor, 
                            [[line substringWithRange:NSMakeRange(start + 1,
                            [line length] - start - 3)] UTF8String],
                            sizeof(adaptor) - strlen(adaptor) - 1);
                        strcat(adaptor, " ");
                        break;
                    }
                }
            } else if([line rangeOfString:@"subclass"].location != NSNotFound) {
                if(adaptor[0] != 0)
                    continue; // skip subclass unless it's all we got
                for(int start = 0; start < [line length]; ++start) {
                    if([line characterAtIndex:start] == '=') {
                        strncat(adaptor, 
                            [[line substringWithRange:NSMakeRange(start + 1,
                            [line length] - start - 2)] UTF8String],
                            sizeof(adaptor) - strlen(adaptor) - 1);
                        break;
                    }
                }
            }
        }

        s = [[NSAttributedString alloc]
            initWithString:[NSString stringWithFormat:@"%s", adaptor]
            attributes:[NSDictionary dictionaryWithObject:fontNormal forKey:NSFontAttributeName]];
        label = [Label labelWithText:s atPoint:NSMakePoint(xpos + width / 4, ypos) 
            withMaxWidth:width - (width / 4)];
        [[self contentView] addSubview:label];
        ypos -= [label size].height;

        pclose(fp);
    }

    return self;
}

@end

