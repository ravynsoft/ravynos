/*
   externs.m

   External data

   Copyright (C) 1997-2017 Free Software Foundation, Inc.

   Author:  Scott Christley <scottc@net-community.com>
   Date: August 1997

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

#import "config.h"
#import <Foundation/NSString.h>
#import <Foundation/NSObjCRuntime.h>

#import "AppKit/NSApplication.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSAppearance.h"
#import "AppKit/NSFontCollection.h"
#import "AppKit/NSTextFinder.h"

// Global strings
NSString *NSModalPanelRunLoopMode = @"NSModalPanelRunLoopMode";
NSString *NSEventTrackingRunLoopMode = @"NSEventTrackingRunLoopMode";

const double NSAppKitVersionNumber = NSAppKitVersionNumber10_4;

//
// Global Exception Strings
//
NSExceptionName NSAbortModalException = @"NSAbortModalException";
NSExceptionName NSAbortPrintingException = @"NSAbortPrintingException";
NSExceptionName NSAppKitIgnoredException = @"NSAppKitIgnoredException";
NSExceptionName NSAppKitVirtualMemoryException = @"NSAppKitVirtualMemoryException";
NSExceptionName NSBadBitmapParametersException = @"NSBadBitmapParametersException";
NSExceptionName NSBadComparisonException = @"NSBadComparisonException";
NSExceptionName NSBadRTFColorTableException = @"NSBadRTFColorTableException";
NSExceptionName NSBadRTFDirectiveException = @"NSBadRTFDirectiveException";
NSExceptionName NSBadRTFFontTableException = @"NSBadRTFFontTableException";
NSExceptionName NSBadRTFStyleSheetException = @"NSBadRTFStyleSheetException";
NSExceptionName NSBrowserIllegalDelegateException = @"NSBrowserIllegalDelegateException";
NSExceptionName NSColorListIOException = @"NSColorListIOException";
NSExceptionName NSColorListNotEditableException = @"NSColorListNotEditableException";
NSExceptionName NSDraggingException = @"NSDraggingException";
NSExceptionName NSFontUnavailableException = @"NSFontUnavailableException";
NSExceptionName NSIllegalSelectorException = @"NSIllegalSelectorException";
NSExceptionName NSImageCacheException = @"NSImageCacheException";
NSExceptionName NSNibLoadingException = @"NSNibLoadingException";
NSExceptionName NSPPDIncludeNotFoundException = @"NSPPDIncludeNotFoundException";
NSExceptionName NSPPDIncludeStackOverflowException = @"NSPPDIncludeStackOverflowException";
NSExceptionName NSPPDIncludeStackUnderflowException = @"NSPPDIncludeStackUnderflowException";
NSExceptionName NSPPDParseException = @"NSPPDParseException";
NSExceptionName NSPrintOperationExistsException = @"NSPrintOperationExistsException";
NSExceptionName NSPrintPackageException = @"NSPrintPackageException";
NSExceptionName NSPrintingCommunicationException = @"NSPrintingCommunicationException";
NSExceptionName NSRTFPropertyStackOverflowException = @"NSRTFPropertyStackOverflowException";
NSExceptionName NSTIFFException = @"NSTIFFException";
NSExceptionName NSTextLineTooLongException = @"NSTextLineTooLongException";
NSExceptionName NSTextNoSelectionException = @"NSTextNoSelectionException";
NSExceptionName NSTextReadException = @"NSTextReadException";
NSExceptionName NSTextWriteException = @"NSTextWriteException";
NSExceptionName NSTypedStreamVersionException = @"NSTypedStreamVersionException";
NSExceptionName NSWindowServerCommunicationException = @"NSWindowServerCommunicationException";
NSExceptionName NSWordTablesReadException = @"NSWordTablesReadException";
NSExceptionName NSWordTablesWriteException = @"NSWordTablesWriteException";

NSExceptionName GSWindowServerInternalException = @"WindowServerInternal";

// NSAnimation
NSString* NSAnimationProgressMarkNotification
= @"NSAnimationProgressMarkNotification";
NSString *NSAnimationProgressMark = @"NSAnimationProgressMark";
NSString *NSAnimationTriggerOrderIn = @"NSAnimationTriggerOrderIn"; 
NSString *NSAnimationTriggerOrderOut = @"NSAnimationTriggerOrderOut"; 

// Application notifications
NSString *NSApplicationDidBecomeActiveNotification
              = @"NSApplicationDidBecomeActiveNotification";
NSString *NSApplicationDidChangeScreenParametersNotification 
              = @"NSApplicationDidChangeScreenParametersNotification";
NSString *NSApplicationDidFinishLaunchingNotification
              = @"NSApplicationDidFinishLaunchingNotification";
NSString *NSApplicationDidHideNotification = @"NSApplicationDidHideNotification";
NSString *NSApplicationDidResignActiveNotification
              = @"NSApplicationDidResignActiveNotification";
NSString *NSApplicationDidUnhideNotification = @"NSApplicationDidUnhideNotification";
NSString *NSApplicationDidUpdateNotification = @"NSApplicationDidUpdateNotification";
NSString *NSApplicationWillBecomeActiveNotification
              = @"NSApplicationWillBecomeActiveNotification";
NSString *NSApplicationWillFinishLaunchingNotification
              = @"NSApplicationWillFinishLaunchingNotification";
NSString *NSApplicationWillTerminateNotification = @"NSApplicationWillTerminateNotification";
NSString *NSApplicationWillHideNotification = @"NSApplicationWillHideNotification";
NSString *NSApplicationWillResignActiveNotification
              = @"NSApplicationWillResignActiveNotification";
NSString *NSApplicationWillUnhideNotification = @"NSApplicationWillUnhideNotification";
NSString *NSApplicationWillUpdateNotification = @"NSApplicationWillUpdateNotification";

// NSBitmapImageRep Global strings
NSString *NSImageCompressionMethod = @"NSImageCompressionMethod";
NSString *NSImageCompressionFactor = @"NSImageCompressionFactor";
NSString *NSImageDitherTransparency = @"NSImageDitherTransparency";
NSString *NSImageRGBColorTable = @"NSImageRGBColorTable";
NSString *NSImageInterlaced = @"NSImageInterlaced";
NSString *NSImageColorSyncProfileData = @"NSImageColorSyncProfileData";  // Mac OS X only
//NSString *GSImageICCProfileData = @"GSImageICCProfileData";  // if & when GNUstep supports color management
NSString *NSImageFrameCount = @"NSImageFrameCount";
NSString *NSImageCurrentFrame = @"NSImageCurrentFrame";
NSString *NSImageCurrentFrameDuration = @"NSImageCurrentFrameDuration";
NSString *NSImageLoopCount = @"NSImageLoopCount";
NSString *NSImageGamma = @"NSImageGamma";
NSString *NSImageProgressive = @"NSImageProgressive";
NSString *NSImageEXIFData = @"NSImageEXIFData";  // No support yet in GNUstep

// NSBrowser notification
NSString *NSBrowserColumnConfigurationDidChangeNotification = @"NSBrowserColumnConfigurationDidChange";

// NSColor Global strings
NSString *NSCalibratedWhiteColorSpace = @"NSCalibratedWhiteColorSpace";
NSString *NSCalibratedBlackColorSpace = @"NSCalibratedBlackColorSpace";
NSString *NSCalibratedRGBColorSpace = @"NSCalibratedRGBColorSpace";
NSString *NSDeviceWhiteColorSpace = @"NSDeviceWhiteColorSpace";
NSString *NSDeviceBlackColorSpace = @"NSDeviceBlackColorSpace";
NSString *NSDeviceRGBColorSpace = @"NSDeviceRGBColorSpace";
NSString *NSDeviceCMYKColorSpace = @"NSDeviceCMYKColorSpace";
NSString *NSNamedColorSpace = @"NSNamedColorSpace";
NSString *NSPatternColorSpace = @"NSPatternColorSpace";
NSString *NSCustomColorSpace = @"NSCustomColorSpace";

// NSColor Global gray values
const CGFloat NSBlack = 0;
const CGFloat NSDarkGray = .333;
const CGFloat NSGray = 0.5;
const CGFloat NSLightGray = .667;
const CGFloat NSWhite = 1;

const CGFloat NSFontWeightUltraLight = -0.8;
const CGFloat NSFontWeightThin = -0.6;
const CGFloat NSFontWeightLight = -0.4;
const CGFloat NSFontWeightRegular = 0;
const CGFloat NSFontWeightMedium = 0.23;
const CGFloat NSFontWeightSemibold = 0.3;
const CGFloat NSFontWeightBold = 0.4;
const CGFloat NSFontWeightHeavy = 0.56;
const CGFloat NSFontWeightBlack = 0.62;

// NSColor notification
NSString *NSSystemColorsDidChangeNotification =
            @"NSSystemColorsDidChangeNotification";

// NSColorList notifications
NSString *NSColorListDidChangeNotification = @"NSColorListDidChangeNotification";

// NSColorPanel notifications
NSString *NSColorPanelColorDidChangeNotification =
  @"NSColorPanelColorDidChangeNotification";

// NSComboBox notifications
NSString *NSComboBoxWillPopUpNotification = 
@"NSComboBoxWillPopUpNotification";
NSString *NSComboBoxWillDismissNotification = 
@"NSComboBoxWillDismissNotification";
NSString *NSComboBoxSelectionDidChangeNotification = 
@"NSComboBoxSelectionDidChangeNotification";
NSString *NSComboBoxSelectionIsChangingNotification = 
@"NSComboBoxSelectionIsChangingNotification";

// NSControl notifications
NSString *NSControlTextDidBeginEditingNotification =
@"NSControlTextDidBeginEditingNotification";
NSString *NSControlTextDidEndEditingNotification =
@"NSControlTextDidEndEditingNotification";
NSString *NSControlTextDidChangeNotification =
@"NSControlTextDidChangeNotification";

// NSDataLink global strings
NSString *NSDataLinkFilenameExtension = @"dlf";

// NSDrawer notifications
NSString *NSDrawerDidCloseNotification =
@"NSDrawerDidCloseNotification";
NSString *NSDrawerDidOpenNotification =
@"NSDrawerDidOpenNotification";
NSString *NSDrawerWillCloseNotification =
@"NSDrawerWillCloseNotification";
NSString *NSDrawerWillOpenNotification =
@"NSDrawerWillOpenNotification";

// NSForm private notification
NSString *_NSFormCellDidChangeTitleWidthNotification 
= @"_NSFormCellDidChangeTitleWidthNotification";

// NSGraphicContext constants
NSString *NSGraphicsContextDestinationAttributeName = 
@"NSGraphicsContextDestinationAttributeName";
NSString *NSGraphicsContextPDFFormat = 
@"NSGraphicsContextPDFFormat";
NSString *NSGraphicsContextPSFormat = 
@"NSGraphicsContextPSFormat";
NSString *NSGraphicsContextRepresentationFormatAttributeName = 
@"NSGraphicsContextRepresentationFormatAttributeName";

// NSHelpManager notifications;
NSString *NSContextHelpModeDidActivateNotification =
@"NSContextHelpModeDidActivateNotification";
NSString *NSContextHelpModeDidDeactivateNotification =
@"NSContextHelpModeDidDeactivateNotification";

// NSFont Global Strings
NSString *NSAFMAscender = @"Ascender";
NSString *NSAFMCapHeight = @"CapHeight";
NSString *NSAFMCharacterSet = @"CharacterSet";
NSString *NSAFMDescender = @"Descender";
NSString *NSAFMEncodingScheme = @"EncodingScheme";
NSString *NSAFMFamilyName = @"FamilyName";
NSString *NSAFMFontName = @"FontName";
NSString *NSAFMFormatVersion = @"FormatVersion";
NSString *NSAFMFullName = @"FullName";
NSString *NSAFMItalicAngle = @"ItalicAngle";
NSString *NSAFMMappingScheme = @"MappingScheme";
NSString *NSAFMNotice = @"Notice";
NSString *NSAFMUnderlinePosition = @"UnderlinePosition";
NSString *NSAFMUnderlineThickness = @"UnderlineThickness";
NSString *NSAFMVersion = @"Version";
NSString *NSAFMWeight = @"Weight";
NSString *NSAFMXHeight = @"XHeight";

// NSFontDescriptor global strings
NSString *NSFontFamilyAttribute = @"NSFontFamilyAttribute";
NSString *NSFontNameAttribute = @"NSFontNameAttribute";
NSString *NSFontFaceAttribute = @"NSFontFaceAttribute";
NSString *NSFontSizeAttribute = @"NSFontSizeAttribute"; 
NSString *NSFontVisibleNameAttribute = @"NSFontVisibleNameAttribute"; 
NSString *NSFontColorAttribute = @"NSFontColorAttribute";
NSString *NSFontMatrixAttribute = @"NSFontMatrixAttribute";
NSString *NSFontVariationAttribute = @"NSCTFontVariationAttribute";
NSString *NSFontCharacterSetAttribute = @"NSCTFontCharacterSetAttribute";
NSString *NSFontCascadeListAttribute = @"NSCTFontCascadeListAttribute";
NSString *NSFontTraitsAttribute = @"NSCTFontTraitsAttribute";
NSString *NSFontFixedAdvanceAttribute = @"NSCTFontFixedAdvanceAttribute";

NSString *NSFontSymbolicTrait = @"NSCTFontSymbolicTrait";
NSString *NSFontWeightTrait = @"NSCTFontWeightTrait";
NSString *NSFontWidthTrait = @"NSCTFontProportionTrait";
NSString *NSFontSlantTrait = @"NSCTFontSlantTrait";

NSString *NSFontVariationAxisIdentifierKey = @"NSCTFontVariationAxisIdentifier";
NSString *NSFontVariationAxisMinimumValueKey = @"NSCTFontVariationAxisMinimumValue";
NSString *NSFontVariationAxisMaximumValueKey = @"NSCTFontVariationAxisMaximumValue";
NSString *NSFontVariationAxisDefaultValueKey = @"NSCTFontVariationAxisDefaultValue";
NSString *NSFontVariationAxisNameKey = @"NSCTFontVariationAxisName";

// NSScreen Global device dictionary key strings
NSString *NSDeviceResolution = @"NSDeviceResolution";
NSString *NSDeviceColorSpaceName = @"NSDeviceColorSpaceName";
NSString *NSDeviceBitsPerSample = @"NSDeviceBitsPerSample";
NSString *NSDeviceIsScreen = @"NSDeviceIsScreen";
NSString *NSDeviceIsPrinter = @"NSDeviceIsPrinter";
NSString *NSDeviceSize = @"NSDeviceSize";

// NSImageRep notifications
NSString *NSImageRepRegistryChangedNotification =
@"NSImageRepRegistryChangedNotification";

// Pasteboard Type Globals
NSString *const NSPasteboardTypeString = @"NSStringPboardType";
NSString *const NSStringPboardType = @"NSStringPboardType";

NSString *const NSPasteboardTypeColor = @"NSColorPboardType";
NSString *const NSColorPboardType = @"NSColorPboardType";

NSString *const NSPasteboardTypeFont = @"NSFontPboardType";
NSString *const NSFontPboardType = @"NSFontPboardType";

NSString *const NSPasteboardTypeRuler = @"NSRulerPboardType";
NSString *const NSRulerPboardType = @"NSRulerPboardType";

NSString *const NSPasteboardTypeTabularText = @"NSTabularTextPboardType";
NSString *const NSTabularTextPboardType = @"NSTabularTextPboardType";

NSString *const NSPasteboardTypeRTF = @"NSRTFPboardType";
NSString *const NSRTFPboardType = @"NSRTFPboardType";

NSString *const NSPasteboardTypeRTFD = @"NSRTFDPboardType";
NSString *const NSRTFDPboardType = @"NSRTFDPboardType";

NSString *const NSPasteboardTypeTIFF = @"NSTIFFPboardType";
NSString *const NSTIFFPboardType = @"NSTIFFPboardType";

NSString *const NSPasteboardTypePDF = @"NSPDFPboardType";
NSString *const NSPDFPboardType = @"NSPDFPboardType";

NSString *const NSPasteboardTypeHTML = @"NSHTMLPboardType";
NSString *const NSHTMLPboardType = @"NSHTMLPboardType";

NSString *NSPasteboardTypePNG = @"NSPasteboardTypePNG";
NSString *NSPasteboardTypeSound = @"NSPasteboardTypeSound";
NSString *NSPasteboardTypeMultipleTextSelection =
  @"NSPasteboardTypeMultipleTextSelection";
NSString *NSPasteboardTypeTextFinderOptions =
  @"NSPasteboardTypeTextFinderOptions";

NSString *NSFileContentsPboardType = @"NSFileContentsPboardType";
NSString *NSFilenamesPboardType = @"NSFilenamesPboardType";
NSString *NSPostScriptPboardType = @"NSPostScriptPboardType";
NSString *NSDataLinkPboardType = @"NSDataLinkPboardType";
NSString *NSGeneralPboardType = @"NSGeneralPboardType";
NSString *NSPICTPboardType = @"NSPICTPboardType";
NSString *NSURLPboardType = @"NSURLPboardType";
NSString *NSVCardPboardType = @"NSVCardPboardType";
NSString *NSFilesPromisePboardType = @"NSFilesPromisePboardType";

// Pasteboard Name Globals
NSString *NSDragPboard = @"NSDragPboard";
NSString *NSFindPboard = @"NSFindPboard";
NSString *NSFontPboard = @"NSFontPboard";
NSString *NSGeneralPboard = @"NSGeneralPboard";
NSString *NSRulerPboard = @"NSRulerPboard";

//
// Pasteboard Exceptions
//
NSString *NSPasteboardCommunicationException
= @"NSPasteboardCommunicationException";

NSString *_NXSmartPaste = @"NeXT smart paste pasteboard type";

// Printing Information Dictionary Keys
NSString *NSPrintAllPages = @"NSPrintAllPages";
NSString *NSPrintBottomMargin = @"NSBottomMargin";
NSString *NSPrintCopies = @"NSCopies";
NSString *NSPrintFaxCoverSheetName = @"NSPrintFaxCoverSheetName";
NSString *NSPrintFaxHighResolution = @"NSPrintFaxHighResolution";
NSString *NSPrintFaxModem = @"NSPrintFaxModem";
NSString *NSPrintFaxReceiverNames = @"NSPrintFaxReceiverNames";
NSString *NSPrintFaxReceiverNumbers = @"NSPrintFaxReceiverNumbers";
NSString *NSPrintFaxReturnReceipt = @"NSPrintFaxReturnReceipt";
NSString *NSPrintFaxSendTime = @"NSPrintFaxSendTime";
NSString *NSPrintFaxTrimPageEnds = @"NSPrintFaxTrimPageEnds";
NSString *NSPrintFaxUseCoverSheet = @"NSPrintFaxUseCoverSheet";
NSString *NSPrintFirstPage = @"NSFirstPage";
NSString *NSPrintHorizontalPagination = @"NSHorizontalPagination";
NSString *NSPrintHorizontallyCentered = @"NSHorizontallyCentered";
NSString *NSPrintJobDisposition = @"NSJobDisposition";
NSString *NSPrintJobFeatures = @"NSJobFeatures";
NSString *NSPrintLastPage = @"NSLastPage";
NSString *NSPrintLeftMargin = @"NSLeftMargin";
NSString *NSPrintManualFeed = @"NSPrintManualFeed";
NSString *NSPrintMustCollate = @"NSMustCollate";
NSString *NSPrintOrientation = @"NSOrientation";
NSString *NSPrintPagesPerSheet = @"NSPagesPerSheet";
NSString *NSPrintPaperFeed = @"NSPaperFeed";
NSString *NSPrintPaperName = @"NSPaperName";
NSString *NSPrintPaperSize = @"NSPaperSize";
NSString *NSPrintPrinter = @"NSPrinter";
NSString *NSPrintReversePageOrder = @"NSReversePageOrder";
NSString *NSPrintRightMargin = @"NSRightMargin";
NSString *NSPrintSavePath = @"NSSavePath";
NSString *NSPrintScalingFactor = @"NSScalingFactor";
NSString *NSPrintTopMargin = @"NSTopMargin";
NSString *NSPrintVerticalPagination = @"NSVerticalPagination";
NSString *NSPrintVerticallyCentered = @"NSVerticallyCentered";
NSString *NSPrintPagesAcross = @"NSPagesAcross";
NSString *NSPrintPagesDown = @"NSPagesDown";
NSString *NSPrintTime = @"NSPrintTime";
NSString *NSPrintDetailedErrorReporting = @"NSDetailedErrorReporting";
NSString *NSPrintFaxNumber = @"NSFaxNumber";
NSString *NSPrintPrinterName = @"NSPrinterName";
NSString *NSPrintHeaderAndFooter = @"NSPrintHeaderAndFooter";

NSString *NSPrintPageDirection = @"NSPrintPageDirection";

// Print Job Disposition Values
NSString  *NSPrintCancelJob = @"NSPrintCancelJob";
NSString  *NSPrintFaxJob = @"NSPrintFaxJob";
NSString  *NSPrintPreviewJob = @"NSPrintPreviewJob";
NSString  *NSPrintSaveJob = @"NSPrintSaveJob";
NSString  *NSPrintSpoolJob = @"NSPrintSpoolJob";

// Print Panel
NSString *NSPrintPanelAccessorySummaryItemNameKey = @"name";
NSString *NSPrintPanelAccessorySummaryItemDescriptionKey = @"description";
NSString *NSPrintPhotoJobStyleHint = @"Photo";

// NSSplitView notifications
NSString *NSSplitViewDidResizeSubviewsNotification =
@"NSSplitViewDidResizeSubviewsNotification";
NSString *NSSplitViewWillResizeSubviewsNotification =
@"NSSplitViewWillResizeSubviewsNotification";

// NSTableView notifications
NSString *NSTableViewColumnDidMove = @"NSTableViewColumnDidMoveNotification";
NSString *NSTableViewColumnDidResize 
= @"NSTableViewColumnDidResizeNotification";
NSString *NSTableViewSelectionDidChange 
= @"NSTableViewSelectionDidChangeNotification";
NSString *NSTableViewSelectionIsChanging 
= @"NSTableViewSelectionIsChangingNotification";

// NSText notifications
NSString *NSTextDidBeginEditingNotification =
@"NSTextDidBeginEditingNotification";
NSString *NSTextDidEndEditingNotification = @"NSTextDidEndEditingNotification";
NSString *NSTextDidChangeNotification = @"NSTextDidChangeNotification";

// NSTextStorage Notifications
NSString *NSTextStorageWillProcessEditingNotification =
  @"NSTextStorageWillProcessEditingNotification";
NSString *NSTextStorageDidProcessEditingNotification =
  @"NSTextStorageDidProcessEditingNotification";

// NSTextView notifications
NSString *NSTextViewDidChangeSelectionNotification =
@"NSTextViewDidChangeSelectionNotification";
NSString *NSTextViewWillChangeNotifyingTextViewNotification =
@"NSTextViewWillChangeNotifyingTextViewNotification";
NSString *NSTextViewDidChangeTypingAttributesNotification =
@"NSTextViewDidChangeTypingAttributesNotification";

// NSView notifications
NSString *NSViewFocusDidChangeNotification
    = @"NSViewFocusDidChangeNotification";
NSString *NSViewFrameDidChangeNotification
    = @"NSViewFrameDidChangeNotification";
NSString *NSViewBoundsDidChangeNotification
    = @"NSViewBoundsDidChangeNotification";
NSString *NSViewGlobalFrameDidChangeNotification
    = @"NSViewGlobalFrameDidChangeNotification";

// NSViewAnimation 
NSString *NSViewAnimationTargetKey     = @"NSViewAnimationTargetKey";
NSString *NSViewAnimationStartFrameKey = @"NSViewAnimationStartFrameKey";
NSString *NSViewAnimationEndFrameKey   = @"NSViewAnimationEndFrameKey";
NSString *NSViewAnimationEffectKey     = @"NSViewAnimationEffectKey";
NSString *NSViewAnimationFadeInEffect  = @"NSViewAnimationFadeInEffect";
NSString *NSViewAnimationFadeOutEffect = @"NSViewAnimationFadeOutEffect";


// NSMenu notifications
NSString* const NSMenuDidSendActionNotification = @"NSMenuDidSendActionNotification";
NSString* const NSMenuWillSendActionNotification = @"NSMenuWillSendActionNotification";
NSString* const NSMenuDidAddItemNotification = @"NSMenuDidAddItemNotification";
NSString* const NSMenuDidRemoveItemNotification = @"NSMenuDidRemoveItemNotification";
NSString* const NSMenuDidChangeItemNotification = @"NSMenuDidChangeItemNotification";
NSString* const NSMenuDidBeginTrackingNotification = @"NSMenuDidBeginTrackingNotification";
NSString* const NSMenuDidEndTrackingNotification = @"NSMenuDidEndTrackingNotification";

// NSPopUpButton notification
NSString *NSPopUpButtonWillPopUpNotification = @"NSPopUpButtonWillPopUpNotification";
NSString *NSPopUpButtonCellWillPopUpNotification = @"NSPopUpButtonCellWillPopUpNotification";

// NSPopover notifications
NSString *NSPopoverWillShowNotification = @"NSPopoverWillShowNotification";
NSString *NSPopoverDidShowNotification = @"NSPopoverDidShowNotification";
NSString *NSPopoverWillCloseNotification = @"NSPopoverWillCloseNotification";
NSString *NSPopoverDidCloseNotification = @"NSPopoverDidCloseNotification";

// NSPopover keys
NSString *NSPopoverCloseReasonKey = @"NSPopoverCloseReasonKey";
NSString *NSPopoverCloseReasonStandard = @"NSPopoverCloseReasonStandard";
NSString *NSPopoverCloseReasonDetachToWindow = @"NSPopoverCloseReasonDetachToWindow";

// NSTable notifications
NSString *NSTableViewSelectionDidChangeNotification = @"NSTableViewSelectionDidChangeNotification";
NSString *NSTableViewColumnDidMoveNotification = @"NSTableViewColumnDidMoveNotification";
NSString *NSTableViewColumnDidResizeNotification = @"NSTableViewColumnDidResizeNotification";
NSString *NSTableViewSelectionIsChangingNotification = @"NSTableViewSelectionIsChangingNotification";

// NSOutlineView notifications
NSString *NSOutlineViewSelectionDidChangeNotification = @"NSOutlineViewSelectionDidChangeNotification";
NSString *NSOutlineViewColumnDidMoveNotification = @"NSOutlineViewColumnDidMoveNotification";
NSString *NSOutlineViewColumnDidResizeNotification = @"NSOutlineViewColumnDidResizeNotification";
NSString *NSOutlineViewSelectionIsChangingNotification = @"NSOutlineViewSelectionIsChangingNotification";
NSString *NSOutlineViewItemDidExpandNotification = @"NSOutlineViewItemDidExpandNotification";
NSString *NSOutlineViewItemDidCollapseNotification = @"NSOutlineViewItemDidCollapseNotification";
NSString *NSOutlineViewItemWillExpandNotification = @"NSOutlineViewItemWillExpandNotification";
NSString *NSOutlineViewItemWillCollapseNotification = @"NSOutlineViewItemWillCollapseNotification";

// NSWindow notifications
NSString *NSWindowDidBecomeKeyNotification = @"NSWindowDidBecomeKeyNotification";
NSString *NSWindowDidBecomeMainNotification = @"NSWindowDidBecomeMainNotification";
NSString *NSWindowDidChangeScreenNotification = @"NSWindowDidChangeScreenNotification";
NSString *NSWindowDidChangeScreenProfileNotification = @"NSWindowDidChangeScreenProfileNotification";
NSString *NSWindowDidDeminiaturizeNotification = @"NSWindowDidDeminiaturizeNotification";
NSString *NSWindowDidEndSheetNotification = @"NSWindowDidEndSheetNotification";
NSString *NSWindowDidExposeNotification = @"NSWindowDidExposeNotification";
NSString *NSWindowDidMiniaturizeNotification = @"NSWindowDidMiniaturizeNotification";
NSString *NSWindowDidMoveNotification = @"NSWindowDidMoveNotification";
NSString *NSWindowDidResignKeyNotification = @"NSWindowDidResignKeyNotification";
NSString *NSWindowDidResignMainNotification = @"NSWindowDidResignMainNotification";
NSString *NSWindowDidResizeNotification = @"NSWindowDidResizeNotification";
NSString *NSWindowDidUpdateNotification = @"NSWindowDidUpdateNotification";
NSString *NSWindowWillBeginSheetNotification = @"NSWindowWillBeginSheetNotification";
NSString *NSWindowWillCloseNotification = @"NSWindowWillCloseNotification";
NSString *NSWindowWillMiniaturizeNotification = @"NSWindowWillMiniaturizeNotification";
NSString *NSWindowWillMoveNotification = @"NSWindowWillMoveNotification";

// Workspace File Type Globals
NSString *NSPlainFileType = @"NSPlainFileType";
NSString *NSDirectoryFileType = @"NSDirectoryFileType";
NSString *NSApplicationFileType = @"NSApplicationFileType";
NSString *NSFilesystemFileType = @"NSFilesystemFileType";
NSString *NSShellCommandFileType = @"NSShellCommandFileType";

// Workspace File Operation Globals
NSString *NSWorkspaceCompressOperation = @"compress";
NSString *NSWorkspaceCopyOperation = @"copy";
NSString *NSWorkspaceDecompressOperation = @"decompress";
NSString *NSWorkspaceDecryptOperation = @"decrypt";
NSString *NSWorkspaceDestroyOperation = @"destroy";
NSString *NSWorkspaceDuplicateOperation = @"duplicate";
NSString *NSWorkspaceEncryptOperation = @"encrypt";
NSString *NSWorkspaceLinkOperation = @"link";
NSString *NSWorkspaceMoveOperation = @"move";
NSString *NSWorkspaceRecycleOperation = @"recycle";

// NSWorkspace notifications
NSString *NSWorkspaceDidLaunchApplicationNotification =
@"NSWorkspaceDidLaunchApplicationNotification";
NSString *NSWorkspaceDidMountNotification = @"NSWorkspaceDidMountNotification";
NSString *NSWorkspaceDidPerformFileOperationNotification =
@"NSWorkspaceDidPerformFileOperationNotification";
NSString *NSWorkspaceDidTerminateApplicationNotification =
@"NSWorkspaceDidTerminateApplicationNotification";
NSString *NSWorkspaceDidUnmountNotification =
@"NSWorkspaceDidUnmountNotification";
NSString *NSWorkspaceWillLaunchApplicationNotification =
@"NSWorkspaceWillLaunchApplicationNotification";
NSString *NSWorkspaceWillPowerOffNotification =
@"NSWorkspaceWillPowerOffNotification";
NSString *NSWorkspaceWillUnmountNotification =
@"NSWorkspaceWillUnmountNotification";
NSString *NSWorkspaceDidWakeNotification =
@"NSWorkspaceDidWakeNotification";
NSString *NSWorkspaceSessionDidBecomeActiveNotification =
@"NSWorkspaceSessionDidBecomeActiveNotification";
NSString *NSWorkspaceSessionDidResignActiveNotification =
@"NSWorkspaceSessionDidResignActiveNotification";
NSString *NSWorkspaceWillSleepNotification =
@"NSWorkspaceWillSleepNotification";

/*
 *	NSStringDrawing NSAttributedString additions
 */
