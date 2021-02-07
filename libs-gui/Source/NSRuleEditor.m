/** <title>NSRuleEditor</title>

   <abstract>The rule editor class</abstract>

   Copyright (C) 2020 Free Software Foundation, Inc.

   Author: Fred Kiefer <fredkiefer@gmx.de>
   Date:   January 2020

   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the
   Free Software Foundation, 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#import "AppKit/NSRuleEditor.h"


@implementation NSRuleEditor
- (void) addRow: (id)sender
{
}

- (BOOL) canRemoveAllRows
{
  return YES;
}

- (NSArray *) criteriaForRow: (NSInteger)index
{
  return nil;
}

- (NSString *) criteriaKeyPath
{
  return nil;
}

- (id) delegate
{
  return nil;
}

- (NSArray *) displayValuesForRow: (NSInteger)index
{
  return nil;
}

- (NSString *) displayValuesKeyPath
{
  return nil;
}

- (NSDictionary *) formattingDictionary
{
  return nil;
}

- (NSString *) formattingStringsFilename
{
  return nil;
}

- (void) insertRowAtIndex: (NSInteger)index
                 withType: (NSRuleEditorRowType)type
            asSubrowOfRow: (NSInteger)row
                  animate: (BOOL)flag
{
}

- (BOOL) isEditable
{
  return YES;
}

- (NSRuleEditorNestingMode) nestingMode
{
  return NSRuleEditorNestingModeSingle;
}

- (NSInteger) numberOfRows
{
  return 0;
}

- (NSInteger) parentRowForRow: (NSInteger)row
{
  return 0;
}

- (NSPredicate *) predicate
{
  return nil;
}

- (NSPredicate *) predicateForRow: (NSInteger)row
{
  return nil;
}

- (void) reloadCriteria
{
}

- (void) reloadPredicate
{
}

- (void) removeRowAtIndex: (NSInteger)index
{
}

- (void) removeRowsAtIndexes: (NSIndexSet *)rowIds includeSubrows: (BOOL)flag
{
}

- (Class) rowClass
{
  return nil;
}

- (NSInteger) rowForDisplayValue: (id)value
{
  return 0;
}

- (CGFloat) rowHeight
{
  return 0.0;
}

- (NSRuleEditorRowType) rowTypeForRow: (NSInteger)row
{
  return NSRuleEditorRowTypeSimple;
}

- (NSString *) rowTypeKeyPath
{
  return nil;
}

- (NSIndexSet *) selectedRowIndexes
{
  return nil;
}

- (void) selectRowIndexes: (NSIndexSet *)ids
     byExtendingSelection: (BOOL)flag
{
}

- (void) setCanRemoveAllRows: (BOOL)flag
{
}

- (void) setCriteria: (NSArray *)crits
    andDisplayValues: (NSArray *)vals
       forRowAtIndex: (NSInteger)index
{
}

- (void) setCriteriaKeyPath: (NSString *)path
{
}

- (void) setDelegate: (id)delegate
{
}

- (void) setDisplayValuesKeyPath: (NSString *)path
{
}

- (void) setEditable: (BOOL)flag
{
}

- (void) setFormattingDictionary: (NSDictionary *)dict
{
}

- (void) setFormattingStringsFilename: (NSString *)filename
{
}

- (void) setNestingMode: (NSRuleEditorNestingMode)flag
{
}

- (void) setRowClass: (Class)rowClass
{
}

- (void) setRowHeight: (CGFloat)height
{
}

- (void) setRowTypeKeyPath: (NSString *)path
{
}

- (void) setSubrowsKeyPath: (NSString *)path
{
}

- (NSIndexSet *) subrowIndexesForRow: (NSInteger)row
{
  return nil;
}

- (NSString *) subrowsKeyPath
{
  return nil;
}

- (void) viewDidMoveToWindow
{
}

- (void) setAction: (SEL)action
{
  _action = action;
}

- (SEL) action
{
  return _action;
}

- (void) setTarget: (id)target
{
  _target = target;
}

- (id) target
{
  return _target;
}

@end
