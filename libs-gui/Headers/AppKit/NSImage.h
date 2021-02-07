/* 
   NSImage.h

   Load, manipulate and display images

   Copyright (C) 1996-2016 Free Software Foundation, Inc.

   Written by:  Adam Fedor <fedor@colorado.edu>
   Date: Feb 1996
   
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

#ifndef _GNUstep_H_NSImage
#define _GNUstep_H_NSImage
#import <GNUstepBase/GSVersionMacros.h>

#import <AppKit/NSGraphicsContext.h>
#import <Foundation/NSBundle.h>
#import <AppKit/NSBitmapImageRep.h>

@class NSString;
@class NSMutableArray;
@class NSData;
@class NSURL;

@class NSPasteboard;
@class NSImageRep;
@class NSColor;
@class NSView;

/* Named images */

APPKIT_EXPORT NSString *NSImageNameMultipleDocuments;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
APPKIT_EXPORT NSString *NSImageNameUserAccounts;
APPKIT_EXPORT NSString *NSImageNamePreferencesGeneral;
APPKIT_EXPORT NSString *NSImageNameAdvanced;
APPKIT_EXPORT NSString *NSImageNameInfo;
APPKIT_EXPORT NSString *NSImageNameFontPanel;
APPKIT_EXPORT NSString *NSImageNameColorPanel;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
APPKIT_EXPORT NSString *NSImageNameTrashEmpty;
APPKIT_EXPORT NSString *NSImageNameTrashFull;
APPKIT_EXPORT NSString *NSImageNameCaution;
#endif

/** Defines how an NSImage is to be cached.  Possible values are:
 *  <list>
 *   <item>NSImageCacheDefault</item>
 *   <item>NSImageCacheAlways</item>
 *   <item>NSImageCacheBySize</item>
 *   <item>NSImageCacheNever</item>
 *  </list>
 *  <p>See Also:</p>
 *  <list>
 *   <item>-setCacheMode:</item>
 *   <item>-cacheMode</item>
 *  </list>
 */
typedef enum {
  NSImageCacheDefault,
  NSImageCacheAlways,
  NSImageCacheBySize,
  NSImageCacheNever
} NSImageCacheMode;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
APPKIT_EXTERN NSString *const NSImageNameQuickLookTemplate;
APPKIT_EXTERN NSString *const NSImageNameBluetoothTemplate;
APPKIT_EXTERN NSString *const NSImageNameIChatTheaterTemplate;
APPKIT_EXTERN NSString *const NSImageNameSlideshowTemplate;
APPKIT_EXTERN NSString *const NSImageNameActionTemplate;
APPKIT_EXTERN NSString *const NSImageNameSmartBadgeTemplate;
APPKIT_EXTERN NSString *const NSImageNameIconViewTemplate;
APPKIT_EXTERN NSString *const NSImageNameListViewTemplate;
APPKIT_EXTERN NSString *const NSImageNameColumnViewTemplate;
APPKIT_EXTERN NSString *const NSImageNameFlowViewTemplate;
APPKIT_EXTERN NSString *const NSImageNamePathTemplate;
APPKIT_EXTERN NSString *const NSImageNameInvalidDataFreestandingTemplate;
APPKIT_EXTERN NSString *const NSImageNameLockLockedTemplate;
APPKIT_EXTERN NSString *const NSImageNameLockUnlockedTemplate;
APPKIT_EXTERN NSString *const NSImageNameGoRightTemplate;
APPKIT_EXTERN NSString *const NSImageNameGoLeftTemplate;
APPKIT_EXTERN NSString *const NSImageNameRightFacingTriangleTemplate;
APPKIT_EXTERN NSString *const NSImageNameLeftFacingTriangleTemplate;
APPKIT_EXTERN NSString *const NSImageNameAddTemplate;
APPKIT_EXTERN NSString *const NSImageNameRemoveTemplate;
APPKIT_EXTERN NSString *const NSImageNameRevealFreestandingTemplate;
APPKIT_EXTERN NSString *const NSImageNameFollowLinkFreestandingTemplate;
APPKIT_EXTERN NSString *const NSImageNameEnterFullScreenTemplate;
APPKIT_EXTERN NSString *const NSImageNameExitFullScreenTemplate;
APPKIT_EXTERN NSString *const NSImageNameStopProgressTemplate;
APPKIT_EXTERN NSString *const NSImageNameStopProgressFreestandingTemplate;
APPKIT_EXTERN NSString *const NSImageNameRefreshTemplate;
APPKIT_EXTERN NSString *const NSImageNameRefreshFreestandingTemplate;
APPKIT_EXTERN NSString *const NSImageNameBonjour;
APPKIT_EXTERN NSString *const NSImageNameComputer;
APPKIT_EXTERN NSString *const NSImageNameFolderBurnable;
APPKIT_EXTERN NSString *const NSImageNameFolderSmart;
APPKIT_EXTERN NSString *const NSImageNameNetwork;
#endif
 
