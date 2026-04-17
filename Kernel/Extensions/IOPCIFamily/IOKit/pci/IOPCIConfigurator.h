/*
 * Copyright (c) 2004-2005 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 2.0 (the
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

#ifndef _IOPCICONFIGURATOR_H
#define _IOPCICONFIGURATOR_H

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef uint64_t IOPCIScalar;

enum {
    kIOPCIRangeFlagMaximizeSize  = 0x00000001,
    kIOPCIRangeFlagNoCollapse    = 0x00000002,
    kIOPCIRangeFlagMaximizeRoot  = 0x00000004,
    kIOPCIRangeFlagSplay         = 0x00000008,
    kIOPCIRangeFlagRelocatable   = 0x00000010,
    kIOPCIRangeFlagReserve       = 0x00000020,
    kIOPCIRangeFlagPermanent     = 0x00000040,
    kIOPCIRangeFlagBar64         = 0x00000080,
};

struct IOPCIRange
{
    IOPCIScalar         start;
    IOPCIScalar         size;
    IOPCIScalar         totalSize;
    IOPCIScalar         extendSize;
    IOPCIScalar         proposedSize;

    // end marker
    IOPCIScalar         end;
    IOPCIScalar         zero;

    IOPCIScalar         alignment;
    IOPCIScalar         minAddress;
    IOPCIScalar         maxAddress;

    uint8_t             type;
    uint8_t             resvB[3];
    uint32_t            flags;
    struct IOPCIRange * next;
    struct IOPCIRange * nextSubRange;
    struct IOPCIRange * allocations;

    struct IOPCIRange *  nextToAllocate;
    struct IOPCIConfigEntry * device; 			// debug
};

IOPCIScalar IOPCIScalarAlign(IOPCIScalar num, IOPCIScalar alignment);
IOPCIScalar IOPCIScalarTrunc(IOPCIScalar num, IOPCIScalar alignment);

IOPCIRange * IOPCIRangeAlloc(void);

void IOPCIRangeFree(IOPCIRange * range);

void IOPCIRangeInit(IOPCIRange * range, uint32_t type,
                  IOPCIScalar start, IOPCIScalar size, IOPCIScalar alignment = 0);
void IOPCIRangeInitAlloc(IOPCIRange * range, uint32_t type,
                         IOPCIScalar start, IOPCIScalar size, IOPCIScalar alignment = 0);

void IOPCIRangeDump(IOPCIRange * head);

bool IOPCIRangeListAddRange(IOPCIRange ** rangeList,
                          uint32_t type,
                          IOPCIScalar start,
                          IOPCIScalar size,
                          IOPCIScalar alignment = 1);

bool IOPCIRangeDeallocateSubRange(IOPCIRange * headRange,
                                IOPCIRange * oldRange);

bool IOPCIRangeListAllocateSubRange(IOPCIRange * headRange,
                                  IOPCIRange * newRange,
                                  IOPCIScalar  newStart = 0);

bool IOPCIRangeListDeallocateSubRange(IOPCIRange * headRange,
                                IOPCIRange * oldRange);

bool IOPCIRangeAppendSubRange(IOPCIRange ** headRange,
                              IOPCIRange * newRange );

IOPCIScalar IOPCIRangeListCollapse(IOPCIRange * headRange, IOPCIScalar alignment);

IOPCIScalar IOPCIRangeCollapse(IOPCIRange * headRange, IOPCIScalar alignment);

void        IOPCIRangeOptimize(IOPCIRange * headRange);

IOPCIScalar IOPCIRangeListLastFree(IOPCIRange * headRange, IOPCIScalar align);
IOPCIScalar IOPCIRangeLastFree(IOPCIRange * headRange, IOPCIScalar align);
void        IOPCIRangeListOptimize(IOPCIRange * headRange);
IOPCIScalar IOPCIRangeListSize(IOPCIRange * first);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef KERNEL

#include <IOKit/IOLib.h>
#include <IOKit/pci/IOPCIBridge.h>


#define kPCIBridgeIOAlignment        (4096)
#define kPCIBridgeMemoryAlignment    (1024*1024)
#define kPCIBridgeBusNumberAlignment (1)


#define FOREACH_CHILD(bridge, child) \
    for(IOPCIConfigEntry * (child) = (bridge)->child; (child); (child) = (child)->peer)

enum {
    kIOPCIConfiguratorIOLog          = 0x00000001,
    kIOPCIConfiguratorKPrintf        = 0x00000002,
    kIOPCIConfiguratorVTLog          = 0x00000004,
    
    kIOPCIConfiguratorAER            = 0x00000008,
    kIOPCIConfiguratorWakeToOff      = 0x00000010,
    kIOPCIConfiguratorDeviceMap      = 0x00000020,

    kIOPCIConfiguratorLogSaveRestore = 0x00000040,
    kIOPCIConfiguratorMapInterrupts  = 0x00000080,
	kIOPCIConfiguratorPanicOnFault   = 0x00000100, 
    kIOPCIConfiguratorNoL1           = 0x00000200,           // disable L1 on thunderbolt

    kIOPCIConfiguratorDeepIdle       = 0x00000400,
    kIOPCIConfiguratorNoTB           = 0x00000800,
    kIOPCIConfiguratorTBMSIEnable    = 0x00001000,

    kIOPCIConfiguratorPFM64          = 0x00002000,
    kIOPCIConfiguratorBoot	         = 0x00004000,
    kIOPCIConfiguratorIGIsMapped     = 0x00008000,
    kIOPCIConfiguratorFPBEnable      = 0x00010000,
//    kIOPCIConfiguratorAllocate       = 0x00020000,
	kIOPCIConfiguratorUsePause       = 0x00040000,

    kIOPCIConfiguratorCheckTunnel    = 0x00080000,
    kIOPCIConfiguratorNoTunnelDrv    = 0x00100000,
    kIOPCIConfiguratorNoTerminate    = 0x00200000,
    kIOPCIConfiguratorDeferHotPlug   = 0x00400000,
    kIOPCIConfiguratorNoACS          = 0x00800000,

    kIOPCIConfiguratorTBPanics       = 0x01000000,
    kIOPCIConfiguratorTBUSBCPanics   = 0x02000000,

    kIOPCIConfiguratorBootDefer      = kIOPCIConfiguratorDeferHotPlug | kIOPCIConfiguratorBoot,
};

enum {
    kIOPCIRangeBAR0               = 0,
    kIOPCIRangeBAR1               = 1,
    kIOPCIRangeBAR2               = 2,
    kIOPCIRangeBAR3               = 3,
    kIOPCIRangeBAR4               = 4,
    kIOPCIRangeBAR5               = 5,
    kIOPCIRangeExpansionROM       = 6,

    // order matches kIOPCIResourceType*
    kIOPCIRangeBridgeMemory       = 7,
    kIOPCIRangeBridgePFMemory     = 8,
    kIOPCIRangeBridgeIO           = 9,
    kIOPCIRangeBridgeBusNumber    = 10,

    kIOPCIRangeCount,

	kIOPCIRangeAllMask		  	  = (1 << kIOPCIRangeCount) - 1,
	kIOPCIRangeAllBarsMask		  = (1 << (kIOPCIRangeExpansionROM + 1)) - 1,
	kIOPCIRangeAllBridgeMask	  =  (1 << kIOPCIRangeBridgeMemory)
								   | (1 << kIOPCIRangeBridgePFMemory)
								   | (1 << kIOPCIRangeBridgeIO)
								   | (1 << kIOPCIRangeBridgeBusNumber),
	

};

enum {
//    kPCIDeviceStateResourceAssigned  = 0x00000001,
    kPCIDeviceStatePropertiesDone    = 0x00000002,
    kPCIDeviceStateTreeConnected     = 0x00000004,
    kPCIDeviceStateConfigurationDone = 0x00000008,
    kPCIDeviceStateScanned          = 0x00000010,
    kPCIDeviceStateAllocatedBus     = 0x00000020,
    kPCIDeviceStateTotalled         = 0x00000040,
    kPCIDeviceStateAllocated        = 0x00000080,
    kPCIDeviceStateChildChanged     = 0x00000100,
	kPCIDeviceStateChildAdded       = 0x00000200,
    kPCIDeviceStateNoLink           = 0x00000400,
    kPCIDeviceStateRangesProbed     = 0x00000800,

	kPCIDeviceStateConfigProtectShift = 15,
	kPCIDeviceStateConfigRProtect	= (VM_PROT_READ  << kPCIDeviceStateConfigProtectShift),
	kPCIDeviceStateConfigWProtect	= (VM_PROT_WRITE << kPCIDeviceStateConfigProtectShift),

    kPCIDeviceStateDead             = 0x80000000,
    kPCIDeviceStateEjected          = 0x40000000,
    kPCIDeviceStateToKill           = 0x20000000,
    kPCIDeviceStatePaused           = 0x10000000,
    kPCIDeviceStateRequestPause     = 0x08000000,
    kPCIDeviceStateHidden           = 0x04000000,

    kPCIDeviceStateDeadOrHidden     = kPCIDeviceStateDead | kPCIDeviceStateHidden
};

enum {
    kPCIHeaderType0 = 0,
    kPCIHeaderType1 = 1,
    kPCIHeaderType2 = 2
};

// value of supportsHotPlug
enum
{
    kPCIHPTypeMask              = 0xf0,

    kPCIHPRoot                  = 0x01,
    kPCIHPRootParent            = 0x02,

    kPCIStatic                  = 0x00,
    kPCIStaticTunnel            = kPCIStatic | 0x01,
    kPCIStaticShared            = kPCIStatic | 0x02,

    kPCILinkChange              = 0x10,

    kPCIHotPlug                 = 0x20,
    kPCIHotPlugRoot             = kPCIHotPlug | kPCIHPRoot,

    kPCIHotPlugTunnel           = 0x30,
    kPCIHotPlugTunnelRoot       = kPCIHotPlugTunnel | kPCIHPRoot,
    kPCIHotPlugTunnelRootParent = kPCIHotPlugTunnel | kPCIHPRootParent,
};

#define kPCIBridgeMaxCount  256

enum 
{
    kConfigOpAddHostBridge = 1,
    kConfigOpScan,
    kConfigOpRealloc,
    kConfigOpGetState,
    kConfigOpNeedsScan,
    kConfigOpEject,
    kConfigOpKill,
    kConfigOpTerminated,
    kConfigOpProtect,
    kConfigOpShadowed,
    kConfigOpPaused,
    kConfigOpUnpaused,
    kConfigOpTestPause,
    kConfigOpFindEntry,
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

struct IOPCIConfigEntry
{
    IOPCIConfigEntry *  parent;
    IOPCIConfigEntry *  child;
    IOPCIConfigEntry *  peer;
    uint32_t			id;
    uint32_t            classCode;
    IOPCIAddressSpace   space;
    uint32_t            vendorProduct;

    uint32_t            expressCapBlock;
    uint32_t            expressDeviceCaps1;

    uint32_t            fpbCapBlock;
    uint32_t            fpbCaps;

    IOPCIRange *        ranges[kIOPCIRangeCount];
    IOPCIRange          busResv;
    uint32_t            rangeBaseChanges;
    uint32_t            rangeSizeChanges;
    uint32_t            rangeRequestChanges;
    uint32_t            haveAllocs;

    uint32_t            deviceState;
    uint8_t             iterator;

    uint8_t             headerType;
    uint8_t             isBridge;
    uint8_t             countMaximize;
    uint8_t             isHostBridge;
    uint8_t             supportsHotPlug;
    uint8_t				linkInterrupts;
    uint8_t             clean64;

	// bridge only:
    uint8_t             secBusNum;
    uint8_t             subBusNum;
    uint8_t             subDeviceNum;
    uint8_t             endDeviceNum;
    uint8_t             fpbUp;
    uint8_t             fpbDown;
    //

    uint32_t			linkCaps;
    uint16_t			expressCaps;
    uint8_t   			expressMaxPayload;
    uint8_t   			expressPayloadSetting;
//	uint16_t            pausedCommand;

    IORegistryEntry *   dtEntry;
#if ACPI_SUPPORT
    IORegistryEntry *   acpiDevice;
#endif
    IORegistryEntry *   dtNub;

	uint8_t *			configShadow;
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

class IOPCIConfigurator : public IOService
{
    friend class IOPCIBridge;
    OSDeclareDefaultStructors( IOPCIConfigurator );

    IOWorkLoop *            fWL;
    IOOptionBits            fFlags;
    IOPCIBridge *           fHostBridge;
    IOPCIConfigEntry *      fRoot;
    uint64_t                fPFM64Size;
	uint32_t				fRootVendorProduct;

	uint8_t					fMaxPayload;

    IOPCIRange *            fConsoleRange;
    IOPCIScalar             fPFMConsole;

    OSSet *                 fChangedServices;
    uint32_t				fWaitingPause;

    uint32_t                fBridgeCount;
    uint32_t                fDeviceCount;
    uint32_t				fNextID;
#if ACPI_SUPPORT
	uint8_t				 	fAddedHost64;
#endif /* ACPI_SUPPORT */

