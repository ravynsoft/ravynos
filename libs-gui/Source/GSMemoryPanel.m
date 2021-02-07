/* GSMemoryPanel.m                                           -*-objc-*-

   A GNUstep panel for tracking memory leaks.

   Copyright (C) 2000, 2002 Free Software Foundation, Inc.

   Author:  Nicola Pero <nicola@brainstorm.co.uk>
   Date: 2000, 2002

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

#import <Foundation/NSArray.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSString.h>
#import <Foundation/NSValue.h>
#import "AppKit/NSButton.h"
#import "AppKit/NSScrollView.h"
#import "AppKit/NSTableColumn.h"
#import "AppKit/NSTableView.h"
#import "GNUstepGUI/GSMemoryPanel.h"
#import "GNUstepGUI/GSHbox.h"
#import "GNUstepGUI/GSVbox.h"

enum {
  OrderByClassName,
  OrderByCount,
  OrderByTotal,
  OrderByPeak
};

static inline NSComparisonResult 
invertComparison (NSComparisonResult comparison)
{
  /* invert comparison */
  if (comparison == NSOrderedAscending)
    {
      comparison = NSOrderedDescending;
    }
  else if (comparison == NSOrderedDescending)
    {
      comparison = NSOrderedAscending;
    }  
  return comparison;
}

/*
 * Internal private class used in reordering entries.
 *
 */
@interface GSMemoryPanelEntry : NSObject
{
  NSString *string;
  NSNumber *count;
  NSNumber *total;
  NSNumber *peak;
}
- (id) initWithString: (NSString *)aString  
		count: (int)aCount
		total: (int)aTotal   
		 peak: (int)aPeak;
- (NSString *) string;
- (NSNumber *) count;
- (NSNumber *) total;
- (NSNumber *) peak;
- (NSComparisonResult) compareByTotal: (GSMemoryPanelEntry *)aEntry;
- (NSComparisonResult) compareByCount: (GSMemoryPanelEntry *)aEntry;
- (NSComparisonResult) compareByPeak: (GSMemoryPanelEntry *)aEntry;
- (NSComparisonResult) compareByClassName: (GSMemoryPanelEntry *)aEntry;
@end

@implementation GSMemoryPanelEntry

- (id) initWithString: (NSString *)aString  
		count: (int)aCount
		total: (int)aTotal   
		 peak: (int)aPeak
{
  ASSIGN (string, aString);
  ASSIGN (count, [NSNumber numberWithInt: aCount]);
  ASSIGN (total, [NSNumber numberWithInt: aTotal]);
  ASSIGN (peak, [NSNumber numberWithInt: aPeak]);
  return self;
}

- (void) dealloc
{
  RELEASE (string);
  RELEASE (count);
  RELEASE (total);  
  RELEASE (peak);
  [super dealloc];
}

- (NSString *) string
{
  return string;
}

- (NSNumber *) count
{
  return count;
}

- (NSNumber *) total
{
  return total;
}

- (NSNumber *) peak
{
  return peak;
}

- (NSComparisonResult) compareByCount: (GSMemoryPanelEntry *)aEntry
{
  NSComparisonResult comparison = [count compare: aEntry->count];

  return invertComparison (comparison);
}

- (NSComparisonResult) compareByTotal: (GSMemoryPanelEntry *)aEntry
{
  NSComparisonResult comparison = [total compare: aEntry->total];

  return invertComparison (comparison);
}

- (NSComparisonResult) compareByPeak: (GSMemoryPanelEntry *)aEntry
{
  NSComparisonResult comparison = [peak compare: aEntry->peak];

  return invertComparison (comparison);
}

- (NSComparisonResult) compareByClassName: (GSMemoryPanelEntry *)aEntry
{
  return [string compare: aEntry->string];
}

@end

/*
 * The Memory Panel code
 */

static GSMemoryPanel *sharedGSMemoryPanel = nil;

@implementation GSMemoryPanel
+ (id) sharedMemoryPanel
{
  if (sharedGSMemoryPanel == nil)
    {
      sharedGSMemoryPanel = [GSMemoryPanel new];
    }

  return sharedGSMemoryPanel;
}

+ (void) update: (id)sender
{
  [[self sharedMemoryPanel] update: sender];
}

