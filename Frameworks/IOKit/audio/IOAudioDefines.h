/*
 * Copyright (c) 1998-2013 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 *
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef _IOAUDIODEFINES_H
#define _IOAUDIODEFINES_H

#define kIOAudioDeviceClassName		"IOAudioDevice"
#define kIOAudioEngineClassName		"IOAudioEngine"
#define kIOAudioStreamClassName		"IOAudioStream"
#define kIOAudioPortClassName		"IOAudioPort"
#define kIOAudioControlClassName	"IOAudioControl"

/*!
 * @defined kIOAudioSampleRateKey
 * @abstract The key in the IORegistry for the IOAudioEngine sample rate attribute
 * @discussion This value is represented as an integer in samples per second.
 */
#define kIOAudioSampleRateKey				"IOAudioSampleRate"

#define kIOAudioSampleRateWholeNumberKey	"IOAudioSampleRateWholeNumber"
#define kIOAudioSampleRateFractionKey		"IOAudioSampleRateFraction"



/******
 *
 * IOAudioDevice  defines
 *
 *****/


/*!
 * @defined kIOAudioDeviceNameKey
 * @abstract The key in the IORegistry for the IOAudioDevice name attribute.
 */
#define kIOAudioDeviceNameKey				"IOAudioDeviceName"

#define kIOAudioDeviceShortNameKey			"IOAudioDeviceShortName"

/*!
 * @defined kIOAudioDeviceManufacturerNameKey
 * @abstract The key in the IORegistry for the IOAudioDevice manufacturer name attribute.
 */
#define kIOAudioDeviceManufacturerNameKey	"IOAudioDeviceManufacturerName"

#define kIOAudioDeviceLocalizedBundleKey	"IOAudioDeviceLocalizedBundle"

#define kIOAudioDeviceTransportTypeKey		"IOAudioDeviceTransportType"

#define kIOAudioDeviceConfigurationAppKey	"IOAudioDeviceConfigurationApplication"

#define kIOAudioDeviceCanBeDefaults			"IOAudioDeviceCanBeDefaults"

#define kIOAudioDeviceModelIDKey			"IOAudioDeviceModelID"


/*!
 * @defined kIOAudioDeviceIconName
 * @abstract The key in the IORegistry for the IOAudioDevice icon name attribute.
 */
#define kIOAudioDeviceIconNameKey		"IOAudioDeviceIconName"

#define kIOAudioDeviceIconTypeKey		"IOAudioDeviceIconType"

#define kIOAudioDeviceIconSubDirKey	"IOAudioDeviceIconSubDir"

/*****
 *
 * IOAudioEngine defines
 *
 *****/


 /*!
 * @defined kIOAudioEngineStateKey
 * @abstract The key in the IORegistry for the IOAudioEngine state atrribute
 * @discussion The value for this key may be one of: "Running", "Stopped" or "Paused".  Currently the "Paused"
 *  state is unimplemented.
 */
#define kIOAudioEngineStateKey		"IOAudioEngineState"

/*!
 * @defined kIOAudioEngineOutputSampleLatencyKey
 * @abstract The key in the IORegistry for the IOAudioEngine output sample latency key
 * @discussion 
 */
#define kIOAudioEngineOutputSampleLatencyKey		"IOAudioEngineOutputSampleLatency"

/*!
 * @defined kIOAudioStreamSampleLatencyKey
 * @abstract The key in the IORegistry for the IOAudioStream output sample latency key
 * @discussion Tells the HAL how much latency is on a particular stream.  If two streams
 * on the same engine have different latencies (e.g. one is analog, one is digital), then
 * set this property on both streams to inform the HAL of the latency differences.  Alternately,
 * you can set the engine latency, and just include the latency additional to that for the particular
 * stream.  The HAL will add the engine and stream latency numbers together to get the total latency.
 */
#define kIOAudioStreamSampleLatencyKey				"IOAudioStreamSampleLatency"

#define kIOAudioEngineInputSampleLatencyKey			"IOAudioEngineInputSampleLatency"

#define kIOAudioEngineSampleOffsetKey				"IOAudioEngineSampleOffset"

