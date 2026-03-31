/*
 *  staging.h
 *  kext_tools
 *
 *  Copyright 2017 Apple Inc. All rights reserved.
 *
 */
#pragma once

#import <IOKit/kext/OSKext.h>

Boolean
stagingEnabled(void);

Boolean
kextIsInSecureLocation(OSKextRef theKext);

Boolean
kextRequiresStaging(OSKextRef theKext);

Boolean
needsGPUBundlesStaged(OSKextRef theKext);

Boolean
stageGPUBundles(OSKextRef theKext);

OSKextRef
createStagedKext(OSKextRef theKext);

CFArrayRef
createStagedKextsFromURLs(CFArrayRef kextURLs, Boolean includeUnstaged);

Boolean
pruneStagingDirectory(void);

Boolean
clearStagingDirectory(void);

CFURLRef
copyUnstagedKextURL(CFURLRef kextURL);