NSString *NSAttachmentAttributeName = @"NSAttachment";
NSString *NSBackgroundColorAttributeName = @"NSBackgroundColor";
NSString *NSBaselineOffsetAttributeName = @"NSBaselineOffset";
NSString *NSCursorAttributeName = @"NSCursor";
NSString *NSExpansionAttributeName = @"NSExpansion";
NSString *NSFontAttributeName = @"NSFont";
NSString *NSForegroundColorAttributeName = @"NSColor";
NSString *NSKernAttributeName = @"NSKern";
NSString *NSLigatureAttributeName = @"NSLigature";
NSString *NSLinkAttributeName = @"NSLink";
NSString *NSObliquenessAttributeName = @"NSObliqueness";
NSString *NSParagraphStyleAttributeName = @"NSParagraphStyle";
NSString *NSShadowAttributeName = @"NSShadow";
NSString *NSStrikethroughColorAttributeName
  = @"NSStrikethroughColor";
NSString *NSStrikethroughStyleAttributeName = @"NSStrikethrough";
NSString *NSStrokeColorAttributeName = @"NSStrokeColor";
NSString *NSStrokeWidthAttributeName = @"NSStrokeWidth";
NSString *NSSuperscriptAttributeName = @"NSSuperScript";
NSString *NSToolTipAttributeName = @"NSToolTip";
NSString *NSUnderlineColorAttributeName = @"NSUnderlineColor";
NSString *NSUnderlineStyleAttributeName = @"NSUnderline";