#define kIOAudioEngineInputSampleOffsetKey			"IOAudioEngineInputSampleOffset"

#define kIOAudioEngineNumSampleFramesPerBufferKey	"IOAudioEngineNumSampleFramesPerBuffer"

#define kIOAudioEngineCoreAudioPlugInKey			"IOAudioEngineCoreAudioPlugIn"

#define kIOAudioEngineNumActiveUserClientsKey		"IOAudioEngineNumActiveUserClients"

#define kIOAudioEngineUserClientActiveKey			"IOAudioEngineUserClientActive"

#define kIOAudioEngineGlobalUniqueIDKey				"IOAudioEngineGlobalUniqueID"

#define kIOAudioEngineDescriptionKey				"IOAudioEngineDescription"

#define kIOAudioEngineClockIsStableKey				"IOAudioEngineClockIsStable"

#define kIOAudioEngineClockDomainKey				"IOAudioEngineClockDomain"

#define kIOAudioEngineIsHiddenKey                   "IOAudioEngineIsHidden"

#define kIOAudioEngineOutputAutoRouteKey            "NoAutoRoute"
/*!
 * @defined kIOAudioEngineFullChannelNamesKey
 * @abstract The key in the IORegistry for the IOAudioEngine's dictionary of fully constructed names for each channel keyed by the device channel
 * @discussion 
 */
#define	kIOAudioEngineFullChannelNamesKey			"IOAudioEngineChannelNames"

/*!
 * @defined kIOAudioEngineFullChannelCategoryNamesKey
 * @abstract The key in the IORegistry for the IOAudioEngine's dictionary of category names for each channel keyed by the device channel
 * @discussion 
 */
#define	kIOAudioEngineFullChannelCategoryNamesKey	"IOAudioEngineChannelCategoryNames"

/*!
 * @defined kIOAudioEngineFullChannelNamesKey
 * @abstract The key in the IORegistry for the IOAudioEngine's dictionary of number names for each channel keyed by the device channel
 * @discussion 
 */
#define	kIOAudioEngineFullChannelNumberNamesKey			"IOAudioEngineChannelNumberNames"

#define	kIOAudioEngineFullChannelNameKeyInputFormat		"InputChannel%u"

#define	kIOAudioEngineFullChannelNameKeyOutputFormat	"OutputChannel%u"

#define kIOAudioEngineFlavorKey							"IOAudioEngineFlavor"

#define	kIOAudioEngineAlwaysLoadCoreAudioPlugInKey		"IOAudioEngineAlwaysLoadCoreAudioPlugIn"

/*!
 * @defined kIOAudioEngineInputChannelLayoutKey
 * @abstract The key in the IORegistry for the IOAudioEngine's dictionary describes an array of OSNumber data that describe the spatial position of each channel.  See IOAudioTypes.h.
 * @discussion
 */

#ifndef __OPEN_SOURCE__
//	<rdar://6868206>
#endif
#define kIOAudioEngineInputChannelLayoutKey				"IOAudioEngineInputChannelLayout"

/*!
 * @defined kIOAudioEngineOutputChannelLayoutKey
 * @abstract The key in the IORegistry for the IOAudioEngine's dictionary describes an array of OSNumber data that describe the spatial position of each channel.  See IOAudioTypes.h.
 * @discussion
 */

#ifndef __OPEN_SOURCE__
//	<rdar://6868206>
#endif
#define kIOAudioEngineOutputChannelLayoutKey			"IOAudioEngineOutputChannelLayout"

/*!
 * @defined kIOAudioEngineDisableClockBoundsCheck
 * @abstract The key in the IORegistry for the IOAudioEngine's dictionary implemented as an OSBoolean that is used to disable the bounds checking on timestamps being passed to the HAL.
 * @discussion By using this key and setting the value to true the driver is asserting that it guarantees that all zero timestamps passed to the HAL increment appropriately at the 
				correct period. This key is used to disable the HAL test that the timestamp is within 1ms of the current time, so that a driver may pass a timestamp that is more than
				1ms in the future. This may be useful when a timestamp is based on a large DMA read/write which encompasses the wrap point but occurs many samples before the end of that point.
 */
