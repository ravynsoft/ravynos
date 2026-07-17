/*
 * Copyright © 1998-2007 Apple Inc. All rights reserved.
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


#ifndef _IOKIT_IOUSBDEVICE_H
#define _IOKIT_IOUSBDEVICE_H

#include <IOKit/usb/IOUSBNub.h>
#include <IOKit/usb/IOUSBPipe.h>
#include <IOKit/usb/IOUSBPipeV2.h>
#include <IOKit/IOBufferMemoryDescriptor.h>
#include <IOKit/IOWorkLoop.h>
#include <IOKit/IOCommandGate.h>

#include <kern/thread_call.h>

// The following are definitions for errata properties needed for different devices.  This
// should be but in the dictionary of the IOUSBDevice in question.  This can be achieved
// by using the AppleUSBMergeNub class and having an IOProviderMergeProperties dictionary
// with the required properties.

// This property allows a device to specify a configuration value of 0 in its configuration
// descriptor.  This does not follow the spec, but we will allow it in order to get the device 
// to work.  The property should be a Boolean
//
#define kAllowConfigValueOfZero		"kAllowZeroConfigValue"
#define kAllowNumConfigsOfZero		"kAllowZeroNumConfigs"


class IOUSBController;
class IOUSBControllerV2;
class IOUSBInterface;
class IOUSBHubPolicyMaker;
/*!
    @class IOUSBDevice
    @abstract The IOService object representing a device on the USB bus.
    @discussion This class provides functionality to configure a device and to create
	IOUSBInterface objects to represent the interfaces of the device.
*/

class IOUSBDevice : public IOUSBNub
{
    friend class IOUSBController;
    friend class IOUSBControllerV2;
    friend class IOUSBInterface;
    friend class IOUSBPipe;
	friend class IOUSBInterfaceUserClientV2;
	friend class IOUSBInterfaceUserClientV3;
	friend class IOUSBDeviceUserClientV2;

    OSDeclareDefaultStructors(IOUSBDevice)

protected:

    USBDeviceAddress				_address;
    IOUSBController *	     		_controller;
    IOUSBPipe *						_pipeZero;
    IOUSBDeviceDescriptor			_descriptor;
    UInt32							_busPowerAvailable;
    UInt8							_speed;
    IOUSBEndpointDescriptor			_endpointZero; 			// Fake ep for control pipe
    void *							_port;					// Obsolete, do not use
    IOBufferMemoryDescriptor**		_configList;
    IOUSBInterface**				_interfaceList;			// Obsolete, do not use
    UInt8							_currentConfigValue;
    UInt8							_numInterfaces;			// Obsolete, do not use
    
