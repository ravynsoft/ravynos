#
#  Main makefile for GNUstep GUI Library
#  Copyright (C) 1997 Free Software Foundation, Inc.
#
#  Author: Scott Christley <scottc@net-community.com>
#
#  This file is part of the GNUstep GUI Library.
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2 of the License, or (at your option) any later version.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; see the file COPYING.LIB.
#  If not, see <http://www.gnu.org/licenses/> or write to the 
#  Free Software Foundation, 51 Franklin Street, Fifth Floor, 
#  Boston, MA 02110-1301, USA.

PACKAGE_NAME = gnustep-gui
GNUSTEP_LOCAL_ADDITIONAL_MAKEFILES=../gui.make
include $(GNUSTEP_MAKEFILES)/common.make

include ../Version
include ../config.make

# Interface version changes with each minor release
libgnustep-gui_INTERFACE_VERSION=${GNUSTEP_GUI_MAJOR_VERSION}.${GNUSTEP_GUI_MINOR_VERSION}

srcdir = .
PACKAGE_NAME = gnustep-gui
LIBRARY_VAR = GNUSTEP_GUI

# The library to be compiled
LIBRARY_NAME = libgnustep-gui

# The Objective-C source files to be compiled
libgnustep-gui_C_FILES =

# The Objective-C source files to be compiled
libgnustep-gui_OBJC_FILES = Functions.m \
NSAccessibility.m \
NSAccessibilityCustomAction.m \
NSAccessibilityCustomRotor.m \
NSAccessibilityElement.m \
NSActionCell.m \
NSAffineTransform.m \
NSAlert.m \
NSAnimation.m \
NSAnimationContext.m \
NSAppearance.m \
NSApplication.m \
NSArrayController.m \
NSAttributedString.m \
NSBezierPath.m \
NSBitmapImageRep.m \
NSBitmapImageRep+GIF.m \
NSBitmapImageRep+ICNS.m \
NSBitmapImageRep+JPEG.m \
NSBitmapImageRep+PNG.m \
NSBitmapImageRep+PNM.m \
NSBox.m \
NSBrowser.m \
NSBrowserCell.m \
NSBundleAdditions.m \
NSButton.m \
NSButtonTouchBarItem.m \
NSButtonCell.m \
NSCachedImageRep.m \
NSCandidateListTouchBarItem.m \
NSCell.m \
NSCIImageRep.m \
NSClipView.m \
NSClickGestureRecognizer.m \
NSCollectionView.m \
NSCollectionViewItem.m \
NSColor.m \
NSColorList.m \
NSColorPanel.m \
NSColorPicker.m \
NSColorPickerTouchBarItem.m \
NSColorSampler.m \
NSColorSpace.m \
NSColorWell.m \
NSComboBox.m \
NSComboBoxCell.m \
NSControl.m \
NSController.m \
NSCursor.m \
NSCustomImageRep.m \
NSCustomTouchBarItem.m \
NSDataAsset.m \
NSDataLink.m \
NSDataLinkManager.m \
NSDataLinkPanel.m \
NSDatePicker.m \
NSDatePickerCell.m \
NSDockTile.m \
NSDocument.m \
NSDocumentController.m \
NSDrawer.m \
NSEPSImageRep.m \
NSEvent.m \
NSFileWrapperExtensions.m \
NSFont.m \
NSFontAssetRequest.m \
NSFontCollection.m \
NSFontDescriptor.m \
NSFontManager.m \
NSFontPanel.m \
NSForm.m \
NSFormCell.m \
NSGroupTouchBarItem.m \
NSMagnificationGestureRecognizer.m \
NSPanGestureRecognizer.m \
NSPathCell.m \
NSPathComponentCell.m \
NSPathControl.m \
NSPathControlItem.m \
NSPickerTouchBarItem.m \
NSPopoverTouchBarItem.m \
NSPressGestureRecognizer.m \
NSRotationGestureRecognizer.m \
NSSharingServicePickerTouchBarItem.m \
NSSlider.m \
NSSliderCell.m \
NSSliderAccessory.m \
NSSliderTouchBarItem.m \
NSStepperTouchBarItem.m \
NSSwitch.m \
NSTitlebarAccessoryViewController.m \
NSTouch.m \
NSTouchBar.m \
NSTouchBarItem.m \
NSGestureRecognizer.m \
NSGradient.m \
NSGlyphGenerator.m \
NSGraphicsContext.m \
NSGroupTouchBarItem.m \
NSHelpPanel.m \
NSHelpManager.m \
NSImage.m \
NSImageCell.m \
NSImageRep.m \
NSImageView.m \
NSInputManager.m \
NSInputServer.m \
NSInterfaceStyle.m \
NSKeyValueBinding.m \
NSLayoutAnchor.m \
NSLayoutConstraint.m \
NSLayoutGuide.m \
NSLayoutManager.m \
NSLevelIndicator.m \
NSLevelIndicatorCell.m \
NSMagnificationGestureRecognizer.m \
NSMediaLibraryBrowserController.m \
NSPanGestureRecognizer.m \
NSPickerTouchBarItem.m \
NSPopoverTouchBarItem.m \
NSPressGestureRecognizer.m \
NSRotationGestureRecognizer.m \
NSSharingServicePickerToolbarItem.m \
NSSharingServicePickerTouchBarItem.m \
NSStepperTouchBarItem.m \
NSStoryboard.m \
NSStoryboardSegue.m \
NSMagnificationGestureRecognizer.m \
NSMatrix.m \
NSMenu.m \
NSMenuView.m \
NSMenuItem.m \
NSMenuItemCell.m \
NSMovie.m \
NSMovieView.m \
NSNib.m \
NSNibConnector.m \
NSNibControlConnector.m \
NSNibOutletConnector.m \
NSNibBindingConnector.m \
NSNibAXAttributeConnector.m \
NSNibAXRelationshipConnector.m \
NSObjectController.m \
NSOpenGLContext.m \
NSOpenGLPixelFormat.m \
NSOpenGLView.m \
NSOpenPanel.m \
NSOutlineView.m \
NSPageController.m \
NSPageLayout.m \
NSPanel.m \
NSPanGestureRecognizer.m \
NSPickerTouchBarItem.m \
NSPopoverTouchBarItem.m \
NSParagraphStyle.m \
NSPasteboardItem.m \
NSPDFInfo.m \
NSPDFImageRep.m \
NSPDFPanel.m \
NSPersistentDocument.m \
NSPICTImageRep.m \
NSPopover.m \
NSPopUpButton.m \
NSPopUpButtonCell.m \
NSPredicateEditor.m \
NSPredicateEditorRowTemplate.m \
NSPressGestureRecognizer.m \
NSPrinter.m \
NSPrintInfo.m \
NSPrintOperation.m \
NSPrintPanel.m \
NSProgressIndicator.m \
NSResponder.m \
NSRotationGestureRecognizer.m \
NSRuleEditor.m \
NSRulerMarker.m \
NSRulerView.m \
NSRunningApplication.m \
NSSavePanel.m \
NSSearchField.m \
NSSearchFieldCell.m \
NSSegmentedCell.m \
NSSegmentedControl.m \
NSScreen.m \
NSScroller.m \
NSScrollView.m \
NSScrubber.m \
NSScrubberItemView.m \
NSScrubberLayout.m \
NSSecureTextField.m \
NSSelection.m \
NSShadow.m \
NSSharingService.m \
NSSharingServicePickerTouchBarItem.m \
NSSound.m  \
NSSpeechRecognizer.m \
NSSpeechSynthesizer.m \
NSStepperTouchBarItem.m \
NSSpellChecker.m \
NSSplitView.m \
NSSplitViewController.m \
NSSplitViewItem.m \
NSStepper.m \
NSStepperCell.m \
NSStringDrawing.m \
NSStatusBar.m \
NSStatusBarButton.m \
NSStatusItem.m \
NSTabView.m \
NSTabViewController.m \
NSTabViewItem.m \
NSTableColumn.m \
NSTableHeaderView.m \
NSTableHeaderCell.m \
NSTableView.m \
NSText.m \
NSTextAlternatives.m \
NSTextAttachment.m \
NSTextBlock.m \
NSTextContainer.m \
NSTextCheckingController.m \
NSTextField.m \
NSTextFieldCell.m \
NSTextFinder.m \
NSTextInputContext.m \
NSTextList.m \
NSTextStorage.m \
NSTextTable.m \
NSTextTableBlock.m \
NSTextView.m \
NSTextView_actions.m \
NSToolbar.m \
NSToolbarItem.m \
NSToolbarItemGroup.m \
NSTokenField.m \
NSTokenFieldCell.m \
NSTrackingArea.m \
NSTreeController.m \
NSTreeNode.m \
NSUserDefaultsController.m \
NSUserInterfaceCompression.m \
NSUserInterfaceItemSearching.m \
NSView.m \
NSViewController.m \
NSVisualEffectView.m \
NSWindow.m \
NSWindowController.m \
NSWorkspace.m \
GSAnimator.m \
GSAutocompleteWindow.m \
GSDisplayServer.m \
GSHelpManagerPanel.m \
GSInfoPanel.m \
GSMemoryPanel.m \
GSSlideView.m \
GSTextStorage.m \
GSTrackingRect.m \
GSServicesManager.m \
tiff.m \
externs.m \
linking.m \
GSCharacterPanel.m \
GSDragView.m \
GSFontInfo.m \
GSTable.m \
GSHbox.m \
GSVbox.m \
GSKeyBindingAction.m \
GSKeyBindingTable.m \
GSTextFinder.m \
GSLayoutManager.m \
GSTypesetter.m \
GSHorizontalTypesetter.m \
GSGormLoading.m \
GSIconManager.m \
GSImageMagickImageRep.m \
GSNibLoading.m \
GSTheme.m \
GSThemeDrawing.m \
GSThemeInspector.m \
GSThemeMenu.m \
GSThemeOpenSavePanels.m \
GSThemePanel.m \
GSThemePrintPanels.m \
GSThemeTools.m \
GSThemeWindow.m \
GSTitleView.m \
GSToolTips.m \
GSToolbarView.m \
GSToolbarCustomizationPalette.m \
GSStandardWindowDecorationView.m \
GSStoryboardTransform.m \
GSWindowDecorationView.m \
GSPrinting.m \
GSPrintOperation.m \
GSEPSPrintOperation.m \
GSPDFPrintOperation.m \
GSModelLoaderFactory.m \
GSGormLoader.m \
GSGModelLoader.m \
GSNibLoader.m \
GSXibElement.m \
GSXibLoader.m \
GSXibLoading.m \
GSXibKeyedUnarchiver.m \
GSXib5KeyedUnarchiver.m \
GSXibObjectContainer.m \
GSHelpAttachment.m 

