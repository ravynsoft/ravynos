/*
   GSHbox.h

   The GSHbox class (a GNU extension)

   Copyright (C) 1999 Free Software Foundation, Inc.

   Author:  Nicola Pero <n.pero@mi.flashnet.it>
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

#ifndef _GNUstep_H_GSHbox
#define _GNUstep_H_GSHbox

#import "GSTable.h"

/** 
  <unit>
  <heading>GSHbox</heading>
 
  <p>
  GSHbox inherits from GSTable the autosizing/autoresizing engine.
  The only real difference between a GSHbox and a GSTable with 1 row 
  is that the GSHbox has a much simpler, easier and friendlier API. 
  </p>
  <p>
  You shouldn't use GSTable methods with GSHbox (exception: methods 
  explicitly quoted in comments to this file as 'inherited from GSTable'). 
  If you need to do that, you should be using GSTable instead.
  </p>
  <p>
  A GSHbox is an invisible view (a logical device) which can
  contain some views.  The GSHbox controls the position and sizes
  of these views so that they are lined up in a row.
  </p>
  <p>
  To initialize a GSHbox, you should always use -init
  method.  Don't use GSTable methods.  The correct way to start
  using a new GSHbox is simply:
  </p>
  <example>
  hbox = [GSHbox new];
  </example>
  <p>
  (well, of course, autoreleasing it if necessary).  You add a view to a
  GSHbox using the method -addView: and its variants.  The
  views you add to a GSHbox are placed by the GSHbox in its
  subview hierarchy, and moved (and/or resized in the vertical direction)
  so that they are positioned one after the other, from left to right, in
  a row.  Before adding views to a box, you should resize them to the
  least comfortable size you want them to have.  The GSHbox
  considers this size as the minimum size your view should ever have, and
  never resizes your view below this size.
  </p>
  <p>
  The initial size of the GSHbox is zero; each time you add a view
  in the GSHbox, the GSHbox resizes itself to fit the new
  contents.  Usually, you simply add objects to the GSHbox, and let
  it compute its size (this is the minimum size); you may get this
  resulting size by
  </p>
  <example>
  size = [yourHBox size];
  </example>
  <p>
  for example, if the GSHbox is to be used as the content view of a window,
  you may create the window exactly with this size.
  </p>
  <p>
  You should never force a GSHbox in a size different from the one
  it has automatically computed.  It sounds quite pointless anyway.  The
  only correct (and meaningful) way to resize a GSHbox is through
  -resizeWithOldSuperviewSize: messages (in the view hierarchy).
  In other words, after you place your box in the view hierarchy, then you
  may resize the superview and (if the superview has autoresizing of
  subviews enabled) your box is resized automatically accordingly.
  </p>
  <p>
  By default, there is no space between the added views.  By using the method
  -addView:withMinXMargin: you may tell the GSHbox to insert
  some space (a <var>margin</var>) between the view you are adding and the
  previous one (the one at its left).
  </p>
  <p>
  If what you want is space around the GSHbox, and not between
  views in the GSHbox, you don't want a margin but a <var>border</var>;
  you should then use -setBorder:, which will add an equal amount
  of space on all the sides of the box.  You can also set a different
  border on each side (see -setMinXBorder: and similar methods).
  </p>
  <p>
  A useful feature of GSHbox is that it supports <var>separators</var>.
  This facility is not directly available in GSTable (to add
  separators to a GSTable you need to create and handle them
  yourself).  A GSHbox separator is a vertical groove line, used to
  mark the separation between different elements of a box.  To add a
  separator, simply invoke the method -addSeparator.  The separator
  is put at the right of the last added view.
  </p>
  <p>
  To use GSHbox proficiently, it is crucial to set correctly the
  autoresizing mask of each view before adding it to the GSHbox.
  </p>
  <p>
  The GSHbox treats each view and its margins as a whole (see the
  GSTable class description for more information).
  </p>
  <p>
  When the GSHbox is resized in the vertical direction (as a
  consequence of user intervertion, for example), what happens is:
  </p>
  <list>
  <item> if the new height is less than the minimum height of the GSHbox 
  (computed as the maximum of the minimum height of the added views), it
  simply resizes all the added views to this minimum height; part of them
  are clipped then.
  </item>
  <item> if the new height is greater than the GSHbox's minimum height, 
  the GSHbox resizes all the added views to the new height.  This
  is done through the standard superview-subview resizing mechanism, so
  that, by setting the autoresizingMask of each view that you add,
  you are able to control exactly how the resizing effects each view and
  its margins.
  </item>
  </list>
  <p>
  When the GSHbox is resized in the horizontal direction, its
  behaviour is as follows:
  </p>
  <list>
  <item> If the new width is less than 
  the minimum width, all the added views are sized to minimum width; part
  of them is clipped then.
  </item>
  <item> If the new width is greater than 
  the minimum width, some of the views are resized.  You may decide which
  views you want to be resized and which not; to disable resizing of a
  certain view in the horizontal direction, you should specify a NO
  value to the option <code>enablingXResizing</code> when you add the view to
  the box.  Views with X Resizing Not Enabled are always kept in their
  minimum width (the original one), and never resized.  If nothing is
  specified, a default of YES for <code>enablingXResizing</code> is
  understood.  So, when the new width is greater than the minimum width,
  the excess width is equally divided between the view with X Resizing
  Enabled.  The actual resizing is done through the usual
  superview-subview resizing mechanism, so that again you may influence
  the way the resizing affects each view by setting the autoresizing mask
  of each view.
  </item>
  </list>

  </unit>
*/
@interface GSHbox: GSTable
{
  BOOL _haveViews;
  float _defaultMinXMargin;
}
//
// Initizialing.  
//
/** Always use init for GSHbox: other methods don't make sense. 
 Don't used GSTable methods.  You do not need to specify 
 the number of views you plan to put in the box 
 when you initialize it. 
*/
-(id) init;

