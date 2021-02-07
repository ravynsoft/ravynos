/* Implementation of class NSMetadataAttributes
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: heron
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

#include <Foundation/NSMetadataAttributes.h>
#include <Foundation/NSString.h>

NSString *const NSMetadataItemAcquisitionMakeKey = @"NSMetadataItemAcquisitionMakeKey";
NSString *const NSMetadataItemAcquisitionModelKey = @"NSMetadataItemAcquisitionModelKey";
NSString *const NSMetadataItemAlbumKey = @"NSMetadataItemAlbumKey";
NSString *const NSMetadataItemAltitudeKey = @"NSMetadataItemAltitudeKey";
NSString *const NSMetadataItemApertureKey = @"NSMetadataItemApertureKey";
NSString *const NSMetadataItemAppleLoopDescriptorsKey = @"NSMetadataItemAppleLoopDescriptorsKey";
NSString *const NSMetadataItemAppleLoopsKeyFilterTypeKey = @"NSMetadataItemAppleLoopsKeyFilterTypeKey";
NSString *const NSMetadataItemAppleLoopsLoopModeKey = @"NSMetadataItemAppleLoopsLoopModeKey";
NSString *const NSMetadataItemAppleLoopsRootKeyKey = @"NSMetadataItemAppleLoopsRootKeyKey";
NSString *const NSMetadataItemApplicationCategoriesKey = @"NSMetadataItemApplicationCategoriesKey";
NSString *const NSMetadataItemAttributeChangeDateKey = @"NSMetadataItemAttributeChangeDateKey";
NSString *const NSMetadataItemAudiencesKey = @"NSMetadataItemAudiencesKey";
NSString *const NSMetadataItemAudioBitRateKey = @"NSMetadataItemAudioBitRateKey";
NSString *const NSMetadataItemAudioChannelCountKey = @"NSMetadataItemAudioChannelCountKey";
NSString *const NSMetadataItemAudioEncodingApplicationKey = @"NSMetadataItemAudioEncodingApplicationKey";
NSString *const NSMetadataItemAudioSampleRateKey = @"NSMetadataItemAudioSampleRateKey";
NSString *const NSMetadataItemAudioTrackNumberKey = @"NSMetadataItemAudioTrackNumberKey";
NSString *const NSMetadataItemAuthorAddressesKey = @"NSMetadataItemAuthorAddressesKey";
NSString *const NSMetadataItemAuthorEmailAddressesKey = @"NSMetadataItemAuthorEmailAddressesKey";
NSString *const NSMetadataItemAuthorsKey = @"NSMetadataItemAuthorsKey";
NSString *const NSMetadataItemBitsPerSampleKey = @"NSMetadataItemBitsPerSampleKey";
NSString *const NSMetadataItemCameraOwnerKey = @"NSMetadataItemCameraOwnerKey";
NSString *const NSMetadataItemCFBundleIdentifierKey = @"NSMetadataItemCFBundleIdentifierKey";
NSString *const NSMetadataItemCityKey = @"NSMetadataItemCityKey";
NSString *const NSMetadataItemCodecsKey = @"NSMetadataItemCodecsKey";
NSString *const NSMetadataItemColorSpaceKey = @"NSMetadataItemColorSpaceKey";
NSString *const NSMetadataItemCommentKey = @"NSMetadataItemCommentKey";
NSString *const NSMetadataItemComposerKey = @"NSMetadataItemComposerKey";
NSString *const NSMetadataItemContactKeywordsKey = @"NSMetadataItemContactKeywordsKey";
NSString *const NSMetadataItemContentCreationDateKey = @"NSMetadataItemContentCreationDateKey";
NSString *const NSMetadataItemContentModificationDateKey = @"NSMetadataItemContentModificationDateKey";
NSString *const NSMetadataItemContentTypeKey = @"NSMetadataItemContentTypeKey";
NSString *const NSMetadataItemContentTypeTreeKey = @"NSMetadataItemContentTypeTreeKey";
NSString *const NSMetadataItemContributorsKey = @"NSMetadataItemContributorsKey";
NSString *const NSMetadataItemCopyrightKey = @"NSMetadataItemCopyrightKey";
NSString *const NSMetadataItemCountryKey = @"NSMetadataItemCountryKey";
NSString *const NSMetadataItemCoverageKey = @"NSMetadataItemCoverageKey";
NSString *const NSMetadataItemCreatorKey = @"NSMetadataItemCreatorKey";
NSString *const NSMetadataItemDateAddedKey = @"NSMetadataItemDateAddedKey";
NSString *const NSMetadataItemDeliveryTypeKey = @"NSMetadataItemDeliveryTypeKey";
NSString *const NSMetadataItemDescriptionKey = @"NSMetadataItemDescriptionKey";
NSString *const NSMetadataItemDirectorKey = @"NSMetadataItemDirectorKey";
NSString *const NSMetadataItemDisplayNameKey = @"NSMetadataItemDisplayNameKey";
NSString *const NSMetadataItemDownloadedDateKey = @"NSMetadataItemDownloadedDateKey";
NSString *const NSMetadataItemDueDateKey = @"NSMetadataItemDueDateKey";
NSString *const NSMetadataItemDurationSecondsKey = @"NSMetadataItemDurationSecondsKey";
NSString *const NSMetadataItemEditorsKey = @"NSMetadataItemEditorsKey";
NSString *const NSMetadataItemEmailAddressesKey = @"NSMetadataItemEmailAddressesKey";
NSString *const NSMetadataItemEncodingApplicationsKey = @"NSMetadataItemEncodingApplicationsKey";
NSString *const NSMetadataItemExecutableArchitecturesKey = @"NSMetadataItemExecutableArchitecturesKey";
NSString *const NSMetadataItemExecutablePlatformKey = @"NSMetadataItemExecutablePlatformKey";
NSString *const NSMetadataItemEXIFGPSVersionKey = @"NSMetadataItemEXIFGPSVersionKey";
NSString *const NSMetadataItemEXIFVersionKey = @"NSMetadataItemEXIFVersionKey";
NSString *const NSMetadataItemExposureModeKey = @"NSMetadataItemExposureModeKey";
NSString *const NSMetadataItemExposureProgramKey = @"NSMetadataItemExposureProgramKey";
NSString *const NSMetadataItemExposureTimeSecondsKey = @"NSMetadataItemExposureTimeSecondsKey";
NSString *const NSMetadataItemExposureTimeStringKey = @"NSMetadataItemExposureTimeStringKey";
NSString *const NSMetadataItemFinderCommentKey = @"NSMetadataItemFinderCommentKey";
NSString *const NSMetadataItemFlashOnOffKey = @"NSMetadataItemFlashOnOffKey";
NSString *const NSMetadataItemFNumberKey = @"NSMetadataItemFNumberKey";
NSString *const NSMetadataItemFocalLength35mmKey = @"NSMetadataItemFocalLength35mmKey";
NSString *const NSMetadataItemFocalLengthKey = @"NSMetadataItemFocalLengthKey";
NSString *const NSMetadataItemFontsKey = @"NSMetadataItemFontsKey";
NSString *const NSMetadataItemFSContentChangeDateKey = @"NSMetadataItemFSContentChangeDateKey";
NSString *const NSMetadataItemFSCreationDateKey = @"NSMetadataItemFSCreationDateKey";
NSString *const NSMetadataItemFSNameKey = @"NSMetadataItemFSNameKey";
NSString *const NSMetadataItemFSSizeKey = @"NSMetadataItemFSSizeKey";
NSString *const NSMetadataItemGenreKey = @"NSMetadataItemGenreKey";
NSString *const NSMetadataItemGPSAreaInformationKey = @"NSMetadataItemGPSAreaInformationKey";
NSString *const NSMetadataItemGPSDateStampKey = @"NSMetadataItemGPSDateStampKey";
NSString *const NSMetadataItemGPSDestBearingKey = @"NSMetadataItemGPSDestBearingKey";
NSString *const NSMetadataItemGPSDestDistanceKey = @"NSMetadataItemGPSDestDistanceKey";
NSString *const NSMetadataItemGPSDestLatitudeKey = @"NSMetadataItemGPSDestLatitudeKey";
NSString *const NSMetadataItemGPSDestLongitudeKey = @"NSMetadataItemGPSDestLongitudeKey";
NSString *const NSMetadataItemGPSDifferentalKey = @"NSMetadataItemGPSDifferentalKey";
NSString *const NSMetadataItemGPSDOPKey = @"NSMetadataItemGPSDOPKey";
NSString *const NSMetadataItemGPSMapDatumKey = @"NSMetadataItemGPSMapDatumKey";
NSString *const NSMetadataItemGPSMeasureModeKey = @"NSMetadataItemGPSMeasureModeKey";
NSString *const NSMetadataItemGPSProcessingMethodKey = @"NSMetadataItemGPSProcessingMethodKey";
NSString *const NSMetadataItemGPSStatusKey = @"NSMetadataItemGPSStatusKey";
NSString *const NSMetadataItemGPSTrackKey = @"NSMetadataItemGPSTrackKey";
NSString *const NSMetadataItemHasAlphaChannelKey = @"NSMetadataItemHasAlphaChannelKey";
NSString *const NSMetadataItemHeadlineKey = @"NSMetadataItemHeadlineKey";
NSString *const NSMetadataItemIdentifierKey = @"NSMetadataItemIdentifierKey";
NSString *const NSMetadataItemImageDirectionKey = @"NSMetadataItemImageDirectionKey";
NSString *const NSMetadataItemInformationKey = @"NSMetadataItemInformationKey";
NSString *const NSMetadataItemInstantMessageAddressesKey = @"NSMetadataItemInstantMessageAddressesKey";
NSString *const NSMetadataItemInstructionsKey = @"NSMetadataItemInstructionsKey";
NSString *const NSMetadataItemIsApplicationManagedKey = @"NSMetadataItemIsApplicationManagedKey";
NSString *const NSMetadataItemIsGeneralMIDISequenceKey = @"NSMetadataItemIsGeneralMIDISequenceKey";
NSString *const NSMetadataItemIsLikelyJunkKey = @"NSMetadataItemIsLikelyJunkKey";
NSString *const NSMetadataItemISOSpeedKey = @"NSMetadataItemISOSpeedKey";
NSString *const NSMetadataItemIsUbiquitousKey = @"NSMetadataItemIsUbiquitousKey";
NSString *const NSMetadataItemKeySignatureKey = @"NSMetadataItemKeySignatureKey";
NSString *const NSMetadataItemKeywordsKey = @"NSMetadataItemKeywordsKey";
NSString *const NSMetadataItemKindKey = @"NSMetadataItemKindKey";
NSString *const NSMetadataItemLanguagesKey = @"NSMetadataItemLanguagesKey";
NSString *const NSMetadataItemLastUsedDateKey = @"NSMetadataItemLastUsedDateKey";
NSString *const NSMetadataItemLatitudeKey = @"NSMetadataItemLatitudeKey";
NSString *const NSMetadataItemLayerNamesKey = @"NSMetadataItemLayerNamesKey";
NSString *const NSMetadataItemLensModelKey = @"NSMetadataItemLensModelKey";
NSString *const NSMetadataItemLongitudeKey = @"NSMetadataItemLongitudeKey";
NSString *const NSMetadataItemLyricistKey = @"NSMetadataItemLyricistKey";
NSString *const NSMetadataItemMaxApertureKey = @"NSMetadataItemMaxApertureKey";
NSString *const NSMetadataItemMediaTypesKey = @"NSMetadataItemMediaTypesKey";
NSString *const NSMetadataItemMeteringModeKey = @"NSMetadataItemMeteringModeKey";
NSString *const NSMetadataItemMusicalGenreKey = @"NSMetadataItemMusicalGenreKey";
NSString *const NSMetadataItemMusicalInstrumentCategoryKey = @"NSMetadataItemMusicalInstrumentCategoryKey";
NSString *const NSMetadataItemMusicalInstrumentNameKey = @"NSMetadataItemMusicalInstrumentNameKey";
NSString *const NSMetadataItemNamedLocationKey = @"NSMetadataItemNamedLocationKey";
NSString *const NSMetadataItemNumberOfPagesKey = @"NSMetadataItemNumberOfPagesKey";
NSString *const NSMetadataItemOrganizationsKey = @"NSMetadataItemOrganizationsKey";
NSString *const NSMetadataItemOrientationKey = @"NSMetadataItemOrientationKey";
NSString *const NSMetadataItemOriginalFormatKey = @"NSMetadataItemOriginalFormatKey";
NSString *const NSMetadataItemOriginalSourceKey = @"NSMetadataItemOriginalSourceKey";
NSString *const NSMetadataItemPageHeightKey = @"NSMetadataItemPageHeightKey";
NSString *const NSMetadataItemPageWidthKey = @"NSMetadataItemPageWidthKey";
NSString *const NSMetadataItemParticipantsKey = @"NSMetadataItemParticipantsKey";
NSString *const NSMetadataItemPathKey = @"NSMetadataItemPathKey";
NSString *const NSMetadataItemPerformersKey = @"NSMetadataItemPerformersKey";
NSString *const NSMetadataItemPhoneNumbersKey = @"NSMetadataItemPhoneNumbersKey";
NSString *const NSMetadataItemPixelCountKey = @"NSMetadataItemPixelCountKey";
NSString *const NSMetadataItemPixelHeightKey = @"NSMetadataItemPixelHeightKey";
NSString *const NSMetadataItemPixelWidthKey = @"NSMetadataItemPixelWidthKey";
NSString *const NSMetadataItemProducerKey = @"NSMetadataItemProducerKey";
NSString *const NSMetadataItemProfileNameKey = @"NSMetadataItemProfileNameKey";
NSString *const NSMetadataItemProjectsKey = @"NSMetadataItemProjectsKey";
NSString *const NSMetadataItemPublishersKey = @"NSMetadataItemPublishersKey";
NSString *const NSMetadataItemRecipientAddressesKey = @"NSMetadataItemRecipientAddressesKey";
NSString *const NSMetadataItemRecipientEmailAddressesKey = @"NSMetadataItemRecipientEmailAddressesKey";
NSString *const NSMetadataItemRecipientsKey = @"NSMetadataItemRecipientsKey";
NSString *const NSMetadataItemRecordingDateKey = @"NSMetadataItemRecordingDateKey";
NSString *const NSMetadataItemRecordingYearKey = @"NSMetadataItemRecordingYearKey";
NSString *const NSMetadataItemRedEyeOnOffKey = @"NSMetadataItemRedEyeOnOffKey";
NSString *const NSMetadataItemResolutionHeightDPIKey = @"NSMetadataItemResolutionHeightDPIKey";
NSString *const NSMetadataItemResolutionWidthDPIKey = @"NSMetadataItemResolutionWidthDPIKey";
NSString *const NSMetadataItemRightsKey = @"NSMetadataItemRightsKey";
NSString *const NSMetadataItemSecurityMethodKey = @"NSMetadataItemSecurityMethodKey";
NSString *const NSMetadataItemSpeedKey = @"NSMetadataItemSpeedKey";
NSString *const NSMetadataItemStarRatingKey = @"NSMetadataItemStarRatingKey";
NSString *const NSMetadataItemStateOrProvinceKey = @"NSMetadataItemStateOrProvinceKey";
NSString *const NSMetadataItemStreamableKey = @"NSMetadataItemStreamableKey";
NSString *const NSMetadataItemSubjectKey = @"NSMetadataItemSubjectKey";
NSString *const NSMetadataItemTempoKey = @"NSMetadataItemTempoKey";
NSString *const NSMetadataItemTextContentKey = @"NSMetadataItemTextContentKey";
NSString *const NSMetadataItemThemeKey = @"NSMetadataItemThemeKey";
NSString *const NSMetadataItemTimeSignatureKey = @"NSMetadataItemTimeSignatureKey";
NSString *const NSMetadataItemTimestampKey = @"NSMetadataItemTimestampKey";
NSString *const NSMetadataItemTitleKey = @"NSMetadataItemTitleKey";
NSString *const NSMetadataItemTotalBitRateKey = @"NSMetadataItemTotalBitRateKey";
NSString *const NSMetadataItemURLKey = @"NSMetadataItemURLKey";
NSString *const NSMetadataItemVersionKey = @"NSMetadataItemVersionKey";
NSString *const NSMetadataItemVideoBitRateKey = @"NSMetadataItemVideoBitRateKey";
NSString *const NSMetadataItemWhereFromsKey = @"NSMetadataItemWhereFromsKey";
NSString *const NSMetadataItemWhiteBalanceKey = @"NSMetadataItemWhiteBalanceKey";
NSString *const NSMetadataUbiquitousItemContainerDisplayNameKey = @"NSMetadataUbiquitousItemContainerDisplayNameKey";
NSString *const NSMetadataUbiquitousItemDownloadingErrorKey = @"NSMetadataUbiquitousItemDownloadingErrorKey";
NSString *const NSMetadataUbiquitousItemDownloadingStatusCurrent = @"NSMetadataUbiquitousItemDownloadingStatusCurrent";
NSString *const NSMetadataUbiquitousItemDownloadingStatusDownloaded = @"NSMetadataUbiquitousItemDownloadingStatusDownloaded";
NSString *const NSMetadataUbiquitousItemDownloadingStatusKey = @"NSMetadataUbiquitousItemDownloadingStatusKey";
NSString *const NSMetadataUbiquitousItemDownloadingStatusNotDownloaded = @"NSMetadataUbiquitousItemDownloadingStatusNotDownloaded";
NSString *const NSMetadataUbiquitousItemDownloadRequestedKey = @"NSMetadataUbiquitousItemDownloadRequestedKey";
NSString *const NSMetadataUbiquitousItemHasUnresolvedConflictsKey = @"NSMetadataUbiquitousItemHasUnresolvedConflictsKey";
NSString *const NSMetadataUbiquitousItemIsDownloadedKey = @"NSMetadataUbiquitousItemIsDownloadedKey";
NSString *const NSMetadataUbiquitousItemIsDownloadingKey = @"NSMetadataUbiquitousItemIsDownloadingKey";
NSString *const NSMetadataUbiquitousItemIsExternalDocumentKey = @"NSMetadataUbiquitousItemIsExternalDocumentKey";
NSString *const NSMetadataUbiquitousItemIsSharedKey = @"NSMetadataUbiquitousItemIsSharedKey";
NSString *const NSMetadataUbiquitousItemIsUploadedKey = @"NSMetadataUbiquitousItemIsUploadedKey";
NSString *const NSMetadataUbiquitousItemIsUploadingKey = @"NSMetadataUbiquitousItemIsUploadingKey";
NSString *const NSMetadataUbiquitousItemPercentDownloadedKey = @"NSMetadataUbiquitousItemPercentDownloadedKey";
NSString *const NSMetadataUbiquitousItemPercentUploadedKey = @"NSMetadataUbiquitousItemPercentUploadedKey";
NSString *const NSMetadataUbiquitousItemUploadingErrorKey = @"NSMetadataUbiquitousItemUploadingErrorKey";
NSString *const NSMetadataUbiquitousItemURLInLocalContainerKey = @"NSMetadataUbiquitousItemURLInLocalContainerKey";
NSString *const NSMetadataUbiquitousSharedItemCurrentUserPermissionsKey = @"NSMetadataUbiquitousSharedItemCurrentUserPermissionsKey";
NSString *const NSMetadataUbiquitousSharedItemCurrentUserRoleKey = @"NSMetadataUbiquitousSharedItemCurrentUserRoleKey";
NSString *const NSMetadataUbiquitousSharedItemMostRecentEditorNameComponentsKey = @"NSMetadataUbiquitousSharedItemMostRecentEditorNameComponentsKey";
NSString *const NSMetadataUbiquitousSharedItemOwnerNameComponentsKey = @"NSMetadataUbiquitousSharedItemOwnerNameComponentsKey";
NSString *const NSMetadataUbiquitousSharedItemPermissionsReadOnly = @"NSMetadataUbiquitousSharedItemPermissionsReadOnly";
NSString *const NSMetadataUbiquitousSharedItemPermissionsReadWrite = @"NSMetadataUbiquitousSharedItemPermissionsReadWrite";
NSString *const NSMetadataUbiquitousSharedItemRoleOwner = @"NSMetadataUbiquitousSharedItemRoleOwner";
NSString *const NSMetadataUbiquitousSharedItemRoleParticipant = @"NSMetadataUbiquitousSharedItemRoleParticipant";
