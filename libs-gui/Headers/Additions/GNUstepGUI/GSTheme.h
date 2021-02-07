/** <title>GSTheme</title>

   <abstract>Useful/configurable drawing functions</abstract>

   Copyright (C) 2004-2006 Free Software Foundation, Inc.

   Author: Adam Fedor <fedor@gnu.org>
   Author: Richard Frith-Macdonald <rfm@gnu.org>
   Date: Jan 2004
   
   This file is part of the GNU Objective C User interface library.

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

  <chapter>
    <heading>The theme management system</heading>
    <p>
      The theme management system for the GNUstep GUI is based around the
      [GSTheme] class, which provides support for loading of theme bundles
      and methods for drawing common user interface elements.<br />
      The theme system works in conjunction with a variety of other GUI
      classes and is intended to eventually allow for very major changes
      in GUI appearance and behavior.
    </p>
    <p>
      Various design imperatives apply to the theme system, but probably
      the key ones are:
    </p>
    <list>
      <item>It should allow designers and other non-technical users to
        easily develop new and interesting GUI styles likely to attract
	new users to GNUstep.
      </item>
      <item>Using and switching between themes should be an easy and
        pleasant experience ... so that people are not put off when they
        try using themes.
      </item>
      <item>It should eventually permit a GNUstep application to
        appear as a native application on ms-windows and other systems.
      </item>
    </list>
    <p>
      To attain these aims implies the recognition of some more specific
      objectives and some possible technical solutions:
    </p>
    <list>
      <item>We must have as simple as possible an API for the
        functions handling the way GUI elements work and the way they
	draw themselves.<br />
	The standard OpenStep/MacOS-X API provides mechanisms for
	controlling the colors used to draw controls (via [NSColor] and
	[NSColorList]) and controlling the way controls behave
	(NSInterfaceStyleForKey() and [NSResponder-interfaceStyle]),
	but we need to extend that with methods to draw controls entirely
	differently if required.
      </item>
      <item>We must have a GUI application for theme development.
        It is not sufficient to provide an API if we want good graphic
	designers and user interface specialists to develop themes
	for us.
      </item>
      <item>It must be possible for an application to dynamically change
        the theme in use while it is running and it should be easy for a
	user to select between available themes.<br />
	This implies that themes must be loadable bundles and that it
	must always be possible to unload a theme as well as loading one.<br />
	It suggests that the theme selection mechanism should be in every
	application, perhaps as an extension to an existing panel such
	as the info panel.
      </item>
    </list>
    <section>
      <heading>Types of theming</heading>
      <p>
        There are various aspects of theming which can be treated pretty
	much separately, so there is no reason why a theme might not be
	created which just employs one of these mechanisms.
      </p>
      <deflist>
	<term>System images</term>
	<desc>
	  Possibly the simples theme change ... a theme might supply a
	  new set of system images used for arrows and other icons that
	  the GUI decorates controls with.
	</desc>
        <term>System colors</term>
	<desc>
	  A theme might simply define a new system color list, so that
	  controls are drawn in a new color range, though they would
	  still function the same way.  Even specifying new colors can
	  make the GUI look quite different though.<br />
	  Beyond system colors, the theming API also provides a mechanism
	  for specifying colors for particular parts of GUI controls.
	</desc>
	<term>Image tiling</term>
	<desc>
	  Controls might be given sets of images used as tiling to draw
	  themselves rather than using the standard line drawing and
	  color fill mechanisms.
	</desc>
	<term>Interface style</term>
	<desc>
	  A theme might supply a set of interface style keys for various
	  controls, defining how those controls should behave subject to
	  the limitation of the range of behaviors coded into the GUI
	  library.
	</desc>
	<term>Method override</term>
	<desc>
	  A theme might actually provide code, in the form of a subclass
	  of [GSTheme] such that drawing methods have completely custom
	  behavior.
	</desc>
      </deflist>
    </section>

    <section>
      <heading>Subclassing GSTheme</heading>
      <p>
	While many themes can be created without subclassing GSTheme,
	there are some cases where writing code is necessary (most
	notably when interfacing to a native theming engine for some
	platform in order to make a GNUstep app have a native look).<br />
	In these cases the subclass should follow certain rules in order
	to operate cleanly and efficiently:
      </p>
      <deflist>
	<term>Stability</term>
	<desc>
	  Theme operation should remain consistent while it is in use,
	  particularly in the provision of resources such as images and
	  colors.<br />
	  If a theme needs to change a resource (such as an image) then
	  it should do so by calling -deactivate, making the change, and
	  then calling -activate.  This sequence ensures that the GUI
	  library is made aware of any changes and can redraw the screen.<br />
	  The deactivation/activation sequence is expensive, so the
	  subclass should attempt to combine multiple resource updates
	  into a group rather than performing the deactivation/activation
	  for each resource individually.
	</desc>
        <term>Activation</term>
	<desc>
	  The standard -activate method replaces existing system images,
	  colors, interface style settings and other user defaults settings
	  with versions stored in the theme bundle.<br />
	  If a subclass wishes to dynamically provide these resources rather
	  than supplying them as static information in the bundle, it may
	  update the in-memory information after the normal operation has
	  taken place.  This should be done by the theme registering itsself
	  as an observer of GSThemeWillActivateNotification and adding the
	  resources just before the theme becomes active.<br />
	  Cleanup may be done in response to a GSThemeWillDeactivateNotification
	  (called before the default cleanup) or a
	  GSThemeDidDeactivateNotification (called after the default cleanup).
	</desc>
        <term>Versioning</term>
	<desc>
	  With a theme which contains only static resources, versioning is
	  not much of an issue, but with a code-based theme (ie where you
	  subclass GSTheme) versioning does become very important.  This is
	  because, while you can load code from a bundle, you can't unload
	  it again ... so if you have two versions of a theme where the
	  subclass has the same name, then you have a conflict and can't
	  load both and swap between the two.<br />
	  Thematic.app solves this problem my incorporating a version number
	  into the name of the GSTheme subclasses it creates, but that's
	  not the only consideration...<br />
	  You must also ensure that either you do not define any other
	  classes in your theme or that, if you do define them you make
	  sure that each of them incorporates the theme version number.<br />
	  A similar consideration would apply to any categories, however
	  category conflicts are far more difficult to resolve since
	  even with different version names of the categories, the
	  categories all effect the same class/methods.  In fact this
	  issue is so tricky to deal with that you should simply not
	  use categories within your theme code.<br />
	  To work around this limitation, the GSTheme class supports
	  overriding of the methods of any other class while the theme
	  is active.  See the -overriddenMethod:for: method for more
	  information.
	</desc>
	<term>Image override</term>
	<desc>
	  System images (those returned by the [NSImage+imageNamed:] method)
	  are handled by the default theming mechanism, for each system image
	  supplied in the theme bundle, the image is loaded from the bundle
	  and used while the theme is active.  Any pre-existing in-memory image
	  is saved on theme activation and restored on theme deactivation.<br />
	  A theme subclass may override the -imageClass method to change the
	  class used to load each image from the bundle ... thus allowing
	  customisation of not just the images but also of the image
	  behavior in the (very rare) cases where this is desirable.<br />
          Finally, a theme may provide application specific images which are
          loaded <em>in preference to</em> named images from the application's
          own bundle.  These images are simply stored in a subdirectory whose
          name is the same as the application's bundleIdentifier.
	</desc>
      </deflist>
    </section>
  </chapter>

 */

