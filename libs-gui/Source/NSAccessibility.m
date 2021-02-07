/** NSAccessibility.m

   <abstract>Contains functions for accessibility functionality</abstract>

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

#import <AppKit/NSAccessibility.h>

NSString *const NSAccessibilityErrorCodeExceptionInfo
  = @"NSAccessibilityErrorCodeExceptionInfo";

NSString *const NSAccessibilityRoleAttribute
  = @"NSAccessibilityRoleAttribute";
NSString *const NSAccessibilityRoleDescriptionAttribute
  = @"NSAccessibilityRoleDescriptionAttribute";
NSString *const NSAccessibilitySubroleAttribute
  = @"NSAccessibilitySubroleAttribute";
NSString *const NSAccessibilityHelpAttribute
  = @"NSAccessibilityHelpAttribute";
NSString *const NSAccessibilityValueAttribute
  = @"NSAccessibilityValueAttribute";
NSString *const NSAccessibilityMinValueAttribute
  = @"NSAccessibilityMinValueAttribute";
NSString *const NSAccessibilityMaxValueAttribute
  = @"NSAccessibilityMaxValueAttribute";
NSString *const NSAccessibilityEnabledAttribute
  = @"NSAccessibilityEnabledAttribute";
NSString *const NSAccessibilityFocusedAttribute
  = @"NSAccessibilityFocusedAttribute";
NSString *const NSAccessibilityParentAttribute
  = @"NSAccessibilityParentAttribute";
NSString *const NSAccessibilityChildrenAttribute
  = @"NSAccessibilityChildrenAttribute";
NSString *const NSAccessibilityWindowAttribute
  = @"NSAccessibilityWindowAttribute";
NSString *const NSAccessibilityTopLevelUIElementAttribute
  = @"NSAccessibilityTopLevelUIElementAttribute";
NSString *const NSAccessibilitySelectedChildrenAttribute
  = @"NSAccessibilitySelectedChildrenAttribute";
NSString *const NSAccessibilityVisibleChildrenAttribute
  = @"NSAccessibilityVisibleChildrenAttribute";
NSString *const NSAccessibilityPositionAttribute
  = @"NSAccessibilityPositionAttribute";
NSString *const NSAccessibilitySizeAttribute
  = @"NSAccessibilitySizeAttribute";
NSString *const NSAccessibilityContentsAttribute
  = @"NSAccessibilityContentsAttribute";
NSString *const NSAccessibilityTitleAttribute
  = @"NSAccessibilityTitleAttribute";
NSString *const NSAccessibilityDescriptionAttribute
  = @"NSAccessibilityDescriptionAttribute";
NSString *const NSAccessibilityShownMenuAttribute
  = @"NSAccessibilityShownMenuAttribute";
NSString *const NSAccessibilityValueDescriptionAttribute
  = @"NSAccessibilityValueDescriptionAttribute";
NSString *const NSAccessibilitySharedFocusElementsAttribute
  = @"NSAccessibilitySharedFocusElementsAttribute";

NSString *const NSAccessibilityPreviousContentsAttribute
  = @"NSAccessibilityPreviousContentsAttribute";
NSString *const NSAccessibilityNextContentsAttribute
  = @"NSAccessibilityNextContentsAttribute";
NSString *const NSAccessibilityHeaderAttribute
  = @"NSAccessibilityHeaderAttribute";
NSString *const NSAccessibilityEditedAttribute
  = @"NSAccessibilityEditedAttribute";
NSString *const NSAccessibilityTabsAttribute
  = @"NSAccessibilityTabsAttribute";
NSString *const NSAccessibilityHorizontalScrollBarAttribute
  = @"NSAccessibilityHorizontalScrollBarAttribute";
NSString *const NSAccessibilityVerticalScrollBarAttribute
  = @"NSAccessibilityVerticalScrollBarAttribute";
NSString *const NSAccessibilityOverflowButtonAttribute
  = @"NSAccessibilityOverflowButtonAttribute";
NSString *const NSAccessibilityIncrementButtonAttribute
  = @"NSAccessibilityIncrementButtonAttribute";
NSString *const NSAccessibilityDecrementButtonAttribute
  = @"NSAccessibilityDecrementButtonAttribute";
NSString *const NSAccessibilityFilenameAttribute
  = @"NSAccessibilityFilenameAttribute";
NSString *const NSAccessibilityExpandedAttribute
  = @"NSAccessibilityExpandedAttribute";
NSString *const NSAccessibilitySelectedAttribute
  = @"NSAccessibilitySelectedAttribute";
NSString *const NSAccessibilitySplittersAttribute
  = @"NSAccessibilitySplittersAttribute";
NSString *const NSAccessibilityDocumentAttribute
  = @"NSAccessibilityDocumentAttribute";
NSString *const NSAccessibilityActivationPointAttribute
  = @"NSAccessibilityActivationPointAttribute";

NSString *const NSAccessibilityURLAttribute
  = @"NSAccessibilityURLAttribute";
NSString *const NSAccessibilityIndexAttribute
  = @"NSAccessibilityIndexAttribute";

NSString *const NSAccessibilityRowCountAttribute
  = @"NSAccessibilityRowCountAttribute";
NSString *const NSAccessibilityColumnCountAttribute
  = @"NSAccessibilityColumnCountAttribute";
NSString *const NSAccessibilityOrderedByRowAttribute
  = @"NSAccessibilityOrderedByRowAttribute";

NSString *const NSAccessibilityWarningValueAttribute
  = @"NSAccessibilityWarningValueAttribute";
NSString *const NSAccessibilityCriticalValueAttribute
  = @"NSAccessibilityCriticalValueAttribute";
NSString *const NSAccessibilityPlaceholderValueAttribute
  = @"NSAccessibilityPlaceholderValueAttribute";
NSString *const NSAccessibilityContainsProtectedContentAttribute
  = @"NSAccessibilityContainsProtectedContentAttribute";
NSString *const NSAccessibilityAlternateUIVisibleAttribute
  = @"NSAccessibilityAlternateUIVisibleAttribute";

NSString *const NSAccessibilityTitleUIElementAttribute
  = @"NSAccessibilityTitleUIElementAttribute";
NSString *const NSAccessibilityServesAsTitleForUIElementsAttribute
  = @"NSAccessibilityServesAsTitleForUIElementsAttribute";
NSString *const NSAccessibilityLinkedUIElementsAttribute
  = @"NSAccessibilityLinkedUIElementsAttribute";

NSString *const NSAccessibilitySelectedTextAttribute
  = @"NSAccessibilitySelectedTextAttribute";
NSString *const NSAccessibilitySelectedTextRangeAttribute
  = @"NSAccessibilitySelectedTextRangeAttribute";
NSString *const NSAccessibilityNumberOfCharactersAttribute
  = @"NSAccessibilityNumberOfCharactersAttribute";
NSString *const NSAccessibilityVisibleCharacterRangeAttribute
  = @"NSAccessibilityVisibleCharacterRangeAttribute";
NSString *const NSAccessibilitySharedTextUIElementsAttribute
  = @"NSAccessibilitySharedTextUIElementsAttribute";
NSString *const NSAccessibilitySharedCharacterRangeAttribute
  = @"NSAccessibilitySharedCharacterRangeAttribute";
NSString *const NSAccessibilityInsertionPointLineNumberAttribute
  = @"NSAccessibilityInsertionPointLineNumberAttribute";
NSString *const NSAccessibilitySelectedTextRangesAttribute
  = @"NSAccessibilitySelectedTextRangesAttribute";

NSString *const NSAccessibilityLineForIndexParameterizedAttribute
  = @"NSAccessibilityLineForIndexParameterizedAttribute";
NSString *const NSAccessibilityRangeForLineParameterizedAttribute
  = @"NSAccessibilityRangeForLineParameterizedAttribute";
NSString *const NSAccessibilityStringForRangeParameterizedAttribute
  = @"NSAccessibilityStringForRangeParameterizedAttribute";
NSString *const NSAccessibilityRangeForPositionParameterizedAttribute
  = @"NSAccessibilityRangeForPositionParameterizedAttribute";
NSString *const NSAccessibilityRangeForIndexParameterizedAttribute
  = @"NSAccessibilityRangeForIndexParameterizedAttribute";
NSString *const NSAccessibilityBoundsForRangeParameterizedAttribute
  = @"NSAccessibilityBoundsForRangeParameterizedAttribute";
NSString *const NSAccessibilityRTFForRangeParameterizedAttribute
  = @"NSAccessibilityRTFForRangeParameterizedAttribute";
NSString *const NSAccessibilityStyleRangeForIndexParameterizedAttribute
  = @"NSAccessibilityStyleRangeForIndexParameterizedAttribute";
NSString *const NSAccessibilityAttributedStringForRangeParameterizedAttribute
  = @"NSAccessibilityAttributedStringForRangeParameterizedAttribute";

NSString *const NSAccessibilityFontTextAttribute
  = @"NSAccessibilityFontTextAttribute";
NSString *const NSAccessibilityForegroundColorTextAttribute
  = @"NSAccessibilityForegroundColorTextAttribute";
NSString *const NSAccessibilityBackgroundColorTextAttribute
  = @"NSAccessibilityBackgroundColorTextAttribute";
NSString *const NSAccessibilityUnderlineColorTextAttribute
  = @"NSAccessibilityUnderlineColorTextAttribute";
NSString *const NSAccessibilityStrikethroughColorTextAttribute
  = @"NSAccessibilityStrikethroughColorTextAttribute";
NSString *const NSAccessibilityUnderlineTextAttribute
  = @"NSAccessibilityUnderlineTextAttribute";
NSString *const NSAccessibilitySuperscriptTextAttribute
  = @"NSAccessibilitySuperscriptTextAttribute";
NSString *const NSAccessibilityStrikethroughTextAttribute
  = @"NSAccessibilityStrikethroughTextAttribute";
NSString *const NSAccessibilityShadowTextAttribute
  = @"NSAccessibilityShadowTextAttribute";
NSString *const NSAccessibilityAttachmentTextAttribute
  = @"NSAccessibilityAttachmentTextAttribute";
NSString *const NSAccessibilityLinkTextAttribute
  = @"NSAccessibilityLinkTextAttribute";
NSString *const NSAccessibilityAutocorrectedTextAttribute
  = @"NSAccessibilityAutocorrectedTextAttribute";

NSString *const NSAccessibilityListItemPrefixTextAttribute
  = @"NSAccessibilityListItemPrefixTextAttribute";
NSString *const NSAccessibilityListItemIndexTextAttribute
  = @"NSAccessibilityListItemIndexTextAttribute";
NSString *const NSAccessibilityListItemLevelTextAttribute
  = @"NSAccessibilityListItemLevelTextAttribute";

NSString *const NSAccessibilityMisspelledTextAttribute
  = @"NSAccessibilityMisspelledTextAttribute";
NSString *const NSAccessibilityMarkedMisspelledTextAttribute
  = @"NSAccessibilityMarkedMisspelledTextAttribute";

NSString *const NSAccessibilityFontNameKey
  = @"NSAccessibilityFontNameKey";
NSString *const NSAccessibilityFontFamilyKey
  = @"NSAccessibilityFontFamilyKey";
NSString *const NSAccessibilityVisibleNameKey
  = @"NSAccessibilityVisibleNameKey";
NSString *const NSAccessibilityFontSizeKey
  = @"NSAccessibilityFontSizeKey";

NSString *const NSAccessibilityMainAttribute
  = @"NSAccessibilityMainAttribute";
NSString *const NSAccessibilityMinimizedAttribute
  = @"NSAccessibilityMinimizedAttribute";
NSString *const NSAccessibilityCloseButtonAttribute
  = @"NSAccessibilityCloseButtonAttribute";
NSString *const NSAccessibilityZoomButtonAttribute
  = @"NSAccessibilityZoomButtonAttribute";
NSString *const NSAccessibilityMinimizeButtonAttribute
  = @"NSAccessibilityMinimizeButtonAttribute";
NSString *const NSAccessibilityToolbarButtonAttribute
  = @"NSAccessibilityToolbarButtonAttribute";
NSString *const NSAccessibilityProxyAttribute
  = @"NSAccessibilityProxyAttribute";
NSString *const NSAccessibilityGrowAreaAttribute
  = @"NSAccessibilityGrowAreaAttribute";
NSString *const NSAccessibilityModalAttribute
  = @"NSAccessibilityModalAttribute";
NSString *const NSAccessibilityDefaultButtonAttribute
  = @"NSAccessibilityDefaultButtonAttribute";
NSString *const NSAccessibilityCancelButtonAttribute
  = @"NSAccessibilityCancelButtonAttribute";
NSString *const NSAccessibilityFullScreenButtonAttribute
  = @"NSAccessibilityFullScreenButtonAttribute";

NSString *const NSAccessibilityMenuBarAttribute
  = @"NSAccessibilityMenuBarAttribute";
NSString *const NSAccessibilityWindowsAttribute
  = @"NSAccessibilityWindowsAttribute";
NSString *const NSAccessibilityFrontmostAttribute
  = @"NSAccessibilityFrontmostAttribute";
NSString *const NSAccessibilityHiddenAttribute
  = @"NSAccessibilityHiddenAttribute";
NSString *const NSAccessibilityMainWindowAttribute
  = @"NSAccessibilityMainWindowAttribute";
NSString *const NSAccessibilityFocusedWindowAttribute
  = @"NSAccessibilityFocusedWindowAttribute";
NSString *const NSAccessibilityFocusedUIElementAttribute
  = @"NSAccessibilityFocusedUIElementAttribute";
NSString *const NSAccessibilityExtrasMenuBarAttribute
  = @"NSAccessibilityExtrasMenuBarAttribute";

NSString *const NSAccessibilityOrientationAttribute
  = @"NSAccessibilityOrientationAttribute";
NSString *const NSAccessibilityVerticalOrientationValue
  = @"NSAccessibilityVerticalOrientationValue";
NSString *const NSAccessibilityHorizontalOrientationValue
  = @"NSAccessibilityHorizontalOrientationValue";
NSString *const NSAccessibilityUnknownOrientationValue
  = @"NSAccessibilityUnknownOrientationValue";

NSString *const NSAccessibilityColumnTitlesAttribute
  = @"NSAccessibilityColumnTitlesAttribute";

NSString *const NSAccessibilitySearchButtonAttribute
  = @"NSAccessibilitySearchButtonAttribute";
NSString *const NSAccessibilitySearchMenuAttribute
  = @"NSAccessibilitySearchMenuAttribute";
NSString *const NSAccessibilityClearButtonAttribute
  = @"NSAccessibilityClearButtonAttribute";

NSString *const NSAccessibilityRowsAttribute
  = @"NSAccessibilityRowsAttribute";
NSString *const NSAccessibilityVisibleRowsAttribute
  = @"NSAccessibilityVisibleRowsAttribute";
NSString *const NSAccessibilitySelectedRowsAttribute
  = @"NSAccessibilitySelectedRowsAttribute";
NSString *const NSAccessibilityColumnsAttribute
  = @"NSAccessibilityColumnsAttribute";
NSString *const NSAccessibilityVisibleColumnsAttribute
  = @"NSAccessibilityVisibleColumnsAttribute";
NSString *const NSAccessibilitySelectedColumnsAttribute
  = @"NSAccessibilitySelectedColumnsAttribute";
NSString *const NSAccessibilitySortDirectionAttribute
  = @"NSAccessibilitySortDirectionAttribute";

NSString *const NSAccessibilitySelectedCellsAttribute
  = @"NSAccessibilitySelectedCellsAttribute";
NSString *const NSAccessibilityVisibleCellsAttribute
  = @"NSAccessibilityVisibleCellsAttribute";
NSString *const NSAccessibilityRowHeaderUIElementsAttribute
  = @"NSAccessibilityRowHeaderUIElementsAttribute";
NSString *const NSAccessibilityColumnHeaderUIElementsAttribute
  = @"NSAccessibilityColumnHeaderUIElementsAttribute";

NSString *const NSAccessibilityCellForColumnAndRowParameterizedAttribute
  = @"NSAccessibilityCellForColumnAndRowParameterizedAttribute";

NSString *const NSAccessibilityRowIndexRangeAttribute
  = @"NSAccessibilityRowIndexRangeAttribute";
NSString *const NSAccessibilityColumnIndexRangeAttribute
  = @"NSAccessibilityColumnIndexRangeAttribute";

NSString *const NSAccessibilityHorizontalUnitsAttribute
  = @"NSAccessibilityHorizontalUnitsAttribute";
NSString *const NSAccessibilityVerticalUnitsAttribute
  = @"NSAccessibilityVerticalUnitsAttribute";
NSString *const NSAccessibilityHorizontalUnitDescriptionAttribute
  = @"NSAccessibilityHorizontalUnitDescriptionAttribute";
NSString *const NSAccessibilityVerticalUnitDescriptionAttribute
  = @"NSAccessibilityVerticalUnitDescriptionAttribute";

NSString *const NSAccessibilityLayoutPointForScreenPointParameterizedAttribute
  = @"NSAccessibilityLayoutPointForScreenPointParameterizedAttribute";
NSString *const NSAccessibilityLayoutSizeForScreenSizeParameterizedAttribute
  = @"NSAccessibilityLayoutSizeForScreenSizeParameterizedAttribute";
NSString *const NSAccessibilityScreenPointForLayoutPointParameterizedAttribute
  = @"NSAccessibilityScreenPointForLayoutPointParameterizedAttribute";
NSString *const NSAccessibilityScreenSizeForLayoutSizeParameterizedAttribute
  = @"NSAccessibilityScreenSizeForLayoutSizeParameterizedAttribute";

NSString *const NSAccessibilityHandlesAttribute
  = @"NSAccessibilityHandlesAttribute";

NSString *const NSAccessibilityAscendingSortDirectionValue
  = @"NSAccessibilityAscendingSortDirectionValue";
NSString *const NSAccessibilityDescendingSortDirectionValue
  = @"NSAccessibilityDescendingSortDirectionValue";
NSString *const NSAccessibilityUnknownSortDirectionValue
  = @"NSAccessibilityUnknownSortDirectionValue";

NSString *const NSAccessibilityDisclosingAttribute
  = @"NSAccessibilityDisclosingAttribute";
NSString *const NSAccessibilityDisclosedRowsAttribute
  = @"NSAccessibilityDisclosedRowsAttribute";
NSString *const NSAccessibilityDisclosedByRowAttribute
  = @"NSAccessibilityDisclosedByRowAttribute";
NSString *const NSAccessibilityDisclosureLevelAttribute
  = @"NSAccessibilityDisclosureLevelAttribute";

NSString *const NSAccessibilityAllowedValuesAttribute
  = @"NSAccessibilityAllowedValuesAttribute";
NSString *const NSAccessibilityLabelUIElementsAttribute
  = @"NSAccessibilityLabelUIElementsAttribute";
NSString *const NSAccessibilityLabelValueAttribute
  = @"NSAccessibilityLabelValueAttribute";

NSString *const NSAccessibilityMatteHoleAttribute
  = @"NSAccessibilityMatteHoleAttribute";
NSString *const NSAccessibilityMatteContentUIElementAttribute
  = @"NSAccessibilityMatteContentUIElementAttribute";

NSString *const NSAccessibilityMarkerUIElementsAttribute
  = @"NSAccessibilityMarkerUIElementsAttribute";
NSString *const NSAccessibilityMarkerValuesAttribute
  = @"NSAccessibilityMarkerValuesAttribute";
NSString *const NSAccessibilityMarkerGroupUIElementAttribute
  = @"NSAccessibilityMarkerGroupUIElementAttribute";
NSString *const NSAccessibilityUnitsAttribute
  = @"NSAccessibilityUnitsAttribute";
NSString *const NSAccessibilityUnitDescriptionAttribute
  = @"NSAccessibilityUnitDescriptionAttribute";
NSString *const NSAccessibilityMarkerTypeAttribute
  = @"NSAccessibilityMarkerTypeAttribute";
NSString *const NSAccessibilityMarkerTypeDescriptionAttribute
  = @"NSAccessibilityMarkerTypeDescriptionAttribute";

NSString *const NSAccessibilityIdentifierAttribute
  = @"NSAccessibilityIdentifierAttribute";

NSString *const NSAccessibilityLeftTabStopMarkerTypeValue
  = @"NSAccessibilityLeftTabStopMarkerTypeValue";
NSString *const NSAccessibilityRightTabStopMarkerTypeValue
  = @"NSAccessibilityRightTabStopMarkerTypeValue";
NSString *const NSAccessibilityCenterTabStopMarkerTypeValue
  = @"NSAccessibilityCenterTabStopMarkerTypeValue";
NSString *const NSAccessibilityDecimalTabStopMarkerTypeValue
  = @"NSAccessibilityDecimalTabStopMarkerTypeValue";
NSString *const NSAccessibilityHeadIndentMarkerTypeValue
  = @"NSAccessibilityHeadIndentMarkerTypeValue";
NSString *const NSAccessibilityTailIndentMarkerTypeValue
  = @"NSAccessibilityTailIndentMarkerTypeValue";
NSString *const NSAccessibilityFirstLineIndentMarkerTypeValue
  = @"NSAccessibilityFirstLineIndentMarkerTypeValue";
NSString *const NSAccessibilityUnknownMarkerTypeValue
  = @"NSAccessibilityUnknownMarkerTypeValue";

NSString *const NSAccessibilityInchesUnitValue
  = @"NSAccessibilityInchesUnitValue";
NSString *const NSAccessibilityCentimetersUnitValue
  = @"NSAccessibilityCentimetersUnitValue";
NSString *const NSAccessibilityPointsUnitValue
  = @"NSAccessibilityPointsUnitValue";
NSString *const NSAccessibilityPicasUnitValue
  = @"NSAccessibilityPicasUnitValue";
NSString *const NSAccessibilityUnknownUnitValue
  = @"NSAccessibilityUnknownUnitValue";

NSString *const NSAccessibilityPressAction
  = @"NSAccessibilityPressAction";
NSString *const NSAccessibilityIncrementAction
  = @"NSAccessibilityIncrementAction";
NSString *const NSAccessibilityDecrementAction
  = @"NSAccessibilityDecrementAction";
NSString *const NSAccessibilityConfirmAction
  = @"NSAccessibilityConfirmAction";
NSString *const NSAccessibilityPickAction
  = @"NSAccessibilityPickAction";
NSString *const NSAccessibilityCancelAction
  = @"NSAccessibilityCancelAction";
NSString *const NSAccessibilityRaiseAction
  = @"NSAccessibilityRaiseAction";
NSString *const NSAccessibilityShowMenuAction
  = @"NSAccessibilityShowMenuAction";
NSString *const NSAccessibilityDeleteAction
  = @"NSAccessibilityDeleteAction";

NSString *const NSAccessibilityShowAlternateUIAction
  = @"NSAccessibilityShowAlternateUIAction";
NSString *const NSAccessibilityShowDefaultUIAction
  = @"NSAccessibilityShowDefaultUIAction";

NSString *const NSAccessibilityMainWindowChangedNotification
  = @"NSAccessibilityMainWindowChangedNotification";
NSString *const NSAccessibilityFocusedWindowChangedNotification
  = @"NSAccessibilityFocusedWindowChangedNotification";
NSString *const NSAccessibilityFocusedUIElementChangedNotification
  = @"NSAccessibilityFocusedUIElementChangedNotification";

NSString *const NSAccessibilityApplicationActivatedNotification
  = @"NSAccessibilityApplicationActivatedNotification";
NSString *const NSAccessibilityApplicationDeactivatedNotification
  = @"NSAccessibilityApplicationDeactivatedNotification";
NSString *const NSAccessibilityApplicationHiddenNotification
  = @"NSAccessibilityApplicationHiddenNotification";
NSString *const NSAccessibilityApplicationShownNotification
  = @"NSAccessibilityApplicationShownNotification";

NSString *const NSAccessibilityWindowCreatedNotification
  = @"NSAccessibilityWindowCreatedNotification";
NSString *const NSAccessibilityWindowMovedNotification
  = @"NSAccessibilityWindowMovedNotification";
NSString *const NSAccessibilityWindowResizedNotification
  = @"NSAccessibilityWindowResizedNotification";
NSString *const NSAccessibilityWindowMiniaturizedNotification
  = @"NSAccessibilityWindowMiniaturizedNotification";
NSString *const NSAccessibilityWindowDeminiaturizedNotification
  = @"NSAccessibilityWindowDeminiaturizedNotification";

NSString *const NSAccessibilityDrawerCreatedNotification
  = @"NSAccessibilityDrawerCreatedNotification";
NSString *const NSAccessibilitySheetCreatedNotification
  = @"NSAccessibilitySheetCreatedNotification";

NSString *const NSAccessibilityUIElementDestroyedNotification
  = @"NSAccessibilityUIElementDestroyedNotification";
NSString *const NSAccessibilityValueChangedNotification
  = @"NSAccessibilityValueChangedNotification";
NSString *const NSAccessibilityTitleChangedNotification
  = @"NSAccessibilityTitleChangedNotification";
NSString *const NSAccessibilityResizedNotification
  = @"NSAccessibilityResizedNotification";
NSString *const NSAccessibilityMovedNotification
  = @"NSAccessibilityMovedNotification";
NSString *const NSAccessibilityCreatedNotification
  = @"NSAccessibilityCreatedNotification";

NSString *const NSAccessibilityLayoutChangedNotification
  = @"NSAccessibilityLayoutChangedNotification";

NSString *const NSAccessibilityHelpTagCreatedNotification
  = @"NSAccessibilityHelpTagCreatedNotification";
NSString *const NSAccessibilitySelectedTextChangedNotification
  = @"NSAccessibilitySelectedTextChangedNotification";
NSString *const NSAccessibilityRowCountChangedNotification
  = @"NSAccessibilityRowCountChangedNotification";
NSString *const NSAccessibilitySelectedChildrenChangedNotification
  = @"NSAccessibilitySelectedChildrenChangedNotification";
NSString *const NSAccessibilitySelectedRowsChangedNotification
  = @"NSAccessibilitySelectedRowsChangedNotification";
NSString *const NSAccessibilitySelectedColumnsChangedNotification
  = @"NSAccessibilitySelectedColumnsChangedNotification";

NSString *const NSAccessibilityRowExpandedNotification
  = @"NSAccessibilityRowExpandedNotification";
NSString *const NSAccessibilityRowCollapsedNotification
  = @"NSAccessibilityRowCollapsedNotification";

NSString *const NSAccessibilitySelectedCellsChangedNotification
  = @"NSAccessibilitySelectedCellsChangedNotification";

NSString *const NSAccessibilityUnitsChangedNotification
  = @"NSAccessibilityUnitsChangedNotification";
NSString *const NSAccessibilitySelectedChildrenMovedNotification
  = @"NSAccessibilitySelectedChildrenMovedNotification";

NSString *const NSAccessibilityAnnouncementRequestedNotification
  = @"NSAccessibilityAnnouncementRequestedNotification";

NSString *const NSAccessibilityUnknownRole
  = @"NSAccessibilityUnknownRole";
NSString *const NSAccessibilityButtonRole
  = @"NSAccessibilityButtonRole";
NSString *const NSAccessibilityRadioButtonRole
  = @"NSAccessibilityRadioButtonRole";
NSString *const NSAccessibilityCheckBoxRole
  = @"NSAccessibilityCheckBoxRole";
NSString *const NSAccessibilitySliderRole
  = @"NSAccessibilitySliderRole";
NSString *const NSAccessibilityTabGroupRole
  = @"NSAccessibilityTabGroupRole";
NSString *const NSAccessibilityTextFieldRole
  = @"NSAccessibilityTextFieldRole";
NSString *const NSAccessibilityStaticTextRole
  = @"NSAccessibilityStaticTextRole";
NSString *const NSAccessibilityTextAreaRole
  = @"NSAccessibilityTextAreaRole";
NSString *const NSAccessibilityScrollAreaRole
  = @"NSAccessibilityScrollAreaRole";
NSString *const NSAccessibilityPopUpButtonRole
  = @"NSAccessibilityPopUpButtonRole";
NSString *const NSAccessibilityMenuButtonRole
  = @"NSAccessibilityMenuButtonRole";
NSString *const NSAccessibilityTableRole
  = @"NSAccessibilityTableRole";
NSString *const NSAccessibilityApplicationRole
  = @"NSAccessibilityApplicationRole";
NSString *const NSAccessibilityGroupRole
  = @"NSAccessibilityGroupRole";
NSString *const NSAccessibilityRadioGroupRole
  = @"NSAccessibilityRadioGroupRole";
NSString *const NSAccessibilityListRole
  = @"NSAccessibilityListRole";
NSString *const NSAccessibilityScrollBarRole
  = @"NSAccessibilityScrollBarRole";
NSString *const NSAccessibilityValueIndicatorRole
  = @"NSAccessibilityValueIndicatorRole";
NSString *const NSAccessibilityImageRole
  = @"NSAccessibilityImageRole";
NSString *const NSAccessibilityMenuBarRole
  = @"NSAccessibilityMenuBarRole";
NSString *const NSAccessibilityMenuRole
  = @"NSAccessibilityMenuRole";
NSString *const NSAccessibilityMenuItemRole
  = @"NSAccessibilityMenuItemRole";
NSString *const NSAccessibilityColumnRole
  = @"NSAccessibilityColumnRole";
NSString *const NSAccessibilityRowRole
  = @"NSAccessibilityRowRole";
NSString *const NSAccessibilityToolbarRole
  = @"NSAccessibilityToolbarRole";
NSString *const NSAccessibilityBusyIndicatorRole
  = @"NSAccessibilityBusyIndicatorRole";
NSString *const NSAccessibilityProgressIndicatorRole
  = @"NSAccessibilityProgressIndicatorRole";
NSString *const NSAccessibilityWindowRole
  = @"NSAccessibilityWindowRole";
NSString *const NSAccessibilityDrawerRole
  = @"NSAccessibilityDrawerRole";
NSString *const NSAccessibilitySystemWideRole
  = @"NSAccessibilitySystemWideRole";
NSString *const NSAccessibilityOutlineRole
  = @"NSAccessibilityOutlineRole";
NSString *const NSAccessibilityIncrementorRole
  = @"NSAccessibilityIncrementorRole";
NSString *const NSAccessibilityBrowserRole
  = @"NSAccessibilityBrowserRole";
NSString *const NSAccessibilityComboBoxRole
  = @"NSAccessibilityComboBoxRole";
NSString *const NSAccessibilitySplitGroupRole
  = @"NSAccessibilitySplitGroupRole";
NSString *const NSAccessibilitySplitterRole
  = @"NSAccessibilitySplitterRole";
NSString *const NSAccessibilityColorWellRole
  = @"NSAccessibilityColorWellRole";
NSString *const NSAccessibilityGrowAreaRole
  = @"NSAccessibilityGrowAreaRole";
NSString *const NSAccessibilitySheetRole
  = @"NSAccessibilitySheetRole";
NSString *const NSAccessibilityHelpTagRole
  = @"NSAccessibilityHelpTagRole";
NSString *const NSAccessibilityMatteRole
  = @"NSAccessibilityMatteRole";
NSString *const NSAccessibilityRulerRole
  = @"NSAccessibilityRulerRole";
NSString *const NSAccessibilityRulerMarkerRole
  = @"NSAccessibilityRulerMarkerRole";
NSString *const NSAccessibilityLinkRole
  = @"NSAccessibilityLinkRole";
NSString *const NSAccessibilityDisclosureTriangleRole
  = @"NSAccessibilityDisclosureTriangleRole";
NSString *const NSAccessibilityGridRole
  = @"NSAccessibilityGridRole";
NSString *const NSAccessibilityRelevanceIndicatorRole
  = @"NSAccessibilityRelevanceIndicatorRole";
NSString *const NSAccessibilityLevelIndicatorRole
  = @"NSAccessibilityLevelIndicatorRole";
NSString *const NSAccessibilityCellRole
  = @"NSAccessibilityCellRole";
NSString *const NSAccessibilityPopoverRole
  = @"NSAccessibilityPopoverRole";
NSString *const NSAccessibilitySortButtonRole
  = @"NSAccessibilitySortButtonRole";

NSString *const NSAccessibilityLayoutAreaRole
  = @"NSAccessibilityLayoutAreaRole";
NSString *const NSAccessibilityLayoutItemRole
  = @"NSAccessibilityLayoutItemRole";
NSString *const NSAccessibilityHandleRole
  = @"NSAccessibilityHandleRole";

NSString *const NSAccessibilityUnknownSubrole
  = @"NSAccessibilityUnknownSubrole";
NSString *const NSAccessibilityCloseButtonSubrole
  = @"NSAccessibilityCloseButtonSubrole";
NSString *const NSAccessibilityZoomButtonSubrole
  = @"NSAccessibilityZoomButtonSubrole";
NSString *const NSAccessibilityMinimizeButtonSubrole
  = @"NSAccessibilityMinimizeButtonSubrole";
NSString *const NSAccessibilityToolbarButtonSubrole
  = @"NSAccessibilityToolbarButtonSubrole";
NSString *const NSAccessibilityTableRowSubrole
  = @"NSAccessibilityTableRowSubrole";
NSString *const NSAccessibilityOutlineRowSubrole
  = @"NSAccessibilityOutlineRowSubrole";
NSString *const NSAccessibilitySecureTextFieldSubrole
  = @"NSAccessibilitySecureTextFieldSubrole";
NSString *const NSAccessibilityStandardWindowSubrole
  = @"NSAccessibilityStandardWindowSubrole";
NSString *const NSAccessibilityDialogSubrole
  = @"NSAccessibilityDialogSubrole";
NSString *const NSAccessibilitySystemDialogSubrole
  = @"NSAccessibilitySystemDialogSubrole";
NSString *const NSAccessibilityFloatingWindowSubrole
  = @"NSAccessibilityFloatingWindowSubrole";
NSString *const NSAccessibilitySystemFloatingWindowSubrole
  = @"NSAccessibilitySystemFloatingWindowSubrole";
NSString *const NSAccessibilityIncrementArrowSubrole
  = @"NSAccessibilityIncrementArrowSubrole";
NSString *const NSAccessibilityDecrementArrowSubrole
  = @"NSAccessibilityDecrementArrowSubrole";
NSString *const NSAccessibilityIncrementPageSubrole
  = @"NSAccessibilityIncrementPageSubrole";
NSString *const NSAccessibilityDecrementPageSubrole
  = @"NSAccessibilityDecrementPageSubrole";
NSString *const NSAccessibilitySearchFieldSubrole
  = @"NSAccessibilitySearchFieldSubrole";
NSString *const NSAccessibilityTextAttachmentSubrole
  = @"NSAccessibilityTextAttachmentSubrole";
NSString *const NSAccessibilityTextLinkSubrole
  = @"NSAccessibilityTextLinkSubrole";
NSString *const NSAccessibilityTimelineSubrole
  = @"NSAccessibilityTimelineSubrole";
NSString *const NSAccessibilitySortButtonSubrole
  = @"NSAccessibilitySortButtonSubrole";
NSString *const NSAccessibilityRatingIndicatorSubrole
  = @"NSAccessibilityRatingIndicatorSubrole";
NSString *const NSAccessibilityContentListSubrole
  = @"NSAccessibilityContentListSubrole";
NSString *const NSAccessibilityDefinitionListSubrole
  = @"NSAccessibilityDefinitionListSubrole";
NSString *const NSAccessibilityFullScreenButtonSubrole
  = @"NSAccessibilityFullScreenButtonSubrole";
NSString *const NSAccessibilityToggleSubrole
  = @"NSAccessibilityToggleSubrole";
NSString *const NSAccessibilitySwitchSubrole
  = @"NSAccessibilitySwitchSubrole";
NSString *const NSAccessibilityDescriptionListSubrole
  = @"NSAccessibilityDescriptionListSubrole";

NSString *const NSAccessibilityUIElementsKey
  = @"NSAccessibilityUIElementsKey";
NSString *const NSAccessibilityPriorityKey
  = @"NSAccessibilityPriorityKey";
NSString *const NSAccessibilityAnnouncementKey
  = @"NSAccessibilityAnnouncementKey";


void NSAccessibilityPostNotification(
  id element,
  NSString *notification)
{
  NSAccessibilityPostNotificationWithUserInfo(element, notification, nil);
}

void NSAccessibilityPostNotificationWithUserInfo(
  id element,
  NSString *notification,
  NSDictionary *userInfo)
{
  // FIXME 
}

id NSAccessibilityUnignoredAncestor(id element)
{
  return nil;
}

id NSAccessibilityUnignoredDescendant(id element)
{
  return nil;
}

NSArray *NSAccessibilityUnignoredChildren(
  NSArray *originalChildren)
{
  return nil;
}

NSArray *NSAccessibilityUnignoredChildrenForOnlyChild(
  id originalChild)
{
  return nil;
}

NSString *NSAccessibilityRoleDescription(
  NSString *role,
  NSString *subrole)
{
  return nil;
}

NSString *NSAccessibilityRoleDescriptionForUIElement(id element)
{
  return nil;
}

NSString *NSAccessibilityActionDescription(NSString *action)
{
  return nil;
}
