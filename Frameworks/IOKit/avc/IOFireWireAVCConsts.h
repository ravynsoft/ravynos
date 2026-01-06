/*
 * Copyright (c) 1998-2001 Apple Computer, Inc. All rights reserved.
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
#ifndef _IOKIT_IOFIREWIREAVCCONSTS_H
#define _IOKIT_IOFIREWIREAVCCONSTS_H

// Fields of AVC frame
typedef enum {
    kAVCCommandResponse = 0,
    kAVCAddress = 1,
    kAVCOpcode = 2,
    kAVCOperand0 = 3,
    kAVCOperand1 = 4,
    kAVCOperand2 = 5,
    kAVCOperand3 = 6,
    kAVCOperand4 = 7,
    kAVCOperand5 = 8,
    kAVCOperand6 = 9,
    kAVCOperand7 = 10,
    kAVCOperand8 = 11
} IOAVCFrameFields;

// Command/Response values
typedef enum {
    kAVCControlCommand                      = 0x00,
    kAVCStatusInquiryCommand                = 0x01,
    kAVCSpecificInquiryCommand				= 0x02,
    kAVCNotifyCommand						= 0x03,
    kAVCGeneralInquiryCommand				= 0x04,
    kAVCNotImplementedStatus				= 0x08,
    kAVCAcceptedStatus						= 0x09,
    kAVCRejectedStatus						= 0x0a,
    kAVCInTransitionStatus					= 0x0b,
    kAVCImplementedStatus					= 0x0c,
    kAVCChangedStatus						= 0x0d,
    kAVCInterimStatus						= 0x0f
} IOAVCCommandResponse;

// Opcodes
typedef enum {

    // Unit commands
    kAVCPlugInfoOpcode						= 0x02,
    kAVCOutputPlugSignalFormatOpcode		= 0x18,
    kAVCInputPlugSignalFormatOpcode			= 0x19,
    kAVCUnitInfoOpcode						= 0x30,
    kAVCSubunitInfoOpcode					= 0x31,
    kAVCConnectionsOpcode					= 0x22,
    kAVCConnectOpcode						= 0x24,
    kAVCDisconnectOpcode					= 0x25,
    kAVCPowerOpcode							= 0xB2,
	kAVCSignalSourceOpcode					= 0x1A,
	
    // Vendor dependent commands
    kAVCVendorDependentOpcode				= 0x00,

    // Subunit commands
    kAVCOutputSignalModeOpcode              = 0x78,
    kAVCInputSignalModeOpcode               = 0x79, 
    kAVCSignalModeSD525_60                  = 0x00,
    kAVCSignalModeSDL525_60                 = 0x04,
    kAVCSignalModeHD1125_60                 = 0x08,
    kAVCSignalModeSD625_50                  = 0x80, 
    kAVCSignalModeSDL625_50                 = 0x84, 
    kAVCSignalModeHD1250_50                 = 0x88,
    kAVCSignalModeDVCPro525_60				= 0x78,
    kAVCSignalModeDVCPro625_50				= 0xf8,
    
    kAVCSignalModeDummyOperand              = 0xff,
    kAVCSignalModeMask_50					= 0x80,
    kAVCSignalModeMask_STYPE				= 0x7c,          
    kAVCSignalModeMask_SDL					= 0x04,          
    kAVCSignalModeMask_DVCPro25				= 0x78        

} IOAVCOpcodes;

// Unit/Subunit types
typedef enum {
    kAVCVideoMonitor						= 0x00,
    kAVCAudio								= 0x01,
    kAVCPrinter								= 0x02,
    kAVCDiskRecorder						= 0x03,
    kAVCTapeRecorder						= 0x04,
    kAVCTuner								= 0x05,
    kAVCVideoCamera							= 0x07,
    kAVCCameraStorage						= 0x0b,
    kAVCVendorUnique						= 0x1c,
    kAVCNumSubUnitTypes						= 0x20
} IOAVCUnitTypes;

#define kAVCAllOpcodes 0xFF
#define kAVCAllSubunitsAndUnit 0xEE
#define kAVCMaxNumPlugs 31
#define kAVCAnyAvailableIsochPlug 0x7F
#define kAVCAnyAvailableExternalPlug 0xFF
#define kAVCAnyAvailableSubunitPlug 0xFF
#define kAVCMultiplePlugs 0xFD
#define kAVCInvalidPlug 0xFE


#define IOAVCAddress(type, id) (((type) << 3) | (id))
#define kAVCUnitAddress 0xff
#define IOAVCType(address) ((address) >> 3)
#define IOAVCId(address) ((address) & 0x7)

// Macros for Plug Control Register field manipulation

// Master control registers
#define kIOFWPCRDataRate FWBitRange(0,1)
#define kIOFWPCRDataRatePhase FWBitRangePhase(0,1)
#define kIOFWPCRExtension FWBitRange(8,15)
#define kIOFWPCRExtensionPhase FWBitRangePhase(8,15)
#define kIOFWPCRNumPlugs FWBitRange(27,31)
#define kIOFWPCRNumPlugsPhase FWBitRangePhase(27,31)

// master output register
#define kIOFWPCRBroadcastBase FWBitRange(2,7)
#define kIOFWPCRBroadcastBasePhase FWBitRangePhase(2,7)

// plug registers
#define kIOFWPCROnline FWBitRange(0,0)
#define kIOFWPCROnlinePhase FWBitRangePhase(0,0)
#define kIOFWPCRBroadcast FWBitRange(1,1)
#define kIOFWPCRBroadcastPhase FWBitRangePhase(1,1)
#define kIOFWPCRP2PCount FWBitRange(2,7)
#define kIOFWPCRP2PCountPhase FWBitRangePhase(2,7)
#define kIOFWPCRChannel FWBitRange(10,15)
#define kIOFWPCRChannelPhase FWBitRangePhase(10,15)

// Extra fields for output plug registers
#define kIOFWPCROutputDataRate FWBitRange(16,17)
#define kIOFWPCROutputDataRatePhase FWBitRangePhase(16,17)
#define kIOFWPCROutputOverhead FWBitRange(18,21)
#define kIOFWPCROutputOverheadPhase FWBitRangePhase(18,21)
#define kIOFWPCROutputPayload FWBitRange(22,31)
#define kIOFWPCROutputPayloadPhase FWBitRangePhase(22,31)

// async plug numbers

enum
{
	kFWAVCAsyncPlug0 	= 0xa0,
	kFWAVCAsyncPlug1 	= 0xa1,
	kFWAVCAsyncPlug2 	= 0xa2,
	kFWAVCAsyncPlug3 	= 0xa3,
	kFWAVCAsyncPlug4 	= 0xa4,
	kFWAVCAsyncPlug5 	= 0xa5,
	kFWAVCAsyncPlug6 	= 0xa6,
	kFWAVCAsyncPlug7 	= 0xa7,
	kFWAVCAsyncPlug8 	= 0xa8,
	kFWAVCAsyncPlug9 	= 0xa9,
	kFWAVCAsyncPlug10 	= 0xa1,
	kFWAVCAsyncPlug11	= 0xab,
	kFWAVCAsyncPlug12 	= 0xac,
	kFWAVCAsyncPlug13	= 0xad,
	kFWAVCAsyncPlug14	= 0xae,
	kFWAVCAsyncPlug15 	= 0xaf,
	kFWAVCAsyncPlug16 	= 0xb0,
	kFWAVCAsyncPlug17 	= 0xb1,
	kFWAVCAsyncPlug18	= 0xb2,
	kFWAVCAsyncPlug19 	= 0xb3,
	kFWAVCAsyncPlug20 	= 0xb4,
	kFWAVCAsyncPlug21 	= 0xb5,
	kFWAVCAsyncPlug22 	= 0xb6,
	kFWAVCAsyncPlug23 	= 0xb7,
	kFWAVCAsyncPlug24 	= 0xb8,
	kFWAVCAsyncPlug25 	= 0xb9,
	kFWAVCAsyncPlug26 	= 0xba,
	kFWAVCAsyncPlug27 	= 0xbb,
	kFWAVCAsyncPlug28 	= 0xbc,
	kFWAVCAsyncPlug29 	= 0xbd,
	kFWAVCAsyncPlug30	= 0xbe,
	kFWAVCAsyncPlugAny	= 0xbf
};

enum
{
    kFWAVCStateBusSuspended 	= 0,
    kFWAVCStateBusResumed 		= 1,
    kFWAVCStatePlugReconnected 	= 2,
    kFWAVCStatePlugDisconnected = 3,
    kFWAVCStateDeviceRemoved	= 4
};

enum
{
    kFWAVCConsumerMode_MORE		= 1,
    kFWAVCConsumerMode_LAST		= 4,
    kFWAVCConsumerMode_LESS		= 5,
    kFWAVCConsumerMode_JUNK		= 6,
    kFWAVCConsumerMode_LOST		= 7
};

enum
{
    kFWAVCProducerMode_SEND		= 5,
    kFWAVCProducerMode_TOSS		= 7
};


typedef enum
{
	IOFWAVCPlugSubunitSourceType,
	IOFWAVCPlugSubunitDestType,
	IOFWAVCPlugIsochInputType,
	IOFWAVCPlugIsochOutputType,
	IOFWAVCPlugAsynchInputType,
	IOFWAVCPlugAsynchOutputType,
	IOFWAVCPlugExternalInputType,
	IOFWAVCPlugExternalOutputType
} IOFWAVCPlugTypes;

typedef enum
{
	kIOFWAVCSubunitPlugMsgConnected,
	kIOFWAVCSubunitPlugMsgDisconnected,
	kIOFWAVCSubunitPlugMsgConnectedPlugModified,
	kIOFWAVCSubunitPlugMsgSignalFormatModified
} IOFWAVCSubunitPlugMessages;

// Some plug signal formats
#define kAVCPlugSignalFormatNTSCDV 0x80000000
#define kAVCPlugSignalFormatPalDV 0x80800000 
#define kAVCPlugSignalFormatMPEGTS 0xA0000000

// Possible states of an AVCAsynchronousCommand
typedef enum
{
	kAVCAsyncCommandStatePendingRequest,
	kAVCAsyncCommandStateRequestSent,
	kAVCAsyncCommandStateRequestFailed,
	kAVCAsyncCommandStateWaitingForResponse,
	kAVCAsyncCommandStateReceivedInterimResponse,
	kAVCAsyncCommandStateReceivedFinalResponse,
	kAVCAsyncCommandStateTimeOutBeforeResponse,
	kAVCAsyncCommandStateBusReset,
	kAVCAsyncCommandStateOutOfMemory,
	kAVCAsyncCommandStateCanceled
} IOFWAVCAsyncCommandState;

#endif // _IOKIT_IOFIREWIREAVCCONSTS_H
