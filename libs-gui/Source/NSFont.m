/** <title>NSFont</title>

   <abstract>The font class</abstract>

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Ovidiu Predescu <ovidiu@net-community.com>
   Date: February 1997
   A completely rewritten version of the original source by Scott Christley.
   
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

#include "config.h"
#import <Foundation/NSAffineTransform.h>
#import <Foundation/NSCoder.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSString.h>
#import <Foundation/NSUserDefaults.h>
#import <Foundation/NSSet.h>
#import <Foundation/NSMapTable.h>
#import <Foundation/NSException.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSValue.h>

#import "AppKit/NSGraphicsContext.h"
#import "AppKit/NSFont.h"
#import "AppKit/NSFontDescriptor.h"
#import "AppKit/NSFontManager.h"
#import "AppKit/NSView.h"
#import "GNUstepGUI/GSFontInfo.h"


@interface NSFont (Private)
- (id) initWithName: (NSString*)name 
             matrix: (const CGFloat*)fontMatrix
         screenFont: (BOOL)screenFont
               role: (int)role;
+ (NSFont*) _fontWithName: (NSString*)aFontName
                     size: (CGFloat)fontSize
                     role: (int)role;
@end

static int currentVersion = 3;


/*
Instances of GSFontMapKey are used to find cached font instances in
globalFontMap.
*/
@interface GSFontMapKey : NSObject
{
@public
  NSString *name;
  BOOL screenFont;
  int role;
  int matrix[6];

  unsigned int hash;
}
@end
@implementation GSFontMapKey
-(NSUInteger) hash
{
  return hash;
}

-(BOOL) isEqual: (id)other
{
  GSFontMapKey *o;
  if (![other isKindOfClass: object_getClass(self)])
    return NO;
  o = other;
  if (hash != o->hash || screenFont != o->screenFont || role != o->role)
    return NO;
  if (![name isEqualToString: o->name])
    return NO;
  if (matrix[0] != o->matrix[0]
      || matrix[1] != o->matrix[1]
      || matrix[2] != o->matrix[2]
      || matrix[3] != o->matrix[3]
      || matrix[4] != o->matrix[4]
      || matrix[5] != o->matrix[5])
    return NO;
  return YES;
}

-(void) dealloc
{
  DESTROY(name);
  [super dealloc];
}

- (NSString *) description
{
  return [NSString stringWithFormat: @"%@ %d %d [%d %d %d %d %d %d]",
                   name, screenFont, role,
                   matrix[0], matrix[1], matrix[2],
                   matrix[3], matrix[4], matrix[5]];
}

@end

static GSFontMapKey *
keyForFont(NSString *name, const CGFloat *matrix, 
           BOOL screenFont, int role)
{
  GSFontMapKey *d;
  d=[GSFontMapKey alloc];
  d->name = [name copy];
  d->screenFont = screenFont;
  d->role = role;
  d->matrix[0] = matrix[0] * 1000;
  d->matrix[1] = matrix[1] * 1000;
  d->matrix[2] = matrix[2] * 1000;
  d->matrix[3] = matrix[3] * 1000;
  d->matrix[4] = matrix[4] * 1000;
  d->matrix[5] = matrix[5] * 1000;
  d->hash = [d->name hash] + screenFont + role * 4
            + d->matrix[0] + d->matrix[1] + d->matrix[2] + d->matrix[3];
  return d;
}

/**
  <unit>
  <heading>NSFont</heading>

  <p>The NSFont class allows control of the fonts used for displaying
  text anywhere on the screen. The primary methods for getting a
  particular font are +fontWithName:matrix: and +fontWithName:size: which
  take the name and size of a particular font and return the NSFont object
  associated with that font. In addition there are several convenience
  mathods which make it easier to get certain types of fonts. </p>
  
  <p>In particular, there are several methods to get the standard fonts
  used by the Application to display text for a partiuclar purpose. See
  the class methods listed below for more information. These default
  fonts can be set using the user defaults system. The default
  font names available are:
  </p>
  <list>
    <item>NSBoldFont                Helvetica-Bold (System bold font)</item>
    <item>NSControlContentFont      System font</item>
    <item>NSFont                    Helvetica (System Font)</item>
    <item>NSLabelFont               System font</item>
    <item>NSMenuFont                System font</item>
    <item>NSMenuBarFont             System font</item>
    <item>NSMessageFont             System font</item>
    <item>NSPaletteFont             System bold font</item>
    <item>NSTitleBarFont            System bold font</item>
    <item>NSToolTipsFont            System font</item>
    <item>NSUserFixedPitchFont      Courier</item>
    <item>NSUserFont                System font</item>
  </list>
  <p>
  The default sizes are:
  </p>
  <list>
    <item>NSBoldFontSize            (none)</item>
    <item>NSControlContentFontSize  (none)</item>
    <item>NSFontSize                12 (System Font Size)</item>
    <item>NSLabelFontSize           (none)</item>
    <item>NSMenuFontSize            (none)</item>
    <item>NSMiniFontSize            8</item>
    <item>NSMessageFontSize         (none)</item>
    <item>NSPaletteFontSize         (none)</item>
    <item>NSSmallFontSize           10</item>
    <item>NSTitleBarFontSize        (none)</item>
    <item>NSToolTipsFontSize        (none)</item>
    <item>NSUserFixedPitchFontSize  (none)</item>
    <item>NSUserFontSize            (none)</item>
  </list>
  <p>
  Font sizes list with (none) default to NSFontSize.
  </p>

  </unit> */
  
@implementation NSFont

/* Class variables*/

/* See comments in +initialize. */
static NSFont *placeHolder = nil;

