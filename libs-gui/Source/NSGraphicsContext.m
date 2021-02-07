/** <title>NSGraphicsContext</title>

   <abstract>GNUstep drawing context class.</abstract>

   Copyright (C) 1998 Free Software Foundation, Inc.

   Written by: Adam Fedor <fedor@gnu.org>
   Date: Nov 1998
   Updated by: Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: Feb 1999
   
   This file is part of the GNUStep GUI Library.

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

#if     !defined(_NONFRAGILE_ABI)
#define EXPOSE_NSThread_IVARS
#endif

#import <Foundation/NSGeometry.h> 
#import <Foundation/NSString.h> 
#import <Foundation/NSArray.h> 
#import <Foundation/NSBundle.h> 
#import <Foundation/NSValue.h> 
#import <Foundation/NSDictionary.h>
#import <Foundation/NSException.h>
#import <Foundation/NSData.h>
#import <Foundation/NSLock.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSSet.h>
#import <Foundation/NSThread.h>
#import <Foundation/NSZone.h>
#import "AppKit/AppKitExceptions.h"
#import "AppKit/NSGraphicsContext.h"
#import "AppKit/NSAffineTransform.h"
#import "AppKit/NSBezierPath.h"
#import "AppKit/NSPrintInfo.h"
#import "AppKit/NSPrintOperation.h"
#import "AppKit/NSWindow.h"
#import "AppKit/NSView.h"
#import "AppKit/DPSOperators.h"
#import "GNUstepGUI/GSVersion.h"
#import "GNUstepGUI/GSDisplayServer.h"

/* The memory zone where all global objects are allocated from (Contexts
   are also allocated from this zone) */
static NSZone *_globalGSZone = NULL;

/* The current concrete class */
static Class defaultNSGraphicsContextClass = NULL;

/* Class variable for holding pointers to method functions */
static NSMutableDictionary *classMethodTable;

/* Lock for use when creating contexts */
static NSRecursiveLock  *contextLock = nil;

#ifndef GNUSTEP_BASE_LIBRARY
static NSString	*NSGraphicsContextThreadKey = @"NSGraphicsContextThreadKey";
#endif
static NSString	*NSGraphicsContextStackKey = @"NSGraphicsContextStackKey";

/* Colorspace constants */
NSString *GSColorSpaceName = @"GSColorSpaceName";
NSString *GSColorSpaceWhitePoint = @"GSColorSpaceWhitePoint";
NSString *GSColorSpaceBlackPoint = @"GSColorSpaceBlackPoint";
NSString *GSColorSpaceGamma = @"GSColorSpaceGamma";
NSString *GSColorSpaceMatrix = @"GSColorSpaceMatrix";
NSString *GSColorSpaceRange = @"GSColorSpaceRange";
NSString *GSColorSpaceComponents = @"GSColorSpaceComponents";
NSString *GSColorSpaceProfile = @"GSColorSpaceProfile";
NSString *GSAlternateColorSpace = @"GSAlternateColorSpace";
NSString *GSBaseColorSpace = @"GSBaseColorSpace";
NSString *GSColorSpaceColorTable = @"GSColorSpaceColorTable";

/*
 *	Function for rapid access to current graphics context.
 */
NSGraphicsContext	*GSCurrentContext(void)
{
#ifdef GNUSTEP_BASE_LIBRARY
/*
 *	gstep-base has a faster mechanism to get the current thread.
 */
  NSThread *th = GSCurrentThread();

# if     defined(_NONFRAGILE_ABI)
  return (NSGraphicsContext*)object_getIvar(th,
    class_getInstanceVariable(object_getClass(th), "_gcontext"));
# else
  return (NSGraphicsContext*) th->_gcontext;
# endif
#else
  NSMutableDictionary *dict = [[NSThread currentThread] threadDictionary];

  return (NSGraphicsContext*) [dict objectForKey: NSGraphicsContextThreadKey];
#endif
}


@interface NSGraphicsContext (Private)
+ (gsMethodTable *) _initializeMethodTable;
@end

/**
  <unit>
  <heading>NSGraphicsContext</heading>

  <p>This is an abstract class which provides a framework for a device
  independant drawing. 
  </p>
  
  <p>In addition, this class provides methods to perform the actual
  drawing. As a convenience, you can also access these through various
  function interfaces. One is a Display Postscript interface using PS
  and DPS operations. Another is a Quartz interface (not yet written).
  </p>

  </unit> */
@implementation NSGraphicsContext 

+ (void) initialize
{
  if (contextLock == nil)
    {
      [gnustep_global_lock lock];
      if (contextLock == nil)
	{
	  contextLock = [NSRecursiveLock new];
	  defaultNSGraphicsContextClass = [NSGraphicsContext class];
	  _globalGSZone = NSDefaultMallocZone();
	  classMethodTable =
	    [[NSMutableDictionary allocWithZone: _globalGSZone] init];
	}
      [gnustep_global_lock unlock];
    }
}

+ (void) initializeBackend
{
  [self subclassResponsibility: _cmd];
}

/** Set the concrete subclass that will provide the device dependant
    implementation.
*/
+ (void) setDefaultContextClass: (Class)defaultContextClass
{
  defaultNSGraphicsContextClass = defaultContextClass;
}

/** Set the current context that will handle drawing. */
+ (void) setCurrentContext: (NSGraphicsContext *)context
{
#ifdef GNUSTEP_BASE_LIBRARY
/*
 *	gstep-base has a faster mechanism to get the current thread.
 */
  NSThread *th = GSCurrentThread();

# if     defined(_NONFRAGILE_ABI)
  object_setIvar(th,
    class_getInstanceVariable(object_getClass(th), "_gcontext"), context);
# else
  ASSIGN(th->_gcontext, context);
# endif
#else
  NSMutableDictionary *dict = [[NSThread currentThread] threadDictionary];

  [dict setObject: context forKey: NSGraphicsContextThreadKey];
#endif
}

/** Returns the current context. Also see the convienience function
    GSCurrentContext() */
+ (NSGraphicsContext *) currentContext
{
  return GSCurrentContext();
}

/** Returns YES if the current context is a display context */
+ (BOOL) currentContextDrawingToScreen
{
  return [GSCurrentContext() isDrawingToScreen];
}

/** 
    <p>Create a graphics context with attributes, which contains key/value
    pairs which describe the specifics of how the context is to
    be initialized. 
    </p>
    */
+ (NSGraphicsContext *) graphicsContextWithAttributes: (NSDictionary *)attributes
{
  NSGraphicsContext *ctxt;

  ctxt = [[self alloc] initWithContextInfo: attributes];
 
  return AUTORELEASE(ctxt);
}

/**
   Create graphics context with attributes speficied by aWindow's
   device description. */
+ (NSGraphicsContext *) graphicsContextWithWindow: (NSWindow *)aWindow
{
  return [self graphicsContextWithAttributes:
                   [NSDictionary dictionaryWithObject: aWindow 
                                 forKey: NSGraphicsContextDestinationAttributeName]];
}

+ (NSGraphicsContext *) graphicsContextWithBitmapImageRep: (NSBitmapImageRep *)bitmap
{
  return [self graphicsContextWithAttributes:
                   [NSDictionary dictionaryWithObject: bitmap 
                                 forKey: NSGraphicsContextDestinationAttributeName]];
}

+ (NSGraphicsContext *) graphicsContextWithGraphicsPort: (void *)port 
                                                flipped: (BOOL)flag
{
  NSGraphicsContext *ctxt;

  ctxt = [[self alloc] initWithGraphicsPort: port
                                    flipped: flag];
 
  return AUTORELEASE(ctxt);
}