#define kIOAudioEngineDisableClockBoundsCheck		"IOAudioEngineDisableClockBoundsCheck"

/*****
 *
 * IOAudioStream defines
 *
 *****/
 
 
#define kIOAudioStreamIDKey					"IOAudioStreamID"
#define kIOAudioStreamDescriptionKey		"IOAudioStreamDescription"
#define kIOAudioStreamNumClientsKey			"IOAudioStreamNumClients"

/*!
 * @defined kIOAudioStreamDirectionKey
 * @abstract The key in the IORegistry for the IOAudioStream direction attribute.
 * @discussion The value for this key may be either "Output" or "Input".
 */
#define kIOAudioStreamDirectionKey				"IOAudioStreamDirection"

#define kIOAudioStreamStartingChannelIDKey		"IOAudioStreamStartingChannelID"
#define kIOAudioStreamStartingChannelNumberKey	"IOAudioStreamStartingChannelNumber"
#define kIOAudioStreamAvailableKey				"IOAudioStreamAvailable"

#define kIOAudioStreamFormatKey					"IOAudioStreamFormat"
#define kIOAudioStreamAvailableFormatsKey		"IOAudioStreamAvailableFormats"

#define kIOAudioStreamNumChannelsKey			"IOAudioStreamNumChannels"
#define kIOAudioStreamSampleFormatKey			"IOAudioStreamSampleFormat"

#define kIOAudioStreamNumericRepresentationKey	"IOAudioStreamNumericRepresentation"

#define kIOAudioStreamFormatFlagsKey			"IOAudioStreamFormatFlags"
#define kIOAudioStreamFramesPerPacketKey		"IOAudioStreamFramesPerPacket"
#define kIOAudioStreamBytesPerPacketKey			"IOAudioStreamBytesPerPacket"


#define kIOAudioStreamBitDepthKey				"IOAudioStreamBitDepth"
#define kIOAudioStreamBitWidthKey				"IOAudioStreamBitWidth"
    
#define kIOAudioStreamAlignmentKey				"IOAudioStreamAlignment"

#define kIOAudioStreamByteOrderKey				"IOAudioStreamByteOrder"

#define kIOAudioStreamIsMixableKey				"IOAudioStreamIsMixable"

#define kIOAudioStreamMinimumSampleRateKey		"IOAudioStreamMinimumSampleRate"
#define kIOAudioStreamMaximumSampleRateKey		"IOAudioStreamMaximumSampleRate"

#define kIOAudioStreamDriverTagKey				"IOAudioStreamDriverTag"

#define kIOAudioStreamTerminalTypeKey			"IOAudioStreamTerminalType"

/*****
 *
 * IOAudioPort defines
 *
 *****/
 
 
 /*!
 * @defined kIOAudioPortTypeKey
 * @abstract The key in the IORegistry for the IOAudioPort type attribute.
 * @discussion This is a driver-defined text attribute that may contain any type.
 *  Common types are defined as: "Speaker", "Headphones", "Microphone", "CD", "Line", "Digital", "Mixer", "PassThru".
 */
#define kIOAudioPortTypeKey			"IOAudioPortType"

/*!
 * @defined kIOAudioPortSubTypeKey
 * @abstract The key in the IORegistry for the IOAudioPort subtype attribute.
 * @discussion The IOAudioPort subtype is a driver-defined text attribute designed to complement the type
 *  attribute.
 */
#define kIOAudioPortSubTypeKey		"IOAudioPortSubType"

/*!
 * @defined kIOAudioPortNameKey
 * @abstract The key in the IORegistry for the IOAudioPort name attribute.
 */
#define kIOAudioPortNameKey			"IOAudioPortName"



/*****
 *
 * IOAudioControl defines
 *
 *****/
 
 
 /*!
 * @defined kIOAudioControlTypeKey
 * @abstract The key in the IORegistry for the IOAudioCntrol type attribute.
 * @discussion The value of this text attribute may be defined by the driver, however system-defined
 *  types recognized by the upper-level software are "Level", "Mute", "Selector".
 */
#define kIOAudioControlTypeKey		"IOAudioControlType"