    struct ExpansionData 
    {
        UInt32					_portNumber;
        thread_call_t			_doPortResetThread;
        IOUSBDevice *			_usbPlaneParent;
        bool					_portResetThreadActive;
        bool					_allowConfigValueOfZero;
        thread_call_t			_doPortSuspendThread;
        bool					_portSuspendThreadActive;
        thread_call_t			_doPortReEnumerateThread;
        bool					_resetInProgress;
        bool					_portHasBeenReset;
        IORecursiveLock*		_XgetConfigLock;				// Obsolete
        bool					_doneWaiting;                   // Obsolete
        bool					_notifiedWhileBooting;          // Obsolete
        IOWorkLoop *			_workLoop;
        IOTimerEventSource *	_notifierHandlerTimer;
        UInt32					_notificationType;
        bool					_suspendInProgress;
        bool					_portHasBeenSuspendedOrResumed;
        bool					_addExtraResetTime;
		bool					_suspendCommand;
		IOCommandGate *			_commandGate;
		OSSet *					_openInterfaces;
		bool					_resetCommand;
		IOReturn				_resetError;
		IOReturn				_suspendError;
        thread_call_t			_doMessageClientsThread;
		IOUSBHubPolicyMaker *	_hubPolicyMaker;
		UInt32					_sleepPowerAllocated;				// how much sleep power we already gave to our client
		UInt32					_wakePowerAllocated;				// how much extra power during wake did we already give our client
		UInt32					_devicePortInfo;
		bool					_deviceIsInternal;					// Will be set if all our upstream hubs are captive (internal to the computer)
		bool					_deviceIsInternalIsValid;			// true if we have already determined whether the device is internal
		bool					_newGetConfigLock;					// new lock, taken within the WL gate, when doing a GetConfig
		UInt32					_resetAndReEnumerateLock;			// "Lock" to prevent us from doing a reset or a re-enumerate while the other one is in progress		
		UInt32					_locationID;
		IOLock*					_interfaceArrayLock;
	    OSArray*				_interfaceArray;
		UInt32					_standardUSBPortPower;				// Largest amount of current that this port can provide (e.g. 500mA for USB2.0, 900mA for USB3.0)
		bool					_usingExtra400mAforUSB3;
		UInt32					_wakeRevocablePowerAllocated;		// how much extra "revocable" power during wake did we already give our client
		UInt32					_wakeUSB3PowerAllocated;			// how much extra "USB3" power during wake did we already give our client
		bool					_attachedToEnclosureAndUsingExtraWakePower;
		bool					_deviceIsOnThunderbolt;					// Will be set if all our upstream hubs are on Thunderbolt

    };
    ExpansionData * _expansionData;

    const IOUSBConfigurationDescriptor *FindConfig(UInt8 configValue, UInt8 *configIndex=0);

    virtual IOUSBInterface * GetInterface(const IOUSBInterfaceDescriptor *interface);

public:
    virtual IOReturn SetFeature(UInt8 feature);

protected:    
    virtual IOReturn GetConfigDescriptor(UInt8 configIndex, void *data, UInt32 len);

    virtual IOReturn GetDeviceDescriptor(IOUSBDeviceDescriptor *desc, UInt32 size);

    virtual bool init(USBDeviceAddress deviceAddress, UInt32 powerAvaiable, UInt8 speed, UInt8 maxPacketSize );

    virtual bool matchPropertyTable(OSDictionary * table, SInt32 *score);
    
public:
    // IOService methods
    virtual bool		init( void );
    virtual bool		start( IOService *provider );	
	virtual bool		handleIsOpen(const IOService *forClient) const;
	virtual bool		handleOpen(IOService *forClient, IOOptionBits options, void *arg);
	virtual void		handleClose(IOService *forClient, IOOptionBits options);
    virtual IOReturn 	message( UInt32 type, IOService * provider,  void * argument = 0 );
    virtual bool		terminate( IOOptionBits options = 0 );
    virtual bool		requestTerminate( IOService * provider, IOOptionBits options );
    virtual void		stop( IOService *provider );
    virtual bool		finalize(IOOptionBits options);
	virtual void		free( void );	

	// IOUSBDevice methods
    virtual void SetProperties();
    
    static IOUSBDevice *NewDevice(void);
    
    virtual void SetPort(void *port);			// Obsolete, do NOT use