#ifndef _GNUstep_H_GSTheme
#define _GNUstep_H_GSTheme

#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>
#import <AppKit/NSBox.h>
#import <AppKit/NSCell.h>
// For gradient types
#import <AppKit/NSButtonCell.h>
// For image frame style
#import <AppKit/NSImageCell.h>
// For scroller constants
#import <AppKit/NSScroller.h>
// For segmented control style constants
#import <AppKit/NSSegmentedControl.h>
// For tab view type 
#import <AppKit/NSTabView.h>
#import <AppKit/NSPrintPanel.h>
#import <AppKit/NSPageLayout.h>

#if	OS_API_VERSION(GS_API_NONE,GS_API_NONE)
@class NSArray;
@class NSBundle;
@class NSBrowserCell;
@class NSDictionary;
@class NSButton;
@class NSColor;
@class NSColorList;
@class NSColorWell;
@class NSImage;
@class NSMenuItemCell;
@class NSPopUpButtonCell;
@class NSMenuView;
@class NSProgressIndicator;
@class NSTableHeaderCell;
@class NSTabViewItem;
@class NSPathControl;
@class NSPathComponentCell;
@class GSDrawTiles;
@class GSTitleView;

APPKIT_EXPORT	NSString	*GSSwitch;
APPKIT_EXPORT   NSString        *GSRadio;

/* First, declare names used for obtaining colors and/or tiles for specific
 * controls and parts of controls.
 */

/* Names for the component parts of a scroller,
 * allowing tiles and/or colors to be set for them.
 */
APPKIT_EXPORT	NSString	*GSScrollerDownArrow;
APPKIT_EXPORT	NSString	*GSScrollerHorizontalKnob;
APPKIT_EXPORT	NSString	*GSScrollerHorizontalSlot;
APPKIT_EXPORT	NSString	*GSScrollerLeftArrow;
APPKIT_EXPORT	NSString	*GSScrollerRightArrow;
APPKIT_EXPORT	NSString	*GSScrollerUpArrow;
APPKIT_EXPORT	NSString	*GSScrollerVerticalKnob;
APPKIT_EXPORT	NSString	*GSScrollerVerticalSlot;

/* Names for table view parts */
APPKIT_EXPORT	NSString	*GSTableHeader;
APPKIT_EXPORT	NSString	*GSTableCorner;

/*
 * Browser part names.
 */
APPKIT_EXPORT  NSString        *GSBrowserHeader;

/*
 * Menu part names.
 */
APPKIT_EXPORT  NSString        *GSMenuHorizontalBackground;
APPKIT_EXPORT  NSString        *GSMenuVerticalBackground;
APPKIT_EXPORT  NSString        *GSMenuTitleBackground;
APPKIT_EXPORT  NSString        *GSMenuHorizontalItem;
APPKIT_EXPORT  NSString        *GSMenuVerticalItem;
APPKIT_EXPORT  NSString        *GSMenuSeparatorItem;

/* NSPopUpButton parts */

APPKIT_EXPORT  NSString        *GSPopUpButton;

/*
 * Progress Indicator part names.
 */
APPKIT_EXPORT  NSString        *GSProgressIndicatorBezel;
APPKIT_EXPORT  NSString        *GSProgressIndicatorBarDeterminate;

/*
 * Color well part names.
 */
APPKIT_EXPORT  NSString        *GSColorWell;
APPKIT_EXPORT  NSString        *GSColorWellInnerBorder;

/* NSSliderCell parts */
APPKIT_EXPORT  NSString        *GSSliderHorizontalTrack;
APPKIT_EXPORT  NSString        *GSSliderVerticalTrack;

/* NSBox parts */
APPKIT_EXPORT  NSString        *GSBoxBorder;

/* NSTabView parts */
APPKIT_EXPORT  NSString        *GSTabViewSelectedTabFill;
APPKIT_EXPORT  NSString        *GSTabViewUnSelectedTabFill;
APPKIT_EXPORT  NSString        *GSTabViewBackgroundTabFill;
APPKIT_EXPORT  NSString        *GSTabViewBottomSelectedTabFill;
APPKIT_EXPORT  NSString        *GSTabViewBottomUnSelectedTabFill;
APPKIT_EXPORT  NSString        *GSTabViewBottomBackgroundTabFill;
APPKIT_EXPORT  NSString        *GSTabViewLeftSelectedTabFill;
APPKIT_EXPORT  NSString        *GSTabViewLeftUnSelectedTabFill;
APPKIT_EXPORT  NSString        *GSTabViewLeftBackgroundTabFill;
APPKIT_EXPORT  NSString        *GSTabViewRightSelectedTabFill;
APPKIT_EXPORT  NSString        *GSTabViewRightUnSelectedTabFill;
APPKIT_EXPORT  NSString        *GSTabViewRightBackgroundTabFill;

/**
 * Structure to describe the size of top/bottom/left/right margins inside
 * a button
 */
typedef struct GSThemeMargins
{
  CGFloat left;
  CGFloat right;
  CGFloat top;
  CGFloat bottom;
} GSThemeMargins;

/**
 * This defines how the values in a tile array should be used when
 * drawing a rectangle.  Mostly this just effects the center, middle
 * image of the rectangle.<br />
 * FillStyleMatrix is provided for the use of theme editors wishing
 * to display the tile.
 */
typedef enum {
  GSThemeFillStyleNone = 0,	/** CM image is not drawn */
  GSThemeFillStyleScale = 1,	/** CM image is scaled to fit */
  GSThemeFillStyleRepeat = 2,	/** CM image is tiled from bottom left */
  GSThemeFillStyleCenter = 3,	/** CM image is tiled from the center */
  GSThemeFillStyleMatrix = 4,	/** a matrix of nine separated images */
  GSThemeFillStyleScaleAll = 5      /** All 'stretchable' images (i.e. not
                                    the four corners) are scaled to fill
                                    their area, instead of being repeated */
} GSThemeFillStyle;

/** Function to convert a fill style enumeration value to a string.<br />
 * Returns nil on failure.
 */
extern NSString *
GSThemeStringFromFillStyle(GSThemeFillStyle s);

/** Function to convert a string to a fill style enumeration value.<br />
 * Returns GSThemeFillStyleNone if the string is not a valid name.
 */
extern GSThemeFillStyle
GSThemeFillStyleFromString(NSString *s);

/**
 * This enumeration provides constants for informing drawing methods
 * what state a control is in (and consequently how the display element
 * being drawn should be presented).
 * NB. GSThemeNormalState must be 0 and GSThemeSelectedState must be the
 * last state, in order to allow code to iterate through all the states.
 */