- (id) init
{
  NSRect winFrame;
  NSTableColumn *classColumn;
  NSTableColumn *countColumn;  
  NSTableColumn *totalColumn;
  NSTableColumn *peakColumn;  
  NSScrollView *scrollView;
  GSVbox *vbox;
  GSHbox *hbox;
  NSButton *button;

  /* Activate debugging of allocation. */
  GSDebugAllocationActive (YES);

  hbox = [GSHbox new];
  [hbox setDefaultMinXMargin: 5];
  [hbox setBorder: 5];
  [hbox setAutoresizingMask: NSViewWidthSizable];

  /* Button updating the table.  */
  button = [NSButton new]; 
  [button setBordered: YES];
  [button setButtonType: NSMomentaryPushButton];
  [button setTitle:  @"Update"];
  [button setImagePosition: NSNoImage]; 
  [button setTarget: self];
  [button setAction: @selector(update:)];
  [button setAutoresizingMask: NSViewMaxXMargin];
  [button sizeToFit];
  [button setTag: 1];

  [hbox addView: button];
  RELEASE (button);
  
  /* Button taking snapshot of the table.  */
  button = [NSButton new]; 
  [button setBordered: YES];
  [button setButtonType: NSMomentaryPushButton];
  [button setTitle:  @"Snapshot"];
  [button setImagePosition: NSNoImage]; 
  [button setTarget: self];
  [button setAction: @selector(snapshot:)];
  [button setAutoresizingMask: NSViewMinXMargin];
  [button sizeToFit];
  [button setTag: 2];

  [hbox addView: button];
  RELEASE (button);
  
  classColumn = [[NSTableColumn alloc] initWithIdentifier: @"Class"];
  [classColumn setEditable: NO];
  [[classColumn headerCell] setStringValue: @"Class Name"];
  [classColumn setMinWidth: 200];

  countColumn = [[NSTableColumn alloc] initWithIdentifier: @"Count"];
  [countColumn setEditable: NO];
  [[countColumn headerCell] setStringValue: @"Current"];
  [countColumn setMinWidth: 50];

  totalColumn = [[NSTableColumn alloc] initWithIdentifier: @"Total"];
  [totalColumn setEditable: NO];
  [[totalColumn headerCell] setStringValue: @"Total"];
  [totalColumn setMinWidth: 50];

  peakColumn = [[NSTableColumn alloc] initWithIdentifier: @"Peak"];
  [peakColumn setEditable: NO];
  [[peakColumn headerCell] setStringValue: @"Peak"];
  [peakColumn setMinWidth: 50];

  table = [[NSTableView alloc] initWithFrame: NSMakeRect (0, 0, 300, 300)];
  [table addTableColumn: classColumn];
  RELEASE (classColumn);
  [table addTableColumn: countColumn];
  RELEASE (countColumn);
  [table addTableColumn: totalColumn];
  RELEASE (totalColumn);
  [table addTableColumn: peakColumn];
  RELEASE (peakColumn);
  [table setDataSource: self];
  [table setDelegate: self];
  [table setDoubleAction: @selector (reorder:)];
  
  scrollView = [[NSScrollView alloc] 
		 initWithFrame: NSMakeRect (0, 0, 350, 300)];
  [scrollView setDocumentView: table];
  [scrollView setHasHorizontalScroller: YES];
  [scrollView setHasVerticalScroller: YES];
  [scrollView setBorderType: NSBezelBorder];
  [scrollView setAutoresizingMask: (NSViewWidthSizable | NSViewHeightSizable)];
  [table sizeToFit];
  RELEASE (table);
 
  vbox = [GSVbox new];
  [vbox setDefaultMinYMargin: 5];
  [vbox setBorder: 5];
  [vbox addView: hbox  enablingYResizing: NO];
  RELEASE (hbox);
  [vbox addView: scrollView];
  RELEASE (scrollView);

  /* FIXME - should actually autosave the memory panel position and frame ! */
  winFrame.size = [vbox frame].size;
  winFrame.origin = NSMakePoint (100, 200);
  
  self = [super initWithContentRect: winFrame
                          styleMask: (NSTitledWindowMask 
                                      | NSClosableWindowMask 
                                      | NSMiniaturizableWindowMask 
                                      | NSResizableWindowMask)
                            backing: NSBackingStoreBuffered
                              defer: NO];
  if (nil == self)
    return nil;

  array = [NSMutableArray new];
  orderingBy = @selector(compareByCount:); 

  [self setReleasedWhenClosed: NO];
  [self setContentView: vbox];
  RELEASE (vbox);
  [self setTitle: @"Memory Panel"];
  
  return self;
}

