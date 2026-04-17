/*
 * Copyright (c) 1998-2014 Apple Inc. All rights reserved.
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

/*!
 * @header IOBlockStorageDriver
 * @abstract
 * This header contains the IOBlockStorageDriver class definition.
 */

#ifndef _IOBLOCKSTORAGEDRIVER_H
#define _IOBLOCKSTORAGEDRIVER_H

#include <IOKit/IOTypes.h>

/*!
 * @defined kIOBlockStorageDriverClass
 * @abstract
 * The name of the IOBlockStorageDriver class.
 */

#define kIOBlockStorageDriverClass "IOBlockStorageDriver"

/*!
 * @defined kIOBlockStorageDriverStatisticsKey
 * @abstract
 * Holds a table of numeric values describing the driver's
 * operating statistics.
 * @discussion
 * This property holds a table of numeric values describing the driver's
 * operating statistics.  The table is an OSDictionary, where each entry
 * describes one given statistic.
 */

#define kIOBlockStorageDriverStatisticsKey "Statistics"

/*!
 * @defined kIOBlockStorageDriverStatisticsBytesReadKey
 * @abstract
 * Describes the number of bytes read since the block storage
 * driver was instantiated.
 * @discussion
 * This property describes the number of bytes read since the block storage
 * driver was instantiated.  It is one of the statistic entries listed under
 * the top-level kIOBlockStorageDriverStatisticsKey property table.  It has
 * an OSNumber value.
 */

#define kIOBlockStorageDriverStatisticsBytesReadKey "Bytes (Read)"

/*!
 * @defined kIOBlockStorageDriverStatisticsBytesWrittenKey
 * @abstract
 * Describes the number of bytes written since the block storage
 * driver was instantiated. 
 * @discussion
 * This property describes the number of bytes written since the block storage
 * driver was instantiated.  It is one of the statistic entries listed under the
 * top-level kIOBlockStorageDriverStatisticsKey property table.  It has an
 * OSNumber value.
 */

#define kIOBlockStorageDriverStatisticsBytesWrittenKey "Bytes (Write)"

/*!
 * @defined kIOBlockStorageDriverStatisticsReadErrorsKey
 * @abstract
 * Describes the number of read errors encountered since the block
 * storage driver was instantiated. 
 * @discussion
 * This property describes the number of read errors encountered since the block
 * storage driver was instantiated.  It is one of the statistic entries listed
 * under the top-level kIOBlockStorageDriverStatisticsKey property table.  It
 * has an OSNumber value.
 */

#define kIOBlockStorageDriverStatisticsReadErrorsKey "Errors (Read)"

/*!
 * @defined kIOBlockStorageDriverStatisticsWriteErrorsKey
 * @abstract
 * Describes the number of write errors encountered since the
 * block storage driver was instantiated.
 * @discussion
 * This property describes the number of write errors encountered since the
 * block storage driver was instantiated.  It is one of the statistic entries
 * listed under the top-level kIOBlockStorageDriverStatisticsKey property table. 
 * It has an OSNumber value.
 */

#define kIOBlockStorageDriverStatisticsWriteErrorsKey "Errors (Write)"

/*!
 * @defined kIOBlockStorageDriverStatisticsLatentReadTimeKey
 * @abstract
 * Describes the number of nanoseconds of latency during reads
 * since the block storage driver was instantiated. 
 * @discussion
 * This property describes the number of nanoseconds of latency during reads
 * since the block storage driver was instantiated.  It is one of the statistic
 * entries listed under the top-level kIOBlockStorageDriverStatisticsKey
 * property table.  It has an OSNumber value.
 */

#define kIOBlockStorageDriverStatisticsLatentReadTimeKey "Latency Time (Read)"

/*!
 * @defined kIOBlockStorageDriverStatisticsLatentWriteTimeKey
 * @abstract
 * Describes the number of nanoseconds of latency during writes
 * since the block storage driver was instantiated. 
 * @discussion
 * This property describes the number of nanoseconds of latency during writes
 * since the block storage driver was instantiated.  It is one of the statistic
 * entries listed under the top-level kIOBlockStorageDriverStatisticsKey
 * property table.  It has an OSNumber value.
 */

#define kIOBlockStorageDriverStatisticsLatentWriteTimeKey "Latency Time (Write)"

/*!
 * @defined kIOBlockStorageDriverStatisticsReadsKey
 * @abstract
 * Describes the number of read operations processed since the
 * block storage driver was instantiated.
 * @discussion
 * This property describes the number of read operations processed since the
 * block storage driver was instantiated.  It is one of the statistic entries
 * listed under the top-level kIOBlockStorageDriverStatisticsKey property table.
 * It has an OSNumber value.
 */

#define kIOBlockStorageDriverStatisticsReadsKey "Operations (Read)"

/*!
 * @defined kIOBlockStorageDriverStatisticsWritesKey
 * @abstract
 * Describes the number of write operations processed since the
 * block storage driver was instantiated.
 * @discussion
 * This property describes the number of write operations processed since the
 * block storage driver was instantiated.  It is one of the statistic entries
 * listed under the top-level kIOBlockStorageDriverStatisticsKey property table.
 * It has an OSNumber value.
 */

#define kIOBlockStorageDriverStatisticsWritesKey "Operations (Write)"

/*!
 * @defined kIOBlockStorageDriverStatisticsReadRetriesKey
 * @abstract
 * Describes the number of read retries required since the block
 * storage driver was instantiated.
 * @discussion
 * This property describes the number of read retries required since the block
 * storage driver was instantiated.  It is one of the statistic entries listed
 * under the top-level kIOBlockStorageDriverStatisticsKey property table.  It
 * has an OSNumber value.
 */

#define kIOBlockStorageDriverStatisticsReadRetriesKey "Retries (Read)"

/*!
 * @defined kIOBlockStorageDriverStatisticsWriteRetriesKey
 * @abstract
 * Describes the number of write retries required since the block
 * storage driver was instantiated.
 * @discussion
 * This property describes the number of write retries required since the block
 * storage driver was instantiated.  It is one of the statistic entries listed
 * under the top-level kIOBlockStorageDriverStatisticsKey property table.  It
 * has an OSNumber value.
 */

#define kIOBlockStorageDriverStatisticsWriteRetriesKey "Retries (Write)"