#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
APPKIT_EXTERN NSString *const NSImageNameFolder;
#endif

@interface NSImage : NSObject <NSCoding, NSCopying>
{
  // Attributes
  NSString	*_name;
  NSString	*_fileName;
  NSSize	_size;
  struct __imageFlags {
    unsigned	archiveByName: 1;
    unsigned	scalable: 1;
    unsigned	dataRetained: 1;
    unsigned	flipDraw: 1;
    unsigned	sizeWasExplicitlySet: 1;
    unsigned	useEPSOnResolutionMismatch: 1;
    unsigned	colorMatchPreferred: 1;
    unsigned	multipleResolutionMatching: 1;
    unsigned	cacheSeparately: 1;
    unsigned	unboundedCacheDepth: 1;
    unsigned	syncLoad: 1;
  } _flags;
  NSMutableArray	*_reps;
  NSColor		*_color;
  NSView                *_lockedView;
  id		        _delegate;
  NSImageCacheMode      _cacheMode;
}

//
// Initializing a New NSImage Instance 
//
/** Initializes and returns a NSImage from the NSString fileName.
 */
- (id) initByReferencingFile: (NSString*)fileName;

/** Initializes and returns a new NSImage from the file 
 *  fileName. fileName should be an absolute path.
 *  <p>See Also:</p>
 *  <list>
 *   <item>[NSImageRep+imageRepsWithContentsOfFile:]</item>
 *  </list>
 */

- (id) initWithContentsOfFile: (NSString*)fileName;

/** Initializes and returns a new NSImage from the NSData data.
 * <p>See Also:</p>
 * <list>
 *  <item>[NSBitmapImageRep+imageRepWithData:]</item>
 *  <item>[NSEPSImageRep+imageRepWithData:]</item>
 * </list>
 */
- (id) initWithData: (NSData*)data;

/** Initializes and returns a new NSImage from the data in pasteboard.
 *  The pasteboard types can be whose defined in
 *  [NSImageRep+imagePasteboardTypes] or NSFilenamesPboardType
 *  <p>See Also:</p>
 *  <list>
 *   <item>[NSImageRep+imageRepsWithPasteboard:]</item>
 *  </list>
 */
- (id) initWithPasteboard: (NSPasteboard*)pasteboard;

/** Initialize and returns a new NSImage with aSize as specified
 *  size.
 *  <p>See Also:</p>
 *  <list>
 *   <item>-setSize:</item>
 *   <item>-size</item>
 *  </list>
 */
- (id) initWithSize: (NSSize)aSize;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (id)initWithBitmapHandle:(void *)bitmap;
- (id)initWithContentsOfURL:(NSURL *)anURL;
- (id)initWithIconHandle:(void *)icon;
#endif

//
// Setting the Size of the Image 
//
/** Sets the NSImage size to aSize. Changing the size recreate
 *  the cache.
 *  <p>See Also:</p>
 *  <list>
 *   <item>-size</item>
 *   <item>-initWithSize:</item>
 *  </list>
 */
- (void) setSize: (NSSize)aSize;

/** Returns NSImage size if the size have been set. Returns the
 *  size of the best representation otherwise.
 *  <p>See Also:</p>
 *  <list>
 *   <item>-setSize:</item>
 *   <item>-initWithSize:</item>
 *  </list>
 */
- (NSSize) size;

//
// Referring to Images by Name 
//
/** Returns the NSImage named aName. The search is done in the main bundle
 *  first and then in the usual images directories.
 */
+ (id) imageNamed: (NSString*)aName;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
#if OS_API_VERSION(GS_API_NONE, GS_API_NONE)
+ (NSImage*) _standardImageWithName: (NSString*)name;
#endif
#endif