+ (NSGraphicsContext *)graphicsContextWithCGContext: (CGContextRef)context
					    flipped: (BOOL)flipped
{
  return [NSGraphicsContext graphicsContextWithGraphicsPort: (void *)context
						    flipped: flipped];
}

+ (void) restoreGraphicsState
{
  NSGraphicsContext *ctxt;
  NSMutableDictionary *dict = [[NSThread currentThread] threadDictionary];
  NSMutableArray *stack = [dict objectForKey: NSGraphicsContextStackKey];

  if (stack == nil)
    {
      [NSException raise: NSGenericException
		   format: @"restoreGraphicsState without previous save"];
    }
  // might be nil, i.e. no current context
  ctxt = [stack lastObject];
  [NSGraphicsContext setCurrentContext: ctxt];
  if (ctxt)
    {
      [stack removeLastObject];
      [ctxt restoreGraphicsState];
    }
}

+ (void) saveGraphicsState
{
  NSGraphicsContext *ctxt;
  NSMutableDictionary *dict = [[NSThread currentThread] threadDictionary];
  NSMutableArray *stack = [dict objectForKey: NSGraphicsContextStackKey];
  if (stack == nil)
    {
      stack = [[NSMutableArray allocWithZone: _globalGSZone] init];
      [dict setObject: stack forKey: NSGraphicsContextStackKey];
      RELEASE(stack);
    }
  // might be nil, i.e. no current context
  ctxt = GSCurrentContext();
  if (ctxt)
    {
      [ctxt saveGraphicsState];
      [stack addObject: ctxt];
    }
}

+ (void) setGraphicsState: (NSInteger)graphicsState
{
  /* FIXME: Need to keep a table of which context goes with a graphicState,
     or perhaps we could rely on the backend? */
  [self notImplemented: _cmd];
}

+ (id) alloc
{
  return [self allocWithZone: _globalGSZone];
}

+ (id) allocWithZone: (NSZone*)z
{
  if (self == [NSGraphicsContext class])
    {
      NSAssert(defaultNSGraphicsContextClass, 
	       @"Internal Error: No default NSGraphicsContext set\n");
      return [defaultNSGraphicsContextClass allocWithZone: z];
    }
  else
    return [super allocWithZone: z];
}

- (void) dealloc
{
  DESTROY(usedFonts);
  DESTROY(focus_stack);
  DESTROY(context_data);
  DESTROY(context_info);
  [super dealloc];
}

- (id) init
{
  return [self initWithContextInfo: nil];
}

/** <init />
 */
- (id) initWithContextInfo: (NSDictionary *)info
{
  self = [super init];
  if (self != nil)
    {
      ASSIGN(context_info, info);
      focus_stack = [[NSMutableArray allocWithZone: [self zone]]
				  initWithCapacity: 1];
      usedFonts = nil;

      /*
       * The classMethodTable dictionary and the list of all contexts must both
       * be protected from other threads.
       */
      [contextLock lock];
      methods = [[classMethodTable objectForKey: [self class]] pointerValue];
      if (methods == 0)
        {
          methods = [[self class] _initializeMethodTable];
          [classMethodTable setObject: [NSValue valueWithPointer: methods]
                            forKey: [self class]];
        }
      [contextLock unlock];
    }
  return self;
}


- (id) initWithGraphicsPort: (void *)port 
                    flipped: (BOOL)flag;
{
  self = [self init];
  if (self != nil)
    {
      _graphicsPort = port;
      _isFlipped = flag;
    }
  return self;
}

- (NSDictionary *) attributes
{
  return context_info;
}

- (void) flushGraphics
{
  [self subclassResponsibility: _cmd];
}

- (void *) graphicsPort
{
  return _graphicsPort;
}

- (CGContextRef)CGContext
{
  return (CGContextRef)[self graphicsPort];
}

- (BOOL) isDrawingToScreen
{
  return NO;
}

- (void) restoreGraphicsState
{
  [self DPSgrestore];
}

- (void) saveGraphicsState
{
  [self DPSgsave];
}

- (void *) focusStack
{
  return focus_stack;
}

- (void) setFocusStack: (void *)stack
{
  ASSIGN(focus_stack, (id)stack);
}

- (void) setImageInterpolation: (NSImageInterpolation)interpolation
{
  _interp = interpolation;
}

- (NSImageInterpolation) imageInterpolation
{
  return _interp;
}

- (void) setShouldAntialias: (BOOL)antialias
{
  _antialias = antialias;
}

- (BOOL) shouldAntialias
{
  return _antialias;
}

- (NSPoint) patternPhase
{
  return _patternPhase;
}

- (void) setPatternPhase: (NSPoint)phase
{
  _patternPhase = phase;
}

- (BOOL) isFlipped
{
  NSView *focusView = [self focusView];

  if (focusView)
    return [focusView isFlipped];
  else
    return _isFlipped;
}
- (NSCompositingOperation) compositingOperation
{
  return _compositingOperation;
}

- (void) setCompositingOperation: (NSCompositingOperation)operation
{
  _compositingOperation = operation;
}

- (NSView*) focusView
{
  return [focus_stack lastObject];
}

- (void) lockFocusView: (NSView*)aView inRect: (NSRect)rect
{
  [focus_stack addObject: aView];
}

- (void) unlockFocusView: (NSView*)aView needsFlush: (BOOL)flush
{
  [focus_stack removeLastObject];
}

- (void) useFont: (NSString*)name
{
  if ([self isDrawingToScreen] == YES)
    return;

  if (usedFonts == nil)
    usedFonts = [[NSMutableSet alloc] initWithCapacity: 2];

  [usedFonts addObject: name];
}

- (void) resetUsedFonts
{
  [usedFonts removeAllObjects];
}

- (NSSet *) usedFonts
{
  return usedFonts;
}

/* Private backend methods */
/** Private backend method. Typically this is called by the window
    server to tell the graphics context that it should flush output
    to a window indicated by the device pointer. The device pointer
    is an opaque type setup by the context so that it knows which 
    context and/or buffer should be. */
+ (void) handleExposeRect: (NSRect)rect forDriver: (void *)driver
{
}

@end

@implementation NSGraphicsContext (Private)

/* Build up method table for fast access to methods. Cast to (void *) to
   avoid compiler warnings */