NSString *NSTextAlternativesAttributeName = @"NSTextAlternatives";
NSString *NSWritingDirectionAttributeName = @"NSWritingDirection";

NSString *NSCharacterShapeAttributeName = @"NSCharacterShape";
NSString *NSGlyphInfoAttributeName = @"NSGlyphInfo";

NSString *NSPaperSizeDocumentAttribute = @"PaperSize";
NSString *NSLeftMarginDocumentAttribute = @"LeftMargin";
NSString *NSRightMarginDocumentAttribute = @"RightMargin";
NSString *NSTopMarginDocumentAttribute = @"TopMargin";
NSString *NSBottomMarginDocumentAttribute = @"BottomMargin";
NSString *NSHyphenationFactorDocumentAttribute = @"HyphenationFactor";
NSString *NSDocumentTypeDocumentAttribute = @"DocumentType";
NSString *NSCharacterEncodingDocumentAttribute = @"CharacterEncoding";
NSString *NSViewSizeDocumentAttribute = @"ViewSize";
NSString *NSViewZoomDocumentAttribute = @"ViewZoom";
NSString *NSViewModeDocumentAttribute = @"ViewMode";
NSString *NSBackgroundColorDocumentAttribute = @"BackgroundColor";
NSString *NSCocoaVersionDocumentAttribute = @"CocoaVersion";
NSString *NSReadOnlyDocumentAttribute = @"ReadOnly";
NSString *NSConvertedDocumentAttribute = @"Converted";
NSString *NSDefaultTabIntervalDocumentAttribute = @"DefaultTabInterval";
NSString *NSTitleDocumentAttribute = @"Title";
NSString *NSCompanyDocumentAttribute = @"Company";
NSString *NSCopyrightDocumentAttribute = @"Copyright";
NSString *NSSubjectDocumentAttribute = @"Subject";
NSString *NSAuthorDocumentAttribute = @"Author";
NSString *NSKeywordsDocumentAttribute = @"Keywords";
NSString *NSCommentDocumentAttribute = @"Comment";
NSString *NSEditorDocumentAttribute = @"Editor";
NSString *NSCreationTimeDocumentAttribute = @"CreationTime";
NSString *NSModificationTimeDocumentAttribute = @"ModificationTime";