typedef enum {
  GSThemeNormalState = 0,	/** A control in its normal state */
  GSThemeFirstResponderState,
  GSThemeDisabledState,		/** A control which is disabled */
  GSThemeHighlightedFirstResponderState,
  GSThemeHighlightedState,	/** A control which is highlighted */
  GSThemeSelectedFirstResponderState,
  GSThemeSelectedState		/** A control which is selected */
} GSThemeControlState;

/** Notification sent when a theme has just become active.<br />
 * The notification is posted by the -activate method.<br />
 * This is primarily for internal use by AppKit controls which
 * need to readjust how they are displayed when a new theme is in use.<br />
 * Theme subclasses must ensure that the theme is ready for use by the
 * time this notification is posted (which generally means that
 * they should have finished putting all resources in place in
 * response to a GSThemeWillActivateNotification).
 */
APPKIT_EXPORT	NSString	*GSThemeDidActivateNotification;

/** Notification sent when a theme has just become inactive.<br />
 * The notification is posted by the -deactivate method.<br />
 * This is primarily for use by subclasses of GSTheme which need to perform
 * additional cleanup after the theme stops being used.
 */
APPKIT_EXPORT	NSString	*GSThemeDidDeactivateNotification;

/** Notification sent when a theme is about to become active.<br />
 * The notification is posted by the -activate method.<br />
 * This is for use by subclasses of GSTheme which need to perform
 * additional setup before the theme starts being used by AppKit controls.<br />
 * At the point when this notification is called, the color, image, and
 * defaults information from the theme will have been installed and the
 * theme subclass may perform final adjustments.
 */
APPKIT_EXPORT	NSString	*GSThemeWillActivateNotification;

/** Notification sent when a theme is about to become inactive.<br />
 * The notification is posted by the -deactivate method.<br />
 * This allows code to make preparatory changes before the current theme
 * is deactivated, but subclasses should not make the theme unusable
 * at this point as the AppKit may be doing its own cleanup in response
 * to the notification.
 */
APPKIT_EXPORT	NSString	*GSThemeWillDeactivateNotification;


/**
  <p><em>This interface is <strong>HIGHLY</strong> unstable
  and incomplete at present.</em>
  </p>
  <p>
  This is a class used for 'theming', which is mostly a matter of
  encapsulating common drawing behaviors so that GUI appearance can
  be easily modified, but also includes mechanisms for altering
  some GUI behavior (such as orientation and position of menus).
  </p>
  <p>
  Methods in this class standardize drawing of buttons, borders
  and other common GUI elements, so that all other classes within
  the GUI will provide a consistent appearance by using these
  methods.
  </p>
  <p>
  The default implementation uses the standard configurable colors
  defined in NSColor, such as <code>controlLightHighlightColor</code>,
  <code>controlShadowColor</code> and <code>controlDarkShadowColor</code>.<br />
  Themes are expected to override the default system color list with their
  own versions, and this class cooperates with [NSColor] and [NSColorList]
  to establish the correct system color list when a theme is activated.
  </p>
  <p>
  The class provides a mechanism for automatic loading of theme bundles
  consisting of resources used to define how drawing is done, plus an
  optional binary subclass of this class (to replace/extend the drawing
  methods this class provides).
  </p>
  <p>
  In future this class should provide mechanisms to draw controls by
  tiling of images, and provide control over GUI behavior by controlling
  the values returned by NSInterfaceStyleForKey() so that controls
  use the appropriate behavior.
  </p>
*/ 
@interface GSTheme : NSObject
{
@private
  void		*_reserved;
}

/**
 * Loads a theme from a theme bundle of the specified name, which may be
 * either a full path name of the theme bundle, or a simple theme name
 * (in which case the standard directories are searched for it) or nil
 * (in which case the default GNUstep theme is returned).<br />
 * Returns the loaded theme but does not make it the current theme,
 * to do that you will need to call the +setTheme: method.
 */
+ (GSTheme*) loadThemeNamed: (NSString*)themeName;

/**
 * Creates and displays a panel allowing selection of different themes
 * and display of the current theme inspector.
 */
+ (void) orderFrontSharedThemePanel: (id)sender;

/**
 * Set the currently active theme to be the instance specified.<br />
 * You do not normally need to call this method as it is called
 * automatically when the user default which specifies the current
 * theme (GSTheme) is updated.
 */
+ (void) setTheme: (GSTheme*)theme;

/**
 * Returns the currently active theme instance.  This is the value most
 * recently set using +setTheme: or (if none has been set) is a default
 * instance of the base class.
 */
+ (GSTheme*) theme;

/**
 * <p>This method is called automatically when the receiver is made into
 * the currently active theme by the +setTheme: method. Subclasses may
 * override it to perform startup operations, however, the method is not
 * really intended to be overridden, and subclasses should generally
 * handle activation work in response to the GSThemeWillActivatenotification
 * posted by this method.
 * </p>
 * <p>The base implementation handles setup and caching of the system
 * color list, standard image information, tiling information,
 * and user defaults.<br />
 * It then sends a GSThemeWillActivateNotification to allow subclasses to
 * perform further activation work, and a GSThemeDidActivateNotification
 * to allow other parts of the GUI library to update themselves from the
 * new theme.
 * </p>
 * <p>Finally, this method marks all windows in the application as needing
 * update ... so they will draw themselves with the new theme information.
 * </p>
 * <p>NB. If a GSTheme subclass is integrating to an external native theming
 * mechanism in order to make GNUstep apps look like native apps, then the
 * external theme may change dynamically and the GSTheme subclass may need
 * to change the GNUstep application to reflect this change.  When this
 * happens, the update should be handled by the subclass calling -deactivate
 * and then -activate to make the changes 'live'.
 * </p>
 */
- (void) activate;

/**
 * Returns the names of the theme's authors.
 */
- (NSArray*) authors;

/**
 * Return the bundle containing the resources used by the current theme.
 */
- (NSBundle*) bundle;

/**
 * Returns the class used by the theme for loading color lists.  The default
 * implementation returns the NSColorList class, but a subclass may override
 * this to return a color list class whose values change dynamically in
 * response to changes of a native theming API for instance.<br />
 * The class returned by this method should be NSColorList or one of its
 * subclasses.  Subclasses should note that GSTheme will initialise the
 * instances of the class using the [NSColerList-initWithName:fromFile:]
 * method.
 */
- (Class) colorClass;

/** Removes the name from the color cache forcing it to be re-created next
 * time the named color is required.<br />
 * Passing nil for aName removes all named colors.<br />
 * Passing a negative value for elementState applies to all caches.
 */
- (void) colorFlush: (NSString*)aName
	      state: (GSThemeControlState)elementState;

/**
 * This returns the color for drawing the item whose name is aName in
 * the specified state.  If aName is nil or if there is no color defined
 * for the particular combination of item name and state, the method
 * returns nil.<br />
 * The standard names used for the parts of various controls are declared
 * in GSTheme.h<br />
 * See also the -tilesNamed:state: method.
 */
- (NSColor*) colorNamed: (NSString*)aName
		  state: (GSThemeControlState)elementState;

