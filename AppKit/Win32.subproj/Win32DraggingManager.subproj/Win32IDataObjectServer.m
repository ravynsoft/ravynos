/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/Win32IDataObjectServer.h>
#import <AppKit/Win32IEnumFORMATETCServer.h>
#import <AppKit/Win32FORMATETC.h>
#import <AppKit/Win32Pasteboard.h>
#import <Foundation/NSString_win32.h>
#import <AppKit/NSRaise.h>

#import <AppKit/NSBitmapImageRep.h>

#import <windows.h>

#ifndef CF_DIBV5
#define CF_DIBV5 17
#endif

#ifndef LCS_sRGB
#define LCS_sRGB 0x73524742 // 'sRGB'
#endif

static void flipRowsAndSwapColors(long width, long height, long bytesPerRow, const uint8_t *src, uint8_t *dest)
{
	src += bytesPerRow * (height - 1); // Ptr to the last line
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			uint8_t r = src[4*j];
			uint8_t g = src[4*j+1];
			uint8_t b = src[4*j+2];
			uint8_t a = src[4*j+3];
			dest[4*j] = b;
			dest[4*j+1] = g;
			dest[4*j+2] = r;
			dest[4*j+3] = a;
		}
		dest += bytesPerRow;
		src -= bytesPerRow;
	}
}	
@implementation Win32IDataObjectServer

-initWithPasteboard:(Win32Pasteboard *)pasteboard {
   [super initAsIDataObject];
   _pasteboard=[pasteboard retain];
   [_pasteboard incrementServerCount];
   return self;
}

-(void)dealloc {
   [_pasteboard decrementServerCount];
   [_pasteboard release];
   [super dealloc];
}

-(Win32Pasteboard *)pasteboard {
   return _pasteboard;
}

-(BOOL)setOnClipboard {
   if(OleSetClipboard([self iUknown])!=S_OK){
    NSLog(@"OleSetClipboard failed");
    return NO;
   }

   return YES;
}

-(NSArray *)formatEtcs {
   NSArray        *types=[_pasteboard types];
   NSMutableArray *result=[NSMutableArray array];
   int             i,count=[types count];
   FORMATETC format;

   format.ptd=NULL;
   format.dwAspect=DVASPECT_CONTENT;
   format.lindex=-1;
   format.tymed=TYMED_HGLOBAL;

   for(i=0;i<count;i++){
    NSString *type=[types objectAtIndex:i];
    if([type isEqualToString:NSStringPboardType]){
     format.cfFormat=CF_UNICODETEXT;
     [result addObject:[Win32FORMATETC formatEtcWithFORMATETC:format]];
     format.cfFormat=CF_TEXT;
     [result addObject:[Win32FORMATETC formatEtcWithFORMATETC:format]];
    }
    else if([type isEqualToString:NSRTFPboardType]){
     format.cfFormat=RegisterClipboardFormat("Rich Text Format");
     [result addObject:[Win32FORMATETC formatEtcWithFORMATETC:format]];
    }
    else if([type isEqualToString:NSPDFPboardType])
    {
     format.cfFormat=RegisterClipboardFormat("Portable Document Format");
     [result addObject:[Win32FORMATETC formatEtcWithFORMATETC:format]];
    }
    else if([type isEqualToString:NSFilenamesPboardType]){
		format.cfFormat=CF_HDROP;
		[result addObject:[Win32FORMATETC formatEtcWithFORMATETC:format]];
    }
    else if([type isEqualToString:NSTIFFPboardType]){
		format.cfFormat=CF_TIFF;
		[result addObject:[Win32FORMATETC formatEtcWithFORMATETC:format]];
		format.cfFormat=CF_DIBV5;
		[result addObject:[Win32FORMATETC formatEtcWithFORMATETC:format]];
		// The doc says DIBV5 should be enough and it would do the conversion if needed
		// Don't believe it - at least it doesn't work with Word 2010
		format.cfFormat=CF_BITMAP;
		format.tymed = TYMED_GDI;
		[result addObject:[Win32FORMATETC formatEtcWithFORMATETC:format]];		
		format.tymed = TYMED_HGLOBAL;
	}
	else {
		format.cfFormat=RegisterClipboardFormat([type cString]);
		[result addObject:[Win32FORMATETC formatEtcWithFORMATETC:format]];
    }
   }
   return result;
}