/*!
 * @defined kIOBlockStorageDriverStatisticsTotalReadTimeKey
 * @abstract
 * Describes the number of nanoseconds spent performing reads
 * since the block storage driver was instantiated.
 * @discussion
 * This property describes the number of nanoseconds spent performing reads
 * since the block storage driver was instantiated.  It is one of the statistic
 * entries listed under the top-level kIOBlockStorageDriverStatisticsKey
 * property table.  It has an OSNumber value.
 */

#define kIOBlockStorageDriverStatisticsTotalReadTimeKey "Total Time (Read)"

/*!
 * @defined kIOBlockStorageDriverStatisticsTotalWriteTimeKey
 * @abstract
 * Describes the number of nanoseconds spent performing writes
 * since the block storage driver was instantiated.
 * @discussion
 * This property describes the number of nanoseconds spent performing writes
 * since the block storage driver was instantiated.  It is one of the statistic
 * entries listed under the top-level kIOBlockStorageDriverStatisticsKey
 * property table.  It has an OSNumber value.
 */

#define kIOBlockStorageDriverStatisticsTotalWriteTimeKey "Total Time (Write)"

/*!
 * @enum IOMediaState
 * @abstract
 * The different states that getMediaState() can report.
 * @constant kIOMediaStateOffline
 * Media is not available.
 * @constant kIOMediaStateOnline
 * Media is available and ready for operations.
 * @constant kIOMediaStateBusy
 * Media is available, but not ready for operations.
 */

enum
{
    kIOMediaStateOffline = 0,
    kIOMediaStateOnline  = 1,
    kIOMediaStateBusy    = 2
};

typedef UInt32 IOMediaState;

#ifdef KERNEL
#ifdef __cplusplus

/*
 * Kernel
 */

#include <IOKit/storage/IOBlockStorageDevice.h>
#include <IOKit/storage/IOMedia.h>
#include <IOKit/storage/IOStorage.h>

/*!
 * @class IOBlockStorageDriver
 * @abstract
 * The common base class for generic block storage drivers.
 * @discussion
 * The IOBlockStorageDriver class is the common base class for generic block
 * storage drivers.  It matches and communicates via an IOBlockStorageDevice
 * interface, and connects to the remainder of the storage framework via the
 * IOStorage protocol. It extends the IOStorage protocol by implementing the
 * appropriate open and close semantics, deblocking for unaligned transfers,
 * polling for ejectable media, locking and ejection policies, media object
 * creation and tear-down, and statistics gathering and reporting.
 *
 * Block storage drivers are split into two parts: the generic driver handles
 * all generic device issues, independent of the lower-level transport
 * mechanism (e.g. SCSI, ATA, USB, FireWire). All storage operations
 * at the generic driver level are translated into a series of generic
 * device operations. These operations are passed via the IOBlockStorageDevice
 * nub to a transport driver, which implements the appropriate
 * transport-dependent protocol to execute these operations.
 *
 * To determine the write-protect state of a device (or media), for
 * example, the generic driver would issue a call to the
 * Transport Driver's reportWriteProtection method. If this were a SCSI
 * device, its transport driver would issue a Mode Sense command to
 * extract the write-protection status bit. The transport driver then
 * reports true or false to the generic driver.
 * 
 * The generic driver therefore has no knowledge of, or involvement
 * with, the actual commands and mechanisms used to communicate with
 * the device. It is expected that the generic driver will rarely, if
 * ever, need to be subclassed to handle device idiosyncrasies; rather,
 * the transport driver should be changed via overrides.
 * 
 * A generic driver could be subclassed to create a different type of
 * generic device. The generic driver IOCDBlockStorageDriver class is
 * a subclass of IOBlockStorageDriver, adding CD functions.
 */

class IOBlockStorageDriver : public IOStorage
{
    OSDeclareDefaultStructors(IOBlockStorageDriver);

public:

    /*!
     * @enum Statistics
     * @abstract
     * Indices for the different statistics that getStatistics() can report.
     * @constant kStatisticsReads Number of read operations thus far.
     * @constant kStatisticsBytesRead Number of bytes read thus far.
     * @constant kStatisticsTotalReadTime Nanoseconds spent performing reads thus far.
     * @constant kStatisticsLatentReadTime Nanoseconds of latency during reads thus far.
     * @constant kStatisticsReadRetries Number of read retries thus far.
     * @constant kStatisticsReadErrors Number of read errors thus far.
     * @constant kStatisticsWrites Number of write operations thus far.
     * @constant kStatisticsSingleBlockWrites Number of write operations for a single block thus far.
     * @constant kStatisticsBytesWritten Number of bytes written thus far.
     * @constant kStatisticsTotalWriteTime Nanoseconds spent performing writes thus far.
     * @constant kStatisticsLatentWriteTime Nanoseconds of latency during writes thus far.
     * @constant kStatisticsWriteRetries Number of write retries thus far.
     * @constant kStatisticsWriteErrors Number of write errors thus far.
     */

    enum Statistics
    {
        kStatisticsReads,
        kStatisticsBytesRead,
        kStatisticsTotalReadTime,
        kStatisticsLatentReadTime,
        kStatisticsReadRetries,
        kStatisticsReadErrors,

        kStatisticsWrites,
        kStatisticsSingleBlockWrites,
        kStatisticsBytesWritten,
        kStatisticsTotalWriteTime,
        kStatisticsLatentWriteTime,
        kStatisticsWriteRetries,
        kStatisticsWriteErrors
    };

    static const UInt32 kStatisticsCount = kStatisticsWriteErrors + 1;

protected:

    struct Context;

    struct ExpansionData
    {
        UInt64         reserved0000;
        UInt64         maxReadBlockTransfer;
        UInt64         maxWriteBlockTransfer;
        IONotifier *   powerEventNotifier;
        UInt32         deblockRequestWriteLockCount;
        UInt64         maxReadSegmentTransfer;
        UInt64         maxWriteSegmentTransfer;
        UInt64         maxReadSegmentByteTransfer;
        UInt64         maxWriteSegmentByteTransfer;
        UInt64         minSegmentAlignmentByteTransfer;
        UInt64         maxSegmentWidthByteTransfer;
        Context *      contexts;
        IOSimpleLock * contextsLock;
        UInt32         contextsCount;
        UInt32         contextsMaxCount;
    };
    ExpansionData * _expansionData;