/* Fonts that are preferred by the application */
static NSArray *_preferredFonts;

/* Class for fonts */
static Class NSFontClass = 0;

/* Cache all created fonts for reuse. */
static NSMapTable* globalFontMap = 0;

static NSUserDefaults *defaults = nil;


/*
The valid font roles. Note that these values are used when encoding and
decoding, so entries may never be removed. Entries may be added after the
last entry, and entries don't have to actually be handled.

Note that these values are multiplied by two before they are used since the
lowest bit is used to indicate an explicit size. If the lowest bit is set,
the size is explicitly specified and encoded.
*/
enum FontRoles
{
RoleExplicit=0,
RoleBoldSystemFont,
RoleSystemFont,
RoleUserFixedPitchFont,
RoleUserFont,
RoleTitleBarFont,
RoleMenuFont,
RoleMessageFont,
RolePaletteFont,
RoleToolTipsFont,
RoleControlContentFont,
RoleLabelFont,
RoleMenuBarFont,
RoleMax
};

typedef struct
{
  /* Defaults key for this font. */
  NSString *key;

  /* If there's no defaults key, fall back to the font for this role. */
  int fallback;

  /* If there's no other role to fall back to, use this font. */
  NSString *defaultFont;

  /* Cached font for the default size of this role. */
  NSFont *cachedFont;
} font_role_info_t;

/*
This table, through getNSFont, controls the behavior of getting the standard
fonts, and must match the table in the documentation above. Each entry should
have a fallback or a defaultFont. There must be a default font for the system
font. Bad Things will happen if entries are invalid.
*/
static font_role_info_t font_roles[RoleMax]={
  {nil                    , 0                 , nil, nil},
  {@"NSBoldFont"          , 0                 , nil /* set by init_font_roles */, nil},
  {@"NSFont"              , 0                 , nil /* set by init_font_roles */, nil},
  {@"NSUserFixedPitchFont", 0                 , nil /* set by init_font_roles */, nil},
  {@"NSUserFont"          , RoleSystemFont    , nil, nil},
  {@"NSTitleBarFont"      , RoleBoldSystemFont, nil, nil},
  {@"NSMenuFont"          , RoleSystemFont    , nil, nil},
  {@"NSMessageFont"       , RoleSystemFont    , nil, nil},
  {@"NSPaletteFont"       , RoleBoldSystemFont, nil, nil},
  {@"NSToolTipsFont"      , RoleSystemFont    , nil, nil},
  {@"NSControlContentFont", RoleSystemFont    , nil, nil},
  {@"NSLabelFont"         , RoleSystemFont    , nil, nil},
  {@"NSMenuBarFont"       , RoleSystemFont    , nil, nil}
};


static BOOL did_init_font_roles;

/*
Called by getNSFont, since font_roles is only accessed from that function
(or fontNameForRole, which is only called by getNSFont). This assures that the
function is called before the table is used, and that it's called _after_ the
backend has been loaded (or, if it isn't, the _fontWithName:... calls will
fail anyway).
*/
static void init_font_roles(void)
{
  GSFontEnumerator *e = [GSFontEnumerator sharedEnumerator];

  font_roles[RoleSystemFont].defaultFont = [e defaultSystemFontName];
  font_roles[RoleBoldSystemFont].defaultFont = [e defaultBoldSystemFontName];
  font_roles[RoleUserFixedPitchFont].defaultFont = [e defaultFixedPitchFontName];
}


static NSString *fontNameForRole(int role, int *actual_entry)
{
  int i;
  NSString *fontName;

  i = role;
  while (1)
    {
      fontName = [defaults stringForKey: font_roles[i].key];
      if (fontName)
        {
          break;
        }
      else if (font_roles[i].fallback)
        {
          i = font_roles[i].fallback;
        }
      else if (font_roles[i].defaultFont)
        {
          fontName = font_roles[i].defaultFont;
          break;
        }
      else
        {
          NSCAssert(NO, @"Invalid font role table entry.");
        }
    }
  if (actual_entry)
    *actual_entry = i;
  return fontName;
}

static NSFont *getNSFont(CGFloat fontSize, int role)
{
  NSString *fontName;
  NSFont *font;
  BOOL defaultSize;
  int i;
  int font_role;

  NSCAssert(role > RoleExplicit && role < RoleMax, @"Invalid font role.");

  if (!did_init_font_roles)
    {
      init_font_roles();
      did_init_font_roles = YES;
    }

  font_role = role * 2;

  defaultSize = (fontSize <= 0.0);
  if (defaultSize)
    {
      if (font_roles[role].cachedFont)
        return AUTORELEASE(RETAIN(font_roles[role].cachedFont));

      fontSize = [defaults floatForKey:
        [NSString stringWithFormat: @"%@Size", font_roles[role].key]];

      if (!fontSize)
        fontSize = [NSFont systemFontSize];
    }
  else
    {
      font_role |= 1;
    }

  fontName = fontNameForRole(role, &i);
  font = [NSFontClass _fontWithName: fontName
                               size: fontSize
                               role: font_role];

  /* That font couldn't be found. */
  if (font == nil)
    {
      /* Warn using the role that specified the invalid font. */
      NSLog(@"The font specified for %@, %@, can't be found.",
        font_roles[i].key, fontName);

      /* Try the system font. */
      fontName = fontNameForRole(RoleSystemFont, NULL);
      font = [NSFontClass _fontWithName: fontName
                                    size: fontSize
                                   role: font_role];
      
      if (font == nil)
        {
          /* Try the default system font and size. */
          fontName = font_roles[RoleSystemFont].defaultFont;
          font = [NSFontClass _fontWithName: fontName
                                       size: 12.0
                                       role: font_role];

          /* It seems we can't get any font here!  Try some well known
           * fonts as a last resort.  */
          if (font == nil)
            {
              font = [NSFontClass _fontWithName: @"Helvetica"
                                           size: 12.0
                                           role: font_role];
            }
          if (font == nil)
            {
              font = [NSFontClass _fontWithName: @"Courier"
                                           size: 12.0
                                           role: font_role];
            }
          if (font == nil)
            {
              font = [NSFontClass _fontWithName: @"Fixed"
                                           size: 12.0
                                           role: font_role];
            }
        }
    }

  if (defaultSize)
    ASSIGN(font_roles[role].cachedFont, font);

  return font;
}