-(HRESULT)formatIsValid:(FORMATETC *)format {
   NSArray *formats=[self formatEtcs]; 
   int      i,count=[formats count];

   if(format->dwAspect!=DVASPECT_CONTENT)
    return DV_E_DVASPECT;
   if(format->lindex!=-1)
    return DV_E_LINDEX;
   for(i=0;i<count;i++){
	   FORMATETC fetc = [[formats objectAtIndex:i] FORMATETC];
	   if(fetc.cfFormat==format->cfFormat && (fetc.tymed&format->tymed)) {
		   return S_OK;
	   }
   }

   return DV_E_FORMATETC;
}

-(HRESULT)GetData:(FORMATETC *)format:(STGMEDIUM *)storageMediump {
   HRESULT   result;

   if((result=[self formatIsValid:format])!=S_OK)
    return result;
   storageMediump->tymed=TYMED_HGLOBAL;

   if(format->cfFormat==CF_TEXT){
    NSString *string=[_pasteboard stringForType:NSStringPboardType];
    char     *buffer;

    storageMediump->hGlobal=GlobalAlloc(GMEM_FIXED,([string cStringLength]+1)*sizeof(char));
    buffer=GlobalLock(storageMediump->hGlobal);

    [string getCString:buffer];

    GlobalUnlock(storageMediump->hGlobal);
   }
   else if(format->cfFormat==CF_UNICODETEXT){
    NSString *string=[_pasteboard stringForType:NSStringPboardType];
    unichar  *buffer;

    storageMediump->hGlobal=GlobalAlloc(GMEM_FIXED,([string length]+1)*sizeof(unichar));
    buffer=GlobalLock(storageMediump->hGlobal);

    [string getCharacters:buffer];
    buffer[[string length]]=0x0000;

    GlobalUnlock(storageMediump->hGlobal);
   }
   else if(format->cfFormat==CF_HDROP){
    NSArray   *files=[_pasteboard propertyListForType:NSFilenamesPboardType];
    int        filesSize=sizeof(DROPFILES),i,count=[files count];
    DROPFILES *dropFiles;
    unichar   *buffer;

    for(i=0;i<count;i++){
     NSString *file=[files objectAtIndex:i];

     filesSize+=([file length]+1)*sizeof(unichar);
    }
    filesSize+=sizeof(unichar);

    storageMediump->hGlobal=GlobalAlloc(GMEM_FIXED,filesSize);
    dropFiles=GlobalLock(storageMediump->hGlobal);
    dropFiles->pFiles=sizeof(DROPFILES);
    dropFiles->pt.x=0;
    dropFiles->pt.y=0;
    dropFiles->fNC=0;
    dropFiles->fWide=YES;

    buffer=((void *)dropFiles)+sizeof(DROPFILES);
    for(i=0;i<count;i++){
     NSString *file=[files objectAtIndex:i];

     [file getCharacters:buffer];
     buffer[[file length]]=0x0000;
     buffer+=[file length]+1;
    }
    *buffer=0x0000;
   } else if (format->cfFormat == CF_TIFF) {
	   NSData *data=[_pasteboard dataForType:NSTIFFPboardType];
	   if(data!=nil){
		   void     *buffer;
		   
		   storageMediump->hGlobal=GlobalAlloc(GMEM_FIXED,[data length]);
		   buffer=GlobalLock(storageMediump->hGlobal);
		   
		   [data getBytes:buffer];
		   
		   GlobalUnlock(storageMediump->hGlobal);
	   }
   } else if (format->cfFormat == CF_DIBV5 || format->cfFormat == CF_BITMAP || format->cfFormat == CF_DIB) {
	   NSData *data=[_pasteboard dataForType:NSTIFFPboardType];
	   if(data!=nil){
		   // Note: we know "NSBitmapImageRep imageRepWithData" is always giving us a 32 bits bitmap
		   NSBitmapImageRep *image = [NSBitmapImageRep imageRepWithData:data];
		   if (image) {
			   uint8_t *bits = [image bitmapData];
			   if (bits) {
				   BITMAPV5HEADER bi;
				   int bisize = sizeof(bi);

				   ZeroMemory(&bi, bisize);
				   bi.bV5Size = bisize;
				   bi.bV5Width = [image pixelsWide];
				   bi.bV5Height = [image pixelsHigh];
				   bi.bV5Planes = 1;
				   bi.bV5BitCount = [image bitsPerPixel];
				   bi.bV5SizeImage = [image bytesPerRow]*[image pixelsHigh];
				   bi.bV5Compression=BI_BITFIELDS;
				   bi.bV5BlueMask=	0x000000FF;
				   bi.bV5GreenMask= 0x0000FF00;
				   bi.bV5RedMask=	0x00FF0000;
				   bi.bV5AlphaMask= 0xFF000000;

				   bi.bV5CSType = LCS_sRGB;
				   bi.bV5Intent = LCS_GM_IMAGES;
				   
				   int bpr = [image bytesPerRow];
				   if (format->cfFormat == CF_BITMAP) {
					   uint8_t *buf;
					   
					   HBITMAP bitmap = CreateDIBSection(NULL, (BITMAPINFO *)&bi, DIB_RGB_COLORS, (void **)&buf, NULL, 0);

					   // Flip the image - in a perfect world, we could just set the bV5Height to a negative value, but 
					   // it seems a lot of Win apps can't deal with that - either are unable to get the image, or get a flipped
					   // version - so we'll flip it ourself...
					   // Same for the color order
					   flipRowsAndSwapColors(bi.bV5Width, bi.bV5Height, bpr, bits, buf);

					   storageMediump->tymed=TYMED_GDI;
					   storageMediump->hBitmap=(HBITMAP)OleDuplicateData(bitmap, CF_BITMAP, 0);;
					   DeleteObject(bitmap);
				   } else {
					   // DIBv5
					   DWORD dibsize = bi.bV5Size+bi.bV5SizeImage;
					   HGLOBAL hglob = GlobalAlloc(GMEM_FIXED,dibsize);
					   uint8_t *buf = (uint8_t*)GlobalLock(hglob); 
					   // Copy the bitmap header
					   memcpy(buf,&bi,bi.bV5Size);
					   
					   // Copy the bitmap bits
					   buf += bi.bV5Size;

					   // Flip the image - in a perfect world, we could just set the bV5Height to a negative value, but 
					   // it seems a lot of Win apps can't deal with that - either are unable to get the image, or get a flipped
					   // version - so we'll flip it ourself...
					   // Same for the color order
					   flipRowsAndSwapColors(bi.bV5Width, bi.bV5Height, bpr, bits, buf);
					   GlobalUnlock(hglob); 
					   
					   storageMediump->hGlobal=hglob;
				   }
			   }
		   }
	   }
   }
   else {
    char      name[2048];
    int       length;

    if((length=GetClipboardFormatName(format->cfFormat,name,2048))==0)
     return DV_E_FORMATETC;
    else {
     NSString *type=[NSString stringWithCString:name length:length];
     NSData   *data=nil;
		if([type isEqualToString:@"Rich Text Format"]) {
			type=NSRTFPboardType;
		}
		data=[_pasteboard dataForType:type];

     if(data!=nil){
      void     *buffer;

      storageMediump->hGlobal=GlobalAlloc(GMEM_MOVEABLE,[data length]);
      buffer=GlobalLock(storageMediump->hGlobal);

      [data getBytes:buffer];

      GlobalUnlock(storageMediump->hGlobal);
     }
    }
   }
   
   storageMediump->pUnkForRelease=NULL;
   return S_OK;
}