NSString *NSTextInsertionUndoableAttributeName =
  @"NSTextInsertionUndoableAttributeName";

const unsigned NSUnderlineByWordMask = 0x01;

NSString *NSSpellingStateAttributeName = @"NSSpellingState";
const unsigned NSSpellingStateSpellingFlag = 1;
const unsigned NSSpellingStateGrammarFlag = 2;

NSString *NSSpellCheckerDidChangeAutomaticSpellingCorrectionNotification =
  @"NSSpellCheckerDidChangeAutomaticSpellingCorrectionNotification";
NSString *NSSpellCheckerDidChangeAutomaticTextReplacementNotification =
  @"NSSpellCheckerDidChangeAutomaticTextReplacementNotification";
NSString *NSSpellCheckerDidChangeAutomaticQuoteSubstitutionNotification =
  @"NSSpellCheckerDidChangeAutomaticQuoteSubstitutionNotification";
NSString *NSSpellCheckerDidChangeAutomaticDashSubstitutionNotification =
  @"NSSpellCheckerDidChangeAutomaticDashSubstitutionNotification";


NSString *NSPlainTextDocumentType = @"NSPlainText";
NSString *NSRTFTextDocumentType = @"NSRTF";
NSString *NSRTFDTextDocumentType = @"NSRTFD";
NSString *NSMacSimpleTextDocumentType = @"NSMacSimpleText";
NSString *NSHTMLTextDocumentType = @"NSHTML";
NSString *NSDocFormatTextDocumentType = @"NSDocFormat";
NSString *NSWordMLTextDocumentType = @"NSWordML";
NSString *NSOfficeOpenXMLTextDocumentType = @"NSOfficeOpenXML";
NSString *NSOpenDocumentTextDocumentType = @"NSOpenDocumentText";