    /*!
	@function FindNextInterfaceDescriptor
	return a pointer to the next interface descriptor within the given full configuration descriptor satisfying the requirements specified, or NULL if there aren't any.
        @param configDescIn the configuration descriptor within which to search. obtained from GetFullConfigurationDescriptor(configIndex). use NULL to specify the current configuration's descriptor.
        @param intfDesc interface descriptor from which to start the search. NULL for the first interface descriptor in the given configuration descriptor
        @param request IOUSBFindInterfaceRequest specifying the desired interface. Not changed.
        @param descOut pointer to placeholder for the returned descriptor ptr
	@result returns kIOReturnBadArgument if configDesc is not correct, or if configDesc is NULL and the device is not configured, or if intfDesc is not in configDesc. Otherwise returns kIOReturnSuccess or kIOUSBInterfaceNotFound.
    */
    virtual IOReturn FindNextInterfaceDescriptor(const IOUSBConfigurationDescriptor *configDescIn, 
                                                 const IOUSBInterfaceDescriptor *intfDesc,
                                                 const IOUSBFindInterfaceRequest *request,
                                                 IOUSBInterfaceDescriptor **descOut);
    /*!
	@function FindNextInterface
	return an pointer to an IOUSBInterface object satisfying the requirements specified in request, or NULL if there aren't any. the device
        must be configured for there to be any interfaces.
        @param current interface to start searching from, NULL to start at the beginning of the device's interface list for the current configuration.
	@param request specifies what properties an interface must have to match.
	@result Pointer to a matching IOUSBInterface, or NULL if none match. Note: The IOUSBInterface is NOT retained for the caller. If the caller wishes to continue to use the returned object, it should call retain() on that object.
    */
    virtual IOUSBInterface *FindNextInterface(IOUSBInterface *current,
                                              IOUSBFindInterfaceRequest *request);

    /*!
        @function CreateInterfaceIterator 
        return an OSIterator to iterate through interfaces satisfying the requirements specified in request.                        
        @param request specifies what properties an interface must have to match.
        @result Pointer to an OSIterator.
    */
    virtual OSIterator *CreateInterfaceIterator(IOUSBFindInterfaceRequest *request);

    // Get pointer to full config info (cached in device, don't free returned pointer)
    /*!
	@function GetFullConfigurationDescriptor
	return a pointer to all the descriptors for the requested configuration.
        @param configIndex The configuration index (not the configuration value)
	@result Pointer to the descriptors, which are cached in the IOUSBDevice object.
    */
    virtual const IOUSBConfigurationDescriptor *GetFullConfigurationDescriptor(UInt8 configIndex);

    /*!
	@function GetConfigurationDescriptor
        Copy the specified amount of data for a configuration into the suppled buffer.
        @param configValue The configuration value
	@param data Buffer to copy data into
	@param len number of bytes to copy
    */
    virtual IOReturn GetConfigurationDescriptor(UInt8 configValue, void *data, UInt32 len);

    /*!
	@function ResetDevice
	Reset the device, returning it to the addressed, unconfigured state.
	This is useful if a device has got badly confused. Note that the AppleUSBComposite driver will automatically
        reconfigure the device if it is a composite device.
    */
    virtual IOReturn ResetDevice();

    /*!
	@function SetConfiguration
	Do a USB SetConfiguration call to the device. The caller must have the device open() in order to 
	actually cause a configuration change. If the device is currently configured, all IOUSBInterface objects
	associated with the device are freed. After the new configuration has been set, all of its IOUSBInterface objects are
	instantiated automatically.
	@param forClient The client requesting the configuration change
	@param configValue The desired configuration value.
	@param startInterfaceMatching A boolean specifying whether IOKit should begin the process of finding
	matching drivers for the new IOUSBInterface objects.
    */
    virtual IOReturn SetConfiguration(IOService *forClient, UInt8 configValue, bool startInterfaceMatching=true);
    
    // Access to addressing and cached info
    /*!
        @function GetAddress
        returns the bus address of the device
    */
    virtual USBDeviceAddress GetAddress(void);
    