# Turn off NSMenuItem warning that NSMenuItem conforms to <NSObject>,
# but does not implement <NSObject>'s methods itself (it inherits
# their implementation)
NSMenuItem.m_FILE_FLAGS = -Wno-protocol

ifneq ($(FOUNDATION_LIB), fd)
libgnustep-gui_OBJC_FILES += NSPasteboard.m
endif

libgnustep-gui_HEADER_FILES_DIR = ../Headers/Additions/GNUstepGUI
libgnustep-gui_HEADER_FILES_INSTALL_DIR = /GNUstepGUI

COCOA_HEADERS = \
Cocoa.h 

APPKIT_HEADERS = \
AppKit.h \
AppKitDefines.h \
AppKitErrors.h \
AppKitExceptions.h \
NSAccessibility.h \
NSAccessibilityConstants.h \
NSAccessibilityCustomAction.h \
NSAccessibilityCustomRotor.h \
NSAccessibilityElement.h \
NSAccessibilityProtocols.h \
NSActionCell.h \
NSAffineTransform.h \
NSAlert.h \
NSAnimation.h \
NSAnimationContext.h \
NSAppearance.h \
NSApplication.h \
NSArrayController.h \
NSBezierPath.h \
NSBox.h \
NSBitmapImageRep.h \
NSBrowser.h \
NSBrowserCell.h \
NSButton.h \
NSButtonCell.h \
NSButtonTouchBarItem.h \
NSCachedImageRep.h \
NSCandidateListTouchBarItem.h \
NSCell.h \
NSCIImageRep.h \
NSClickGestureRecognizer.h \
NSClipView.h \
NSCollectionView.h \
NSCollectionViewItem.h \
NSColor.h \
NSColorList.h \
NSColorPanel.h \
NSColorPicker.h \
NSColorPickerTouchBarItem.h \
NSColorSampler.h \
NSColorSpace.h \
NSColorWell.h \
NSComboBox.h \
NSComboBoxCell.h \
NSControl.h \
NSController.h \
NSCursor.h \
NSCustomImageRep.h \
NSCustomTouchBarItem.h \
NSDataAsset.h \
NSDataLink.h \
NSDataLinkManager.h \
NSDataLinkPanel.h \
NSDatePicker.h \
NSDatePickerCell.h \
NSDockTile.h \
NSDocument.h \
NSDocumentController.h \
NSDrawer.h \
NSEPSImageRep.h \
NSErrors.h \
NSEvent.h \
NSFileWrapper.h \
NSFileWrapperExtensions.h \
NSFont.h \
NSFontAssetRequest.h \
NSFontCollection.h \
NSFontDescriptor.h \
NSFontManager.h \
NSFontPanel.h \
NSForm.h \
NSFormCell.h \
NSGestureRecognizer.h \
NSGradient.h \
NSGroupTouchBarItem.h \
NSGlyphGenerator.h \
NSGraphicsContext.h \
NSHelpPanel.h \
NSHelpManager.h \
NSImage.h \
NSImageCell.h \
NSImageRep.h \
NSImageView.h \
NSInputManager.h \
NSInputServer.h \
NSInterfaceStyle.h \
NSKeyValueBinding.h \
NSLayoutAnchor.h \
NSLayoutConstraint.h \
NSLayoutGuide.h \
NSLayoutManager.h \
NSLevelIndicator.h \
NSLevelIndicatorCell.h \
NSMagnificationGestureRecognizer.h \
NSMatrix.h \
NSMediaLibraryBrowserController.h \
NSMenu.h \
NSMenuItem.h \
NSMenuItemCell.h \
NSMenuView.h \
NSMovie.h \
NSMovieView.h \
NSObjectController.h \
NSOpenPanel.h \
NSOpenGL.h \
NSOpenGLView.h \
NSOutlineView.h \
NSPageController.h \
NSPageLayout.h \
NSPanel.h \
NSPanGestureRecognizer.h \
NSPathCell.h \
NSPathComponentCell.h \
NSPathControl.h \
NSPathControlItem.h \
NSParagraphStyle.h \
NSPasteboard.h \
NSPasteboardItem.h \
NSPDFInfo.h \
NSPDFImageRep.h \
NSPDFPanel.h \
NSPersistentDocument.h \
NSPICTImageRep.h \
NSPickerTouchBarItem.h \
NSPopoverTouchBarItem.h \
NSPopover.h \
NSPopUpButton.h \
NSPopUpButtonCell.h \
NSPredicateEditor.h \
NSPredicateEditorRowTemplate.h \
NSPressGestureRecognizer.h \
NSPrintInfo.h \
NSPrintOperation.h \
NSPrintPanel.h \
NSPrinter.h \
NSProgressIndicator.h \
NSResponder.h \
NSRotationGestureRecognizer.h \
NSRuleEditor.h \
NSRulerMarker.h \
NSRulerView.h \
NSRunningApplication.h \
NSSavePanel.h \
NSScreen.h \
NSScrollView.h \
NSScroller.h \
NSScrubber.h \
NSScrubberItemView.h \
NSScrubberLayout.h \
NSSearchField.h \
NSSearchFieldCell.h \
NSSecureTextField.h \
NSSegmentedCell.h \
NSSegmentedControl.h \
NSSelection.h \
NSShadow.h \
NSSharingService.h \
NSSharingServicePickerToolbarItem.h \
NSSharingServicePickerTouchBarItem.h \
NSSlider.h \
NSSliderCell.h \
NSSliderAccessory.h \
NSSliderTouchBarItem.h \
NSSound.h  \
NSSpeechRecognizer.h \
NSSpeechSynthesizer.h \
NSSpellChecker.h \
NSSpellServer.h \
NSSplitView.h \
NSSplitViewController.h \
NSSplitViewItem.h \
NSStepper.h \
NSStepperCell.h \
NSStepperTouchBarItem.h \
NSStoryboard.h \
NSStoryboardSegue.h \
NSSeguePerforming.h \
NSStringDrawing.h \
NSStatusBar.h \
NSStatusBarButton.h \
NSStatusItem.h \
NSSwitch.h \
NSTabView.h \
NSTabViewController.h \
NSTabViewItem.h \
NSTableColumn.h \
NSTableHeaderCell.h \
NSTableHeaderView.h \
NSTableView.h \
NSText.h \
NSTextAlternatives.h \
NSTextAttachment.h \
NSTextContainer.h \
NSTextCheckingClient.h \
NSTextCheckingController.h \
NSTextField.h \
NSTextFieldCell.h \
NSTextFinder.h \
NSTextInputClient.h \
NSTextInputContext.h \
NSTextList.h \
NSTextStorage.h \
NSTextTable.h \
NSTextView.h \
NSTitlebarAccessoryViewController.h \
NSToolbar.h \
NSToolbarItem.h \
NSToolbarItemGroup.h \
NSTouch.h \
NSTouchBar.h \
NSTouchBarItem.h \
NSTokenField.h \
NSTokenFieldCell.h \
NSTrackingArea.h \
NSTreeController.h \
NSTreeNode.h \
NSUserDefaultsController.h \
NSUserInterfaceLayout.h \
NSView.h \
NSViewController.h \
NSVisualEffectView.h \
NSWindow.h \
NSWindowController.h \
NSWindowRestoration.h \
NSWorkspace.h \
NSAttributedString.h \
NSColorPicking.h \
NSDragging.h \
NSGraphics.h \
NSNibConnector.h \
NSNibControlConnector.h \
NSNibOutletConnector.h \
NSNibDeclarations.h \
NSNibLoading.h \
NSNib.h \
NSSpellProtocol.h \
NSUserInterfaceCompression.h \
NSUserInterfaceItemSearching.h \
NSUserInterfaceItemIdentification.h \
NSUserInterfaceValidation.h \
DPSOperators.h \
PSOperators.h 

