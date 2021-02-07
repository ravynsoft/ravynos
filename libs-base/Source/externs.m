/** All of the external data
   Copyright (C) 1997 Free Software Foundation, Inc.

   Written by:  Scott Christley <scottc@net-community.com>
   Date: August 1997

   This file is part of the GNUstep Base Library.

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

#import "common.h"

#import "Foundation/NSArray.h"
#import "Foundation/NSException.h"

#import "GSPrivate.h"

/*
 PENDING some string constants are scattered about in the class impl
         files and should be moved here
         furthermore, the test for this in Testing/exported-strings.m
         needs to be updated
*/


/*
 * NSConnection Notification Strings.
 */
GS_DECLARE NSString *NSConnectionDidDieNotification = @"NSConnectionDidDieNotification";

GS_DECLARE NSString *NSConnectionDidInitializeNotification = @"NSConnectionDidInitializeNotification";


/*
 * NSDistributedNotificationCenter types.
 */
GS_DECLARE NSString *NSLocalNotificationCenterType = @"NSLocalNotificationCenterType";
GS_DECLARE NSString *GSNetworkNotificationCenterType = @"GSNetworkNotificationCenterType";
GS_DECLARE NSString *GSPublicNotificationCenterType = @"GSPublicNotificationCenterType";

/*
 * NSThread Notifications
 */
GS_DECLARE NSString *NSWillBecomeMultiThreadedNotification = @"NSWillBecomeMultiThreadedNotification";

GS_DECLARE NSString *NSThreadDidStartNotification = @"NSThreadDidStartNotification";

GS_DECLARE NSString *NSThreadWillExitNotification = @"NSThreadWillExitNotification";


/*
 * Port Notifications
 */

GS_DECLARE NSString *NSPortDidBecomeInvalidNotification = @"NSPortDidBecomeInvalidNotification";

/* NSTask notifications */
GS_DECLARE NSString *NSTaskDidTerminateNotification = @"NSTaskDidTerminateNotification";

/* NSUndoManager notifications */
GS_DECLARE NSString *NSUndoManagerCheckpointNotification = @"NSUndoManagerCheckpointNotification";

GS_DECLARE NSString *NSUndoManagerDidOpenUndoGroupNotification = @"NSUndoManagerDidOpenUndoGroupNotification";

GS_DECLARE NSString *NSUndoManagerDidRedoChangeNotification = @"NSUndoManagerDidRedoChangeNotification";

GS_DECLARE NSString *NSUndoManagerDidUndoChangeNotification = @"NSUndoManagerDidUndoChangeNotification";

GS_DECLARE NSString *NSUndoManagerWillCloseUndoGroupNotification = @"NSUndoManagerWillCloseUndoGroupNotification";

GS_DECLARE NSString *NSUndoManagerWillRedoChangeNotification = @"NSUndoManagerWillRedoChangeNotification";

GS_DECLARE NSString *NSUndoManagerWillUndoChangeNotification = @"NSUndoManagerWillUndoChangeNotification";

/*
 * NSUbiquitousKeyValueStore notifications
 */
GS_DECLARE NSString *NSUbiquitousKeyValueStoreDidChangeExternallyNotification = @"NSUbiquitousKeyValueStoreDidChangeExternallyNotification";
GS_DECLARE NSString *NSUbiquitousKeyValueStoreChangeReasonKey = @"NSUbiquitousKeyValueStoreChangeReasonKey";

