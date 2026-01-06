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

#ifndef _IOKIT_IOAUDIOTYPES_H
#define _IOKIT_IOAUDIOTYPES_H

#include <libkern/OSTypes.h>
#include <mach/message.h>


/*!
 * @enum IOAudioEngineMemory
 * @abstract Used to identify the type of memory requested by a client process to be mapped into its process space
 * @discussion This is the parameter to the type field of IOMapMemory when called on an IOAudioEngine.  This is
 *  only intended for use by the Audio Device API library.
 * @constant kIOAudioSampleBuffer This requests the IOAudioEngine's sample buffer
 * @constant kIOAudioStatusBuffer This requests the IOAudioEngine's status buffer.  It's type is IOAudioEngineStatus.
 * @constant kIOAudioMixBuffer This requests the IOAudioEngine's mix buffer
*/
typedef enum _IOAudioEngineMemory {
    kIOAudioStatusBuffer 			= 0,
    kIOAudioSampleBuffer			= 1,
    kIOAudioMixBuffer				= 2,
	kIOAudioBytesInInputBuffer		= 3,
	kIOAudioBytesInOutputBuffer		= 4
} IOAudioEngineMemory;

/*!
 * @enum IOAudioEngineCalls
 * @abstract The set of constants passed to IOAudioEngineUserClient::getExternalMethodForIndex() when making calls
 *  from the IOAudioFamily user client code.
 */
typedef enum _IOAudioEngineCalls {
    kIOAudioEngineCallRegisterClientBuffer			= 0,
    kIOAudioEngineCallUnregisterClientBuffer		= 1,
    kIOAudioEngineCallGetConnectionID				= 2,
    kIOAudioEngineCallStart							= 3,
    kIOAudioEngineCallStop							= 4,
	kIOAudioEngineCallGetNearestStartTime			= 5
} IOAudioEngineCalls;

/*! @defined kIOAudioEngineNumCalls The number of elements in the IOAudioEngineCalls enum. */
#define kIOAudioEngineNumCalls		6

typedef enum _IOAudioEngineTraps {
    kIOAudioEngineTrapPerformClientIO				= 0
} IOAudioEngineTraps;

typedef enum _IOAudioEngineNotifications {
    kIOAudioEngineAllNotifications 					= 0,
    kIOAudioEngineStreamFormatChangeNotification	= 1,
    kIOAudioEngineChangeNotification 				= 2,
    kIOAudioEngineStartedNotification 				= 3,
    kIOAudioEngineStoppedNotification 				= 4,
    kIOAudioEnginePausedNotification				= 5,
    kIOAudioEngineResumedNotification				= 6
} IOAudioEngineNotifications;

/*!
 * @enum IOAudioEngineState
 * @abstract Represents the state of an IOAudioEngine
 * @constant kIOAudioEngineRunning The IOAudioEngine is currently running - it is transferring data to or 
 *           from the device.
 * @constant kIOAudioEngineStopped The IOAudioEngine is currently stopped - no activity is occurring.
 */

typedef enum _IOAudioEngineState {
    kIOAudioEngineStopped							= 0,
    kIOAudioEngineRunning							= 1,
    kIOAudioEnginePaused							= 2,
    kIOAudioEngineResumed							= 3
} IOAudioEngineState;


/*!
 * @typedef IOAudioEngineStatus
 * @abstract Shared-memory structure giving audio engine status
 * @discussion
 * @field fVersion Indicates version of this structure
 * @field fCurrentLoopCount Number of times around the ring buffer since the audio engine started
 * @field fLastLoopTime Timestamp of the last time the ring buffer wrapped
 * @field fEraseHeadSampleFrame Location of the erase head in sample frames - erased up to but not
 *        including the given sample frame
 */

typedef struct _IOAudioEngineStatus {
    UInt32					fVersion;
    volatile UInt32			fCurrentLoopCount;
    volatile AbsoluteTime   fLastLoopTime;
    volatile UInt32			fEraseHeadSampleFrame;
} IOAudioEngineStatus;

#define kIOAudioEngineCurrentStatusStructVersion		2