/**
 * Returns the system color list defined by the receiver.<br />
 * The default implementation returns the color list provided in the
 * theme bundle (if any) or the default system color list.
 */
- (NSColorList*) colors;

/**
 * <p>This method is called automatically when the receiver is stopped from
 * being the currently active theme by the use of the +setTheme: method
 * to make another theme active. Subclasses may override it to perform
 * shutdown operations, but it is preferred for subclasses to perform
 * their own deactivation in response to a GSThemeWillDeactivateNotification.
 * </p>
 * <p>The base implementation sends a GSThemeWillDeactivateNotification to
 * allow subclasses to perform cleanup, then restores image, color and default
 * information to the state before the theme was activates, and finally
 * sends a GSThemeDidDeactivateNotification to allow other parts of the
 * GUI library to update themselves.
 * </p>
 * <p>NB. If a GSTheme subclass is integrating to an external native theming
 * mechanism in order to make GNUstep apps look like native apps, then the
 * external theme may change dynamically and the GSTheme subclass may need
 * to change the GNUstep application to reflect this change.  When this
 * happens, the update should be handled by the subclass calling -deactivate
 * and then -activate to make the changes 'live'.
 * </p>
 */
- (void) deactivate;

/**
 * Returns the theme's icon.
 */
- (NSImage*) icon;

/**
 * Returns the class used by the theme for loading images. The default
 * implementation returns the NSImage class, but a subclass may override
 * this to return an image class whose instances dynamically alter what
 * they draw in response to changes of a native theming API for instance.<br />
 * This method must return the NSImage class or one of its subclasses.
 * Subclass implementations should note that instances will be initialised
 * using the [NSImage-initWithContentsOfFile:] method and will use the
 * [NSImage-imageFileTypes] method to determine which image files can be
 * loaded.
 */
- (Class) imageClass;

/** <init />
 * Initialise an instance of a theme with the specified resource bundle.<br />
 * You don't need to call this method directly, but if you are subclassing
 * you may need to override this to provide additional initialisation.
 */
- (id) initWithBundle: (NSBundle*)bundle;

/**
 * <p>Returns the info dictionary for this theme.  In the base class
 * implementation this is simply the info dictionary of the theme
 * bundle, but subclasses may override this method to return extra
 * or different information.
 * </p>
 * <p>Keys found in this dictionary include:
 * </p>
 * <deflist>
 *   <term>GSThemeDomain</term>
 *   <desc>A dictionary whose key/value pairs are used to set up new values
 *   in the GSThemeDomain domain of the user defaults system, and hence
 *   define values for these unless overridden by values set explicitly by
 *   the user.
 *   </desc>
 *   <term>GSThemeTiles</term>
 *   <desc>A dictionary keyed on tile names and containing the following:
 *     <deflist>
 *       <term>FileName</term>
 *       <desc>Name of the file (within the GSThemeTiles directory in the
 *       bundle) in which the image for this tile is stored.
 *       </desc>
 *       <term>HorizontalDivision</term>
 *       <desc>The offset along the X-axis used to divide the image into
 *       columns of tiles.
 *       </desc>
 *       <term>VerticalDivision</term>
 *       <desc>The offer along the Y-axis used to divide the image into
 *       rows of tiles.
 *       </desc>
 *     </deflist>
 *   </desc>
 * </deflist>
 */
- (NSDictionary*) infoDictionary;

/**
 * Return the theme's name.
 */
- (NSString*) name;

/** Returns the name used to locate theming resources for a particular GUI
 * element.  If no name has been set for the particular object this method
 * returns nil.
 */
- (NSString*) nameForElement: (id)anObject;

/** <p>Returns the original implementation of a method overridden by this
 * theme, or zero if the method was not overridden.
 * </p>
 * <p>A theme may override a method of another class by implementing a method
 * whose name is '_overrideXXXMethod_YYY' where 'XXX' is the name of the
 * class whose method is to be overridden, and 'YYY' is the normal name of
 * the method in that class.<br />
 * eg. _overrideNSScrollerMethod_drawRect:
 * </p>
 * <p>NB. The overriding method may not access instance variable directly and
 * must cast all uses of 'self' to be the correct class.
 * </p>
 */
- (IMP) overriddenMethod: (SEL)selector for: (id)receiver;

/** Set the name of this theme ... used for testing by Thematic.app
 */
- (void) setName: (NSString*)aString;

/** Set the name that is used to identify theming resources for a particular
 * control or other GUI element.  This is used so that where an element is
 * part of a control it can be displayed differently from the same class of
 * element used outside that control.<br />
 * Supplying a nil value for aString simply removes any name setting for
 * anObject.<br />
 * Supplying nil for anObject is illegal (raises an exception) unless
 * the value of aString is also nil (and the method does nothing).<br />
 * Any control which uses this method to set names for subsidiary elements
 * must also make sure to remove the name mapping before that element is
 * deallocated, unless the takeOwnership option is YES, in which case
 * anObject is retained, the name mapping lasts only until the receiver
 * is deactivated, and at that point anObject is released.
 */
- (void) setName: (NSString*)aString
      forElement: (id)anObject
       temporary: (BOOL)takeOwnership;
	

/**
 * <p>Provides a standard inspector window used to display information about
 * the receiver.  The default implementation displays the icon, the name,
 * and the authors of the theme.
 * </p>
 * <p>The code managing this object (if any) must be prepared to have the
 * content view of the window re-parented into another window for display
 * on screen.
 * </p>
 */
- (NSWindow*) themeInspector;

/** Removes the name tile images from cache, forcing re-creation next
 * time the named tiles are required.<br />
 * Passing nil for aName removes all named tiles.<br />
 * Passing a negative value for elementState applies to all caches.
 */
- (void) tilesFlush: (NSString*)aName
	      state: (GSThemeControlState)elementState;

/**
 * Returns the tile image information for a particular image name,
 * or nil if there is no such information or the name is nil.<br />
 * The standard names used for the parts of various controls are declared
 * in GSTheme.h<br />
 * The GUI library uses this internally to handling tiling of image
 * information to draw user interface elements.  The tile information
 * returned by this method can be passed to the
 * -fillRect:withTiles:background:fillStyle: method.<br />
 * The elementState argument specifies the state for which tiles are
 * requested.
 * See the -colorNamed:state: method for determining colors to be
 * used for drawing specific GUI elements.
 */
- (GSDrawTiles*) tilesNamed: (NSString*)aName
		      state: (GSThemeControlState)elementState;

/**
 * Return the theme's version string.
 */
- (NSString*) versionString;

/**
 * Return the theme's license.
 */
- (NSString*) license;

@end

/**
 * Theme drawing methods.<br />
 * Methods which return information/resources are generally expected
 * (ie unless explicitly documented otherwise) to be returning something
 * which persists until the method is called again or until the current
 * theme is deactivated (whichever comes first).<br />
 * This means that drawing code should <strong>not</strong> need to
 * retain/release any returned object (the theme is responsible for
 * retaining the object), and should also be able to cache size information
 * etc for later drawing.<br />
 * This simple rule means that drawing code can be written to be as
 * efficient as possible while keeping caching strategies simple and
 * uniform.<br />
 * To facilitate this within the theme code itsself, it is recommended
 * that you make use of the -setName:forElement:temporary: method to
 * retain any vended object until deactivation.
 */