static void setNSFont(NSString *key, NSFont *font)
{
  int i;

  [defaults setObject: [font fontName] forKey: key];
  [defaults setObject: [NSNumber numberWithFloat: [font pointSize]]
            forKey: [NSString stringWithFormat: @"%@Size",key]];
  
  for (i = 1; i < RoleMax; i++)
    {
      DESTROY(font_roles[i].cachedFont);
    }

  /* Don't care about errors */
  [defaults synchronize];
}


//
// Class methods
//
+ (void) initialize
{
  if (self == [NSFont class])
    {
      NSFontClass = self;

      /*
       * The placeHolder is a dummy NSFont instance which is never used
       * as a font ... the initialiser knows that whenever it gets the
       * placeHolder it should either return a cached font or return a
       * newly allocated font to replace it.  This mechanism stops the
       * +fontWithName:... methods from having to allocate fonts instances
       * which would immediately have to be released for replacement by
       * a cache object.
       */
      placeHolder = [self alloc];
      globalFontMap = NSCreateMapTable(NSObjectMapKeyCallBacks,
                                       NSNonRetainedObjectMapValueCallBacks, 64);

      if (defaults == nil)
        {
          defaults = RETAIN([NSUserDefaults standardUserDefaults]);
        }

      _preferredFonts = [defaults objectForKey: @"NSPreferredFonts"];
      [self setVersion: currentVersion];
    }
}

/* Getting the preferred user fonts.  */

/**<p>Returns the default bold font for use in menus and heading in standard
  gui components.  If fontSize is &lt;= 0, the default
  size is used.</p><p>See Also: +fontWithName:size:</p>
 */
+ (NSFont*) boldSystemFontOfSize: (CGFloat)fontSize
{
  return getNSFont(fontSize, RoleBoldSystemFont);
}

/**<p> Returns the default font for use in menus and heading in standard
   gui components.  If fontSize is &lt;= 0, the default
   size is used.</p><p>See Also: +boldSystemFontOfSize: userFontOfSize:
   userFixedPitchFontOfSize: +fontWithName:size:</p>
 */
+ (NSFont*) systemFontOfSize: (CGFloat)fontSize
{
  return getNSFont(fontSize, RoleSystemFont);
}

/**<p>Returns the default fixed pitch font for use in locations other
   than standard gui components.  If fontSize is &lt;= 0, the default
   size is used.</p><p>See Also: +setUserFixedPitchFont: +userFontOfSize: 
   +boldSystemFontOfSize: +systemFontOfSize: +fontWithName:size:</p>
 */
+ (NSFont*) userFixedPitchFontOfSize: (CGFloat)fontSize
{
  return getNSFont(fontSize, RoleUserFixedPitchFont);
}

/**<p> Returns the default font for use in locations other
  than standard gui components.  If fontSize is &lt;= 0, the default
  size is used.</p><p>See Also: +setUserFont: +boldSystemFontOfSize: 
  systemFontOfSize: userFixedPitchFontOfSize: +fontWithName:size:</p>
 */
+ (NSFont*) userFontOfSize: (CGFloat)fontSize
{
  return getNSFont(fontSize, RoleUserFont);
}

+ (NSFont *) fontWithDescriptor: (NSFontDescriptor *)descriptor
                           size: (CGFloat)size
{
  NSArray *a;

  descriptor = [descriptor matchingFontDescriptorWithMandatoryKeys: 
    [NSSet setWithArray: [[descriptor fontAttributes] allKeys]]];

  if (descriptor == nil)
    return nil;

  a = [[NSFontManager sharedFontManager] availableFontNamesMatchingFontDescriptor: 
					   descriptor];
  if ((a == nil) || ([a count] == 0))
    return nil;

  return [self fontWithName: [a objectAtIndex: 0]
	       size: size];
}

+ (NSFont*) fontWithDescriptor: (NSFontDescriptor*)descriptor 
                 textTransform: (NSAffineTransform*)transform
{
  NSArray *a;
  CGFloat fontMatrix[6];
  NSAffineTransformStruct ats;

  descriptor = [descriptor matchingFontDescriptorWithMandatoryKeys: 
    [NSSet setWithArray: [[descriptor fontAttributes] allKeys]]];

  if (descriptor == nil)
    return nil;

  a = [[NSFontManager sharedFontManager] availableFontNamesMatchingFontDescriptor: 
					   descriptor];
  if ((a == nil) || ([a count] == 0))
    return nil;

  ats = [transform transformStruct];
  fontMatrix[0] = ats.m11;
  fontMatrix[1] = ats.m12;
  fontMatrix[2] = ats.m21;
  fontMatrix[3] = ats.m22;
  fontMatrix[4] = ats.tX;
  fontMatrix[5] = ats.tY;

  return [self fontWithName: [a objectAtIndex: 0]
	       matrix: fontMatrix];
}