typedef struct _IOAudioStreamFormat {
    UInt32	fNumChannels;
    UInt32	fSampleFormat;
    UInt32	fNumericRepresentation;
    UInt8	fBitDepth;
    UInt8	fBitWidth;
    UInt8	fAlignment;
    UInt8	fByteOrder;
    UInt8	fIsMixable;
    UInt32	fDriverTag;
} IOAudioStreamFormat;

#define kFormatExtensionInvalidVersion					0
#define kFormatExtensionCurrentVersion					1

typedef struct _IOAudioStreamFormatExtension {
    UInt32	fVersion;
    UInt32	fFlags;
    UInt32	fFramesPerPacket;
    UInt32	fBytesPerPacket;
} IOAudioStreamFormatExtension;

typedef struct _IOAudioBufferDataDescriptor {
	UInt32	fActualDataByteSize;
	UInt32	fActualNumSampleFrames;
	UInt32	fTotalDataByteSize;
	UInt32	fNominalDataByteSize;
	UInt8	fData[1];
} IOAudioBufferDataDescriptor;

#define kStreamDataDescriptorInvalidVersion				0
#define kStreamDataDescriptorCurrentVersion				1

typedef struct _IOAudioStreamDataDescriptor {
    UInt32	fVersion;
    UInt32	fNumberOfStreams;
    UInt32	fStreamLength[1];			// Array with fNumberOfStreams number of entries
} IOAudioStreamDataDescriptor;

typedef struct _IOAudioSampleIntervalDescriptor {
	UInt32	sampleIntervalHi;
	UInt32	sampleIntervalLo;
} IOAudioSampleIntervalDescriptor;

/*!
    @struct         SMPTETime
    @abstract       A structure for holding a SMPTE time.
    @field          fSubframes
                        The number of subframes in the full message.
    @field          fSubframeDivisor
                        The number of subframes per frame (typically 80).
    @field          fCounter
                        The total number of messages received.
    @field          fType
                        The kind of SMPTE time using the SMPTE time type constants.
    @field          fFlags
                        A set of flags that indicate the SMPTE state.
    @field          fHours
                        The number of hourse in the full message.
    @field          fMinutes
                        The number of minutes in the full message.
    @field          fSeconds
                        The number of seconds in the full message.
    @field          fFrames
                        The number of frames in the full message.
*/
typedef struct _IOAudioSMPTETime
{
    SInt16  fSubframes;
    SInt16  fSubframeDivisor;
    UInt32  fCounter;
    UInt32  fType;
    UInt32  fFlags;
    SInt16  fHours;
    SInt16  fMinutes;
    SInt16  fSeconds;
    SInt16  fFrames;

} IOAudioSMPTETime;

//	constants describing SMPTE types (taken from the MTC spec), <rdar://11955717>
enum
{
	kIOAudioSMPTETimeType24			= 0,
	kIOAudioSMPTETimeType25			= 1,
	kIOAudioSMPTETimeType30Drop		= 2,
	kIOAudioSMPTETimeType30			= 3,
	kIOAudioSMPTETimeType2997		= 4,
	kIOAudioSMPTETimeType2997Drop	= 5,
    kIOAudioSMPTETimeType60         = 6,
    kIOAudioSMPTETimeType5994       = 7,
    kIOAudioSMPTETimeType60Drop     = 8,
    kIOAudioSMPTETimeType5994Drop   = 9,
    kIOAudioSMPTETimeType50         = 10,
    kIOAudioSMPTETimeType2398       = 11
};

//	flags describing a SMPTE time stamp
enum
{
	kIOAudioSMPTETimeValid		= (1L << 0),	//	the full time is valid
	kIOAudioSMPTETimeRunning	= (1L << 1)		//	time is running
};