//
//  Adding a View. 
//
/** See  -addView:enablingXResizing:withMinXMargin: */
-(void) addView: (NSView *)aView;

/** See  -addView:enablingXResizing:withMinXMargin: */
-(void) addView: (NSView *)aView
enablingXResizing: (BOOL)aFlag;

/** See  -addView:enablingXResizing:withMinXMargin: */
-(void) addView: (NSView *)aView
 withMinXMargin: (float)aMargin;

/** <p> Pack views in the GSHbox. 
  Don't use the corresponding methods of GSTable, which are far more general 
  and far more complicate.  If you need to do that, use GSTable instead. 
  </p>
  <p>
  Add a view to the box, enabling X Resizing only if flag is
  YES, and a MinXMargin aMargin.  If aFlag is
  YES the [view and its margins] should be resized in the
  horizontal direction when the GSHbox is resized in the horizontal
  direction.  If aFlag is NO the view is never X-resized and
  always left in its original width.  The default is YES.
  </p>
  <p>
  The min X margin is used to separate the view from the preceding one.
  The first view added to the box has no min X margin; if you try setting
  one for it, it is ignored (zero is used instead).
  </p>
  <p>
  When views are added to the GSHbox, it might happen that some of
  the added views have a greater height than others.  When this happens,
  the GSHbox resizes all the views to the highest height.  As
  usual, each view is resized with its margins; the effect of the resizing
  on each view is determined by the autoresizing mask of the view.  The
  classical options are
  </p>
  <deflist>
  <term> (NSViewMinYMargin | NSViewMaxYMargin) </term>
  <desc>Center the view vertically</desc>
  <term> NSViewMinYMargin </term>
  <desc>Flush the view up (down if the GSHbox is flipped)</desc>
  <term> NSViewMaxYMargin  </term>
  <desc>Flush the view down (up if the GSHbox is flipped)</desc>
  <term> NSViewHeightSizable </term>
  <desc>Expand the view to the whole height</desc>
  </deflist>
  <p>
  (you may need to OR these masks with the mask you use in the
  horizontal direction, if you use any).
  </p>
  <p>
  With a GSHbox, only one margin is set when you add views to the GSHbox: 
  the margin between each view and the preceding one. 
  Exception: the first view is special, and has no margin set (it has no 
  preceding view to be separated from). 
  Space above or below the view may result if the view is shorter, 
  in the vertical direction, than the other views in the GSHbox; 
  in that case the view is resized to fit vertically, 
  according to its autoresizingMask.  
  By changing the autoresizingMask you may decide whether the space 
  should go to the view or to its vertical margins; this for example 
  lets you center vertically or flush up/down your view.
  </p>
*/
-(void) addView: (NSView *)aView
  enablingXResizing: (BOOL)aFlag
  withMinXMargin: (float)aMargin;

//
// Adding a Separator. 
//
/** Add a separator with the default MinXMargin. */
-(void) addSeparator;

/** Add a separator (a vertical groove line encompassing all the
height of the GSHbox) to the GSHbox, inserting a margin aMargin
between the separator and the last added view.
*/
-(void) addSeparatorWithMinXMargin: (float)aMargin;

//
//  Setting Margins.  
//
  
/** Use only the following method to set a default margin. 
 The default margin set with the following method will be used 
 for all the views added after.  
 (Exception: the first view put in the box has no margins at all)
 It will not affect already added views.
 In a GSHbox, only one margin is used, the one between each view 
 and the preceding one.  If what you want is space around the GSHbox, 
 you don't want a margin but a border; use setBorder: 
 (see GSTable, "Setting Border"). 
 If you need more complicated margins/borders, use GSTable. 
*/
-(void) setDefaultMinXMargin: (float)aMargin;

//
// Getting Number of Views
//

/** Return the number of views in the GSHbox (separators included).  */
-(int) numberOfViews;
@end

#endif /* _GNUstep_H_GSHbox */
