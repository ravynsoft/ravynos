/*
 *
 * Copyright (c) 2016 Apple Inc. All rights reserved.
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

#import "BonjourSCStore.h"
#import <Foundation/Foundation.h>
#import <AssertMacros.h>

@implementation BonjourSCStore

+ (NSArray * _Nullable)objectForKey:(NSString * _Nonnull)key
{
    NSArray *           result = nil;
    SCPreferencesRef	store;
    OSStatus            err;
    NSDictionary *		origDict;
    
    store = SCPreferencesCreateWithAuthorization(kCFAllocatorDefault, SC_DYNDNS_PREFS_KEY, NULL, NULL);
    require_action(store != NULL, SysConfigErr, err = SCError());
    require_action(true == SCPreferencesLock(store, true), LockFailed, err = SCError());
    
    origDict = (__bridge NSDictionary *)SCPreferencesPathGetValue(store, SC_DYNDNS_SYSTEM_KEY);
    if (origDict)  origDict = [NSDictionary dictionaryWithDictionary: origDict];
    
    result = [origDict objectForKey: key];
    
    SCPreferencesUnlock(store);
    
LockFailed:
    CFRelease(store);
SysConfigErr:
    return(result);
}

+ (void)setObject:(NSArray * _Nullable)value forKey:(NSString * _Nonnull)key
{
    SCPreferencesRef	    store;
    OSStatus				err;
    NSMutableDictionary *   origDict;
    Boolean					success;
    
    store = SCPreferencesCreateWithAuthorization(kCFAllocatorDefault, SC_DYNDNS_PREFS_KEY, NULL, NULL);
    require_action(store != NULL, SysConfigErr, err = SCError());
    require_action(true == SCPreferencesLock(store, true), LockFailed, err = SCError());
    
    origDict = (__bridge NSMutableDictionary *)SCPreferencesPathGetValue(store, SC_DYNDNS_SYSTEM_KEY);
    if (!origDict)     origDict = [NSMutableDictionary dictionary];
    else               origDict = [NSMutableDictionary dictionaryWithDictionary: origDict];
    
    if (value.count)   [origDict setObject: value forKey: key];
    else               [origDict removeObjectForKey: key];
    
    success = SCPreferencesPathSetValue(store, SC_DYNDNS_SYSTEM_KEY, (__bridge CFDictionaryRef)origDict);
    require_action(success, SCError, err = SCError(););
    
    success = SCPreferencesCommitChanges(store);
    require_action(success, SCError, err = SCError());
    success = SCPreferencesApplyChanges(store);
    require_action(success, SCError, err = SCError());
    
SCError:
    SCPreferencesUnlock(store);
LockFailed:
    CFRelease(store);
SysConfigErr:
    return;
}


@end

