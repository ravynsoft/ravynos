/* WIN32GState - Implements graphic state drawing for MSWindows

   Copyright (C) 2002 Free Software Foundation, Inc.

   Written by: Fred Kiefer <FredKiefer@gmx.de>
   Additions by: Christopher Armstrong <carmstrong@fastmail.com.au>

   Date: March 2002
   
   This file is part of the GNU Objective C User Interface Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

// Currently the use of alpha blending is switched off by default.
#define USE_ALPHABLEND

// Define this so we pick up AlphaBlend, when loading windows.h
#ifdef USE_ALPHABLEND
#if	!defined(WINVER)
#define WINVER 0x0500
#elif (WINVER < 0x0500)
#undef	WINVER
#define WINVER 0x0500
#endif
#endif

#import <Foundation/NSData.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSString.h>

#import <AppKit/NSAffineTransform.h>
#import <AppKit/NSBezierPath.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSFont.h>
#import <AppKit/NSGraphics.h>

#import "winlib/WIN32GState.h"
#import "winlib/WIN32Context.h"
#import "winlib/WIN32FontInfo.h"
#import "win32/WIN32Server.h"

#include <math.h>
#include <limits.h>

#ifndef AC_SRC_ALPHA
// Missing definitions from wingdi.h
#define AC_SRC_ALPHA    0x01
#endif

static inline
int WindowHeight(HWND window)
{
  RECT rect;

  if (!window)
    {
      NSLog(@"No window for coordinate transformation.");
      return 0;
    }

  if (!GetClientRect(window, &rect))
    {
      NSLog(@"No window rectangle for coordinate transformation.");
      return 0;
    }

  return rect.bottom - rect.top;
}

static inline
POINT GSWindowPointToMS(WIN32GState *s, NSPoint p)
{
  POINT p1;

  p1.x = p.x - s->offset.x;
  p1.y = s->offset.y - p.y;
  
  return p1;
}

static inline
RECT GSWindowRectToMS(WIN32GState *s, NSRect r)
{
  RECT r1;

  r1.left = r.origin.x - s->offset.x;
  r1.bottom = s->offset.y - r.origin.y;
  r1.right = r1.left + r.size.width;
  r1.top = r1.bottom - r.size.height;

  return r1;
}

static inline
POINT GSViewPointToWin(WIN32GState *s, NSPoint p)
{
  p = [s->ctm transformPoint: p];
  return GSWindowPointToMS(s, p);
}

static inline
RECT GSViewRectToWin(WIN32GState *s, NSRect r)
{
  r = [s->ctm rectInMatrixSpace: r];
  return GSWindowRectToMS(s, r);
}

void* get_bits(HDC dc,
	       int w,
	       int h,
	       HBITMAP *bitmap)
{
  void          *bits = NULL;
  BITMAPINFO     info;
  HDC            cDC;

  info.bmiHeader.biSize = sizeof(BITMAPINFO);
  info.bmiHeader.biWidth = w;
  info.bmiHeader.biHeight = h;
  info.bmiHeader.biPlanes = 1;
  info.bmiHeader.biBitCount = 32;
  info.bmiHeader.biCompression = BI_RGB;
  info.bmiHeader.biSizeImage = 0;
  info.bmiHeader.biXPelsPerMeter = 0;
  info.bmiHeader.biYPelsPerMeter = 0;
  info.bmiHeader.biClrUsed = 0;
  info.bmiHeader.biClrImportant = 0;

  if(!(cDC = CreateCompatibleDC(dc)))
    {
      NSLog(@"Could not create compatible DC");
      return NULL;
    }
  
  if(!(*bitmap = CreateDIBSection(dc, (LPBITMAPINFO)&info, 
				  DIB_RGB_COLORS, &bits, 
				  NULL, 0)))
    {
      NSLog(@"Could not create bit map from DC");
      return NULL;
    }
  
  SelectObject(cDC, *bitmap);
  
  return bits;
}

BOOL alpha_blend_source_over(HDC destDC, 
			     HDC srcDC, 
			     RECT rectFrom, 
			     int x, int y, int w, int h, 
			     CGFloat delta)
{
  BOOL success = YES;

#ifdef USE_ALPHABLEND
  // Use (0..1) fraction to set a (0..255) alpha constant value
  BYTE SourceConstantAlpha = (BYTE)(delta * 255);
  BLENDFUNCTION blendFunc
    = {AC_SRC_OVER, 0, SourceConstantAlpha, AC_SRC_ALPHA};

  /* There is actually a very real chance this could fail, even on 
     computers that supposedly support it. It's not known why it
     fails though... */
  success = AlphaBlend(destDC,
		       x, y, w, h,
		       srcDC,
		       rectFrom.left, rectFrom.top,
		       w, h, blendFunc);
  // #else
  // HBITMAP    sbitmap;
  // HBITMAP    dbitmap; 
  // unsigned char *sbits = (unsigned char *)get_bits(srcDC,w,h,&sbitmap);
  // unsigned char *dbits = (unsigned char *)get_bits(destDC,w,h,&dbitmap);

#endif

  return success;
}

@interface WIN32GState (WinOps)
- (void) setStyle: (HDC)hDC;
- (void) restoreStyle: (HDC)hDC;
- (HDC) getHDC;
- (void) releaseHDC: (HDC)hDC;
@end

@implementation WIN32GState 

- (id) deepen
{
  [super deepen];

  if (clipRegion)
    {
      HRGN newClipRegion;

      newClipRegion = CreateRectRgn(0, 0, 1, 1);
      CombineRgn(newClipRegion, clipRegion, NULL, RGN_COPY);
      clipRegion = newClipRegion;
    }

  oldBrush = NULL;
  oldPen = NULL;
  oldClipRegion = NULL;

  return self;
}

- (void) dealloc
{
  DeleteObject(clipRegion);
  [super dealloc];
}

- (void) setWindow: (HWND)number
{
  window = number;
}

- (HWND) window
{
  return window;
}

- (void) setColor: (device_color_t *)acolor state: (color_state_t)cState
{
  device_color_t color;
  [super setColor: acolor state: cState];
  color = *acolor;
  gsColorToRGB(&color);
  if (cState & COLOR_FILL)
    wfcolor = RGB(color.field[0]*255, color.field[1]*255, color.field[2]*255);
  if (cState & COLOR_STROKE)
    wscolor = RGB(color.field[0]*255, color.field[1]*255, color.field[2]*255);
}

