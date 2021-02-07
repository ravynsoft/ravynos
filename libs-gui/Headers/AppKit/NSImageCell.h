/* 
   NSImageCell.h

   The cell class for NSImage

   Copyright (C) 1999 Free Software Foundation, Inc.

   Author:  Jonathan Gapen <jagapen@chem.wisc.edu>
   Date: 1999
   
   This file is part of the GNUstep GUI Library.

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

#ifndef _GNUstep_H_NSImageCell
#define _GNUstep_H_NSImageCell
#import <GNUstepBase/GSVersionMacros.h>

#import <AppKit/NSCell.h>

/**
 *  <p>Enumeration of the ways that you can align an image inside an
 *  NSImageCell when the image is not taking up all the space inside
 *  the cell (for example, because you are using NSScaleNone or
 *  NSScaleProportionally and the cell size is bigger than the natural
 *  image size).  The available ones are: <code>NSImageAlignCenter,
 *  NSImageAlignTop, NSImageAlignTopLeft, NSImageAlignTopRight,
 *  NSImageAlignLeft, NSImageAlignBottom, NSImageAlignBottomLeft,
 *  NSImageAlignBottomRight, NSImageAlignRight</code>.</p>
 */
typedef enum {
    NSImageAlignCenter = 0,
    NSImageAlignTop,
    NSImageAlignTopLeft,
    NSImageAlignTopRight,
    NSImageAlignLeft,
    NSImageAlignBottom,
    NSImageAlignBottomLeft,
    NSImageAlignBottomRight,
    NSImageAlignRight
} NSImageAlignment;

/**
 *  <p>Enumeration of the types of frame that can be used in an
 *  NSImageCell.  The available ones are: <code>NSImageFrameNone, 
 *  NSImageFramePhoto, NSImageFrameGrayBezel, NSImageFrameGroove,
 *  NSImageFrameButton</code>.</p>
 */
typedef enum {
    NSImageFrameNone = 0,
    NSImageFramePhoto,
    NSImageFrameGrayBezel,
    NSImageFrameGroove,
    NSImageFrameButton
} NSImageFrameStyle;

/**
 *  <p>An NSImageCell is a cell that can display a single image.  It
 *  is normally associated with an NSImageView control; but you can
 *  use it as a building block in your own controls.</p>
 *
 *  <p>The image to display is set using the -setImage: method 
 *  which is inherited from the superclass.</p>
 *
 *  <p>The -setImageAlignment: and -setImageScaling: methods can be
 *  used to control how the image is drawn inside a rectangle which is
 *  larger or smaller than the image size; the image might need to be
 *  scaled, cropped or aligned.</p>
 *
 *  <p>The -setImageFrameStyle: method can be used to control if the
 *  cell should display a frame border, and which one.</p>
 */
@interface NSImageCell : NSCell
{
  NSImageAlignment _imageAlignment;
  NSImageFrameStyle _frameStyle;
  NSImageScaling _imageScaling;
  NSSize _original_image_size;
}

/**
 * Returns the alignment used when displaying the image inside
 * a cell that is bigger than the image, and NSScaleToFit has
 * not been selected.
 */
- (NSImageAlignment) imageAlignment;
/**
 * Sets the alignment used when displaying the image inside a cell
 * that is bigger than the image, and NSScaleToFit has not been
 * selected.
 */
- (void) setImageAlignment: (NSImageAlignment)anAlignment;

/**
 * Returns the type of image scaling used on the image when the
 * image natural size and the cell size are different.
 */
- (NSImageScaling) imageScaling;
/**
 * Sets the type of image scaling used on the image when the image
 * natural size and the cell size are different.
 */
- (void) setImageScaling: (NSImageScaling)scaling;

/**
 * Returns the style used to draw the frame around the image.
 */
- (NSImageFrameStyle) imageFrameStyle;
/**
 * Sets the style used to draw the frame around the image.
 */
- (void) setImageFrameStyle: (NSImageFrameStyle)aFrameStyle;

@end

#endif // _GNUstep_H_NSImageCell
