//
//  system_utilities.c
//  mDNSResponder
//
//  Copyright (c) 2019 Apple Inc. All rights reserved.
//

#include <os/variant_private.h> // os_variant_has_internal_diagnostics
#include "mDNSEmbeddedAPI.h"
#include "system_utilities.h"

mDNSexport mDNSBool IsAppleInternalBuild(void)
{
	return (os_variant_has_internal_diagnostics("com.apple.mDNSResponder") ? mDNStrue : mDNSfalse);
}