+ (NSFont *) fontWithDescriptor: (NSFontDescriptor *)descriptor
                           size: (CGFloat)size
                  textTransform: (NSAffineTransform *)transform
{
  if (transform)
    {
      return [self fontWithDescriptor: descriptor 
		   textTransform: transform];
    }
  else
    {
      return [self fontWithDescriptor: descriptor
		   size: size];
    }
}


/**<p>Returns an array of the names of preferred fonts.</p>
   <p>See Also: +setPreferredFontNames:</p>
 */
+ (NSArray*) preferredFontNames
{
  return _preferredFonts;
}

/* Setting the preferred user fonts*/

+ (void) setUserFixedPitchFont: (NSFont*)aFont
{
  setNSFont (@"NSUserFixedPitchFont", aFont);
}

+ (void) setUserFont: (NSFont*)aFont
{
  setNSFont (@"NSUserFont", aFont);
}

/** <p>Sets an array of the names of preferred fonts to fontsNames/</p>
    <p>See Also: +preferredFontNames</p>
 */
+ (void) setPreferredFontNames: (NSArray*)fontNames
{
  ASSIGN(_preferredFonts, fontNames);
  // FIXME: Should this store back the preferred fonts in the user defaults?
}

/* Getting various fonts*/

+ (NSFont*) controlContentFontOfSize: (CGFloat)fontSize
{
  return getNSFont(fontSize, RoleControlContentFont);
}

+ (NSFont*) labelFontOfSize: (CGFloat)fontSize
{
  return getNSFont(fontSize, RoleLabelFont);
}

+ (NSFont*) menuFontOfSize: (CGFloat)fontSize
{
  return getNSFont(fontSize, RoleMenuFont);
}

+ (NSFont*) menuBarFontOfSize: (CGFloat)fontSize
{
  return getNSFont(fontSize, RoleMenuBarFont);
}

+ (NSFont*) titleBarFontOfSize: (CGFloat)fontSize
{
  return getNSFont(fontSize, RoleTitleBarFont);
}

+ (NSFont*) messageFontOfSize: (CGFloat)fontSize
{
  return getNSFont(fontSize, RoleMessageFont);
}

+ (NSFont*) paletteFontOfSize: (CGFloat)fontSize
{
  return getNSFont(fontSize, RolePaletteFont);
}

+ (NSFont*) toolTipsFontOfSize: (CGFloat)fontSize
{
  return getNSFont(fontSize, RoleToolTipsFont);
}

//
// Font Sizes
//
+ (CGFloat) labelFontSize
{
  CGFloat fontSize = [defaults floatForKey: @"NSLabelFontSize"];
  
  if (fontSize == 0)
    {
      return [self systemFontSize];
    }

  return fontSize;
}

+ (CGFloat) smallSystemFontSize
{
  CGFloat fontSize = [defaults floatForKey: @"NSSmallFontSize"];
  
  if (fontSize == 0)
    {
      fontSize = 10;
    }

  return fontSize;
}

+ (CGFloat) systemFontSize
{
  CGFloat fontSize = [defaults floatForKey: @"NSFontSize"];
  
  if (fontSize == 0)
    {
      fontSize = 12;
    }

  return fontSize;
}

+ (CGFloat) systemFontSizeForControlSize: (NSControlSize)controlSize
{
  switch (controlSize)
    {
      case NSMiniControlSize:
        {
          CGFloat fontSize = [defaults floatForKey: @"NSMiniFontSize"];
  
          if (fontSize == 0)
            {
              fontSize = 8;
            }
          
          return fontSize;
        }
      case NSSmallControlSize:
        return [self smallSystemFontSize];
      case NSRegularControlSize:
      default:
        return [self systemFontSize];
    }
}

/** <p>Returns an autoreleased font with name aFontName and matrix fontMatrix
    .</p><p>The fontMatrix is a standard size element matrix as used in
    PostScript to describe the scaling of the font, typically it just includes
    the font size as [fontSize 0 0 fontSize 0 0]. You can use the constant
    NSFontIdentityMatrix in place of [1 0 0 1 0 0]. If NSFontIdentityMatrix, 
    then the font will automatically flip itself when set in a flipped view.
    </p>
 */
+ (NSFont*) fontWithName: (NSString*)aFontName 
                  matrix: (const CGFloat*)fontMatrix
{
  NSFont *font;

  font = [placeHolder initWithName: aFontName
                            matrix: fontMatrix
                        screenFont: NO
                              role: RoleExplicit];

  return AUTORELEASE(font);
}

/**<p> Returns an autoreleased font with name aFontName and size fontSize.</p>
 * <p>Fonts created using this method will automatically flip themselves
 * when set in a flipped view.</p>
 */
+ (NSFont*) fontWithName: (NSString*)aFontName
                    size: (CGFloat)fontSize
{
  return [self _fontWithName: aFontName
                        size: fontSize
                        role: RoleExplicit];
}

+ (NSFont*) _fontWithName: (NSString*)aFontName
                     size: (CGFloat)fontSize
                     role: (int)aRole
{
  NSFont *font;
  CGFloat fontMatrix[6] = { 0, 0, 0, 0, 0, 0 };

  if (fontSize == 0)
    {
      fontSize = [defaults floatForKey: @"NSUserFontSize"];
      if (fontSize == 0)
        {
          fontSize = 12;
        }
    }
  fontMatrix[0] = fontSize;
  fontMatrix[3] = fontSize;

  font = [placeHolder initWithName: aFontName
                            matrix: fontMatrix
                        screenFont: NO
                              role: aRole];
  return AUTORELEASE(font);
}

/**
 */
+ (void) useFont: (NSString*)aFontName
{
  [GSCurrentContext() useFont: aFontName];
}

