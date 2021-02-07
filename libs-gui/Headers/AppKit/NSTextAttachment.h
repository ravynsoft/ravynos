/*
   NSTextAttachment.h

   Classes to represent text attachments.
   
   Copyright (C) 1996 Free Software Foundation, Inc.

   Author:  Daniel Böhringer <boehring@biomed.ruhr-uni-bochum.de>
   Date: August 1998
   Source by Daniel Böhringer integrated into GNUstep gui
   by Felipe A. Rodriguez <far@ix.netcom.com> 
   
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

/**
NSTextAttachment is used to represent text attachments. When inline, 
text attachments appear as the value of the NSAttachmentAttributeName 
attached to the special character NSAttachmentCharacter.
   
NSTextAttachment uses an object obeying the NSTextAttachmentCell 
protocol to get input from the user and to display an image.

NSTextAttachmentCell is a simple subclass of NSCell which provides 
the NSTextAttachment protocol.
*/

#ifndef _GNUstep_H_NSTextAttachment
#define _GNUstep_H_NSTextAttachment

#import <GNUstepBase/GSVersionMacros.h>

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)

#import <Foundation/NSAttributedString.h>
#import <Foundation/NSGeometry.h>
#import <Foundation/NSObject.h>
#import <AppKit/NSCell.h>

@class NSFileWrapper;
@class NSLayoutManager;
@class NSTextAttachment;
@class NSTextContainer;

enum {
    NSAttachmentCharacter = 0xfffc	/* To denote attachments. */
};

/* 
   These are the only methods required of cells in text attachments... 
   The default NSCell class implements most of these; the NSTextAttachmentCell 
   class is a subclass which implements all and provides some additional 
   functionality.
 */
@protocol NSTextAttachmentCell <NSObject>
- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView;
- (void)drawWithFrame:(NSRect)cellFrame 
	       inView:(NSView *)controlView 
       characterIndex:(NSUInteger)charIndex;
- (void)drawWithFrame:(NSRect)cellFrame 
	       inView:(NSView *)controlView 
       characterIndex:(NSUInteger)charIndex
	layoutManager:(NSLayoutManager *)layoutManager;
- (void)highlight:(BOOL)flag 
	withFrame:(NSRect)cellFrame 
	   inView:(NSView *)controlView;


/** The old way of placing the cell. The text system will never call
these directly (TODO: make sure it doesn't), but other things might. The
class implements the new method by calling these, so subclasses can
easily change behavior by overriding these. **/

- (NSSize)cellSize;

/* Returns the offset from the current point when typesetting to the lower
left corner of the cell. The class returns (0,0). Positive y is probably
up. (TODO) */
- (NSPoint)cellBaselineOffset;


/** The new way of placing the cell. **/

/* Returns the rectangle in which the cell should be drawn. The rectangle
is relative to the current point when typesetting. Positive y is up.

lineFrag is the line frag rect that this cell might be placed in, and
position is the current position in that line frag rect (positive y is
up). Note that the line frag rect and glyph position may not be where
the cell is actually placed.

Note that this might be called many times for the same attachment. Eg. if
you return a rectangle that won't fit in the proposed line frag rect, the
typesetter might try to adjust things so it will fit. It will then send
this message with a new proposed line frag rect and glyph position. Thus,
great care must be taken when using the line frag rect to calculate the
returned rectangle to prevent the typesetting process from getting stuck.

The class uses -cellSize and -cellBaselineOffset to return a rect.
*/
-(NSRect) cellFrameForTextContainer: (NSTextContainer *)textContainer
	       proposedLineFragment: (NSRect)lineFrag
		      glyphPosition: (NSPoint)position
		     characterIndex: (NSUInteger)charIndex;


- (BOOL)wantsToTrackMouse;
- (BOOL)wantsToTrackMouseForEvent:(NSEvent *)theEvent 
			   inRect:(NSRect)cellFrame 
			   ofView:(NSView *)controlView
		 atCharacterIndex:(NSUInteger)charIndex;
- (BOOL)trackMouse:(NSEvent *)theEvent 
	    inRect:(NSRect)cellFrame 
	    ofView:(NSView *)controlView 
      untilMouseUp:(BOOL)flag;
- (BOOL)trackMouse:(NSEvent *)theEvent 
	    inRect:(NSRect)cellFrame 
	    ofView:(NSView *)controlView
  atCharacterIndex:(NSUInteger)charIndex 
      untilMouseUp:(BOOL)flag;
- (void)setAttachment:(NSTextAttachment *)anObject;
- (NSTextAttachment *)attachment;
@end


/* 
   Simple class to provide basic attachment cell functionality. 
   By default this class causes NSTextView to send out delegate 
   messages when the attachment is clicked on or dragged.
 */
@interface NSTextAttachmentCell : NSCell <NSTextAttachmentCell> {
    NSTextAttachment *_attachment;
}
@end


@interface NSTextAttachment : NSObject <NSCoding> {
    NSFileWrapper *_fileWrapper;
    id <NSTextAttachmentCell>_cell;
    struct GSTextAttachmentFlagsType { 
      // total 32 bits.  31 bits left.
      unsigned cell_explicitly_set: 1;
      unsigned unused:31;
    } _taflags;
}

/* 
   Designated initializer.
 */
- (id)initWithFileWrapper:(NSFileWrapper *)fileWrapper;

/* 
   The fileWrapper is the meat of most types of attachment.  
   It can be set or queried with these methods.  An NSTextAttachment 
   usually has a fileWrapper.  setFileWrapper does not update the 
   attachment's cell in any way.
 */
- (void)setFileWrapper:(NSFileWrapper *)fileWrapper;
- (NSFileWrapper *)fileWrapper;

/* 
   The cell which handles user interaction. 
   By default an instance of NSTextAttachmentCell is used.
 */
- (id <NSTextAttachmentCell>)attachmentCell;
- (void)setAttachmentCell:(id <NSTextAttachmentCell>)cell;

@end

@interface NSAttributedString (NSTextAttachment)
+ (NSAttributedString*) attributedStringWithAttachment: 
                 (NSTextAttachment*)attachment;
- (BOOL) containsAttachments;
@end

@interface NSMutableAttributedString (NSTextAttachment)
- (void) updateAttachmentsFromPath: (NSString *)path;
@end

#endif

#endif /* _GNUstep_H_NSTextAttachment */

