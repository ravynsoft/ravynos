/*
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * Copyright (c) 2019 Apple Computer, Inc.  All Rights Reserved.
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef IOHIDDeviceKeys_h
#define IOHIDDeviceKeys_h

/*!
 * @define kIOHIDTransportKey
 *
 * @abstract
 * Number property that describes the transport of the device.
 */
#define kIOHIDTransportKey "Transport"

/*!
 * @define kIOHIDVendorIDKey
 *
 * @abstract
 * Number property that describes the vendor ID of the device.
 */
#define kIOHIDVendorIDKey "VendorID"

/*!
 * @define kIOHIDProductIDKey
 *
 * @abstract
 * Number property that describes the product ID of the device.
 */
#define kIOHIDProductIDKey "ProductID"

/*!
 * @define kIOHIDVersionNumberKey
 *
 * @abstract
 * Number property that describes the version number of the device.
 */
#define kIOHIDVersionNumberKey "VersionNumber"

/*!
 * @define kIOHIDManufacturerKey
 *
 * @abstract
 * String property that describes the manufacturer of the device.
 */
#define kIOHIDManufacturerKey "Manufacturer"

/*!
 * @define kIOHIDProductKey
 *
 * @abstract
 * String property that describes the product of the device.
 */
#define kIOHIDProductKey "Product"

/*!
 * @define kIOHIDSerialNumberKey
 *
 * @abstract
 * String property that describes the serial number of the device.
 */
#define kIOHIDSerialNumberKey "SerialNumber"

/*!
 * @define kIOHIDCountryCodeKey
 *
 * @abstract
 * Number property that describes the country code of the device.
 */
#define kIOHIDCountryCodeKey "CountryCode"

/*!
 * @define kIOHIDLocationIDKey
 *
 * @abstract
 * Number property that describes the location ID of the device.
 */
#define kIOHIDLocationIDKey "LocationID"

/*!
 * @define kIOHIDDeviceUsagePairsKey
 *
 * @abstract
 * Array property that describes the top level usages of the device. The array
 * will have dictionaries of usage pages/usages of each top level collection
 * that exists on the device.
 */
#define kIOHIDDeviceUsagePairsKey "DeviceUsagePairs"

/*!
 * @define kIOHIDDeviceUsageKey
 *
 * @abstract
 * Number property used in the device usage pairs array above. Describes a
 * usage of the device.
 */
#define kIOHIDDeviceUsageKey "DeviceUsage"

/*!
 * @define kIOHIDDeviceUsagePageKey
 *
 * @abstract
 * Number property used in the device usage pairs array above. Describes a
 * usage page of the device.
 */
#define kIOHIDDeviceUsagePageKey "DeviceUsagePage"

/*!
 * @define kIOHIDPrimaryUsageKey
 *
 * @abstract
 * Number property that describes the primary usage page of the device.
 */
#define kIOHIDPrimaryUsageKey "PrimaryUsage"

/*!
 * @define kIOHIDPrimaryUsagePageKey
 *
 * @abstract
 * Number property that describes the primary usage of the device.
 */
#define kIOHIDPrimaryUsagePageKey "PrimaryUsagePage"

/*!
 * @define kIOHIDMaxInputReportSizeKey
 *
 * @abstract
 * Number property that describes the max input report size of the device. This
 * is derived from the report descriptor data provided in the
 * kIOHIDReportDescriptorKey key.
 */
#define kIOHIDMaxInputReportSizeKey "MaxInputReportSize"

/*!
 * @define kIOHIDMaxOutputReportSizeKey
 *
 * @abstract
 * Number property that describes the max output report size of the device. This
 * is derived from the report descriptor data provided in the
 * kIOHIDReportDescriptorKey key.
 */
#define kIOHIDMaxOutputReportSizeKey "MaxOutputReportSize"

/*!
 * @define kIOHIDMaxFeatureReportSizeKey
 *
 * @abstract
 * Number property that describes the max feature report size of the device.
 * This is derived from the report descriptor data provided in the
 * kIOHIDReportDescriptorKey key.
 */
#define kIOHIDMaxFeatureReportSizeKey "MaxFeatureReportSize"

/*!
 * @define kIOHIDReportIntervalKey
 *
 * @abstract
 * Number property set on the device from a client that describes the interval
 * at which the client wishes to receive reports. It is up to the device to
 * determine how to handle this key, if it chooses to do so.
 */
#define kIOHIDReportIntervalKey "ReportInterval"

/*!
 * @define kIOHIDBatchIntervalKey
 *
 * @abstract
 * Number property set on the device from a client that describes the interval
 * at which the client wishes to receive batched reports. It is up to the device
 * to determine how to handle this key, if it chooses to do so.
 */
#define kIOHIDBatchIntervalKey "BatchInterval"

/*!
 * @define kIOHIDRequestTimeoutKey
 *
 * @abstract
 * Number property that describes the request timeout in us for get/setReport calls.
 * It is up to the device to determine how to handle this key, if it chooses to
 * do so.
 */
#define kIOHIDRequestTimeoutKey "RequestTimeout"

/*!
 * @define kIOHIDReportDescriptorKey
 *
 * @abstract
 * Data property that describes the report descriptor of the device.
 */
#define kIOHIDReportDescriptorKey "ReportDescriptor"

/*!
 * @define kIOHIDBuiltInKey
 *
 * @abstract
 * Number property that describes if the device is built in.
 */
#define kIOHIDBuiltInKey "Built-In"

/*!
 * @define kIOHIDPhysicalDeviceUniqueIDKey
 *
 * @abstract
 * String property that describes a unique identifier of the device.
 */
#define kIOHIDPhysicalDeviceUniqueIDKey "PhysicalDeviceUniqueID"

#endif /* IOHIDDeviceKeys_h */