/* NSURL constants */
GS_DECLARE NSString *NSURLFileScheme = @"file";

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
GS_DECLARE NSString *NSURLNameKey = @"NSURLNameKey";
GS_DECLARE NSString *NSURLLocalizedNameKey = @"NSURLLocalizedNameKey";
GS_DECLARE NSString *NSURLIsRegularFileKey = @"NSURLIsRegularFileKey";
GS_DECLARE NSString *NSURLIsDirectoryKey = @"NSURLIsDirectoryKey";
GS_DECLARE NSString *NSURLIsSymbolicLinkKey = @"NSURLIsSymbolicLinkKey";
GS_DECLARE NSString *NSURLIsVolumeKey = @"NSURLIsVolumeKey";
GS_DECLARE NSString *NSURLIsPackageKey = @"NSURLIsPackageKey";
GS_DECLARE NSString *NSURLIsSystemImmutableKey = @"NSURLIsSystemImmutableKey";
GS_DECLARE NSString *NSURLIsUserImmutableKey = @"NSURLIsUserImmutableKey";
GS_DECLARE NSString *NSURLIsHiddenKey = @"NSURLIsHiddenKey";
GS_DECLARE NSString *NSURLHasHiddenExtensionKey = @"NSURLHasHiddenExtensionKey";
GS_DECLARE NSString *NSURLCreationDateKey = @"NSURLCreationDateKey";
GS_DECLARE NSString *NSURLContentAccessDateKey = @"NSURLContentAccessDateKey";
GS_DECLARE NSString *NSURLContentModificationDateKey = @"NSURLContentModificationDateKey";
GS_DECLARE NSString *NSURLAttributeModificationDateKey = @"NSURLAttributeModificationDateKey";
GS_DECLARE NSString *NSURLLinkCountKey = @"NSURLLinkCountKey";
GS_DECLARE NSString *NSURLParentDirectoryURLKey = @"NSURLParentDirectoryURLKey";
GS_DECLARE NSString *NSURLVolumeURLKey = @"NSURLVolumeURLKey";
GS_DECLARE NSString *NSURLTypeIdentifierKey = @"NSURLTypeIdentifierKey";
GS_DECLARE NSString *NSURLLocalizedTypeDescriptionKey = @"NSURLLocalizedTypeDescriptionKey";
GS_DECLARE NSString *NSURLLabelNumberKey = @"NSURLLabelNumberKey";
GS_DECLARE NSString *NSURLLabelColorKey = @"NSURLLabelColorKey";
GS_DECLARE NSString *NSURLLocalizedLabelKey = @"NSURLLocalizedLabelKey";
GS_DECLARE NSString *NSURLEffectiveIconKey = @"NSURLEffectiveIconKey";
GS_DECLARE NSString *NSURLCustomIconKey = @"NSURLCustomIconKey";
GS_DECLARE NSString *NSURLFileSizeKey = @"NSURLFileSizeKey";
GS_DECLARE NSString *NSURLFileAllocatedSizeKey = @"NSURLFileAllocatedSizeKey";
GS_DECLARE NSString *NSURLIsAliasFileKey = @"NSURLIsAliasFileKey";
GS_DECLARE NSString *NSURLVolumeLocalizedFormatDescriptionKey = @"NSURLVolumeLocalizedFormatDescriptionKey";
GS_DECLARE NSString *NSURLVolumeTotalCapacityKey = @"NSURLVolumeTotalCapacityKey";
GS_DECLARE NSString *NSURLVolumeAvailableCapacityKey = @"NSURLVolumeAvailableCapacityKey";
GS_DECLARE NSString *NSURLVolumeResourceCountKey = @"NSURLVolumeResourceCountKey";
GS_DECLARE NSString *NSURLVolumeSupportsPersistentIDsKey = @"NSURLVolumeSupportsPersistentIDsKey";
GS_DECLARE NSString *NSURLVolumeSupportsSymbolicLinksKey = @"NSURLVolumeSupportsSymbolicLinksKey";
GS_DECLARE NSString *NSURLVolumeSupportsHardLinksKey = @"NSURLVolumeSupportsHardLinksKey";
GS_DECLARE NSString *NSURLVolumeSupportsJournalingKey = @"NSURLVolumeSupportsJournalingKey";
GS_DECLARE NSString *NSURLVolumeIsJournalingKey = @"NSURLVolumeIsJournalingKey";
GS_DECLARE NSString *NSURLVolumeSupportsSparseFilesKey = @"NSURLVolumeSupportsSparseFilesKey";
GS_DECLARE NSString *NSURLVolumeSupportsZeroRunsKey = @"NSURLVolumeSupportsZeroRunsKey";
GS_DECLARE NSString *NSURLVolumeSupportsCaseSensitiveNamesKey = @"NSURLVolumeSupportsCaseSensitiveNamesKey";
GS_DECLARE NSString *NSURLVolumeSupportsCasePreservedNamesKey = @"NSURLVolumeSupportsCasePreservedNamesKey";
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_7, GS_API_LATEST)
GS_DECLARE NSString *NSURLFileResourceIdentifierKey = @"NSURLFileResourceIdentifierKey";
GS_DECLARE NSString *NSURLVolumeIdentifierKey = @"NSURLVolumeIdentifierKey";
GS_DECLARE NSString *NSURLPreferredIOBlockSizeKey = @"NSURLPreferredIOBlockSizeKey";
GS_DECLARE NSString *NSURLIsReadableKey = @"NSURLIsReadableKey";
GS_DECLARE NSString *NSURLIsWritableKey = @"NSURLIsWritableKey";
GS_DECLARE NSString *NSURLIsExecutableKey = @"NSURLIsExecutableKey";
GS_DECLARE NSString *NSURLFileSecurityKey = @"NSURLFileSecurityKey";
GS_DECLARE NSString *NSURLIsMountTriggerKey = @"NSURLIsMountTriggerKey";
GS_DECLARE NSString *NSURLFileResourceTypeKey = @"NSURLFileResourceTypeKey";
GS_DECLARE NSString *NSURLTotalFileSizeKey = @"NSURLTotalFileSizeKey";
GS_DECLARE NSString *NSURLTotalFileAllocatedSizeKey = @"NSURLTotalFileAllocatedSizeKey";
GS_DECLARE NSString *NSURLVolumeSupportsRootDirectoryDatesKey = @"NSURLVolumeSupportsRootDirectoryDatesKey";
GS_DECLARE NSString *NSURLVolumeSupportsVolumeSizesKey = @"NSURLVolumeSupportsVolumeSizesKey";
GS_DECLARE NSString *NSURLVolumeSupportsRenamingKey = @"NSURLVolumeSupportsRenamingKey";
GS_DECLARE NSString *NSURLVolumeSupportsAdvisoryFileLockingKey = @"NSURLVolumeSupportsAdvisoryFileLockingKey";
GS_DECLARE NSString *NSURLVolumeSupportsExtendedSecurityKey = @"NSURLVolumeSupportsExtendedSecurityKey";
GS_DECLARE NSString *NSURLVolumeIsBrowsableKey = @"NSURLVolumeIsBrowsableKey";
GS_DECLARE NSString *NSURLVolumeMaximumFileSizeKey = @"NSURLVolumeMaximumFileSizeKey";
GS_DECLARE NSString *NSURLVolumeIsEjectableKey = @"NSURLVolumeIsEjectableKey";
GS_DECLARE NSString *NSURLVolumeIsRemovableKey = @"NSURLVolumeIsRemovableKey";
GS_DECLARE NSString *NSURLVolumeIsInternalKey = @"NSURLVolumeIsInternalKey";
GS_DECLARE NSString *NSURLVolumeIsAutomountedKey = @"NSURLVolumeIsAutomountedKey";
GS_DECLARE NSString *NSURLVolumeIsLocalKey = @"NSURLVolumeIsLocalKey";
GS_DECLARE NSString *NSURLVolumeIsReadOnlyKey = @"NSURLVolumeIsReadOnlyKey";
GS_DECLARE NSString *NSURLVolumeCreationDateKey = @"NSURLVolumeCreationDateKey";
GS_DECLARE NSString *NSURLVolumeURLForRemountingKey = @"NSURLVolumeURLForRemountingKey";
GS_DECLARE NSString *NSURLVolumeUUIDStringKey = @"NSURLVolumeUUIDStringKey";
GS_DECLARE NSString *NSURLVolumeNameKey = @"NSURLVolumeNameKey";
GS_DECLARE NSString *NSURLVolumeLocalizedNameKey = @"NSURLVolumeLocalizedNameKey";
GS_DECLARE NSString *NSURLIsUbiquitousItemKey = @"NSURLIsUbiquitousItemKey";
GS_DECLARE NSString *NSURLUbiquitousItemHasUnresolvedConflictsKey = @"NSURLUbiquitousItemHasUnresolvedConflictsKey";
GS_DECLARE NSString *NSURLUbiquitousItemIsDownloadingKey = @"NSURLUbiquitousItemIsDownloadingKey";
GS_DECLARE NSString *NSURLUbiquitousItemIsUploadedKey = @"NSURLUbiquitousItemIsUploadedKey";
GS_DECLARE NSString *NSURLUbiquitousItemIsUploadingKey = @"NSURLUbiquitousItemIsUploadingKey";
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_8, GS_API_LATEST)
GS_DECLARE NSString *NSURLIsExcludedFromBackupKey = @"NSURLIsExcludedFromBackupKey";
GS_DECLARE NSString *NSURLPathKey = @"NSURLPathKey";
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_9, GS_API_LATEST)
GS_DECLARE NSString *NSURLTagNamesKey = @"NSURLTagNamesKey";
GS_DECLARE NSString *NSURLUbiquitousItemDownloadingStatusKey = @"NSURLUbiquitousItemDownloadingStatusKey";
GS_DECLARE NSString *NSURLUbiquitousItemDownloadingErrorKey = @"NSURLUbiquitousItemDownloadingErrorKey";
GS_DECLARE NSString *NSURLUbiquitousItemUploadingErrorKey = @"NSURLUbiquitousItemUploadingErrorKey";
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_10, GS_API_LATEST)
GS_DECLARE NSString *NSURLGenerationIdentifierKey = @"NSURLGenerationIdentifierKey";
GS_DECLARE NSString *NSURLDocumentIdentifierKey = @"NSURLDocumentIdentifierKey";
GS_DECLARE NSString *NSURLAddedToDirectoryDateKey = @"NSURLAddedToDirectoryDateKey";
GS_DECLARE NSString *NSURLQuarantinePropertiesKey = @"NSURLQuarantinePropertiesKey";
GS_DECLARE NSString *NSThumbnail1024x1024SizeKey = @"NSThumbnail1024x1024SizeKey";
GS_DECLARE NSString *NSURLUbiquitousItemDownloadRequestedKey = @"NSURLUbiquitousItemDownloadRequestedKey";
GS_DECLARE NSString *NSURLUbiquitousItemContainerDisplayNameKey = @"NSURLUbiquitousItemContainerDisplayNameKey";
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_11, GS_API_LATEST)
GS_DECLARE NSString *NSURLIsApplicationKey = @"NSURLIsApplicationKey";
GS_DECLARE NSString *NSURLApplicationIsScriptableKey = @"NSURLApplicationIsScriptableKey";
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_7, GS_API_LATEST)
GS_DECLARE NSString *NSURLFileResourceTypeNamedPipe = @"NSURLFileResourceTypeNamedPipe";
GS_DECLARE NSString *NSURLFileResourceTypeCharacterSpecial = @"NSURLFileResourceTypeCharacterSpecial";
GS_DECLARE NSString *NSURLFileResourceTypeDirectory = @"NSURLFileResourceTypeDirectory";
GS_DECLARE NSString *NSURLFileResourceTypeBlockSpecial = @"NSURLFileResourceTypeBlockSpecial";
GS_DECLARE NSString *NSURLFileResourceTypeRegular = @"NSURLFileResourceTypeRegular";
GS_DECLARE NSString *NSURLFileResourceTypeSymbolicLink = @"NSURLFileResourceTypeSymbolicLink";
GS_DECLARE NSString *NSURLFileResourceTypeSocket = @"NSURLFileResourceTypeSocket";
GS_DECLARE NSString *NSURLFileResourceTypeUnknown = @"NSURLFileResourceTypeUnknown";
#endif