// NOTE: Ideally this code should be moved to -DPSinitgraphics and not called 
// directly, but this would imply to rewrite WIN32GState more extensively since 
// the current code expects the GDI base coordinates in many methods.
- (void) setUpAppKitBaseCoordinatesForHDC: (HDC)hDC
{
  int surfaceHeight = WindowHeight(window);
  int bottomOffset = offset.y - surfaceHeight;
  XFORM xForm;

  // NOTE: The world transform is the GDI CTM.
  GetWorldTransform(hDC, &oldWorldTransform);

  // Enables the use of transforms
  SetGraphicsMode(hDC, GM_ADVANCED);
  // Maps units to pixels
  SetMapMode(hDC, MM_TEXT);

  /* Flip the GDI base coordinate to match the AppKit because GDI draws the 
     other way around with MM_TEXT.
     Also offset the drawing area to take in account the window border. */

  xForm.eM11 = (FLOAT)1; 
  xForm.eM12 = (FLOAT)0; 
  xForm.eM21 = (FLOAT)0; 
  xForm.eM22 = (FLOAT)-1; 
  xForm.eDx  = (FLOAT)-offset.x; 
  xForm.eDy  = (FLOAT)surfaceHeight + bottomOffset;

  SetWorldTransform(hDC, &xForm);
}

- (void) restoreGDIBaseCoordinatesForHDC: (HDC)hDC
{
  SetWorldTransform(hDC, &oldWorldTransform);
  SetGraphicsMode(hDC, GM_COMPATIBLE);
}

/* For debugging */
- (void) drawOrientationMarkersIn: (HDC)hDC
{
  RECT rect1 = { 0, 0, 20, 10 };
  RECT rect2 = { 0, 30, 20, 30 + 10 };
  FillRect(hDC, &rect1, CreateSolidBrush(RGB(255, 0, 255)));
  FillRect(hDC, &rect2, CreateSolidBrush(RGB(0, 255, 0)));
}

// FIXME: Drawing rotated images is broken (the origin is wrong).
// If rewritten, viewIsFlipped  check must be removed.
- (void) compositeGState: (WIN32GState *)source
		fromRect: (NSRect)sourceRect
		 toPoint: (NSPoint)destPoint
		      op: (NSCompositingOperation)op
		fraction: (CGFloat)delta
{
  HDC sourceDC;
  HDC hDC;
  RECT rectFrom;
  RECT rectTo;
  int h, w;
  int x;
  int y;
  NSRect destRect;
  BOOL success = NO;

  NSDebugLLog(@"WIN32GState",
    @"compositeGState: fromRect: %@ toPoint: %@ op: %d",
    NSStringFromRect(sourceRect), NSStringFromPoint(destPoint), op);

  if (viewIsFlipped && (self != source))
    {
      destPoint.y -= sourceRect.size.height;
    }

  destRect.origin = destPoint;
  destRect.size = sourceRect.size;
  [ctm boundingRectFor: destRect result: &destRect];
  rectTo = GSWindowRectToMS(self, destRect);
  x = rectTo.left;
  y = rectTo.bottom - sourceRect.size.height;

  {
    NSRect newRect;

    [source->ctm boundingRectFor: sourceRect result: &newRect];
    rectFrom = GSWindowRectToMS(source, newRect);
    y += (sourceRect.size.height - newRect.size.height); // adjust location for scaled source
  }
  h = rectFrom.bottom - rectFrom.top;
  w = rectFrom.right - rectFrom.left;

  sourceDC = [source getHDC];
  if (!sourceDC)
    {
      return;
    }

  if (self == source)
    {
      hDC = sourceDC;
    }
  else
    {
      hDC = [self getHDC];
      if (!hDC)
        {
          [source releaseHDC: sourceDC];
          return;
        }
    }

  switch (op)
    {
    case NSCompositeSourceOver:
    case NSCompositeHighlight:
      {
	success = alpha_blend_source_over(hDC, 
					  sourceDC, 
					  rectFrom, 
					  x, y, w, h, 
					  delta);
	if (success)
	  break;
      }
    case NSCompositeCopy:
      {
        success = BitBlt(hDC, x, y, w, h,
                         sourceDC, rectFrom.left, rectFrom.top, SRCCOPY);
        break;	      
      }
    case NSCompositeClear:
      {
        break;
      }
    default:
      success = BitBlt(hDC, x, y, w, h,
                       sourceDC, rectFrom.left, rectFrom.top, SRCCOPY);
      break;
    }

  if (!success)
    {
      NSLog(@"Blit operation failed %d", GetLastError());
      NSLog(@"Orig Copy Bits to %@ from %@", NSStringFromPoint(destPoint),
            NSStringFromRect(destRect));
      NSLog(@"Copy bits to {%d, %d} from {%d, %d} size {%d, %d}", x, y,
            rectFrom.left, rectFrom.top, w, h);
    }

  if (self != source)
    {
      [self releaseHDC: hDC];
    }
  [source releaseHDC: sourceDC];
}

- (void) compositerect: (NSRect)aRect
                    op: (NSCompositingOperation)op
{
  CGFloat gray;

  // FIXME: This is taken from the xlib backend
  [self DPScurrentgray: &gray];
  if (fabs(gray - 0.667) < 0.005)
    [self DPSsetgray: 0.333];
  else    
    [self DPSsetrgbcolor: 0.121 : 0.121 : 0];

  switch (op)
    {
      case   NSCompositeClear:
	break;
      case   GSCompositeHighlight:
	{
	  HDC hDC;
	  RECT rect = GSViewRectToWin(self, aRect);

	  hDC = [self getHDC];
	  if (!hDC)
	    {
	      return;
	    } 

	  InvertRect(hDC, &rect);
	  [self releaseHDC: hDC];
	  break;
	}
      case   NSCompositeCopy:
      // FIXME
      case   NSCompositeSourceOver:
      case   NSCompositeHighlight:
      case   NSCompositeSourceIn:
      case   NSCompositeSourceOut:
      case   NSCompositeSourceAtop:
      case   NSCompositeDestinationOver:
      case   NSCompositeDestinationIn:
      case   NSCompositeDestinationOut:
      case   NSCompositeDestinationAtop:
      case   NSCompositeXOR:
      case   NSCompositePlusDarker:
      case   NSCompositePlusLighter:
      default:
	[self DPSrectfill: NSMinX(aRect) : NSMinY(aRect) 
	      : NSWidth(aRect) : NSHeight(aRect)];
	break;
    }
}