//
// Instance methods
//
- (id) init
{
  [NSException raise: NSInternalInconsistencyException
              format: @"Called -init on NSFont ... illegal"];
  return self;
}

/*
  Last fallback: If a system font was explicitly requested
  and this font does not exist, try to replace it with the
  corresponding font in the current setup.
*/
- (NSString*) _replacementFontName
{
  if (([fontName isEqualToString: @"Helvetica"] &&
       ![font_roles[RoleSystemFont].defaultFont isEqualToString: @"Helvetica"])
      || ([fontName isEqualToString: @"LucidaGrande"]))
    {
      return font_roles[RoleSystemFont].defaultFont;
    }
  else if (([fontName isEqualToString: @"Helvetica-Bold"] &&
            ![font_roles[RoleBoldSystemFont].defaultFont isEqualToString: @"Helvetica-Bold"])
           || ([fontName isEqualToString: @"LucidaGrande-Bold"]))
    {
      return font_roles[RoleBoldSystemFont].defaultFont;
    }
  else if ([fontName isEqualToString: @"Courier"] &&
           ![font_roles[RoleUserFixedPitchFont].defaultFont isEqualToString: @"Courier"])
    {
      return font_roles[RoleUserFixedPitchFont].defaultFont;
    }
  else if ([fontName hasPrefix: @"Helvetica-"] &&
       ![font_roles[RoleSystemFont].defaultFont isEqualToString: @"Helvetica"])
    {
      return [NSString stringWithFormat: @"%@-%@",
                       font_roles[RoleSystemFont].defaultFont,
                       [fontName substringFromIndex: 10]];
    }
  return nil;
}

/** <init />
 * Initializes a newly created font instance from the name and
 * information given in the fontMatrix. The fontMatrix is a standard
 * size element matrix as used in PostScript to describe the scaling
 * of the font, typically it just includes the font size as
 * [fontSize 0 0 fontSize 0 0].<br />
 * This method may destroy the receiver and return a cached instance.
 */
- (id) initWithName: (NSString*)name
             matrix: (const CGFloat*)fontMatrix
         screenFont: (BOOL)screen
               role: (int)aRole
{
  GSFontMapKey *key;
  NSFont *font;

  /* Should never be called on an initialised font! */
  NSAssert(fontName == nil, NSInternalInconsistencyException);

  /* Check whether the font is cached */
  key = keyForFont(name, fontMatrix,
                   screen, aRole);
  font = (id)NSMapGet(globalFontMap, (void *)key);
  if (font == nil)
    {
      if (self == placeHolder)
        {
          /*
           * If we are initialising the placeHolder, we actually want to
           * leave it be (for later re-use) and initialise a newly created
           * instance instead.
           */
          self = [NSFontClass alloc];
        }
      fontName = [name copy];
      memcpy(matrix, fontMatrix, sizeof(matrix));
      screenFont = screen;
      role = aRole;
      fontInfo = RETAIN([GSFontInfo fontInfoForFontName: fontName
                                                 matrix: fontMatrix
                                             screenFont: screen]);
      if ((fontInfo == nil) && (aRole == RoleExplicit))
        {
          NSString *replacementFontName = [self _replacementFontName];

          if (replacementFontName != nil)
            {
              fontInfo = RETAIN([GSFontInfo fontInfoForFontName: replacementFontName
                                                         matrix: fontMatrix
                                                     screenFont: screen]);
            }
        }
      if (fontInfo == nil)
        {
          DESTROY(fontName);
          DESTROY(key);
          RELEASE(self);
          return nil;
        }
      
      /* Cache the font for later use */
      NSMapInsert(globalFontMap, (void *)key, (void *)self);
    }
  else
    {
      if (self != placeHolder)
        {
          RELEASE(self);
        }
      self = RETAIN(font);
    }
  RELEASE(key);

  return self;
}

- (void) dealloc
{
  if (fontName != nil)
    {
      GSFontMapKey *key;

      key = keyForFont(fontName, matrix,
                       screenFont, role);
      NSMapRemove(globalFontMap, (void *)key);
      RELEASE(key);
      RELEASE(fontName);
    }
  TEST_RELEASE(fontInfo);
  DESTROY(cachedFlippedFont);
  DESTROY(cachedScreenFont);
  [super dealloc];
}

- (NSString *) description
{
  NSString *nameWithMatrix;
  NSString *description;

  nameWithMatrix = [[NSString alloc] initWithFormat:
    @"%@ %.3f %.3f %.3f %.3f %.3f %.3f %c %i", fontName,
    matrix[0], matrix[1], matrix[2], matrix[3], matrix[4], matrix[5],
    screenFont ? 'S' : 'P',
    role];
  description = [[super description] stringByAppendingFormat: @" %@",
    nameWithMatrix];
  RELEASE(nameWithMatrix);
  return description;
}

- (BOOL) isEqual: (id)anObject
{
  if (anObject == self)
    return YES;
  if ([anObject isKindOfClass: object_getClass(self)] == NO)
    return NO;
  if ([[anObject fontName] isEqual: fontName] == NO)
    return NO;
  if (memcmp(matrix, [(NSFont*)anObject matrix], sizeof(matrix)) != 0)
    return NO;
  return YES;
}

- (NSUInteger) hash
{
  int i, sum;
  sum = 0;
  for (i = 0; i < 6; i++)
    sum += matrix[i] * ((i+1) * 17);
  return ([fontName hash] + sum);
}

/**
 * The NSFont class caches instances ... to actually make copies
 * of instances would defeat the whole point of caching, so the
 * effect of copying an NSFont is imply to retain it.
 */
- (id) copyWithZone: (NSZone*)zone
{
  return RETAIN(self);
}