/** Possible values for Ubiquitous Item Downloading Key **/
#if OS_API_VERSION(MAC_OS_X_VERSION_10_9, GS_API_LATEST)
GS_DECLARE NSString *NSURLUbiquitousItemDownloadingStatusNotDownloaded = @"NSURLUbiquitousItemDownloadingStatusNotDownloaded";
GS_DECLARE NSString *NSURLUbiquitousItemDownloadingStatusDownloaded = @"NSURLUbiquitousItemDownloadingStatusDownloaded";
GS_DECLARE NSString *NSURLUbiquitousItemDownloadingStatusCurrent = @"NSURLUbiquitousItemDownloadingStatusCurrent";
#endif

/* RunLoop modes */
GS_DECLARE NSString *NSConnectionReplyMode = @"NSConnectionReplyMode";

/* NSValueTransformer constants */
GS_DECLARE NSString *const NSNegateBooleanTransformerName
  = @"NSNegateBoolean";
GS_DECLARE NSString *const NSIsNilTransformerName
  = @"NSIsNil";
GS_DECLARE NSString *const NSIsNotNilTransformerName
  = @"NSIsNotNil"; 
GS_DECLARE NSString *const NSUnarchiveFromDataTransformerName
  = @"NSUnarchiveFromData";


/* Standard domains */
GS_DECLARE NSString *NSArgumentDomain = @"NSArgumentDomain";