@interface	GSTheme (Drawing)

/**
 * Allows the theme to set an image or set attributes for drawing the
 * button differently based on the key equivalent which is set.
 */
- (void) setKeyEquivalent: (NSString *)key 
            forButtonCell: (NSButtonCell *)cell;

/**
 * Draws a button frame and background (not its content) for the specified
 * cell and view.
 */
- (void) drawButton: (NSRect)frame
	         in: (NSCell*)cell
	       view: (NSView*)view
	      style: (int)style
	      state: (GSThemeControlState)state;

/**
 * Amount by which the button is inset by the border.
 */
- (GSThemeMargins) buttonMarginsForCell: (NSCell*)cell
				  style: (int)style 
				  state: (GSThemeControlState)state;

/** 
 * Draws the indicator (normally a dotted rectangle) to show that
 * the view currently has keyboard focus.
 */
- (void) drawFocusFrame: (NSRect)frame view: (NSView*)view;

/**
 * Draws the background of a window ... normally a simple fill with the
 * the window's background color.
 */
- (void) drawWindowBackground: (NSRect)frame view: (NSView*)view;

/**
 * Draw a border of the specified border type.
 */
- (void) drawBorderType: (NSBorderType)aType 
                  frame: (NSRect)frame 
                   view: (NSView*)view;

/**
 * Determine the size for the specified border type .
 */
- (NSSize) sizeForBorderType: (NSBorderType)aType;

/**
 * Draw a border of the specified frame style.
 */
- (void) drawBorderForImageFrameStyle: (NSImageFrameStyle)frameStyle
                                frame: (NSRect)frame 
                                 view: (NSView*)view;

/**
 * Determine the size for the specified frame style.
 */
- (NSSize) sizeForImageFrameStyle: (NSImageFrameStyle)frameStyle;

/**
 * Return YES if the scroller arrows are at the same end.
 * Return NO to get one scroller arrow at each end of the scroller.
 *
 * The default implementation first checks the default GSScrollerArrowsSameEnd
 * and if that is not set, delegates to the NSInterfaceStyle.
 */
- (BOOL) scrollerArrowsSameEndForScroller: (NSScroller *)aScroller;

/**
 * Returns YES if clicking in the scroller slot should scroll by one page,
 * NO if the scroller should jump to the location clicked.
 *
 * The default implementation first checks the default GSScrollerScrollsByPage
 * and if that is not set, delegates to the NSInterfaceStyle.
 */
- (BOOL) scrollerScrollsByPageForScroller: (NSScroller *)aScroller;

/** 
 * Creates and returns the cell to be used to draw a scroller arrow of the
 * specified type and orientation.<br />
 * The theme instance is responsible for ensuring that the cell continues
 * to exist until the theme is deactivated (the default implementation does
 * this by naming the cell using the -setName:forElement:temporary:
 * method, which also provides a name for the cell color and image).
 */
- (NSButtonCell*) cellForScrollerArrow: (NSScrollerArrow)part
			    horizontal: (BOOL)horizontal;

/** 
 * Creates and returns the cell to be used to draw a scroller knob of the
 * specified orientation.<br />
 * The theme instance is responsible for ensuring that the cell continues
 * to exist until the theme is deactivated (the default implementation does
 * this by naming the cell using the -setName:forElement:temporary:
 * method).
 */
- (NSCell*) cellForScrollerKnob: (BOOL)horizontal;

/** 
 * Creates and returns the cell to be used to draw a scroller slot of the
 * specified orientation.<br />
 * The theme instance is responsible for ensuring that the cell continues
 * to exist until the theme is deactivated (the default implementation does
 * this by naming the cell using the -setName:forElement:temporary:
 * method).
 */
- (NSCell*) cellForScrollerKnobSlot: (BOOL)horizontal;

/** Returns the width which should be allowed for a scroller within the
 * current theme.  Drawing code is entitled to assume that this value will
 * remain constant until the theme is deactivated.
 */
- (float) defaultScrollerWidth;

/**
 * If YES, instructs NSScrollView to leave an empty square space where
 * the horizontal and vertical scrollers meet.
 * 
 * Controlled by user default GSScrollViewUseBottomCorner; default YES.
 */
- (BOOL) scrollViewUseBottomCorner;

/**
 * If YES, instructs NSScrollView to make the scrollers overlap the border.
 * The scroll view border is drawn using the NSScrollView part, which
 * must be provided by the theme if this method returns YES.
 * 
 * Controlled by user default GSScrollViewScrollersOverlapBorders; default NO;
 */
- (BOOL) scrollViewScrollersOverlapBorders;

/** 
 * Method for toolbar theming.
 */
- (NSColor *) toolbarBackgroundColor;
- (NSColor *) toolbarBorderColor;
- (void) drawToolbarRect: (NSRect)aRect
                   frame: (NSRect)viewFrame
              borderMask: (unsigned int)borderMask;
- (BOOL) toolbarIsOpaque;

// Methods to deal with steppers..
/**
 * Draw a stepper cell
 */
- (void) drawStepperCell: (NSCell*)cell
               withFrame: (NSRect)cellFrame
                  inView: (NSView*)controlView
             highlightUp: (BOOL)highlightUp
           highlightDown: (BOOL)highlightDown;

// Stepper cell helper methods
- (NSRect) stepperUpButtonRectWithFrame: (NSRect)frame;
- (NSRect) stepperDownButtonRectWithFrame: (NSRect)frame;
- (void) drawStepperBorder: (NSRect)frame;

/**
 * Draw light colored stepper using the border and clip rects
 */
- (NSRect) drawStepperLightButton: (NSRect)border : (NSRect)clip;
/**
 * Draw normal stepper up button.
 */
- (void) drawStepperUpButton: (NSRect)aRect;
/**
 * Draw highlighted up stepper button.
 */
- (void) drawStepperHighlightUpButton: (NSRect)aRect;
/**
 * Draw down button for stepper
 */
- (void) drawStepperDownButton: (NSRect)aRect;
/**
 * Draw highlighted stepper down button
 */
- (void) drawStepperHighlightDownButton: (NSRect)aRect;

// NSSwitch drawing methods
- (void) drawSwitchKnob: (NSRect)frame
               forState: (NSControlStateValue)value
                enabled: (BOOL)enabled;


- (void) drawSwitchBezel: (NSRect)frame
                forState: (NSControlStateValue)v
                 enabled: (BOOL)enabled;

- (void) drawSwitchInRect: (NSRect)rect
                 forState: (NSControlStateValue)state
                  enabled: (BOOL)enabled;

// NSPathComponentCell

- (void) drawPathComponentCellWithFrame: (NSRect)f
                                 inView: (NSPathControl *)pc
                               withCell: (NSPathComponentCell *)cell
                        isLastComponent: (BOOL)last;

