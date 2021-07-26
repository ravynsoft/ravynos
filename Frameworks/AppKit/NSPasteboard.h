/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/Foundation.h>
#import <AppKit/AppKitExport.h>

// New Pasteboard Types (added in 10.6) and interchangeable with the old pasteboard types in Cocotron
APPKIT_EXPORT NSString *const NSPasteboardTypeString;
APPKIT_EXPORT NSString *const NSPasteboardTypePDF;
APPKIT_EXPORT NSString *const NSPasteboardTypeTIFF;
APPKIT_EXPORT NSString *const NSPasteboardTypeRTF;
APPKIT_EXPORT NSString *const NSPasteboardTypeRTFD;
APPKIT_EXPORT NSString *const NSPasteboardTypeHTML;
APPKIT_EXPORT NSString *const NSPasteboardTypeTabularText;
APPKIT_EXPORT NSString *const NSPasteboardTypeFont;
APPKIT_EXPORT NSString *const NSPasteboardTypeRuler;
APPKIT_EXPORT NSString *const NSPasteboardTypeColor;

// Old Pasteboard Types
APPKIT_EXPORT NSString *const NSColorPboardType;
APPKIT_EXPORT NSString *const NSFileContentsPboardType;
APPKIT_EXPORT NSString *const NSFilenamesPboardType;
APPKIT_EXPORT NSString *const NSFontPboardType;
APPKIT_EXPORT NSString *const NSPDFPboardType;
APPKIT_EXPORT NSString *const NSPICTPboardType;
APPKIT_EXPORT NSString *const NSPostScriptPboardType;
APPKIT_EXPORT NSString *const NSRTFDPboardType;
APPKIT_EXPORT NSString *const NSRTFPboardType;
APPKIT_EXPORT NSString *const NSRulerPboardType;
APPKIT_EXPORT NSString *const NSStringPboardType;
APPKIT_EXPORT NSString *const NSTabularTextPboardType;
APPKIT_EXPORT NSString *const NSTIFFPboardType;
APPKIT_EXPORT NSString *const NSURLPboardType;

APPKIT_EXPORT NSString *const NSDragPboard;
APPKIT_EXPORT NSString *const NSFindPboard;
APPKIT_EXPORT NSString *const NSFontPboard;
APPKIT_EXPORT NSString *const NSGeneralPboard;
APPKIT_EXPORT NSString *const NSRulerPboard;

@interface NSPasteboard : NSObject

+ (NSPasteboard *)generalPasteboard;
+ (NSPasteboard *)pasteboardWithName:(NSString *)name;

- (int)changeCount;

- (NSArray *)types;
- (NSString *)availableTypeFromArray:(NSArray *)types;

- (NSData *)dataForType:(NSString *)type;
- (NSString *)stringForType:(NSString *)type;
- propertyListForType:(NSString *)type;

- (int)declareTypes:(NSArray *)types owner:owner;
- (int)addTypes:(NSArray *)types owner:(id)owner;

- (BOOL)setData:(NSData *)data forType:(NSString *)type;
- (BOOL)setString:(NSString *)string forType:(NSString *)type;
- (BOOL)setPropertyList:plist forType:(NSString *)type;

@end

@interface NSObject (NSPasteboard)
- (void)pasteboard:(NSPasteboard *)sender provideDataForType:(NSString *)type;
- (void)pasteboardChangedOwner:(NSPasteboard *)sender;
@end