//	A struct for encapsulating the parts of a time stamp. The flags
//	say which fields are valid.
typedef struct _IOAudioTimeStamp
{
	UInt64				fSampleTime;	//	the absolute sample time, was a Float64
	UInt64				fHostTime;		//	the host's root timebase's time
	UInt64				fRateScalar;	//	the system rate scalar, was a Float64
	UInt64				fWordClockTime;	//	the word clock time
	IOAudioSMPTETime	fSMPTETime;		//	the SMPTE time
	UInt32				fFlags;			//	the flags indicate which fields are valid
	UInt32				fReserved;		//	reserved, pads the structure out to force 8 byte alignment
} IOAudioTimeStamp;

//	flags for the AudioTimeStamp sturcture
enum
{
	kIOAudioTimeStampSampleTimeValid	= (1L << 0),
	kIOAudioTimeStampHostTimeValid		= (1L << 1),
	kIOAudioTimeStampRateScalarValid	= (1L << 2),
	kIOAudioTimeStampWordClockTimeValid	= (1L << 3),
	kIOAudioTimeStampSMPTETimeValid		= (1L << 4)
};

//	Some commonly used combinations of timestamp flags
enum
{
	kIOAudioTimeStampSampleHostTimeValid	= (kIOAudioTimeStampSampleTimeValid | kIOAudioTimeStampHostTimeValid)
};

/*!
* @enum IOAudioStreamDirection
 * @abstract Represents the direction of an IOAudioStream
 * @constant kIOAudioStreamDirectionOutput Output buffer
 * @constant kIOAudioStreamDirectionInput Input buffer
 */

typedef enum _IOAudioStreamDirection {
    kIOAudioStreamDirectionOutput	= 0,
    kIOAudioStreamDirectionInput	= 1
} IOAudioStreamDirection;

enum {
	kIOAudioDeviceCanBeDefaultNothing	= 0,
	kIOAudioDeviceCanBeDefaultInput		= (1L << 0),
	kIOAudioDeviceCanBeDefaultOutput	= (1L << 1),
	kIOAudioDeviceCanBeSystemOutput		= (1L << 2)
};

/*!
 * @defined kIOAudioEngineDefaultMixBufferSampleSize
 */

#define kIOAudioEngineDefaultMixBufferSampleSize		sizeof(float)

/* The following are for use only by the IOKit.framework audio family code */

/*!
 * @enum IOAudioControlCalls
 * @abstract The set of constants passed to IOAudioControlUserClient::getExternalMethodForIndex() when making calls
 *  from the IOAudioFamily user client code.
 * @constant kIOAudioControlCallSetValue Used to set the value of an IOAudioControl.
 * @constant kIOAudioControlCallGetValue Used to get the value of an IOAudioControl.
 */
typedef enum _IOAudioControlCalls {
    kIOAudioControlCallSetValue = 0,
    kIOAudioControlCallGetValue = 1
} IOAudioControlCalls;

/*! @defined kIOAudioControlNumCalls The number of elements in the IOAudioControlCalls enum. */
#define kIOAudioControlNumCalls 	2

/*!
 * @enum IOAudioControlNotifications
 * @abstract The set of constants passed in the type field of IOAudioControlUserClient::registerNotificaitonPort().
 * @constant kIOAudioControlValueChangeNotification Used to request value change notifications.
 * @constant kIOAudioControlRangeChangeNotification Used to request range change notifications.
 */
typedef enum _IOAudioControlNotifications {
    kIOAudioControlValueChangeNotification = 0,
	kIOAudioControlRangeChangeNotification = 1
} IOAudioControlNotifications;

/*!
 * @struct IOAudioNotificationMessage
 * @abstract Used in the mach message for IOAudio notifications.
 * @field messageHeader Standard mach message header
 * @field ref The param passed to registerNotificationPort() in refCon.
 */
typedef struct _IOAudioNotificationMessage {
    mach_msg_header_t	messageHeader;
    UInt32		type;
    UInt32		ref;
    void *		sender;
} IOAudioNotificationMessage;

typedef struct _IOAudioSampleRate {
    UInt32	whole;
    UInt32	fraction;
} IOAudioSampleRate;

#define kNoIdleAudioPowerDown		0xffffffffffffffffULL