protected:

    static void safeProbeCallback( void * refcon );
    static void configProbeCallback( void * refcon );

    static void matchDTEntry( IORegistryEntry * dtEntry, void * _context );
#if ACPI_SUPPORT
    static void matchACPIEntry( IORegistryEntry * dtEntry, void * _context );
	void        removeFixedRanges(IOPCIConfigEntry * bridge);
#endif

    typedef int32_t (IOPCIConfigurator::*IterateProc)(void * ref, IOPCIConfigEntry * bridge);
    void    iterate(const char * what, 
                    IterateProc topProc, IterateProc bottomProc, 
                    void * ref = NULL);

    int32_t scanProc(void * ref, IOPCIConfigEntry * bridge);
	int32_t bootResetProc(void * ref, IOPCIConfigEntry * bridge);
    int32_t totalProc(void * ref, IOPCIConfigEntry * bridge);
    int32_t allocateProc(void * ref, IOPCIConfigEntry * bridge);
	int32_t bridgeFinalizeConfigProc(void * unused, IOPCIConfigEntry * bridge);

    void    configure(uint32_t options);
    void    bridgeScanBus(IOPCIConfigEntry * bridge, uint8_t busNum);

    void    logAllocatorRange(IOPCIConfigEntry * device, IOPCIRange * range, char c);
    IOPCIRange * bridgeGetRange(IOPCIConfigEntry * bridge, uint32_t type);
    bool    bridgeTotalResources(IOPCIConfigEntry * bridge, uint32_t typeMask);
    int32_t bridgeAllocateResources( IOPCIConfigEntry * bridge, uint32_t typeMask );

	bool    bridgeDeallocateChildRanges(IOPCIConfigEntry * bridge, IOPCIConfigEntry * dead,
								        uint32_t deallocTypes, uint32_t freeTypes);

    void    doConfigure(uint32_t options);

    void    applyConfiguration(IOPCIConfigEntry * device, uint32_t typeMask, bool dolog);
    void    deviceApplyConfiguration(IOPCIConfigEntry * device, uint32_t typeMask, bool dolog);
    void    bridgeApplyConfiguration(IOPCIConfigEntry * bridge, uint32_t typeMask, bool dolog);
    uint16_t disableAccess(IOPCIConfigEntry * device, bool disable);
    void    restoreAccess(IOPCIConfigEntry * device, UInt16 command);
    void    bridgeAddChild(IOPCIConfigEntry * bridge, IOPCIConfigEntry * child);
    void    bridgeRemoveChild(IOPCIConfigEntry * bridge, IOPCIConfigEntry * dead,
								uint32_t deallocTypes, uint32_t freeTypes,
								IOPCIConfigEntry ** childList);
	void    bridgeMoveChildren(IOPCIConfigEntry * to, IOPCIConfigEntry * list);
    void    bridgeDeadChild(IOPCIConfigEntry * bridge, IOPCIConfigEntry * dead);
    void    bridgeProbeChild(IOPCIConfigEntry * bridge, IOPCIAddressSpace space);
    void    bridgeProbeChildRanges(IOPCIConfigEntry * bridge, uint32_t resetMask);
    void    probeBaseAddressRegister(IOPCIConfigEntry * device, uint32_t lastBarNum, uint32_t resetMask);
    void    safeProbeBaseAddressRegister(IOPCIConfigEntry * device, uint32_t lastBarNum, uint32_t resetMask);
    void    deviceProbeRanges(IOPCIConfigEntry * device, uint32_t resetMask);
    void    bridgeProbeRanges(IOPCIConfigEntry * bridge, uint32_t resetMask);
    void    cardbusProbeRanges(IOPCIConfigEntry * bridge, uint32_t resetMask);
    void    bridgeProbeBusRange(IOPCIConfigEntry * bridge, uint32_t resetMask);
	uint32_t findPCICapability(IOPCIConfigEntry * device,
                               uint32_t capabilityID, uint32_t * found);
    void    checkCacheLineSize(IOPCIConfigEntry * device);
    void    writeLatencyTimer(IOPCIConfigEntry * device);

	bool    treeInState(IOPCIConfigEntry * entry, uint32_t state, uint32_t mask);
    void    markChanged(IOPCIConfigEntry * entry);
    void    bridgeConnectDeviceTree(IOPCIConfigEntry * bridge);
    void    bridgeFinishProbe(IOPCIConfigEntry * bridge);
    bool    bridgeConstructDeviceTree(void * unused, IOPCIConfigEntry * bridge);
    OSDictionary * constructProperties(IOPCIConfigEntry * device);
    void           constructAddressingProperties(IOPCIConfigEntry * device, OSDictionary * propTable);

    bool     createRoot(void);
    IOReturn addHostBridge(IOPCIBridge * hostBridge);
    IOPCIConfigEntry * findEntry(IOPCIAddressSpace space);

	bool     configAccess(IOPCIConfigEntry * device, bool write);
    void     configAccess(IOPCIConfigEntry * device, uint32_t access, uint32_t offset, void * data);

    uint32_t configRead32( IOPCIAddressSpace space, uint32_t offset);
	void     configWrite32(IOPCIAddressSpace space, uint32_t offset, uint32_t data);
	uint32_t findPCICapability(IOPCIAddressSpace space,
                               uint32_t capabilityID, uint32_t * found);

    uint32_t configRead32( IOPCIConfigEntry * device, uint32_t offset);
    uint16_t configRead16( IOPCIConfigEntry * device, uint32_t offset);
    uint8_t  configRead8(  IOPCIConfigEntry * device, uint32_t offset);
    void     configWrite32(IOPCIConfigEntry * device, uint32_t offset, uint32_t data);
    void     configWrite16(IOPCIConfigEntry * device, uint32_t offset, uint16_t data);
    void     configWrite8( IOPCIConfigEntry * device, uint32_t offset, uint8_t  data);

public:
    bool init(IOWorkLoop * wl, uint32_t flags);
    virtual IOWorkLoop * getWorkLoop() const;
    virtual void     free(void);

    IOReturn configOp(IOService * device, uintptr_t op, void * result, void * arg2 = NULL);
};

#endif /* KERNEL */

#endif /* !_IOPCICONFIGURATOR_H */