+ (gsMethodTable *) _initializeMethodTable
{
  gsMethodTable methodTable;
  gsMethodTable *mptr;

#define	GET_IMP(X) ((void*) [self instanceMethodForSelector: (X)])

/* ----------------------------------------------------------------------- */
/* Color operations */
/* ----------------------------------------------------------------------- */
  methodTable.DPScurrentalpha_ =
    GET_IMP(@selector(DPScurrentalpha:));
  methodTable.DPScurrentcmykcolor____ =
    GET_IMP(@selector(DPScurrentcmykcolor::::));
  methodTable.DPScurrentgray_ =
    GET_IMP(@selector(DPScurrentgray:));
  methodTable.DPScurrenthsbcolor___ =
    GET_IMP(@selector(DPScurrenthsbcolor:::));
  methodTable.DPScurrentrgbcolor___ =
    GET_IMP(@selector(DPScurrentrgbcolor:::));
  methodTable.DPSsetalpha_ =
    GET_IMP(@selector(DPSsetalpha:));
  methodTable.DPSsetcmykcolor____ =
    GET_IMP(@selector(DPSsetcmykcolor::::));
  methodTable.DPSsetgray_ =
    GET_IMP(@selector(DPSsetgray:));
  methodTable.DPSsethsbcolor___ =
    GET_IMP(@selector(DPSsethsbcolor:::));
  methodTable.DPSsetrgbcolor___ =
    GET_IMP(@selector(DPSsetrgbcolor:::));

  methodTable.GSSetFillColorspace_ =
    GET_IMP(@selector(GSSetFillColorspace:));
  methodTable.GSSetStrokeColorspace_ =
    GET_IMP(@selector(GSSetStrokeColorspace:));
  methodTable.GSSetFillColor_ =
    GET_IMP(@selector(GSSetFillColor:));
  methodTable.GSSetStrokeColor_ =
    GET_IMP(@selector(GSSetStrokeColor:));

/* ----------------------------------------------------------------------- */
/* Text operations */
/* ----------------------------------------------------------------------- */
  methodTable.DPSashow___ =
    GET_IMP(@selector(DPSashow:::));
  methodTable.DPSawidthshow______ =
    GET_IMP(@selector(DPSawidthshow::::::));
  methodTable.DPScharpath__ =
    GET_IMP(@selector(DPScharpath::));
  methodTable.DPSshow_ =
    GET_IMP(@selector(DPSshow:));
  methodTable.DPSwidthshow____ =
    GET_IMP(@selector(DPSwidthshow::::));
  methodTable.DPSxshow___ =
    GET_IMP(@selector(DPSxshow:::));
  methodTable.DPSxyshow___ =
    GET_IMP(@selector(DPSxyshow:::));
  methodTable.DPSyshow___ =
    GET_IMP(@selector(DPSyshow:::));

  methodTable.GSSetCharacterSpacing_ =
    GET_IMP(@selector(GSSetCharacterSpacing:));
  methodTable.GSSetFont_ =
    GET_IMP(@selector(GSSetFont:));
  methodTable.GSSetFontSize_ =
    GET_IMP(@selector(GSSetFontSize:));
  methodTable.GSGetTextCTM =
    GET_IMP(@selector(GSGetTextCTM));
  methodTable.GSGetTextPosition =
    GET_IMP(@selector(GSGetTextPosition));
  methodTable.GSSetTextCTM_ =
    GET_IMP(@selector(GSSetTextCTM:));
  methodTable.GSSetTextDrawingMode_ =
    GET_IMP(@selector(GSSetTextDrawingMode:));
  methodTable.GSSetTextPosition_ =
    GET_IMP(@selector(GSSetTextPosition:));
  methodTable.GSShowText__ =
    GET_IMP(@selector(GSShowText::));
  methodTable.GSShowGlyphs__ =
    GET_IMP(@selector(GSShowGlyphs::));
  methodTable.GSShowGlyphsWithAdvances__ =
    GET_IMP(@selector(GSShowGlyphsWithAdvances:::));

/* ----------------------------------------------------------------------- */
/* Gstate Handling */
/* ----------------------------------------------------------------------- */
  methodTable.DPSgrestore =
    GET_IMP(@selector(DPSgrestore));
  methodTable.DPSgsave =
    GET_IMP(@selector(DPSgsave));
  methodTable.DPSinitgraphics =
    GET_IMP(@selector(DPSinitgraphics));
  methodTable.DPSsetgstate_ =
    GET_IMP(@selector(DPSsetgstate:));

  methodTable.GSDefineGState =
    GET_IMP(@selector(GSDefineGState));
  methodTable.GSUndefineGState_ =
    GET_IMP(@selector(GSUndefineGState:));
  methodTable.GSReplaceGState_ =
    GET_IMP(@selector(GSReplaceGState:));

/* ----------------------------------------------------------------------- */
/* Gstate operations */
/* ----------------------------------------------------------------------- */
  methodTable.DPScurrentflat_ =
    GET_IMP(@selector(DPScurrentflat:));
  methodTable.DPScurrentlinecap_ =
    GET_IMP(@selector(DPScurrentlinecap:));
  methodTable.DPScurrentlinejoin_ =
    GET_IMP(@selector(DPScurrentlinejoin:));
  methodTable.DPScurrentlinewidth_ =
    GET_IMP(@selector(DPScurrentlinewidth:));
  methodTable.DPScurrentmiterlimit_ =
    GET_IMP(@selector(DPScurrentmiterlimit:));
  methodTable.DPScurrentpoint__ =
    GET_IMP(@selector(DPScurrentpoint::));
  methodTable.DPScurrentstrokeadjust_ =
    GET_IMP(@selector(DPScurrentstrokeadjust:));
  methodTable.DPSsetdash___ =
    GET_IMP(@selector(DPSsetdash:::));
  methodTable.DPSsetflat_ =
    GET_IMP(@selector(DPSsetflat:));
  methodTable.DPSsethalftonephase__ =
    GET_IMP(@selector(DPSsethalftonephase::));
  methodTable.DPSsetlinecap_ =
    GET_IMP(@selector(DPSsetlinecap:));
  methodTable.DPSsetlinejoin_ =
    GET_IMP(@selector(DPSsetlinejoin:));
  methodTable.DPSsetlinewidth_ =
    GET_IMP(@selector(DPSsetlinewidth:));
  methodTable.DPSsetmiterlimit_ =
    GET_IMP(@selector(DPSsetmiterlimit:));
  methodTable.DPSsetstrokeadjust_ =
    GET_IMP(@selector(DPSsetstrokeadjust:));

/* ----------------------------------------------------------------------- */
/* Matrix operations */
/* ----------------------------------------------------------------------- */
  methodTable.DPSconcat_ =
    GET_IMP(@selector(DPSconcat:));
  methodTable.DPSinitmatrix =
    GET_IMP(@selector(DPSinitmatrix));
  methodTable.DPSrotate_ =
    GET_IMP(@selector(DPSrotate:));
  methodTable.DPSscale__ =
    GET_IMP(@selector(DPSscale::));
  methodTable.DPStranslate__ =
    GET_IMP(@selector(DPStranslate::));

  methodTable.GSCurrentCTM =
    GET_IMP(@selector(GSCurrentCTM));
  methodTable.GSSetCTM_ =
    GET_IMP(@selector(GSSetCTM:));
  methodTable.GSConcatCTM_ =
    GET_IMP(@selector(GSConcatCTM:));

/* ----------------------------------------------------------------------- */
/* Paint operations */
/* ----------------------------------------------------------------------- */
  methodTable.DPSarc_____ =
    GET_IMP(@selector(DPSarc:::::));
  methodTable.DPSarcn_____ =
    GET_IMP(@selector(DPSarcn:::::));
  methodTable.DPSarct_____ =
    GET_IMP(@selector(DPSarct:::::));
  methodTable.DPSclip =
    GET_IMP(@selector(DPSclip));
  methodTable.DPSclosepath =
    GET_IMP(@selector(DPSclosepath));
  methodTable.DPScurveto______ =
    GET_IMP(@selector(DPScurveto::::::));
  methodTable.DPSeoclip =
    GET_IMP(@selector(DPSeoclip));
  methodTable.DPSeofill =
    GET_IMP(@selector(DPSeofill));
  methodTable.DPSfill =
    GET_IMP(@selector(DPSfill));
  methodTable.DPSflattenpath =
    GET_IMP(@selector(DPSflattenpath));
  methodTable.DPSinitclip =
    GET_IMP(@selector(DPSinitclip));
  methodTable.DPSlineto__ =
    GET_IMP(@selector(DPSlineto::));
  methodTable.DPSmoveto__ =
    GET_IMP(@selector(DPSmoveto::));
  methodTable.DPSnewpath =
    GET_IMP(@selector(DPSnewpath));
  methodTable.DPSpathbbox____ =
    GET_IMP(@selector(DPSpathbbox::::));
  methodTable.DPSrcurveto______ =
    GET_IMP(@selector(DPSrcurveto::::::));
  methodTable.DPSrectclip____ =
    GET_IMP(@selector(DPSrectclip::::));
  methodTable.DPSrectfill____ =
    GET_IMP(@selector(DPSrectfill::::));
  methodTable.DPSrectstroke____ =
    GET_IMP(@selector(DPSrectstroke::::));
  methodTable.DPSreversepath =
    GET_IMP(@selector(DPSreversepath));
  methodTable.DPSrlineto__ =
    GET_IMP(@selector(DPSrlineto::));
  methodTable.DPSrmoveto__ =
    GET_IMP(@selector(DPSrmoveto::));
  methodTable.DPSstroke =
    GET_IMP(@selector(DPSstroke));
  methodTable.DPSshfill =
    GET_IMP(@selector(DPSshfill:));

  methodTable.GSSendBezierPath_ =
    GET_IMP(@selector(GSSendBezierPath:));
  methodTable.GSRectClipList__ =
    GET_IMP(@selector(GSRectClipList::));
  methodTable.GSRectFillList__ =
    GET_IMP(@selector(GSRectFillList::));

/* ----------------------------------------------------------------------- */
/* Window system ops */
/* ----------------------------------------------------------------------- */
  methodTable.GSCurrentDevice___ =
    GET_IMP(@selector(GSCurrentDevice:::));
  methodTable.DPScurrentoffset__ =
    GET_IMP(@selector(DPScurrentoffset::));
  methodTable.GSSetDevice___ =
    GET_IMP(@selector(GSSetDevice:::));
  methodTable.DPSsetoffset__ =
    GET_IMP(@selector(DPSsetoffset::));

/*-------------------------------------------------------------------------*/
/* Graphics Extensions Ops */
/*-------------------------------------------------------------------------*/
  methodTable.DPScomposite________ =
    GET_IMP(@selector(DPScomposite::::::::));
  methodTable.DPScompositerect_____ =
    GET_IMP(@selector(DPScompositerect:::::));
  methodTable.DPSdissolve________ =
    GET_IMP(@selector(DPSdissolve::::::::));

  methodTable.GSDrawImage__ =
    GET_IMP(@selector(GSDrawImage::));

/* ----------------------------------------------------------------------- */
/* Postscript Client functions */
/* ----------------------------------------------------------------------- */
  methodTable.DPSPrintf__ =
    GET_IMP(@selector(DPSPrintf::));
  methodTable.DPSWriteData__ =
    GET_IMP(@selector(DPSWriteData::));

/* ----------------------------------------------------------------------- */
/* NSGraphics Ops */	
/* ----------------------------------------------------------------------- */
  methodTable.GSReadRect_ =
    GET_IMP(@selector(GSReadRect:));

  methodTable.NSBeep =
    GET_IMP(@selector(NSBeep));

/* Context helper wraps */
  methodTable.GSWSetViewIsFlipped_ =
    GET_IMP(@selector(GSWSetViewIsFlipped:));
  methodTable.GSWViewIsFlipped =
    GET_IMP(@selector(GSWViewIsFlipped));

/*
 * Render Bitmap Images
 */
  methodTable.NSDrawBitmap___________ = 
    GET_IMP(@selector(NSDrawBitmap:::::::::::));

  mptr = NSZoneMalloc(_globalGSZone, sizeof(gsMethodTable));
  memcpy(mptr, &methodTable, sizeof(gsMethodTable));
  return mptr;
}

