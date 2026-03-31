//
//  signposts.h
//  kext_tools
//
//  Copyright 2018 Apple Inc. All rights reserved.
//
#pragma once

#include <os/log.h>
#include <os/signpost.h>
#include <os/signpost_private.h>

// shared events
#define SIGNPOST_EVENT_BOOTCACHE_UPDATE_REASON  "BootcacheUpdateReason"
#define SIGNPOST_EVENT_CSFDE_NEEDS_UPDATE       "CSFDENeedsUpdate"
#define SIGNPOST_EVENT_KEXT_URL                 "KextURL"
#define SIGNPOST_EVENT_KEXT_BUNDLE_ID           "KextBundleID"
#define SIGNPOST_EVENT_RESULT                   "Result"
#define SIGNPOST_EVENT_POWER_ASSERTION          "PowerAssertion"
#define SIGNPOST_EVENT_PRELINKED_KERNEL_PATH    "PrelinkedKernelPath"
#define SIGNPOST_EVENT_VOLUME_URL               "VolumeURL"
#define SIGNPOST_EVENT_VOLUME_WATCHED           "VolumeWatched"
#define SIGNPOST_EVENT_FORK_KEXTCACHE           "ForkKextcache"
#define SIGNPOST_EVENT_KEXTALLOW_LINE           "KextAllowListLine"

// kextd intervals
#define SIGNPOST_KEXTD_INIT                     "KextdInit"
#define SIGNPOST_KEXTD_CHECK_REBUILD            "KextdCheckRebuild"
#define SIGNPOST_KEXTD_RESCAN_EXTENSIONS        "KextdRescanExtensions"
#define SIGNPOST_KEXTD_PERSONALITY_SCRAPE       "KextdPersonalityScrape"
#define SIGNPOST_KEXTD_DEXT_LAUNCH              "KextdDextLaunch"
#define SIGNPOST_KEXTD_KERNEL_LOAD              "KextdKernelLoad"
#define SIGNPOST_KEXTD_KERNEL_RESOURCE          "KextdKernelResource"
#define SIGNPOST_KEXTD_USER_LOAD                "KextdUserLoad"
#define SIGNPOST_KEXTD_EXTMAN_VALIDATE          "KextdExtensionManagerValidate"
#define SIGNPOST_KEXTD_EXTMAN_UPDATE            "KextdExtensionManagerUpdate"
#define SIGNPOST_KEXTD_EXTMAN_STOP              "KextdExtensionManagerStop"
#define SIGNPOST_KEXTD_VOLUME_APPEARED          "KextdVolumeAppeared"
#define SIGNPOST_KEXTD_VOLUME_CHANGED           "KextdVolumeChanged"
#define SIGNPOST_KEXTD_VOLUME_DISAPPEARED       "KextdVolumeDisappeared"
#define SIGNPOST_KEXTD_KEXTAUDITMAKEKALN        "KextdAuditMakeKALN"
#define SIGNPOST_KEXTD_KEXTAUDITLOADCALLBACK    "KextdAuditCallback"
// kextcache intervals
#define SIGNPOST_KEXTCACHE_BUILD_PRELINKED_KERNEL "KextcacheBuildPrelinkedKernel"
#define SIGNPOST_KEXTCACHE_UPDATE_VOLUME        "KextcacheUpdateVolume"
#define SIGNPOST_KEXTCACHE_UPDATE_PLISTS        "KextcacheUpdatePlists"
// Other intervals
#define SIGNPOST_KEXT_ALLOW_LIST_WRITE          "WriteKextAllowList"
#define SIGNPOST_KEXT_ALLOW_LIST_READ           "ReadKextAllowList"

// Helper functions
void signpost_kext_properties(OSKextRef theKext, os_signpost_id_t spid);
os_signpost_id_t generate_signpost_id(void);