// FIXME: Drawing images with alpha blending is broken
- (void) drawGState: (WIN32GState *)source 
           fromRect: (NSRect)aRect 
            toPoint: (NSPoint)aPoint 
                 op: (NSCompositingOperation)op
           fraction: (CGFloat)delta
{
  HDC sourceDC;
  HDC hDC;
  XFORM xForm, xForm2;
  NSAffineTransformStruct tstruct = [ctm transformStruct];
  RECT rectFrom = GSWindowRectToMS(source, aRect);
  int x = aPoint.x;
  int y = aPoint.y;
  int w = aRect.size.width;
  int h = aRect.size.height;
  BOOL success = YES;

  sourceDC = [source getHDC];
  if (!sourceDC)
    {
      return;
    } 

  if (self == source)
    {
      hDC = sourceDC;
    }
  else
    {
      hDC = [self getHDC];
      if (!hDC)
        {
          [source releaseHDC: sourceDC];
          return;
        }
    }

  /* Set up the AppKit base coordinates as the World transform */

  [self setUpAppKitBaseCoordinatesForHDC: hDC];

  /* Apply the AppKit CTM */

  xForm.eM11 = (FLOAT)tstruct.m11; 
  xForm.eM12 = (FLOAT)tstruct.m12; 
  xForm.eM21 = (FLOAT)tstruct.m21; 
  xForm.eM22 = (FLOAT)tstruct.m22; 
  xForm.eDx  = (FLOAT)tstruct.tX;
  xForm.eDy  = (FLOAT)tstruct.tY;

  // Concat the transform by prepending it to the CTM
  ModifyWorldTransform(hDC, &xForm, MWT_LEFTMULTIPLY);

  /* Flip the CTM to compensate the fact that images are drawn upside down 
     by -DPSimage which uses the GDI top left origin coordinates. */

  xForm2.eM11 = (FLOAT)1;
  xForm2.eM12 = (FLOAT)0; 
  xForm2.eM21 = (FLOAT)0; 
  xForm2.eM22 = (FLOAT)-1; 
  xForm2.eDx  = (FLOAT)0; 
  xForm2.eDy  = (FLOAT)y * 2 + aRect.size.height;

  // Concat the flip transform by prepending it to the CTM
  ModifyWorldTransform(hDC, &xForm2, MWT_LEFTMULTIPLY);
  
  switch (op)
    {
    case NSCompositeSourceOver:
    case NSCompositeHighlight:
      {
	success = alpha_blend_source_over(hDC, 
					  sourceDC, 
					  rectFrom, 
					  x, y, w, h, 
					  delta);
	if (success)
	  break;
      }
    case NSCompositeCopy:
      {
        success = BitBlt(hDC, x, y, w, h,
                         sourceDC, rectFrom.left, rectFrom.top, SRCCOPY);
        break;	      
      }
    case NSCompositeClear:
      {
        break;
      }
    default:
      success = BitBlt(hDC, x, y, w, h,
                       sourceDC, rectFrom.left, rectFrom.top, SRCCOPY);
      break;
    }
  //[self drawOrientationMarkersIn: hDC];

  if (!success)
    {
      NSLog(@"Blit operation failed %d", GetLastError());
    }

  [self restoreGDIBaseCoordinatesForHDC: hDC];

  if (self != source)
    {
      [self releaseHDC: hDC];
    }
  [source releaseHDC: sourceDC];
}