- (id) subclassResponsibility: (SEL)aSel
{
  [NSException raise: GSWindowServerInternalException
    format: @"subclass %s(%s) should override %s", 
	       class_getName(object_getClass(self)),
	       GSObjCIsInstance(self) ? "instance" : "class",
	       sel_getName(aSel)];
  return nil;
}

@end


/*
 *	The 'Ops' catagory contains the methods to implement all the
 *	PostScript functions.  In this abstract class, these will all
 *	raise an exception.  Concrete instances of the NSGraphicsContext
 *	class should override these methods in order to implement the
 *	PostScript functions.
 */
@implementation NSGraphicsContext (Ops)

/* ----------------------------------------------------------------------- */
/* Color operations */
/* ----------------------------------------------------------------------- */
/** Returns the current alpha component (DPS). */
- (void) DPScurrentalpha: (CGFloat *)a
{
  [self subclassResponsibility: _cmd];
}

/** Returns the current color according to the CMYK color model (DPS). */
- (void) DPScurrentcmykcolor: (CGFloat*)c : (CGFloat*)m : (CGFloat*)y : (CGFloat*)k 
{
  [self subclassResponsibility: _cmd];
}

/** Returns the gray-level equivalent in the current color space. The
    value may depend on the current color space and may be 0 if the
    current color space has no notion of a gray value (DPS) */
- (void) DPScurrentgray: (CGFloat*)gray 
{
  [self subclassResponsibility: _cmd];
}

/** Returns the current color according to the HSB color model (DPS). */
- (void) DPScurrenthsbcolor: (CGFloat*)h : (CGFloat*)s : (CGFloat*)b 
{
  [self subclassResponsibility: _cmd];
}

/** Returns the current color according to the RGB color model (DPS). */
- (void) DPScurrentrgbcolor: (CGFloat*)r : (CGFloat*)g : (CGFloat*)b 
{
  [self subclassResponsibility: _cmd];
}

/** Sets the alpha drawing component. For this and other color setting
    commands that have no differentiation between fill and stroke colors,
    both the fill and stroke alpha are set (DPS). */
- (void) DPSsetalpha: (CGFloat)a
{
  [self subclassResponsibility: _cmd];
}

/** Sets the current colorspace to Device CMYK and the current color
    based on the indicated values. For this and other color setting
    commands that have no differentiation between fill and stroke colors,
    both the fill and stroke colors are set (DPS). */
- (void) DPSsetcmykcolor: (CGFloat)c : (CGFloat)m : (CGFloat)y : (CGFloat)k 
{
  [self subclassResponsibility: _cmd];
}

/** Sets the current colorspace to Device Gray and the current gray value 
    (DPS). */
- (void) DPSsetgray: (CGFloat)gray 
{
  [self subclassResponsibility: _cmd];
}

/** Sets the current colorspace to Device RGB and the current color based on 
   the indicated values (DPS). */
- (void) DPSsethsbcolor: (CGFloat)h : (CGFloat)s : (CGFloat)b 
{
  [self subclassResponsibility: _cmd];
}

/** Sets the current colorspace to Device RGB and the current color based on 
   the indicated values (DPS). */
- (void) DPSsetrgbcolor: (CGFloat)r : (CGFloat)g : (CGFloat)b 
{
  [self subclassResponsibility: _cmd];
}

- (void) GSSetPatterColor: (NSImage*)image 
{
  [self subclassResponsibility: _cmd];
}

/**
   <p>Sets the colorspace for fill operations based on values in the supplied
   dictionary dict.</p>
   <p>For device colorspaces (GSDeviceGray, GSDeviceRGB,
   GSDeviceCMYK), only the name of the colorspace needs to be set
   using the GSColorSpaceName key.</p>
   <p>Other colorspaces will be documented later (Quartz). </p>
*/
- (void) GSSetFillColorspace: (void *)spaceref
{
  [self subclassResponsibility: _cmd];
}

/** Sets the colorspace for stroke operations based on the values in
    the supplied dictionary. See -GSSetFillColorspace: for a
    description of the values that need to be supplied (Quartz). */
