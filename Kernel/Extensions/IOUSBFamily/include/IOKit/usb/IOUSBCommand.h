/*
 * Copyright © 1998-2012 Apple Inc. All rights reserved.
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

#ifndef _IOKIT_IOUSBCOMMAND_H
#define _IOKIT_IOUSBCOMMAND_H

#include <IOKit/IOCommand.h>
#include <IOKit/IOCommandPool.h>
#include <IOKit/IOMemoryDescriptor.h>
#include <IOKit/IODMACommand.h>
#include <IOKit/usb/USB.h>

typedef IOUSBDevRequest *IOUSBDeviceRequestPtr;

/*
 * USB Command
 * This is the command block for a USB command to be queued on the
 * Command Gate of the WorkLoop.  Although the actual work of creating
 * the command and enqueueing it is done for them via the deviceRequest,
 * read, and write methods, this is the primary way that devices will get
 * work done.
 * Note: This is private to IOUSBController and should be moved to a
 * private header.
 */

typedef enum {
    DEVICE_REQUEST,						// Device request using pointer
    READ,
    WRITE,
    CREATE_EP,
    DELETE_EP,
    DEVICE_REQUEST_DESC,				// Device request using descriptor
	DEVICE_REQUEST_BUFFERCOMMAND,		// Device request using a second IOUSBCommand in _bufferCommand
	INVALID_SELECTOR	=	0xFF,		// For use to invalidate a IOUSBCommand that has been returned
} usbCommand;

#define 	kUSBCommandScratchBuffers	10

/*
 IOUSBCommand
 Subclass of IOCommand that is used to add USB specific data.
*/
class IOUSBCommand : public IOCommand
{
    OSDeclareAbstractStructors(IOUSBCommand)

protected:
    usbCommand				_selector;
    IOUSBDeviceRequestPtr	_request;
    USBDeviceAddress		_address;
    UInt8					_endpoint;
    UInt8					_direction;
    UInt8					_type;
    bool					_bufferRounding;
    IOMemoryDescriptor *	_buffer;
    IOUSBCompletion			_uslCompletion;
    IOUSBCompletion			_clientCompletion;
    UInt32					_dataRemaining;							// For Control transfers
    UInt8					_stage;									// For Control transfers
    IOReturn				_status;
    IOMemoryDescriptor *	_origBuffer;
    IOUSBCompletion			_disjointCompletion;
    IOByteCount				_dblBufLength;
    UInt32					_noDataTimeout;
    UInt32					_completionTimeout;
    UInt32					_UIMScratch[kUSBCommandScratchBuffers];
    
    struct ExpansionData
    {
        IOByteCount			_reqCount;
		IOMemoryDescriptor *_requestMemoryDescriptor;
		IOMemoryDescriptor *_bufferMemoryDescriptor;
		bool				_multiTransferTransaction;
		bool				_finalTransferInTransaction;
        bool				_useTimeStamp;
        AbsoluteTime		_timeStamp;
		bool				_isSyncTransfer;						// Returns true if the command is used for a synchronous transfer
		IODMACommand		*_dmaCommand;							// used to get memory mapping
		IOUSBCommand		*_bufferUSBCommand;						// points to another IOUSBCommand used for phase 2 of control transactions
		IOUSBCommand		*_masterUSBCommand;						// points from the bufferUSBCommand back to the parent command
		UInt32				_streamID;
		void *				_backTrace[kUSBCommandScratchBuffers];
    };
    ExpansionData * 		_expansionData;
    
    // we override these OSObject method in order to allocate and release our expansion data
    virtual bool init();
    virtual void free();

public:

    // static constructor
    static IOUSBCommand *	NewCommand(void);