static
HBITMAP GSCreateBitmap(HDC hDC, NSInteger pixelsWide, NSInteger pixelsHigh,
		       NSInteger bitsPerSample, NSInteger samplesPerPixel,
		       NSInteger bitsPerPixel, NSInteger bytesPerRow,
		       BOOL isPlanar, BOOL hasAlpha,
		       NSString *colorSpaceName,
		       const unsigned char *const data[5])
{
  const unsigned char *bits = data[0];
  HBITMAP hbitmap;
  BITMAPINFO *bitmap;
  BITMAPINFOHEADER *bmih;
  int xres, yres;
  UINT fuColorUse;

  if (isPlanar
      || (![colorSpaceName isEqualToString: NSDeviceRGBColorSpace]
          && ![colorSpaceName isEqualToString: NSCalibratedRGBColorSpace]
          && ![colorSpaceName isEqualToString: NSDeviceWhiteColorSpace]
          && ![colorSpaceName isEqualToString: NSCalibratedWhiteColorSpace]
          && ![colorSpaceName isEqualToString: NSDeviceBlackColorSpace]
          && ![colorSpaceName isEqualToString: NSCalibratedBlackColorSpace]))
    {
      NSLog(@"Bitmap type currently not supported %d %@",
	isPlanar, colorSpaceName);
      return NULL;
    }

  // default is 8 bit grayscale 
  if (!bitsPerSample)
    bitsPerSample = 8;
  if (!samplesPerPixel)
    samplesPerPixel = 1;

  // FIXME - does this work if we are passed a planar image but no hints ?
  if (!bitsPerPixel)
    bitsPerPixel = bitsPerSample * samplesPerPixel;
  if (!bytesPerRow)
    bytesPerRow = (bitsPerPixel * pixelsWide) / 8;

  // make sure its sane - also handles row padding if hint missing
  while ((bytesPerRow * 8) < (bitsPerPixel * pixelsWide))
    bytesPerRow++;

  if (!(GetDeviceCaps(hDC, RASTERCAPS) &  RC_DI_BITMAP)) 
    {
      NSLog(@"Device %d does not support bitmap operations", hDC);
      return NULL;
    }

  hbitmap = CreateCompatibleBitmap(hDC, pixelsWide, pixelsHigh);
  if (!hbitmap)
    {
      NSLog(@"Failed to CreateCompatibleBitmap (%d, %d). Error %d", 
            pixelsWide, pixelsHigh, GetLastError());
      return NULL;
    }

  if (bitsPerPixel > 8)
    {
      bitmap = malloc(sizeof(BITMAPV4HEADER));
    }
  else 
    {
      // Leave some extra space for colour map. (Currently not used)
      bitmap = malloc(sizeof(BITMAPINFOHEADER) +  
			   (1 << bitsPerPixel) * sizeof(RGBQUAD));
    }
  if (!bitmap)
    {
       NSLog(@"Failed to allocate memory for bitmap. Error %d", GetLastError());
       DeleteObject(hbitmap);
       return NULL;
    }

  bmih = (BITMAPINFOHEADER*)bitmap;
  bmih->biSize = sizeof(BITMAPINFOHEADER);
  bmih->biWidth = pixelsWide;
  // Top down orientation
  bmih->biHeight = -pixelsHigh;
  bmih->biPlanes = 1;
  bmih->biBitCount = bitsPerPixel;
  bmih->biCompression = BI_RGB;
  bmih->biSizeImage = 0;
  xres = GetDeviceCaps(hDC, HORZRES) / GetDeviceCaps(hDC, HORZSIZE);
  yres = GetDeviceCaps(hDC, VERTRES) / GetDeviceCaps(hDC, VERTSIZE);
  bmih->biXPelsPerMeter = xres;
  bmih->biYPelsPerMeter = yres;
  bmih->biClrUsed = 0;
  bmih->biClrImportant = 0;
  fuColorUse = 0;

  if (bitsPerPixel == 8 && samplesPerPixel == 1)
    {
      unsigned char* tmp;
      unsigned int pixels = pixelsHigh * pixelsWide;
      unsigned int i = 0;
      unsigned int j = 0;
 
      bmih->biBitCount = 32;

      NSDebugLLog(@"WIN32GState", @"8bit greyscale picture with pixelsWide:%d "
	@"pixelsHigh:%d", pixelsWide, pixelsHigh);
      
      tmp = malloc(pixels * 4);
      if (!tmp)
        {
          NSLog(@"Failed to allocate temporary memory for bitmap. Error %d", 
                GetLastError());
          free(bitmap);
          DeleteObject(hbitmap);
          return NULL;
        }

      if ([colorSpaceName isEqualToString: NSDeviceWhiteColorSpace] ||
          [colorSpaceName isEqualToString: NSCalibratedWhiteColorSpace])
        {
          while (i < (pixels*4))
            {
	      unsigned char pix;
	      pix = bits[j];
              tmp[i+0] = pix;
              tmp[i+1] = pix;
              tmp[i+2] = pix;
              tmp[i+3] = 0xFF;
	      i+=4;
              j++;
            }
	  }
      else if ([colorSpaceName isEqualToString: NSDeviceBlackColorSpace] ||
               [colorSpaceName isEqualToString: NSCalibratedBlackColorSpace])
        {
          while (i < (pixels*4))
            {
	      unsigned char pix;
	      pix = UCHAR_MAX - bits[j];
              tmp[i+0] = pix;
              tmp[i+1] = pix;
              tmp[i+2] = pix;
              tmp[i+3] = 0xFF;
	      i+=4;
              j++;
            }
	  }
      else
        {
          NSLog(@"Unexpected condition, greyscale which is neither white nor black");
          free(tmp);
          free(bitmap);
          DeleteObject(hbitmap);
          return NULL;
        }
      bits = tmp;
    }
  else if (bitsPerPixel == 16 && samplesPerPixel == 2) // 8 bit greyscale 8 bit alpha
    {
      BITMAPV4HEADER	*bmih;
      unsigned char	*tmp;
      unsigned int	pixels = pixelsHigh * pixelsWide;
      unsigned int	i = 0;
      unsigned int	j = 0;

      ((BITMAPINFOHEADER*)bitmap)->biBitCount = 32;

      bmih = (BITMAPV4HEADER*)bitmap;
      bmih->bV4Size = sizeof(BITMAPV4HEADER);
      bmih->bV4V4Compression = BI_BITFIELDS;
      bmih->bV4BlueMask = 0x000000FF;
      bmih->bV4GreenMask = 0x0000FF00;
      bmih->bV4RedMask = 0x00FF0000;
      bmih->bV4AlphaMask = 0xFF000000;
      tmp = malloc(pixels * 4);
      if (!tmp)
        {
          NSLog(@"Failed to allocate temporary memory for bitmap. Error %d", 
                GetLastError());
          free(bitmap);
          DeleteObject(hbitmap);
          return NULL;
        }

      if ([colorSpaceName isEqualToString: NSDeviceWhiteColorSpace] ||
          [colorSpaceName isEqualToString: NSCalibratedWhiteColorSpace])
        {
          while (i < (pixels*4))
            {
	      unsigned char pix;
	      pix = bits[j];
              tmp[i+0] = pix;
              tmp[i+1] = pix;
              tmp[i+2] = pix;
              tmp[i+3] = bits[j + 1];
	      i += 4;
              j += 2;
            }
	  }
      else if ([colorSpaceName isEqualToString: NSDeviceBlackColorSpace] ||
               [colorSpaceName isEqualToString: NSCalibratedBlackColorSpace])
        {
          while (i < (pixels*4))
            {
	      unsigned char pix;
	      pix = UCHAR_MAX - bits[j];
              tmp[i+0] = pix;
              tmp[i+1] = pix;
              tmp[i+2] = pix;
              tmp[i+3] = bits[j + 1];
	      i += 4;
              j += 2;
            }
	  }
      else
        {
          NSLog(@"Unexpected condition, greyscale which is neither white nor black");
          free(tmp);
          free(bitmap);
          DeleteObject(hbitmap);
          return NULL;
        }
      bits = tmp;
    }
  else if (bitsPerPixel == 32)
    {
      BITMAPV4HEADER	*bmih;
      unsigned char	*tmp;
      unsigned int	pixels = pixelsHigh * pixelsWide;
      unsigned int	i = 0;

      bmih = (BITMAPV4HEADER*)bitmap;
      bmih->bV4Size = sizeof(BITMAPV4HEADER);
      bmih->bV4V4Compression = BI_BITFIELDS;
      bmih->bV4BlueMask = 0x000000FF;
      bmih->bV4GreenMask = 0x0000FF00;
      bmih->bV4RedMask = 0x00FF0000;
      bmih->bV4AlphaMask = 0xFF000000;
      tmp = malloc(pixels * 4);
      if (!tmp)
        {
          NSLog(@"Failed to allocate temporary memory for bitmap. Error %d", 
                GetLastError());
          free(bitmap);
          DeleteObject(hbitmap);
          return NULL;
        }

      while (i < pixels*4)
	{
	  tmp[i+0] = bits[i+2];
	  tmp[i+1] = bits[i+1];
	  tmp[i+2] = bits[i+0];
	  tmp[i+3] = bits[i+3];
	  i += 4;
	}
      bits = tmp;
    }
  else if (bitsPerPixel == 24)
   {
      unsigned char* tmp;
      unsigned int pixels = pixelsHigh * pixelsWide;
      unsigned int i = 0, j = 0;
 
      bmih->biBitCount = 32;

      NSDebugLLog(@"WIN32GState", @"24bit picure with pixelsWide:%d "
	@"pixelsHigh:%d", pixelsWide, pixelsHigh);
      
      tmp = malloc(pixels * 4);
      if (!tmp)
        {
          NSLog(@"Failed to allocate temporary memory for bitmap. Error %d", 
                GetLastError());
          free(bitmap);
          DeleteObject(hbitmap);
          return NULL;
        }

      while (i < (pixels*4))
        {
          // We expand the bytes in a 24bit image into 32bits as Windows
	  // seems to handle it better (and I can't figure out the correct
	  // rearrangement for 24bit images anyway).
          tmp[i+0] = bits[j+2];
          tmp[i+1] = bits[j+1];
          tmp[i+2] = bits[j+0];
          tmp[i+3] = 0xFF;
	  i+=4;
          j+=3;
        }
      bits = tmp;
    }
  else
    {
      if (bitsPerPixel <= 8 && samplesPerPixel > 1)
        {
          // FIXME How to get a colour palette?
          NSLog(@"Need to define colour map for images with %d bits", bitsPerPixel);
          //bitmap->bmiColors;
          //fuColorUse = DIB_RGB_COLORS;
        }
      else
        {
          NSLog(@"Unsure how to handle images with %d bpp %d spp", bitsPerPixel, samplesPerPixel);
        }
      free(bitmap);
      DeleteObject(hbitmap);
      return NULL;
    }

  if (!SetDIBits(hDC, hbitmap, 0, pixelsHigh, bits, bitmap, fuColorUse))
    {
      NSLog(@"SetDIBits failed. Error %d", GetLastError());
      DeleteObject(hbitmap);
      hbitmap = NULL;
    }

  if (bits != data[0])
    {
      /* cast bits to Void Pointer to fix warning in compile */
      free((void *)(bits));
    }
  free(bitmap);
  return hbitmap;
}

