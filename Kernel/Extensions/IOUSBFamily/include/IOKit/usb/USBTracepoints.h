/*
 * Copyright © 2009-2013 Apple Inc.  All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
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

#ifndef __IOKIT_IO_USB_FAMILY_TRACEPOINTS__
#define __IOKIT_IO_USB_FAMILY_TRACEPOINTS__

#include <IOKit/IOTypes.h>

#include <sys/kdebug.h>

#ifdef __cplusplus
extern "C" {
#endif
	
#define USB_SYSCTL			"debug.USB"
#define kUSBTypeDebug		'USBD'
#define DEBUG_UNUSED( X )	( void )( X )
	
	extern UInt32 gUSBStackDebugFlags;
	extern UInt32 gEHCIBandwidthLogLevel;
	extern UInt32 gEHCIRLvalue;
	extern UInt32 gXHCIEPlimit;
	
	typedef struct USBSysctlArgs
		{
			uint32_t		type;
			uint32_t		operation;
			uint32_t		debugFlags;
		} USBSysctlArgs;
	
	enum
	{
		kUSBOperationGetFlags 	= 0,
		kUSBOperationSetFlags	= 1
	};
	
	// the following bits/masks are for use in the usb boot args
	// e.g. boot-args="usb=0x102" will turn on Trace Points and set the EHCI retry count to 1
	// the bits can also be set with a sysctl call
	enum
	{
		kUSBEnableDebugLoggingBit		= 0,	// bit 0 currently not used
		kUSBEnableTracePointsBit		= 1,	// bit 1 used to turn on Trace Points when a USB controller first loads
		kUSBEnableErrorLogBit			= 2,	// bit 2 (4) turns level 1 log for errors
        kUSBPanicLostRegisterAccessBit  = 3,    // bit 3 (0x08) If we read invalid values from a USB controller (built-in or TB), panic
		kUSBForceCompanionControllers	= 4,	// bit 4 (0x10) forces companion controllers to be on even if they might not normally
  		kUSBDontAllowHubLowPower		= 5,	// bit 5 (0x20) prevents hubs from being automatically suspended
        kUSBDontAllowDeepIdle           = 6,    // bit 6 (0x40) prevents hubs and controllers from doing "deep idle" or sleep mode while running
        
		kUSBDebugRetryCountShift		= 8,	// bits 8 and 9 will set the retry count for low level bus transactions
		kUSBDebugRetryCountReserved		= 9,	// must be 1, 2, or 3 (0 is invalid, 3 is the default)
        
		kUSBDisableMuxedPorts			= 11,	// bit 11 prevents xMuxing EHCI
		kUSBEnableAllXHCIControllers	= 12,	// bit 12: Will allow XHCI driver to match to any vendor/productID
		kUSBUASControl					= 13,	// bit 13: Disable UAS support
        kUSBMasterAbortLogging          = 14,
        kUSBMasterAbortPanic            = 15,
		
		kUSBEnableDebugLoggingMask			= (1 << kUSBEnableDebugLoggingBit),     // 0x0001
		kUSBEnableTracePointsMask			= (1 << kUSBEnableTracePointsBit),      // 0x0002
		kUSBEnableErrorLogMask				= (1 << kUSBEnableErrorLogBit),         // 0x0004
		kUSBPanicLostRegisterAccessMask     = (1 << kUSBPanicLostRegisterAccessBit),// 0x0008
		kUSBForceCompanionControllersMask	= (1 << kUSBForceCompanionControllers), // 0x0010
		kUSBDontAllowHubLowPowerMask		= (1 << kUSBDontAllowHubLowPower),      // 0x0020
        kUSBDontAllowDeepIdleMask           = (1 << kUSBDontAllowDeepIdle),			// 0x0040
		kUSBDebugRetryCountMask				= (3 << kUSBDebugRetryCountShift),      // 0x0300
		kUSBDisableMuxedPortsMask			= (1 << kUSBDisableMuxedPorts),         // 0x0800
		kUSBEnableAllXHCIControllersMask	= (1 << kUSBEnableAllXHCIControllers),  // 0x1000
		kUSBUASControlMask					= (1 << kUSBUASControl),                // 0x2000
        kUSBMasterAbortLoggingMask          = (1 << kUSBMasterAbortLogging),        // 0x4000
        kUSBMasterAbortPanicMask            = (1 << kUSBMasterAbortPanic)           // 0x8000
	};
	
	
	/* Kernel Tracepoints
	 *
	 * Kernel tracepoints are a logging mechanism reduces the size of a log-laden binary.
	 * Codes are placed into a buffer, from the kernel, and picked up by a userspace
	 * tool that displays a unique log message for each tracepoint. Additionally, each
	 * tracepoint may contain up-to four 32-bit (may change with LP64) arguments. 
	 *
	 * To add a tracepoint, use the code below as an example:
	 * USBTrace( kUSBTController, kTPControllerStart, (uintptr_t)myArgValue );
	 * Next, add the corresponding tracepoint code in the USBTracer tool, using
	 * the existing examples. Avoid using confidential information in the log strings.
	 * Some functions have a argument counter, to signify which of the function's tracepoints
	 * are actually being logged. When adding a tracepoint using an existing code, you
	 * must verify that you increment this argument counter properly.
	 *
	 * The trace codes consist of the following:
	 *
	 * ----------------------------------------------------------------------
	 *| Class (8)      | SubClass (8)   | USBGroup(6) | Code (8)    |Func   |
	 *| DBG_IOKIT      | DBG_IOUSB      |             |             |Qual(2)|
	 * ----------------------------------------------------------------------
	 *
	 * DBG_IOKIT(05h)  DBG_IOUSB(2Dh)
	 *
	 * See <sys/kdebug.h> and IOTimeStamp.h for more details.
	 */
	
	
	// USB groupings (max of 64)
	enum
	{
		// Family groupings
		kUSBTController				= 0,
		kUSBTControllerUserClient	= 1,
		
		kUSBTDevice					= 2,
		kUSBTDeviceUserClient		= 3,

		kUSBTHub					= 4,
		kUSBTHubPort				= 5,
		kUSBTHSHubUserClient		= 6,
		kUSBTHID					= 7,
		kUSBTPipe					= 8,

		kUSBTInterfaceUserClient	= 9,
		
		kUSBTEnumeration				= 10,		// Tracepoints in various components to debug USB enumeration

		
		// UIM groupings
		kUSBTUHCI					= 11,
		kUSBTUHCIUIM				= 12,
		kUSBTUHCIInterrupts			= 13,

		kUSBTOHCI					= 14,
		kUSBTOHCIInterrupts			= 15,
		kUSBTOHCIDumpQueues			= 16,
		
		kUSBTEHCI					= 20,
		kUSBTEHCIHubInfo			= 22,
		kUSBTEHCIInterrupts			= 23,
		kUSBTEHCIDumpQueues			= 24,
		
		kUSBTXHCI					= 28,
		kUSBTXHCIInterrupts			= 29,
		kUSBTXHCIRootHubs			= 30,
		kUSBTXHCIPrintTRB			= 31,

		// 30-33 reserved

		// 21-25 reserved
		kUSBTHubPolicyMaker			= 35,
		kUSBTCompositeDriver		= 36,
		
		// Actions
		kUSBTOutstandingIO			= 42,

		// kUSBAudio
		kUSBAudio					= 50

		// 61-63 reserved
	};
	
	// USB Controller Tracepoints		0x052D0000 - 0x052D03FF
	// kUSBTController
	enum
	{
		kTPControllerStart							= 1,
		kTPControllerControlPacketHandler			= 2,
		kTPControllerMakeDevice						= 3,
		kTPControllerMakeHubDevice					= 4,
		kTPControllerCreateRootHubDevice			= 5,
		kTPControllerClearTTHandler					= 6,
		kTPControllerClearTT						= 7,
		kTPControllerDoCreateEP						= 8,
		kTPControllerReadV2							= 9,
		kTPControllerReturnIsochDoneQueue			= 10,
		kTPControllerPowerState                     = 11,
		kTPControllerCheckPowerModeBeforeGatedCall	= 12,
		kTPControllerGatedPowerChange				= 13,
		kTPControllerCheckForRootHubChanges			= 14,
		kTPControllerRootHubQueueInterruptRead		= 15,
		kTPControllerRootHubTimer					= 16,
		kTPControllerDisjointCompletion				= 17,
		kTPControllerCheckForDisjointDescriptor		= 18,
		kTPControllerRead							= 19,
		kTPControllerV3Start						= 20,
		kTPAllocatePowerStateArray					= 21,
		kTPInitForPM								= 22,
		kTPIsocIOLL									= 23,
		kTPIsocIO									= 24,
		kTPControllerWrite							= 25,
		kTPCompletionCall							= 26,
		kTPControlTransaction						= 27,
		kTPInterruptTransaction						= 28,
		kTPInterruptTransactionData					= 29,
		kTPBulkTransaction							= 30,
		kTPBulkTransactionData						= 31,
		kTPIsocTransaction							= 32,
		kTPInterruptPacketHandler					= 33,
		kTPBulkPacketHandler						= 34,
		kTPDevZeroLock								= 35,
		kTPControlPacketHandlerData					= 36,
		kTPDoIOTransferIntrSync						= 37,
		kTPDoIOTransferBulkSync						= 38,
		kTPBulkPacketHandlerData					= 39,
		kTPInterruptPacketHandlerData				= 40,
        kTPControllerPutTDOnDoneQueue               = 41,
        kTPControllerHibernationWake                = 42
	};
	
	// USB Device Tracepoints			
	// kUSBTDevice
	enum
	{
		kTPDeviceInit								= 1,
		kTPDeviceMessage							= 2,
		kTPDeviceResetDevice						= 3,
		kTPDeviceGetFullConfigurationDescriptor		= 4,
		kTPDeviceGetDeviceDescriptor				= 5,
		kTPDeviceGetConfigDescriptor				= 6,
		kTPDeviceSetConfiguration					= 7,
		kTPDeviceSetFeature							= 8,
		kTPDeviceDeviceRequest						= 9,
		kTPDeviceGetConfiguration					= 10,
		kTPDeviceGetDeviceStatus					= 11,
		kTPDeviceSuspendDevice						= 12,
		kTPDeviceReEnumerateDevice					= 13,
		kTPDeviceConfigLock							= 14
	};
	
	// USB Pipe Tracepoints			
	// kUSBTPipe
	enum
	{
		kTPPipeInitToEndpoint						= 1,
		kTPBulkPipeRead								= 2,
		kTPBulkPipeWrite							= 3,
		kTPIsocPipeRead								= 4,
		kTPIsocPipeWrite							= 5,
		kTPIsocPipeReadLL							= 6,
		kTPIsocPipeWriteLL							= 7,
		kTPIBulkReadTS								= 8,
		kTPPipeControlRequest						= 9,
		kTPPipeControlRequestMemDesc				= 10,
        kTPPipeClearPipeStall                   = 11
	};
	
	
	// USB kUSBTControllerUserClient Tracepoints			
	// kUSBTControllerUserClient
	enum
	{
		kTPControllerUCStart					= 1,
		kTPControllerUCOpen						= 2,
		kTPControllerUCReadRegister				= 3,
		kTPControllerUCWriteRegister			= 4
	};
	
	// USB kUSBTDeviceUserClient Tracepoints			
	// kUSBTDeviceUserClient
	enum
	{
		kTPDeviceUCDeviceRequestIn				= 1,
		kTPDeviceUCDeviceRequestOut				= 2,
		kTPDeviceUCChangeOutstandingIO			= 3,
		kTPDeviceUCGetGatedOutstandingIO		= 4,
		kTPDeviceUCReqComplete					= 5
	};
	
	// USB InterfaceUserClient Tracepoints			
	// kUSBTInterfaceUserClient
	enum
	{
		kTPInterfaceUCReadPipe					= 1,
		kTPInterfaceUCWritePipe					= 2,
		kTPInterfaceUCControlRequestOut			= 3,
		kTPInterfaceUCControlRequestIn			= 4,
		kTPInterfaceUCDoIsochPipeAsync			= 5,
		kTPInterfaceUCLowLatencyPrepareBuffer	= 6,
		kTPInterfaceUCChangeOutstandingIO		= 7,
		kTPInterfaceUCReqComplete				= 8,
		kTPInterfaceUCIsoReqComplete			= 9,
		kTPInterfaceUCLLIsoReqComplete			= 10,
		kTPInterfaceUCOustandingTxct			= 11,
        kTPInterfaceUCReadIsochPipe             = 12,
        kTPInterfaceUCWriteIsochPipe            = 13,
        kTPInterfaceUCReadLLIsochPipe           = 14,
        kTPInterfaceUCWriteLLIsochPipe          = 15,
	};
	
	
	// USB HSHubUserClient Tracepoints			
	// kUSBTHSHubUserClient
	enum
	{
		kTPHSHubUCInitWithTask					= 1,
		kTPHSHubUCStart							= 2,
		kTPHSHubUCClose							= 3,
		kTPHSHubUCGetNumberOfPorts				= 4,
		kTPHSHubUCGetLocationID					= 5,
		kTPHSHubUCPutPortIntoTestMode			= 6,
		kTPHSHubUCSupportsIndicators			= 7,
		kTPHSHubUCSetIndicatorForPort			= 8,
		kTPHSHubUCGetPortIndicatorControl		= 9,
		kTPHSHubUCSetIndicatorsToAutomatic		= 10,
		kTPHSHubUCGetPowerSwitchingMode			= 11,
		kTPHSHubUCGetPortPower					= 12,
		kTPHSHubUCSetPortPower					= 13,
		kTPHSHubUCDisablePwrMgmt				= 14,
		kTPHSHubUCSetPortReset					= 15,
		kTPHSHubUCSetPortWarmReset				= 16,
		kTPHSHubUCSetPortLinkState				= 17,
		kTPHSHubUCSetPortU1Timeout				= 18,
		kTPHSHubUCSetPortU2Timeout				= 19,
		kTPHSHubUCSetPortRemoteWakeMask			= 20
	};
	
	// USB Hub Tracepoints			
	// kUSBTHub
	enum
	{
		kTPHubStart								= 1,
		kTPHubMessage							= 2,
		kTPHubWillTerminate						= 3,
		kTPHubPowerStateWillChangeTo			= 4,
		kTPHubPowerChangeDone					= 5,
		kTPHubConfigureHub						= 6,
		kTPHubCheckPowerPowerRequirements		= 7,
		kTPHubHubPowerChange					= 8,
		kTPHubAreAllPortsDisconnectedOrSuspended= 9,
		kTPHubSuspendPorts						= 10,
		kTPHubSetPortFeature					= 11,
		kTPHubClearPortFeature					= 12,
		kTPHubDoPortAction						= 13,
		kTPHubInterruptReadHandler				= 14,
		kTPHubResetPortZero						= 15,
		kTPHubProcessStateChanged				= 16,
		kTPHubRearmInterruptRead				= 17,
		kTPHubDoDeviceRequest					= 18,
		kTPHubWaitForPortResumes				= 19,
		kTPHubResetMyPort						= 20,
		kTPHubDecrementOutstandingIO			= 21,
		kTPHubChangeOutstandingIO				= 22, 
		kTPHubChangeRaisedPowerState			= 23,
		kTPHubChangeOutstandingResumes			= 24,
		kTPHubEnterTestMode						= 25,
		kTPHubLeaveTestMode						= 26,
		kTPHubPutPortIntoTestMode				= 27,
		kTPHubGetPortIndicatorControl			= 28,
		kTPHubSetIndicatorsToAutomatic			= 29,
		kTPHubGetPortPower						= 30,
		kTPHubEnsureUsability					= 31,
		kTPHubCheckPowerRequirements			= 32,
		kTPHubWaitForPowerOn					= 33,
		kTPHubDoPortActionLock					= 34,
		kTPHubCheckForDeadDevice				= 35,
		kTPHubPortDeviceDisconnected			= 36,
		kTPHubGetPortStatus						= 37,
		kTPHubGetPortStatusErrors				= 38,
        kTPHubTimer                             = 39,
		kTPHubRootHubHibernationWake			= 40
		
	};
	
	// USB HubPort Tracepoints			
	// kUSBTHubPort
	enum
	{
		kTPHubPortStop								= 1,
		kTPHubPortAddDevice							= 2,
		kTPHubPortSuspendPort						= 3,
		kTPHubPortFatalError						= 4,
		kTPHubPortAddDeviceResetChangeHandler		= 5,
		kTPHubPortHandleResetPortHandler			= 6,
		kTPHubPortDefaultOverCrntChangeHandler		= 7,
		kTPHubPortDefaultConnectionChangeHandler	= 8,
		kTPHubPortReleaseDevZeroLock				= 9,
		kTPHubPortDetachDevice						= 10,
		kTPHubPortGetDevZeroDescriptorWithRetries	= 11,
		kTPHubPortDisplayOverCurrentNotice			= 12,
		kTPHubPortWakeSuspendCommand				= 13,
		kTPHubPortWaitForSuspendCommand				= 14,
		kTPHubPortEnablePowerAfterOvercurrent		= 15,
		kTPHubPortRetrySSDevice						= 16,
		kTPHubPortRemoveDevice						= 17,
        kTPHubPortSDCardWorkarounds                 = 18,
	};
	
	// USB HID Tracepoints			
	// kUSBTHID
	enum
	{
		kTPHIDStart							= 1,
		kTPHIDpowerStateWillChangeTo		= 2,
		kTPHIDsetPowerState					= 3,
		kTPHIDpowerStateDidChangeTo			= 4,
		kTPHIDpowerChangeDone				= 5,
		kTPHIDhandleStart					= 6,
		kTPHIDClearFeatureEndpointHalt		= 7,
		kTPHIDHandleReport					= 8,
		kTPHIDSuspendPort					= 9,
		kTPHIDAbortAndSuspend				= 10,
		kTPHIDClaimPendingRead				= 11,
		kTPHIDChangeOutstandingIO			= 12,
		kTPHIDRearmInterruptRead			= 13,
		kTPHIDInitializeUSBHIDPowerManagement = 14,
		kTPHIDInterruptRead					= 15,
		kTPHIDInterruptReadError			= 16,
		kTPHIDCheckForDeadDevice			= 17,
        kTPHIDInterruptPipeRead             = 18
	};
	
	// USB Enumeration Tracepoints			
	// kUSBTEnumeration
	enum
	{
		kTPEnumerationProcessStatusChanged	= 1,
		kTPEnumerationInitialGetPortStatus	= 2,
		kTPEnumerationCallAddDevice			= 3,
		kTPEnumerationAddDevice				= 4,
		kTPEnumerationResetPort				= 5,
		kTPEnumerationAddDeviceResetChangeHandler	= 6,
		kTPEnumerationRegisterService		= 7,
		kTPEnumerationLowSpeedDevice		= 8,
		kTPEnumerationFullSpeedDevice		= 9
		
	};
	
	// USB UHCI Tracepoints			
	// kUSBTUHCI
	enum
	{
		kTPUHCIMessage						= 1,
		kTPUHCIGetFrameNumber				= 2,
		kTPUHCIScavengeIsocTransactions		= 3,
		kTPUHCIScavengeQueueHeads			= 4,
		kTPUHCIAllocateQH					= 5,
		kTPUHCIRootHubStatusChange			= 6,
		kTPUHCIRHSuspendPort				= 7,
		kTPUHCIRHHoldPortReset				= 8,
		kTPUHCIRHResumePortCompletion		= 9,
		kTPUHCISuspendController			= 10,
		KTPUHCIResumeController				= 11,
		KTPUHCIResetControllerState			= 12,
		KTPUHCIRestartControllerFromReset	= 13,
		KTPUHCIEnableInterrupts				= 14,
		KTPUHCIDozeController				= 15,
		KTPUHCIWakeFromDoze					= 16,
		KTPUHCIPowerState					= 17,
	};

	
	// USB UHCI Tracepoints			
	// kUSBTUHCIUIM
	enum
	{
		kTPUHCIUIMCreateControlEndpoint		= 1,
		kTPUHCIUIMCreateBulkEndpoint		= 2,
		kTPUHCIUIMCreateInterruptEndpoint	= 3,
		kTPUHCIUIMCreateIsochEndpoint		= 4,
		kTPUHCIUIMCreateIsochTransfer		= 5,
		kTPUHCIUIMDeleteEndpoint			= 6,
		kTPUHCIUIMUnlinkQueueHead			= 7,
		kTPUHCIUIMCheckForTimeouts			= 8,
		kTPUHCIUIMReturnOneTransaction		= 9,
		kTPUHCIUIMAllocateTDChain			= 10,
		kTPUHCIUIMAddIsochFramesToSchedule	= 11,
		kTPUHCIUIMAbortIsochEP				= 12,
		kTPUHCIUIMCreateInterruptTransfer	= 13,
		kTPUHCIUIMHandleEndpointAbort		= 14
	};
	
	// USB UHCI Interrupt Tracepoints			
	// kUSBTUHCIInterrupts
	enum
	{
		kTPUHCIInterruptsGetFrameNumberInternal	= 1,
		kTPUHCIInterruptsFilterInterrupt		= 2,
		kTPUHCIInterruptsHandleInterrupt		= 3,
		kTPUHCIUpdateFrameList					= 4
	};
	
	// USB EHCI Tracepoints			
	// kUSBTEHCI
	enum
	{
		kTPEHCIRootHubResetPort					= 1,
		kTPEHCIRootHubPortEnable				= 2,
		kTPEHCIRootHubPortSuspend				= 3,
		// kTPEHCIRootHubStatusChange			= 4,
		kTPEHCIRHResumePortCompletion			= 5,
		kTPEHCIUIMFinalize						= 6,
		kTPEHCIGetFrameNumber32					= 7, 
		kTPEHCIGetFrameNumber					= 8,
		kTPEHCIAllocateQH						= 9,
		kTPEHCIEnableAsyncSchedule				= 10,
		kTPEHCIDisableAsyncSchedule				= 11,
		kTPEHCIEnablePeriodicSchedule			= 12,
		kTPEHCIDisablePeriodicSchedule			= 13,
		kTPEHCIMessage							= 14,
		kTPEHCIMakeEmptyEndPoint				= 15,
		kTPEHCIAllocateTDs						= 16,
		kTPEHCIMungeEHCIStatus					= 17,
		kTPEHCIScavengeIsocTransactions			= 18,
		kTPEHCIScavengeAnEndpointQueue			= 19,
		kTPEHCIScavengeCompletedTransactions	= 20,
		kTPEHCICreateBulkEndpoint				= 21,
		kTPEHCICreateBulkTransfer				= 22,

		kTPEHCICreateInterruptEndpoint			= 24,
		kTPEHCICreateIsochEndpoint				= 25,
		kTPEHCIAbortIsochEP						= 26,
		kTPEHCIHandleEndpointAbort				= 27,
		kTPEHCICreateInterruptTransfer			= 28,
		kTPEHCIUnlinkAsyncEndpoint				= 29,
		kTPEHCIDeleteEndpoint					= 30,
		kTPEHCICreateHSIsochTransfer			= 31,
		kTPEHCICreateSplitIsochTransfer			= 32,
		kTPEHCICreateIsochTransfer				= 33,
		kTPEHCIAddIsocFramesToSchedule			= 34,
		kTPEHCIReturnOneTransaction				= 35,
		kTPEHCICheckEDListForTimeouts			= 36,
		kTPEHCISuspendUSBBus					= 37,
		kTPEHCICheckForTimeouts					= 38,
		kTPEHCIResumeUSBBus						= 39,
		kTPEHCIRestartUSBBus					= 40,
		kTPEHCIResetControllerState				= 41,
		kTPEHCIDozeController					= 42,
		kTPEHCIWakeControllerFromDoze			= 43,
		kTPEHCIEnableInterrupts					= 44,
		kTPEHCIPowerState						= 45,
		kTPEHCIStopUSBBus						= 46,
		kTPEHCIRestartControllerFromReset		= 47,
		kTPEHCIDemarcation						= 48,
        kTPEHCIreturnTransactions               = 49
	};

	// USB EHCI Interrupt Tracepoints			
	// kUSBTEHCIInterrupts
	enum
	{
		kTPEHCIInterruptsPollInterrupts			= 1,
		kTPEHCIInterruptsPrimaryInterruptFilter	= 2,
		kTPEHCIUpdateFrameList					= 3,
		kTPEHCIUpdateFrameListBits				= 4
	};
	
	
	// USB EHCI Interrupt Tracepoints			
	// kUSBTEHCIHubInfo
	enum
	{
		kTPEHCIAvailableIsochBandwidth			= 1,
		kTPEHCIAllocateIsochBandwidth			= 2
	};
	
	// USB OHCI Tracepoints			
	// kUSBTOHCI
	enum
	{
		kTPOHCIInitialize						= 1,
		kTPOHCICreateGeneralTransfer			= 2,
		kTPOHCIAbortEndpoint					= 3,
		kTPOHCIDeleteEndpoint					= 4,
		kTPOHCIEndpointStall					= 5,
		kTPOHCICreateIsochTransfer				= 6,
		kTPOHCIAllocateTD						= 7,
		kTPOHCIAllocateITD						= 8,
		kTPOHCIAllocateED						= 9,
		kTPOHCIProcessCompletedITD				= 10,
		kTPOHCIReturnTransactions				= 11,
		kTPOHCIMessage							= 12,
		KTPOHCISuspendUSBBus					= 13,
		KTPOHCIResumeUSBBus						= 14,
		KTPOHCIResetControllerState				= 15,
		KTPOHCIRestartControllerFromReset		= 16,
		KTPOHCIEnableInterrupts					= 17,
		KTPOHCIDozeController					= 18,
		KTPOHCIWakeControllerFromDoze			= 19,
		KTPOHCIPowerState						= 20,
		kTPOHCIDoneQueueCompletion				= 21,
		kTPOHCIDemarcation						= 22
	};
		
	// USB OHCI Interrupt Tracepoints			
	// kUSBTOHCIInterrupts
	enum
	{
		kTPOHCIInterruptsPollInterrupts			= 1,
		kTPOHCIInterruptsPrimaryInterruptFilter	= 2,
		kTPOHCIUpdateFrameList					= 3
	};
	
	// USB OHCI DumpQs Tracepoints			
	// kUSBTOHCIDumpQueues
	enum
	{
		kTPOHCIDumpQED1							= 1,
		kTPOHCIDumpQED2							= 2,
		kTPOHCIDumpQED3							= 3,
		kTPOHCIDumpQTD1							= 4,
		kTPOHCIDumpQTD2							= 5,
		kTPOHCIDumpQTD3							= 6,
		kTPOHCIDumpQTD4							= 7,
		kTPOHCIDumpQTD5							= 8,
		kTPOHCIDumpQTD6							= 9
	};
	//
	
	// USB EHCI DumpQs Tracepoints			
	// kUSBTEHCIDumpQueues
	enum
	{
		kTPEHCIDumpQH1							= 1,
		kTPEHCIDumpQH2							= 2,
		kTPEHCIDumpQH3							= 3,
		kTPEHCIDumpQH4							= 4,
		kTPEHCIDumpQH5							= 5,
		kTPEHCIDumpQH6							= 6,
		kTPEHCIDumpQH7							= 7,
		kTPEHCIDumpQH8							= 8,
		kTPEHCIDumpQH9							= 9,
		kTPEHCIDumpQH10							= 10,
		kTPEHCIDumpQH11							= 11,
		// reserve some room
		kTPEHCIDumpTD1							= 15,
		kTPEHCIDumpTD2							= 16,
		kTPEHCIDumpTD3							= 17,
		kTPEHCIDumpTD4							= 18,
		kTPEHCIDumpTD5							= 19,
		kTPEHCIDumpTD6							= 20,
		kTPEHCIDumpTD7							= 21,
		kTPEHCIDumpTD8							= 22
	};
	//
	
	// USB XHCI Tracepoints	(max of 256)	
	// kUSBTXHCI
	enum
	{
		kTPXHCIUIMInitialize					= 2,
		kTPXHCIWaitForCmd						= 3,
		kTPXHCIUIMFinalize						= 4,
		kTPXHCIUIMCreateTransfer				= 5,
		kTPXHCIUIMClearEndpointStall			= 6,
        kTPXHCIPrintContext                     = 7,
        kTPXHCIClearEndpoint                    = 8,
        kTPXHCIStartEndpoint                    = 9,
        kTPXHCIStopEndpoint                     = 10,
        kTPXHCIReturnATransfer                  = 11,
        kTPXHCIReturnAllTransfers               = 12,
        kTPXHCIReInitTransferRing               = 13,
        kTPXHCIFilterEventRing                  = 14,
        kTPXHCIPollEventRing2                   = 15,
		kTPXHCIPollForCMDCompletion				= 16,
        kTPXHCICheckEPForTimeOuts               = 17,
        kTPXHCICheckForTimeouts                 = 18,
        kTPXHCIResetEndpoint                    = 19,
		kTPXHCIUIMDeviceToBeReset				= 20,
		kTPXHCIUIMGetDeviceActualAddress		= 21,
		kTPXHCIGetFrameNumber					= 22,
        kTPXHCIQuiesceEndpoint                  = 23,
        kTPXHCIAbortIsochEP                     = 24,
        kTPXHCIUIMAbortStream					= 25,
        
        // 26-29 for isoch
        kTPXHCIAddIsochFramesToSchedule         = 26,
        kTPXHCIScavengeIsocTransactions         = 27,
		kTPXHCIUIMCreateIsocEndpoint			= 28,
		kTPXHCIDeleteIsochEP					= 29,

        // 30-39 for async queue
        kTPXHCIAsyncEPCreateTD                  = 30,
        kTPXHCIAsyncEPScheduleTD                = 31,
        kTPXHCIAsyncEPScavengeTD                = 32,
        kTPXHCIAsyncEPUpdateTimeout             = 33,
        kTPXHCIAsyncEPAbort                     = 34,
        kTPXHCIAsyncEPComplete                  = 35,
        kTPXHCIAsyncEPAlloc                     = 36,
        kTPXHCIAsyncEPFree                      = 37,
		kTPXHCIAsyncFlushTDsWithStatus			= 38,
		
        // 40-49 for register access
        kTPXHCIRead8Reg                         = 40,
        kTPXHCIRead16Reg                        = 41,
        kTPXHCIRead32Reg                        = 42,
        kTPXHCIRead64Reg                        = 43,
        kTPXHCIWrite32Reg                       = 44,
        kTPXHCIWrite64Reg                       = 45,
        
        // 50-59 error logging
        kTPXHCISoftRetry                        = 50,

       
        // 60-80 for UIM Calls
        kTPXHCIUIMHubMaintenance                = 60,
        kTPXHCIUIMCreateControlEndpoint         = 61,
        kTPXHCIUIMCreateControlTransfer         = 62,
        kTPXHCIUIMCreateStreams                 = 63,
        kTPXHCICreateBulkEndpoint               = 64,
        kTPXHCIUIMCreateBulkTransfer            = 65,
        kTPXHCICreateInterruptEndpoint          = 66,
        kTPXHCIUIMCreateInterruptTransfer       = 67,
        kTPXHCIUIMCreateIsochTransfer           = 68,
        kTPXHCIUIMDeleteEndpoint                = 69,
        kTPXHCIUIMEnableAddressEndpoints        = 70,
        kTPXHCIUIMSetTestMode                   = 71,
        
        // 81+ misc
        kTPXHCIMuxTransitionPoints              = 81,
        kTPXHCIUIMRootHubStatusChange           = 82,
        kTPXHCIGetRootHubPortStatus             = 83,
        kTPXHCISetAddress                       = 84,
        kTPXHCIConfiguredEndpointCount          = 85,
        kTPXHCIGetEndpointID                    = 86,
        kTPXHCIGetSlotID                        = 87,
        kTPXHCIMungeIsochStatus                 = 88,
        
        // 100 XHCI Power Management
        kTPXHCIRestartUSBBus                    = 100,
        kTPXHCImaxCapabilityForDomainState      = 101
   };
	
	// USB XHCI Interrupt Tracepoints			
	// kUSBTXHCIInterrupts
	enum
	{
		kTPXHCIInterruptsPollInterrupts			= 1,
		kTPXHCIInterruptsPrimaryInterruptFilter	= 2,
	};

    // kUSBTXHCIRootHubs
	enum
	{
		kTPXHCIRootHubResetPort					= 1,
		kTPXHCIRootHubPortEnable				= 2,
		kTPXHCIRootHubPortSuspend				= 3,
		kTPXHCIRootHubResetResetChange			= 4,
		kTPXHCIRootHubResetPortCallout			= 5
	};
	
	// kUSBTXHCIPrintTRB
	enum
	{
		kTPXHCIPrintTRBTransfer					= 1,
		kTPXHCIPrintTRBEvent					= 2,
		kTPXHCIPrintTRBCommand					= 3
	};
    
	// USB HubPolicyMaker Tracepoints			
	// kUSBTHubPolicyMaker
	enum
	{
		kTPSetPowerState						 = 1
	};
	
	// USB CompositeDriver Tracepoints			
	// kUSBTCompositeDriver
	enum
	{
		kTPCompositeDriverConfigureDevice		 = 1
	};

	// USB USB OutstandingIO Tracepoints			
	// kUSBTOutstandingIO
	enum
	{
		kTPDeviceUCDecrement	= 1,
		kTPDeviceUCIncrement	= 2,
		kTPInterfaceUCDecrement	= 3,
		kTPInterfaceUCIncrement	= 4,
		kTPHubDecrement			= 5,
		kTPHubIncrement			= 6,
		kTPHIDDecrement			= 7,
		kTPHIDIncrement			= 8
	};
	
	// USB Audio driver Tracepoints			
	// kUSBAudio
	enum
	{
		kTPAudioDriverRead						= 1,
		kTPAudioDriverCoalesceInputSamples		= 2,
		kTPAudioDriverCoalesceError				= 3,
		kTPAudioDriverCoalesce					= 4,
		kTPAudioDriverReadHandler				= 5,
		kTPAudioDriverCoalesceError2			= 6,
		kTPAudioDriverConvertInputSamples		= 7
	};
	
	// Tracepoint macros.