    // Manipulators
    void					SetSelector(usbCommand sel);
    void  					SetRequest(IOUSBDeviceRequestPtr req);
    void  					SetAddress(USBDeviceAddress addr);
    void  					SetEndpoint(UInt8 ep);
    void  					SetDirection(UInt8 dir);
    void  					SetType(UInt8 type);
    void  					SetBufferRounding(bool br);
    void  					SetBuffer(IOMemoryDescriptor *buf);
    void  					SetUSLCompletion(IOUSBCompletion completion);
    void  					SetClientCompletion(IOUSBCompletion completion);
    void  					SetDataRemaining(UInt32 dr);
    void  					SetStage(UInt8 stage);
    void 					SetStatus(IOReturn stat);
    void 					SetOrigBuffer(IOMemoryDescriptor *buf);
    void 					SetDisjointCompletion(IOUSBCompletion completion);
    void 					SetDblBufLength(IOByteCount len);
    void 					SetNoDataTimeout(UInt32 to);
    void 					SetCompletionTimeout(UInt32 to);
    void 					SetUIMScratch(UInt32 index, UInt32 value);
    void 					SetReqCount(IOByteCount reqCount);
    void					SetRequestMemoryDescriptor(IOMemoryDescriptor *requestMemoryDescriptor);
    void					SetBufferMemoryDescriptor(IOMemoryDescriptor *bufferMemoryDescriptor);
    void					SetMultiTransferTransaction(bool);
    void					SetFinalTransferInTransaction(bool);
    void					SetUseTimeStamp(bool);
    void					SetTimeStamp(AbsoluteTime timeStamp);
	void					SetIsSyncTransfer(bool);
	inline void				SetDMACommand(IODMACommand *dmaCommand)					{ _expansionData->_dmaCommand = dmaCommand; }
	inline void				SetStreamID(UInt32 streamID)					{ _expansionData->_streamID = streamID; }
	void					SetBufferUSBCommand(IOUSBCommand *bufferUSBCommand);
	void					SetBT(UInt32 index, void * value);
	
	// Accessors
    usbCommand					GetSelector(void);
    IOUSBDeviceRequestPtr		GetRequest(void);
    USBDeviceAddress			GetAddress(void);
    UInt8						GetEndpoint(void);
    UInt8						GetDirection(void);
    UInt8						GetType(void);
    bool						GetBufferRounding(void);
    IOMemoryDescriptor *		GetBuffer(void);
    IOUSBCompletion				GetUSLCompletion(void);
    IOUSBCompletion				GetClientCompletion(void);
    UInt32						GetDataRemaining(void);
    UInt8						GetStage(void);
    IOReturn					GetStatus(void);
    IOMemoryDescriptor *		GetOrigBuffer(void);
    IOUSBCompletion				GetDisjointCompletion(void);
    IOByteCount					GetDblBufLength(void);
    UInt32						GetNoDataTimeout(void);
    UInt32						GetCompletionTimeout(void);
    UInt32						GetUIMScratch(UInt32 index);
    IOByteCount					GetReqCount(void);
    IOMemoryDescriptor *		GetRequestMemoryDescriptor(void);
    IOMemoryDescriptor *		GetBufferMemoryDescriptor(void);
    bool						GetMultiTransferTransaction(void);
    bool						GetFinalTransferInTransaction(void);
    bool						GetUseTimeStamp(void);
    AbsoluteTime				GetTimeStamp(void);
	bool						GetIsSyncTransfer(void);
	inline IODMACommand *		GetDMACommand(void)							{return _expansionData->_dmaCommand; }
	inline UInt32				GetStreamID(void)							{return _expansionData->_streamID; }
	inline IOUSBCommand *		GetBufferUSBCommand(void)					{return _expansionData->_bufferUSBCommand; }
};


class IOUSBIsocCommand : public IOCommand
{
    OSDeclareAbstractStructors(IOUSBIsocCommand)

protected:
    usbCommand				_selector;
    USBDeviceAddress		_address;
    UInt8					_endpoint;
    UInt8					_direction;
    IOMemoryDescriptor *	_buffer;
    IOUSBIsocCompletion		_completion;
    UInt64					_startFrame;
    UInt32					_numFrames;
    IOUSBIsocFrame *		_frameList;
    IOReturn				_status;
    
    struct ExpansionData
    {
        UInt32				_updateFrequency;
        bool				_useTimeStamp;
        AbsoluteTime		_timeStamp;
		bool				_isSyncTransfer;								// Returns true if the command is used for a synchronous transfer
		bool				_rosettaClient;
		IODMACommand *		_dmaCommand;
		IOUSBIsocCompletion	_uslCompletion;
		bool				_lowLatency;
		UInt32				_UIMScratch[kUSBCommandScratchBuffers];
    };
    ExpansionData * 		_expansionData;

