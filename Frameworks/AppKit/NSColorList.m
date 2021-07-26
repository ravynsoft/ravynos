/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSColorList.h>
#import <AppKit/NSRaise.h>

NSString * const NSColorListDidChangeNotification = @"NSColorListDidChangeNotification";

@implementation NSColorList

static NSMutableDictionary *_namedColorLists = nil;

+ (void)_createDefaultColorLists
{
   static struct WebColor {
    NSString *name;
    unsigned  value;
   } webColors[217]={
    { @"Alice Blue",0xF0F8FF },
    { @"Antique White",0xFAEBD7 },
    { @"Aqua",0x00FFFF },
    { @"Aquamarine",0x7FFFD4 },
    { @"Azure",0xF0FFFF },
    { @"Beige",0xF5F5DC },
    { @"Bisque",0xFFE4C4 },
    { @"Black",0x000000 },
    { @"Blanched Almond",0xFFEBCD },
    { @"Blue",0x0000FF },
    { @"Blue Violet",0x8A2BE2 },
    { @"Brown",0xA52A2A },
    { @"Burly Wood",0xDEB887 },
    { @"Cadet Blue",0x5F9EA0 },
    { @"Chartreuse",0x7FFF00 },
    { @"Chocolate",0xD2691E },
    { @"Coral",0xFF7F50 },
    { @"Cornflower Blue",0x6495ED },
    { @"Cornsilk",0xFFF8DC },
    { @"Crimson",0xDC143C },
    { @"Cyan",0x00FFFF },
    { @"Dark Blue",0x00008B },
    { @"Dark Cyan",0x008B8B },
    { @"Dark GoldenRod",0xB8860B },
    { @"Dark Gray",0xA9A9A9 },
    { @"Dark Green",0x006400 },
    { @"Dark Khaki",0xBDB76B },
    { @"Dark Magenta",0x8B008B },
    { @"Dark Olive Green",0x556B2F },
    { @"Dark Orange",0xFF8C00 },
    { @"Dark Orchid",0x9932CC },
    { @"Dark Red",0x8B0000 },
    { @"Dark Salmon",0xE9967A },
    { @"Dark Sea Green",0x8FBC8F },
    { @"Dark Slate Blue",0x483D8B },
    { @"Dark Slate Gray",0x2F4F4F },
    { @"Dark Turquoise",0x00CED1 },
    { @"Dark Violet",0x9400D3 },
    { @"Deep Pink",0xFF1493 },
    { @"Deep Sky Blue",0x00BFFF },
    { @"Dim Gray",0x696969 },
    { @"Dodger Blue",0x1E90FF },
    { @"Fire Brick",0xB22222 },
    { @"Floral White",0xFFFAF0 },
    { @"Forest Green",0x228B22 },
    { @"Gainsboro",0xDCDCDC },
    { @"Ghost White",0xF8F8FF },
    { @"Gold",0xFFD700 },
    { @"Golden Rod",0xDAA520 },
    { @"Gray",0x808080 },
    { @"Green",0x008000 },
    { @"Green Yellow",0xADFF2F },
    { @"Honey Dew",0xF0FFF0 },
    { @"Hot Pink",0xFF69B4 },
    { @"Indian Red",0xCD5C5C },
    { @"Indigo",0x4B0082 },
    { @"Ivory",0xFFFFF0 },
    { @"Khaki",0xF0E68C },
    { @"Lavender",0xE6E6FA },
    { @"Lavender Blush",0xFFF0F5 },
    { @"Lawn Green",0x7CFC00 },
    { @"Lemon Chiffon",0xFFFACD },
    { @"Light Blue",0xADD8E6 },
    { @"Light Coral",0xF08080 },
    { @"Light Cyan",0xE0FFFF },
    { @"Light Golden Rod Yellow",0xFAFAD2 },
    { @"Light Gray",0xD3D3D3 },
    { @"Light Green",0x90EE90 },
    { @"Light Pink",0xFFB6C1 },
    { @"Light Salmon",0xFFA07A },
    { @"Light Sea Green",0x20B2AA },
    { @"Light Sky Blue",0x87CEFA },
    { @"Light Slate Blue",0x8470FF },
    { @"Light Slate Gray",0x778899 },
    { @"Light Steel Blue",0xB0C4DE },
    { @"Light Yellow",0xFFFFE0 },
    { @"Lime",0x00FF00 },
    { @"Lime Green",0x32CD32 },
    { @"Linen",0xFAF0E6 },
    { @"Magenta",0xFF00FF },
    { @"Maroon",0x800000 },
    { @"Medium Aqua Marine",0x66CDAA },
    { @"Medium Blue",0x0000CD },
    { @"Medium Orchid",0xBA55D3 },
    { @"Medium Purple",0x9370D8 },
    { @"Medium Sea Green",0x3CB371 },
    { @"Medium Slate Blue",0x7B68EE },
    { @"Medium Spring Green",0x00FA9A },
    { @"Medium Turquoise",0x48D1CC },
    { @"Medium Violet Red",0xC71585 },
    { @"Midnight Blue",0x191970 },
    { @"Mint Cream",0xF5FFFA },
    { @"Misty Rose",0xFFE4E1 },
    { @"Moccasin",0xFFE4B5 },
    { @"Navajo White",0xFFDEAD },
    { @"Navy",0x000080 },
    { @"Old Lace",0xFDF5E6 },
    { @"Olive",0x808000 },
    { @"Olive Drab",0x6B8E23 },
    { @"Orange",0xFFA500 },
    { @"Orange Red",0xFF4500 },
    { @"Orchid",0xDA70D6 },
    { @"Pale Golden Rod",0xEEE8AA },
    { @"Pale Green",0x98FB98 },
    { @"Pale Turquoise",0xAFEEEE },
    { @"PaleViolet Red",0xD87093 },
    { @"Papaya Whip",0xFFEFD5 },
    { @"Peach Puff",0xFFDAB9 },
    { @"Peru",0xCD853F },
    { @"Pink",0xFFC0CB },
    { @"Plum",0xDDA0DD },
    { @"Powder Blue",0xB0E0E6 },
    { @"Purple",0x800080 },
    { @"Red",0xFF0000 },
    { @"Rosy Brown",0xBC8F8F },
    { @"Royal Blue",0x4169E1 },
    { @"Saddle Brown",0x8B4513 },
    { @"Salmon",0xFA8072 },
    { @"Sandy Brown",0xF4A460 },
    { @"Sea Green",0x2E8B57 },
    { @"Sea Shell",0xFFF5EE },
    { @"Sienna",0xA0522D },
    { @"Silver",0xC0C0C0 },
    { @"Sky Blue",0x87CEEB },
    { @"Slate Blue",0x6A5ACD },
    { @"Slate Gray",0x708090 },
    { @"Snow",0xFFFAFA },
    { @"Spring Green",0x00FF7F },
    { @"Steel Blue",0x4682B4 },
    { @"Tan",0xD2B48C },
    { @"Teal",0x008080 },
    { @"Thistle",0xD8BFD8 },
    { @"Tomato",0xFF6347 },
    { @"Turquoise",0x40E0D0 },
    { @"Violet",0xEE82EE },
    { @"Violet Red",0xD02090 },
    { @"Wheat",0xF5DEB3 },
    { @"White",0xFFFFFF },
    { @"White Smoke",0xF5F5F5 },
    { @"Yellow",0xFFFF00 },
    { @"Yellow Green",0x9ACD32 },
    { nil, 0x0 }
   };

   NSColorList *basicColorList = [[[NSColorList alloc] initWithName:@"Basic"] autorelease];
   NSColorList *systemColorList = [[[NSColorList alloc] initWithName:@"System"] autorelease];
   NSColorList *webColorList = [[[NSColorList alloc] initWithName:@"Web"] autorelease];
   int i;

   for(i=0;webColors[i].name!=nil;i++){
    unsigned value=webColors[i].value;
    float red=((value>>16)&0xFF)/255.0;
    float green=((value>>8)&0xFF)/255.0;
    float blue=(value&0xFF)/255.0;
    NSColor *color=[NSColor colorWithCalibratedRed:red green:green blue:blue alpha:1.0];

    [webColorList setColor:color forKey: webColors[i].name];
   }

    [basicColorList setColor:[NSColor blackColor] forKey:@"Black"];
    [basicColorList setColor:[NSColor blueColor] forKey:@"Blue"];
    [basicColorList setColor:[NSColor brownColor] forKey:@"Brown"];
    [basicColorList setColor:[NSColor cyanColor] forKey:@"Cyan"];
    [basicColorList setColor:[NSColor greenColor] forKey:@"Green"];
    [basicColorList setColor:[NSColor magentaColor] forKey:@"Magenta"];
    [basicColorList setColor:[NSColor orangeColor] forKey:@"Orange"];
    [basicColorList setColor:[NSColor purpleColor] forKey:@"Purple"];
    [basicColorList setColor:[NSColor redColor] forKey:@"Red"];
    [basicColorList setColor:[NSColor yellowColor] forKey:@"Yellow"];
    [basicColorList setColor:[NSColor whiteColor] forKey:@"White"];

    [systemColorList setColor:[NSColor alternateSelectedControlColor] forKey:@"alternateSelectedControlColor"];
    [systemColorList setColor:[NSColor alternateSelectedControlTextColor] forKey:@"alternateSelectedControlTextColor"];
    [systemColorList setColor:[NSColor controlBackgroundColor] forKey:@"controlBackgroundColor"];
    [systemColorList setColor:[NSColor controlColor] forKey:@"controlColor"];
    [systemColorList setColor:[NSColor controlDarkShadowColor] forKey:@"controlDarkShadowColor"];
    [systemColorList setColor:[NSColor controlHighlightColor] forKey:@"controlHighlightColor"];
    [systemColorList setColor:[NSColor controlLightHighlightColor] forKey:@"controlLightHighlightColor"];
    [systemColorList setColor:[NSColor controlShadowColor] forKey:@"controlShadowColor"];
    [systemColorList setColor:[NSColor controlTextColor] forKey:@"controlTextColor"];
    [systemColorList setColor:[NSColor disabledControlTextColor] forKey:@"disabledControlTextColor"];
    [systemColorList setColor:[NSColor gridColor] forKey:@"gridColor"];
    [systemColorList setColor:[NSColor headerColor] forKey:@"headerColor"];
    [systemColorList setColor:[NSColor headerTextColor] forKey:@"headerTextColor"];
    [systemColorList setColor:[NSColor highlightColor] forKey:@"highlightColor"];
    [systemColorList setColor:[NSColor keyboardFocusIndicatorColor] forKey:@"keyboardFocusIndicatorColor"];
    [systemColorList setColor:[NSColor knobColor] forKey:@"knobColor"];
    [systemColorList setColor:[NSColor scrollBarColor] forKey:@"scrollBarColor"];
    [systemColorList setColor:[NSColor secondarySelectedControlColor] forKey:@"secondarySelectedControlColor"];
    [systemColorList setColor:[NSColor selectedControlColor] forKey:@"selectedControlColor"];
    [systemColorList setColor:[NSColor selectedControlTextColor] forKey:@"selectedControlTextColor"];
    [systemColorList setColor:[NSColor selectedKnobColor] forKey:@"selectedKnobColor"];
    [systemColorList setColor:[NSColor selectedMenuItemColor] forKey:@"selectedMenuItemColor"];
    [systemColorList setColor:[NSColor selectedMenuItemTextColor] forKey:@"selectedMenuItemTextColor"];
    [systemColorList setColor:[NSColor selectedTextBackgroundColor] forKey:@"selectedTextBackgroundColor"];
    [systemColorList setColor:[NSColor selectedTextColor] forKey:@"selectedTextColor"];
    [systemColorList setColor:[NSColor shadowColor] forKey:@"shadowColor"];
    [systemColorList setColor:[NSColor textBackgroundColor] forKey:@"textBackgroundColor"];
    [systemColorList setColor:[NSColor textColor] forKey:@"textColor"];
    [systemColorList setColor:[NSColor windowBackgroundColor] forKey:@"windowBackgroundColor"];
    [systemColorList setColor:[NSColor windowFrameColor] forKey:@"windowFrameColor"];


    _namedColorLists = [[NSMutableDictionary alloc] init];
    [_namedColorLists setObject:basicColorList forKey:@"Basic"];
    [_namedColorLists setObject:systemColorList forKey:@"System"];
    [_namedColorLists setObject:webColorList forKey:@"Web"];
}