    #define _maxReadBlockTransfer            \
              IOBlockStorageDriver::_expansionData->maxReadBlockTransfer
    #define _maxWriteBlockTransfer           \
              IOBlockStorageDriver::_expansionData->maxWriteBlockTransfer
    #define _powerEventNotifier              \
              IOBlockStorageDriver::_expansionData->powerEventNotifier
    #define _deblockRequestWriteLockCount    \
              IOBlockStorageDriver::_expansionData->deblockRequestWriteLockCount
    #define _maxReadSegmentTransfer          \
              IOBlockStorageDriver::_expansionData->maxReadSegmentTransfer
    #define _maxWriteSegmentTransfer         \
              IOBlockStorageDriver::_expansionData->maxWriteSegmentTransfer
    #define _maxReadSegmentByteTransfer      \
              IOBlockStorageDriver::_expansionData->maxReadSegmentByteTransfer
    #define _maxWriteSegmentByteTransfer     \
              IOBlockStorageDriver::_expansionData->maxWriteSegmentByteTransfer
    #define _minSegmentAlignmentByteTransfer \
              IOBlockStorageDriver::_expansionData->minSegmentAlignmentByteTransfer
    #define _maxSegmentWidthByteTransfer     \
              IOBlockStorageDriver::_expansionData->maxSegmentWidthByteTransfer
    #define _contexts                        \
              IOBlockStorageDriver::_expansionData->contexts
    #define _contextsLock                    \
              IOBlockStorageDriver::_expansionData->contextsLock
    #define _contextsCount                   \
              IOBlockStorageDriver::_expansionData->contextsCount
    #define _contextsMaxCount                \
              IOBlockStorageDriver::_expansionData->contextsMaxCount

    OSSet *         _openClients;
    OSNumber *      _statistics[kStatisticsCount];

    /*
     * @struct Context
     * @discussion
     * Context structure for a read/write operation.  It describes the block size,
     * and where applicable, a block type and block sub-type, for a data transfer,
     * as well as the completion information for the original request.  Note that
     * the block type field is unused in the IOBlockStorageDriver class.
     * @field block.size
     * Block size for the operation.
     * @field block.type
     * Block type for the operation.  Unused in IOBlockStorageDriver.  The default
     * value for this field is IOBlockStorageDriver::kBlockTypeStandard.
     * @field block.typeSub
     * Block sub-type for the operation.  It's definition depends on block.type.
     * Unused in IOBlockStorageDriver.
     * @field request.byteStart
     * Starting byte offset for the data transfer.
     * @param request.buffer
     * Buffer for the data transfer.  The size of the buffer implies the size of
     * the data transfer.
     * @param request.attributes
     * Attributes of the data transfer.  See IOStorageAttributes.
     * @param request.completion
     * Completion routine to call once the data transfer is complete.
     */

    struct Context
    {
        struct
        {
            UInt64               byteStart;
            IOMemoryDescriptor * buffer;
            IOStorageAttributes  attributes;
            IOStorageCompletion  completion;
        } request;

        struct
        {
            UInt32               size;
            UInt8                type;
            UInt8                typeSub[3];
        } block;

        AbsoluteTime timeStart;

        UInt64 reserved0704;
        UInt64 reserved0768;
        UInt64 reserved0832;
        UInt64 reserved0896;

        Context * next;
    };

    static const UInt8 kBlockTypeStandard = 0x00;

    using IOService::open;

    /*
     * Free all of this object's outstanding resources.
     *
     * This method's implementation is not typically overridden.
     */

    void free() APPLE_KEXT_OVERRIDE;

    /*!
     * @function handleOpen
     * @discussion
     * The handleOpen method grants or denies permission to access this object
     * to an interested client.  The argument is an IOStorageAccess value that
     * specifies the level of access desired -- reader or reader-writer.
     *
     * This method can be invoked to upgrade or downgrade the access level for
     * an existing client as well.  The previous access level will prevail for
     * upgrades that fail, of course.   A downgrade should never fail.  If the
     * new access level should be the same as the old for a given client, this
     * method will do nothing and return success.  In all cases, one, singular
     * close-per-client is expected for all opens-per-client received.
     *
     * This implementation replaces the IOService definition of handleIsOpen().
     * @param client
     * Client requesting the open.
     * @param options
     * Options for the open.  Set to zero.
     * @param access
     * Access level for the open.  Set to kIOStorageAccessReader or
     * kIOStorageAccessReaderWriter.
     * @result
     * Returns true if the open was successful, false otherwise.
     */

    virtual bool handleOpen(IOService *  client,
                            IOOptionBits options,
                            void *       access) APPLE_KEXT_OVERRIDE;

    /*!
     * @function handleIsOpen
     * @discussion
     * The handleIsOpen method determines whether the specified client, or any
     * client if none is specified, presently has an open on this object.
     *
     * This implementation replaces the IOService definition of handleIsOpen().
     * @param client
     * Client to check the open state of.  Set to zero to check the open state
     * of all clients.
     * @result
     * Returns true if the client was (or clients were) open, false otherwise.
     */

    virtual bool handleIsOpen(const IOService * client) const APPLE_KEXT_OVERRIDE;

    /*!
     * @function handleClose
     * @discussion
     * The handleClose method closes the client's access to this object.
     *
     * This implementation replaces the IOService definition of handleIsOpen().
     * @param client
     * Client requesting the close.
     * @param options
     * Options for the close.  Set to zero.
     */

    virtual void handleClose(IOService * client, IOOptionBits options) APPLE_KEXT_OVERRIDE;

    /*!
     * @function addToBytesTransferred
     * @discussion
     * Update the total number of bytes transferred, the total transfer time,
     * and the total latency time -- used for statistics.
     *
     * This method's implementation is not typically overridden.
     * @param bytesTransferred
     * Number of bytes transferred in this operation.
     * @param totalTime
     * Nanoseconds spent performing this operation.
     * @param latentTime
     * Nanoseconds of latency during this operation.
     * @param isWrite
     * Indicates whether this operation was a write, otherwise is was a read.
     */

    virtual void addToBytesTransferred(UInt64 bytesTransferred,
                                       UInt64 totalTime,
                                       UInt64 latentTime,
                                       bool   isWrite);

    /*!
     * @function incrementErrors
     * @discussion
     * Update the total error count -- used for statistics.
     *
     * This method's implementation is not typically overridden.
     * @param isWrite
     * Indicates whether this operation was a write, otherwise is was a read.
     */

    virtual void incrementErrors(bool isWrite);

    /*!
     * @function incrementRetries
     * @discussion
     * Update the total retry count -- used for statistics.
     *
     * This method's implementation is not typically overridden.
     * @param isWrite
     * Indicates whether this operation was a write, otherwise is was a read.
     */

    virtual void incrementRetries(bool isWrite);