/** Sets aName as the name of the receiver.
 */
- (BOOL) setName: (NSString*)aName;

/** Returns the name of the receiver.
 */
- (NSString*) name;

//
// Specifying the Image 
//
/** Adds the NSImageRep imageRep to the NSImage's representations array.
 *  <p>See Also:</p>
 *  <list>
 *   <item>-addRepresentations:</item>
 *   <item>-removeRepresentation:</item>
 *  </list>
 */
- (void) addRepresentation: (NSImageRep*)imageRep;

/** Adds the NSImageRep array imageRepArray to the NSImage's
 *  representations array.
 *  <p>See Also:</p>
 *  <list>
 *   <item>-addRepresentation:</item>
 *   <item>-removeRepresentation:</item>
 *  </list>
 */
- (void) addRepresentations: (NSArray*)imageRepArray;

/** Locks the focus on the best representation.
 *  <p>See Also:</p>
 *  <list>
 *   <item>-lockFocusOnRepresentation:</item>
 *  </list>
 */
- (void) lockFocus;

/** Locks the focus in the imageRep. If imageRep is nil this method
 *  locks the focus on the best representation.
 */
- (void) lockFocusOnRepresentation: (NSImageRep*)imageRep;

/** Unlocks the focus on the receiver.
 *  <p>See Also:</p>
 *  <list>
 *   <item>-lockFocus</item>
 *  </list>
 */
- (void) unlockFocus;

//
// Using the Image 
//
- (void) compositeToPoint: (NSPoint)aPoint
		operation: (NSCompositingOperation)op;
- (void) compositeToPoint: (NSPoint)aPoint
		 fromRect: (NSRect)aRect
		operation: (NSCompositingOperation)op;
- (void) dissolveToPoint: (NSPoint)aPoint
		fraction: (CGFloat)aFloat;
- (void) dissolveToPoint: (NSPoint)aPoint
		fromRect: (NSRect)aRect
		fraction: (CGFloat)aFloat;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (void) compositeToPoint: (NSPoint)aPoint
		 fromRect: (NSRect)srcRect
		operation: (NSCompositingOperation)op
		 fraction: (CGFloat)delta;
- (void) compositeToPoint: (NSPoint)aPoint
		operation: (NSCompositingOperation)op
		 fraction: (CGFloat)delta;
#endif 

//
// Choosing Which Image Representation to Use 
//
/** Sets the preferred representation of a NSImage.
 *  <p>See Also:</p>
 *  <list>
 *   <item>-prefersColorMatch</item>
 *   <item>-bestRepresentationForDevice:</item>
 *  </list>
 */
- (void) setPrefersColorMatch: (BOOL)flag;

/** Returns YES if color matching is the preferred representation
 *  and NO otherwise.
 */
- (BOOL) prefersColorMatch;
- (void) setUsesEPSOnResolutionMismatch: (BOOL)flag;
- (BOOL) usesEPSOnResolutionMismatch;
- (void) setMatchesOnMultipleResolution: (BOOL)flag;
- (BOOL) matchesOnMultipleResolution;

//
// Getting the Representations 
//
/** Finds the best representation for deviceDescription.  If
 *  deviceDescription is nil, it guesses where drawing is taking
 *  place and finds the best representation.
 */
- (NSImageRep*) bestRepresentationForDevice: (NSDictionary*)deviceDescription;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
/**
 * Finds the best representation for drawing in the specified rect.
 * 
 * context and hints are currently ignored.
 */
- (NSImageRep *)bestRepresentationForRect: (NSRect)rect
				  context: (NSGraphicsContext *)context
				    hints: (NSDictionary *)hints;
#endif

- (NSArray*) representations;

/** Remove the NSImageRep imageRep from the NSImage's representations 
 *  array
 *  <p>See Also:</p>
 *  <list>
 *   <item>-addRepresentations:</item>
 *   <item>-addRepresentation:</item>
 *  </list>
 */
- (void) removeRepresentation: (NSImageRep*)imageRep;