- (void) GSSetStrokeColorspace: (void *)spaceref
{
  [self subclassResponsibility: _cmd];
}

/** Sets the current color for fill operations. The values array
    should have n components, where n corresponds to the number of
    color components required to specify the color in the current
    colorspace (Quartz). */
- (void) GSSetFillColor: (const CGFloat *)values
{
  [self subclassResponsibility: _cmd];
}

/** Sets the current color for stroke operations. The values array
    should have n components, where n corresponds to the number of
    color components required to specify the color in the current
    colorspace (Quartz). */
- (void) GSSetStrokeColor: (const CGFloat *)values
{
  [self subclassResponsibility: _cmd];
}

/* ----------------------------------------------------------------------- */
/* Text operations */
/* ----------------------------------------------------------------------- */

/** Displays a string as in DPSshow, except that (x,y) is added to
    the advancement of every glyph.  An alternative means of achieving
    the same effect is to use the -GSSetCharacterSpacing: method.  Either
    approach should be more efficient that using -DPSshow: with appropriate
    -DPSrmoveto:: operations.
*/
- (void) DPSashow: (CGFloat)x : (CGFloat)y : (const char *)s 
{
  [self subclassResponsibility: _cmd];
}

/** Displays a string as in a combination of DPSashow and DPSwidthshow:
    (ax,ay) is added to the advancement of every glyph, while (cx,cy) is
    also added to the advancement for character c's glyph specifically.
    Using this method should be more efficient that using -DPSshow:
    with appropriate -DPSrmoveto:: operations.
*/
- (void) DPSawidthshow: (CGFloat)cx : (CGFloat)cy : (int)c
                      : (CGFloat)ax : (CGFloat)ay : (const char *)s 
{
  [self subclassResponsibility: _cmd];
}

/** Appends to the current path a path that is equivalent to the
    outlines of the glyphs in the string. This results in a path
    that can be used for stroking, filling or clipping (DPS). */
- (void) DPScharpath: (const char *)s : (int)b 
{
  [self subclassResponsibility: _cmd];
}

- (void) appendBezierPathWithPackedGlyphs: (const char *)packedGlyphs
                                     path: (NSBezierPath*)aPath
{
  [self subclassResponsibility: _cmd];
}

/** Display the string s using the current font (DPS). */
- (void) DPSshow: (const char *)s 
{
  [self subclassResponsibility: _cmd];
}

/** Displays a string as in DPSshow, except that, for character c only,
    the glpyh x and y advancement is determined by the values (x,y),
    instead of by the glyph itself.  This is often used to adjust the
    length of a line of text by changing the width of the space character.
    Using this method should be more efficient than using -DPSshow:
    with appropriate -DPSrmoveto:: operations.
*/
- (void) DPSwidthshow: (CGFloat)x : (CGFloat)y : (int)c : (const char *)s 
{
  [self subclassResponsibility: _cmd];
}

/** Displays a string as in DPSshow, except that the glyph x
    advancement is determined by the values in numarray, one for
    each glyph, instead of by the glyphs themselves.  size
    should be equal to the length of s in glyphs.  Using this method
    should be more efficient than using -DPSshow: with appropriate
    -DPSrmoveto:: operations.
*/
- (void) DPSxshow: (const char *)s : (const CGFloat*)numarray : (int)size 
{
  [self subclassResponsibility: _cmd];
}

/** Displays a string as in DPSshow, except that the glyph x and y
    advancement is determined by the values in numarray, one x and one y
    for each glyph, in alternating order, instead of by the glyphs themselves.
    size should be equal to the length of s in glyphs.  Using this method
    should be more efficient than using -DPSshow: with appropriate
    -DPSrmoveto:: operations.
*/
- (void) DPSxyshow: (const char *)s : (const CGFloat*)numarray : (int)size 
{
  [self subclassResponsibility: _cmd];
}

/** Displays a string as in DPSshow, except that the glyph y
    advancement is determined by the values in numarray, one for
    each glyph, instead of by the glyphs themselves.  size
    should be equal to the length of s in glyphs.  Using this method
    should be more efficient than using -DPSshow: with appropriate
    -DPSrmoveto:: operations.
*/
- (void) DPSyshow: (const char *)s : (const CGFloat*)numarray : (int)size 
{
  [self subclassResponsibility: _cmd];
}

/** Use this method to set the additional spacing between characters
    (glyphs). This spacing is added to the normal spacing for each
    character. Units are in text-space coordinate system. (Quartz).
*/
- (void) GSSetCharacterSpacing: (CGFloat)extra
{
  [self subclassResponsibility: _cmd];
}

/** Set the current font for drawing glyphs. (DPS, Quartz). */
- (void) GSSetFont: (void *)fontref
{
  [self subclassResponsibility: _cmd];
}

/** Set the font size of the current NSFont used for drawing glyphs.
    (DPS, Quartz). */
- (void) GSSetFontSize: (CGFloat)size
{
  [self subclassResponsibility: _cmd];
}

/** 
   <p> Returns the transfer function for transforming text from text space
   to user space. See -GSSetTextCTM: for additiona information. (Quartz).
   </p>
*/
- (NSAffineTransform *) GSGetTextCTM
{
  [self subclassResponsibility: _cmd];
  return nil;
}

/** Returns the location at which text will be drawn. In text-space
   coordinates. (Quartz).
*/
- (NSPoint) GSGetTextPosition
{
  [self subclassResponsibility: _cmd];
  return NSMakePoint(0,0);
}

/** 
   <p> Set the transfer function for transforming text from text space
   to user space. This transform is only applied to text objects and
   is in addition to the normal coordinate transform matrix. When
   drawing text, this transform is applied before the normal CTM.
   </p>

   <p> The text matrix can be changed by either modifying it directly,
   or just by drawing text, in which case the tx and ty offset
   veriables are modified to point to the location of the next
   character that could be rendered (Quartz). </p>
*/
- (void) GSSetTextCTM: (NSAffineTransform *)ctm
{
  [self subclassResponsibility: _cmd];
}

/** Set the current text drawing mode. The mode can be one of several
    values that fill/stroke the text or add it to the current clipping
    path. (Quartz).
*/
- (void) GSSetTextDrawingMode: (GSTextDrawingMode)mode
{
  [self subclassResponsibility: _cmd];
}

/** Set the location at which text will be drawn, in text-space
    coordinates.  This routine updates the current text coordinate
    matrix. (Quartz).
*/
- (void) GSSetTextPosition: (NSPoint)loc
{
  [self subclassResponsibility: _cmd];
}

/** Paints text represented by the characters in string in the current
   font. (Quartz).
*/
- (void) GSShowText: (const char *)string : (size_t) length
{
  [self subclassResponsibility: _cmd];
}

/** Paints the glyphs using the current font. (Quartz). */
- (void) GSShowGlyphs: (const NSGlyph *)glyphs : (size_t) length
{
  [self subclassResponsibility: _cmd];
}

/** Paints the glyphs with the specified advances using the current font.
    (Quartz). */
- (void) GSShowGlyphsWithAdvances: (const NSGlyph *)glyphs : (const NSSize *)advances : (size_t) length
{
  [self subclassResponsibility: _cmd];
}

/* ----------------------------------------------------------------------- */
/* Gstate Handling */
/* ----------------------------------------------------------------------- */

/** Pops a previously saved gstate from the gstate stack and makes it
    current. Drawing information in the previously saved gstate
    becomes the current information. (DPS, Quartz). */
- (void) DPSgrestore
{
  [self subclassResponsibility: _cmd];
}

