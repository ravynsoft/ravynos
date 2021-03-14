/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#import <AppKit/NSFontFamily.h>
#import <AppKit/NSFontTypeface.h>
#import <AppKit/NSGraphicsContext.h>
#import <AppKit/NSDisplay.h>

@interface NSFontFamily () 
+(NSMutableArray *)fontFamilies;
+(void)buildFontFamilies;
@end

@implementation NSFontFamily

+(NSMutableArray *)fontFamilies {
   static NSMutableArray *shared=nil;
   
   if(shared==nil) {
      shared=[NSMutableArray new];
      [self buildFontFamilies];
   }
   
   return shared;
}

+(NSArray *)allFontFamilyNames {
   NSMutableArray *result=[NSMutableArray new];
   NSArray        *families=[self fontFamilies];
   int             i,count=[families count];

   for(i=0;i<count;i++)
    [result addObject:[[families objectAtIndex:i] name]];

   [result sortUsingSelector:@selector(compare:)];

   return result;
}

+(void)addFontFamily:(NSFontFamily *)family {
   [[self fontFamilies] addObject:family];
}

+(void)buildFontFamilies {
   NSDisplay    *display=[NSDisplay currentDisplay];
   NSSet        *allNames=[display allFontFamilyNames];
   NSEnumerator *next=[allNames objectEnumerator];
   NSString     *name;
   NSArray      *families;
   int           i,count;

   while((name=[next nextObject])!=nil)
    [NSFontFamily addFontFamily:[[[NSFontFamily alloc] initWithName:name] autorelease]];

   families=[self fontFamilies];
   count=[families count];
   for(i=0;i<count;i++){
    NSFontFamily *family=[families objectAtIndex:i];
    NSArray      *typefaces=[display fontTypefacesForFamilyName:[family name]];

    [family addTypefaces:typefaces];
   }
}

+(NSFontFamily *)fontFamilyWithName:(NSString *)name {
   NSArray *families=[self fontFamilies];
   int      i,count=[families count];

   for(i=0;i<count;i++){
    NSFontFamily *check=[families objectAtIndex:i];

    if([[check name] isEqualToString:name])
     return check;
   }
   return nil;
}

+(NSFontFamily *)fontFamilyWithTypefaceName:(NSString *)name {
   NSArray *families=[self fontFamilies];
   int      i,count=[families count];

   for(i=0;i<count;i++){
    NSFontFamily   *check=[families objectAtIndex:i];
    NSFontTypeface *typeface=[check typefaceWithName:name];

    if(typeface!=nil)
     return check;
   }

   return nil;
}

+(NSFontTypeface *)fontTypefaceWithName:(NSString *)name {
   NSArray *families=[self fontFamilies];
   int      i,count=[families count];

   for(i=0;i<count;i++){
    NSFontFamily   *check=[families objectAtIndex:i];
    NSFontTypeface *typeface=[check typefaceWithName:name];

    if(typeface!=nil)
     return typeface;
   }

   return nil;
}

-initWithName:(NSString *)name {
	_name=[name copy];
	_typefaces=[NSMutableArray new];
	return self;
}

-(void)dealloc {
	[_name release];
   [_typefaces release];
   [super dealloc];
}

-(NSString *)name {
	return _name;
}

-(NSFontTypeface *)typefaceWithName:(NSString *)name {
   int i,count=[_typefaces count];

   for(i=0;i<count;i++){
    NSFontTypeface *typeface=[_typefaces objectAtIndex:i];

    if([[typeface name] isEqualToString:name])
     return typeface;
   }

   return nil;
}

-(NSFontTypeface *)typefaceWithTraits:(NSFontTraitMask)traits {
   int i,count=[_typefaces count];

   for(i=0;i<count;i++){
    NSFontTypeface *typeface=[_typefaces objectAtIndex:i];

    if([typeface traits]==traits)
     return typeface;
   }

   return nil;
}

-(void)addTypeface:(NSFontTypeface *)typeface {
   [_typefaces addObject:typeface];
}

-(void)addTypefaces:(NSArray *)typefaces {
   [_typefaces addObjectsFromArray:typefaces];
}

-(NSArray *)typefaces {
   return _typefaces;
}

-(NSString *)description {
	return [NSString stringWithFormat:@"<%@ 0x%x %@ %@>",isa,self,_name,_typefaces];
}
@end