GUI_HEADERS = \
GSVersion.h \
GMAppKit.h \
GMArchiver.h \
GSAnimator.h \
GSTheme.h \
GSFontInfo.h \
GSMemoryPanel.h \
GSInfoPanel.h \
GSMethodTable.h \
GSPasteboardServer.h \
GSServicesManager.h \
GSTextConverter.h \
GSTrackingRect.h \
GSHelpManagerPanel.h \
GSGormLoading.h \
GSNibContainer.h \
GSDisplayServer.h \
GSTable.h \
GSHbox.h \
GSVbox.h \
IMConnectors.h \
IMCustomObject.h \
IMLoading.h \
GSLayoutManager.h \
GSLayoutManager_internal.h \
GSTypesetter.h \
GSHorizontalTypesetter.h \
GSToolbarView.h \
GSNibLoading.h \
GSDragView.h \
GSTitleView.h \
GSPrinting.h \
GSPrintOperation.h \
GSEPSPrintOperation.h \
GSPDFPrintOperation.h \
GSModelLoaderFactory.h \
GSImageMagickImageRep.h \
GSInstantiator.h \
GSSoundSink.h \
GSSoundSource.h \
GSWindowDecorationView.h \
GSXibElement.h \
GSXibLoading.h \
GSXibKeyedUnarchiver.h \
GSXibObjectContainer.h \
GSHelpAttachment.h

libgnustep-gui_HEADER_FILES = ${GUI_HEADERS}

HEADERS_INSTALL = ${APPKIT_HEADERS} \
                  ${GUI_HEADERS} \
                  ${COCOA_HEADERS}

# Resources
RESOURCE_SET_NAME = libgui-resources
libgui-resources_INSTALL_DIR = $(GNUSTEP_LIBRARY)/Libraries/gnustep-gui/Versions/$(libgnustep-gui_INTERFACE_VERSION)/Resources
libgui-resources_RESOURCE_DIRS =
libgui-resources_RESOURCE_FILES = Info-gnustep.plist


# Extra DLL exports file
libgnustep-gui_DLL_DEF = libgnustep-gui.def

include GNUmakefile.preamble

include $(GNUSTEP_MAKEFILES)/library.make
include $(GNUSTEP_MAKEFILES)/resource-set.make

include GNUmakefile.postamble

include CompatibilityHeaders.make