/** Saves (pushes) a copy of the current gstate information onto the
    gstate stack. This saves drawing information contained in the
    gstate, such as the current path, ctm and colors. (DPS, Quartz). */
- (void) DPSgsave
{
  [self subclassResponsibility: _cmd];
}

- (void) DPSinitgraphics
{
  [self subclassResponsibility: _cmd];
}

/** Makes the gstate indicated by the tag gst the current gstate. Note
    that the gstate is copied, so that changes to either gstate do not
    affect the other. (DPS, Quartz). */
- (void) DPSsetgstate: (NSInteger)gst
{
  [self subclassResponsibility: _cmd];
}

/** Creates a copy of the current gstate and associates it with a tag,
    which is given in the return value. This tag can later be used in
    -DPSsetgstate: to set the gstate as being current again. (DPS, Quartz). */
- (NSInteger)  GSDefineGState
{
  [self subclassResponsibility: _cmd];
  return 0;
}

/** Disassociates the tag gst with it's gstate and destroys the gstate
    object. The tag will no longer be valid and should not be used to
    refer to the gstate again. (DPS, Quartz). */
- (void) GSUndefineGState: (NSInteger)gst
{
  [self subclassResponsibility: _cmd];
}

/** Replaces the gstate refered to by the tag gst with the current
    gstate. The former gstate is destroyed. (DPS, Quartz). */
- (void) GSReplaceGState: (NSInteger)gst
{
  [self subclassResponsibility: _cmd];
}

/* ----------------------------------------------------------------------- */
/* Gstate operations */
/* ----------------------------------------------------------------------- */
/** Returns the current flattness parameter, which controls how curved
    lines are drawn. (DPS, Quartz). */
- (void) DPScurrentflat: (CGFloat*)flatness
{
  [self subclassResponsibility: _cmd];
}

/** Returns the current linecap value. (DPS, Quartz). */
- (void) DPScurrentlinecap: (int*)linecap
{
  [self subclassResponsibility: _cmd];
}

/** Returns the current linejoin value. (DPS, Quartz). */
- (void) DPScurrentlinejoin: (int*)linejoin
{
  [self subclassResponsibility: _cmd];
}

/** Returns the current line width. (DPS, Quartz). */
- (void) DPScurrentlinewidth: (CGFloat*)width
{
  [self subclassResponsibility: _cmd];
}

/** Returns the current linecap value. (DPS, Quartz). */
- (void) DPScurrentmiterlimit: (CGFloat*)limit
{
  [self subclassResponsibility: _cmd];
}

/** Returns the current point. (DPS, Quartz). */
- (void) DPScurrentpoint: (CGFloat*)x : (CGFloat*)y
{
  [self subclassResponsibility: _cmd];
}

/** Returns the strokeadjust value. (DPS). */
- (void) DPScurrentstrokeadjust: (int*)b
{
  [self subclassResponsibility: _cmd];
}

/** Set the pattern for line dashes like the Postscript setdash operator.
    (DPS, Quartz). */
- (void) DPSsetdash: (const CGFloat*)pat : (NSInteger)size : (CGFloat)offset
{
  [self subclassResponsibility: _cmd];
}

/** Sets the current flattness parameter, which controls how curved
    lines are drawn. (DPS, Quartz). */
- (void) DPSsetflat: (CGFloat)flatness
{
  [self subclassResponsibility: _cmd];
}

- (void) DPSsethalftonephase: (CGFloat)x : (CGFloat)y
{
  [self subclassResponsibility: _cmd];
}

/** Set the current linecap value. (DPS, Quartz). */
- (void) DPSsetlinecap: (int)linecap
{
  [self subclassResponsibility: _cmd];
}

/** Set the current linejoin value. (DPS, Quartz). */
- (void) DPSsetlinejoin: (int)linejoin
{
  [self subclassResponsibility: _cmd];
}

/** Set the current line width. (DPS, Quartz). */
- (void) DPSsetlinewidth: (CGFloat)width
{
  [self subclassResponsibility: _cmd];
}

/** Set the current meter limit value. (DPS, Quartz). */
- (void) DPSsetmiterlimit: (CGFloat)limit
{
  [self subclassResponsibility: _cmd];
}

- (void) DPSsetstrokeadjust: (int)b
{
  [self subclassResponsibility: _cmd];
}

/* ----------------------------------------------------------------------- */
/* Matrix operations */
/* ----------------------------------------------------------------------- */
/** Concatenates the coordinate transform represented by the matrix m
    with the current coordinate transform. (DPS). */
- (void) DPSconcat: (const CGFloat*)m
{
  [self subclassResponsibility: _cmd];
}

/** Sets the coordinate transform matrix to the initial values for
    the particular context */
- (void) DPSinitmatrix
{
  [self subclassResponsibility: _cmd];
}

/** Rotate the coordinate system. (DPS). */
- (void) DPSrotate: (CGFloat)angle
{
  [self subclassResponsibility: _cmd];
}

/** Scale the coordinate system. (DPS). */
- (void) DPSscale: (CGFloat)x : (CGFloat)y
{
  [self subclassResponsibility: _cmd];
}

/** Translate the coordinate system. (DPS). */
- (void) DPStranslate: (CGFloat)x : (CGFloat)y
{
  [self subclassResponsibility: _cmd];
}

/** Returns the current coordinate transform matrix. (Quartz). */
- (NSAffineTransform *) GSCurrentCTM
{
  [self subclassResponsibility: _cmd];
  return nil;
}

/** Sets the coordinate transform matrix which describes how graphics
   will be transformed into device coordinates. (Quartz). */
- (void) GSSetCTM: (NSAffineTransform *)ctm
{
  [self subclassResponsibility: _cmd];
}

/** Concatenates the matrix ctm onto the current coordinate transform
    matrix. (Quartz).
*/
- (void) GSConcatCTM: (NSAffineTransform *)ctm
{
  [self subclassResponsibility: _cmd];
}

/* ----------------------------------------------------------------------- */
/* Paint operations */
/* ----------------------------------------------------------------------- */
- (void) DPSarc: (CGFloat)x : (CGFloat)y : (CGFloat)r : (CGFloat)angle1 
	       : (CGFloat)angle2
{
  [self subclassResponsibility: _cmd];
}

- (void) DPSarcn: (CGFloat)x : (CGFloat)y : (CGFloat)r : (CGFloat)angle1 
		: (CGFloat)angle2
{
  [self subclassResponsibility: _cmd];
}

- (void) DPSarct: (CGFloat)x1 : (CGFloat)y1 : (CGFloat)x2 : (CGFloat)y2 : (CGFloat)r
{
  [self subclassResponsibility: _cmd];
}

/** Clip to the current path. (DPS, Quartz). */
- (void) DPSclip
{
  [self subclassResponsibility: _cmd];
}

- (void) DPSclosepath
{
  [self subclassResponsibility: _cmd];
}

- (void) DPScurveto: (CGFloat)x1 : (CGFloat)y1 : (CGFloat)x2 : (CGFloat)y2 
		   : (CGFloat)x3 : (CGFloat)y3
{
  [self subclassResponsibility: _cmd];
}

- (void) DPSeoclip
{
  [self subclassResponsibility: _cmd];
}

- (void) DPSeofill
{
  [self subclassResponsibility: _cmd];
}

/** Fill the current path. (DPS, Quartz). */
- (void) DPSfill
{
  [self subclassResponsibility: _cmd];
}

- (void) DPSflattenpath
{
  [self subclassResponsibility: _cmd];
}

- (void) DPSinitclip
{
  [self subclassResponsibility: _cmd];
}