    /*!
        @function GetSpeed
        returns the speed of the device
    */
    virtual UInt8 GetSpeed(void);
    /*!
        @function GetBus
        returns a pointer to the IOUSBController object for the device
    */
    virtual IOUSBController *GetBus(void);
    /*!
        @function GetBusPowerAvailable
        returns the power available to the device, in units of 2mA
    */
    virtual UInt32 GetBusPowerAvailable( void );
    /*!
        @function GetMaxPacketSize
        returns the maximum packet size for endpoint zero (only 8, 16, 32, 64 are valid)
    */
    virtual UInt8 GetMaxPacketSize(void);
    /*!
        @function GetVendorID
        returns the Vendor ID of the device
    */
    virtual UInt16 GetVendorID(void);
    /*!
        @function GetProductID
        returns the Product ID of the device
    */
    virtual UInt16 GetProductID(void);
    /*!
        @function GetDeviceRelease
        returns the DeviceRelease information
    */
    virtual UInt16 GetDeviceRelease(void);
    /*!
        @function GetNumConfigurations
        returns the number of configs in the device config descriptor
    */
    virtual UInt8 GetNumConfigurations(void);
    /*!
        @function GetManufacturerStringIndex
        returns the index of string descriptor describing manufacturer
    */
    virtual UInt8 GetManufacturerStringIndex(void );
    /*!
        @function GetProductStringIndex
        returns the index of string descriptor describing product
    */
    virtual UInt8 GetProductStringIndex(void );
    /*!
        @function GetSerialNumberStringIndex
        returns the index of string descriptor describing the device's serial number
    */
    virtual UInt8 GetSerialNumberStringIndex(void );
    /*!
        @function GetPipeZero
        returns a pointer to the device's default control pipe
    */
    virtual IOUSBPipe * GetPipeZero(void);

	// Deprecated but needed for binary compatibility
    virtual IOUSBPipe*	MakePipe(const IOUSBEndpointDescriptor *ep);
    
    // this method is deprecated. use the other DeviceRequest methods
    virtual IOReturn DeviceRequest(IOUSBDevRequest	*request,
                                IOUSBCompletion	*completion = 0);

    // Same but with a memory descriptor
    virtual IOReturn DeviceRequest(IOUSBDevRequestDesc	*request,
                                   IOUSBCompletion	*completion = 0);

    /*!
	@function GetConfiguration
	Gets the current configuration from the IOUSBDevice object.  Note that this call will send a control
        request on the bus to get the current configuration from the physical device.
	@param configNum Pointer to place to store configuration value.
    */
    virtual IOReturn GetConfiguration(UInt8 *configNumber);

    /*!
        @function GetDeviceStatus
        Gets the device's status.  Note that this sends a control request to the physical device.
        @param status Pointer to place to store the status.
    */
    virtual IOReturn GetDeviceStatus(USBStatus *status);

    /*! 
	@function GetStringDescriptor
	Get a string descriptor as ASCII, in the specified language (default is US English)
	@param index Index of the string descriptor to get.
	@param buf Pointer to place to store ASCII string
	@param maxLen Size of buffer pointed to by buf
        @param lang Language to get string in (default is US English)
    */
    virtual IOReturn GetStringDescriptor(UInt8 index, char *buf, int maxLen, UInt16 lang=0x409);
    
    /*! 
	@function GetChildLocationID
	Get the locationID (UInt32) given the port number and the parent's location
	@param parentLocationID locationID for the hub to which this device is attached.
	@param port Port number of the hub where this device is attached
    */
    virtual UInt32 GetChildLocationID(UInt32 parentLocationID, int port);

    virtual const IOUSBDescriptorHeader* FindNextDescriptor(const void *cur, UInt8 descType);

    virtual void 	DisplayNotEnoughPowerNotice();
    
    // These are non-virtual function so that we don't have to take up a binary compatibility slot.
    UInt16	GetbcdUSB(void);
    UInt8   GetDeviceClass(void);
    UInt8   GetDeviceSubClass(void);
    UInt8	GetProtocol(void);
    UInt32  GetLocationID(void);
	void	SetBusPowerAvailable(UInt32 newPower);