+ (NSArray *)availableColorLists
{
    if (_namedColorLists == nil)
        [NSColorList _createDefaultColorLists];
    
    return [_namedColorLists allValues];
}

-initWithName:(NSString *)name fromFile:(NSString *)path
{
    _keys = [[NSMutableArray alloc] init];
    _colors = [[NSMutableArray alloc] init];
    _name = [name copy];
    _path = [path copy];

    if (_path != nil) {
// FIX, file loading doesnt work for NSColorList
         // DYFIX: load list
    }

    return self;
}

-initWithName:(NSString *)name
{
    return [self initWithName:name fromFile:nil];
}

-(void)dealloc
{
    [_keys release];
    [_colors release];
    [_name release];
    [_path release];

    [super dealloc];
}

+ (NSColorList *)colorListNamed:(NSString *)name
{
    if (_namedColorLists == nil)
        [NSColorList _createDefaultColorLists];
    
    return [_namedColorLists objectForKey:name];
}

-(BOOL)isEditable {
   NSUnimplementedMethod();
   return NO;
}

-(NSString *)name { return _name; }

-(NSArray *)allKeys { return _keys; }

-(NSColor *)colorWithKey:(NSString *)soughtKey indexPtr:(unsigned *)index
{
    NSEnumerator *keyEnumerator = [_keys objectEnumerator];
    NSString *thisKey;

    *index = 0;
    while ((thisKey = [keyEnumerator nextObject])!=nil) {
        if ([thisKey isEqualToString:soughtKey])
            return [_colors objectAtIndex:*index];
        (*index)++;
    }

    return nil;
}