NSString *NSExcludedElementsDocumentAttribute = @"ExcludedElements";
NSString *NSTextEncodingNameDocumentAttribute = @"TextEncodingName";
NSString *NSPrefixSpacesDocumentAttribute = @"PrefixSpaces";

NSString *NSBaseURLDocumentOption = @"BaseURL";
NSString *NSCharacterEncodingDocumentOption = @"CharacterEncoding";
NSString *NSDefaultAttributesDocumentOption = @"DefaultAttributes";
NSString *NSDocumentTypeDocumentOption = @"DocumentType";
NSString *NSTextEncodingNameDocumentOption = @"TextEncodingName";
NSString *NSTextSizeMultiplierDocumentOption = @"TextSizeMultiplier";
NSString *NSTimeoutDocumentOption = @"Timeout";
NSString *NSWebPreferencesDocumentOption = @"WebPreferences";
NSString *NSWebResourceLoadDelegateDocumentOption = @"WebResourceLoadDelegate";

// NSTextTab
NSString *NSTabColumnTerminatorsAttributeName = @"NSTabColumnTerminatorsAttributeName"; 

// Private Exports
NSString *NSMarkedClauseSegmentAttributeName =
  @"NSMarkedClauseSegmentAttributeName";
NSString *NSTextInputReplacementRangeAttributeName =
  @"NSTextInputReplacementRangeAttributeName";