    OSMetaClassDeclareReservedUsed(IOUSBDevice,  0);
    /*!
	@function DeviceRequest
        @abstract execute a control request to the default control pipe (pipe zero)
        @param request The parameter block to send to the device
        @param noDataTimeout Specifies an amount of time (in ms) after which the command will be aborted
        if no data has been transferred on the bus.
        @param completionTimeout Specifies an amount of time (in ms) after which the command will be aborted if the entire command has
        not been completed.
		@param completion Function to call when request completes. If omitted then
	 DeviceRequest() executes synchronously, blocking until the request is complete.  If the request is asynchronous, the client must make sure that
	 the IOUSBDevRequest is not released until the callback has occurred.

    */
    virtual IOReturn DeviceRequest(IOUSBDevRequest	*request,
				UInt32 noDataTimeout,
				UInt32 completionTimeout,
                                IOUSBCompletion	*completion = 0);

    /*!
	 @function DeviceRequest
	 @abstract execute a control request to the default control pipe (pipe zero)
	 @param request The parameter block to send to the device (with the pData as an IOMemoryDesriptor)
	 @param noDataTimeout Specifies an amount of time (in ms) after which the command will be aborted
	 if no data has been transferred on the bus.
	 @param completionTimeout Specifies an amount of time (in ms) after which the command will be aborted if the entire command has
	 not been completed.
	 @param completion Function to call when request completes. If omitted then
	 DeviceRequest() executes synchronously, blocking until the request is complete.  If the request is asynchronous, the client must make sure that
	 the IOUSBDevRequest is not released until the callback has occurred.
	 
	 */
    OSMetaClassDeclareReservedUsed(IOUSBDevice,  1);
    virtual IOReturn DeviceRequest(IOUSBDevRequestDesc	*request,
				    UInt32 noDataTimeout,
				    UInt32 completionTimeout,
				    IOUSBCompletion	*completion = 0);

    OSMetaClassDeclareReservedUsed(IOUSBDevice,  2);
    /*!
	@function SuspendDevice
        @abstract Instruct the hub to which this device is attached to suspend or resume the port to which the device is attached.
        Note that if there are any outstanding transactions on any pipes in the device, those transactions will get returned with a 
        kIOReturnNotResponding error.
        @param suspend Boolean value. true = suspend, false = resume.
    */
    virtual IOReturn SuspendDevice( bool suspend);
    
    OSMetaClassDeclareReservedUsed(IOUSBDevice,  3);
    /*!
	@function ReEnumerateDevice
        @abstract Instruct the hub to which this device is attached to reset the port to which this device is attached. This causes the
        IOUSBDevice object and any child objects (IOUSBInterface objects or driver objects) to be terminated, and the device to be
        completely reenumerated, as if it had been detached and reattached.
        @param options Reserved for future use.
    */
    virtual IOReturn ReEnumerateDevice( UInt32 options );
    
    OSMetaClassDeclareReservedUsed(IOUSBDevice,  4);
    /*!
        @function DisplayUserNotification
        @abstract  Will use the Notification Center to display a notification to the user.  Only Low Power and Overcurrent notifications are supported.
        @param notificationType Which notification to display.
     */
    virtual void	DisplayUserNotification(UInt32 notificationType);
    
    OSMetaClassDeclareReservedUsed(IOUSBDevice,  5);
	// Deprecated but needed for binary compatibility
    virtual IOUSBPipe*	MakePipe(const IOUSBEndpointDescriptor *ep, IOUSBInterface *interface);
    
	
    OSMetaClassDeclareReservedUsed(IOUSBDevice,  6);
	/*!
	 @function SetHubParent
	 @abstract Used by the hub driver to give the nub a pointer to its HubPolicyMaker object
	 @param hubPolicyMaker	The object representing the Hub driver
	 */
    virtual void	SetHubParent(IOUSBHubPolicyMaker *hubParent);
    
    OSMetaClassDeclareReservedUsed(IOUSBDevice,  7);
	/*!
	 @function GetHubParent
	 @abstract Used by the hub driver to give the nub a pointer to its HubPolicyMaker object
	 @param hubPolicyMaker	The object representing the Hub driver
	 */
    virtual IOUSBHubPolicyMaker*	GetHubParent();
    