- (void) DPSlineto: (CGFloat)x : (CGFloat)y
{
  [self subclassResponsibility: _cmd];
}

- (void) DPSmoveto: (CGFloat)x : (CGFloat)y
{
  [self subclassResponsibility: _cmd];
}

- (void) DPSnewpath
{
  [self subclassResponsibility: _cmd];
}

- (void) DPSpathbbox: (CGFloat*)llx : (CGFloat*)lly : (CGFloat*)urx : (CGFloat*)ury
{
  [self subclassResponsibility: _cmd];
}

- (void) DPSrcurveto: (CGFloat)x1 : (CGFloat)y1 : (CGFloat)x2 : (CGFloat)y2 
		    : (CGFloat)x3 : (CGFloat)y3
{
  [self subclassResponsibility: _cmd];
}

- (void) DPSrectclip: (CGFloat)x : (CGFloat)y : (CGFloat)w : (CGFloat)h
{
  [self subclassResponsibility: _cmd];
}

- (void) DPSrectfill: (CGFloat)x : (CGFloat)y : (CGFloat)w : (CGFloat)h
{
  [self subclassResponsibility: _cmd];
}

- (void) DPSrectstroke: (CGFloat)x : (CGFloat)y : (CGFloat)w : (CGFloat)h
{
  [self subclassResponsibility: _cmd];
}

- (void) DPSreversepath
{
  [self subclassResponsibility: _cmd];
}

- (void) DPSrlineto: (CGFloat)x : (CGFloat)y
{
  [self subclassResponsibility: _cmd];
}

- (void) DPSrmoveto: (CGFloat)x : (CGFloat)y
{
  [self subclassResponsibility: _cmd];
}

- (void) DPSstroke
{
  [self subclassResponsibility: _cmd];
}

- (void) DPSshfill: (NSDictionary *)shaderDictionary
{
  [self subclassResponsibility: _cmd];
}

/** Set the bezier path as the current path */
- (void) GSSendBezierPath: (NSBezierPath *)path
{
  [self subclassResponsibility: _cmd];
}

/** Append the array of rects to the current clip path (DPS, Quartz). */
- (void) GSRectClipList: (const NSRect *)rects : (int) count
{
  [self subclassResponsibility: _cmd];
}

/** Draw and fill the array of rects. (DPS, Quartz) */
- (void) GSRectFillList: (const NSRect *)rects : (int) count
{
  [self subclassResponsibility: _cmd];
}

/* ----------------------------------------------------------------------- */
/* Window system ops */
/* ----------------------------------------------------------------------- */
/** This is a private method used between the window server and the context.
    It should not be used in any application. Typically used by the
    window server to find out what window the context is drawing graphics
    to. The device pointer is an opaque type that contains information 
    about the window. The x and y pointers indicate the offset of the
    origin of the window from the lower left-hand corner */
- (void) GSCurrentDevice: (void **)device : (int *)x : (int *)y
{
  [self subclassResponsibility: _cmd];
}

- (void) DPScurrentoffset: (int *)x : (int *)y
{
  [self subclassResponsibility: _cmd];
}

/** This is a private method used between the window server and the context.
    It should not be used in any application. Typically called by the
    window server to tell the context what window it should draw graphics
    to. The device pointer is an opaque type that contains information 
    about the window. The x and y values tell the context that it
    should put the origin of the transform matrix at the indicated
    x and y values from the lower left-hand corner of the window */
- (void) GSSetDevice: (void *)device : (int)x : (int)y
{
  [self subclassResponsibility: _cmd];
}

- (void) DPSsetoffset: (short int)x : (short int)y
{
  [self subclassResponsibility: _cmd];
}

/*-------------------------------------------------------------------------*/
/* Graphics Extension Ops */
/*-------------------------------------------------------------------------*/
- (void) DPScomposite: (CGFloat)x : (CGFloat)y : (CGFloat)w : (CGFloat)h 
		     : (NSInteger)gstateNum : (CGFloat)dx : (CGFloat)dy : (NSCompositingOperation)op
{
  [self subclassResponsibility: _cmd];
}

- (void) DPScompositerect: (CGFloat)x : (CGFloat)y : (CGFloat)w : (CGFloat)h : (NSCompositingOperation)op
{
  [self subclassResponsibility: _cmd];
}

- (void) DPSdissolve: (CGFloat)x : (CGFloat)y : (CGFloat)w : (CGFloat)h 
		    : (NSInteger)gstateNum : (CGFloat)dx : (CGFloat)dy : (CGFloat)delta
{
  [self subclassResponsibility: _cmd];
}

/*
  As currently not all backends support mixed composite and dissolve operations, 
  this method is here to dispatch to the best suited one implemented
 */
- (void) GScomposite: (NSInteger)gstateNum
	     toPoint: (NSPoint)aPoint
	    fromRect: (NSRect)srcRect
	   operation: (NSCompositingOperation)op
	    fraction: (CGFloat)delta
{
  [self subclassResponsibility: _cmd];
}

/** <override-dummy />
Returns whether the backend supports a GSDraw operator.

By default, returns NO.<br />
When a GSContext backend subclass overrides this method to return YES, the 
backend must also implement -drawGState:fromRect:toPoint:op:fraction: in its 
GSState subclass.

When YES is returned, -[NSImage drawXXX] methods that involves rotation, 
scaling etc. will delegate as much as possible the image drawing to the backend, 
rather than trying to emulate the resulting image in Gui by using intermediate 
images to rotate and scale the content, and then composite the result with 
-GScomposite:toPoint:fromRect:operation:fraction:.

Backends which doesn't implement -compositeGState:fromRect:toPoint:op:fraction: 
can draw rotated or scaled images, but the semantic won't exactly match the 
NSImage documentation in non-trivial cases. */
- (BOOL) supportsDrawGState
{
  return NO;
}

/** <override-dummy />
Draws a gstate in a way that fully respects the destination transform, 
unlike the GSComposite operator which ignores the rotation and the scaling 
effect on the content.

Note: For the GScomposite operator, the scaling and rotation affects the 
destination point but not the content. */
- (void) GSdraw: (NSInteger)gstateNum
        toPoint: (NSPoint)aPoint
       fromRect: (NSRect)srcRect
      operation: (NSCompositingOperation)op
       fraction: (CGFloat)delta
{
  [self subclassResponsibility: _cmd];
}

/** Generic method to draw an image into a rect. The image is defined
    by imageref, an opaque structure. Support for this method hasn't
    been implemented yet, so it should not be used anywhere. */
- (void) GSDrawImage: (NSRect)rect : (void *)imageref
{
  [self subclassResponsibility: _cmd];
}

/* ----------------------------------------------------------------------- */
/* Client functions */
/* ----------------------------------------------------------------------- */
/** Write the string (with printf substitutions) to a PostScript context.
    Other output contexts will likely ignore this */
- (void) DPSPrintf: (const char *)fmt : (va_list)args
{
  [self subclassResponsibility: _cmd];
}

/** Write the encoded data to a PostScript context.
    Other output contexts will likely ignore this */
- (void) DPSWriteData: (const char *)buf : (unsigned int)count
{
  [self subclassResponsibility: _cmd];
}

@end

/* ----------------------------------------------------------------------- */
/* NSGraphics Ops */	
/* ----------------------------------------------------------------------- */
@implementation NSGraphicsContext (NSGraphics)

- (NSDictionary *) GSReadRect: (NSRect) rect
{
  return nil;
}

/** Generic method to render bitmap images. This method shouldn't be used
    anywhere except in the AppKit itself. It will be replaced by the more
    flexible GSDrawImage method sometime in the future. (Quartz).
 */