GS_DECLARE NSString *NSGlobalDomain = @"NSGlobalDomain";

GS_DECLARE NSString *NSRegistrationDomain = @"NSRegistrationDomain";

GS_DECLARE NSString *GSConfigDomain = @"GSConfigDomain";


/* Public notification */
GS_DECLARE NSString *NSUserDefaultsDidChangeNotification = @"NSUserDefaultsDidChangeNotification";


/* Keys for language-dependent information */
GS_DECLARE NSString *NSWeekDayNameArray = @"NSWeekDayNameArray";

GS_DECLARE NSString *NSShortWeekDayNameArray = @"NSShortWeekDayNameArray";

GS_DECLARE NSString *NSMonthNameArray = @"NSMonthNameArray";

GS_DECLARE NSString *NSShortMonthNameArray = @"NSShortMonthNameArray";

GS_DECLARE NSString *NSTimeFormatString = @"NSTimeFormatString";

GS_DECLARE NSString *NSDateFormatString = @"NSDateFormatString";

GS_DECLARE NSString *NSShortDateFormatString = @"NSShortDateFormatString";

GS_DECLARE NSString *NSTimeDateFormatString = @"NSTimeDateFormatString";

GS_DECLARE NSString *NSShortTimeDateFormatString = @"NSShortTimeDateFormatString";

