/*
 * Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
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


#ifndef _IOKIT_IOPCIPRIVATE_H
#define _IOKIT_IOPCIPRIVATE_H

#if defined(KERNEL)

#include <IOKit/pci/IOPCIDevice.h>
#include <IOKit/IORangeAllocator.h>
#include <IOKit/IOInterruptController.h>
#include <libkern/OSDebug.h>
#include <IOKit/IOUserClient.h>

#if !defined(__ppc__)
#define USE_IOPCICONFIGURATOR   1
#define USE_MSI                 1
#define USE_LEGACYINTS          1
#endif

#if defined(__i386__) || defined(__x86_64__)
#define ACPI_SUPPORT            1
#else
#define ACPI_SUPPORT            0
#endif

struct IOPCIDeviceExpansionData
{
    uint16_t powerCapability;
    uint8_t  pmSleepEnabled;     // T if a client has enabled PCI Power Management
    uint8_t  pmControlStatus;    // if >0 this device supports PCI Power Management
    uint16_t sleepControlBits;   // bits to set the control/status register to for sleep
    uint16_t pmLastWakeBits;     // bits read on wake

    uint16_t expressCapability;
    uint16_t expressCapabilities;
    uint16_t expressASPMDefault;
	uint16_t aspmCaps;
    uint16_t l1pmCapability;
    uint32_t l1pmCaps;

    uint16_t fpbCapability;

    uint16_t aerCapability;

    uint16_t            msiCapability;
    uint16_t            msiControl;
	uint16_t            msiPhysVectorCount;
	uint16_t            msiVectorCount;
    uint8_t             msiMode;
    uint8_t             msiEnable;
	uint64_t            msiTable;
	uint64_t            msiPBA;
	IOInterruptVector * msiVectors;

    uint16_t latencyToleranceCapability;
    uint16_t acsCapability;
    uint16_t acsCaps;

    uint8_t  headerType;
    uint8_t  rootPort;

    uint8_t  configProt;
    uint8_t  pmActive;
    uint8_t  pmeUpdate;
    uint8_t  updateWakeReason;
    uint8_t  pmWait;
    uint8_t  pmState;
	uint8_t  pciPMState;
    uint8_t  pauseFlags;
    uint8_t  needsProbe;
    uint8_t  dead;
    uint8_t  pmHibernated;

	IOLock * lock;
    struct IOPCIConfigEntry * configEntry;

	IOOptionBits sessionOptions;

	IOPCIDevice * ltrDevice;
	IOByteCount   ltrOffset;
	uint32_t      ltrReg1;
	uint8_t       ltrReg2;

    uint8_t       tunnelL1Allow;

#if ACPI_SUPPORT
	int8_t        psMethods[kIOPCIDevicePowerStateCount];
	int8_t        lastPSMethod;
#endif
};

enum
{
    kTunnelL1Disable = false,
    kTunnelL1Enable  = true,
    kTunnelL1NotSet  = 2
};

#define expressV2(device) ((15 & device->reserved->expressCapabilities) > 1)

enum
{
    kIOPCIConfigShadowRegs        = 32,
    kIOPCIConfigEPShadowRegs      = 16,
    kIOPCIConfigBridgeShadowRegs  = 32,

    kIOPCIConfigShadowSize        = kIOPCIConfigShadowRegs,

    kIOPCISaveRegsMask            = 0xFFFFFFFF
//                                  & ~(1 << (kIOPCIConfigVendorID >> 2))
};

struct IOPCIConfigSave
{
    uint32_t                 savedConfig[kIOPCIConfigShadowSize];

	// express save
	uint16_t				 savedDeviceControl;
	uint16_t				 savedLinkControl;
	uint16_t				 savedSlotControl;
	uint16_t				 savedDeviceControl2;
	uint16_t				 savedLinkControl2;
	uint16_t				 savedSlotControl2;

	// msi save
	uint32_t				 savedMSIAddress0;
	uint32_t				 savedMSIAddress1;
	uint32_t				 savedMSIData;
	uint16_t				 savedMSIControl;
	uint32_t				 savedMSIEnable;

	// l1pm save	
	uint32_t				 savedL1PM0;
	uint32_t				 savedL1PM1;
	
	// ltr save
	uint32_t				 savedLTR;

    // acs save
    uint16_t                 savedACS;

	// aer save
	uint32_t				 savedAERCapsControl; // 0x18
	uint32_t				 savedAERSeverity;    // 0x0C
	uint32_t				 savedAERUMask;       // 0x08
	uint32_t				 savedAERCMask;       // 0x14
	uint32_t				 savedAERRootCommand; // 0x2c

	// fpb save
	uint32_t				 savedFPBControl1;    // 0x08
	uint32_t				 savedFPBControl2;    // 0x0C
	uint32_t				 savedFPBRIDVector0;   // 0x20
};

struct IOPCIConfigShadow
{
    IOPCIConfigSave          configSave;
    uint32_t                 flags;
	uint8_t                  tunnelled;
	uint8_t                  hpType;
    queue_chain_t            link;
    queue_chain_t            linkFinish;
	queue_head_t             dependents;
	IOLock      *            dependentsLock;
	IOPCIDevice *			 tunnelRoot;
	IOPCIDevice *			 sharedRoot;
    IOPCIDevice *            device;
    IOPCI2PCIBridge *        bridge;
    OSObject *               tunnelID;
    IOPCIDeviceConfigHandler handler;
    void *                   handlerRef;
    uint64_t                 restoreCount;
    IOOptionBits             sharedRootASPMState;
};

#define configShadow(device)    ((IOPCIConfigShadow *) &device->savedConfig[0])


// flags in kIOPCIConfigShadowFlags
enum
{
    kIOPCIConfigShadowValid            = 0x00000001,
    kIOPCIConfigShadowBridge           = 0x00000002,
    kIOPCIConfigShadowHostBridge       = 0x00000004,
    kIOPCIConfigShadowBridgeDriver     = 0x00000008,
    kIOPCIConfigShadowBridgeInterrupts = 0x00000010,
	kIOPCIConfigShadowSleepLinkDisable = 0x00000020,
	kIOPCIConfigShadowSleepReset       = 0x00000040,
	kIOPCIConfigShadowHotplug          = 0x00000080,
	kIOPCIConfigShadowVolatile         = 0x00000100,
	kIOPCIConfigShadowWakeL1PMDisable  = 0x00000200,
};

// whatToDo for setDevicePowerState()
enum
{
    kSaveDeviceState    = 0,
    kRestoreDeviceState = 1,
    kSaveBridgeState    = 2,
    kRestoreBridgeState = 3
};

enum
{
	kMachineRestoreBridges      = 0x00000001,
    kMachineRestoreEarlyDevices = 0x00000002,
	kMachineRestoreDehibernate  = 0x00000004,
	kMachineRestoreTunnels      = 0x00000008,
};

#define PCI_ADDRESS_TUPLE(device)   \
        device->space.s.busNum,     \
        device->space.s.deviceNum,  \
        device->space.s.functionNum


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define kIOPCIEjectableKey        "IOPCIEjectable"
#define kIOPCIHotPlugKey          "IOPCIHotPlug"
#define kIOPCILinkChangeKey       "IOPCILinkChange"
#define kIOPCITunnelLinkChangeKey "IOPCITunnelLinkChange"
#define kIOPCITunnelBootDeferKey  "IOPCITunnelBootDefer"
#define kIOPCIResetKey            "IOPCIReset"
#define kIOPCIOnlineKey           "IOPCIOnline"
#define kIOPCIConfiguredKey       "IOPCIConfigured"
#define kIOPCIResourcedKey        "IOPCIResourced"
#define kIOPCIPMCSStateKey        "IOPCIPMCSState"
#define kIOPCIHPTypeKey           "IOPCIHPType"
#define kIOPCIMSIFlagsKey         "pci-msi-flags"
#define kIOPCIMSILimitKey         "pci-msi-limit"

#ifndef kACPIDevicePathKey
#define kACPIDevicePathKey             "acpi-path"
#endif

#ifndef kACPIDevicePropertiesKey
#define kACPIDevicePropertiesKey       "device-properties"
#endif

#ifndef kACPIPCILinkChangeKey
#define kACPIPCILinkChangeKey       "pci-supports-link-change"
#endif

#define kIOPCIExpressASPMDefaultKey	"pci-aspm-default"

#define kIOPCIExpressMaxLatencyKey	"pci-max-latency"

#define kIOPCIExpressErrorUncorrectableMaskKey	    "pci-aer-uncorrectable"
#define kIOPCIExpressErrorUncorrectableSeverityKey	"pci-aer-uncorrectable-severity"
#define kIOPCIExpressErrorCorrectableMaskKey	    "pci-aer-correctable"
#define kIOPCIExpressErrorControlKey	            "pci-aer-control"

// property to disable LTR on wake
#define kIOPMPCIWakeL1PMDisableKey      "pci-wake-l1pm-disable"

#define kIOPCIFunctionsDependentKey     "pci-functions-dependent"

enum
{
	kCheckLinkParents  = 0x00000001,
	kCheckLinkForPower = 0x00000002,
};

enum
{
	kLinkCapDataLinkLayerActiveReportingCapable = (1 << 20),
	kLinkStatusDataLinkLayerLinkActive 			= (1 << 13),
	kSlotCapHotplug					 			= (1 << 6)
};

enum
{
	kIOPCIExpressASPML0s   = 0x00000001,
	kIOPCIExpressASPML1    = 0x00000002,
	kIOPCIExpressCommonClk = 0x00000040,
	kIOPCIExpressClkReq    = 0x00000100
};

enum
{
    kIOPCIMSIFlagRespect = 0x00000001,
};

enum
{
    kIOPCIExpressACSSourceValidation            = (1 << 0),
    kIOPCIExpressACSTranslationBlocking         = (1 << 1),
    kIOPCIExpressACSP2PRequestRedirect          = (1 << 2),
    kIOPCIExpressACSP2PCompletionRedirect       = (1 << 3),
    kIOPCIExpressACSP2PUpstreamForwarding       = (1 << 4),
    kIOPCIExpressACSP2PEgressControl            = (1 << 5),
    kIOPCIExpressACSDirectTranslatedP2PEnable   = (1 << 6)
};
#define kIOPCIExpressACSDefault (kIOPCIExpressACSSourceValidation | kIOPCIExpressACSTranslationBlocking)

#define kIOPCIExpressL1PMControlKey	"pci-l1pm-control"
#define kIOPCIDeviceHiddenKey       "pci-device-hidden"

#ifndef kIODebugArgumentsKey
#define kIODebugArgumentsKey	 "IODebugArguments"
#endif

#ifndef kIOMemoryDescriptorOptionsKey
#define kIOMemoryDescriptorOptionsKey	 "IOMemoryDescriptorOptions"
#endif

#define kIOPCIDeviceChangedKey			"IOPCIDeviceChanged"

// Entitlements
#define kIOPCITransportDextEntitlement       "com.apple.developer.driverkit.transport.pci"
#define kIOPCITransportBridgeDextEntitlement "com.apple.developer.driverkit.transport.pci.bridge"

extern const    IORegistryPlane * gIOPCIACPIPlane;
extern const    OSSymbol *        gIOPlatformDeviceASPMEnableKey;
extern uint32_t                   gIOPCIFlags;
extern const OSSymbol *           gIOPlatformGetMessagedInterruptControllerKey;
extern const OSSymbol *           gIOPlatformGetMessagedInterruptAddressKey;
extern const OSSymbol *           gIOPCIThunderboltKey;
extern const OSSymbol *           gIOPCIHotplugCapableKey;
extern const OSSymbol *           gIOPCITunnelledKey;
extern const OSSymbol *           gIOPCIHPTypeKey;
extern const OSSymbol *           gIOPCIDeviceHiddenKey;

extern const OSSymbol *           gIOPolledInterfaceActiveKey;
#if ACPI_SUPPORT
extern const OSSymbol *           gIOPCIPSMethods[kIOPCIDevicePowerStateCount];
#endif

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#if ACPI_SUPPORT
extern IOReturn IOPCIPlatformInitialize(void);
extern IOReturn IOPCISetMSIInterrupt(uint32_t vector, uint32_t count, uint32_t * msiData);
extern uint64_t IOPCISetAPICInterrupt(uint64_t entry);
#endif

extern IOReturn IOPCIRegisterPowerDriver(IOService * service, bool hostbridge);
extern IOService * IOPCIDeviceDMAOriginator(IOPCIDevice * device);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define arrayCount(x)	(sizeof(x) / sizeof(x[0]))

enum
{
    kMSIX       = 0x01
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

class IOPCIMessagedInterruptController : public IOInterruptController
{
    OSDeclareDefaultStructors( IOPCIMessagedInterruptController )

protected:

    // The base global system interrupt number.

    SInt32                  _vectorBase;
    UInt32                  _vectorCount;
    UInt32                  _parentOffset;

    IORangeAllocator *      _messagedInterruptsAllocator;

public:

  virtual IOReturn registerInterrupt(IOService *nub, int source,
				     void *target,
				     IOInterruptHandler handler,
				     void *refCon);
  virtual IOReturn unregisterInterrupt(IOService *nub, int source);
  
  virtual IOReturn getInterruptType(IOService *nub, int source,
				    int *interruptType);
  
  virtual IOReturn enableInterrupt(IOService *nub, int source);
  virtual IOReturn disableInterrupt(IOService *nub, int source);
  
  virtual IOReturn handleInterrupt(void *refCon, IOService *nub,
				   int source);

public:

	static IOInterruptVector * allocVectors(uint32_t count);
    static void initDevice(IOPCIDevice * device, IOPCIConfigSave * save);
	static void saveDeviceState(IOPCIDevice * device, IOPCIConfigSave * save);
	static void restoreDeviceState(IOPCIDevice * device, IOPCIConfigSave * save);

    void enableDeviceMSI(IOPCIDevice *device);
    void disableDeviceMSI(IOPCIDevice *device);

    bool init(UInt32 numVectors, UInt32 baseVector);

    bool init(UInt32 numVectors);

	bool reserveVectors(UInt32 vector, UInt32 count);

    virtual void     initVector( IOInterruptVectorNumber vectorNumber,
                                 IOInterruptVector * vector );

    virtual int      getVectorType(IOInterruptVectorNumber vectorNumber, IOInterruptVector *vector);

    virtual bool     vectorCanBeShared( IOInterruptVectorNumber vectorNumber,
                                        IOInterruptVector * vector );

    virtual void     enableVector( IOInterruptVectorNumber vectorNumber,
                                   IOInterruptVector * vector );

    virtual void     disableVectorHard( IOInterruptVectorNumber vectorNumber,
                                        IOInterruptVector * vector );

    virtual bool     addDeviceInterruptProperties(
                                    IORegistryEntry * device,
                                    UInt32            controllerIndex,
                                    UInt32            interruptFlags,
                                    SInt32 *          deviceIndex);

    IOReturn allocateDeviceInterrupts(
				IOService * entry, uint32_t numVectors, uint32_t msiConfig,
				uint64_t * msiAddress = 0, uint32_t * msiData = 0);
    IOReturn         deallocateDeviceInterrupts(IOService * device);

    virtual void     deallocateInterrupt(UInt32 vector);

protected:
    virtual bool     allocateInterruptVectors( IOService *device,
                                               uint32_t numVectors,
                                               IORangeScalar *rangeStartOut);

};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

class IOPCIDiagnosticsClient : public IOUserClient
{
    OSDeclareDefaultStructors(IOPCIDiagnosticsClient)

    friend class IOPCIBridge;

    IOPCIBridge * owner;

public:
    virtual bool initWithTask(task_t owningTask,
							  void * securityID,
							  UInt32 type,
							  OSDictionary * properties);
    virtual IOReturn    clientClose(void);
    virtual IOService * getService(void);
    virtual IOReturn    setProperties(OSObject * properties);
    virtual IOReturn    externalMethod(uint32_t selector, IOExternalMethodArguments * args,
                                       IOExternalMethodDispatch * dispatch, OSObject * target, void * reference);
};

#endif /* defined(KERNEL) */

enum
{
	kIOPCISessionOptionDriverkit = 0x00010000
};

enum
{
    kIOPCIDiagnosticsClientType = 0x99000001
};

enum
{
    kIOPCIProbeOptionLinkInt      = 0x40000000,
};


enum {
	kIOPCIDiagnosticsMethodRead  = 0,
	kIOPCIDiagnosticsMethodWrite = 1,
	kIOPCIDiagnosticsMethodCount
};

struct IOPCIDiagnosticsParameters
{
	uint32_t			          options;
	uint32_t 		              spaceType;
	uint32_t			          bitWidth;
	uint32_t			          _resv;
	uint64_t			          value;
    union
    {
        uint64_t addr64;
        struct {
            unsigned int offset     :16;
            unsigned int function   :3;
            unsigned int device     :5;
            unsigned int bus        :8;
            unsigned int segment    :16;
            unsigned int reserved   :16;
        } pci;
    }                             address;
};
typedef struct IOPCIDiagnosticsParameters IOPCIDiagnosticsParameters;

#endif /* ! _IOKIT_IOPCIPRIVATE_H */