    /*!
     * @function allocateContext
     * @discussion
     * Allocate a context structure for a read/write operation.
     * @result
     * Context structure.
     */

    virtual Context * allocateContext();

    /*!
     * @function deleteContext
     * @discussion
     * Delete a context structure from a read/write operation.
     * @param context
     * Context structure to be deleted.
     */

    virtual void deleteContext(Context * context);

    /*!
     * @function deblockRequest
     * @discussion
     * The deblockRequest method checks to see if the incoming request rests
     * on the media's block boundaries, and if not, deblocks it.  Deblocking
     * involves rounding out the request to the nearest block boundaries and
     * transferring the excess bytes into a scratch buffer.
     *
     * This method is part of a sequence of methods invoked for each read/write
     * request.  The first is prepareRequest, which allocates and prepares some
     * context for the transfer; the second is deblockRequest, which aligns the
     * transfer at the media's block boundaries; third is breakUpRequest, which
     * breaks up the transfer into multiple sub-transfers when certain hardware
     * constraints are exceeded; fourth is executeRequest, which implements the
     * actual transfer from the block storage device.
     *
     * This method's implementation is not typically overridden.
     * @param byteStart
     * Starting byte offset for the data transfer.
     * @param buffer
     * Buffer for the data transfer.  The size of the buffer implies the size of
     * the data transfer.
     * @param attributes
     * Attributes of the data transfer.  See IOStorageAttributes.  It is the
     * responsibility of the callee to maintain the information for the duration
     * of the data transfer, as necessary.
     * @param completion
     * Completion routine to call once the data transfer is complete.  It is the
     * responsibility of the callee to maintain the information for the duration
     * of the data transfer, as necessary.
     * @param context
     * Additional context information for the data transfer (e.g. block size).
     */

    virtual void deblockRequest(UInt64                byteStart,
                                IOMemoryDescriptor *  buffer,
                                IOStorageAttributes * attributes,
                                IOStorageCompletion * completion,
                                Context *             context);

    /*!
     * @function executeRequest
     * @discussion
     * Execute an asynchronous storage request.  The request is guaranteed to be
     * block-aligned.
     *
     * This method is part of a sequence of methods invoked for each read/write
     * request.  The first is prepareRequest, which allocates and prepares some
     * context for the transfer; the second is deblockRequest, which aligns the
     * transfer at the media's block boundaries; third is breakUpRequest, which
     * breaks up the transfer into multiple sub-transfers when certain hardware
     * constraints are exceeded; fourth is executeRequest, which implements the
     * actual transfer from the block storage device.
     * @param byteStart
     * Starting byte offset for the data transfer.
     * @param buffer
     * Buffer for the data transfer.  The size of the buffer implies the size of
     * the data transfer.
     * @param attributes
     * Attributes of the data transfer.  See IOStorageAttributes.  It is the
     * responsibility of the callee to maintain the information for the duration
     * of the data transfer, as necessary.
     * @param completion
     * Completion routine to call once the data transfer is complete.  It is the
     * responsibility of the callee to maintain the information for the duration
     * of the data transfer, as necessary.
     * @param context
     * Additional context information for the data transfer (e.g. block size).
     */

    virtual void executeRequest(UInt64                byteStart,
                                IOMemoryDescriptor *  buffer,
                                IOStorageAttributes * attributes,
                                IOStorageCompletion * completion,
                                Context *             context);

    /*!
     * @function handleStart
     * @discussion
     * Prepare the block storage driver for operation.
     *
     * This is where a media object needs to be created for fixed media, and
     * optionally for removable media.
     *
     * Note that this method is called from within the start() routine;
     * if this method returns successfully,  it should be prepared to accept
     * any of IOBlockStorageDriver's APIs.
     * @param provider
     * This object's provider.
     * @result
     * Returns true on success, false otherwise.
     */

    virtual bool handleStart(IOService * provider);

#if TARGET_OS_OSX && defined(__x86_64__)
    virtual bool handleYield(IOService *  provider,
                             IOOptionBits options  = 0,
                             void *       argument = 0) __attribute__ ((deprecated));
#endif /* TARGET_OS_OSX && defined(__x86_64__) */

    /*!
     * @function getMediaBlockSize
     * @discussion
     * Ask the driver about the media's natural block size.
     * @result
     * Natural block size, in bytes.
     */

    virtual UInt64 getMediaBlockSize() const;

public:

    using IOStorage::open;
    using IOStorage::read;
    using IOStorage::write;

    /*
     * Initialize this object's minimal state.
     *
     * This method's implementation is not typically overridden.
     */

    virtual bool init(OSDictionary * properties = 0) APPLE_KEXT_OVERRIDE;

    /*
     * This method is called once we have been attached to the provider object.
     *
     * This method's implementation is not typically overridden.
     */

    virtual bool start(IOService * provider) APPLE_KEXT_OVERRIDE;

    /*
     * This method is called before we are detached from the provider object.
     *
     * This method's implementation is not typically overridden.
     */

    virtual void stop(IOService * provider) APPLE_KEXT_OVERRIDE;

    virtual bool didTerminate(IOService *  provider,
                              IOOptionBits options,
                              bool *       defer) APPLE_KEXT_OVERRIDE;

#if TARGET_OS_OSX && defined(__x86_64__)
    virtual bool yield(IOService *  provider,
                       IOOptionBits options  = 0,
                       void *       argument = 0) __attribute__ ((deprecated));
#endif /* TARGET_OS_OSX && defined(__x86_64__) */

    /*!
     * @function read
     * @discussion
     * The read method is the receiving end for all read requests from the
     * storage framework (through the media object created by this driver).
     *
     * This method initiates a sequence of methods (stages) for each read/write
     * request.  The first is prepareRequest, which allocates and prepares some
     * context for the transfer; the second is deblockRequest, which aligns the
     * transfer at the media's block boundaries; third is breakUpRequest, which
     * breaks up the transfer into multiple sub-transfers when certain hardware
     * constraints are exceeded; fourth is executeRequest, which implements the
     * actual transfer from the block storage device.
     *
     * This method's implementation is not typically overridden.
     * @param client
     * Client requesting the read.
     * @param byteStart
     * Starting byte offset for the data transfer.
     * @param buffer
     * Buffer for the data transfer.  The size of the buffer implies the size of
     * the data transfer.
     * @param attributes
     * Attributes of the data transfer.  See IOStorageAttributes.  It is the
     * responsibility of the callee to maintain the information for the duration
     * of the data transfer, as necessary.
     * @param completion
     * Completion routine to call once the data transfer is complete.  It is the
     * responsibility of the callee to maintain the information for the duration
     * of the data transfer, as necessary.
     */