-(HRESULT)GetDataHere:(FORMATETC *)format :(STGMEDIUM *)storageMedium {
   NSUnimplementedMethod();
   return DV_E_FORMATETC;
}

-(HRESULT)QueryGetData:(FORMATETC *)format {
   return [self formatIsValid:format];
}

-(HRESULT)GetCanonicalFormatEtc:(FORMATETC *)format :(FORMATETC *)pformatetcOut {
   NSUnimplementedMethod();
   return S_OK;
}

-(HRESULT)SetData:(FORMATETC *)format :(STGMEDIUM *)pmedium :(BOOL)fRelease {
   NSUnimplementedMethod();
   return S_OK;
}

-(HRESULT)EnumFormatEtc:(DWORD)dwDirection :(IEnumFORMATETC  **)ppenumFormatEtc {
   Win32IEnumFORMATETCServer *server=[[Win32IEnumFORMATETCServer alloc] initAsIEnumFORMATETC];

   [server setFormatEtcs:[self formatEtcs]];

   *ppenumFormatEtc=[server iUknown];

   return S_OK;
}

-(HRESULT)DAdvise:(FORMATETC *)pformatetc :(DWORD)advf :(IAdviseSink *)pAdvSink :(DWORD *)pdwConnection {
   NSUnimplementedMethod();
   return E_NOTIMPL;
}

-(HRESULT)DUnadvise:(DWORD)dwConnection {
   NSUnimplementedMethod();
   return E_NOTIMPL;
}

-(HRESULT)EnumDAdvise:(IEnumSTATDATA **)ppenumAdvise {
   NSUnimplementedMethod();
   return E_NOTIMPL;
}

@end