// NSToolbar notifications
NSString *NSToolbarDidRemoveItemNotification = @"NSToolbarDidRemoveItemNotification";
NSString *NSToolbarWillAddItemNotification = @"NSToolbarWillAddItemNotification";

// NSToolbarItem constants
NSString *NSToolbarSeparatorItemIdentifier = @"NSToolbarSeparatorItem";
NSString *NSToolbarSpaceItemIdentifier = @"NSToolbarSpaceItem";
NSString *NSToolbarFlexibleSpaceItemIdentifier = @"NSToolbarFlexibleSpaceItem";
NSString *NSToolbarShowColorsItemIdentifier = @"NSToolbarShowColorsItem";
NSString *NSToolbarShowFontsItemIdentifier = @"NSToolbarShowFontsItem";
NSString *NSToolbarCustomizeToolbarItemIdentifier = @"NSToolbarCustomizeToolbarItem";
NSString *NSToolbarPrintItemIdentifier = @"NSToolbarPrintItem";

NSString *NSImageNameTrashEmpty = @"NSImageTrashEmpty";
NSString *NSImageNameTrashFull = @"NSImageTrashFull";

// Misc named images
NSString *NSImageNameMultipleDocuments = @"NSImageNameMultipleDocuments";

/*
 * NSTextView userInfo for notifications 
 */