    virtual void read(IOService *           client,
                      UInt64                byteStart,
                      IOMemoryDescriptor *  buffer,
                      IOStorageAttributes * attributes,
                      IOStorageCompletion * completion) APPLE_KEXT_OVERRIDE;

    /*!
     * @function write
     * @discussion
     * The write method is the receiving end for all write requests from the
     * storage framework (through the media object created by this driver).
     *
     * This method initiates a sequence of methods (stages) for each read/write
     * request.  The first is prepareRequest, which allocates and prepares some
     * context for the transfer; the second is deblockRequest, which aligns the
     * transfer at the media's block boundaries; third is breakUpRequest, which
     * breaks up the transfer into multiple sub-transfers when certain hardware
     * constraints are exceeded; fourth is executeRequest, which implements the
     * actual transfer from the block storage device.
     *
     * This method's implementation is not typically overridden.
     * @param client
     * Client requesting the write.
     * @param byteStart
     * Starting byte offset for the data transfer.
     * @param buffer
     * Buffer for the data transfer.  The size of the buffer implies the size of
     * the data transfer.
     * @param attributes
     * Attributes of the data transfer.  See IOStorageAttributes.  It is the
     * responsibility of the callee to maintain the information for the duration
     * of the data transfer, as necessary.
     * @param completion
     * Completion routine to call once the data transfer is complete.  It is the
     * responsibility of the callee to maintain the information for the duration
     * of the data transfer, as necessary.
     */

    virtual void write(IOService *           client,
                       UInt64                byteStart,
                       IOMemoryDescriptor *  buffer,
                       IOStorageAttributes * attributes,
                       IOStorageCompletion * completion) APPLE_KEXT_OVERRIDE;

    /*!
     * @function synchronize
     * @discussion
     * Flush the cached data in the storage object, if any.
     * @param client
     * Client requesting the synchronization.
     * @param byteStart
     * Starting byte offset for the synchronization.
     * @param byteCount
     * Size of the synchronization.  Set to zero to specify the end-of-media.
     * @param options
     * Options for the synchronization.  See IOStorageSynchronizeOptions.
     * @result
     * Returns the status of the synchronization.
     */

    virtual IOReturn synchronize(IOService *                 client,
                                 UInt64                      byteStart,
                                 UInt64                      byteCount,
                                 IOStorageSynchronizeOptions options = 0) APPLE_KEXT_OVERRIDE;

    /*!
     * @function unmap
     * @discussion
     * Delete unused data from the storage object at the specified byte offsets.
     * @param client
     * Client requesting the operation.
     * @param extents
     * List of extents.  See IOStorageExtent.  It is legal for the callee to
     * overwrite the contents of this buffer in order to satisfy the request.
     * @param extentsCount
     * Number of extents.
     * @param options
     * Options for the unmap.  See IOStorageUnmapOptions.
     * @result
     * Returns the status of the operation.
     */

    virtual IOReturn unmap(IOService *           client,
                           IOStorageExtent *     extents,
                           UInt32                extentsCount,
                           IOStorageUnmapOptions options = 0) APPLE_KEXT_OVERRIDE;

    /*!
     * @function getProvisionStatus
     * @discussion
     * Get device block provision status
     * @param client
     * Client requesting the synchronization.
     * @param byteStart
     * Byte offset of logical extent on the device.
     * @param byteCount
     * Byte length of logical extent on the device, 0 mean the entire remaining space.
     * @param extentsCount
     * Number of extents allocated in extents. On return, this parameter indicate number
     * of provision extents returned.
     * @param extents
     * List of provision extents. See IOStorageProvisionExtents.
     * @result
     * Returns the status of the getProvisionStatus.
     */

    virtual IOReturn getProvisionStatus(IOService *                           client,
                                        UInt64                                byteStart,
                                        UInt64                                byteCount,
                                        UInt32 *                              extentsCount,
                                        IOStorageProvisionExtent *            extents,
                                        IOStorageGetProvisionStatusOptions    options = 0) APPLE_KEXT_OVERRIDE;

    /*!
     * @function lockPhysicalExtents
     * @discussion
     * Lock the contents of the storage object against relocation temporarily,
     * for the purpose of getting physical extents.
     * @param client
     * Client requesting the operation.
     * @result
     * Returns true if the lock was successful, false otherwise.
     */

    virtual bool lockPhysicalExtents(IOService * client) APPLE_KEXT_OVERRIDE;

    /*!
     * @function copyPhysicalExtent
     * @discussion
     * Convert the specified byte offset into a physical byte offset, relative
     * to a physical storage object.  This call should only be made within the
     * context of lockPhysicalExtents().
     * @param client
     * Client requesting the operation.
     * @param byteStart
     * Starting byte offset for the operation.  Returns a physical byte offset,
     * relative to the physical storage object, on success.
     * @param byteCount
     * Size of the operation.  Returns the actual number of bytes which can be
     * transferred, relative to the physical storage object, on success. 
     * @result
     * A reference to the physical storage object, which should be released by
     * the caller, or a null on error.
     */

    virtual IOStorage * copyPhysicalExtent(IOService * client,
                                           UInt64 *    byteStart,
                                           UInt64 *    byteCount) APPLE_KEXT_OVERRIDE;

    /*!
     * @function unlockPhysicalExtents
     * @discussion
     * Unlock the contents of the storage object for relocation again.  This
     * call must balance a successful call to lockPhysicalExtents().
     * @param client
     * Client requesting the operation.
     */

    virtual void unlockPhysicalExtents(IOService * client) APPLE_KEXT_OVERRIDE;

    /*!
     * @function setPriority
     * @discussion
     * Reprioritize read or write requests at the specified byte offsets.
     * @param client
     * Client requesting the operation.
     * @param extents
     * List of extents.  See IOStorageExtent.  It is legal for the callee to
     * overwrite the contents of this buffer in order to satisfy the request.
     * @param extentsCount
     * Number of extents.
     * @param priority
     * New priority.  See IOStoragePriority.
     * @result
     * Returns the status of the operation.
     */

    virtual IOReturn setPriority(IOService *       client,
                                 IOStorageExtent * extents,
                                 UInt32            extentsCount,
                                 IOStoragePriority priority) APPLE_KEXT_OVERRIDE;

    /*!
     * @function ejectMedia
     * @discussion
     * Eject the media from the device.  The driver is responsible for tearing
     * down the media object it created before proceeding with the eject.   If
     * the tear-down fails, an error should be returned.
     * @result
     * An IOReturn code.
     */

