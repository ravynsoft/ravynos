/* Copyright (c) 2006-2007 Christopher J. W. Lloyd - <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/Win32IDataObjectClient.h>
#import <AppKit/Win32IStreamClient.h>
#import <AppKit/Win32FORMATETC.h>
#import <Foundation/NSString_win32.h>
#import <Foundation/NSUnicodeCaseMapping.h>

// We need to access the context dc to do image format conversion
#import <AppKit/O2Context_gdi.h>

#import <AppKit/NSPasteboard.h>
#import <AppKit/NSBitmapImageRep.h>

#ifndef CF_DIBV5
#define CF_DIBV5 17
#endif


// These methods come from MS sample code
static WORD DibNumColors (VOID FAR * pv)
{
    INT                 bits;
    LPBITMAPINFOHEADER  lpbi;
    LPBITMAPCOREHEADER  lpbc;
	
    lpbi = ((LPBITMAPINFOHEADER)pv);
    lpbc = ((LPBITMAPCOREHEADER)pv);
	
    /*  With the BITMAPINFO format headers, the size of the palette
     *  is in biClrUsed, whereas in the BITMAPCORE - style headers, it
     *  is dependent on the bits per pixel ( = 2 raised to the power of
     *  bits/pixel).
     */
    if (lpbi->biSize != sizeof(BITMAPCOREHEADER)){
        if (lpbi->biClrUsed != 0)
            return (WORD)lpbi->biClrUsed;
        bits = lpbi->biBitCount;
    }
    else
        bits = lpbc->bcBitCount;
	
    switch (bits){
        case 1:
			return 2;
        case 4:
			return 16;
        case 8:
			return 256;
        default:
			/* A 24 bitcount DIB has no color table */
			return 0;
    }
}

static WORD PaletteSize (VOID FAR * pv)
{
    LPBITMAPINFOHEADER lpbi;
    WORD               NumColors;
	
    lpbi      = (LPBITMAPINFOHEADER)pv;
    NumColors = DibNumColors(lpbi);
	
    if (lpbi->biSize == sizeof(BITMAPCOREHEADER))
        return (WORD)(NumColors * sizeof(RGBTRIPLE));
    else
        return (WORD)(NumColors * sizeof(RGBQUAD));
}

@implementation Win32IDataObjectClient

-initWithIDataObject:(struct IDataObject *)dataObject {
   _dataObject=dataObject;
   _dataObject->lpVtbl->AddRef(_dataObject);
   return self;
}

-initWithClipboard {
   struct IDataObject *data;

   if(OleGetClipboard(&data)!=S_OK){
#if DEBUG
    NSLog(@"OleGetClipboard(&data) failed");
#endif
    [self dealloc];
    return nil;
   }

   return [self initWithIDataObject:data];
}

-(void)dealloc {
   if(_dataObject!=NULL)
    _dataObject->lpVtbl->Release(_dataObject);

   [super dealloc];
}


-(NSArray *)availableTypes {
   NSMutableArray        *result=[NSMutableArray array];
   struct IEnumFORMATETC *enumerator;

   if(_dataObject->lpVtbl->EnumFormatEtc(_dataObject,DATADIR_GET,&enumerator)!=S_OK){
#if DEBUG
    NSLog(@"EnumFormatEtc failed");
#endif
    return nil;
   }

   while(YES){
    FORMATETC format;
    NSString *type=nil;

    if(enumerator->lpVtbl->Next(enumerator,1,&format,NULL)!=S_OK)
     break;

    switch(format.cfFormat){
     case CF_TEXT:
      type=NSStringPboardType;
      break;

     case CF_BITMAP: 
     break;

     case CF_METAFILEPICT:
      break;

     case CF_SYLK:
      break;

     case CF_DIF:
      break;

     case CF_TIFF:
      break;

     case CF_OEMTEXT:
      break;

		case CF_DIBV5:
			type=NSTIFFPboardType;
			break;
			
		case CF_PALETTE:
      break;

     case CF_PENDATA:
      break;

     case CF_RIFF:
      break;

     case CF_WAVE:
      break;

     case CF_UNICODETEXT:
      type=NSStringPboardType;
      break;

     case CF_ENHMETAFILE:
      break;

     case CF_HDROP:
      type=NSFilenamesPboardType;
      break;

     case CF_LOCALE:
      break;

    default:{
      char      name[2048];
      int       length;

      if((length=GetClipboardFormatName(format.cfFormat,name,2048))>0)
       type=[NSString stringWithCString:name length:length];
        if ([type isEqualToString:@"Rich Text Format"]) {
            type = NSRTFPboardType;
        }
     }
     break;
    }
    if(type!=nil)
     [result addObject:type];

    if(format.ptd!=NULL)
     CoTaskMemFree(format.ptd);

    switch(format.dwAspect){
     case DVASPECT_CONTENT:
      break;

     case DVASPECT_THUMBNAIL:
      break;

     case DVASPECT_ICON:
      break;

     case DVASPECT_DOCPRINT:
      break;

     default:
      break;
    }

    // format.lindex

    if(format.tymed&TYMED_HGLOBAL)
     ;
    if(format.tymed&TYMED_FILE)
     ;
    if(format.tymed&TYMED_ISTREAM)
     ;
    if(format.tymed&TYMED_ISTORAGE)
     ;
    if(format.tymed&TYMED_GDI)
     ;
    if(format.tymed&TYMED_MFPICT)
     ;
    if(format.tymed&TYMED_ENHMF)
     ;
    if(format.tymed&TYMED_NULL)
     ;
   }

   enumerator->lpVtbl->Release(enumerator);

   return result;
}