// NOTE: Draws the image with the GDI top left coordinate origin rather than 
// the AppKit bottom left coordinate origin. Hence drawing from an image cache 
// window into an AppKit base coordinate space results in an upside down image.
// Ideally we should draw image in their cache windows with the AppKit base 
// coordinates to prevent extra flip transforms to be required later on (see 
// drawGState:fromRect:toPoint:op:fraction).
// For some unknown reason, invoking -setUpAppKitBaseCoordinatesForHDC: in 
// -DPSimage: can cause images to be drawn with a 1px horizontal line cut at 
// the top. That's why -DPSimage we still use the GDI top left coordinates.
- (void)DPSimage: (NSAffineTransform*) matrix 
		: (NSInteger) pixelsWide : (NSInteger) pixelsHigh
		: (NSInteger) bitsPerSample : (NSInteger) samplesPerPixel 
		: (NSInteger) bitsPerPixel : (NSInteger) bytesPerRow : (BOOL) isPlanar
		: (BOOL) hasAlpha : (NSString *) colorSpaceName
		: (const unsigned char *const [5]) data
{
  NSAffineTransform *old_ctm = nil;
  HDC hDC;
  HBITMAP hbitmap;
  HGDIOBJ old;
  HDC hDC2;
  POINT pa[3];

/*
  NSDebugLLog(@"WIN32GState", @"DPSImage : pixelsWide = %d : pixelsHigh = %d"
	      ": bitsPerSample = %d : samplesPerPixel = %d"
	      ": bitsPerPixel = %d : bytesPerRow = %d "
	      ": isPlanar = %d"
	      ": hasAlpha = %d : colorSpaceName = %@",
	      pixelsWide, pixelsHigh,
	      bitsPerSample, samplesPerPixel,
	      bitsPerPixel, bytesPerRow,
	      isPlanar, hasAlpha, colorSpaceName);
*/

  if (window == NULL)
    {
      NSLog(@"No window in DPSImage");
      return;
    }

  hDC = GetDC((HWND)window);
  if (!hDC)
    {
      NSLog(@"No DC for window %d in DPSImage. Error %d", 
	    (int)window, GetLastError());
      return;
    }

  hbitmap = GSCreateBitmap(hDC, pixelsWide, pixelsHigh,
			   bitsPerSample, samplesPerPixel,
			   bitsPerPixel, bytesPerRow,
			   isPlanar, hasAlpha,
			   colorSpaceName, data);
  if (!hbitmap)
    {
      NSLog(@"Created bitmap failed %d", GetLastError());
      ReleaseDC((HWND)window, hDC);
      return;
    }

  hDC2 = CreateCompatibleDC(hDC); 
  if (!hDC2)
    {
      NSLog(@"No Compatible DC for window %d in DPSImage. Error %d", 
	    (int)window, GetLastError());
      DeleteObject(hbitmap);
      ReleaseDC((HWND)window, hDC);
      return;
    }

  old = SelectObject(hDC2, hbitmap);
  if (!old)
    {
      NSLog(@"SelectObject failed for window %d in DPSImage. Error %d", 
	    (int)window, GetLastError());
      DeleteDC(hDC2);
      DeleteObject(hbitmap);
      ReleaseDC((HWND)window, hDC);
      return;
    }

  //SetMapMode(hDC2, GetMapMode(hDC));
  ReleaseDC((HWND)window, hDC);

  hDC = [self getHDC];

  // Apply the additional transformation
  if (matrix)
    {
      old_ctm = [ctm copy];
      [ctm prependTransform: matrix];
    }

  pa[0] = GSViewPointToWin(self, NSMakePoint(0, pixelsHigh));
  pa[1] = GSViewPointToWin(self, NSMakePoint(pixelsWide, pixelsHigh));
  pa[2] = GSViewPointToWin(self, NSMakePoint(0, 0));

  /*if (viewIsFlipped)
    {
      pa[0].y += pixelsHigh;
      pa[1].y += pixelsHigh;
      pa[2].y += pixelsHigh;
    }*/

  if (old_ctm != nil)
    {
      RELEASE(ctm);
      // old_ctm is already retained
      ctm = old_ctm;
    }

  if ((GetDeviceCaps(hDC, RASTERCAPS) &  RC_BITBLT)) 
    {
      SetStretchBltMode(hDC, COLORONCOLOR);

      if (!PlgBlt(hDC, pa, hDC2, 0, 0, pixelsWide, pixelsHigh, 0, 0, 0))
        {
          NSLog(@"Copy bitmap failed %d", GetLastError());
          NSLog(@"DPSimage with %d %d %d %d to %d, %d", pixelsWide, pixelsHigh, 
	        bytesPerRow, bitsPerPixel, pa[0].x, pa[0].y);
        }
    }
  
  [self releaseHDC: hDC];

  SelectObject(hDC2, old);
  DeleteDC(hDC2);
  DeleteObject(hbitmap);
}