GS_DECLARE NSString *NSCurrencySymbol = @"NSCurrencySymbol";

GS_DECLARE NSString *NSDecimalSeparator = @"NSDecimalSeparator";

GS_DECLARE NSString *NSThousandsSeparator = @"NSThousandsSeparator";

GS_DECLARE NSString *NSInternationalCurrencyString = @"NSInternationalCurrencyString";

GS_DECLARE NSString *NSCurrencyString = @"NSCurrencyString";

GS_DECLARE NSString *NSNegativeCurrencyFormatString = @"NSNegativeCurrencyFormatString";

GS_DECLARE NSString *NSPositiveCurrencyFormatString = @"NSPositiveCurrencyFormatString";

GS_DECLARE NSString *NSDecimalDigits = @"NSDecimalDigits";

GS_DECLARE NSString *NSAMPMDesignation = @"NSAMPMDesignation";


GS_DECLARE NSString *NSHourNameDesignations = @"NSHourNameDesignations";

GS_DECLARE NSString *NSYearMonthWeekDesignations = @"NSYearMonthWeekDesignations";

GS_DECLARE NSString *NSEarlierTimeDesignations = @"NSEarlierTimeDesignations";

GS_DECLARE NSString *NSLaterTimeDesignations = @"NSLaterTimeDesignations";