    virtual IOReturn ejectMedia();

    /*!
     * @function formatMedia
     * @discussion
     * Format the media with the specified byte capacity.  The driver is
     * responsible for tearing down the media object and recreating it.
     * @param byteCapacity
     * Number of bytes to format media to.
     * @result
     * An IOReturn code.
     */

    virtual IOReturn formatMedia(UInt64 byteCapacity);

#if TARGET_OS_OSX && defined(__x86_64__)
    virtual IOReturn lockMedia(bool lock) __attribute__ ((deprecated));

    virtual IOReturn pollMedia() __attribute__ ((deprecated));
#endif /* TARGET_OS_OSX && defined(__x86_64__) */

    /*!
     * @function isMediaEjectable
     * @discussion
     * Ask the driver whether the media is ejectable.
     * @result
     * Returns true if the media is ejectable, false otherwise.
     */

    virtual bool isMediaEjectable() const;

    /*!
     * @function isMediaRemovable
     * @discussion
     * Ask the driver whether the media is ejectable.
     * @result
     * Returns true if the media is ejectable, false otherwise.
     */

    virtual bool isMediaRemovable() const;

#if TARGET_OS_OSX && defined(__x86_64__)
    virtual bool isMediaPollExpensive() const __attribute__ ((deprecated));

    virtual bool isMediaPollRequired() const __attribute__ ((deprecated));
#endif /* TARGET_OS_OSX && defined(__x86_64__) */

    /*!
     * @function isMediaWritable
     * @discussion
     * Ask the driver whether the media is writable.
     * @result
     * Returns true if the media is writable, false otherwise.
     */

    virtual bool isMediaWritable() const;

    /*!
     * @function getMediaState
     * @discussion
     * Ask the driver about the media's current state.
     * @result
     * An IOMediaState value.
     */

    virtual IOMediaState getMediaState() const;

    /*!
     * @function getFormatCapacities
     * @discussion
     * Ask the driver to report the feasible formatting capacities for the
     * inserted media (in bytes).  This routine fills the caller's buffer,
     * up to the maximum count specified if the real number of capacities
     * would overflow the buffer.   The return value indicates the actual
     * number of capacities copied to the buffer.
     *
     * If the capacities buffer is not supplied or if the maximum count is
     * zero, the routine returns the proposed count of capacities instead.
     * @param capacities
     * Buffer that will receive the UInt64 capacity values.
     * @param capacitiesMaxCount
     * Maximum number of capacity values that can be held in the buffer.
     * @result
     * Actual number of capacity values copied to the buffer, or if no buffer
     * is given, the total number of capacity values available.
     */

    virtual UInt32 getFormatCapacities(UInt64 * capacities,
                                       UInt32   capacitiesMaxCount) const;

    /*!
     * @function getStatistics
     * @discussion
     * Ask the driver to report its operating statistics.
     *
     * The statistics are each indexed by IOBlockStorageDriver::Statistics
     * indices.  This routine fills the caller's buffer, up to the maximum
     * count specified if the real number of statistics would overflow the
     * buffer.  The return value indicates the actual number of statistics
     * copied to the buffer.
     *
     * If the statistics buffer is not supplied or if the maximum count is
     * zero, the routine returns the proposed count of statistics instead.
     * @param statistics
     * Buffer that will receive the UInt64 statistic values.
     * @param statisticsMaxCount
     * Maximum number of statistic values that can be held in the buffer.
     * @result
     * Actual number of statistic values copied to the buffer, or if no buffer
     * is given, the total number of statistic values available.
     */

    virtual UInt32 getStatistics(UInt64 * statistics,
                                 UInt32   statisticsMaxCount) const;

    /*!
     * @function getStatistic
     * @discussion
     * Ask the driver to report one of its operating statistics.
     * @param statistic
     * Statistic index (an IOBlockStorageDriver::Statistics index).
     * @result
     * Statistic value.
     */

    virtual UInt64 getStatistic(Statistics statistic) const;

    /*
     * Generic entry point for calls from the provider.  A return value of
     * kIOReturnSuccess indicates that the message was received, and where
     * applicable, that it was successful.
     */

    virtual IOReturn message(UInt32 type, IOService * provider, void * argument) APPLE_KEXT_OVERRIDE;

    /*
     * Obtain this object's provider.  We override the superclass's method to
     * return a more specific subclass of IOService -- IOBlockStorageDevice.  
     * This method serves simply as a convenience to subclass developers.
     */

    virtual IOBlockStorageDevice * getProvider() const APPLE_KEXT_OVERRIDE;

protected:

    IOLock *      _deblockRequestWriteLock;

    UInt64        _reserved1024;

    static void breakUpRequestExecute(void * parameter, void * target);

    static void deblockRequestExecute(void * parameter, void * target);

    /*
     * This is the completion routine for the broken up breaker sub-requests.
     * It verifies the success of the just-completed stage,  transitions to
     * the next stage, then builds and issues a transfer for the next stage.
     */

    static void breakUpRequestCompletion(void *   target,
                                         void *   parameter,
                                         IOReturn status,
                                         UInt64   actualByteCount);

    /*
     * This is the completion routine for the aligned deblocker sub-requests.
     * It verifies the success of the just-completed stage,  transitions to
     * the next stage, then builds and issues a transfer for the next stage.
     */

    static void deblockRequestCompletion(void *   target,
                                         void *   parameter,
                                         IOReturn status,
                                         UInt64   actualByteCount);

    /*
     * This is the completion routine for the prepared request.  It updates
     * the driver's statistics, performs some clean up work, then calls the
     * original request's completion routine.
     */

    static void prepareRequestCompletion(void *   target,
                                         void *   parameter,
                                         IOReturn status,
                                         UInt64   actualByteCount);

#if TARGET_OS_OSX && defined(__x86_64__)
    virtual void schedulePoller() __attribute__ ((deprecated));

    virtual void unschedulePoller() __attribute__ ((deprecated));
#endif /* TARGET_OS_OSX && defined(__x86_64__) */

    /*
     * This method is the power event handler for restarts and shutdowns.
     */

    static IOReturn handlePowerEvent(void *      target,
                                     void *      parameter,
                                     UInt32      messageType,
                                     IOService * provider,
                                     void *      messageArgument,
                                     vm_size_t   messageArgumentSize);

protected:

    /* Device info: */