@end

@implementation WIN32GState (PathOps)

- (void) _paintPath: (ctxt_object_t) drawType
{
  unsigned count;
  HDC hDC;

  hDC = [self getHDC];
  if (!hDC)
    {
      return;
    } 

  count = [path elementCount];
  if (count)
    {
      NSBezierPathElement type;
      NSPoint   points[3];
      unsigned	j, i = 0;
      POINT p;

      BeginPath(hDC);

      for (j = 0; j < count; j++) 
        {
	  type = [path elementAtIndex: j associatedPoints: points];
	  switch(type) 
	    {
	    case NSMoveToBezierPathElement:
	      p = GSWindowPointToMS(self, points[0]);
	      MoveToEx(hDC, p.x, p.y, NULL);
	      break;
	    case NSLineToBezierPathElement:
	      p = GSWindowPointToMS(self, points[0]);
	      // FIXME This gives one pixel too few
	      LineTo(hDC, p.x, p.y);
	      break;
	    case NSCurveToBezierPathElement:
	      {
		POINT bp[3];
		
		for (i = 0; i < 3; i++)
		  {
		    bp[i] = GSWindowPointToMS(self, points[i]);
		  }
		PolyBezierTo(hDC, bp, 3);
	      }
	      break;
	    case NSClosePathBezierPathElement:
	      CloseFigure(hDC);
	      break;
	    default:
	      break;
	    }
	}  
      EndPath(hDC);

      // Now operate on the path
      switch (drawType)
	{
	case path_stroke:
	  if (strokeColor.field[AINDEX] != 0.0)
	    {
	      StrokePath(hDC);
	    }
	  break;
	case path_eofill:
	  if (fillColor.field[AINDEX] != 0.0)
	    {
	      SetPolyFillMode(hDC, ALTERNATE);
	      FillPath(hDC);
	    }
	  break;
	case path_fill:
	  if (fillColor.field[AINDEX] != 0.0)
	    {
	      SetPolyFillMode(hDC, WINDING);
	      FillPath(hDC);
	    }
	  break;
	case path_eoclip:
	  {
	    HRGN region;

	    SetPolyFillMode(hDC, ALTERNATE);
	    region = PathToRegion(hDC);
	    if (clipRegion)
              {
                CombineRgn(clipRegion, clipRegion, region, RGN_AND);
                DeleteObject(region);
              }
	    else
	      {
		clipRegion = region;
	      }
	    break;
	  }
	case path_clip:
	  {
	    HRGN region;

	    SetPolyFillMode(hDC, WINDING);
	    region = PathToRegion(hDC);
	    if (clipRegion)
              {
                CombineRgn(clipRegion, clipRegion, region, RGN_AND);
                DeleteObject(region);
              }
	    else
	      {
		clipRegion = region;
	      }
	    break;
	  }
	default:
	  break;
	}
    }
  [self releaseHDC: hDC];

  /*
   * clip does not delete the current path, so we only clear the path if the
   * operation was not a clipping operation.
   */
  if ((drawType != path_clip) && (drawType != path_eoclip))
    {
      [path removeAllPoints];
    }
}

- (void)DPSclip 
{
  [self _paintPath: path_clip];
}

- (void)DPSeoclip 
{
  [self _paintPath: path_eoclip];
}

- (void)DPSeofill 
{
  if (pattern != nil)
    {
      [self eofillPath: path withPattern: pattern];
      return;
    }
  [self _paintPath: path_eofill];
}

- (void)DPSfill 
{
  if (pattern != nil)
    {
      [self fillPath: path withPattern: pattern];
      return;
    }

  [self _paintPath: path_fill];
}

- (void)DPSstroke 
{
  [self _paintPath: path_stroke];
}

- (void) DPSinitclip
{
  if (clipRegion)
    {
      DeleteObject(clipRegion);
      clipRegion = NULL;
    }
}

- (void)DPSshow: (const char *)s 
{
  NSPoint current = [path currentPoint];
  POINT p;
  HDC hDC;

  hDC = [self getHDC];
  if (!hDC)
    {
      return;
    } 

  p = GSWindowPointToMS(self, current);
  [(WIN32FontInfo*)font draw: s length:  strlen(s)
		   onDC: hDC at: p];
  [self releaseHDC: hDC];
}


- (void) GSShowGlyphsWithAdvances: (const NSGlyph *)glyphs : (const NSSize *)advances : (size_t) length
{
  // FIXME: Currently advances is ignored
  NSPoint current = [path currentPoint];
  POINT p;
  HDC hDC;

  hDC = [self getHDC];
  if (!hDC)
    {
      return;
    } 

  p = GSWindowPointToMS(self, current);
  [(WIN32FontInfo*)font drawGlyphs: glyphs
			    length: length
			      onDC: hDC
				at: p];
  [self releaseHDC: hDC];
}
@end

@implementation WIN32GState (GStateOps)

- (void)DPSinitgraphics 
{
  [super DPSinitgraphics];
}


- (void) DPSsetdash: (const CGFloat*)thePattern : (NSInteger)count : (CGFloat)phase
{
  if (!path)
    {
      path = [NSBezierPath new];
    }

  // FIXME: Convert to ctm first
  [path setLineDash: thePattern count: count phase: phase];
}

- (void)DPScurrentmiterlimit: (CGFloat *)limit 
{
  *limit = miterlimit;
}

- (void)DPSsetmiterlimit: (CGFloat)limit 
{
  // FIXME: Convert to ctm first
  miterlimit = limit;
}

- (void)DPScurrentlinecap: (int *)linecap 
{
  *linecap = lineCap;
}

- (void)DPSsetlinecap: (int)linecap 
{
  lineCap = linecap;
}

