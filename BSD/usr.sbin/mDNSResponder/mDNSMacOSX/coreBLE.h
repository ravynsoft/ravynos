/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2015-2016 Apple Inc. All rights reserved.
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

#ifndef _coreBLE_H_
#define _coreBLE_H_

#if ENABLE_BLE_TRIGGERED_BONJOUR

#include "BLE.h"

@interface coreBLE : NSObject <CBCentralManagerDelegate, CBPeripheralManagerDelegate, CBPeripheralDelegate>

- (id)init;
- (void) updateBeacon:(serviceHash_t) bloomFilter;
- (void) startBeacon;
- (bool) isBeaconing;
- (void) stopBeacon;
- (void) startScan;
- (void) stopScan;

@end

#endif  // ENABLE_BLE_TRIGGERED_BONJOUR

#endif /* _coreBLE_H_ */