    OSMetaClassDeclareReservedUsed(IOUSBDevice,  8);
    /*!
	 @function 	GetDeviceInformation
	 @abstract 	Returns status information about the USB device, such as whether the device is captive or whether it is in the suspended state.
	 @param requestedPower 	The desired amount of power that the client wishes to reserve
	 @result 		Actual power that was reserved
	 
		*/
	virtual IOReturn	GetDeviceInformation(UInt32 *info);
    
    OSMetaClassDeclareReservedUsed(IOUSBDevice,  9);
    /*!
	 @function				RequestExtraPower
	 @abstract				Clients can use this API to reserve extra power for use by this device while the machine is asleep or while it is awake.  Units are milliAmps (mA).
	 @param type			Indicates whether the power is to be used during wake or sleep (One of kUSBPowerDuringSleep or kUSBPowerDuringWake)
	 @param requestedPower 	Amount of power desired, in mA
	 @result				Amount of power actually reserved, in mA.  It can be less than the requested or zero.
	 
	 */
	virtual UInt32	RequestExtraPower(UInt32 type, UInt32 requestedPower);
    
    OSMetaClassDeclareReservedUsed(IOUSBDevice,  10);
    /*!
	 @function				ReturnExtraPower
	 @abstract				Clients can use this API to tell the system that they will not use power that was previously reserved by using the RequestExtraPower API.
	 @param type			Indicates whether the power is to be used during wake or sleep (One of kUSBPowerDuringSleep or kUSBPowerDuringWake)
	 @param returnedPower 	Amount of power that is no longer needed, in mA
	 @result				If the returnedPower was not previously allocated, an error will be returned.  This will include the case for power that was requested for sleep but was returned for wake.
	 
	 */
	virtual IOReturn		ReturnExtraPower(UInt32 type, UInt32 returnedPower);
    
    OSMetaClassDeclareReservedUsed(IOUSBDevice,  11);
    /*!
	 @function				GetExtraPowerAllocated
	 @abstract				Clients can use this API to ask how much extra power has already been reserved by this device.  Units are milliAmps (mA).
	 @param type			Indicates whether the allocated power was to be used during wake or sleep (One of kUSBPowerDuringSleep or kUSBPowerDuringWake)
	 @result				Amount of power allocated, in mA.
	 
	 */
	virtual UInt32			GetExtraPowerAllocated(UInt32 type);
    
    OSMetaClassDeclareReservedUsed(IOUSBDevice,  12);
    /*!
	 @function				DoLocationOverrideAndModelMatch
	 @abstract				Will look for a kOverrideIfAtLocationID array proerty with locationID entries and a "MacModel" property.  If any of the locationIDs match to the Mac Model, will return true.
	 						If there is no kOverrideAtLocationID property, it will also return true.
	 @result				True if we have a match, false otherwise
	 */
	virtual	bool			DoLocationOverrideAndModelMatch();
	
    OSMetaClassDeclareReservedUsed(IOUSBDevice,  13);
    /*!
	 @function SetConfiguration
	 Do a USB SetConfiguration call to the device. The caller must have the device open() in order to 
	 actually cause a configuration change. If the device is currently configured, all IOUSBInterface objects
	 associated with the device are freed. After the new configuration has been set, all of its IOUSBInterface objects are
	 instantiated automatically.
	 @param forClient The client requesting the configuration change
	 @param configValue The desired configuration value.
	 @param startInterfaceMatching A boolean specifying whether IOKit should begin the process of finding
	 @param issueRemoteWakeup A boolean specifying whether we should issue the SetFeature(kUSBFeatureDeviceRemoteWakeup) immediately after setting the configuration, before loading any drivers.
	 matching drivers for the new IOUSBInterface objects.
	 */
    virtual IOReturn SetConfiguration(IOService *forClient, UInt8 configValue, bool startInterfaceMatching, bool issueRemoteWakeup);
    