enum {
    kIOAudioPortTypeOutput		= 'outp',
    kIOAudioPortTypeInput		= 'inpt',
    kIOAudioPortTypeMixer		= 'mixr',
    kIOAudioPortTypePassThru	= 'pass',
    kIOAudioPortTypeProcessing	= 'proc'
};

enum {
    kIOAudioOutputPortSubTypeInternalSpeaker	= 'ispk',
    kIOAudioOutputPortSubTypeExternalSpeaker	= 'espk',
    kIOAudioOutputPortSubTypeHeadphones			= 'hdpn',
    kIOAudioOutputPortSubTypeLine				= 'line',
    kIOAudioOutputPortSubTypeSPDIF				= 'spdf',
    
    kIOAudioInputPortSubTypeInternalMicrophone	= 'imic',
    kIOAudioInputPortSubTypeExternalMicrophone	= 'emic',
    kIOAudioInputPortSubTypeCD					= 'cd  ',
    kIOAudioInputPortSubTypeLine				= 'line',
    kIOAudioInputPortSubTypeSPDIF				= 'spdf'
};

enum {
    kIOAudioControlTypeLevel			= 'levl',
    kIOAudioControlTypeToggle			= 'togl',
	kIOAudioControlTypeJack				= 'jack',
    kIOAudioControlTypeSelector			= 'slct'
};

//	<rdar://8325563>	Added kIOAudioToggleControlSubTypePhantomPower, kIOAudioToggleControlSubTypePhaseInvert & 
//	kIOAudioSelectorControlSubTypeChannelHighPassFilter
enum {
    kIOAudioLevelControlSubTypeVolume						= 'vlme',
	kIOAudioLevelControlSubTypeLFEVolume					= 'subv',
	kIOAudioLevelControlSubTypePRAMVolume					= 'pram',
    kIOAudioToggleControlSubTypeMute						= 'mute',
    kIOAudioToggleControlSubTypeSolo						= 'solo',
	kIOAudioToggleControlSubTypeLFEMute						= 'subm',
	kIOAudioToggleControlSubTypeiSubAttach					= 'atch',
	kIOAudioToggleControlSubTypePhantomPower				= 'phan',
	kIOAudioToggleControlSubTypePhaseInvert					= 'phsi',
    kIOAudioSelectorControlSubTypeOutput					= 'outp',
    kIOAudioSelectorControlSubTypeInput						= 'inpt',
    kIOAudioSelectorControlSubTypeClockSource				= 'clck',
    kIOAudioSelectorControlSubTypeDestination				= 'dest',
	kIOAudioSelectorControlSubTypeChannelNominalLineLevel	= 'nlev',
	kIOAudioSelectorControlSubTypeChannelLevelPlus4dBu		= '4dbu',
	kIOAudioSelectorControlSubTypeChannelLevelMinus10dBV	= '10db',
	kIOAudioSelectorControlSubTypeChannelLevelMinus20dBV	= '20db',
	kIOAudioSelectorControlSubTypeChannelLevelMicLevel		= 'micl',
	kIOAudioSelectorControlSubTypeChannelLevelInstrumentLevel		= 'istl',
	kIOAudioSelectorControlSubTypeChannelHighPassFilter		= 'hipf'
};

enum {
    kIOAudioControlUsageOutput				= 'outp',
    kIOAudioControlUsageInput				= 'inpt',
    kIOAudioControlUsagePassThru			= 'pass',
    kIOAudioControlUsageCoreAudioProperty	= 'prop'
};

enum {
    kIOAudioControlChannelNumberInactive				= -1,
    kIOAudioControlChannelIDAll							= 0,
    kIOAudioControlChannelIDDefaultLeft					= 1,
    kIOAudioControlChannelIDDefaultRight				= 2,
    kIOAudioControlChannelIDDefaultCenter				= 3,
    kIOAudioControlChannelIDDefaultLeftRear				= 4,
    kIOAudioControlChannelIDDefaultRightRear			= 5,
    kIOAudioControlChannelIDDefaultSub					= 6,
    kIOAudioControlChannelIDDefaultFrontLeftCenter		= 7,
    kIOAudioControlChannelIDDefaultFrontRightCenter		= 8,
    kIOAudioControlChannelIDDefaultRearCenter			= 9,
    kIOAudioControlChannelIDDefaultSurroundLeft			= 10,
    kIOAudioControlChannelIDDefaultSurroundRight		= 11
};