-(NSColor *)colorWithKey:(NSString *)soughtKey
{
    unsigned index;	// unused
    return [self colorWithKey:soughtKey indexPtr:&index];
}

-(void)setColor:(NSColor *)color forKey:(NSString *)key
{
    unsigned index;

    // if we already have a color with this key, replace it...
    if ([self colorWithKey:key indexPtr:&index])
        [_colors replaceObjectAtIndex:index withObject:color];
    else {
        [_keys addObject:key];		// otherwise the color/key combo are added to the end of the list
        [_colors addObject:color];
    }

    [[NSNotificationCenter defaultCenter] postNotificationName:NSColorListDidChangeNotification object:self];
}

-(void)removeColorWithKey:(NSString *)key
{
    unsigned index;

    if ([self colorWithKey:key indexPtr:&index]) {
        [_colors removeObjectAtIndex:index];
        [_keys removeObjectAtIndex:index];

        [[NSNotificationCenter defaultCenter] postNotificationName:NSColorListDidChangeNotification object:self];
    }
}

-(void)insertColor:(NSColor *)color key:(NSString *)key atIndex:(unsigned)index
{
    [_colors insertObject:color atIndex:index];
    [_keys insertObject:key atIndex:index];

    [[NSNotificationCenter defaultCenter] postNotificationName:NSColorListDidChangeNotification object:self];
}

-(void)writeToFile:(NSString *)path
{
    NSUnimplementedMethod();
}

-(void)removeFile
{
    [[NSFileManager defaultManager] removeFileAtPath:_path handler:nil];
}

@end

