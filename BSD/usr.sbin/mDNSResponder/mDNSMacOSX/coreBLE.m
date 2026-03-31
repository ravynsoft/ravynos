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

#if ENABLE_BLE_TRIGGERED_BONJOUR

#include "mDNSEmbeddedAPI.h"
#include "DNSCommon.h"

#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>
#import <CoreBluetooth/CoreBluetooth_Private.h>
#import "mDNSMacOSX.h"
#import "BLE.h"
#import "coreBLE.h"

static coreBLE * coreBLEptr;

// Call Bluetooth subsystem to start/stop the the Bonjour BLE beacon and
// beacon scanning based on the current Bloom filter.
void updateBLEBeacon(serviceHash_t bloomFilter)
{
    if (coreBLEptr == 0)
        coreBLEptr = [[coreBLE alloc] init];

    LogInfo("updateBLEBeacon: bloomFilter = 0x%lx", bloomFilter);

    [coreBLEptr updateBeacon:bloomFilter];
}

// Stop the current BLE beacon.
void stopBLEBeacon(void)
{
    if (coreBLEptr == 0)
        coreBLEptr = [[coreBLE alloc] init];

    [coreBLEptr stopBeacon];
}

bool currentlyBeaconing(void)
{
    if (coreBLEptr == 0)
        coreBLEptr = [[coreBLE alloc] init];

    return [coreBLEptr isBeaconing];
}

// Start the scan.
void startBLEScan(void)
{
    if (coreBLEptr == 0)
        coreBLEptr = [[coreBLE alloc] init];
    [coreBLEptr startScan];
}

// Stop the scan.
void stopBLEScan(void)
{
    if (coreBLEptr == 0)
        coreBLEptr = [[coreBLE alloc] init];

    [coreBLEptr stopScan];
}

@implementation coreBLE
{
    CBCentralManager     *_centralManager;
    CBPeripheralManager  *_peripheralManager;

    NSData               *_currentlyAdvertisedData;

    // [_centralManager isScanning] is only available on iOS and not OSX,
    // so track scanning state locally.
    BOOL                 _isScanning;
    BOOL                 _centralManagerIsOn;
    BOOL                 _peripheralManagerIsOn;
}

- (id)init
{
    self = [super init];

    if (self)
    {
        _centralManager     = [[CBCentralManager alloc] initWithDelegate:self queue:dispatch_get_main_queue()];
        _peripheralManager  = [[CBPeripheralManager alloc] initWithDelegate:self queue:dispatch_get_main_queue()];
        _currentlyAdvertisedData = nil;
        _isScanning = NO;
        _centralManagerIsOn = NO;
        _peripheralManagerIsOn = NO;

        if (_centralManager == nil || _peripheralManager == nil )
        {
            LogMsg("coreBLE initialization failed!");
        } 
        else
        {
            LogInfo("coreBLE initialized");
        }
    }

    return self;
}

#define ADVERTISEMENTDATALENGTH 28 // 31 - 3 (3 bytes for flags)

// TODO: 
// Define DBDeviceTypeBonjour for prototyping until we move to the TDS beacon format.
// The Bluetooth team recommended using a value < 32 for prototyping, since 32 is the number of
// beacon types they can track in their duplicate beacon filtering logic.
#define DBDeviceTypeBonjour     26

// Beacon flags and version byte
#define BonjourBLEVersion     1

extern mDNS mDNSStorage;
extern mDNSInterfaceID AWDLInterfaceID;

// Transmit the last beacon indicating we are no longer advertising or browsing any services for two seconds.
#define LastBeaconTime 2

