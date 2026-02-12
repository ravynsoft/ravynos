/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2011 Apple Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _P2P_PACKET_FILTER_H_
#define _P2P_PACKET_FILTER_H_

#include "helpermsg-types.h"

enum
{
    PF_SET_RULES,
    PF_CLEAR_RULES
};

int P2PPacketFilterAddBonjourRuleSet(const char * interfaceName, u_int32_t count, pfArray_t portArray, pfArray_t protocolArray );
int P2PPacketFilterClearBonjourRules(void);

#endif /* _P2P_PACKET_FILTER_H_ */