// NSSegmentedControl drawing methods

- (void) drawSegmentedControlSegment: (NSCell *)cell
                           withFrame: (NSRect)cellFrame
                              inView: (NSView *)controlView
                               style: (NSSegmentStyle)style
                               state: (GSThemeControlState)state
                         roundedLeft: (BOOL)roundedLeft
                        roundedRight: (BOOL)roundedRight;

/**
 * <p>Returns the color used to draw a menu view background.</p>
 *
 * <p>By default, looks up the color named <em>menuBackgroundColor</em>, 
 * otherwise returns the window background color.</p>
 *
 * <p>The returned color is used by 
 * -drawBackgroundForMenuView:withFrame:dirtyRect:horizontal:</p>
 * 
 * <p>Can be overridden in subclasses to return a custom color, but generally
 * should not.  Instead themes should provide default colors in the
 * GSThemeDomain (in their Info.plist).</p>
 */
- (NSColor *) menuBackgroundColor;

/**
 * <p>Returns the color used to draw a menu item background.</p>
 *
 * The menu item background is drawn atop the menu background.
 *
 * <p>By default, looks up the color named <em>menuItemBackgroundColor</em>, 
 * otherwise returns the control background color.<br />
 * When selected or highlighted, the background color is provided by 
 * [NSColor+selectedMenuItemColor].</p>
 *
 * <p>The returned value used by 
 * -drawBorderAndBackgroundForMenuItemCell:withFrame:inView:state:isHorizontal: 
 * and [NSMenuItemCell-backgroundColor].</p>
 * 
 * <p>Can be overridden in subclasses to return a custom color, but generally
 * should not.  Instead themes should provide default colors in the
 * GSThemeDomain (in their Info.plist).</p>
 */
- (NSColor *) menuItemBackgroundColor;
- (NSColor *) menuBarBackgroundColor;
- (NSColor *) menuBarBorderColor;

/**
 * <p>Returns the color used to draw a menu view border.</p>
 *
 * <p>By default, looks up the color named <em>menuBorderColor</em>, 
 * otherwise returns the dark gray color.</p>
 *
 * <p>The returned color is used by 
 * -drawBackgroundForMenuView:withFrame:dirtyRect:horizontal:</p>
 * 
 * <p>Can be overridden in subclasses to return a custom color, but generally
 * should not.  Instead themes should provide default colors in the
 * GSThemeDomain (in their Info.plist).</p>
 */
- (NSColor *) menuBorderColor;

/**
 * <p>Returns a color to draw each edge in a menu view border.</p>
 *
 * <p>By default, returns -menuBorderColor for the upper and left edges of a  
 * vertical menu, or for the bottom edge of a horizontal one.</p>
 *
 * <p>The returned edge color is used by 
 * -drawBackgroundForMenuView:withFrame:dirtyRect:horizontal:</p>
 * 
 * <p>Can be overridden in subclasses to return a custom color, but generally
 * should not.  Instead themes should provide default colors in the
 * GSThemeDomain (in their Info.plist).</p>
 */
- (NSColor *) menuBorderColorForEdge: (NSRectEdge)edge 
                        isHorizontal: (BOOL)horizontal;
- (void) drawBackgroundForMenuView: (NSMenuView*)menuView
                         withFrame: (NSRect)bounds
                         dirtyRect: (NSRect)dirtyRect
                        horizontal: (BOOL)horizontal;
/**
 * <p>Returns whether the menu item border should be drawn or not.</p>
 *
 * <p>By default, returns [NSMenuItemCell-isBordered] value. The value is NO 
 * when the menu is horizontal, YES when vertical.</p>
 *
 * <p>The returned value used by 
 * -drawBorderAndBackgroundForMenuItemCell:withFrame:inView:state:isHorizontal:</p>
 * 
 * <p>Can be overridden in subclasses to return a custom color, but generally
 * should not.  Instead themes should provide default colors in the
 * GSThemeDomain (in their Info.plist).</p>
 */
- (BOOL) drawsBorderForMenuItemCell: (NSMenuItemCell *)cell 
                              state: (GSThemeControlState)state
                       isHorizontal: (BOOL)horizontal;
// menu item cell drawing method
- (void) drawBorderAndBackgroundForMenuItemCell: (NSMenuItemCell *)cell
                                      withFrame: (NSRect)cellFrame
                                         inView: (NSView *)controlView
                                          state: (GSThemeControlState)state
                                   isHorizontal: (BOOL)isHorizontal;

/**
 * <p>Draws the menu item title.</p>
 *
 * <p>Can be overridden to customize the text font, size and position.<br />
 * You can use <code>[[cell menuItem] title]</code> to get the title.</p>
 *
 * <p>The title color is mapped to the theme state as described below:</p>
 * <deflist>
 * <term>GSThemeSelectedState</term>
 * <desc>[NSColor+selectedMenuItemTextColor]</desc>
 * <term>GSThemeDisabledState</term>
 * <desc>[NSColor+controlTextColor] or 
 * [NSColor+disabledControlTextColor]</desc>
 * </deflist>
 */
- (void) drawTitleForMenuItemCell: (NSMenuItemCell *)cell
                        withFrame: (NSRect)cellFrame
                           inView: (NSView *)controlView
                            state: (GSThemeControlState)state
                     isHorizontal: (BOOL)isHorizontal;
/**
 * <p>Returns the color used to draw a separator line in a menu.</p>
 *
 * <p>By default, looks up the color named <em>menuSeparatorColor</em>, 
 * otherwise returns nil.</p>
 *
 * <p>The returned color is used by 
 * -drawSeparatorItemForMenuItemCell:withFrame:inView:isHorizontal:</p>
 * 
 * <p>Can be overridden in subclasses to return a custom color, but generally
 * should not.  Instead themes should provide default colors in the
 * GSThemeDomain (in their Info.plist).</p>
 */
- (NSColor *) menuSeparatorColor;

/**
 * <p>Returns the left and right inset used to draw a separator line in a
 * menu.</p>
 *
 * <p>By default, returns 3.0.</p>
 *
 * <p>The returned color is used by 
 * -drawSeparatorItemForMenuItemCell:withFrame:inView:isHorizontal:</p>
 * 
 * <p>Can be overridden in subclasses to return a custom color, but generally
 * should not.  Instead themes should provide default colors in the
 * GSThemeDomain (in their Info.plist).</p>
 */
- (CGFloat) menuSeparatorInset;

/**
 * Amount that submenus overlap their parent menu by, horizontally.
 * (i.e. applies to vertical menus)
 *
 * Controlled by GSMenuSubmenuHorizontalOverlap default
 */
- (CGFloat) menuSubmenuHorizontalOverlap;

/**
 * Amount that submenus overlap the horizontal menu bar by, vertically.
 *
 * Controlled by GSMenuSubmenuVerticalOverlap default
 */
- (CGFloat) menuSubmenuVerticalOverlap;