- (void) updateBeacon:(serviceHash_t) bloomFilter
{
    uint8_t advertisingData[ADVERTISEMENTDATALENGTH] = {0, 0xff, 0x4c, 0x00 };
    uint8_t advertisingLength = 4;

    // If no longer browsing or advertising, beacon this state for 'LastBeaconTime' seconds
    // so that peers have a chance to notice the state change.
    if (bloomFilter == 0)
    {
        LogInfo("updateBeacon: Stopping beacon in %d seconds", LastBeaconTime);

        if (mDNSStorage.timenow == 0)
        {
            // This should never happen since all calling code paths should have called mDNS_Lock(), which
            // initializes the mDNSStorage.timenow value.
            LogMsg("updateBeacon: NOTE, timenow == 0 ??");
        }

        mDNSStorage.NextBLEServiceTime = NonZeroTime(mDNSStorage.timenow + (LastBeaconTime * mDNSPlatformOneSecond));
        finalBeacon = true;
    }
    else
    {
        // Cancel any pending final beacon processing.
        finalBeacon = false;
    }

    // The beacon type.
    advertisingData[advertisingLength++] = DBDeviceTypeBonjour;

    // Flags and Version field
    advertisingData[advertisingLength++] = BonjourBLEVersion;

    memcpy(& advertisingData[advertisingLength], & bloomFilter, sizeof(serviceHash_t));
    advertisingLength += sizeof(serviceHash_t);

    // Add the MAC address of the awdl0 interface.  Don't cache it since
    // it can get updated periodically.
    if (AWDLInterfaceID)
    {
        NetworkInterfaceInfoOSX *intf = IfindexToInterfaceInfoOSX(AWDLInterfaceID);
        if (intf)
            memcpy(& advertisingData[advertisingLength], & intf->ifinfo.MAC, sizeof(mDNSEthAddr));
        else 
            memset( & advertisingData[advertisingLength], 0, sizeof(mDNSEthAddr));
    }
    else
    {
        // Just use zero if not avaiblable.
       memset( & advertisingData[advertisingLength], 0, sizeof(mDNSEthAddr));
    }
    advertisingLength += sizeof(mDNSEthAddr);

    // Total length of data advertised, minus this length byte.
    advertisingData[0] = (advertisingLength - 1);

    LogInfo("updateBeacon: advertisingLength = %d", advertisingLength);

    if (_currentlyAdvertisedData)
        [_currentlyAdvertisedData release];
    _currentlyAdvertisedData = [[NSData alloc] initWithBytes:advertisingData length:advertisingLength];
    [self startBeacon];
}

- (void) startBeacon
{
    if (!_peripheralManagerIsOn)
    {
        LogInfo("startBeacon: Not starting beacon, CBPeripheralManager not powered on");
        return;
    }

    if (_currentlyAdvertisedData == nil)
    {
        LogInfo("startBeacon: Not starting beacon, no data to advertise");
        return;
    }

    if ([_peripheralManager isAdvertising])
    {
        LogInfo("startBeacon: Stop current beacon transmission before restarting");
        [_peripheralManager stopAdvertising];
    }
    LogInfo("startBeacon: Starting beacon");

#if 0   // Move to this code during Fall 2018 develelopment if still using these APIs.
    [_peripheralManager startAdvertising:@{ CBAdvertisementDataAppleMfgData : _currentlyAdvertisedData, CBManagerIsPrivilegedDaemonKey : @YES, @"kCBAdvOptionUseFGInterval" : @YES }];
#else
    // While CBCentralManagerScanOptionIsPrivilegedDaemonKey is deprecated in current MobileBluetooth project, it's still defined in the current and
    // previous train SDKs.  Suppress deprecated warning for now since we intend to move to a different Bluetooth API to manage the BLE Triggered Bonjour 
    // beacons when this code is enabled by default.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    [_peripheralManager startAdvertising:@{ CBAdvertisementDataAppleMfgData : _currentlyAdvertisedData, CBCentralManagerScanOptionIsPrivilegedDaemonKey : @YES, @"kCBAdvOptionUseFGInterval" : @YES }];
#pragma GCC diagnostic pop
#endif
}

- (bool) isBeaconing
{
    return (_currentlyAdvertisedData != nil);
}

- (void) stopBeacon
{
    if (!_peripheralManagerIsOn)
    {
        LogInfo("stopBeacon: CBPeripheralManager is not powered on");
        return;
    }

    // Only beaconing if we have advertised data to send.
    if (_currentlyAdvertisedData)
    {
        LogInfo("stoptBeacon: Stopping beacon");
        [_peripheralManager stopAdvertising];
        [_currentlyAdvertisedData release];
        _currentlyAdvertisedData = nil;
    }
    else
        LogInfo("stoptBeacon: Note currently beaconing");
}