GS_DECLARE NSString *NSThisDayDesignations = @"NSThisDayDesignations";

GS_DECLARE NSString *NSNextDayDesignations = @"NSNextDayDesignations";

GS_DECLARE NSString *NSNextNextDayDesignations = @"NSNextNextDayDesignations";

GS_DECLARE NSString *NSPriorDayDesignations = @"NSPriorDayDesignations";

GS_DECLARE NSString *NSDateTimeOrdering = @"NSDateTimeOrdering";


/* These are in OPENSTEP 4.2 */
GS_DECLARE NSString *NSLanguageCode = @"NSLanguageCode";

GS_DECLARE NSString *NSLanguageName = @"NSLanguageName";

GS_DECLARE NSString *NSFormalName = @"NSFormalName";

/* For GNUstep */
GS_DECLARE NSString *GSLocale = @"GSLocale";


/*
 * Keys for the NSDictionary returned by [NSConnection -statistics]
 */
/* These in OPENSTEP 4.2 */
GS_DECLARE NSString *NSConnectionRepliesReceived = @"NSConnectionRepliesReceived";

GS_DECLARE NSString *NSConnectionRepliesSent = @"NSConnectionRepliesSent";

GS_DECLARE NSString *NSConnectionRequestsReceived = @"NSConnectionRequestsReceived";

GS_DECLARE NSString *NSConnectionRequestsSent = @"NSConnectionRequestsSent";

/* These Are GNUstep extras */
GS_DECLARE NSString *NSConnectionLocalCount = @"NSConnectionLocalCount";

GS_DECLARE NSString *NSConnectionProxyCount = @"NSConnectionProxyCount";

/* Class description notification */
GS_DECLARE NSString *NSClassDescriptionNeededForClassNotification = @"NSClassDescriptionNeededForClassNotification";


/*
 * Optimization function called when NSObject is initialised.
 * We replace all the constant strings so they can
 * cache their hash values and be used much more efficiently as keys in
 * dictionaries etc.
 * We initialize with constant strings so that
 * code executed before NSObject +initialize calls us,
 * will have valid values.
 */