    // we override these OSObject method in order to allocate and release our expansion data
    virtual bool init();
    virtual void free();

public:
    // static constructor
    static IOUSBIsocCommand *NewCommand(void);
    
    // Manipulators
    void					SetSelector(usbCommand sel)							{_selector = sel; }
    void					SetAddress(USBDeviceAddress addr)					{_address = addr; }
    void					SetEndpoint(UInt8 ep)								{_endpoint = ep; }
    void					SetDirection(UInt8 dir)								{_direction = dir; }
    void					SetBuffer(IOMemoryDescriptor *buf)					{_buffer = buf; }
    void					SetCompletion(IOUSBIsocCompletion completion)		{_completion = completion; }
    void					SetStartFrame(UInt64 sf)							{_startFrame = sf; }
    void					SetNumFrames(UInt32 nf)								{_numFrames = nf; }
    void					SetFrameList(IOUSBIsocFrame *fl)					{_frameList = fl; }
    void					SetStatus(IOReturn stat)							{_status = stat; }
    void					SetUpdateFrequency( UInt32 fr)						{ _expansionData->_updateFrequency = fr; }
    void 					SetUIMScratch(UInt32 index, UInt32 value)			{ if(index < kUSBCommandScratchBuffers)_expansionData->_UIMScratch[index] = value; }
    void					SetUseTimeStamp(bool useIt)							{ _expansionData->_useTimeStamp= useIt; }
    void					SetTimeStamp(AbsoluteTime timeStamp)				{ _expansionData->_timeStamp= timeStamp; }
 	void					SetIsSyncTransfer(bool isSync)						{ _expansionData->_isSyncTransfer = isSync; }
 	void					SetRosettaClient(bool isRosetta)					{ _expansionData->_rosettaClient = isRosetta; }
 	void					SetDMACommand(IODMACommand *dmaCommand)				{ _expansionData->_dmaCommand = dmaCommand; }
    void					SetUSLCompletion(IOUSBIsocCompletion completion)	{ _expansionData->_uslCompletion = completion; }
 	void					SetLowLatency(bool lowLatency)						{ _expansionData->_lowLatency = lowLatency; }

	// Accessors
    usbCommand				GetSelector(void)								{ return _selector; }
    USBDeviceAddress		GetAddress(void)								{ return _address; }
    UInt8					GetEndpoint(void)								{ return _endpoint; }
    UInt8					GetDirection(void)								{ return _direction; }
    IOMemoryDescriptor *	GetBuffer(void)									{ return _buffer; }
    IOUSBIsocCompletion		GetCompletion(void)								{ return _completion; }
    UInt64					GetStartFrame(void)								{ return _startFrame; }
    UInt32					GetNumFrames(void)								{ return _numFrames; }
    IOUSBIsocFrame *		GetFrameList(void)								{ return _frameList; }
    UInt32					GetUpdateFrequency(void)						{ return _expansionData->_updateFrequency; }
    UInt32					GetUIMScratch(UInt32 index)						{ return( (index < kUSBCommandScratchBuffers)?_expansionData->_UIMScratch[index]:0); }
    IOReturn				GetStatus(void)									{ return _status; }
    bool					GetUseTimeStamp(void)							{ return _expansionData->_useTimeStamp; }
    AbsoluteTime			GetTimeStamp(void)								{ return _expansionData->_timeStamp; }
	bool					GetIsSyncTransfer(void)							{ return _expansionData->_isSyncTransfer; }
	bool					GetIsRosettaClient(void)						{ return _expansionData->_rosettaClient; }
	IODMACommand *			GetDMACommand(void)								{ return _expansionData->_dmaCommand; }
    IOUSBIsocCompletion		GetUSLCompletion(void)							{ return _expansionData->_uslCompletion; }
	bool					GetLowLatency(void)								{ return _expansionData->_lowLatency; }
};

class IOUSBCommandPool : public IOCommandPool
{
    OSDeclareDefaultStructors( IOUSBCommandPool )
	
protected:
    virtual IOReturn gatedReturnCommand(IOCommand * command);
	virtual IOReturn gatedGetCommand(IOCommand ** command, bool blockForCommand);
	
public:
    static IOCommandPool * withWorkLoop(IOWorkLoop * inWorkLoop);
};


#endif