enum {
    kIOAudioSelectorControlSelectionValueNone				= 'none',
    
    // Output-specific selection IDs 
    kIOAudioSelectorControlSelectionValueInternalSpeaker	= 'ispk',
    kIOAudioSelectorControlSelectionValueExternalSpeaker	= 'espk',
    kIOAudioSelectorControlSelectionValueHeadphones			= 'hdpn',
    
    // Input-specific selection IDs
    kIOAudioSelectorControlSelectionValueInternalMicrophone	= 'imic',
    kIOAudioSelectorControlSelectionValueExternalMicrophone	= 'emic',
    kIOAudioSelectorControlSelectionValueCD					= 'cd  ',
    
    // Common selection IDs
    kIOAudioSelectorControlSelectionValueLine				= 'line',
    kIOAudioSelectorControlSelectionValueSPDIF				= 'spdf'
};

enum {
    kIOAudioStreamSampleFormatLinearPCM		= 'lpcm',
    kIOAudioStreamSampleFormatIEEEFloat		= 'ieee',
    kIOAudioStreamSampleFormatALaw			= 'alaw',
    kIOAudioStreamSampleFormatMuLaw			= 'ulaw',
    kIOAudioStreamSampleFormatMPEG			= 'mpeg',
    kIOAudioStreamSampleFormatAC3			= 'ac-3',
    kIOAudioStreamSampleFormat1937AC3		= 'cac3',
    kIOAudioStreamSampleFormat1937MPEG1		= 'mpg1',
    kIOAudioStreamSampleFormat1937MPEG2		= 'mpg2',
	kIOAudioStreamSampleFormatTimeCode		= 'time'		//	a stream of IOAudioTimeStamp structures that capture any incoming time code information
};

enum {
    kIOAudioStreamNumericRepresentationSignedInt	= 'sint',
    kIOAudioStreamNumericRepresentationUnsignedInt	= 'uint',
	kIOAudioStreamNumericRepresentationIEEE754Float = 'flot'
};

enum {
	kIOAudioClockSelectorTypeInternal			= 'int ',
	kIOAudioClockSelectorTypeExternal			= 'ext ',
	kIOAudioClockSelectorTypeAESEBU				= 'asbu',
	kIOAudioClockSelectorTypeTOSLink			= 'tosl',
	kIOAudioClockSelectorTypeSPDIF				= 'spdf',
	kIOAudioClockSelectorTypeADATOptical		= 'adto',
	kIOAudioClockSelectorTypeADAT9Pin			= 'adt9',
	kIOAudioClockSelectorTypeSMPTE				= 'smpt',
	kIOAudioClockSelectorTypeVideo				= 'vdeo',
	kIOAudioClockSelectorTypeControl			= 'cnrl',
	kIOAudioClockSelectorTypeOther				= 'othr',
};

enum {
    kIOAudioStreamAlignmentLowByte					= 0,
    kIOAudioStreamAlignmentHighByte					= 1
};

enum {
    kIOAudioStreamByteOrderBigEndian				= 0,
    kIOAudioStreamByteOrderLittleEndian				= 1
};

enum {
    kIOAudioLevelControlNegativeInfinity			= 0xffffffff
};

enum {
#ifndef __OPEN_SOURCE__
    kIOAudioBuiltInSystemClockDomain                = 0x737973,
#endif
    kIOAudioNewClockDomain							= 0xffffffff
};