//
// Determining How the Image is Stored 
//
- (void) setCachedSeparately: (BOOL)flag;
- (BOOL) isCachedSeparately;
- (void) setDataRetained: (BOOL)flag;
- (BOOL) isDataRetained;
- (void) setCacheDepthMatchesImageDepth: (BOOL)flag;
- (BOOL) cacheDepthMatchesImageDepth;
- (void) setCacheMode: (NSImageCacheMode)mode;
- (NSImageCacheMode) cacheMode;

//
// Drawing 
//
- (BOOL) drawRepresentation: (NSImageRep*)imageRep
		     inRect: (NSRect)aRect;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
/** Calls -drawAtPoint:fromRect:operation:fraction: with
 *  <code>dstRect</code> given by <code>point</code> and the size of
 *  <code>srcRect</code>.
 */
- (void) drawAtPoint: (NSPoint)point
	    fromRect: (NSRect)srcRect
	   operation: (NSCompositingOperation)op
	    fraction: (CGFloat)delta;

/** <p>Takes the part of the receiver given by <code>srcRect</code> and
 *  draws it in <code>dstRect</code> in the current coordinate system,
 *  transforming the image as necessary.
 *  </p><p>
 *  The image is drawn as if it was drawn to a cleared window, then
 *  dissolved using the fraction <code>delta</code> to another cleared
 *  window, and finally composited using <code>op</code> to the
 *  destination.
 *  </p><p>
 *  Note that compositing and dissolving doesn't work on all devices
 *  (printers, in particular).
 *  </p> 
 */
- (void) drawInRect: (NSRect)dstRect
	   fromRect: (NSRect)srcRect
	  operation: (NSCompositingOperation)op
	   fraction: (CGFloat)delta;
#endif 

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)

- (void) drawInRect: (NSRect)dstRect
	   fromRect: (NSRect)srcRect
	  operation: (NSCompositingOperation)op
	   fraction: (CGFloat)delta
     respectFlipped: (BOOL)respectFlipped
	      hints: (NSDictionary*)hints;

#endif

/**
 * <p>Draws the entire image in <code>rect</code> scaling if needed.<br>
 * Drawing is done using <code>NSCompositeSourceOver</code>.
 * </p>
 */

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
- (void) drawInRect: (NSRect)rect;
#endif

//
// Determining How the Image is Drawn 
//
- (BOOL) isValid;
- (void) setScalesWhenResized: (BOOL)flag;
- (BOOL) scalesWhenResized;

/** Sets the color of the NSImage's background to aColor.
 *  <p>See Also:</p>
 *  <list>
 *   <item>-backgroundColor</item>
 *  </list>
 */
- (void) setBackgroundColor: (NSColor*)aColor;

/** Returns the color of the NSImage's background.
 *  <p>See Also:</p>
 *  <list>
 *   <item>-setBackgroundColor:</item>
 *  </list>
 */
- (NSColor*) backgroundColor;
- (void) recache;
- (void) setFlipped: (BOOL)flag;
- (BOOL) isFlipped;

//
// Assigning a Delegate 
//
- (void) setDelegate: (id)anObject;
- (id) delegate;

//
// Producing TIFF Data for the Image 
//
- (NSData*) TIFFRepresentation;
- (NSData*) TIFFRepresentationUsingCompression: (NSTIFFCompression)comp
					factor: (float)aFloat;

//
// Managing NSImageRep Subclasses 
//
+ (NSArray*) imageUnfilteredFileTypes;
+ (NSArray*) imageUnfilteredPasteboardTypes;

//
// Testing Image Data Sources 
//
+ (BOOL) canInitWithPasteboard: (NSPasteboard*)pasteboard;
+ (NSArray*) imageFileTypes;
+ (NSArray*) imagePasteboardTypes;

@end

@interface NSImage (GSQuartz)
#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
- (CGImageRef)CGImageForProposedRect: (NSRect *)proposedDestRect 
                         context: (NSGraphicsContext *)referenceContext 
                           hints: (NSDictionary *)hints;
#endif
@end

@interface NSBundle (NSImageAdditions)

- (NSString*) pathForImageResource: (NSString*)name;

@end

#if OS_API_VERSION(GS_API_NONE, GS_API_NONE)
/*
 * A formal protocol that duplicates the informal protocol for delegates.
 */
@protocol GSImageDelegateProtocol

- (NSImage*) imageDidNotDraw: (id)sender
		      inRect: (NSRect)aRect;

@end
#endif

#endif // _GNUstep_H_NSImage