- (NSFont *)_flippedViewFont
{
  if (cachedFlippedFont == nil)
    {
      CGFloat fontMatrix[6];
      memcpy(fontMatrix, matrix, sizeof(matrix));
      fontMatrix[3] *= -1;
      cachedFlippedFont = [placeHolder initWithName: fontName
                                             matrix: fontMatrix
                                         screenFont: screenFont
                                               role: role];
    }
  return AUTORELEASE(RETAIN(cachedFlippedFont));
}

static BOOL flip_hack;
+(void) _setFontFlipHack: (BOOL)flip
{
  flip_hack = flip;
}

//
// Setting the Font
//
/** Sets the receiver as the font used for text drawing operations. If the
    current view is a flipped view, the reciever automatically flips itself
    to display correctly in the flipped view */
- (void) set
{
  [self setInContext: GSCurrentContext()];
}

- (void) setInContext: (NSGraphicsContext*)context
{
  if ([[NSView focusView] isFlipped] || flip_hack)
    [context GSSetFont: [[self _flippedViewFont] fontRef]];
  else
    [context GSSetFont: [self fontRef]];

  [context useFont: fontName];
}

//
// Querying the Font
//
- (CGFloat) pointSize 
{ 
  return [fontInfo pointSize]; 
}

- (NSString*) fontName
{ 
  return fontName; 
}

- (const CGFloat*) matrix                
{ 
  return matrix; 
}

- (NSAffineTransform*) textTransform
{
  NSAffineTransform *transform;
  NSAffineTransformStruct tstruct;

  tstruct.m11 = matrix[0];
  tstruct.m12 = matrix[1];
  tstruct.m21 = matrix[2];
  tstruct.m22 = matrix[3];
  tstruct.tX = matrix[4];
  tstruct.tY = matrix[5];
 
  transform = [NSAffineTransform transform];
  [transform setTransformStruct: tstruct];
  return transform;
}

- (NSString*) encodingScheme
{
  return [fontInfo encodingScheme]; 
}

- (NSString*) familyName
{ 
  return [fontInfo familyName]; 
}

- (NSRect) boundingRectForFont        
{
  return [fontInfo boundingRectForFont]; 
}

- (BOOL) isFixedPitch
{
  return [fontInfo isFixedPitch]; 
}

- (BOOL) isBaseFont
{
  return [fontInfo isBaseFont]; 
}

/* Usually the display name of font is the font name.*/
- (NSString*) displayName
{
  return fontName; 
}

- (NSDictionary*) afmDictionary
{ 
  return [fontInfo afmDictionary]; 
}

/**<p>This method returns nil in the GNUstep implementation</p>
 */
- (NSString*) afmFileContents
{ 
  return [fontInfo afmFileContents]; 
}

- (NSFont*) printerFont
{
  if (!screenFont)
    return self;
  return AUTORELEASE([placeHolder initWithName: fontName
                            matrix: matrix
                        screenFont: NO
                              role: role]);
}

- (NSFont*) screenFont
{
  if (screenFont)
    return self;
  /*
  If we haven't already created the real screen font instance, do so now.
  Note that if the font has no corresponding screen font, cachedScreenFont
  will be set to nil.
  */
  if (cachedScreenFont == nil)
    cachedScreenFont = [placeHolder initWithName: fontName
                            matrix: matrix
                        screenFont: YES
                              role: role];
  return AUTORELEASE(RETAIN(cachedScreenFont));
}

- (NSFont*) screenFontWithRenderingMode: (NSFontRenderingMode)mode
{
  // FIXME
  return [self screenFont];
}

- (NSFontRenderingMode) renderingMode
{
  // FIXME
  return NSFontDefaultRenderingMode;
}

- (CGFloat) ascender                { return [fontInfo ascender]; }
- (CGFloat) descender                { return [fontInfo descender]; }
- (CGFloat) capHeight                { return [fontInfo capHeight]; }
- (CGFloat) italicAngle                { return [fontInfo italicAngle]; }
- (NSSize) maximumAdvancement        { return [fontInfo maximumAdvancement]; }
- (NSSize) minimumAdvancement        { return [fontInfo minimumAdvancement]; }
- (CGFloat) underlinePosition        { return [fontInfo underlinePosition]; }
- (CGFloat) underlineThickness        { return [fontInfo underlineThickness]; }
- (CGFloat) xHeight                { return [fontInfo xHeight]; }
- (CGFloat) defaultLineHeightForFont { return [fontInfo defaultLineHeightForFont]; }

- (CGFloat) leading
{
  // FIXME
  return 0.0;
}

/* Computing font metrics attributes*/
- (CGFloat) widthOfString: (NSString*)string
{
  return [fontInfo widthOfString: string];
}

- (NSUInteger) numberOfGlyphs
{
  return [fontInfo numberOfGlyphs];
}

- (NSCharacterSet*) coveredCharacterSet
{
  return [fontInfo coveredCharacterSet];
}

- (NSFontDescriptor*) fontDescriptor
{
  return [fontInfo fontDescriptor];
}

/* The following methods have to be implemented by backends */

//
// Manipulating Glyphs
//
- (NSSize) advancementForGlyph: (NSGlyph)aGlyph
{
  return [fontInfo advancementForGlyph: aGlyph];
}

- (NSRect) boundingRectForGlyph: (NSGlyph)aGlyph
{
  return [fontInfo boundingRectForGlyph: aGlyph];
}

- (BOOL) glyphIsEncoded: (NSGlyph)aGlyph
{
  return [fontInfo glyphIsEncoded: aGlyph];
}

- (NSMultibyteGlyphPacking) glyphPacking
{
  return [fontInfo glyphPacking];
}