// Device connection types
#ifndef __OPEN_SOURCE__
//	<rdar://7130813>	Added kIOAudioDeviceTransportTypeDisplayPort
#endif
enum {
	kIOAudioDeviceTransportTypeBuiltIn				= 'bltn',
	kIOAudioDeviceTransportTypePCI					= 'pci ',
	kIOAudioDeviceTransportTypeUSB					= 'usb ',
	kIOAudioDeviceTransportTypeFireWire				= '1394',
	kIOAudioDeviceTransportTypeNetwork				= 'ntwk',
	kIOAudioDeviceTransportTypeWireless				= 'wrls',
	kIOAudioDeviceTransportTypeOther				= 'othr',
	kIOAudioDeviceTransportTypeBluetooth			= 'blue',
	kIOAudioDeviceTransportTypeVirtual				= 'virt',
	kIOAudioDeviceTransportTypeDisplayPort			= 'dprt',
	kIOAudioDeviceTransportTypeHdmi					= 'hdmi',
    kIOAudioDeviceTransportTypeAVB            		= 'eavb',			//<rdar://10874672>
    kIOAudioDeviceTransportTypeThunderbolt    		= 'thun'			//<rdar://10874672>
};

// types that go nowhere
enum {
	OUTPUT_NULL										= 0x0100,
	INPUT_NULL										= 0x0101
};

// Input terminal types
enum {
	INPUT_UNDEFINED									= 0x0200,
	INPUT_MICROPHONE								= 0x0201,
	INPUT_DESKTOP_MICROPHONE						= 0x0202,
	INPUT_PERSONAL_MICROPHONE						= 0x0203,
	INPUT_OMNIDIRECTIONAL_MICROPHONE				= 0x0204,
	INPUT_MICROPHONE_ARRAY							= 0x0205,
	INPUT_PROCESSING_MICROPHONE_ARRAY				= 0x0206,
	INPUT_MODEM_AUDIO								= 0x207
};

// Output terminal types
enum {
	OUTPUT_UNDEFINED								= 0x0300,
	OUTPUT_SPEAKER									= 0x0301,
	OUTPUT_HEADPHONES								= 0x0302,
	OUTPUT_HEAD_MOUNTED_DISPLAY_AUDIO				= 0x0303,
	OUTPUT_DESKTOP_SPEAKER							= 0x0304,
	OUTPUT_ROOM_SPEAKER								= 0x0305,
	OUTPUT_COMMUNICATION_SPEAKER					= 0x0306,
	OUTPUT_LOW_FREQUENCY_EFFECTS_SPEAKER			= 0x0307
};

// Bi-directional terminal types
enum {
	BIDIRECTIONAL_UNDEFINED							= 0x0400,
	BIDIRECTIONAL_HANDSET							= 0x0401,
	BIDIRECTIONAL_HEADSET							= 0x0402,
	BIDIRECTIONAL_SPEAKERPHONE_NO_ECHO_REDX			= 0x0403,
	BIDIRECTIONAL_ECHO_SUPPRESSING_SPEAKERPHONE		= 0x0404,
	BIDIRECTIONAL_ECHO_CANCELING_SPEAKERPHONE		= 0x0405
};

// Telephony terminal types
enum {
	TELEPHONY_UNDEFINED								= 0x0500,
	TELEPHONY_PHONE_LINE							= 0x0501,
	TELEPHONY_TELEPHONE								= 0x0502,
	TELEPHONY_DOWN_LINE_PHONE						= 0x0503
};

// External terminal types
enum {
	EXTERNAL_UNDEFINED								= 0x0600,
	EXTERNAL_ANALOG_CONNECTOR						= 0x0601,
	EXTERNAL_DIGITAL_AUDIO_INTERFACE				= 0x0602,
	EXTERNAL_LINE_CONNECTOR							= 0x0603,
	EXTERNAL_LEGACY_AUDIO_CONNECTOR					= 0x0604,
	EXTERNAL_SPDIF_INTERFACE						= 0x0605,
	EXTERNAL_1394_DA_STREAM							= 0x0606,
	EXTERNAL_1394_DV_STREAM_SOUNDTRACK				= 0x0607,
	EXTERNAL_ADAT									= 0x0608,
	EXTERNAL_TDIF									= 0x0609,
	EXTERNAL_MADI									= 0x060A
};