- (void) startScan
{
    if (!_centralManagerIsOn)
    {
        LogInfo("startScan: Not starting scan, CBCentralManager is not powered on");
        return;
    }

    if (_isScanning)
    {
        LogInfo("startScan: already scanning, stopping scan before restarting");
        [_centralManager stopScan];
    }

    LogInfo("startScan: Starting scan");

    _isScanning = YES;

#if 0   // Move to this code during Fall 2018 develelopment if still using these APIs.
    [_centralManager scanForPeripheralsWithServices:nil options:@{ CBCentralManagerScanOptionAllowDuplicatesKey : @YES , CBManagerIsPrivilegedDaemonKey : @YES}];
#else
    // While CBCentralManagerScanOptionIsPrivilegedDaemonKey is deprecated in current MobileBluetooth project, it's still defined in the current and
    // previous train SDKs.  Suppress deprecated warning for now since we intend to move to a different Bluetooth API to manage the BLE Triggered Bonjour 
    // beacons when this code is enabled by default.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    [_centralManager scanForPeripheralsWithServices:nil options:@{ CBCentralManagerScanOptionAllowDuplicatesKey : @YES , CBCentralManagerScanOptionIsPrivilegedDaemonKey : @YES}];
#pragma GCC diagnostic pop
#endif
}

- (void) stopScan
{
    if (!_centralManagerIsOn)
    {
        LogInfo("stopScan: Not stopping scan, CBCentralManager is not powered on");
        return;
    }

    if (_isScanning)
    {
        LogInfo("stopScan: Stopping scan");
        [_centralManager stopScan];
        _isScanning = NO;
    }
    else
    {
        LogInfo("stopScan: Not currently scanning");
    }
}

#pragma mark - CBCentralManagerDelegate protocol

- (void)centralManagerDidUpdateState:(CBCentralManager *)central
{
    switch (central.state) {
        case CBManagerStateUnknown:
            LogInfo("centralManagerDidUpdateState: CBManagerStateUnknown");
            break;

        case CBManagerStateResetting:
            LogInfo("centralManagerDidUpdateState: CBManagerStateResetting");
            break;

        case CBManagerStateUnsupported:
            LogInfo("centralManagerDidUpdateState: CBManagerStateUnsupported");
            break;

        case CBManagerStateUnauthorized:
            LogInfo("centralManagerDidUpdateState: CBManagerStateUnauthorized");
            break;

        case CBManagerStatePoweredOff:
            LogInfo("centralManagerDidUpdateState: CBManagerStatePoweredOff");
            break;

        case CBManagerStatePoweredOn:
            // Hold lock to synchronize with main thread from this callback thread.
            KQueueLock();

            LogInfo("centralManagerDidUpdateState: CBManagerStatePoweredOn");
            _centralManagerIsOn = YES;
            // Only start scan if we have data we will be transmitting or if "suppressBeacons"
            // is set, indicating we should be scanning, but not beaconing.
            if (_currentlyAdvertisedData || suppressBeacons)
                [self startScan];
            else
                LogInfo("centralManagerDidUpdateState:: Not starting scan");

            KQueueUnlock("CBManagerStatePoweredOn");
            break;

        default:
            LogInfo("centralManagerDidUpdateState: Unknown state ??");
            break;
    }
}

#define beaconTypeByteIndex  2   // Offset of beacon type in received CBAdvertisementDataManufacturerDataKey byte array.
#define beaconDataLength    18  // Total number of bytes in the CBAdvertisementDataManufacturerDataKey.

