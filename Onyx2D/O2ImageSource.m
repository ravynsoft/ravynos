/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2ImageSource.h>
#import <Foundation/NSData.h>
#import <Onyx2D/O2DataProvider.h>
#import <Onyx2D/O2Exceptions.h>

NSString *kO2ImagePropertyDPIWidth=@"DPIWidth";
NSString *kO2ImagePropertyDPIHeight=@"DPIHeight";
NSString *kO2ImagePropertyPixelHeight=@"PixelHeight";
NSString *kO2ImagePropertyPixelWidth=@"PixelWidth";
NSString *kO2ImagePropertyOrientation=@"Orientation";

NSString *kO2ImagePropertyTIFFDictionary=@"{TIFF}";
NSString *kO2ImagePropertyExifDictionary=@"{Exif}";

NSString *kO2ImagePropertyTIFFXResolution=@"XResolution";
NSString *kO2ImagePropertyTIFFYResolution=@"YResolution";
NSString *kO2ImagePropertyTIFFOrientation=@"Orientation";

@interface _O2ImageSource : O2ImageSource
@end

@implementation O2ImageSource

+(O2ImageSourceRef)newImageSourceWithDataProvider:(O2DataProvider *)provider options:(CFDictionaryRef)options {
   NSString *classes[]={
    @"O2ImageSource_PNG",
    @"O2ImageSource_TIFF",
    @"O2ImageSource_JPEG",
    @"O2ImageSource_BMP",
    @"O2ImageSource_GIF",
    @"O2ImageSource_ICNS",
    nil
   };
   int i;
   
   for(i=0;classes[i]!=nil;i++){
    Class cls=NSClassFromString(classes[i]);
   
    if([cls isPresentInDataProvider:provider]){
     [provider rewind];
     return [[cls alloc] initWithDataProvider:provider options:(NSDictionary *)options];
    }
   }
       
   return nil;
}

+(O2ImageSourceRef)newImageSourceWithData:(CFDataRef)data options:(CFDictionaryRef)options {
    O2DataProviderRef provider=O2DataProviderCreateWithCFData(data);
    O2ImageSourceRef result=[self newImageSourceWithDataProvider:provider options:options];
    O2DataProviderRelease(provider);
    return result;
}

+(O2ImageSourceRef)newImageSourceWithURL:(NSURL *)url options:(CFDictionaryRef)options {
   O2DataProviderRef provider=[[O2DataProvider alloc] initWithURL:url];
   O2ImageSourceRef result=[self newImageSourceWithDataProvider:provider options:options];
   O2DataProviderRelease(provider);
   return result;
}

+(BOOL)isPresentInDataProvider:(O2DataProvider *)provider {
   return NO;
}

-initWithDataProvider:(O2DataProvider *)provider options:(NSDictionary *)options {
   _provider=[provider retain];
   _options=[options retain];
   return self;
}

-(void)dealloc
{
	[_provider release];
	[_options release];
	[super dealloc];
}

- (CFStringRef)type
{
    O2InvalidAbstractInvocation();
    return nil;
}

-(unsigned)count {
   O2InvalidAbstractInvocation();
   return 0;
}

-(CFDictionaryRef)copyPropertiesAtIndex:(unsigned)index options:(CFDictionaryRef)options {
   return nil;
}

-(O2Image *)createImageAtIndex:(unsigned)index options:(CFDictionaryRef)options {
  O2InvalidAbstractInvocation();
  return nil;
}

O2ImageRef O2ImageSourceCreateImageAtIndex(O2ImageSourceRef self,size_t index,CFDictionaryRef options) {
  return [(O2ImageSource *)self createImageAtIndex:index options:options];
}

@end