#define kIOAudioControlSubTypeKey	"IOAudioControlSubType"

#define kIOAudioControlUsageKey		"IOAudioControlUsage"

#define kIOAudioControlIDKey		"IOAudioControlID"

/*!
 * @defined kIOAudioControlChannelIDKey
 * @abstract The key in the IORegistry for the IOAudioControl channel ID attribute
 * @discussion The value for this key is an integer which may be driver defined.  Default values for
 *  common channel types are provided in the following defines.
 */
#define kIOAudioControlChannelIDKey		"IOAudioControlChannelID"

#define kIOAudioControlChannelNumberKey			"IOAudioControlChannelNumber"

#define kIOAudioControlCoreAudioPropertyIDKey	"IOAudioControlCoreAudioPropertyID"
/*!
 * @defined kIOAudioControlChannelNameKey
 * @abstract The key in the IORegistry for the IOAudioControl name attribute.
 * @discussion This name should be a human-readable name for the channel(s) represented by the port.
 *  *** NOTE *** We really need to make all of the human-readable attributes that have potential to
 *  be used in a GUI localizable.  There will need to be localized strings in the kext bundle matching
 *  the text.
 */
#define kIOAudioControlChannelNameKey		"IOAudioControlChannelName"

/*!
 * @defined kIOAudioControlChannelNameAll
 * @abstract The value for the kIOAudioControlChannelNameKey in the IORegistry representing
 *  the channel name for all channels.
 */
#define kIOAudioControlChannelNameAll		"All Channels"

/*!
 * @defined kIOAudioControlChannelNameLeft
 * @abstract The value for the kIOAudioControlChannelNameKey in the IORegistry representing
 *  the channel name for the left channel.
 */
#define kIOAudioControlChannelNameLeft		"Left"

/*!
 * @defined kIOAudioControlChannelNameRight
 * @abstract The value for the kIOAudioControlChannelNameKey in the IORegistry representing
 *  the channel name for the right channel.
 */
#define kIOAudioControlChannelNameRight		"Right"

/*!
 * @defined kIOAudioControlChannelNameCenter
 * @abstract The value for the kIOAudioControlChannelNameKey in the IORegistry representing
 *  the channel name for the center channel.
 */
#define kIOAudioControlChannelNameCenter	"Center"

/*!
 * @defined kIOAudioControlChannelNameLeftRear
 * @abstract The value for the kIOAudioControlChannelNameKey in the IORegistry representing
 *  the channel name for the left rear channel.
 */
#define kIOAudioControlChannelNameLeftRear	"LeftRear"

/*!
 * @defined kIOAudioControlChannelNameRightRear
 * @abstract The value for the kIOAudioControlChannelNameKey in the IORegistry representing
 *  the channel name for the right rear channel.
 */
#define kIOAudioControlChannelNameRightRear	"RightRear"

/*!
 * @defined kIOAudioControlChannelNameSub
 * @abstract The value for the kIOAudioControlChannelNameKey in the IORegistry representing
 *  the channel name for the sub/LFE channel.
 */
#define kIOAudioControlChannelNameSub		"Sub"

/*!
 * @defined kIOAudioControlChannelNameFrontLeftCenter
 * @abstract The value for the kIOAudioControlChannelNameKey in the IORegistry representing
 *  the channel name for the FrontLeftCenter channel.
 */
#define kIOAudioControlChannelNameFrontLeftCenter		"FrontLeftCenter"

/*!
 * @defined kIOAudioControlChannelNameFrontRightCenter
 * @abstract The value for the kIOAudioControlChannelNameKey in the IORegistry representing
 *  the channel name for the FrontRightCenter channel.
 */
#define kIOAudioControlChannelNameFrontRightCenter		"FrontRightCenter"

/*!
 * @defined kIOAudioControlChannelNameRearCenter
 * @abstract The value for the kIOAudioControlChannelNameKey in the IORegistry representing
 *  the channel name for the RearCenter channel.
 */
#define kIOAudioControlChannelNameRearCenter		"RearCenter"

