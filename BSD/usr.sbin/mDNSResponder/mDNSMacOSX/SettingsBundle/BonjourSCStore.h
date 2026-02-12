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

#import <SystemConfiguration/SystemConfiguration.h>
#import <Security/Security.h>

#define SC_DYNDNS_PREFS_KEY         CFSTR("com.apple.preference.bonjour")
#define SC_DYNDNS_SYSTEM_KEY        CFSTR("/System/Network/DynamicDNS")
#define SC_DYNDNS_REGDOMAINS_KEY    CFSTR("RegistrationDomains")
#define SC_DYNDNS_BROWSEDOMAINS_KEY CFSTR("BrowseDomains")
#define SC_DYNDNS_HOSTNAMES_KEY     CFSTR("HostNames")
#define SC_DYNDNS_DOMAIN_KEY        CFSTR("Domain")
#define SC_DYNDNS_KEYNAME_KEY       CFSTR("KeyName")
#define SC_DYNDNS_SECRET_KEY        CFSTR("Secret")
#define SC_DYNDNS_ENABLED_KEY       CFSTR("Enabled")
#define SC_DYNDNS_STATUS_KEY        CFSTR("Status")

#define SC_DYNDNS_SETUP_KEY         CFSTR("Setup:/Network/DynamicDNS")
#define SC_DYNDNS_STATE_KEY         CFSTR("State:/Network/DynamicDNS")

@interface BonjourSCStore : NSObject

+ (NSArray *_Nullable)objectForKey:(NSString *_Nonnull)key;
+ (void)setObject:(NSArray *_Nullable)value forKey:(NSString *_Nonnull)key;

@end