- (NSGlyph) glyphWithName: (NSString*)glyphName
{
  return [fontInfo glyphWithName: glyphName];
}

- (NSPoint) positionOfGlyph: (NSGlyph)curGlyph
            precededByGlyph: (NSGlyph)prevGlyph
                  isNominal: (BOOL*)nominal
{
  return [fontInfo positionOfGlyph: curGlyph precededByGlyph: prevGlyph
                         isNominal: nominal];
}

- (NSPoint) positionOfGlyph: (NSGlyph)aGlyph 
               forCharacter: (unichar)aChar 
             struckOverRect: (NSRect)aRect
{
  return [fontInfo positionOfGlyph: aGlyph 
                      forCharacter: aChar 
                    struckOverRect: aRect];
}

- (NSPoint) positionOfGlyph: (NSGlyph)aGlyph 
            struckOverGlyph: (NSGlyph)baseGlyph 
               metricsExist: (BOOL *)flag
{
  return [fontInfo positionOfGlyph: aGlyph 
                   struckOverGlyph: baseGlyph 
                      metricsExist: flag];
}

- (NSPoint) positionOfGlyph: (NSGlyph)aGlyph 
             struckOverRect: (NSRect)aRect 
               metricsExist: (BOOL *)flag
{
  return [fontInfo positionOfGlyph: aGlyph 
                    struckOverRect: aRect 
                      metricsExist: flag];
}

- (NSPoint) positionOfGlyph: (NSGlyph)aGlyph 
               withRelation: (NSGlyphRelation)relation 
                toBaseGlyph: (NSGlyph)baseGlyph
           totalAdvancement: (NSSize *)offset 
               metricsExist: (BOOL *)flag
{
  return [fontInfo positionOfGlyph: aGlyph 
                      withRelation: relation 
                       toBaseGlyph: baseGlyph
                  totalAdvancement: offset 
                      metricsExist: flag];
}

- (int) positionsForCompositeSequence: (NSGlyph *)glyphs 
                       numberOfGlyphs: (int)numGlyphs 
                           pointArray: (NSPoint *)points
{
  int i;
  NSGlyph base = glyphs[0];

  points[0] = NSZeroPoint;

  for (i = 1; i < numGlyphs; i++)
    {
      BOOL flag;
      // This only places the glyphs relative to the base glyph 
      // not to each other
      points[i] = [self positionOfGlyph: glyphs[i] 
                        struckOverGlyph: base 
                           metricsExist: &flag];
      if (!flag)
        return i - 1;
    }

  return i;
}

- (void) getAdvancements: (NSSizeArray)advancements
               forGlyphs: (const NSGlyph*)glyphs
                   count: (NSUInteger)count
{
  // FIXME
  int i;

  for (i = 0; i < count; i++)
    {
      advancements[i] = [self advancementForGlyph: glyphs[i]];
    }
}

- (void) getAdvancements: (NSSizeArray)advancements
         forPackedGlyphs: (const void*)glyphs
                   count: (NSUInteger)count
{
  // FIXME
}

- (void) getBoundingRects: (NSRectArray)bounds
                forGlyphs: (const NSGlyph*)glyphs
                    count: (NSUInteger)count
{
  // FIXME
  int i;

  for (i = 0; i < count; i++)
    {
      bounds[i] = [self boundingRectForGlyph: glyphs[i]];
    }
}

- (NSStringEncoding) mostCompatibleStringEncoding
{
  return [fontInfo mostCompatibleStringEncoding];
}