// Embedded terminal types
enum {
	EMBEDDED_UNDEFINED								= 0x0700,
	EMBEDDED_LEVEL_CALIBRATION_NOISE_SOURCE			= 0x0701,
	EMBEDDED_EQUALIZATION_NOISE						= 0x0702,
	EMBEDDED_CD_PLAYER								= 0x0703,
	EMBEDDED_DAT									= 0x0704,
	EMBEDDED_DCC									= 0x0705,
	EMBEDDED_MINIDISK								= 0x0706,
	EMBEDDED_ANALOG_TAPE							= 0x0707,
	EMBEDDED_PHONOGRAPH								= 0x0708,
	EMBEDDED_VCR_AUDIO								= 0x0709,
	EMBEDDED_VIDEO_DISC_AUDIO						= 0x070A,
	EMBEDDED_DVD_AUDIO								= 0x070B,
	EMBEDDED_TV_TUNER_AUDIO							= 0x070C,
	EMBEDDED_SATELLITE_RECEIVER_AUDIO				= 0x070D,
	EMBEDDED_CABLE_TUNER_AUDIO						= 0x070E,
	EMBEDDED_DSS_AUDIO								= 0x070F,
	EMBEDDED_RADIO_RECEIVER							= 0x0710,
	EMBEDDED_RADIO_TRANSMITTER						= 0x0711,
	EMBEDDED_MULTITRACK_RECORDER					= 0x0712,
	EMBEDDED_SYNTHESIZER							= 0x0713
};

// Processing terminal types
enum {
	PROCESSOR_UNDEFINED								= 0x0800,
	PROCESSOR_GENERAL								= 0x0801
};

//	Channel spatial position types

#ifndef __OPEN_SOURCE__
//	<rdar://6868206>	NOTE: the following are derived from CoreAudioTypes.h
#endif

#define	kIOAudioChannelLabel_Discrete_field_ba		16
enum {
    kIOAudioChannelLabel_Unknown                  = 0xFFFFFFFF,   // unknown or unspecified other use
    kIOAudioChannelLabel_Unused                   = 0,            // channel is present, but has no intended use or destination
    kIOAudioChannelLabel_UseCoordinates           = 100,          // channel is described by the mCoordinates fields.
	
    kIOAudioChannelLabel_Left                     = 1,
    kIOAudioChannelLabel_Right                    = 2,
    kIOAudioChannelLabel_Center                   = 3,
    kIOAudioChannelLabel_LFEScreen                = 4,
    kIOAudioChannelLabel_LeftSurround             = 5,            // WAVE: "Back Left"
    kIOAudioChannelLabel_RightSurround            = 6,            // WAVE: "Back Right"
    kIOAudioChannelLabel_LeftCenter               = 7,
    kIOAudioChannelLabel_RightCenter              = 8,
    kIOAudioChannelLabel_CenterSurround           = 9,            // WAVE: "Back Center" or plain "Rear Surround"
    kIOAudioChannelLabel_LeftSurroundDirect       = 10,           // WAVE: "Side Left"
    kIOAudioChannelLabel_RightSurroundDirect      = 11,           // WAVE: "Side Right"
    kIOAudioChannelLabel_TopCenterSurround        = 12,
    kIOAudioChannelLabel_VerticalHeightLeft       = 13,           // WAVE: "Top Front Left"
    kIOAudioChannelLabel_VerticalHeightCenter     = 14,           // WAVE: "Top Front Center"
    kIOAudioChannelLabel_VerticalHeightRight      = 15,           // WAVE: "Top Front Right"
	
    kIOAudioChannelLabel_TopBackLeft              = 16,
    kIOAudioChannelLabel_TopBackCenter            = 17,
    kIOAudioChannelLabel_TopBackRight             = 18,
	
    kIOAudioChannelLabel_RearSurroundLeft         = 33,
    kIOAudioChannelLabel_RearSurroundRight        = 34,
    kIOAudioChannelLabel_LeftWide                 = 35,
    kIOAudioChannelLabel_RightWide                = 36,
    kIOAudioChannelLabel_LFE2                     = 37,
    kIOAudioChannelLabel_LeftTotal                = 38,           // matrix encoded 4 channels
    kIOAudioChannelLabel_RightTotal               = 39,           // matrix encoded 4 channels
    kIOAudioChannelLabel_HearingImpaired          = 40,
    kIOAudioChannelLabel_Narration                = 41,
    kIOAudioChannelLabel_Mono                     = 42,
    kIOAudioChannelLabel_DialogCentricMix         = 43,
	