- (void)DPScurrentlinejoin: (int *)linejoin 
{
  *linejoin = joinStyle;
}

- (void)DPSsetlinejoin: (int)linejoin 
{
  joinStyle = linejoin;
}

- (void)DPScurrentlinewidth: (CGFloat *)width 
{
  *width = lineWidth;
}

- (void)DPSsetlinewidth: (CGFloat)width 
{
  // FIXME: Convert to ctm first
  lineWidth = width;
}

- (void)DPScurrentstrokeadjust: (int *)b 
{
}

- (void)DPSsetstrokeadjust: (int)b 
{
}

@end

@implementation WIN32GState (WinOps)

- (void) setStyle: (HDC)hDC
{
  HPEN pen;
  HBRUSH brush;
  LOGBRUSH br; 
  int join;
  int cap;
  DWORD penStyle;

  // Temporary variables for gathering pen information
  DWORD* iPattern = NULL;
  NSInteger patternCount = 0;
  
  SetBkMode(hDC, TRANSPARENT);
  br.lbStyle = BS_SOLID;
  br.lbColor = wfcolor;
  br.lbHatch = 0;
  /*
  brush = CreateBrushIndirect(&br);
  */
  brush = CreateSolidBrush(wfcolor);
  oldBrush = SelectObject(hDC, brush);

  switch (joinStyle)
    {
      case NSBevelLineJoinStyle:
	join = PS_JOIN_BEVEL;
	break;
      case NSMiterLineJoinStyle:
	join = PS_JOIN_MITER;
	break;
      case NSRoundLineJoinStyle:
	join = PS_JOIN_ROUND;
	break;
      default:
	join = PS_JOIN_MITER;
	break;
    }

  switch (lineCap)
    {
      case NSButtLineCapStyle:
	cap = PS_ENDCAP_FLAT;
	break;
      case NSSquareLineCapStyle:
	cap = PS_ENDCAP_SQUARE;
	break;
      case NSRoundLineCapStyle:
	cap = PS_ENDCAP_ROUND;
	break;
      default:
	cap = PS_ENDCAP_SQUARE;
	break;
    }
  
  // Get the size of the pen line dash
  [path getLineDash: NULL count: &patternCount phase: NULL];
  
  if (patternCount > 0)
    {
      NSInteger i = 0;
      CGFloat* thePattern[patternCount];
      CGFloat phase = 0.0;

      penStyle = PS_GEOMETRIC | PS_USERSTYLE;

      // The user has defined a dash pattern for stroking on
      // the path. Note that we lose the floating point information
      // here, as windows only supports DWORD elements, not float.
      [path getLineDash: thePattern count: &patternCount phase: &phase];

      iPattern = malloc(sizeof(DWORD) * patternCount);
      for (i = 0 ; i < patternCount; i ++)
        {
          iPattern[i] = (DWORD)thePattern[i];
        }
    }
  else
    {
      penStyle = PS_GEOMETRIC | PS_SOLID;
    }

  pen = ExtCreatePen(penStyle | join | cap, 
		     lineWidth,
		     &br,
		     patternCount, iPattern);

  if (iPattern)
    {
      free(iPattern);
      iPattern = NULL;
    }

  oldPen = SelectObject(hDC, pen);

  SetMiterLimit(hDC, miterlimit, NULL);

  SetTextColor(hDC, wfcolor);

  oldClipRegion = CreateRectRgn(0, 0, 1, 1);
  if (1 != GetClipRgn(hDC, oldClipRegion))
    {
      DeleteObject(oldClipRegion);
      oldClipRegion = NULL;
    }

  SelectClipRgn(hDC, clipRegion);
}

- (void) restoreStyle: (HDC)hDC
{
  HGDIOBJ old;

  SelectClipRgn(hDC, oldClipRegion);
  DeleteObject(oldClipRegion);
  oldClipRegion = NULL;

  old = SelectObject(hDC, oldBrush);
  DeleteObject(old);

  old = SelectObject(hDC, oldPen);
  DeleteObject(old);
}

- (HDC) getHDC
{
  WIN_INTERN *win;
  HDC hDC;

  if (NULL == window)
    {
      //NSLog(@"No window in getHDC");
      return NULL;
    }

  win = (WIN_INTERN *)GetWindowLong((HWND)window, GWL_USERDATA);
  if (win && win->useHDC)
    {
      hDC = win->hdc;
      //NSLog(@"getHDC found DC %d", hDC);
    }
  else
    {
      hDC = GetDC((HWND)window);    
      //NSLog(@"getHDC using window DC %d", hDC);
    }
  
  if (!hDC)
    {
      //NSLog(@"No DC in getHDC");
      return NULL;	
    }

  [self setStyle: hDC];
  return hDC;
}

- (void) releaseHDC: (HDC)hDC
{
  WIN_INTERN *win;

  if (NULL == window ||
      NULL == hDC)
    {
      return;
    }

  [self restoreStyle: hDC];
  win = (WIN_INTERN *)GetWindowLong((HWND)window, GWL_USERDATA);
  if (win && !win->useHDC)
    ReleaseDC((HWND)window, hDC);
}

@end


@implementation WIN32GState (PatternColor)

- (void *) saveClip
{
  if (clipRegion)
    {
      HRGN newClipRegion;

      newClipRegion = CreateRectRgn(0, 0, 1, 1);
      CombineRgn(newClipRegion, clipRegion, NULL, RGN_COPY);

      return newClipRegion;
    }
  return clipRegion;
}

- (void) restoreClip: (void *)savedClip
{
  if (clipRegion)
    {
      DeleteObject(clipRegion);
    }
  clipRegion = savedClip;
}

@end

@implementation WIN32GState (ReadRect)