#define USB_TRACE(USBClass, code, funcQualifier)	( ( ( DBG_IOKIT & 0xFF ) << 24) | ( ( DBG_IOUSB & 0xFF ) << 16 ) | ( ( USBClass & 0x3F ) << 10 ) | ( ( code & 0xFF ) << 2 ) | ( funcQualifier & 0x3 ) )

#define kTPAllUSB								USB_TRACE ( 0, 0, 0 )

#define USB_CONTROLLER_TRACE(code)				USB_TRACE( kUSBTController, code, DBG_FUNC_NONE )
#define USB_CONTROLLER_UC_TRACE(code)			USB_TRACE( kUSBTControllerUserClient, code, DBG_FUNC_NONE )
#define USB_DEVICE_TRACE(code)					USB_TRACE( kUSBTDevice, code, DBG_FUNC_NONE )
#define USB_DEVICE_UC_TRACE(code)				USB_TRACE( kUSBTDeviceUserClient, code, DBG_FUNC_NONE )

#define USB_HUB_TRACE(code)						USB_TRACE( kUSBTHub, code, DBG_FUNC_NONE )
#define USB_HUB_PORT_TRACE(code)				USB_TRACE( kUSBTHubPort, code, DBG_FUNC_NONE )
#define USB_HUB_UC_TRACE(code)					USB_TRACE( kUSBTHSHubUserClient, code, DBG_FUNC_NONE )
#define USB_HID_TRACE(code)						USB_TRACE( kUSBTHID, code, DBG_FUNC_NONE )
#define USB_PIPE_TRACE(code)					USB_TRACE( kUSBTPipe, code, DBG_FUNC_NONE )