NSString *NSOldSelectedCharacterRange = @"NSOldSelectedCharacterRange";

/* NSFont matrix */
const CGFloat NSFontIdentityMatrix[] = {1, 0, 0, 1, 0, 0};

/* Drawing engine externs */
NSString *NSBackendContext = @"NSBackendContext";

typedef int NSWindowDepth;

/**** Color function externs ****/
/* Since these are constants it was not possible
   to do the OR directly.  If you change the
   _GS*BitValue numbers, please remember to
   change the corresponding depth values */
const NSWindowDepth _GSGrayBitValue = 256;
const NSWindowDepth _GSRGBBitValue = 512;
const NSWindowDepth _GSCMYKBitValue = 1024;
const NSWindowDepth _GSNamedBitValue = 2048;
const NSWindowDepth _GSCustomBitValue = 4096;
const NSWindowDepth NSDefaultDepth = 0;            // GRAY = 256, RGB = 512
const NSWindowDepth NSTwoBitGrayDepth = 258;       // 0100000010 GRAY | 2bps
const NSWindowDepth NSEightBitGrayDepth = 264;     // 0100001000 GRAY | 8bps
const NSWindowDepth NSEightBitRGBDepth = 514;      // 1000000010 RGB  | 2bps
const NSWindowDepth NSTwelveBitRGBDepth = 516;     // 1000000100 RGB  | 4bps
const NSWindowDepth GSSixteenBitRGBDepth = 517;    // 1000000101 RGB  | 5bps GNUstep specific
const NSWindowDepth NSTwentyFourBitRGBDepth = 520; // 1000001000 RGB  | 8bps
const NSWindowDepth _GSWindowDepths[7] = { 258, 264, 514, 516, 517, 520, 0 };

/* End of color functions externs */

// NSKeyValueBinding
NSString *NSObservedObjectKey = @"NSObservedObject";
NSString *NSObservedKeyPathKey = @"NSObservedKeyPath";
NSString *NSOptionsKey = @"NSOptions";

NSString *NSAllowsEditingMultipleValuesSelectionBindingOption = @"NSAllowsEditingMultipleValuesSelection";
NSString *NSAllowsNullArgumentBindingOption = @"NSAllowsNullArgument";
NSString *NSConditionallySetsEditableBindingOption = @"NSConditionallySetsEditable";
NSString *NSConditionallySetsEnabledBindingOption = @"NSConditionallySetsEnabled";
NSString *NSConditionallySetsHiddenBindingOption = @"NSConditionallySetsHidden";
NSString *NSContinuouslyUpdatesValueBindingOption = @"NSContinuouslyUpdatesValue";
NSString *NSCreatesSortDescriptorBindingOption = @"NSCreatesSortDescriptor";
NSString *NSDeletesObjectsOnRemoveBindingsOption = @"NSDeletesObjectsOnRemove";
NSString *NSDisplayNameBindingOption = @"NSDisplayName";
NSString *NSDisplayPatternBindingOption = @"NSDisplayPattern";
NSString *NSHandlesContentAsCompoundValueBindingOption = @"NSHandlesContentAsCompoundValue";
NSString *NSInsertsNullPlaceholderBindingOption = @"NSInsertsNullPlaceholder";
NSString *NSInvokesSeparatelyWithArrayObjectsBindingOption = @"NSInvokesSeparatelyWithArrayObjects";
NSString *NSMultipleValuesPlaceholderBindingOption = @"NSMultipleValuesPlaceholder";
NSString *NSNoSelectionPlaceholderBindingOption = @"NSNoSelectionPlaceholder";
NSString *NSNotApplicablePlaceholderBindingOption = @"NSNotApplicablePlaceholder";
NSString *NSNullPlaceholderBindingOption = @"NSNullPlaceholder";
NSString *NSPredicateFormatBindingOption = @"NSPredicateFormat";
NSString *NSRaisesForNotApplicableKeysBindingOption = @"NSRaisesForNotApplicableKeys";
NSString *NSSelectorNameBindingOption = @"NSSelectorName";
NSString *NSSelectsAllWhenSettingContentBindingOption = @"NSSelectsAllWhenSettingContent";
NSString *NSValidatesImmediatelyBindingOption = @"NSValidatesImmediately";
NSString *NSValueTransformerNameBindingOption = @"NSValueTransformerName";
NSString *NSValueTransformerBindingOption = @"NSValueTransformer";
 