- (void)centralManager:(CBCentralManager *)central didDiscoverPeripheral:(CBPeripheral *)peripheral advertisementData:(NSDictionary<NSString *, id> *)advertisementData RSSI:(NSNumber *)RSSI
{
    (void) central;
    (void) peripheral;
    (void) RSSI;

    NSData *data = [advertisementData objectForKey:CBAdvertisementDataManufacturerDataKey];
   
    // Just return if the beacon data does not match what we are looking for.
    if (!data || ([data length] != beaconDataLength))
    {
        return;
    }

    unsigned char *bytes = (unsigned char *)data.bytes;
    
    // Just parse the DBDeviceTypeBonjour beacons.
    if (bytes[beaconTypeByteIndex] == DBDeviceTypeBonjour)
    {
        serviceHash_t peerBloomFilter;
        mDNSEthAddr   peerMAC;
        unsigned char flagsAndVersion;
        unsigned char *ptr;

#if VERBOSE_BLE_DEBUG
        LogInfo("didDiscoverPeripheral: received DBDeviceTypeBonjour beacon, length = %d", [data length]);
        LogInfo("didDiscoverPeripheral: central = 0x%x, peripheral = 0x%x", central, peripheral);
#endif // VERBOSE_BLE_DEBUG

        // The DBDeviceTypeBonjour beacon bytes will be:
        // 0x4C (1 byte), 0x0 (1 byte), DBDeviceTypeBonjour byte, flags and version byte, 8 byte Bloom filter,
        // 6 byte sender AWDL MAC address

        ptr = & bytes[beaconTypeByteIndex + 1];
        flagsAndVersion = *ptr++;
        memcpy(& peerBloomFilter, ptr, sizeof(serviceHash_t));
        ptr += sizeof(serviceHash_t);
        memcpy(& peerMAC, ptr, sizeof(peerMAC));

#if VERBOSE_BLE_DEBUG
        LogInfo("didDiscoverPeripheral: version = 0x%x, peerBloomFilter = 0x%x",
                flagsAndVersion, peerBloomFilter);
        LogInfo("didDiscoverPeripheral: sender MAC = 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",
            peerMAC.b[0], peerMAC.b[1], peerMAC.b[2], peerMAC.b[3], peerMAC.b[4], peerMAC.b[5]);
#else
        (void)flagsAndVersion; // Unused
#endif  // VERBOSE_BLE_DEBUG

        responseReceived(peerBloomFilter, & peerMAC);
    }
}

#pragma mark - CBPeripheralManagerDelegate protocol

- (void)peripheralManagerDidUpdateState:(CBPeripheralManager *)peripheral
{

    switch (peripheral.state) {
        case CBManagerStateUnknown:
            LogInfo("peripheralManagerDidUpdateState: CBManagerStateUnknown");
            break;

        case CBManagerStateResetting:
            LogInfo("peripheralManagerDidUpdateState: CBManagerStateResetting");
            break;

        case CBManagerStateUnsupported:
            LogInfo("peripheralManagerDidUpdateState: CBManagerStateUnsupported");
            break;

        case CBManagerStateUnauthorized:
            LogInfo("peripheralManagerDidUpdateState: CBManagerStateUnauthorized");
            break;

        case CBManagerStatePoweredOff:
            LogInfo("peripheralManagerDidUpdateState: CBManagerStatePoweredOff");
            break;

        case CBManagerStatePoweredOn:
            // Hold lock to synchronize with main thread from this callback thread.
            KQueueLock();

            LogInfo("peripheralManagerDidUpdateState: CBManagerStatePoweredOn");
            _peripheralManagerIsOn = YES;

            // Start beaconing if we have initialized beacon data to send.
            if (_currentlyAdvertisedData)
                [self startBeacon];

            KQueueUnlock("CBManagerStatePoweredOn");
            break;

        default:
            LogInfo("peripheralManagerDidUpdateState: Unknown state ??");
            break;
    }
}

- (void)peripheralManagerDidStartAdvertising:(CBPeripheralManager *)peripheral error:(nullable NSError *)error
{
    (void) peripheral;

    if (error)
    {
        const char * errorString = [[error localizedDescription] cStringUsingEncoding:NSASCIIStringEncoding];
        LogInfo("peripheralManagerDidStartAdvertising: error = %s", errorString ? errorString: "unknown");
    }
    else
    {
        LogInfo("peripheralManagerDidStartAdvertising:");
    }
}

@end
#endif  // ENABLE_BLE_TRIGGERED_BONJOUR