#define USB_INTERFACE_UC_TRACE(code)			USB_TRACE( kUSBTInterfaceUserClient, code, DBG_FUNC_NONE )

#define USB_ENUMERATION_TRACE(code)				USB_TRACE( kUSBTEnumeration, code, DBG_FUNC_NONE )

#define USB_UHCI_TRACE(code)					USB_TRACE( kUSBTUHCI, code, DBG_FUNC_NONE )
#define USB_UHCI_UIM_TRACE(code)				USB_TRACE( kUSBTUHCIUIM, code, DBG_FUNC_NONE )
#define USB_UHCI_INTERRUPTS_TRACE(code)			USB_TRACE( kUSBTUHCIInterrupts, code, DBG_FUNC_NONE )

#define USB_OHCI_TRACE(code)					USB_TRACE( kUSBTOHCI, code, DBG_FUNC_NONE )
#define USB_OHCI_INTERRUPTS_TRACE(code)			USB_TRACE( kUSBTOHCIInterrupts, code, DBG_FUNC_NONE )
#define USB_OHCI_DUMPQS_TRACE(code)				USB_TRACE( kUSBTOHCIDumpQueues, code, DBG_FUNC_NONE )

#define USB_EHCI_TRACE(code)					USB_TRACE( kUSBTEHCI, code, DBG_FUNC_NONE )
#define USB_EHCI_HUBINFO_TRACE(code)			USB_TRACE( kUSBTEHCIHubInfo, code, DBG_FUNC_NONE )
#define USB_EHCI_INTERRUPTS_TRACE(code)			USB_TRACE( kUSBTEHCIInterrupts, code, DBG_FUNC_NONE )
#define USB_EHCI_DUMPQS_TRACE(code)				USB_TRACE( kUSBTEHCIDumpQueues, code, DBG_FUNC_NONE )