-(NSData *)dataForType:(NSString *)type {
	NSData   *result=nil;
	BOOL      copyData=YES;
	FORMATETC formatEtc;
	STGMEDIUM storageMedium;
	
	if([type isEqualToString:NSStringPboardType])
		formatEtc.cfFormat=CF_UNICODETEXT;
	else if([type isEqualToString:NSFilenamesPboardType])
		formatEtc.cfFormat=CF_HDROP;
	else if([type isEqualToString:NSTIFFPboardType]) {
		// TIFF data can actually arrive as DIB data (from a paste for example), so we'll have to convert it.
		// But in the eventuality that it's not (from a drag within our app), we'll go back to trying TIFF to in a moment.
		formatEtc.cfFormat=CF_DIBV5;
	} else {
        if ([type isEqualToString:NSRTFPboardType]) {
            type = @"Rich Text Format";
        }

		if((formatEtc.cfFormat=RegisterClipboardFormat([type cString]))==0){
#if DEBUG
			NSLog(@"RegisterClipboardFormat failed for type: %@", type);
#endif
			return nil;
		}
	}
	
	formatEtc.ptd=NULL;
	formatEtc.dwAspect=DVASPECT_CONTENT;
	formatEtc.lindex=-1;
	formatEtc.tymed=TYMED_HGLOBAL|TYMED_ISTREAM;
	
	HRESULT hresult = S_OK;
	hresult = _dataObject->lpVtbl->QueryGetData(_dataObject,&formatEtc);
	if (hresult != S_OK) {
		if (formatEtc.cfFormat == CF_DIBV5) {
			// Perhaps it's really a TIFF? RegisterClipboardFormat seems to conjure a cfFormat value
			// that QueryGetData likes. CF_TIFF surprisingly is not that value - so there's some Win32 magic
			// happening somewhere.
			formatEtc.cfFormat=RegisterClipboardFormat([type cString]);
			hresult = _dataObject->lpVtbl->QueryGetData(_dataObject,&formatEtc);
		}
	}
	
	if (hresult != S_OK) {
#if DEBUG
		NSLog(@"QueryGetData failed for type: %@", type);
#endif
		return nil;
	}
	
	_dataObject->lpVtbl->GetData(_dataObject,&formatEtc,&storageMedium);
	
	switch(storageMedium.tymed){
			
		case TYMED_GDI: // hBitmap
			break;
			
		case TYMED_MFPICT: // hMetaFilePict
			break;
			
		case TYMED_ENHMF: // hEnhMetaFile
			break;
			
		case TYMED_HGLOBAL: 
		{ // hGlobal 
			uint8_t  *bytes=GlobalLock(storageMedium.hGlobal); 
			NSUInteger byteLength=GlobalSize(storageMedium.hGlobal); 
			if(formatEtc.cfFormat==CF_DIBV5 && (byteLength > 0)) {
				NSBitmapImageRep *imageRep = nil;
				
				// Make TIFF data from the DIB data
				LPBITMAPINFO    lpBI = (LPBITMAPINFO)bytes;
				void*            pDIBBits = (LPBYTE)lpBI + (WORD)lpBI->bmiHeader.biSize + PaletteSize(lpBI); 
				int w = ABS(lpBI->bmiHeader.biWidth);
				int h = ABS(lpBI->bmiHeader.biHeight);

				// To convert the DIB into data Cocotron can understand, we'll draw the DIB into a CGImage 
				// and return a TIFF representation of it
				CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
				CGContextRef ctx = CGBitmapContextCreate(NULL, w, h, 8, 4*w, colorspace, kCGBitmapByteOrder32Little|kCGImageAlphaPremultipliedFirst);
				CGColorSpaceRelease(colorspace);
				// Contexts created on the Win32 platform are supposed to have a "dc" method
				HDC dc = (HDC)[(O2Context_gdi *)ctx dc];
				if (dc) {
					StretchDIBits(
								  dc,
								  0, 0, ABS(w), ABS(h),
								  0, 0, ABS(w), ABS(h),
								  pDIBBits, lpBI, DIB_RGB_COLORS, SRCCOPY
								  );
					if (lpBI->bmiHeader.biBitCount != 32) {
						// We need to manually set the alpha to 0xFF or we get a transparent image
						char *bytes = CGBitmapContextGetData(ctx);
						for (int i = 3; i < 4*w*h; i+=4) {
							bytes[i] = 0xff; 
						}
					}
					CGImageRef image = CGBitmapContextCreateImage(ctx);
					if (image) {
						imageRep = [[[NSBitmapImageRep alloc] initWithCGImage: image] autorelease];
						CGImageRelease(image);
					}
				}
				CGContextRelease(ctx);
				
				if (imageRep) {
					result = [imageRep TIFFRepresentation];
				}
				
			} else if(formatEtc.cfFormat==CF_UNICODETEXT && (byteLength > 0)) { 
                if(byteLength % 2)  { // odd data length. WTF? 
                    uint8_t lastbyte = bytes[byteLength-1]; 
                    if(lastbyte != 0) { // not a null oddbyte, log it. 
                        NSLog(@"%s:%u[%s] -- \n*****CF_UNICODETEXT byte count not even and odd byte (%0X,'%c') not null",__FILE__, __LINE__, __PRETTY_FUNCTION__,(unsigned)lastbyte, 
							  lastbyte); 
                    } 
					--byteLength; // truncate regardless 
                }  
				
                while(byteLength>0)  { // zortch any terminating null unichars 
                    if(((unichar *) bytes)[(byteLength-2)/2] != 0) { 
                        break; 
                    } 
                    else { 
                        byteLength -= 2; 
                    } 
                }; 
				
				/* check for BOM, if not it is big endian. */
                if(byteLength>=2){
					if(bytes[0]==0xFE && bytes[1]==0xFF){
						copyData=NO;
						bytes=(uint8_t *)NSUnicodeFromBytesUTF16BigEndian(bytes+2, byteLength-2, &byteLength);
						byteLength*=2;
					}
					else if(bytes[0]==0xFF && bytes[1]==0xFE){
						copyData=NO;
						bytes=(uint8_t *)NSUnicodeFromBytesUTF16LittleEndian(bytes+2, byteLength-2, &byteLength);
						byteLength*=2;
					}
                }
                if(copyData){
					copyData=NO;
					bytes=(uint8_t *)NSUnicodeFromBytesUTF16BigEndian(bytes, byteLength, &byteLength);
					byteLength*=2;
                }
			}
			if (result == nil) {
				if(copyData)
					result=[NSData dataWithBytes:bytes length:byteLength]; 
				else
					result=[NSData dataWithBytesNoCopy:bytes length:byteLength freeWhenDone:YES]; 
			}				
            GlobalUnlock(storageMedium.hGlobal); 
		}
			break; 
			
		case TYMED_FILE: // lpszFileName
			break;
			
		case TYMED_ISTREAM:{ // pstm
			Win32IStreamClient *stream=[[Win32IStreamClient alloc] initWithIStream:storageMedium.pstm release:NO];
			
			result=[stream readDataToEndOfFile];
			
			[stream release];
		}
			break;
			
		case TYMED_ISTORAGE: // pstg
			break;
	}
	
	ReleaseStgMedium(&storageMedium);
	
	return result;
}

-(NSArray *)filenamesFromDROPFILES:(const DROPFILES *)dropFiles {
   NSMutableArray *result=[NSMutableArray array];
   const void     *fileBytes=((char *)dropFiles)+dropFiles->pFiles;
   BOOL            unicode=dropFiles->fWide;

   if(unicode){
    const unichar *ptr=fileBytes;
    unsigned length=0;

    while(ptr[length]!='\0'){
     while(ptr[length]!='\0')
      length++;
     [result addObject:[NSString stringWithCharacters:ptr length:length]];
     length++;
     ptr+=length;
     length=0;
    }
   }
   else {
    const char *ptr=fileBytes;
    unsigned length=0;

    while(ptr[length]!='\0'){
     while(ptr[length]!='\0')
      length++;

     [result addObject:[NSString stringWithCString:ptr length:length]];
     length++;
     ptr+=length;
     length=0;
    }
   }

   return result;
}

-(NSArray *)filenames {
   NSData *data=[self dataForType:NSFilenamesPboardType];

   if(data==nil)
    return nil;

   return [self filenamesFromDROPFILES:[data bytes]];
}

@end