    /*!
     * @var _removable
     * True if the media is removable; False if it is fixed (not removable).
     */
    bool		_removable;

    /*!
     * @var _ejectable
     * True if the media is ejectable under software control.
     */
    bool		_ejectable;		/* software-ejectable */

    UInt16		_reserved1104;

    UInt32		_openAssertions;

    /* Media info and states: */

    /*!
     * @var _mediaObject
     * A pointer to the media object we have instantiated (if any).
     */
    IOMedia *		_mediaObject;

    /*!
     * @var _mediaType
     * Type of the media (can be used to differentiate between the
     * different types of CD media, DVD media, etc).
     */
    UInt32		_mediaType;

    bool		_solidState;

    /*!
     * @var _writeProtected
     * True if the media is write-protected; False if not.
     */
    bool		_writeProtected;

    UInt16		_reserved1264;
    UInt64		_reserved1280;

    /*!
     * @var _mediaBlockSize
     * The block size of the media, in bytes.
     */
    UInt64		_mediaBlockSize;

    /*!
     * @var _maxBlockNumber
     * The maximum allowable block number for the media, zero-based.
     */
    UInt64		_maxBlockNumber;

    /*!
     * @var _maxReadByteTransfer
     * The maximum byte transfer allowed for read operations.
     */
    UInt64		_maxReadByteTransfer;

    /*!
     * @var _maxWriteByteTransfer
     * The maximum byte transfer allowed for write operations.
     */
    UInt64		_maxWriteByteTransfer;

    /*!
     * @function acceptNewMedia
     * @abstract
     * React to new media insertion.
     * @discussion
     * This method logs the media block size and block count, then calls
     * instantiateMediaObject to get a media object instantiated. The
     * media object is then attached above us and registered.
     * 
     * This method can be overridden to control what happens when new media
     * is inserted. The default implementation deals with one IOMedia object.
     */
    virtual IOReturn	acceptNewMedia(void);
    
#if TARGET_OS_OSX && defined(__x86_64__)
    virtual UInt64	constrainByteCount(UInt64 requestedCount,bool isWrite) __attribute__ ((deprecated));
#endif /* TARGET_OS_OSX && defined(__x86_64__) */

    /*!
     * @function decommissionMedia
     * @abstract
     * Decommission an existing piece of media that has gone away.
     * @discussion
     * This method wraps a call to terminate, to tear down the stack and
     * the IOMedia object for the media. If "forcible" is true, the media
     * object will be forgotten, and initMediaState will be called. A
     * forcible decommission would occur when an unrecoverable error
     * happens during tear-down (e.g. perhaps a client is still open), but
     * we must still forget about the media.
     * @param forcible
     * True to force forgetting of the media object even if terminate reports
     * that there was an active client.
     */
    virtual IOReturn	decommissionMedia(bool forcible);

    /*!
     * @function instantiateDesiredMediaObject
     * @abstract
     * Create an IOMedia object for media.
     * @discussion
     * This method creates the exact type of IOMedia object desired. It is called by
     * instantiateMediaObject. A subclass may override this one-line method to change
     * the type of media object actually instantiated.
     */
    virtual IOMedia *	instantiateDesiredMediaObject(void);

    /*!
     * @function instantiateMediaObject
     * @abstract
     * Create an IOMedia object for media.
     * @discussion
     * This method creates an IOMedia object from the supplied parameters. It is a
     * convenience method to wrap the handful of steps to do the job.
     * @param base
     * Byte number of beginning of active data area of the media. Usually zero.
     * @param byteSize
     * Size of the data area of the media, in bytes.
     * @param blockSize
     * Block size of the media, in bytes.
     * @param mediaName
     * Name of the IOMedia object.
     * @result
     * A pointer to the created IOMedia object, or a null on error.
     */
    virtual IOMedia *	instantiateMediaObject(UInt64 base,UInt64 byteSize,
                                            UInt32 blockSize,char *mediaName);

    /*!
     * @function recordMediaParameters
     * @abstract
     * Obtain media-related parameters on media insertion.
     * @discussion
     * This method obtains media-related parameters via calls to the
     * Transport Driver's reportBlockSize, reportMaxValidBlock,
     * and reportWriteProtection methods.
     */
    virtual IOReturn	recordMediaParameters(void);

    /*!
     * @function rejectMedia
     * @abstract
     * Reject new media.
     * @discussion
     * This method will be called if validateNewMedia returns False (thus rejecting
     * the new media. A vendor may choose to override this method to control behavior
     * when media is rejected.
     * 
     * The default implementation simply calls ejectMedia.
     */
    virtual void	rejectMedia(void);	/* default ejects */
    
    /*!
     * @function validateNewMedia
     * @abstract
     * Verify that new media is acceptable.
     * @discussion
     * This method will be called whenever new media is detected. Return true to accept
     * the media, or false to reject it (and call rejectMedia). Vendors might override
     * this method to handle password-protection for new media.
     * 
     * The default implementation always returns True, indicating media is accepted.
     */
    virtual bool	validateNewMedia(void);

    /* --- Internally used methods. --- */

    /*
     * @group
     * Internally Used Methods
     * @discussion
     * These methods are used internally, and will not generally be modified.
     */
    
    /*!
     * @function checkForMedia
     * @abstract
     * Check if media has newly arrived or disappeared.
     * @discussion
     * This method does most of the work in polling for media, first
     * calling the block storage device's reportMediaState method. If
     * reportMediaState reports no change in the media state, kIOReturnSuccess
     * is returned. If the media state has indeed changed, a call is made to
     * mediaStateHasChanged to act on the event.
     */
    virtual IOReturn	checkForMedia(void);

    /*!
     * @function getDeviceTypeName
     * @abstract
     * Return the desired device name.
     * @discussion
     * This method returns a string, used to compare the 
     * kIOBlockStorageDeviceTypeKey of our provider. This method is called from
     * probe.
     *  
     * The default implementation of this method returns 
     * kIOBlockStorageDeviceTypeGeneric.
     */
    virtual const char * getDeviceTypeName(void);

    /*!
     * @function initMediaState
     * @abstract
     * Initialize media-related instance variables.
     * @discussion
     * Called when media is not present, this method marks the device state
     * as not having media present, not spun up, and write-enabled.
     */
    virtual void	initMediaState(void);
    