- (void) dealloc
{
  RELEASE(array);
  [super dealloc];
}

- (NSInteger) numberOfRowsInTableView: (NSTableView *)aTableView
{
  return [array count];
}

- (id)           tableView: (NSTableView *)aTableView 
 objectValueForTableColumn: (NSTableColumn *)aTableColumn 
		       row:(NSInteger)rowIndex
{
  GSMemoryPanelEntry *entry = [array objectAtIndex: rowIndex];
  id identifier = [aTableColumn identifier];

  if ([identifier isEqual: @"Class"])
    {
      return [entry string];
    }
  else if ([identifier isEqual: @"Count"])
    {
      return [entry count];
    }
  else if ([identifier isEqual: @"Total"])
    {
      return [entry total];
    }
  else if ([identifier isEqual: @"Peak"])
    {
      return [entry peak];
    }

  NSLog (@"Hi, I am a bug in your table view");

  return @"";
}

- (void) snapshot: (id)sender
{
  GSMemoryPanel	*snapshot = [GSMemoryPanel new];

  [snapshot setTitle:
    [NSString stringWithFormat: @"Memory Snapshot at %@", [NSDate date]]];
  [[[snapshot contentView] viewWithTag: 1] removeFromSuperview];
  [[[snapshot contentView] viewWithTag: 2] removeFromSuperview];
  [snapshot setReleasedWhenClosed: YES];
  [snapshot makeKeyAndOrderFront: self];
  [snapshot update: self];
}

- (void) update: (id)sender
{
  Class *classList = GSDebugAllocationClassList ();
  Class *pointer;
  GSMemoryPanelEntry *entry;
  int i, count, total, peak;
  NSString *className;

  pointer = classList;
  i = 0;
  [array removeAllObjects];
  while (pointer[i] != NULL)
    {
      className = NSStringFromClass (pointer[i]);
      count = GSDebugAllocationCount (pointer[i]);
      total = GSDebugAllocationTotal (pointer[i]);
      peak = GSDebugAllocationPeak (pointer[i]);
      
      /* Insert into array */
      entry = [[GSMemoryPanelEntry alloc]
                initWithString: className
                         count: count  
                         total: total  
                          peak: peak];
      [array addObject: entry];
      RELEASE (entry);
      i++;
    }
  NSZoneFree(NSDefaultMallocZone(), classList);

  [array sortUsingSelector: orderingBy];

  [table reloadData];
}

- (void) reorder: (id)sender
{
  int selectedColumn = [table clickedColumn];
  NSArray *tableColumns = [table tableColumns];
  id identifier;
  SEL newOrderingBy = @selector(compareByCount:); 

  if (selectedColumn == -1)
    {
      return;
    }

  identifier
    = [(NSTableColumn*)[tableColumns objectAtIndex: selectedColumn] identifier];

  if ([identifier isEqual: @"Class"])
    {
      newOrderingBy = @selector(compareByClassName:); 
    }
  else if ([identifier isEqual: @"Count"])
    {
      newOrderingBy = @selector(compareByCount:); 
    }
  else if ([identifier isEqual: @"Total"])
    {
      newOrderingBy = @selector(compareByTotal:); 
    }
  else if ([identifier isEqual: @"Peak"])
    {
      newOrderingBy = @selector(compareByPeak:); 
    }

  if (newOrderingBy == orderingBy)
    {
      return;
    }
  else
    {
      orderingBy = newOrderingBy;
      [array sortUsingSelector: orderingBy];
      [table reloadData];
    }
}

@end

@implementation NSApplication (memoryPanel)

- (void) orderFrontSharedMemoryPanel: (id)sender
{
  GSMemoryPanel *memoryPanel;

  memoryPanel = [GSMemoryPanel sharedMemoryPanel];
  [memoryPanel update: self];
  [memoryPanel orderFront: self];
}

@end
