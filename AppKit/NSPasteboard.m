/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSPasteboard.h>
#import <AppKit/NSDisplay.h>
#import <AppKit/NSRaise.h>

NSString *const NSPasteboardTypeString = @"NSStringPboardType";
NSString *const NSPasteboardTypePDF = @"NSPDFPboardType";
NSString *const NSPasteboardTypeTIFF = @"NSTIFFPboardType";
NSString *const NSPasteboardTypeRTF = @"NSRTFPboardType";
NSString *const NSPasteboardTypeRTFD = @"NSRTFDPboardType";
NSString *const NSPasteboardTypeHTML = @"NSPasteboardTypeHTML";
NSString *const NSPasteboardTypeTabularText = @"NSTabularTextPboardType";
NSString *const NSPasteboardTypeFont = @"NSFontPboardType";
NSString *const NSPasteboardTypeRuler = @"NSRulerPboardType";
NSString *const NSPasteboardTypeColor = @"NSColorPboardType";

NSString * const NSColorPboardType=@"NSColorPboardType";
NSString * const NSFileContentsPboardType=@"NSFileContentsPboardType";
NSString * const NSFilenamesPboardType=@"NSFilenamesPboardType";
NSString * const NSFontPboardType=@"NSFontPboardType";
NSString * const NSPDFPboardType=@"NSPDFPboardType";
NSString * const NSPICTPboardType=@"NSPICTPboardType";
NSString * const NSPostScriptPboardType=@"NSPostScriptPboardType";
NSString * const NSRTFDPboardType=@"NSRTFDPboardType";
NSString * const NSRTFPboardType=@"NSRTFPboardType";
NSString * const NSRulerPboardType=@"NSRulerPboardType";
NSString * const NSStringPboardType=@"NSStringPboardType";
NSString * const NSTabularTextPboardType=@"NSTabularTextPboardType";
NSString * const NSTIFFPboardType=@"NSTIFFPboardType";
NSString * const NSURLPboardType=@"NSURLPboardType";

NSString * const NSDragPboard=@"NSDragPboard";
NSString * const NSFindPboard=@"NSFindPboard";
NSString * const NSFontPboard=@"NSFontPboard";
NSString * const NSGeneralPboard=@"NSGeneralPboard";
NSString * const NSRulerPboard=@"NSRulerPboard";

@implementation NSPasteboard

+(NSPasteboard *)generalPasteboard {
   return [self pasteboardWithName:NSGeneralPboard];
}

+(NSPasteboard *)pasteboardWithName:(NSString *)name {
   return [[NSDisplay currentDisplay] pasteboardWithName:name];
}

-(int)changeCount {
   NSUnimplementedMethod();
   return 0;
}

-(NSArray *)types {
   NSUnimplementedMethod();
   return nil;
}

-(NSString *)availableTypeFromArray:(NSArray *)types {
   NSArray *available=[self types];
   int      i,count=[types count];

   for(i=0;i<count;i++){
    NSString *check=[types objectAtIndex:i];

    if([available containsObject:check])
     return check;
   }

   return nil;
}


-(NSData *)dataForType:(NSString *)type {
   NSUnimplementedMethod();
   return nil;
}

-(NSString *)stringForType:(NSString *)type {
   NSData *data=[self dataForType:type];

   return [[[NSString alloc] initWithData:data encoding:NSUnicodeStringEncoding] autorelease];
}

-(id)propertyListForType:(NSString *)type {
	NSData* data = [self dataForType: type];
	NSString* errorDesc = nil;
	id plist = [NSPropertyListSerialization propertyListFromData: data mutabilityOption: NSPropertyListImmutable format: NULL errorDescription: &errorDesc];
	if (plist && errorDesc == nil) {
		return plist;
	}
	NSLog(@"propertyListForType: produced error: %@", errorDesc);
	return nil;
}

-(int)declareTypes:(NSArray *)types owner:(id)owner {
   NSUnimplementedMethod();
   return 0;
}

-(int)addTypes:(NSArray *)types owner:(id)owner {
    NSUnimplementedMethod();
    return 0;
}

-(BOOL)setData:(NSData *)data forType:(NSString *)type {
   NSUnimplementedMethod();
   return NO;
}

-(BOOL)setString:(NSString *)string forType:(NSString *)type {
   NSData *data=[string dataUsingEncoding:NSUnicodeStringEncoding];
   return [self setData:data forType:type];
}

-(BOOL)setPropertyList:(id)plist forType:(NSString *)type {
	NSString* errorDesc = nil;
	NSData* data = [NSPropertyListSerialization dataFromPropertyList: plist format: NSPropertyListXMLFormat_v1_0 errorDescription: &errorDesc];
	if (data && errorDesc == nil) {
		return [self setData: data forType: type]; 
	}
	NSLog(@"setPropertyList:forType: produced error: %@", errorDesc);
   return NO;
}

@end