    /*!
     * @function mediaStateHasChanged
     * @abstract
     * React to a new media insertion or a media removal.
     * @discussion
     * This method is called on a media state change, that is, an arrival
     * or removal. If media has just become available, calls are made to
     * recordMediaParameters and acceptNewMedia. If media has just gone
     * away, a call is made to decommissionMedia, with the forcible
     * parameter set to true. The forcible tear-down is needed to enforce
     * the disappearance of media, regardless of interested clients.
     */
    virtual IOReturn	mediaStateHasChanged(IOMediaState state);

    /*
     * @endgroup
     */

protected:

    /*!
     * @function breakUpRequest
     * @discussion
     * The breakUpRequest method checks to see if the incoming request exceeds
     * our transfer constraints, and if so, breaks up the request into smaller
     * sub-requests.
     *
     * This method is part of a sequence of methods invoked for each read/write
     * request.  The first is prepareRequest, which allocates and prepares some
     * context for the transfer; the second is deblockRequest, which aligns the
     * transfer at the media's block boundaries; third is breakUpRequest, which
     * breaks up the transfer into multiple sub-transfers when certain hardware
     * constraints are exceeded; fourth is executeRequest, which implements the
     * actual transfer from the block storage device.
     *
     * This method's implementation is not typically overridden.
     * @param byteStart
     * Starting byte offset for the data transfer.
     * @param buffer
     * Buffer for the data transfer.  The size of the buffer implies the size of
     * the data transfer.
     * @param attributes
     * Attributes of the data transfer.  See IOStorageAttributes.  It is the
     * responsibility of the callee to maintain the information for the duration
     * of the data transfer, as necessary.
     * @param completion
     * Completion routine to call once the data transfer is complete.  It is the
     * responsibility of the callee to maintain the information for the duration
     * of the data transfer, as necessary.
     * @param context
     * Additional context information for the data transfer (e.g. block size).
     */

    virtual void breakUpRequest(UInt64                byteStart,
                                IOMemoryDescriptor *  buffer,
                                IOStorageAttributes * attributes,
                                IOStorageCompletion * completion,
                                Context *             context);

    /*!
     * @function prepareRequest
     * @discussion
     * The prepareRequest method allocates and prepares state for the transfer.
     *
     * This method is part of a sequence of methods invoked for each read/write
     * request.  The first is prepareRequest, which allocates and prepares some
     * context for the transfer; the second is deblockRequest, which aligns the
     * transfer at the media's block boundaries; third is breakUpRequest, which
     * breaks up the transfer into multiple sub-transfers when certain hardware
     * constraints are exceeded; fourth is executeRequest, which implements the
     * actual transfer from the block storage device.
     *
     * This method's implementation is not typically overridden.
     * @param byteStart
     * Starting byte offset for the data transfer.
     * @param buffer
     * Buffer for the data transfer.  The size of the buffer implies the size of
     * the data transfer.
     * @param attributes
     * Attributes of the data transfer.  See IOStorageAttributes.  It is the
     * responsibility of the callee to maintain the information for the duration
     * of the data transfer, as necessary.
     * @param completion
     * Completion routine to call once the data transfer is complete.  It is the
     * responsibility of the callee to maintain the information for the duration
     * of the data transfer, as necessary.
     */

    virtual void prepareRequest(UInt64                byteStart,
                                IOMemoryDescriptor *  buffer,
                                IOStorageAttributes * attributes,
                                IOStorageCompletion * completion);

public:

    /*!
     * @function requestIdle
     * @abstract
     * Request that the device enter an idle state.
     * @discussion
     * Request that the device enter an idle state.  The device will exit this state on the
     * next read or write request, or as it sees necessary.  One example is for a DVD drive
     * to spin down when it enters such an idle state, and spin up on the next read request
     * from the system.
     */
    virtual IOReturn	requestIdle(void);

    OSMetaClassDeclareReservedUnused(IOBlockStorageDriver,  0);
    OSMetaClassDeclareReservedUnused(IOBlockStorageDriver,  1);
    OSMetaClassDeclareReservedUnused(IOBlockStorageDriver,  2);
    OSMetaClassDeclareReservedUnused(IOBlockStorageDriver,  3);
    OSMetaClassDeclareReservedUnused(IOBlockStorageDriver,  4);
    OSMetaClassDeclareReservedUnused(IOBlockStorageDriver,  5);
    OSMetaClassDeclareReservedUnused(IOBlockStorageDriver,  6);
    OSMetaClassDeclareReservedUnused(IOBlockStorageDriver,  7);
    OSMetaClassDeclareReservedUnused(IOBlockStorageDriver,  8);
    OSMetaClassDeclareReservedUnused(IOBlockStorageDriver,  9);
    OSMetaClassDeclareReservedUnused(IOBlockStorageDriver, 10);
    OSMetaClassDeclareReservedUnused(IOBlockStorageDriver, 11);
    OSMetaClassDeclareReservedUnused(IOBlockStorageDriver, 12);
    OSMetaClassDeclareReservedUnused(IOBlockStorageDriver, 13);
    OSMetaClassDeclareReservedUnused(IOBlockStorageDriver, 14);
    OSMetaClassDeclareReservedUnused(IOBlockStorageDriver, 15);
    OSMetaClassDeclareReservedUnused(IOBlockStorageDriver, 16);
    OSMetaClassDeclareReservedUnused(IOBlockStorageDriver, 17);
    OSMetaClassDeclareReservedUnused(IOBlockStorageDriver, 18);
    OSMetaClassDeclareReservedUnused(IOBlockStorageDriver, 19);
    OSMetaClassDeclareReservedUnused(IOBlockStorageDriver, 20);
    OSMetaClassDeclareReservedUnused(IOBlockStorageDriver, 21);
    OSMetaClassDeclareReservedUnused(IOBlockStorageDriver, 22);
    OSMetaClassDeclareReservedUnused(IOBlockStorageDriver, 23);
    OSMetaClassDeclareReservedUnused(IOBlockStorageDriver, 24);
    OSMetaClassDeclareReservedUnused(IOBlockStorageDriver, 25);
    OSMetaClassDeclareReservedUnused(IOBlockStorageDriver, 26);
    OSMetaClassDeclareReservedUnused(IOBlockStorageDriver, 27);
    OSMetaClassDeclareReservedUnused(IOBlockStorageDriver, 28);
    OSMetaClassDeclareReservedUnused(IOBlockStorageDriver, 29);
    OSMetaClassDeclareReservedUnused(IOBlockStorageDriver, 30);
    OSMetaClassDeclareReservedUnused(IOBlockStorageDriver, 31);
};

#endif /* __cplusplus */
#endif /* KERNEL */
#endif /* !_IOBLOCKSTORAGEDRIVER_H */
