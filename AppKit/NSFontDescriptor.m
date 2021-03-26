/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSFontDescriptor.h>
#import <Foundation/NSString.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSValue.h>
#import <AppKit/NSRaise.h>

NSString * const NSFontNameAttribute=@"NSFontNameAttribute";
NSString * const NSFontFamilyAttribute=@"NSFontFamilyAttribute";
NSString * const NSFontSizeAttribute=@"NSFontSizeAttribute";
NSString * const NSFontMatrixAttribute=@"NSFontMatrixAttribute";
NSString * const NSFontCharacterSetAttribute=@"NSFontCharacterSetAttribute";
NSString * const NSFontTraitsAttribute=@"NSFontTraitsAttribute";
NSString * const NSFontFaceAttribute=@"NSFontFaceAttribute";
NSString * const NSFontFixedAdvanceAttribute=@"NSFontFixedAdvanceAttribute";
NSString * const NSFontVisibleNameAttribute=@"NSFontVisibleNameAttribute";

NSString * const NSFontSymbolicTrait = @"NSFontSymbolicTrait";
NSString * const NSFontWeightTrait = @"NSFontWeightTrait";
NSString * const NSFontWidthTrait = @"NSFontWidthTrait";
NSString * const NSFontSlantTrait = @"NSFontSlantTrait";

@implementation NSFontDescriptor : NSObject

-initWithFontAttributes:(NSDictionary *)attributes {
   _attributes=[attributes copy];
   return self;
}

-(void)dealloc {
   [_attributes release];
   [super dealloc];
}

-copyWithZone:(NSZone *)zone {
   return [self retain];
}


-(NSString *)description {
	return [NSString stringWithFormat:@"<%@ %@>",isa,_attributes];
}

+fontDescriptorWithFontAttributes:(NSDictionary *)attributes {
   return [[[self allocWithZone:NULL] initWithFontAttributes:attributes] autorelease];
}

+fontDescriptorWithName:(NSString *)name matrix:(NSAffineTransform *)matrix {
   NSDictionary *attributes=[NSDictionary dictionaryWithObjectsAndKeys:
    name,NSFontNameAttribute,
    matrix,NSFontMatrixAttribute,
    nil];
   return [[[self allocWithZone:NULL] initWithFontAttributes:attributes] autorelease];
}

+fontDescriptorWithName:(NSString *)name size:(float)pointSize {
   NSDictionary *attributes=[NSDictionary dictionaryWithObjectsAndKeys:
    name,NSFontNameAttribute,
    [[NSNumber numberWithFloat:pointSize] stringValue],NSFontSizeAttribute,
    nil];
   
   return [[[self allocWithZone:NULL] initWithFontAttributes:attributes] autorelease];
}

-(NSDictionary *)fontAttributes {
   return _attributes;
}

-objectForKey:(NSString *)attributeKey {
   return [_attributes objectForKey:attributeKey];
}

-(float)pointSize {
   return [[_attributes objectForKey:NSFontSizeAttribute] floatValue];
}

-(NSAffineTransform *)matrix {
   return [_attributes objectForKey:NSFontMatrixAttribute];
}

-(NSFontSymbolicTraits)symbolicTraits {
	NSDictionary* fontTraits = [_attributes objectForKey:NSFontTraitsAttribute];
	return [[fontTraits objectForKey: NSFontSymbolicTrait] unsignedIntValue];
}

-(NSFontDescriptor *)fontDescriptorByAddingAttributes:(NSDictionary *)attributes {
   NSMutableDictionary *copy=[NSMutableDictionary dictionaryWithDictionary:_attributes];
   
   [copy addEntriesFromDictionary:attributes];
   
   return [isa fontDescriptorWithFontAttributes:copy];
}

-(NSFontDescriptor *)fontDescriptorWithFace:(NSString *)face {
   NSMutableDictionary *copy=[NSMutableDictionary dictionaryWithDictionary:_attributes];

   [copy setObject:face forKey:NSFontFaceAttribute];
   
   return [isa fontDescriptorWithFontAttributes:copy];
}

-(NSFontDescriptor *)fontDescriptorWithFamily:(NSString *)family {
   NSMutableDictionary *copy=[NSMutableDictionary dictionaryWithDictionary:_attributes];

   [copy setObject:family forKey:NSFontFamilyAttribute];

   return [isa fontDescriptorWithFontAttributes:copy];
}

-(NSFontDescriptor *)fontDescriptorWithMatrix:(NSAffineTransform *)matrix {
   NSMutableDictionary *copy=[NSMutableDictionary dictionaryWithDictionary:_attributes];

   [copy setObject:matrix forKey:NSFontMatrixAttribute];

   return [isa fontDescriptorWithFontAttributes:copy];
}

-(NSFontDescriptor *)fontDescriptorWithSize:(float)pointSize {
   NSMutableDictionary *copy=[NSMutableDictionary dictionaryWithDictionary:_attributes];

   [copy setObject:[NSNumber numberWithFloat:pointSize] forKey:NSFontSizeAttribute];

   return [isa fontDescriptorWithFontAttributes:copy];
}

-(NSFontDescriptor *)fontDescriptorWithSymbolicTraits:(NSFontSymbolicTraits)traits {
   NSMutableDictionary *copy=[NSMutableDictionary dictionaryWithDictionary:_attributes];

	NSMutableDictionary *traitsCopy = [NSMutableDictionary dictionaryWithDictionary: [_attributes objectForKey: NSFontTraitsAttribute]];
	[traitsCopy setObject: [NSNumber numberWithUnsignedInt: traits] forKey: NSFontSymbolicTrait];
   [copy setObject:traitsCopy forKey:NSFontTraitsAttribute];

   return [isa fontDescriptorWithFontAttributes:copy];
}

-(NSArray *)matchingFontDescriptorsWithMandatoryKeys:(NSSet *)keys {
   NSUnimplementedMethod();
   return nil;
}

-(NSFontDescriptor *)matchingFontDescriptorWithMandatoryKeys:(NSSet *)keys {
   NSUnimplementedMethod();
   return nil;
}

@end
