//
//  xpc_service_log_utility.h
//  mDNSResponder
//
//  Copyright (c) 2019 Apple Inc. All rights reserved.
//

#ifndef XPC_SERVICE_LOG_UTILITY_H
#define XPC_SERVICE_LOG_UTILITY_H

#include "mDNSEmbeddedAPI.h"

#define MDSNRESPONDER_STATE_DUMP_DIR "/private/var/log/mDNSResponder"
#define MDSNRESPONDER_STATE_DUMP_FILE_NAME "mDNSResponder_state_dump"

mDNSexport void init_log_utility_service(void);

#endif /* XPC_SERVICE_LOG_UTILITY_H */