/**
 * <p>Draws a separator between normal menu items in a menu.</p>
 *
 * <p>Each separator corresponds to a menu item that returns YES to 
 * -isSeparatorItem</p>
 *
 * <p>You can provide an image tile named <em>GSMenuSeparatorItem</em> to 
 * draw the separator.<br />
 * Can be overridden in subclasses to customize the drawing.</p>
 *
 * <p>See also -menuSeparatorColor and -menuSeparatorInset</p>
 */
- (void) drawSeparatorItemForMenuItemCell: (NSMenuItemCell *)cell
                                withFrame: (NSRect)cellFrame
                                   inView: (NSView *)controlView
                             isHorizontal: (BOOL)isHorizontal;
/**
 * Returns the class used to create the title bar in the given menu view.
 *
 * By default, returns GSTitleView.<br />
 * A subclass can be returned to customize the title view look and behavior.
 */
- (Class) titleViewClassForMenuView: (NSMenuView *)aMenuView;

- (NSRect) drawMenuTitleBackground: (GSTitleView *)aTitleView
			withBounds: (NSRect)bounds
			  withClip: (NSRect)clipRect;

- (CGFloat) menuBarHeight;
- (CGFloat) menuItemHeight;
- (CGFloat) menuSeparatorHeight;

// NSColorWell drawing method
- (NSRect) drawColorWellBorder: (NSColorWell*)well
                    withBounds: (NSRect)bounds
                      withClip: (NSRect)clipRect;

// progress indicator drawing methods
- (void) drawProgressIndicator: (NSProgressIndicator*)progress
                    withBounds: (NSRect)bounds
                      withClip: (NSRect)rect
                       atCount: (int)count
                      forValue: (double)val;

- (NSRect) drawProgressIndicatorBezel: (NSRect)bounds withClip: (NSRect) rect;
- (void) drawProgressIndicatorBarDeterminate: (NSRect)bounds;

// Table drawing methods
- (NSColor *) tableHeaderTextColorForState: (GSThemeControlState)state;

- (void) drawTableCornerView: (NSView*)cornerView
                    withClip: (NSRect)aRect;
- (void) drawTableHeaderCell: (NSTableHeaderCell *)cell
                   withFrame: (NSRect)cellFrame
                      inView: (NSView *)controlView
                       state: (GSThemeControlState)state;

- (float) titlebarHeight;

- (float) resizebarHeight;

- (float) titlebarButtonSize;

- (float) titlebarPaddingRight;

- (float) titlebarPaddingTop;

- (float) titlebarPaddingLeft;

- (void) drawWindowBorder: (NSRect)rect
                withFrame: (NSRect)frame 
             forStyleMask: (unsigned int)styleMask
                    state: (int)inputState
                 andTitle: (NSString*)title;

- (NSColor *) browserHeaderTextColor;

- (void) drawBrowserHeaderCell: (NSTableHeaderCell*)cell
		     withFrame: (NSRect)rect
			inView: (NSView*)view;

- (NSRect) browserHeaderDrawingRectForCell: (NSTableHeaderCell*)cell
				 withFrame: (NSRect)rect;

- (NSRect) tabViewContentRectForBounds: (NSRect)aRect
			   tabViewType: (NSTabViewType)type
			       tabView: (NSTabView *)view;

- (void) drawTabViewRect: (NSRect)rect
		  inView: (NSView *)view
	       withItems: (NSArray *)items
	    selectedItem: (NSTabViewItem *)item;

- (void) drawScrollerRect: (NSRect)rect
		   inView: (NSView *)view
      		  hitPart: (NSScrollerPart)hitPart
	     isHorizontal: (BOOL)isHorizontal;

- (void) drawBrowserRect: (NSRect)rect
		  inView: (NSView *)view
	withScrollerRect: (NSRect)scrollerRect
	      columnSize: (NSSize)columnSize;

- (CGFloat) browserColumnSeparation;

- (CGFloat) browserVerticalPadding;

- (BOOL) browserUseBezels;

- (void) drawMenuRect: (NSRect)rect
	       inView: (NSView *)view
	 isHorizontal: (BOOL)horizontal
	    itemCells: (NSArray *)itemCells;

- (void) drawScrollViewRect: (NSRect)rect
	             inView: (NSView *)view;

- (void) drawSliderBorderAndBackground: (NSBorderType)aType
				 frame: (NSRect)cellFrame
				inCell: (NSCell *)cell
			  isHorizontal: (BOOL)horizontal;

- (void) drawBarInside: (NSRect)rect
		inCell: (NSCell *)cell
	       flipped: (BOOL)flipped;

- (void) drawKnobInCell: (NSCell *)cell;

- (NSRect) tableHeaderCellDrawingRectForBounds: (NSRect)theRect;
							
- (void) drawTableHeaderRect: (NSRect)aRect		
		      inView: (NSView *)view;

- (void) drawPopUpButtonCellInteriorWithFrame: (NSRect)cellFrame
				     withCell: (NSCell *)cell
				       inView: (NSView *)controlView;

- (void) drawTableViewBackgroundInClipRect: (NSRect)clipRect
				    inView: (NSView *)view
		       withBackgroundColor: (NSColor *)backgroundColor;

- (void) drawTableViewRect: (NSRect)aRect
		    inView: (NSView *)view;

- (void) drawTableViewGridInClipRect: (NSRect)aRect
		      	      inView: (NSView *)view;

- (void) highlightTableViewSelectionInClipRect: (NSRect)clipRect
					inView: (NSView *)view
			      selectingColumns: (BOOL)selectingColumns;

- (void) drawTableViewRow: (NSInteger)rowIndex 
		 clipRect: (NSRect)clipRect
		   inView: (NSView *)view;

- (void) drawBoxInClipRect: (NSRect)clipRect
		   boxType: (NSBoxType)boxType
		borderType: (NSBorderType)borderType
		    inView: (NSBox *)box;
@end

/**
 * Helper functions for drawing standard items.
 */
@interface	GSTheme (MidLevelDrawing)
/** Draw a standard button */
- (NSRect) drawButton: (NSRect)border withClip: (NSRect)clip;

/** Draw a dark bezel border */ 
- (NSRect) drawDarkBezel: (NSRect)border withClip: (NSRect)clip;

/** Draw a "dark" button border (used in tableviews) */
- (NSRect) drawDarkButton: (NSRect)border withClip: (NSRect)clip;

/** Draw a frame photo border.  Used in NSImageView.   */
- (NSRect) drawFramePhoto: (NSRect)border withClip: (NSRect)clip;

/** Draw a gradient border. */
- (NSRect) drawGradientBorder: (NSGradientType)gradientType 
		       inRect: (NSRect)border 
		     withClip: (NSRect)clip;

/** Draw a gray bezel border */
- (NSRect) drawGrayBezel: (NSRect)border withClip: (NSRect)clip;

/** Draw a groove border */
- (NSRect) drawGroove: (NSRect)border withClip: (NSRect)clip;

/** Draw a light bezel border */
- (NSRect) drawLightBezel: (NSRect)border withClip: (NSRect)clip;