/*!
 * @defined kIOAudioControlChannelNameSurroundLeft
 * @abstract The value for the kIOAudioControlChannelNameKey in the IORegistry representing
 *  the channel name for the SurroundLeft channel.
 */
#define kIOAudioControlChannelNameSurroundLeft		"SurroundLeft"

/*!
 * @defined kIOAudioControlChannelNameSurroundRight
 * @abstract The value for the kIOAudioControlChannelNameKey in the IORegistry representing
 *  the channel name for the SurroundRight channel.
 */
#define kIOAudioControlChannelNameSurroundRight		"SurroundRight"


/*!
 * @defined kIOAudioControlValueKey
 * @abstract The key in the IORegistry for the IOAudioControl value attribute.
 * @discussion The value returned by this key is a 32-bit integer representing the current value of the IOAudioControl.
 */
#define kIOAudioControlValueKey				"IOAudioControlValue"

/*!
 * @defined kIOAudioControlValueIsReadOnlyKey
 * @abstract The key in the IORegistry for the IOAudioControl value-is-read-only attribute.
 * @discussion The value returned by this key is a 32-bit integer but the value doesn't have any direct meaning.
 *  Instead, the presence of this key indicates that the value for the control is read-only
 */
#define kIOAudioControlValueIsReadOnlyKey	"IOAudioControlValueIsReadOnly"

/*!
 * @defined kIOAudioLevelControlMinValueKey
 * @abstract The key in the IORegistry for the IOAudioControl minimum value attribute.
 * @discussion The value returned by this key is a 32-bit integer representing the minimum value for the IOAudioControl.
 *  This is currently only valid for Level controls or other driver-defined controls that have a minimum and maximum
 *  value.
 */
#define kIOAudioLevelControlMinValueKey		"IOAudioLevelControlMinValue"

/*!
 * @defined kIOAudioLevelControlMaxValueKey
 * @abstract The key in the IORegistry for the IOAudioControl maximum value attribute.
 * @discussion The value returned by this key is a 32-bit integer representing the maximum value for the IOAudioControl.
 *  This is currently only valid for Level controls or other driver-defined controls that have a minimum and maximum
 *  value.
 */
#define kIOAudioLevelControlMaxValueKey		"IOAudioLevelControlMaxValue"

/*!
 * @defined kIOAudioLevelControlMinDBKey
 * @abstract The key in the IORgistry for the IOAudioControl minimum db value attribute.
 * @discussion The value returned by this key is a fixed point value in 16.16 format represented as a 32-bit
 *  integer.  It represents the minimum value in db for the IOAudioControl.  This value matches the minimum
 *  value attribute.  This is currently valid for Level controls or other driver-defined controls that have a
 *  minimum and maximum db value.
 */
#define kIOAudioLevelControlMinDBKey		"IOAudioLevelControlMinDB"

/*!
 * @defined kIOAudioLevelControlMaxDBKey
 * @abstract The key in the IORgistry for the IOAudioControl maximum db value attribute.
 * @discussion The value returned by this key is a fixed point value in 16.16 format represented as a 32-bit
 *  integer.  It represents the maximum value in db for the IOAudioControl.  This value matches the maximum
 *  value attribute.  This is currently valid for Level controls or other driver-defined controls that have a
 *  minimum and maximum db value.
 */
#define kIOAudioLevelControlMaxDBKey		"IOAudioLevelControlMaxDB"

#define kIOAudioLevelControlRangesKey		"IOAudioLevelControlRanges"

#define kIOAudioLevelControlUseLinearScale	"IOAudioLevelControlUseLinearScale"

#define kIOAudioSelectorControlAvailableSelectionsKey	"IOAudioSelectorControlAvailableSelections"
#define kIOAudioSelectorControlSelectionValueKey		"IOAudioSelectorControlSelectionValue"
#define kIOAudioSelectorControlSelectionDescriptionKey	"IOAudioSelectorControlSelectionDescriptionKey"
#define kIOAudioSelectorControlTransportValueKey		"IOAudioSelectorControlTransportValue"					// <rdar://8202424>

#define kIOAudioSelectorControlClockSourceKey			"IOAudioSelectorControlClockSourceKey"


#endif /* _IOAUDIODEFINES_H */