- (NSDictionary *) GSReadRect: (NSRect)r
{
  NSMutableDictionary *dict;
  NSAffineTransform *matrix;
  double x, y;
  NSMutableData *data;
  unsigned char *cdata;
  unsigned char *bits;
  int i = 0;
  HDC hDC;
  HDC hdcMemDC = NULL;
  HBITMAP hbitmap = NULL;
  BITMAP bmpCopied;
  HGDIOBJ old;
  RECT rcClient;
  DWORD dwBmpSize;
  HANDLE hDIB;
  LPBITMAPV4HEADER lpbi;

  if (window == NULL)
    {
      NSLog(@"No window in GSReadRect");
      return nil;
    }

  r = [ctm rectInMatrixSpace: r];
  x = NSWidth(r);
  y = NSHeight(r);

  dict = [NSMutableDictionary dictionary];
  [dict setObject: NSDeviceRGBColorSpace forKey: @"ColorSpace"];

  [dict setObject: [NSNumber numberWithUnsignedInt: 8]
	   forKey: @"BitsPerSample"];
  [dict setObject: [NSNumber numberWithUnsignedInt: 32]
	   forKey: @"Depth"];
  [dict setObject: [NSNumber numberWithUnsignedInt: 4] 
           forKey: @"SamplesPerPixel"];
  [dict setObject: [NSNumber numberWithUnsignedInt: 1]
           forKey: @"HasAlpha"];

  matrix = [self GSCurrentCTM];
  [matrix translateXBy: -r.origin.x - offset.x 
		   yBy: r.origin.y + NSHeight(r) - offset.y];
  [dict setObject: matrix forKey: @"Matrix"];

  hDC = GetDC((HWND)window);
  if (!hDC)
    {
      NSLog(@"No DC for window %d in GSReadRect. Error %d", 
	(int)window, GetLastError());
      return nil;
    }

  // Create a compatible DC which is used in a BitBlt from the window DC
  hdcMemDC = CreateCompatibleDC(hDC); 
  if (!hdcMemDC)
    {
      NSLog(@"No Compatible DC for window %d in GSReadRect. Error %d", 
	(int)window, GetLastError());
      ReleaseDC((HWND)window, hDC);
      return nil;
    }

  ReleaseDC((HWND)window, hDC);
  hDC = [self getHDC];

  // Get the client area for size calculation
  rcClient = GSWindowRectToMS(self, r);

  // Create a compatible bitmap from the Window DC
  hbitmap = CreateCompatibleBitmap(hDC, rcClient.right - rcClient.left, 
  rcClient.bottom - rcClient.top);
  if (!hbitmap)
    {
      NSLog(@"No Compatible bitmap for window %d in GSReadRect. Error %d", 
	(int)window, GetLastError());
      ReleaseDC((HWND)window, hdcMemDC);
      [self releaseHDC: hDC];
      return nil;
    }

  // Select the compatible bitmap into the compatible memory DC.
  old = SelectObject(hdcMemDC, hbitmap);
  if (!old)
    {
      NSLog(@"SelectObject failed for window %d in GSReadRect. Error %d", 
	(int)window, GetLastError());
      DeleteObject(hbitmap);
      ReleaseDC((HWND)window, hdcMemDC);
      [self releaseHDC: hDC];
      return nil;
    }

  // Bit block transfer into our compatible memory DC.
  if (!BitBlt(hdcMemDC, 0, 0, 
    rcClient.right - rcClient.left, 
    rcClient.bottom - rcClient.top, 
    hDC, 
    0, 0,
    SRCCOPY))
    {
      NSLog(@"BitBlt failed for window %d in GSReadRect. Error %d", 
	(int)window, GetLastError());
      DeleteObject(hbitmap);
      ReleaseDC((HWND)window, hdcMemDC);
      [self releaseHDC: hDC];
      return nil;
    }

  // Get the BITMAP from the HBITMAP
  GetObject(hbitmap, sizeof(BITMAP), &bmpCopied);

  dwBmpSize = bmpCopied.bmWidth * 4 * bmpCopied.bmHeight;

  hDIB = GlobalAlloc(GHND, dwBmpSize + sizeof(BITMAPV4HEADER)); 
  lpbi = (LPBITMAPV4HEADER)GlobalLock(hDIB);    
  lpbi->bV4Size = sizeof(BITMAPV4HEADER);
  lpbi->bV4V4Compression = BI_BITFIELDS;
  lpbi->bV4BlueMask = 0x000000FF;
  lpbi->bV4GreenMask = 0x0000FF00;
  lpbi->bV4RedMask = 0x00FF0000;
  lpbi->bV4AlphaMask = 0xFF000000;
  lpbi->bV4Width = bmpCopied.bmWidth;    
  lpbi->bV4Height = bmpCopied.bmHeight;  
  lpbi->bV4Planes = 1;    
  lpbi->bV4BitCount = 32;    
  lpbi->bV4SizeImage = 0;  
  lpbi->bV4XPelsPerMeter = 0;    
  lpbi->bV4YPelsPerMeter = 0;    
  lpbi->bV4ClrUsed = 0;    
  lpbi->bV4ClrImportant = 0;
  bits = (unsigned char *)lpbi + sizeof(BITMAPV4HEADER);

  // Gets the "bits" from the bitmap and copies them into a buffer 
  // which is pointed to by lpbi
  if (GetDIBits(hdcMemDC, hbitmap, 0, (UINT)bmpCopied.bmHeight, bits,
    (LPBITMAPINFO)lpbi, DIB_RGB_COLORS) == 0)
    {
      NSLog(@"GetDIBits failed for window %d in GSReadRect. Error %d", 
	(int)window, GetLastError());
      GlobalUnlock(hDIB);    
      GlobalFree(hDIB);
      DeleteObject(hbitmap);
      ReleaseDC((HWND)window, hdcMemDC);
      [self releaseHDC: hDC];
      return nil;
    }

  data = [NSMutableData dataWithLength: dwBmpSize];
  if (data == nil)
    {
      GlobalUnlock(hDIB);    
      GlobalFree(hDIB);
      DeleteObject(hbitmap);
      ReleaseDC((HWND)window, hdcMemDC);
      [self releaseHDC: hDC];
      return nil;
    }

  // Copy to data
  cdata = [data mutableBytes];
  while (i < dwBmpSize)
    {
      cdata[i+0] = bits[i+2];
      cdata[i+1] = bits[i+1];
      cdata[i+2] = bits[i+0];
      cdata[i+3] = bits[i+3];
      i += 4;
    }


  //Unlock and Free the DIB from the heap
  GlobalUnlock(hDIB);    
  GlobalFree(hDIB);

  //Clean up
  DeleteObject(hbitmap);
  ReleaseDC((HWND)window, hdcMemDC);
  [self releaseHDC: hDC];

  [dict setObject: [NSValue valueWithSize: NSMakeSize(bmpCopied.bmWidth,
  bmpCopied.bmHeight)]
  forKey: @"Size"];
  [dict setObject: data forKey: @"Data"];

  return dict;
}

@end