//
// NSCoding protocol
//
- (Class) classForCoder
{
  return NSFontClass;
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      [aCoder encodeObject: fontName forKey: @"NSName"];
      [aCoder encodeFloat: [self pointSize] forKey: @"NSSize"];
      
      switch (role >> 1)
        {
          // FIXME: Many cases still missing
          case RoleControlContentFont:
            [aCoder encodeInt: 16 forKey: @"NSfFlags"];
            break;
          case RoleLabelFont:
            [aCoder encodeInt: 20 forKey: @"NSfFlags"];
            break;
          case RoleTitleBarFont:
            [aCoder encodeInt: 22 forKey: @"NSfFlags"];
            break;
          default:
            break;
        }
    }
  else 
    {
      [aCoder encodeValueOfObjCType: @encode(int) at: &role];

      if (role == 0)
        {
          float fontMatrix[6];
          BOOL fix = NO;

          fontMatrix[0] = matrix[0];
          fontMatrix[1] = matrix[1];
          fontMatrix[2] = matrix[2];
          fontMatrix[3] = matrix[3];
          fontMatrix[4] = matrix[4];
          fontMatrix[5] = matrix[5];
          [aCoder encodeObject: fontName];
          [aCoder encodeArrayOfObjCType: @encode(float)  count: 6  at: fontMatrix];
          [aCoder encodeValueOfObjCType: @encode(BOOL) at: &fix];
        }
      else if (role & 1)
        {
          float size = matrix[0];
          [aCoder encodeValueOfObjCType: @encode(float) at: &size];
        }
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  if ([aDecoder allowsKeyedCoding])
    {
      NSString *name = [aDecoder decodeObjectForKey: @"NSName"];
      float size = [aDecoder decodeFloatForKey: @"NSSize"];
      
      DESTROY(self);
      if ([aDecoder containsValueForKey: @"IBIsSystemFont"])
     	{
     	  self = RETAIN([NSFont systemFontOfSize: size]);
     	}
      else
        {
     	  self = RETAIN([NSFont fontWithName: name size: size]);
     	}
      if (self == nil)
        {
	  if ([aDecoder containsValueForKey: @"NSfFlags"])
	    {
	      int flags = [aDecoder decodeIntForKey: @"NSfFlags"];
	      // FIXME
	      if (flags == 16)
		{
		  return RETAIN([NSFont controlContentFontOfSize: size]);
		}
	      else if (flags == 20)
		{
		  return RETAIN([NSFont labelFontOfSize: size]);
		}
	      else if (flags == 22)
		{
		  return RETAIN([NSFont titleBarFontOfSize: size]);
		}
	    }
          self = RETAIN([NSFont systemFontOfSize: size]);
        }

      return self;
    }
  else
    {
      int version = [aDecoder versionForClassName: @"NSFont"];
      id name;
      float fontMatrix[6];
      CGFloat cgMatrix[6];
      int the_role;
      
      if (version == 3)
        {
          [aDecoder decodeValueOfObjCType: @encode(int)
                                       at: &the_role];
        }
      else
        {
          the_role = RoleExplicit;
        }
      
      if (the_role == RoleExplicit)
        {
          /* The easy case: an explicit font, or a font encoded with
             version <= 2. */
          name = [aDecoder decodeObject];
          [aDecoder decodeArrayOfObjCType: @encode(float)
                                    count: 6
                                       at: fontMatrix];
          
          if (version >= 2)
            {
              BOOL fix;
              [aDecoder decodeValueOfObjCType: @encode(BOOL)
                                           at: &fix];
            }

          cgMatrix[0] = fontMatrix[0];
          cgMatrix[1] = fontMatrix[1];
          cgMatrix[2] = fontMatrix[2];
          cgMatrix[3] = fontMatrix[3];
          cgMatrix[4] = fontMatrix[4];
          cgMatrix[5] = fontMatrix[5];
          self = [self initWithName: name
                             matrix: cgMatrix
                         screenFont: NO
                               role: RoleExplicit];
          if (self)
            return self;

          self = [NSFont userFontOfSize: fontMatrix[0]];
          NSAssert(self != nil, @"Couldn't find a valid font when decoding.");
          return RETAIN(self);
        }
      else
        {
          /* A non-explicit font. */
          float size;
          NSFont *new;
          
          if (the_role & 1)
            {
              [aDecoder decodeValueOfObjCType: @encode(float)
                                               at: &size];
            }
          else
            {
              size = 0.0;
            }
          
          switch (the_role >> 1)
            {
              case RoleBoldSystemFont:
                new = [NSFont boldSystemFontOfSize: size];
                break;
              case RoleSystemFont:
                new = [NSFont systemFontOfSize: size];
                break;
              case RoleUserFixedPitchFont:
                new = [NSFont userFixedPitchFontOfSize: size];
                break;
              case RoleTitleBarFont:
                new = [NSFont titleBarFontOfSize: size];
                break;
              case RoleMenuFont:
                new = [NSFont menuFontOfSize: size];
                break;
              case RoleMessageFont:
                new = [NSFont messageFontOfSize: size];
                break;
              case RolePaletteFont:
                new = [NSFont paletteFontOfSize: size];
                break;
              case RoleToolTipsFont:
                new = [NSFont toolTipsFontOfSize: size];
                break;
              case RoleControlContentFont:
                new = [NSFont controlContentFontOfSize: size];
                break;
              case RoleLabelFont:
                new = [NSFont labelFontOfSize: size];
                break;
              case RoleMenuBarFont:
                new = [NSFont menuBarFontOfSize: size];
                break;

              default:
                NSDebugLLog(@"NSFont", @"unknown role %i", the_role);
                /* fall through */
              case RoleUserFont:
                new = [NSFont userFontOfSize: size];
                break;
            }

          RELEASE(self);
          if (new)
            return RETAIN(new);

          new = [NSFont userFontOfSize: size];
          NSAssert(new != nil, @"Couldn't find a valid font when decoding.");
          return RETAIN(new);
        }
    }
}

@end /* NSFont */

@implementation NSFont (GNUstep)
//
// Private method for NSFontManager and backend
//
- (GSFontInfo*) fontInfo
{
  return fontInfo;
}

- (void *) fontRef
{
  if (_fontRef == nil)
    _fontRef = fontInfo;
  return _fontRef;
}

// This is a private but popular Cocoa method.
- (NSGlyph) _defaultGlyphForChar: (unichar)theChar
{
  return [fontInfo glyphForCharacter: theChar];
}

@end


int NSConvertGlyphsToPackedGlyphs(NSGlyph *glBuf, 
                                  int count, 
                                  NSMultibyteGlyphPacking packing, 
                                  char *packedGlyphs)
{
  int i;
  int j;

  j = 0;
  // Store the number of glyphs in the first byte.
  packedGlyphs[j++] = count;
  for (i = 0; i < count; i++)
    {
      NSGlyph g = glBuf[i];

      switch (packing)
        {
            case NSOneByteGlyphPacking: 
                packedGlyphs[j++] = (char)(g & 0xFF); 
                break;
            case NSTwoByteGlyphPacking: 
                packedGlyphs[j++] = (char)((g & 0xFF00) >> 8) ; 
                packedGlyphs[j++] = (char)(g & 0xFF); 
                break;
            case NSFourByteGlyphPacking:
                packedGlyphs[j++] = (char)((g & 0xFF000000) >> 24) ; 
                packedGlyphs[j++] = (char)((g & 0xFF0000) >> 16); 
                packedGlyphs[j++] = (char)((g & 0xFF00) >> 8) ; 
                packedGlyphs[j++] = (char)(g & 0xFF); 
                break;          
            case NSJapaneseEUCGlyphPacking:
            case NSAsciiWithDoubleByteEUCGlyphPacking:
            default:
                // FIXME
                break;
        }
    } 

  return j;
}