NSString *NSAlignmentBinding = @"alignment";
NSString *NSContentArrayBinding = @"contentArray";
NSString *NSContentBinding = @"content";
NSString *NSContentObjectBinding = @"contentObject";
NSString *NSContentValuesBinding = @"contentValues";
NSString *NSEditableBinding = @"editable";
NSString *NSEnabledBinding = @"enabled";
NSString *NSFontBinding = @"font";
NSString *NSFontNameBinding = @"fontName";
NSString *NSFontSizeBinding = @"fontSize";
NSString *NSHiddenBinding = @"hidden";
NSString *NSSelectedIndexBinding = @"selectedIndex";
NSString *NSSelectedObjectBinding = @"selectedObject";
NSString *NSSelectedTagBinding = @"selectedTag";
NSString *NSSelectedValueBinding = @"selectedValue";
NSString *NSSelectionIndexesBinding = @"selectionIndexes";
NSString *NSSortDescriptorsBinding = @"sortDescriptors";
NSString *NSTextColorBinding = @"textColor";
NSString *NSTitleBinding = @"title";
NSString *NSToolTipBinding = @"toolTip";
NSString *NSValueBinding = @"value";

// FIXME: Need to define class _NSStateMarker!
id NSMultipleValuesMarker = @"<MULTIPLE VALUES MARKER>";
id NSNoSelectionMarker = @"<NO SELECTION MARKER>";
id NSNotApplicableMarker = @"<NOT APPLICABLE MARKER>";


// NSNib
NSString *NSNibTopLevelObjects = @"NSTopLevelObjects";
NSString *NSNibOwner = @"NSOwner";

// NSImage directly mapped NS named images constants
NSString *NSImageNameUserAccounts = @"NSUserAccounts";
NSString *NSImageNamePreferencesGeneral = @"NSPreferencesGeneral";
NSString *NSImageNameAdvanced = @"NSAdvanced";
NSString *NSImageNameInfo = @"NSInfo";
NSString *NSImageNameFontPanel = @"NSFontPanel";
NSString *NSImageNameColorPanel = @"NSColorPanel";
NSString *NSImageNameCaution = @"NSCaution";

// NSRuleEditor
NSString *const NSRuleEditorPredicateLeftExpression = @"NSRuleEditorPredicateLeftExpression";
NSString *const NSRuleEditorPredicateRightExpression = @"NSRuleEditorPredicateRightExpression";
NSString *const NSRuleEditorPredicateComparisonModifier = @"NSRuleEditorPredicateComparisonModifier";
NSString *const NSRuleEditorPredicateOptions = @"NSRuleEditorPredicateOptions";
NSString *const NSRuleEditorPredicateOperatorType = @"NSRuleEditorPredicateOperatorType";
NSString *const NSRuleEditorPredicateCustomSelector = @"NSRuleEditorPredicateCustomSelector";
NSString *const NSRuleEditorPredicateCompoundType = @"NSRuleEditorPredicateCompoundType";

NSString *NSRuleEditorRowsDidChangeNotification = @"NSRuleEditorRowsDidChangeNotification";

// NSAppearance
const NSAppearanceName NSAppearanceNameAqua = @"NSAppearanceNameAqua";
const NSAppearanceName NSAppearanceNameDarkAqua = @"NSAppearanceNameDarkAqua";
const NSAppearanceName NSAppearanceNameVibrantLight = @"NSAppearanceNameVibrantLight";
const NSAppearanceName NSAppearanceNameVibrantDark = @"NSAppearanceNameVibrantDark";
const NSAppearanceName NSAppearanceNameAccessibilityHighContrastAqua = @"NSAppearanceNameAccessibilityHighContrastAqua";
const NSAppearanceName NSAppearanceNameAccessibilityHighContrastDarkAqua = @"NSAppearanceNameAccessibilityHighContrastDarkAqua";
const NSAppearanceName NSAppearanceNameAccessibilityHighContrastVibrantLight =
  @"NSAppearanceNameAccessibilityHighContrastVibrantLight";
const NSAppearanceName NSAppearanceNameAccessibilityHighContrastVibrantDark =
  @"NSAppearanceNameAccessibilityHighContrastVibrantDark";
const NSAppearanceName NSAppearanceNameLightContent = @"NSAppearanceNameLightContent";

// Values for NSFontCollectionAction
NSFontCollectionActionTypeKey const NSFontCollectionWasShown = @"NSFontCollectionWasShown";
NSFontCollectionActionTypeKey const NSFontCollectionWasHidden = @"NSFontCollectionWasHidden";
NSFontCollectionActionTypeKey const NSFontCollectionWasRenamed = @"NSFontCollectionWasRenamed";

// Standard named collections
NSFontCollectionName const NSFontCollectionAllFonts = @"NSFontCollectionAllFonts";
NSFontCollectionName const NSFontCollectionUser = @"NSFontCollectionUser";
NSFontCollectionName const NSFontCollectionFavorites = @"NSFontCollectionFavorites";
NSFontCollectionName const NSFontCollectionRecentlyUsed = @"NSFontCollectionRecentlyUsed";

// Collections
NSFontCollectionMatchingOptionKey const NSFontCollectionIncludeDisabledFontsOption = @"NSFontCollectionIncludeDisabledFontsOption";
NSFontCollectionMatchingOptionKey const NSFontCollectionRemoveDuplicatesOption = @"NSFontCollectionRemoveDuplicatesOption";
NSFontCollectionMatchingOptionKey const NSFontCollectionDisallowAutoActivationOption = @"NSFontCollectionDisallowAutoActivationOption";

// Speech recognition...
const NSString *GSSpeechRecognizerDidRecognizeWordNotification = @"GSSpeechRecognizerDidRecognizeWordNotification"; 

// NSTextInputContext notifications
NSString *NSTextInputContextKeyboardSelectionDidChangeNotification =
  @"NSTextInputContextKeyboardSelectionDidChangeNotification";

NSPasteboardTypeTextFinderOptionKey const NSTextFinderCaseInsensitiveKey = @"NSTextFinderCaseInsensitiveKey";
NSPasteboardTypeTextFinderOptionKey const NSTextFinderMatchingTypeKey = @"NSTextFinderMatchingTypeKey";

extern void __objc_gui_force_linking (void);

void
__objc_gui_force_linking (void)
{
  extern void __objc_gui_linking (void);
  __objc_gui_linking ();
}