- (void) NSDrawBitmap: (NSRect) rect : (NSInteger) pixelsWide : (NSInteger) pixelsHigh
		     : (NSInteger) bitsPerSample : (NSInteger) samplesPerPixel 
		     : (NSInteger) bitsPerPixel : (NSInteger) bytesPerRow : (BOOL) isPlanar
		     : (BOOL) hasAlpha : (NSString *) colorSpaceName
		     : (const unsigned char *const [5]) data
{
  [self subclassResponsibility: _cmd];
}

/** Play the System Beep */
- (void) NSBeep
{
  [GSCurrentServer() beep];
}

/** This method is used by the backend, but has been rendered obsolete.
    Do not use it in any code or in any backend implementation as it
    may disappear at any point. */
- (void) GSWSetViewIsFlipped: (BOOL) flipped
{
}

/** Returns YES if the current focused view is flipped. This is an
    obsolete method. Use [[NSView focusView] isFlipped] instead */
- (BOOL) GSWViewIsFlipped
{
  return [[self focusView] isFlipped];
}

@end

@implementation NSGraphicsContext (Printing)

- (void) beginPage: (int)ordinalNum
             label: (NSString*)aString
              bBox: (NSRect)pageRect
             fonts: (NSString*)fontNames
{
  if (aString == nil)
    aString = [[NSNumber numberWithInt: ordinalNum] description];
  DPSPrintf(self, "%%%%Page: %s %d\n", [aString lossyCString], ordinalNum);
  if (NSIsEmptyRect(pageRect) == NO)
    DPSPrintf(self, "%%%%PageBoundingBox: %d %d %d %d\n",
	      (int)NSMinX(pageRect), (int)NSMinY(pageRect), 
	      (int)NSMaxX(pageRect), (int)NSMaxY(pageRect));
  if (fontNames)
    DPSPrintf(self, "%%%%PageFonts: %s\n", [fontNames lossyCString]);
  DPSPrintf(self, "%%%%BeginPageSetup\n");
}

- (void) beginPrologueBBox: (NSRect)boundingBox
              creationDate: (NSString*)dateCreated
                 createdBy: (NSString*)anApplication
                     fonts: (NSString*)fontNames
                   forWhom: (NSString*)user
                     pages: (int)numPages
                     title: (NSString*)aTitle
{
  NSPrintOperation *printOp = [NSPrintOperation currentOperation];
  NSPrintingOrientation orient;
  BOOL epsOp;

  epsOp = [printOp isEPSOperation];
  orient = [[printOp printInfo] orientation];

  if (epsOp)
    DPSPrintf(self, "%%!PS-Adobe-3.0 EPSF-3.0\n");
  else
    DPSPrintf(self, "%%!PS-Adobe-3.0\n");
  DPSPrintf(self, "%%%%Title: %s\n", [aTitle lossyCString]);
  DPSPrintf(self, "%%%%Creator: %s\n", [anApplication lossyCString]);
  DPSPrintf(self, "%%%%CreationDate: %s\n", 
	    [[dateCreated description] lossyCString]);
  DPSPrintf(self, "%%%%For: %s\n", [user lossyCString]);
  if (fontNames)
    DPSPrintf(self, "%%%%DocumentFonts: %s\n", [fontNames lossyCString]);
  else
    DPSPrintf(self, "%%%%DocumentFonts: (atend)\n");

  if (NSIsEmptyRect(boundingBox) == NO)
    DPSPrintf(self, "%%%%BoundingBox: %d %d %d %d\n", 
              (int)NSMinX(boundingBox), (int)NSMinY(boundingBox), 
              (int)NSMaxX(boundingBox), (int)NSMaxY(boundingBox));
  else
    DPSPrintf(self, "%%%%BoundingBox: (atend)\n");

  if (epsOp == NO)
    {
      if (numPages)
        DPSPrintf(self, "%%%%Pages: %d\n", numPages);
      else
        DPSPrintf(self, "%%%%Pages: (atend)\n");
      if ([printOp pageOrder] == NSDescendingPageOrder)
        DPSPrintf(self, "%%%%PageOrder: Descend\n");
      else if ([printOp pageOrder] == NSAscendingPageOrder)
        DPSPrintf(self, "%%%%PageOrder: Ascend\n");
      else if ([printOp pageOrder] == NSSpecialPageOrder)
        DPSPrintf(self, "%%%%PageOrder: Special\n");

      if (orient == NSPortraitOrientation)
        DPSPrintf(self, "%%%%Orientation: Portrait\n");
      else
        DPSPrintf(self, "%%%%Orientation: Landscape\n");
    }

  DPSPrintf(self, "%%%%GNUstepVersion: %d.%d.%d\n", 
	    GNUSTEP_GUI_MAJOR_VERSION, GNUSTEP_GUI_MINOR_VERSION,
	    GNUSTEP_GUI_SUBMINOR_VERSION);
}

- (void) beginSetup
{
  DPSPrintf(self, "%%%%BeginSetup\n");
}

- (void) beginTrailer
{
  DPSPrintf(self, "%%%%Trailer\n");
}

- (void) endDocumentPages: (int)pages
            documentFonts: (NSSet*)fontNames
{
  if (pages != 0)
    {
      DPSPrintf(self, "%%%%Pages: %d\n", pages);
    }
  if (fontNames && [fontNames count])
    {
      NSString *name;
      NSEnumerator *e = [fontNames objectEnumerator];

      DPSPrintf(self, "%%%%DocumentFonts: %@\n", [e nextObject]);
      while ((name = [e nextObject]))
        {
          DPSPrintf(self, "%%%%+ %@\n", name);
        }
    }
 
}

- (void) endHeaderComments
{
  DPSPrintf(self, "%%%%EndComments\n\n");
}

- (void) endPageSetup
{
  DPSPrintf(self, "%%%%EndPageSetup\n");
}

- (void) endPrologue
{
  DPSPrintf(self, "%%%%EndProlog\n\n");
}

- (void) endSetup
{
  DPSPrintf(self, "%%%%EndSetup\n\n");
}

- (void) endSheet
{
  NSPrintOperation *printOp = [NSPrintOperation currentOperation];

  if ([printOp isEPSOperation] == NO)
    {
      [self showPage];
    }
  DPSPrintf(self, "%%%%PageTrailer\n\n");
}

- (void) endTrailer
{
  DPSPrintf(self, "%%%%EOF\n");
}

- (void) printerProlog
{
  NSString *prolog;

  DPSPrintf(self, "%%%%BeginProlog\n");
  prolog = [NSBundle pathForLibraryResource: @"GSProlog"
                     ofType: @"ps"
                     inDirectory: @"PostScript"];
  if (prolog == nil)
    {
      NSLog(@"Cannot find printer prolog file");
      return;
    }
  prolog = [NSString stringWithContentsOfFile: prolog];
  DPSPrintf(self, [prolog cString]);
}

- (void) showPage
{
  DPSPrintf(self, "showpage\n");
}

@end

@implementation NSGraphicsContext (NSGradient)
- (void) drawGradient: (NSGradient*)gradient
           fromCenter: (NSPoint)startCenter
               radius: (CGFloat)startRadius
             toCenter: (NSPoint)endCenter 
               radius: (CGFloat)endRadius
              options: (NSUInteger)options
{
  [self subclassResponsibility: _cmd];
}

- (void) drawGradient: (NSGradient*)gradient
            fromPoint: (NSPoint)startPoint
              toPoint: (NSPoint)endPoint
              options: (NSUInteger)options
{
  [self subclassResponsibility: _cmd];
}

@end
