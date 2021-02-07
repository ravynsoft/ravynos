/* -*-objc-*-
   NSRuleEditor.h

   The rule editor class

   Copyright (C) 2020 Free Software Foundation, Inc.

   Created by Fabian Spillner on 03.12.07.

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

#ifndef _GNUstep_H_NSRuleEditor
#define _GNUstep_H_NSRuleEditor

#import <AppKit/NSControl.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)

enum {
	NSRuleEditorNestingModeSingle,
	NSRuleEditorNestingModeList,
	NSRuleEditorNestingModeCompound,
	NSRuleEditorNestingModeSimple
};
typedef NSUInteger NSRuleEditorNestingMode;

enum {
	NSRuleEditorRowTypeSimple,
	NSRuleEditorRowTypeCompound
};
typedef NSUInteger NSRuleEditorRowType;

extern NSString * const NSRuleEditorPredicateLeftExpression;
extern NSString * const NSRuleEditorPredicateRightExpression;
extern NSString * const NSRuleEditorPredicateComparisonModifier;
extern NSString * const NSRuleEditorPredicateOptions;
extern NSString * const NSRuleEditorPredicateOperatorType;
extern NSString * const NSRuleEditorPredicateCustomSelector;
extern NSString * const NSRuleEditorPredicateCompoundType;

extern NSString *NSRuleEditorRowsDidChangeNotification;

@interface NSRuleEditor : NSControl {
  id _target;
  SEL _action;
}

- (void) addRow: (id)sender;
- (BOOL) canRemoveAllRows;
- (NSArray *) criteriaForRow: (NSInteger)index;
- (NSString *) criteriaKeyPath;
- (id) delegate;
- (NSArray *) displayValuesForRow: (NSInteger)index;
- (NSString *) displayValuesKeyPath;
- (NSDictionary *) formattingDictionary;
- (NSString *) formattingStringsFilename;
- (void) insertRowAtIndex: (NSInteger)index
                 withType: (NSRuleEditorRowType)type
            asSubrowOfRow: (NSInteger)row
                  animate: (BOOL)flag;
- (BOOL) isEditable;
- (NSRuleEditorNestingMode) nestingMode;
- (NSInteger) numberOfRows;
- (NSInteger) parentRowForRow: (NSInteger)row;
- (NSPredicate *) predicate;
- (NSPredicate *) predicateForRow: (NSInteger)row;
- (void) reloadCriteria;
- (void) reloadPredicate;
- (void) removeRowAtIndex: (NSInteger)index;
- (void) removeRowsAtIndexes: (NSIndexSet *)rowIds includeSubrows: (BOOL)flag;
- (Class) rowClass;
- (NSInteger) rowForDisplayValue: (id)value;
- (CGFloat) rowHeight;
- (NSRuleEditorRowType) rowTypeForRow: (NSInteger)row;
- (NSString *) rowTypeKeyPath;
- (NSIndexSet *) selectedRowIndexes;
- (void) selectRowIndexes: (NSIndexSet *)ids
     byExtendingSelection: (BOOL)flag;
- (void) setCanRemoveAllRows: (BOOL)flag;
- (void) setCriteria: (NSArray *)crits
    andDisplayValues: (NSArray *)vals
       forRowAtIndex: (NSInteger)index;
- (void) setCriteriaKeyPath: (NSString *)path;
- (void) setDelegate: (id)delegate;
- (void) setDisplayValuesKeyPath: (NSString *)path;
- (void) setEditable: (BOOL)flag;
- (void) setFormattingDictionary: (NSDictionary *)dict;
- (void) setFormattingStringsFilename: (NSString *)filename;
- (void) setNestingMode: (NSRuleEditorNestingMode)flag;
- (void) setRowClass: (Class)rowClass;
- (void) setRowHeight: (CGFloat)height;
- (void) setRowTypeKeyPath: (NSString *)path;
- (void) setSubrowsKeyPath: (NSString *)path;
- (NSIndexSet *) subrowIndexesForRow: (NSInteger)row;
- (NSString *) subrowsKeyPath;

@end

#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)

@protocol NSRuleEditorDelegate

- (id) ruleEditor: (NSRuleEditor *)editor
            child: (NSInteger)idx
     forCriterion: (id)crit
      withRowType: (NSRuleEditorRowType) type;
- (id) ruleEditor: (NSRuleEditor *)editor
displayValueForCriterion: (id)crit
            inRow: (NSInteger)row;
- (NSInteger) ruleEditor: (NSRuleEditor *)editor
numberOfChildrenForCriterion: (id)crit
             withRowType: (NSRuleEditorRowType)type;
- (NSDictionary *) ruleEditor: (NSRuleEditor *)editor
   predicatePartsForCriterion: (id)crit
             withDisplayValue: (id)val
                        inRow: (NSInteger)row;
- (void) ruleEditorRowsDidChange: (NSNotification *)notif;

@end

#endif
#endif /* _GNUstep_H_NSRuleEditor */