/** Draw a white bezel border */
- (NSRect) drawWhiteBezel: (NSRect)border withClip: (NSRect)clip;

@end

/**
 * Low level drawing methods ... themes may use these for drawing,
 * but should not normally override them.
 */
@interface	GSTheme (LowLevelDrawing)
/**
 * Method to tile the supplied image to fill the horizontal rectangle.<br />
 * The rect argument is the rectangle to be filled.<br />
 * The image argument is the data to fill with.<br />
 * The source argument is the rectangle within the image which is used.<br />
 * The flipped argument specifies what sort of coordinate system is in
 * use in the view where we are drawing.
 */
- (void) fillHorizontalRect: (NSRect)rect
		  withImage: (NSImage*)image
		   fromRect: (NSRect)source
		    flipped: (BOOL)flipped;

/**
 * Tile rect with image.  The tiling starts with the origin of the
 * first copy of the image at the bottom left corner of the rect
 * unless center is YES, in which case the image is centered in rect
 * and tiled outwards from that.
 */
- (void) fillRect: (NSRect)rect
withRepeatedImage: (NSImage*)image
	 fromRect: (NSRect)source
	   center: (BOOL)center;

/**
 * Method to tile a rectangle given a group of up to nine tile images.<br />
 * The GSDrawTiles object encapsulates the tile images and information
 * about what parts of each image are used for tiling.<br />
 * This draws the left, right, top and bottom borders by tiling the
 * images at left, right, top and bottom.  It then draws the four corner
 * images and finally deals with the remaining space in the middle according
 * to the specified style.<br />
 * The background color specified is used to fill the center where
 * style is FillStyleNone.<br />
 * The return value is the central rectangle (inside the border images).
 */
- (NSRect) fillRect: (NSRect)rect
	  withTiles: (GSDrawTiles*)tiles
	 background: (NSColor*)color
	  fillStyle: (GSThemeFillStyle)style;

/**
 * Method to tile a rectangle given a group of up to nine tile images.<br />
 * The GSDrawTiles object encapsulates the tile images and information
 * about what parts of each image are used for tiling.<br />
 * This draws the left, right, top and bottom borders by tiling the
 * images at left, right, top and bottom.  It then draws the four corner
 * images and finally deals with the remaining space in the middle according
 * to the default style set for the GSDrawTiles object used.<br />
 * The background color specified is used to fill the center where
 * style is FillStyleNone.<br />
 * The return value is the central rectangle (inside the border images).
 */
- (NSRect) fillRect: (NSRect)rect
	  withTiles: (GSDrawTiles*)tiles
	 background: (NSColor*)color;

- (NSRect) fillRect: (NSRect)rect
	  withTiles: (GSDrawTiles*)tiles;

/**
 * Method to tile the supplied image to fill the vertical rectangle.<br />
 * The rect argument is the rectangle to be filled.<br />
 * The image argument is the data to fill with.<br />
 * The source argument is the rectangle within the image which is used.<br />
 * The flipped argument specifies what sort of coordinate system is in
 * use in the view where we are drawing.
 */
- (void) fillVerticalRect: (NSRect)rect
		withImage: (NSImage*)image
		 fromRect: (NSRect)source
		  flipped: (BOOL)flipped;
@end

@interface GSTheme (Menus)
/**
 * This method sets the menu for the window using the current theme 
 * In the default theme this calls the setMenu: method on the window
 * giving the menu parameter as the argument. 
 */ 
- (void)  setMenu: (NSMenu *)menu forWindow: (NSWindow *)window;

/**
 * Display the context menu when the right mouse button is pressed.
 */
- (void) rightMouseDisplay: (NSMenu *)menu
                  forEvent: (NSEvent *)theEvent;

/**
 * Display popup menu item.
 */
- (void) displayPopUpMenu: (NSMenuView *)mr
	    withCellFrame: (NSRect)cellFrame
	controlViewWindow: (NSWindow *)cvWin
	    preferredEdge: (NSRectEdge)edge
	     selectedItem: (int)selectedItem;

/**
 * Process events for popups.
 */
- (BOOL) doesProcessEventsForPopUpMenu;

/**
 * Display the menu icon in the application.
 */
- (BOOL) menuShouldShowIcon;

/**
 * Processes menu events for the theme.   The default implementation
 * does nothing.  
 */
- (void)  processCommand: (void *)context;

/**
 * Calculate the height of the menu for in-window menus.  The default
 * implementation returns [NSMenuView menuBarHeight];
 */
- (float) menuHeightForWindow: (NSWindow *)window;

/**
 * Update the menu for the window.  This refreshes the menu contents.
 * The default implementation of this method does nothing.
 */
- (void)  updateMenu: (NSMenu *)menu forWindow: (NSWindow *)window;
- (void) updateAllWindowsWithMenu: (NSMenu *) menu;
@end 

@interface GSTheme (OpenSavePanels)
/**
 * This method returns the open panel class needed by the
 * native environment.
 */ 
- (Class) openPanelClass;

/**
 * This method returns the open panel class needed by the
 * native environment.
 */ 
- (Class) savePanelClass;
@end

// Panels which can be overridden by the theme...
@interface GSPrintPanel : NSPrintPanel
@end

@interface GSPageLayout : NSPageLayout
@end

@interface GSTheme (PrintPanels)
/**
 * This method returns the print panel class needed by the
 * native environment.
 */
- (Class) printPanelClass;

/**
 * This method returns the page layout class needed by the 
 * native environment.
 */
- (Class) pageLayoutClass;
@end

@interface GSTheme (NSWindow)
/**
 * This method returns the standard window button for the
 * given mask for the current theme.
 */ 
- (NSButton *) standardWindowButton: (NSWindowButton)button
		       forStyleMask: (NSUInteger) mask;
				  
/** 
 * This method does any additional setup after the default 
 * cell is set.
 */
- (void) didSetDefaultButtonCell: (NSButtonCell *)aCell;
@end
					 
@interface GSTheme (NSBrowserCell)
/** 
 * Draw editor in cell
 */
- (void) drawEditorForCell: (NSCell *)cell
		 withFrame: (NSRect)cellFrame
	       	    inView: (NSView *)view;

/** 
 * Draw attributed text in cell
 */
- (void) drawInCell: (NSCell *)cell
     attributedText: (NSAttributedString *)stringValue
	    inFrame: (NSRect)cellFrame;

/** 
 * Draw the interior of the browser cell
 */
- (void) drawBrowserInteriorWithFrame: (NSRect)cellFrame 
			     withCell: (NSBrowserCell *)cell
			       inView: (NSView *)controlView
			    withImage: (NSImage *)theImage
		       alternateImage: (NSImage *)alternateImage
			isHighlighted: (BOOL)isHighlighted
				state: (int)state
			       isLeaf: (BOOL)isLeaf;

/** 
 * This method returns the branch image
 */
- (NSImage *) branchImage;

/** 
 * This method returns the highlighted version of 
 * the branch image
 */
- (NSImage *) highlightedBranchImage;
@end


#endif /* OS_API_VERSION */
#endif /* _GNUstep_H_GSTheme */