    OSMetaClassDeclareReservedUsed(IOUSBDevice,  14);
    /*!
	 @function		OpenOrCloseAllInterfacePipes		
	 @abstract		Iterates over all interfaces and either opens all the pipes, or closes them all		
	 @param			open, if true pipes are opened, else closed
	 */
	virtual void OpenOrCloseAllInterfacePipes(bool open);
	
	
	
	
    OSMetaClassDeclareReservedUsed(IOUSBDevice,  15);
	/*!
	 @function MakePipe
	 @abstract build a pipe on a given endpoint
	 @param ep A description of the endpoint
	 @param sscd The SuperSpeed Companion Descriptor, if any (NULL if none)
	 @param interface The IOUSBInterface object requesting the pipe
	 returns the desired IOUSBPipe object
	 */
    virtual IOUSBPipeV2*	MakePipe(const IOUSBEndpointDescriptor *ep, IOUSBSuperSpeedEndpointCompanionDescriptor *sscd, IOUSBInterface *interface);

    OSMetaClassDeclareReservedUsed(IOUSBDevice,  16);
    /*!
	 @function SetAddress
	 @param 	Sets the bus address of the device
	 */
    virtual void 			SetAddress(USBDeviceAddress address);
    

    OSMetaClassDeclareReservedUnused(IOUSBDevice,  17);
    OSMetaClassDeclareReservedUnused(IOUSBDevice,  18);
    OSMetaClassDeclareReservedUnused(IOUSBDevice,  19);

private:

    static void			ProcessPortResetEntry(__unused OSObject *target){};			// obsolete
    void				ProcessPortReset(void){};									// obsolete

    void				TerminateInterfaces(bool terminate);

    static void			ProcessPortReEnumerateEntry(OSObject *target, thread_call_param_t options);
    void				ProcessPortReEnumerate(UInt32 options);
	
    static void			DoMessageClientsEntry(OSObject *target, thread_call_param_t messageStruct);
    void				DoMessageClients( void * messageStructPtr);
	
    static void			DisplayUserNotificationForDeviceEntry (OSObject *owner, IOTimerEventSource *sender);
    void				DisplayUserNotificationForDevice( );
    
    UInt32              SimpleUnicodeToUTF8(UInt16 uChar, UInt8 utf8Bytes[4]);
    void                SwapUniWords (UInt16  **unicodeString, UInt32 uniSize);

    IOReturn			TakeGetConfigLock(void);
    IOReturn			ReleaseGetConfigLock(void);
    static IOReturn		ChangeGetConfigLock(OSObject *target, void *arg0, void *arg1, void *arg2, void *arg3);
	
	void				RegisterInterfaces(void);
	
    static IOReturn		_ResetDevice(OSObject *target, void *arg0, void *arg1, void *arg2, void *arg3);
    static IOReturn		_ReEnumerateDevice(OSObject *target, void *arg0, void *arg1, void *arg2, void *arg3);
	static IOReturn		_DeviceRequest(OSObject *target, void *arg0, void *arg1, __unused void *arg2, __unused void *arg3);
	static IOReturn		_DeviceRequestDesc(OSObject *target, void *arg0, void *arg1, __unused void *arg2, __unused void *arg3);
	static IOReturn		_GetConfiguration(OSObject *target, void *arg0,  __unused void *arg1, __unused void *arg2, __unused void *arg3);
	static IOReturn		_GetDeviceStatus(OSObject *target, void *arg0,  __unused void *arg1, __unused void *arg2, __unused void *arg3);
	static IOReturn		_DeviceRequestWithTimeout(OSObject *target, void *arg0, void *arg1, void *arg2, void *arg3);
	static IOReturn		_DeviceRequestDescWithTimeout(OSObject *target, void *arg0, void *arg1, void *arg2, void *arg3);

};

#endif