    kIOAudioChannelLabel_CenterSurroundDirect     = 44,           // back center, non diffuse
    
    kIOAudioChannelLabel_Haptic                   = 45,
	
    // first order ambisonic channels
    kIOAudioChannelLabel_Ambisonic_W              = 200,
    kIOAudioChannelLabel_Ambisonic_X              = 201,
    kIOAudioChannelLabel_Ambisonic_Y              = 202,
    kIOAudioChannelLabel_Ambisonic_Z              = 203,
	
    // Mid/Side Recording
    kIOAudioChannelLabel_MS_Mid                   = 204,
    kIOAudioChannelLabel_MS_Side                  = 205,
	
    // X-Y Recording
    kIOAudioChannelLabel_XY_X                     = 206,
    kIOAudioChannelLabel_XY_Y                     = 207,
	
    // other
    kIOAudioChannelLabel_HeadphonesLeft           = 301,
    kIOAudioChannelLabel_HeadphonesRight          = 302,
    kIOAudioChannelLabel_ClickTrack               = 304,
    kIOAudioChannelLabel_ForeignLanguage          = 305,
	
    // generic discrete channel
    kIOAudioChannelLabel_Discrete                 = 400,
	
    // numbered discrete channel
    kIOAudioChannelLabel_Discrete_0               = ( 1 << kIOAudioChannelLabel_Discrete_field_ba ) | 0,
    kIOAudioChannelLabel_Discrete_1               = ( 1 << kIOAudioChannelLabel_Discrete_field_ba ) | 1,
    kIOAudioChannelLabel_Discrete_2               = ( 1 << kIOAudioChannelLabel_Discrete_field_ba ) | 2,
    kIOAudioChannelLabel_Discrete_3               = ( 1 << kIOAudioChannelLabel_Discrete_field_ba ) | 3,
    kIOAudioChannelLabel_Discrete_4               = ( 1 << kIOAudioChannelLabel_Discrete_field_ba ) | 4,
    kIOAudioChannelLabel_Discrete_5               = ( 1 << kIOAudioChannelLabel_Discrete_field_ba ) | 5,
    kIOAudioChannelLabel_Discrete_6               = ( 1 << kIOAudioChannelLabel_Discrete_field_ba ) | 6,
    kIOAudioChannelLabel_Discrete_7               = ( 1 << kIOAudioChannelLabel_Discrete_field_ba ) | 7,
    kIOAudioChannelLabel_Discrete_8               = ( 1 << kIOAudioChannelLabel_Discrete_field_ba ) | 8,
    kIOAudioChannelLabel_Discrete_9               = ( 1 << kIOAudioChannelLabel_Discrete_field_ba ) | 9,
    kIOAudioChannelLabel_Discrete_10              = ( 1 << kIOAudioChannelLabel_Discrete_field_ba ) | 10,
    kIOAudioChannelLabel_Discrete_11              = ( 1 << kIOAudioChannelLabel_Discrete_field_ba ) | 11,
    kIOAudioChannelLabel_Discrete_12              = ( 1 << kIOAudioChannelLabel_Discrete_field_ba ) | 12,
    kIOAudioChannelLabel_Discrete_13              = ( 1 << kIOAudioChannelLabel_Discrete_field_ba ) | 13,
    kIOAudioChannelLabel_Discrete_14              = ( 1 << kIOAudioChannelLabel_Discrete_field_ba ) | 14,
    kIOAudioChannelLabel_Discrete_15              = ( 1 << kIOAudioChannelLabel_Discrete_field_ba ) | 15,
    kIOAudioChannelLabel_Discrete_65535           = ( 1 << kIOAudioChannelLabel_Discrete_field_ba ) | 65535
};



#endif /* _IOKIT_IOAUDIOTYPES_H */
