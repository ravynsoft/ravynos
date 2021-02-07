/** NSAccessibilityConstants.h

   <abstract>Contains constants for Accessibility functionality</abstract>

   Copyright <copy>(C) 2017 Free Software Foundation, Inc.</copy>

   Author: Daniel Ferreira <dtf@stanford.edu>
   Date: 2017

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

#ifndef _GNUstep_H_NSAccessibilityConstants
#define _GNUstep_H_NSAccessibilityConstants

#import <AppKit/AppKitDefines.h>
#import <Foundation/Foundation.h>

APPKIT_EXPORT NSString *const NSAccessibilityErrorCodeExceptionInfo;

APPKIT_EXPORT NSString *const NSAccessibilityRoleAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityRoleDescriptionAttribute;
APPKIT_EXPORT NSString *const NSAccessibilitySubroleAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityHelpAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityValueAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityMinValueAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityMaxValueAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityEnabledAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityFocusedAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityParentAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityChildrenAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityWindowAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityTopLevelUIElementAttribute;
APPKIT_EXPORT NSString *const NSAccessibilitySelectedChildrenAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityVisibleChildrenAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityPositionAttribute;
APPKIT_EXPORT NSString *const NSAccessibilitySizeAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityContentsAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityTitleAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityDescriptionAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityShownMenuAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityValueDescriptionAttribute;
APPKIT_EXPORT NSString *const NSAccessibilitySharedFocusElementsAttribute;

APPKIT_EXPORT NSString *const NSAccessibilityPreviousContentsAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityNextContentsAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityHeaderAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityEditedAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityTabsAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityHorizontalScrollBarAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityVerticalScrollBarAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityOverflowButtonAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityIncrementButtonAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityDecrementButtonAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityFilenameAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityExpandedAttribute;
APPKIT_EXPORT NSString *const NSAccessibilitySelectedAttribute;
APPKIT_EXPORT NSString *const NSAccessibilitySplittersAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityDocumentAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityActivationPointAttribute;

APPKIT_EXPORT NSString *const NSAccessibilityURLAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityIndexAttribute;

APPKIT_EXPORT NSString *const NSAccessibilityRowCountAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityColumnCountAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityOrderedByRowAttribute;

APPKIT_EXPORT NSString *const NSAccessibilityWarningValueAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityCriticalValueAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityPlaceholderValueAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityContainsProtectedContentAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityAlternateUIVisibleAttribute;

APPKIT_EXPORT NSString *const NSAccessibilityTitleUIElementAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityServesAsTitleForUIElementsAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityLinkedUIElementsAttribute;

APPKIT_EXPORT NSString *const NSAccessibilitySelectedTextAttribute;
APPKIT_EXPORT NSString *const NSAccessibilitySelectedTextRangeAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityNumberOfCharactersAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityVisibleCharacterRangeAttribute;
APPKIT_EXPORT NSString *const NSAccessibilitySharedTextUIElementsAttribute;
APPKIT_EXPORT NSString *const NSAccessibilitySharedCharacterRangeAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityInsertionPointLineNumberAttribute;
APPKIT_EXPORT NSString *const NSAccessibilitySelectedTextRangesAttribute;

APPKIT_EXPORT NSString *const NSAccessibilityLineForIndexParameterizedAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityRangeForLineParameterizedAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityStringForRangeParameterizedAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityRangeForPositionParameterizedAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityRangeForIndexParameterizedAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityBoundsForRangeParameterizedAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityRTFForRangeParameterizedAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityStyleRangeForIndexParameterizedAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityAttributedStringForRangeParameterizedAttribute;

APPKIT_EXPORT NSString *const NSAccessibilityFontTextAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityForegroundColorTextAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityBackgroundColorTextAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityUnderlineColorTextAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityStrikethroughColorTextAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityUnderlineTextAttribute;
APPKIT_EXPORT NSString *const NSAccessibilitySuperscriptTextAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityStrikethroughTextAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityShadowTextAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityAttachmentTextAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityLinkTextAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityAutocorrectedTextAttribute;

APPKIT_EXPORT NSString *const NSAccessibilityListItemPrefixTextAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityListItemIndexTextAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityListItemLevelTextAttribute;

APPKIT_EXPORT NSString *const NSAccessibilityMisspelledTextAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityMarkedMisspelledTextAttribute;

APPKIT_EXPORT NSString *const NSAccessibilityFontNameKey;
APPKIT_EXPORT NSString *const NSAccessibilityFontFamilyKey;
APPKIT_EXPORT NSString *const NSAccessibilityVisibleNameKey;
APPKIT_EXPORT NSString *const NSAccessibilityFontSizeKey;

APPKIT_EXPORT NSString *const NSAccessibilityMainAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityMinimizedAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityCloseButtonAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityZoomButtonAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityMinimizeButtonAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityToolbarButtonAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityProxyAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityGrowAreaAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityModalAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityDefaultButtonAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityCancelButtonAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityFullScreenButtonAttribute;

APPKIT_EXPORT NSString *const NSAccessibilityMenuBarAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityWindowsAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityFrontmostAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityHiddenAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityMainWindowAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityFocusedWindowAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityFocusedUIElementAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityExtrasMenuBarAttribute;

typedef NSInteger NSAccessibilityOrientation;
enum
{
  NSAccessibilityOrientationUnknown = 0,
  NSAccessibilityOrientationVertical = 1,
  NSAccessibilityOrientationHorizontal = 2,
};

APPKIT_EXPORT NSString *const NSAccessibilityOrientationAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityVerticalOrientationValue;
APPKIT_EXPORT NSString *const NSAccessibilityHorizontalOrientationValue;
APPKIT_EXPORT NSString *const NSAccessibilityUnknownOrientationValue;

APPKIT_EXPORT NSString *const NSAccessibilityColumnTitlesAttribute;

APPKIT_EXPORT NSString *const NSAccessibilitySearchButtonAttribute;
APPKIT_EXPORT NSString *const NSAccessibilitySearchMenuAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityClearButtonAttribute;

APPKIT_EXPORT NSString *const NSAccessibilityRowsAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityVisibleRowsAttribute;
APPKIT_EXPORT NSString *const NSAccessibilitySelectedRowsAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityColumnsAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityVisibleColumnsAttribute;
APPKIT_EXPORT NSString *const NSAccessibilitySelectedColumnsAttribute;
APPKIT_EXPORT NSString *const NSAccessibilitySortDirectionAttribute;

APPKIT_EXPORT NSString *const NSAccessibilitySelectedCellsAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityVisibleCellsAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityRowHeaderUIElementsAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityColumnHeaderUIElementsAttribute;

APPKIT_EXPORT NSString *const NSAccessibilityCellForColumnAndRowParameterizedAttribute;

APPKIT_EXPORT NSString *const NSAccessibilityRowIndexRangeAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityColumnIndexRangeAttribute;

APPKIT_EXPORT NSString *const NSAccessibilityHorizontalUnitsAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityVerticalUnitsAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityHorizontalUnitDescriptionAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityVerticalUnitDescriptionAttribute;

APPKIT_EXPORT NSString *const NSAccessibilityLayoutPointForScreenPointParameterizedAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityLayoutSizeForScreenSizeParameterizedAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityScreenPointForLayoutPointParameterizedAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityScreenSizeForLayoutSizeParameterizedAttribute;

APPKIT_EXPORT NSString *const NSAccessibilityHandlesAttribute;

APPKIT_EXPORT NSString *const NSAccessibilityAscendingSortDirectionValue;
APPKIT_EXPORT NSString *const NSAccessibilityDescendingSortDirectionValue;
APPKIT_EXPORT NSString *const NSAccessibilityUnknownSortDirectionValue;

typedef NSInteger NSAccessibilitySortDirection;
enum
{
  NSAccessibilitySortDirectionUnknown = 0,
  NSAccessibilitySortDirectionAscending = 1,
  NSAccessibilitySortDirectionDescending = 2,
};

APPKIT_EXPORT NSString *const NSAccessibilityDisclosingAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityDisclosedRowsAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityDisclosedByRowAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityDisclosureLevelAttribute;

APPKIT_EXPORT NSString *const NSAccessibilityAllowedValuesAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityLabelUIElementsAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityLabelValueAttribute;

APPKIT_EXPORT NSString *const NSAccessibilityMatteHoleAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityMatteContentUIElementAttribute;

APPKIT_EXPORT NSString *const NSAccessibilityMarkerUIElementsAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityMarkerValuesAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityMarkerGroupUIElementAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityUnitsAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityUnitDescriptionAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityMarkerTypeAttribute;
APPKIT_EXPORT NSString *const NSAccessibilityMarkerTypeDescriptionAttribute;

APPKIT_EXPORT NSString *const NSAccessibilityIdentifierAttribute;

APPKIT_EXPORT NSString *const NSAccessibilityLeftTabStopMarkerTypeValue;
APPKIT_EXPORT NSString *const NSAccessibilityRightTabStopMarkerTypeValue;
APPKIT_EXPORT NSString *const NSAccessibilityCenterTabStopMarkerTypeValue;
APPKIT_EXPORT NSString *const NSAccessibilityDecimalTabStopMarkerTypeValue;
APPKIT_EXPORT NSString *const NSAccessibilityHeadIndentMarkerTypeValue;
APPKIT_EXPORT NSString *const NSAccessibilityTailIndentMarkerTypeValue;
APPKIT_EXPORT NSString *const NSAccessibilityFirstLineIndentMarkerTypeValue;
APPKIT_EXPORT NSString *const NSAccessibilityUnknownMarkerTypeValue;

typedef NSInteger NSAccessibilityRulerMarkerType;
enum
{
  NSAccessibilityRulerMarkerTypeUnknown = 0,
  NSAccessibilityRulerMarkerTypeTabStopLeft = 1,
  NSAccessibilityRulerMarkerTypeTabStopRight = 2,
  NSAccessibilityRulerMarkerTypeTabStopCenter = 3,
  NSAccessibilityRulerMarkerTypeTabStopDecimal = 4,
  NSAccessibilityRulerMarkerTypeIndentHead = 5,
  NSAccessibilityRulerMarkerTypeIndentTail = 6,
  NSAccessibilityRulerMarkerTypeIndentFirstLine = 7
};

APPKIT_EXPORT NSString *const NSAccessibilityInchesUnitValue;
APPKIT_EXPORT NSString *const NSAccessibilityCentimetersUnitValue;
APPKIT_EXPORT NSString *const NSAccessibilityPointsUnitValue;
APPKIT_EXPORT NSString *const NSAccessibilityPicasUnitValue;
APPKIT_EXPORT NSString *const NSAccessibilityUnknownUnitValue;

typedef NSInteger NSAccessibilityUnits;
enum
{
  NSAccessibilityUnitsUnknown = 0,
  NSAccessibilityUnitsInches = 1,
  NSAccessibilityUnitsCentimeters = 2,
  NSAccessibilityUnitsPoints = 3,
  NSAccessibilityUnitsPicas = 4
};

APPKIT_EXPORT NSString *const NSAccessibilityPressAction;
APPKIT_EXPORT NSString *const NSAccessibilityIncrementAction;
APPKIT_EXPORT NSString *const NSAccessibilityDecrementAction;
APPKIT_EXPORT NSString *const NSAccessibilityConfirmAction;
APPKIT_EXPORT NSString *const NSAccessibilityPickAction;
APPKIT_EXPORT NSString *const NSAccessibilityCancelAction;
APPKIT_EXPORT NSString *const NSAccessibilityRaiseAction;
APPKIT_EXPORT NSString *const NSAccessibilityShowMenuAction;
APPKIT_EXPORT NSString *const NSAccessibilityDeleteAction;

APPKIT_EXPORT NSString *const NSAccessibilityShowAlternateUIAction;
APPKIT_EXPORT NSString *const NSAccessibilityShowDefaultUIAction;

APPKIT_EXPORT NSString *const NSAccessibilityMainWindowChangedNotification;
APPKIT_EXPORT NSString *const NSAccessibilityFocusedWindowChangedNotification;
APPKIT_EXPORT NSString *const NSAccessibilityFocusedUIElementChangedNotification;

APPKIT_EXPORT NSString *const NSAccessibilityApplicationActivatedNotification;
APPKIT_EXPORT NSString *const NSAccessibilityApplicationDeactivatedNotification;
APPKIT_EXPORT NSString *const NSAccessibilityApplicationHiddenNotification;
APPKIT_EXPORT NSString *const NSAccessibilityApplicationShownNotification;

APPKIT_EXPORT NSString *const NSAccessibilityWindowCreatedNotification;
APPKIT_EXPORT NSString *const NSAccessibilityWindowMovedNotification;
APPKIT_EXPORT NSString *const NSAccessibilityWindowResizedNotification;
APPKIT_EXPORT NSString *const NSAccessibilityWindowMiniaturizedNotification;
APPKIT_EXPORT NSString *const NSAccessibilityWindowDeminiaturizedNotification;

APPKIT_EXPORT NSString *const NSAccessibilityDrawerCreatedNotification;
APPKIT_EXPORT NSString *const NSAccessibilitySheetCreatedNotification;

APPKIT_EXPORT NSString *const NSAccessibilityUIElementDestroyedNotification;
APPKIT_EXPORT NSString *const NSAccessibilityValueChangedNotification;
APPKIT_EXPORT NSString *const NSAccessibilityTitleChangedNotification;
APPKIT_EXPORT NSString *const NSAccessibilityResizedNotification;
APPKIT_EXPORT NSString *const NSAccessibilityMovedNotification;
APPKIT_EXPORT NSString *const NSAccessibilityCreatedNotification;

APPKIT_EXPORT NSString *const NSAccessibilityLayoutChangedNotification;

APPKIT_EXPORT NSString *const NSAccessibilityHelpTagCreatedNotification;
APPKIT_EXPORT NSString *const NSAccessibilitySelectedTextChangedNotification;
APPKIT_EXPORT NSString *const NSAccessibilityRowCountChangedNotification;
APPKIT_EXPORT NSString *const NSAccessibilitySelectedChildrenChangedNotification;
APPKIT_EXPORT NSString *const NSAccessibilitySelectedRowsChangedNotification;
APPKIT_EXPORT NSString *const NSAccessibilitySelectedColumnsChangedNotification;

APPKIT_EXPORT NSString *const NSAccessibilityRowExpandedNotification;
APPKIT_EXPORT NSString *const NSAccessibilityRowCollapsedNotification;

APPKIT_EXPORT NSString *const NSAccessibilitySelectedCellsChangedNotification;

APPKIT_EXPORT NSString *const NSAccessibilityUnitsChangedNotification;
APPKIT_EXPORT NSString *const NSAccessibilitySelectedChildrenMovedNotification;

APPKIT_EXPORT NSString *const NSAccessibilityAnnouncementRequestedNotification;

APPKIT_EXPORT NSString *const NSAccessibilityUnknownRole;
APPKIT_EXPORT NSString *const NSAccessibilityButtonRole;
APPKIT_EXPORT NSString *const NSAccessibilityRadioButtonRole;
APPKIT_EXPORT NSString *const NSAccessibilityCheckBoxRole;
APPKIT_EXPORT NSString *const NSAccessibilitySliderRole;
APPKIT_EXPORT NSString *const NSAccessibilityTabGroupRole;
APPKIT_EXPORT NSString *const NSAccessibilityTextFieldRole;
APPKIT_EXPORT NSString *const NSAccessibilityStaticTextRole;
APPKIT_EXPORT NSString *const NSAccessibilityTextAreaRole;
APPKIT_EXPORT NSString *const NSAccessibilityScrollAreaRole;
APPKIT_EXPORT NSString *const NSAccessibilityPopUpButtonRole;
APPKIT_EXPORT NSString *const NSAccessibilityMenuButtonRole;
APPKIT_EXPORT NSString *const NSAccessibilityTableRole;
APPKIT_EXPORT NSString *const NSAccessibilityApplicationRole;
APPKIT_EXPORT NSString *const NSAccessibilityGroupRole;
APPKIT_EXPORT NSString *const NSAccessibilityRadioGroupRole;
APPKIT_EXPORT NSString *const NSAccessibilityListRole;
APPKIT_EXPORT NSString *const NSAccessibilityScrollBarRole;
APPKIT_EXPORT NSString *const NSAccessibilityValueIndicatorRole;
APPKIT_EXPORT NSString *const NSAccessibilityImageRole;
APPKIT_EXPORT NSString *const NSAccessibilityMenuBarRole;
APPKIT_EXPORT NSString *const NSAccessibilityMenuRole;
APPKIT_EXPORT NSString *const NSAccessibilityMenuItemRole;
APPKIT_EXPORT NSString *const NSAccessibilityColumnRole;
APPKIT_EXPORT NSString *const NSAccessibilityRowRole;
APPKIT_EXPORT NSString *const NSAccessibilityToolbarRole;
APPKIT_EXPORT NSString *const NSAccessibilityBusyIndicatorRole;
APPKIT_EXPORT NSString *const NSAccessibilityProgressIndicatorRole;
APPKIT_EXPORT NSString *const NSAccessibilityWindowRole;
APPKIT_EXPORT NSString *const NSAccessibilityDrawerRole;
APPKIT_EXPORT NSString *const NSAccessibilitySystemWideRole;
APPKIT_EXPORT NSString *const NSAccessibilityOutlineRole;
APPKIT_EXPORT NSString *const NSAccessibilityIncrementorRole;
APPKIT_EXPORT NSString *const NSAccessibilityBrowserRole;
APPKIT_EXPORT NSString *const NSAccessibilityComboBoxRole;
APPKIT_EXPORT NSString *const NSAccessibilitySplitGroupRole;
APPKIT_EXPORT NSString *const NSAccessibilitySplitterRole;
APPKIT_EXPORT NSString *const NSAccessibilityColorWellRole;
APPKIT_EXPORT NSString *const NSAccessibilityGrowAreaRole;
APPKIT_EXPORT NSString *const NSAccessibilitySheetRole;
APPKIT_EXPORT NSString *const NSAccessibilityHelpTagRole;
APPKIT_EXPORT NSString *const NSAccessibilityMatteRole;
APPKIT_EXPORT NSString *const NSAccessibilityRulerRole;
APPKIT_EXPORT NSString *const NSAccessibilityRulerMarkerRole;
APPKIT_EXPORT NSString *const NSAccessibilityLinkRole;
APPKIT_EXPORT NSString *const NSAccessibilityDisclosureTriangleRole;
APPKIT_EXPORT NSString *const NSAccessibilityGridRole;
APPKIT_EXPORT NSString *const NSAccessibilityRelevanceIndicatorRole;
APPKIT_EXPORT NSString *const NSAccessibilityLevelIndicatorRole;
APPKIT_EXPORT NSString *const NSAccessibilityCellRole;
APPKIT_EXPORT NSString *const NSAccessibilityPopoverRole;
APPKIT_EXPORT NSString *const NSAccessibilitySortButtonRole;

APPKIT_EXPORT NSString *const NSAccessibilityLayoutAreaRole;
APPKIT_EXPORT NSString *const NSAccessibilityLayoutItemRole;
APPKIT_EXPORT NSString *const NSAccessibilityHandleRole;

APPKIT_EXPORT NSString *const NSAccessibilityUnknownSubrole;
APPKIT_EXPORT NSString *const NSAccessibilityCloseButtonSubrole;
APPKIT_EXPORT NSString *const NSAccessibilityZoomButtonSubrole;
APPKIT_EXPORT NSString *const NSAccessibilityMinimizeButtonSubrole;
APPKIT_EXPORT NSString *const NSAccessibilityToolbarButtonSubrole;
APPKIT_EXPORT NSString *const NSAccessibilityTableRowSubrole;
APPKIT_EXPORT NSString *const NSAccessibilityOutlineRowSubrole;
APPKIT_EXPORT NSString *const NSAccessibilitySecureTextFieldSubrole;
APPKIT_EXPORT NSString *const NSAccessibilityStandardWindowSubrole;
APPKIT_EXPORT NSString *const NSAccessibilityDialogSubrole;
APPKIT_EXPORT NSString *const NSAccessibilitySystemDialogSubrole;
APPKIT_EXPORT NSString *const NSAccessibilityFloatingWindowSubrole;
APPKIT_EXPORT NSString *const NSAccessibilitySystemFloatingWindowSubrole;
APPKIT_EXPORT NSString *const NSAccessibilityIncrementArrowSubrole;
APPKIT_EXPORT NSString *const NSAccessibilityDecrementArrowSubrole;
APPKIT_EXPORT NSString *const NSAccessibilityIncrementPageSubrole;
APPKIT_EXPORT NSString *const NSAccessibilityDecrementPageSubrole;
APPKIT_EXPORT NSString *const NSAccessibilitySearchFieldSubrole;
APPKIT_EXPORT NSString *const NSAccessibilityTextAttachmentSubrole;
APPKIT_EXPORT NSString *const NSAccessibilityTextLinkSubrole;
APPKIT_EXPORT NSString *const NSAccessibilityTimelineSubrole;
APPKIT_EXPORT NSString *const NSAccessibilitySortButtonSubrole;
APPKIT_EXPORT NSString *const NSAccessibilityRatingIndicatorSubrole;
APPKIT_EXPORT NSString *const NSAccessibilityContentListSubrole;
APPKIT_EXPORT NSString *const NSAccessibilityDefinitionListSubrole;
APPKIT_EXPORT NSString *const NSAccessibilityFullScreenButtonSubrole;
APPKIT_EXPORT NSString *const NSAccessibilityToggleSubrole;
APPKIT_EXPORT NSString *const NSAccessibilitySwitchSubrole;
APPKIT_EXPORT NSString *const NSAccessibilityDescriptionListSubrole;

APPKIT_EXPORT NSString *const NSAccessibilityUIElementsKey;
APPKIT_EXPORT NSString *const NSAccessibilityPriorityKey;
APPKIT_EXPORT NSString *const NSAccessibilityAnnouncementKey;

typedef NSInteger NSAccessibilityPriorityLevel;
enum
{
  NSAccessibilityPriorityLow = 10,
  NSAccessibilityPriorityMedium = 50,
  NSAccessibilityPriorityHigh = 90
};

APPKIT_EXPORT void NSAccessibilityPostNotificationWithUserInfo(
    id element,
    NSString *notification,
    NSDictionary *userInfo);

#endif