#define USB_XHCI_TRACE(code)					USB_TRACE( kUSBTXHCI, code, DBG_FUNC_NONE )
#define USB_XHCI_INTERRUPTS_TRACE(code)			USB_TRACE( kUSBTXHCIInterrupts, code, DBG_FUNC_NONE )
#define USB_XHCI_ROOTHUBS_TRACE(code)			USB_TRACE( kUSBTXHCIRootHubs, code, DBG_FUNC_NONE )
#define USB_XHCI_PRINTTRB_TRACE(code)			USB_TRACE( kUSBTXHCIPrintTRB, code, DBG_FUNC_NONE )

#define USB_HUB_POLICYMAKER_TRACE(code)			USB_TRACE( kUSBTHubPolicyMaker, code, DBG_FUNC_NONE )
#define USB_COMPOSITE_DRIVER_TRACE(code)		USB_TRACE( kUSBTCompositeDriver, code, DBG_FUNC_NONE )

#define USB_OUTSTANDING_IO_TRACE(code)			USB_TRACE( kUSBTOutstandingIO, code, DBG_FUNC_NONE )	
	
#define USB_AUDIO_DRIVER_TRACE(code)			USB_TRACE( kUSBAudio, code, DBG_FUNC_NONE )	

#ifdef KERNEL
	
#include <IOKit/IOTimeStamp.h>
		
#define USBTrace(USBClass, code, a, b, c, d) {														\
			if (gUSBStackDebugFlags & kUSBEnableTracePointsMask) {									\
				IOTimeStampConstant( USB_TRACE(USBClass, code, DBG_FUNC_NONE), a, b, c, d );		\
			}																						\
		}

#define USBTrace_Start(USBClass, code, a, b, c, d) {												\
			if (gUSBStackDebugFlags & kUSBEnableTracePointsMask) {									\
				IOTimeStampConstant( USB_TRACE(USBClass, code, DBG_FUNC_START), a, b, c, d );		\
			}																						\
		}

#define USBTrace_End(USBClass, code, a, b, c, d) {														\
			if (gUSBStackDebugFlags & kUSBEnableTracePointsMask) {									\
				IOTimeStampConstant( USB_TRACE(USBClass, code, DBG_FUNC_END), a, b, c, d );			\
			}																						\
		}

#endif
	
	
	
#ifdef __cplusplus
}
#endif


#endif