void
GSPrivateBuildStrings()
{
  static Class	NSStringClass = 0;

  if (NSStringClass == 0)
    {
      NSStringClass = [NSString class];

      /*
       * Ensure that NSString is initialized ... because we are called
       * from [NSObject +initialize] which might be executing as a
       * result of a call to [NSString +initialize] !
       * Use performSelector: to avoid compiler warning about clash of
       * return value types in two different versions of initialize.
       */
      [NSStringClass performSelector: @selector(initialize)];

      GS_REPLACE_CONSTANT_STRING(GSNetworkNotificationCenterType);
      GS_REPLACE_CONSTANT_STRING(NSAMPMDesignation);
      GS_REPLACE_CONSTANT_STRING(NSArgumentDomain);
      GS_REPLACE_CONSTANT_STRING(NSClassDescriptionNeededForClassNotification);
      GS_REPLACE_CONSTANT_STRING(NSConnectionDidDieNotification);
      GS_REPLACE_CONSTANT_STRING(NSConnectionDidInitializeNotification);
      GS_REPLACE_CONSTANT_STRING(NSConnectionLocalCount);
      GS_REPLACE_CONSTANT_STRING(NSConnectionProxyCount);
      GS_REPLACE_CONSTANT_STRING(NSConnectionRepliesReceived);
      GS_REPLACE_CONSTANT_STRING(NSConnectionRepliesSent);
      GS_REPLACE_CONSTANT_STRING(NSConnectionReplyMode);
      GS_REPLACE_CONSTANT_STRING(NSConnectionRequestsReceived);
      GS_REPLACE_CONSTANT_STRING(NSConnectionRequestsSent);
      GS_REPLACE_CONSTANT_STRING(NSCurrencyString);
      GS_REPLACE_CONSTANT_STRING(NSCurrencySymbol);
      GS_REPLACE_CONSTANT_STRING(NSDateFormatString);
      GS_REPLACE_CONSTANT_STRING(NSDateTimeOrdering);
      GS_REPLACE_CONSTANT_STRING(NSDecimalDigits);
      GS_REPLACE_CONSTANT_STRING(NSDecimalSeparator);
      GS_REPLACE_CONSTANT_STRING(NSEarlierTimeDesignations);
      GS_REPLACE_CONSTANT_STRING(NSFormalName);
      GS_REPLACE_CONSTANT_STRING(NSGlobalDomain);
      GS_REPLACE_CONSTANT_STRING(NSHourNameDesignations);
      GS_REPLACE_CONSTANT_STRING(NSInternationalCurrencyString);
      GS_REPLACE_CONSTANT_STRING(NSLanguageCode);
      GS_REPLACE_CONSTANT_STRING(NSLanguageName);
      GS_REPLACE_CONSTANT_STRING(NSLaterTimeDesignations);
      GS_REPLACE_CONSTANT_STRING(GSLocale);
      GS_REPLACE_CONSTANT_STRING(NSLocalNotificationCenterType);
      GS_REPLACE_CONSTANT_STRING(NSMonthNameArray);
      GS_REPLACE_CONSTANT_STRING(NSNegativeCurrencyFormatString);
      GS_REPLACE_CONSTANT_STRING(NSNextDayDesignations);
      GS_REPLACE_CONSTANT_STRING(NSNextNextDayDesignations);
      GS_REPLACE_CONSTANT_STRING(NSPortDidBecomeInvalidNotification);
      GS_REPLACE_CONSTANT_STRING(NSPositiveCurrencyFormatString);
      GS_REPLACE_CONSTANT_STRING(NSPriorDayDesignations);
      GS_REPLACE_CONSTANT_STRING(NSRegistrationDomain);
      GS_REPLACE_CONSTANT_STRING(NSShortDateFormatString);
      GS_REPLACE_CONSTANT_STRING(NSShortMonthNameArray);
      GS_REPLACE_CONSTANT_STRING(NSShortTimeDateFormatString);
      GS_REPLACE_CONSTANT_STRING(NSShortWeekDayNameArray);
      GS_REPLACE_CONSTANT_STRING(NSTaskDidTerminateNotification);
      GS_REPLACE_CONSTANT_STRING(NSThisDayDesignations);
      GS_REPLACE_CONSTANT_STRING(NSThousandsSeparator);
      GS_REPLACE_CONSTANT_STRING(NSThreadDidStartNotification);
      GS_REPLACE_CONSTANT_STRING(NSThreadWillExitNotification);
      GS_REPLACE_CONSTANT_STRING(NSTimeDateFormatString);
      GS_REPLACE_CONSTANT_STRING(NSTimeFormatString);
      GS_REPLACE_CONSTANT_STRING(NSUndoManagerCheckpointNotification);
      GS_REPLACE_CONSTANT_STRING(NSUndoManagerDidOpenUndoGroupNotification);
      GS_REPLACE_CONSTANT_STRING(NSUndoManagerDidRedoChangeNotification);
      GS_REPLACE_CONSTANT_STRING(NSUndoManagerDidUndoChangeNotification);
      GS_REPLACE_CONSTANT_STRING(NSUndoManagerWillCloseUndoGroupNotification);
      GS_REPLACE_CONSTANT_STRING(NSUndoManagerWillRedoChangeNotification);
      GS_REPLACE_CONSTANT_STRING(NSUndoManagerWillUndoChangeNotification);
      GS_REPLACE_CONSTANT_STRING(NSURLFileScheme);
      GS_REPLACE_CONSTANT_STRING(NSUserDefaultsDidChangeNotification);
      GS_REPLACE_CONSTANT_STRING(NSWeekDayNameArray);
      GS_REPLACE_CONSTANT_STRING(NSWillBecomeMultiThreadedNotification);
      GS_REPLACE_CONSTANT_STRING(NSYearMonthWeekDesignations);
    }
}



/* For bug in gcc 3.1. See NSByteOrder.h */
void _gcc3_1_hack(void){}
