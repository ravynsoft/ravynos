/* Definition of NSMetadataAttributes
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: Gregory John Casamento <greg.casamento@gmail.com>
   Date: Tue Oct 29 00:53:11 EDT 2019

   This file is part of the GNUstep Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
*/

#ifndef _NSMetadataAttributes_h_GNUSTEP_BASE_INCLUDE
#define _NSMetadataAttributes_h_GNUSTEP_BASE_INCLUDE

#include <Foundation/NSObject.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_0, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSString;
  
GS_EXPORT NSString* const NSMetadataItemAcquisitionMakeKey;  
GS_EXPORT NSString* const NSMetadataItemAcquisitionModelKey;  
GS_EXPORT NSString* const NSMetadataItemAlbumKey;  
GS_EXPORT NSString* const NSMetadataItemAltitudeKey;  
GS_EXPORT NSString* const NSMetadataItemApertureKey;  
GS_EXPORT NSString* const NSMetadataItemAppleLoopDescriptorsKey;  
GS_EXPORT NSString* const NSMetadataItemAppleLoopsKeyFilterTypeKey;  
GS_EXPORT NSString* const NSMetadataItemAppleLoopsLoopModeKey;  
GS_EXPORT NSString* const NSMetadataItemAppleLoopsRootKeyKey;  
GS_EXPORT NSString* const NSMetadataItemApplicationCategoriesKey;  
GS_EXPORT NSString* const NSMetadataItemAttributeChangeDateKey;  
GS_EXPORT NSString* const NSMetadataItemAudiencesKey;  
GS_EXPORT NSString* const NSMetadataItemAudioBitRateKey;  
GS_EXPORT NSString* const NSMetadataItemAudioChannelCountKey;  
GS_EXPORT NSString* const NSMetadataItemAudioEncodingApplicationKey;  
GS_EXPORT NSString* const NSMetadataItemAudioSampleRateKey;  
GS_EXPORT NSString* const NSMetadataItemAudioTrackNumberKey;  
GS_EXPORT NSString* const NSMetadataItemAuthorAddressesKey;  
GS_EXPORT NSString* const NSMetadataItemAuthorEmailAddressesKey;  
GS_EXPORT NSString* const NSMetadataItemAuthorsKey;  
GS_EXPORT NSString* const NSMetadataItemBitsPerSampleKey;  
GS_EXPORT NSString* const NSMetadataItemCameraOwnerKey;  
GS_EXPORT NSString* const NSMetadataItemCFBundleIdentifierKey;  
GS_EXPORT NSString* const NSMetadataItemCityKey;  
GS_EXPORT NSString* const NSMetadataItemCodecsKey;  
GS_EXPORT NSString* const NSMetadataItemColorSpaceKey;  
GS_EXPORT NSString* const NSMetadataItemCommentKey;  
GS_EXPORT NSString* const NSMetadataItemComposerKey;  
GS_EXPORT NSString* const NSMetadataItemContactKeywordsKey;  
GS_EXPORT NSString* const NSMetadataItemContentCreationDateKey;  
GS_EXPORT NSString* const NSMetadataItemContentModificationDateKey;  
GS_EXPORT NSString* const NSMetadataItemContentTypeKey;
GS_EXPORT NSString* const NSMetadataItemContentTypeTreeKey;
GS_EXPORT NSString* const NSMetadataItemContributorsKey;  
GS_EXPORT NSString* const NSMetadataItemCopyrightKey;  
GS_EXPORT NSString* const NSMetadataItemCountryKey;  
GS_EXPORT NSString* const NSMetadataItemCoverageKey;  
GS_EXPORT NSString* const NSMetadataItemCreatorKey;  
GS_EXPORT NSString* const NSMetadataItemDateAddedKey;  
GS_EXPORT NSString* const NSMetadataItemDeliveryTypeKey;  
GS_EXPORT NSString* const NSMetadataItemDescriptionKey;  
GS_EXPORT NSString* const NSMetadataItemDirectorKey;  
GS_EXPORT NSString* const NSMetadataItemDisplayNameKey;
GS_EXPORT NSString* const NSMetadataItemDownloadedDateKey;  
GS_EXPORT NSString* const NSMetadataItemDueDateKey;  
GS_EXPORT NSString* const NSMetadataItemDurationSecondsKey;  
GS_EXPORT NSString* const NSMetadataItemEditorsKey;  
GS_EXPORT NSString* const NSMetadataItemEmailAddressesKey;  
GS_EXPORT NSString* const NSMetadataItemEncodingApplicationsKey;  
GS_EXPORT NSString* const NSMetadataItemExecutableArchitecturesKey;  
GS_EXPORT NSString* const NSMetadataItemExecutablePlatformKey;  
GS_EXPORT NSString* const NSMetadataItemEXIFGPSVersionKey;  
GS_EXPORT NSString* const NSMetadataItemEXIFVersionKey;  
GS_EXPORT NSString* const NSMetadataItemExposureModeKey;  
GS_EXPORT NSString* const NSMetadataItemExposureProgramKey;  
GS_EXPORT NSString* const NSMetadataItemExposureTimeSecondsKey;  
GS_EXPORT NSString* const NSMetadataItemExposureTimeStringKey;  
GS_EXPORT NSString* const NSMetadataItemFinderCommentKey;  
GS_EXPORT NSString* const NSMetadataItemFlashOnOffKey;  
GS_EXPORT NSString* const NSMetadataItemFNumberKey;  
GS_EXPORT NSString* const NSMetadataItemFocalLength35mmKey;  
GS_EXPORT NSString* const NSMetadataItemFocalLengthKey;  
GS_EXPORT NSString* const NSMetadataItemFontsKey;  
GS_EXPORT NSString* const NSMetadataItemFSContentChangeDateKey; 
GS_EXPORT NSString* const NSMetadataItemFSCreationDateKey; 
GS_EXPORT NSString* const NSMetadataItemFSNameKey;
GS_EXPORT NSString* const NSMetadataItemFSSizeKey; 
GS_EXPORT NSString* const NSMetadataItemGenreKey;  
GS_EXPORT NSString* const NSMetadataItemGPSAreaInformationKey;  
GS_EXPORT NSString* const NSMetadataItemGPSDateStampKey;  
GS_EXPORT NSString* const NSMetadataItemGPSDestBearingKey;  
GS_EXPORT NSString* const NSMetadataItemGPSDestDistanceKey;  
GS_EXPORT NSString* const NSMetadataItemGPSDestLatitudeKey;  
GS_EXPORT NSString* const NSMetadataItemGPSDestLongitudeKey;  
GS_EXPORT NSString* const NSMetadataItemGPSDifferentalKey;  
GS_EXPORT NSString* const NSMetadataItemGPSDOPKey;  
GS_EXPORT NSString* const NSMetadataItemGPSMapDatumKey;  
GS_EXPORT NSString* const NSMetadataItemGPSMeasureModeKey;  
GS_EXPORT NSString* const NSMetadataItemGPSProcessingMethodKey;  
GS_EXPORT NSString* const NSMetadataItemGPSStatusKey;  
GS_EXPORT NSString* const NSMetadataItemGPSTrackKey;  
GS_EXPORT NSString* const NSMetadataItemHasAlphaChannelKey;  
GS_EXPORT NSString* const NSMetadataItemHeadlineKey;  
GS_EXPORT NSString* const NSMetadataItemIdentifierKey;  
GS_EXPORT NSString* const NSMetadataItemImageDirectionKey;  
GS_EXPORT NSString* const NSMetadataItemInformationKey;  
GS_EXPORT NSString* const NSMetadataItemInstantMessageAddressesKey;  
GS_EXPORT NSString* const NSMetadataItemInstructionsKey;  
GS_EXPORT NSString* const NSMetadataItemIsApplicationManagedKey;   
GS_EXPORT NSString* const NSMetadataItemIsGeneralMIDISequenceKey;  
GS_EXPORT NSString* const NSMetadataItemIsLikelyJunkKey;  
GS_EXPORT NSString* const NSMetadataItemISOSpeedKey;  
GS_EXPORT NSString* const NSMetadataItemIsUbiquitousKey;
GS_EXPORT NSString* const NSMetadataItemKeySignatureKey;  
GS_EXPORT NSString* const NSMetadataItemKeywordsKey;  
GS_EXPORT NSString* const NSMetadataItemKindKey;  
GS_EXPORT NSString* const NSMetadataItemLanguagesKey;  
GS_EXPORT NSString* const NSMetadataItemLastUsedDateKey;  
GS_EXPORT NSString* const NSMetadataItemLatitudeKey;  
GS_EXPORT NSString* const NSMetadataItemLayerNamesKey;  
GS_EXPORT NSString* const NSMetadataItemLensModelKey;  
GS_EXPORT NSString* const NSMetadataItemLongitudeKey;  
GS_EXPORT NSString* const NSMetadataItemLyricistKey;  
GS_EXPORT NSString* const NSMetadataItemMaxApertureKey;  
GS_EXPORT NSString* const NSMetadataItemMediaTypesKey;  
GS_EXPORT NSString* const NSMetadataItemMeteringModeKey;  
GS_EXPORT NSString* const NSMetadataItemMusicalGenreKey;  
GS_EXPORT NSString* const NSMetadataItemMusicalInstrumentCategoryKey;  
GS_EXPORT NSString* const NSMetadataItemMusicalInstrumentNameKey;  
GS_EXPORT NSString* const NSMetadataItemNamedLocationKey;  
GS_EXPORT NSString* const NSMetadataItemNumberOfPagesKey;  
GS_EXPORT NSString* const NSMetadataItemOrganizationsKey;  
GS_EXPORT NSString* const NSMetadataItemOrientationKey;  
GS_EXPORT NSString* const NSMetadataItemOriginalFormatKey;  
GS_EXPORT NSString* const NSMetadataItemOriginalSourceKey;  
GS_EXPORT NSString* const NSMetadataItemPageHeightKey;  
GS_EXPORT NSString* const NSMetadataItemPageWidthKey;  
GS_EXPORT NSString* const NSMetadataItemParticipantsKey;  
GS_EXPORT NSString* const NSMetadataItemPathKey; 
GS_EXPORT NSString* const NSMetadataItemPerformersKey;  
GS_EXPORT NSString* const NSMetadataItemPhoneNumbersKey;  
GS_EXPORT NSString* const NSMetadataItemPixelCountKey;  
GS_EXPORT NSString* const NSMetadataItemPixelHeightKey;  
GS_EXPORT NSString* const NSMetadataItemPixelWidthKey;  
GS_EXPORT NSString* const NSMetadataItemProducerKey;  
GS_EXPORT NSString* const NSMetadataItemProfileNameKey;  
GS_EXPORT NSString* const NSMetadataItemProjectsKey;  
GS_EXPORT NSString* const NSMetadataItemPublishersKey;  
GS_EXPORT NSString* const NSMetadataItemRecipientAddressesKey;  
GS_EXPORT NSString* const NSMetadataItemRecipientEmailAddressesKey;  
GS_EXPORT NSString* const NSMetadataItemRecipientsKey;  
GS_EXPORT NSString* const NSMetadataItemRecordingDateKey;  
GS_EXPORT NSString* const NSMetadataItemRecordingYearKey;  
GS_EXPORT NSString* const NSMetadataItemRedEyeOnOffKey;  
GS_EXPORT NSString* const NSMetadataItemResolutionHeightDPIKey;  
GS_EXPORT NSString* const NSMetadataItemResolutionWidthDPIKey;  
GS_EXPORT NSString* const NSMetadataItemRightsKey;  
GS_EXPORT NSString* const NSMetadataItemSecurityMethodKey;  
GS_EXPORT NSString* const NSMetadataItemSpeedKey;  
GS_EXPORT NSString* const NSMetadataItemStarRatingKey;  
GS_EXPORT NSString* const NSMetadataItemStateOrProvinceKey;  
GS_EXPORT NSString* const NSMetadataItemStreamableKey;  
GS_EXPORT NSString* const NSMetadataItemSubjectKey;  
GS_EXPORT NSString* const NSMetadataItemTempoKey;  
GS_EXPORT NSString* const NSMetadataItemTextContentKey;  
GS_EXPORT NSString* const NSMetadataItemThemeKey;  
GS_EXPORT NSString* const NSMetadataItemTimeSignatureKey;  
GS_EXPORT NSString* const NSMetadataItemTimestampKey;  
GS_EXPORT NSString* const NSMetadataItemTitleKey;  
GS_EXPORT NSString* const NSMetadataItemTotalBitRateKey;  
GS_EXPORT NSString* const NSMetadataItemURLKey;
GS_EXPORT NSString* const NSMetadataItemVersionKey;  
GS_EXPORT NSString* const NSMetadataItemVideoBitRateKey;  
GS_EXPORT NSString* const NSMetadataItemWhereFromsKey;  
GS_EXPORT NSString* const NSMetadataItemWhiteBalanceKey;  
GS_EXPORT NSString* const NSMetadataUbiquitousItemContainerDisplayNameKey;  
GS_EXPORT NSString* const NSMetadataUbiquitousItemDownloadingErrorKey; 
GS_EXPORT NSString* const NSMetadataUbiquitousItemDownloadingStatusCurrent;         
GS_EXPORT NSString* const NSMetadataUbiquitousItemDownloadingStatusDownloaded;
GS_EXPORT NSString* const NSMetadataUbiquitousItemDownloadingStatusKey;
GS_EXPORT NSString* const NSMetadataUbiquitousItemDownloadingStatusNotDownloaded;
GS_EXPORT NSString* const NSMetadataUbiquitousItemDownloadRequestedKey;  
GS_EXPORT NSString* const NSMetadataUbiquitousItemHasUnresolvedConflictsKey;
GS_EXPORT NSString* const NSMetadataUbiquitousItemIsDownloadedKey;
GS_EXPORT NSString* const NSMetadataUbiquitousItemIsDownloadingKey; 
GS_EXPORT NSString* const NSMetadataUbiquitousItemIsExternalDocumentKey;  
GS_EXPORT NSString* const NSMetadataUbiquitousItemIsSharedKey;  
GS_EXPORT NSString* const NSMetadataUbiquitousItemIsUploadedKey; 
GS_EXPORT NSString* const NSMetadataUbiquitousItemIsUploadingKey; 
GS_EXPORT NSString* const NSMetadataUbiquitousItemPercentDownloadedKey; 
GS_EXPORT NSString* const NSMetadataUbiquitousItemPercentUploadedKey; 
GS_EXPORT NSString* const NSMetadataUbiquitousItemUploadingErrorKey;  
GS_EXPORT NSString* const NSMetadataUbiquitousItemURLInLocalContainerKey;  
GS_EXPORT NSString* const NSMetadataUbiquitousSharedItemCurrentUserPermissionsKey;  
GS_EXPORT NSString* const NSMetadataUbiquitousSharedItemCurrentUserRoleKey;  
GS_EXPORT NSString* const NSMetadataUbiquitousSharedItemMostRecentEditorNameComponentsKey;  
GS_EXPORT NSString* const NSMetadataUbiquitousSharedItemOwnerNameComponentsKey;  
GS_EXPORT NSString* const NSMetadataUbiquitousSharedItemPermissionsReadOnly; 
GS_EXPORT NSString* const NSMetadataUbiquitousSharedItemPermissionsReadWrite;  
GS_EXPORT NSString* const NSMetadataUbiquitousSharedItemRoleOwner;  
GS_EXPORT NSString* const NSMetadataUbiquitousSharedItemRoleParticipant;  

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSMetadataAttributes_h_GNUSTEP_BASE_INCLUDE */

