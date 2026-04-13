/*
 * Copyright (c) 1998-2015 Apple Inc. All rights reserved.
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

#include <IOKit/assert.h>
#include <IOKit/IODeviceTreeSupport.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IOLib.h>
#include <IOKit/IOMapper.h>
#include <IOKit/IOMemoryDescriptor.h>
#include <IOKit/IOSubMemoryDescriptor.h>
#include <IOKit/pwr_mgt/RootDomain.h>
#include <IOKit/storage/IOBlockStorageDevice.h>
#include <IOKit/storage/IOBlockStorageDriver.h>
#include <IOKit/storage/IOMedia.h>
#include <kern/energy_perf.h>
#include <kern/thread_call.h>

#define super IOStorage
OSDefineMetaClassAndStructors(IOBlockStorageDriver, IOStorage)

static char * strclean(char * s)
{
    //
    // strclean() trims any spaces at either end of the string, strips any
    // control characters within the string, and collapses any sequence of
    // spaces within the string into a single space.
    //
 
    int sourceIndex = 0, targetIndex = 0, targetLength = 0;

    for ( ; s[sourceIndex] > '\0' && s[sourceIndex] <= ' '; sourceIndex++ );

    for ( ; s[sourceIndex]; sourceIndex++ )
    {
        if ( s[sourceIndex] < '\0' || s[sourceIndex] >= ' ' )
        {
            if ( s[sourceIndex] != ' ' )
            {
                if ( targetLength < targetIndex )
                {
                    targetIndex = targetLength + 1;
                }

                targetLength = targetIndex + 1;
            }

            s[targetIndex++] = s[sourceIndex];
        }
    }

    s[targetLength] = '\0';

    return s;
}

IOBlockStorageDevice * IOBlockStorageDriver::getProvider() const
{
    //
    // Obtain this object's provider.  We override the superclass's method to
    // return a more specific subclass of IOService -- IOBlockStorageDevice.  
    // This method serves simply as a convenience to subclass developers.
    //

    return (IOBlockStorageDevice *) IOService::getProvider();
}

bool IOBlockStorageDriver::init(OSDictionary * properties)
{
    //
    // Initialize this object's minimal state.
    //

    // Ask our superclass' opinion.

    if (super::init(properties) == false)  return false;

    // Initialize our state.

    _expansionData = IONew(ExpansionData, 1);
    if (_expansionData == 0)  return false;

    initMediaState();
    
    _ejectable                       = false;
    _removable                       = false;
    
    _mediaBlockSize                  = 0;
    _maxBlockNumber                  = 0;
    _writeProtected                  = false;

    _maxReadBlockTransfer            = 0;
    _maxWriteBlockTransfer           = 0;
    _maxReadByteTransfer             = 0;
    _maxWriteByteTransfer            = 0;
    _maxReadSegmentTransfer          = 0;
    _maxWriteSegmentTransfer         = 0;
    _maxReadSegmentByteTransfer      = 0;
    _maxWriteSegmentByteTransfer     = 0;
    _minSegmentAlignmentByteTransfer = 4;
    _maxSegmentWidthByteTransfer     = 0;

    _contexts                        = NULL;
    _contextsLock                    = IOSimpleLockAlloc();
    _contextsCount                   = 0;
    _contextsMaxCount                = 32;

    if (_contextsLock == 0)
        return false;

    _deblockRequestWriteLock         = IOLockAlloc();
    _deblockRequestWriteLockCount    = 0;
    _openAssertions                  = 0;
    _openClients                     = OSSet::withCapacity(2);
    _powerEventNotifier              = 0;

    for (unsigned index = 0; index < kStatisticsCount; index++)
        _statistics[index] = OSNumber::withNumber(0ULL, 64);

    if (_deblockRequestWriteLock == 0 || _openClients == 0)
        return false;

    for (unsigned index = 0; index < kStatisticsCount; index++)
        if (_statistics[index] == 0)  return false;

    // Create our registry properties.

    OSDictionary * statistics = OSDictionary::withCapacity(kStatisticsCount);

    if (statistics == 0)  return false;

    statistics->setObject( kIOBlockStorageDriverStatisticsBytesReadKey,
                           _statistics[kStatisticsBytesRead] );
    statistics->setObject( kIOBlockStorageDriverStatisticsBytesWrittenKey,
                           _statistics[kStatisticsBytesWritten] );
    statistics->setObject( kIOBlockStorageDriverStatisticsReadErrorsKey,
                           _statistics[kStatisticsReadErrors] );
    statistics->setObject( kIOBlockStorageDriverStatisticsWriteErrorsKey,
                           _statistics[kStatisticsWriteErrors] );
    statistics->setObject( kIOBlockStorageDriverStatisticsLatentReadTimeKey,
                           _statistics[kStatisticsLatentReadTime] );
    statistics->setObject( kIOBlockStorageDriverStatisticsLatentWriteTimeKey,
                           _statistics[kStatisticsLatentWriteTime] );
    statistics->setObject( kIOBlockStorageDriverStatisticsReadsKey,
                           _statistics[kStatisticsReads] );
    statistics->setObject( kIOBlockStorageDriverStatisticsWritesKey,
                           _statistics[kStatisticsWrites] );
    statistics->setObject( kIOBlockStorageDriverStatisticsReadRetriesKey,
                           _statistics[kStatisticsReadRetries] );
    statistics->setObject( kIOBlockStorageDriverStatisticsWriteRetriesKey,
                           _statistics[kStatisticsWriteRetries] );
    statistics->setObject( kIOBlockStorageDriverStatisticsTotalReadTimeKey,
                           _statistics[kStatisticsTotalReadTime] );
    statistics->setObject( kIOBlockStorageDriverStatisticsTotalWriteTimeKey,
                           _statistics[kStatisticsTotalWriteTime] );

    setProperty(kIOBlockStorageDriverStatisticsKey, statistics);

    statistics->release();

    return true;
}

bool IOBlockStorageDriver::start(IOService * provider)
{
    //
    // This method is called once we have been attached to the provider object.
    //

    bool success;

    // Open the block storage device.

    success = open(this);

    if (success)
    {
        // Prepare the block storage driver for operation.

        success = handleStart(provider);

        // Close the block storage device.

        close(this);
    }

    if (success)
    {
        // Register this object so it can be found via notification requests. It is
        // not being registered to have I/O Kit attempt to have drivers match on it,
        // which is the reason most other services are registered -- that's not the
        // intention of this registerService call.

        registerService();
    }

    return success;
}

bool IOBlockStorageDriver::didTerminate(IOService *  provider,
                                        IOOptionBits options,
                                        bool *       defer)
{
    // Try to teardown.

    decommissionMedia(false);

    return super::didTerminate(provider, options, defer);
}

#if TARGET_OS_OSX && defined(__x86_64__)
bool IOBlockStorageDriver::yield(IOService *  provider,
                                 IOOptionBits options,
                                 void *       argument)
{
    return false;
}
#endif /* TARGET_OS_OSX && defined(__x86_64__) */

void IOBlockStorageDriver::free()
{
    //
    // Free all of this object's outstanding resources.
    //

    while (_contexts)
    {
        Context * context = _contexts;

        _contexts = context->next;
        _contextsCount--;

        IODelete(context, Context, 1);
    }

    if (_contextsLock)  IOSimpleLockFree(_contextsLock);

    if (_deblockRequestWriteLock)  IOLockFree(_deblockRequestWriteLock);
    if (_openClients)  _openClients->release();

    for (unsigned index = 0; index < kStatisticsCount; index++)
        if (_statistics[index])  _statistics[index]->release();

    if (_expansionData)  IODelete(_expansionData, ExpansionData, 1);

    super::free();
}

bool IOBlockStorageDriver::handleOpen(IOService *  client,
                                      IOOptionBits options,
                                      void *       argument)
{
    //
    // The handleOpen method grants or denies permission to access this object
    // to an interested client.  The argument is an IOStorageAccess value that
    // specifies the level of access desired -- reader or reader-writer.
    //
    // This method can be invoked to upgrade or downgrade the access level for
    // an existing client as well.  The previous access level will prevail for
    // upgrades that fail, of course.   A downgrade should never fail.  If the
    // new access level should be the same as the old for a given client, this
    // method will do nothing and return success.  In all cases, one, singular
    // close-per-client is expected for all opens-per-client received.
    //
    // This method assumes that the arbitration lock is held.
    //

    assert(client);

    // Ensure there is media in the block storage device.

    if (client != this)
    {
        if (_mediaObject == NULL)
        {
            return false;
        }
    }

    // Handle the first open in a special case.

    if (_openClients->getCount() == 0)
    {
        // Open the block storage device.

        if (getProvider()->open(this) == false)
        {
            return false;
        }
    }

    // Process the open.

    if (client != this)
    {
        _openClients->setObject(client);
    }
    else
    {
        if (_openAssertions == 0)
        {
            _openClients->setObject(client);
        }

        _openAssertions++;
    }

    return true;
}

bool IOBlockStorageDriver::handleIsOpen(const IOService * client) const
{
    //
    // The handleIsOpen method determines whether the specified client, or any
    // client if none is specificed, presently has an open on this object.
    //
    // This method assumes that the arbitration lock is held.
    //

    if (client)
    {
        return _openClients->containsObject(client);
    }
    else
    {
        return _openClients->getCount() ? true : false;
    }
}

void IOBlockStorageDriver::handleClose(IOService * client, IOOptionBits options)
{
    //
    // The handleClose method drops the incoming client's access to this object.
    //
    // This method assumes that the arbitration lock is held.
    //

    assert(client);

    // Process the close.

    if (client != this)
    {
        _openClients->removeObject(client);
    }
    else
    {
        _openAssertions--;

        if (_openAssertions == 0)
        {
            _openClients->removeObject(client);
        }
    }

    // Handle the last close in a special case.

    if (_openClients->getCount() == 0)
    {
        if (_mediaObject)
        {
            if (_mediaObject->isInactive())
            {
                message(kIOMessageServiceIsRequestingClose, getProvider(), 0);
            }
        }

        // Close the block storage device.

        getProvider()->close(this);
    }
}

void IOBlockStorageDriver::read(IOService *           client,
                                UInt64                byteStart,
                                IOMemoryDescriptor *  buffer,
                                IOStorageAttributes * attributes,
                                IOStorageCompletion * completion)
{
    //
    // The read method is the receiving end for all read requests from the
    // storage framework, ie. via the media object created by this driver.
    //
    // This method initiates a sequence of methods (stages) for each read/write
    // request.  The first is prepareRequest, which allocates and prepares some
    // context for the transfer; the second is deblockRequest, which aligns the
    // transfer at the media's block boundaries; third is breakUpRequest, which
    // breaks up the transfer into multiple sub-transfers when certain hardware
    // constraints are exceeded; fourth is executeRequest, which implements the
    // actual transfer from the block storage device.
    //

    // State our assumptions.

    assert( buffer->getDirection( ) == kIODirectionIn );

    // Prepare the transfer.

    prepareRequest( byteStart, buffer, attributes, completion );
}

void IOBlockStorageDriver::write(IOService *           client,
                                 UInt64                byteStart,
                                 IOMemoryDescriptor *  buffer,
                                 IOStorageAttributes * attributes,
                                 IOStorageCompletion * completion)
{
    //
    // The write method is the receiving end for all write requests from the
    // storage framework, ie. via the media object created by this driver.
    //
    // This method initiates a sequence of methods (stages) for each read/write
    // request.  The first is prepareRequest, which allocates and prepares some
    // context for the transfer; the second is deblockRequest, which aligns the
    // transfer at the media's block boundaries; third is breakUpRequest, which
    // breaks up the transfer into multiple sub-transfers when certain hardware
    // constraints are exceeded; fourth is executeRequest, which implements the
    // actual transfer from the block storage device.
    //

    // State our assumptions.

    assert( buffer->getDirection( ) == kIODirectionOut );

    // Prepare the transfer.

    prepareRequest( byteStart, buffer, attributes, completion );
}

void IOBlockStorageDriver::addToBytesTransferred(UInt64 bytesTransferred,
                                                 UInt64 totalTime,       // (ns)
                                                 UInt64 latentTime,      // (ns)
                                                 bool   isWrite)
{
    //
    // Update the total number of bytes transferred, the total transfer time,
    // and the total latency time -- used for statistics.
    //

    if (isWrite)
    {
        _statistics[kStatisticsWrites]->addValue(1);
        _statistics[kStatisticsBytesWritten]->addValue(bytesTransferred);
        _statistics[kStatisticsTotalWriteTime]->addValue(totalTime);
        _statistics[kStatisticsLatentWriteTime]->addValue(latentTime);
    }
    else
    {
        _statistics[kStatisticsReads]->addValue(1);
        _statistics[kStatisticsBytesRead]->addValue(bytesTransferred);
        _statistics[kStatisticsTotalReadTime]->addValue(totalTime);
        _statistics[kStatisticsLatentReadTime]->addValue(latentTime);
    }
}

void IOBlockStorageDriver::incrementRetries(bool isWrite)
{
    //
    // Update the total retry count -- used for statistics.
    //

    if (isWrite)
        _statistics[kStatisticsWriteRetries]->addValue(1);
    else
        _statistics[kStatisticsReadRetries]->addValue(1);
}

void IOBlockStorageDriver::incrementErrors(bool isWrite)
{
    //
    // Update the total error count -- used for statistics.
    //

    if (isWrite)
        _statistics[kStatisticsWriteErrors]->addValue(1);
    else
        _statistics[kStatisticsReadErrors]->addValue(1);
}

UInt32 IOBlockStorageDriver::getStatistics(UInt64 * statistics,
                                           UInt32   statisticsMaxCount) const
{
    //
    // Ask the driver to report its operating statistics.
    //
    // The statistics are each indexed by IOBlockStorageDriver::Statistics
    // indices.  This routine fills the caller's buffer, up to the maximum
    // count specified if the real number of statistics would overflow the
    // buffer.  The return value indicates the actual number of statistics
    // copied to the buffer.
    //
    // If the statistics buffer is not supplied or if the maximum count is
    // zero, the routine returns the proposed count of statistics instead.
    //

    if (statistics == 0)
        return kStatisticsCount;

    UInt32 statisticsCount = min(kStatisticsCount, statisticsMaxCount);

    for (unsigned index = 0; index < statisticsCount; index++)
        statistics[index] = _statistics[index]->unsigned64BitValue();

    return statisticsCount;
}

UInt64 IOBlockStorageDriver::getStatistic(Statistics statistic) const
{
    //
    // Ask the driver to report one of its operating statistics.
    //

    if ((UInt32) statistic >= kStatisticsCount)  return 0;

    return _statistics[statistic]->unsigned64BitValue();
}

IOBlockStorageDriver::Context * IOBlockStorageDriver::allocateContext()
{
    //
    // Allocate a context structure for a read/write operation.
    //

    Context * context;

    IOSimpleLockLock(_contextsLock);

    context = _contexts;

    if (context)
    {
        _contexts = context->next;
        _contextsCount--;
    }

    IOSimpleLockUnlock(_contextsLock);

    if (context == 0)
    {
        context = IONew(Context, 1);
    }

    if (context)
    {
        bzero(context, sizeof(Context));
    }

    return context;
}

void IOBlockStorageDriver::deleteContext(
                                        IOBlockStorageDriver::Context * context)
{
    //
    // Delete a context structure from a read/write operation.
    //

    IOSimpleLockLock(_contextsLock);

    if (_contextsCount < _contextsMaxCount)
    {
        context->next = _contexts;

        _contexts = context;
        _contextsCount++;

        context = 0;
    }

    IOSimpleLockUnlock(_contextsLock);

    if (context)
    {
        IODelete(context, Context, 1);
    }
}

void IOBlockStorageDriver::prepareRequestCompletion(void *   target,
                                                    void *   parameter,
                                                    IOReturn status,
                                                    UInt64   actualByteCount)
{
    //
    // This is the completion routine for the prepared request.  It updates
    // the driver's statistics, performs some clean up work, then calls the
    // original request's completion routine.
    //

    Context *              context = (Context              *) parameter;
    IOBlockStorageDriver * driver  = (IOBlockStorageDriver *) target;
    bool                   isWrite;
    AbsoluteTime           time;
    UInt64                 timeInNanoseconds;

    isWrite = (context->request.buffer->getDirection() == kIODirectionOut);

    // Update the error state, on a short transfer.

    if (actualByteCount < context->request.buffer->getLength())
    {
        if (status == kIOReturnSuccess)
        {
            status = kIOReturnUnderrun;
        }
    }

    // Update the total number of bytes transferred and the total transfer time.

    clock_get_uptime(&time);
    SUB_ABSOLUTETIME(&time, &context->timeStart);
    absolutetime_to_nanoseconds(time, &timeInNanoseconds);

    driver->addToBytesTransferred(actualByteCount, timeInNanoseconds, 0, isWrite);

    // Update the total error count.

    if (status != kIOReturnSuccess)
    {
        driver->incrementErrors(isWrite);
    }

    // Complete the transfer request.

    IOStorage::complete(&context->request.completion, status, actualByteCount);

    // Release our resources.

    context->request.buffer->release();

    driver->deleteContext(context);

    driver->release();
}

#if TARGET_OS_OSX && defined(__x86_64__)
void IOBlockStorageDriver::schedulePoller()
{

}

void IOBlockStorageDriver::unschedulePoller()
{

}
#endif /* TARGET_OS_OSX && defined(__x86_64__) */

IOReturn IOBlockStorageDriver::message(UInt32      type,
                                       IOService * provider,
                                       void *      argument)
{
    //
    // Generic entry point for calls from the provider.  A return value of
    // kIOReturnSuccess indicates that the message was received, and where
    // applicable, that it was successful.
    //

    switch (type)
    {
        case kIOMessageMediaParametersHaveChanged:
        {
            IOReturn status;
            if (open(this)) {
                status = recordMediaParameters();
                if (status == kIOReturnSuccess) {
                    UInt64 nbytes;
                    IOMedia *m;
                    if (_maxBlockNumber) {
                        nbytes = _mediaBlockSize * (_maxBlockNumber + 1);
                    } else {
                        nbytes = 0;
                    }
                    m = instantiateMediaObject(0,nbytes,(UInt32) _mediaBlockSize,NULL);
                    if (m) {
                        lockForArbitration();
                        if (_mediaObject) {
                            _mediaObject->init( m->getBase(),
                                                m->getSize(),
                                                m->getPreferredBlockSize(),
                                                m->getAttributes(),
                                                m->isWhole(),
                                                m->isWritable(),
                                                m->getContentHint() );
                            _mediaObject->messageClients(kIOMessageServicePropertyChange);
                        } else {
                            status = kIOReturnNoMedia;
                        }
                        unlockForArbitration();
                        m->release();
                    } else {
                        status = kIOReturnBadArgument;
                    }
                }
                close(this);
            } else {
                status = kIOReturnNotAttached;
            }
            return status;
        }
        case kIOMessageMediaStateHasChanged:
        {
            IOReturn status;
            if (open(this)) {
                status = mediaStateHasChanged((IOMediaState) (uintptr_t) argument);
                close(this);
            } else {
                status = kIOReturnNotAttached;
            }
            return status;
        }
        case kIOMessageServiceIsRequestingClose:
        {
            IOReturn status;
            status = decommissionMedia(false);
            return status;
        }
        default:
        {
            return super::message(type, provider, argument);
        }
    }
}

/* Accept a new piece of media, doing whatever's necessary to make it
 * show up properly to the system. The arbitration lock is assumed to
 * be held during the call.
 */
IOReturn
IOBlockStorageDriver::acceptNewMedia(void)
{
    IOReturn result;
    bool ok;
    UInt64 nbytes;
    char name[128];
    bool nameSep;
    IOMedia *m;

    if (_maxBlockNumber) {
        nbytes = _mediaBlockSize * (_maxBlockNumber + 1);  
    } else {
        nbytes = 0;
    }

    /* Instantiate a media object and attach it to ourselves. */

    name[0] = 0;
    nameSep = false;
    if (getProvider()->getVendorString()) {
        strlcat(name, getProvider()->getVendorString(), sizeof(name) - strlen(name));
        nameSep = true;
    }
    if (getProvider()->getProductString()) {
        if (nameSep == true)  strlcat(name, " ", sizeof(name) - strlen(name));
        strlcat(name, getProvider()->getProductString(), sizeof(name) - strlen(name));
        nameSep = true;
    }
    if (nameSep == true)  strlcat(name, " ", sizeof(name) - strlen(name));
    strlcat(name, "Media", sizeof(name) - strlen(name));
    strclean(name);

    m = instantiateMediaObject(0,nbytes,(UInt32) _mediaBlockSize,name);
    result = (m) ? kIOReturnSuccess : kIOReturnBadArgument;

    if (result == kIOReturnSuccess) {
        if (getProperty(kIOMediaIconKey, gIOServicePlane)) {
            m->removeProperty(kIOMediaIconKey);
        }
        ok = m->attach(this);	/* attach media object above us */
        if (ok) {
            IOService *parent = this;
            OSNumber *unit = NULL;
            OSNumber *unitLUN = NULL;
            OSString *unitName = NULL;

            /* Wire the media object to the device tree. */

            while ((parent = parent->getProvider())) {
                if (!unit) {
                    unit = OSDynamicCast(OSNumber, parent->getProperty("IOUnit"));
                }
                if (!unitLUN) {
                    unitLUN = OSDynamicCast(OSNumber, parent->getProperty("IOUnitLUN"));
                }
                if (!unitName) {
                    unitName = OSDynamicCast(OSString, parent->getProperty("IOUnitName"));
                }
                if (parent->inPlane(gIODTPlane)) {
                    IORegistryEntry *child;
                    IORegistryIterator *children;
                    if (!unit || !parent->getProvider()) {
                        break;
                    }

                    children = IORegistryIterator::iterateOver(parent, gIODTPlane);
                    if (!children) {
                        break;
                    }
                    while ((child = children->getNextObject())) {
                        if (!OSDynamicCast(IOMedia, child)) {
                            child->detachAll(gIODTPlane);
                        }
                    }
                    children->release();

                    if (m->attachToParent(parent, gIODTPlane)) {
                        char location[ 32 ];
                        if (unitLUN && unitLUN->unsigned32BitValue()) {
                            snprintf(location, sizeof(location), "%x,%x:0", unit->unsigned32BitValue(), unitLUN->unsigned32BitValue());
                        } else {
                            snprintf(location, sizeof(location), "%x:0", unit->unsigned32BitValue());
                        }
                        m->setLocation(location, gIODTPlane);
                        m->setName(unitName ? unitName->getCStringNoCopy() : "", gIODTPlane);
                    }
                    break;
                }
            }

            lockForArbitration();
            _mediaObject = m;
            _mediaObject->retain();
            unlockForArbitration();

            m->registerService(kIOServiceAsynchronous);		/* enable matching */
        } else {
            result = kIOReturnNoMemory;	/* give up now */
        }
        m->release();
    }

    return(result);
}

IOReturn
IOBlockStorageDriver::checkForMedia(void)
{
    IOReturn result;
    bool currentState;
    bool changed;
    
    result = getProvider()->reportMediaState(&currentState,&changed);
    if (result != kIOReturnSuccess) {		/* the poll operation failed */
        IOLog("%s[IOBlockStorageDriver]::checkForMedia; err '%s' from reportMediaState\n",
              getName(),stringFromReturn(result));
    } else {
        changed = _mediaObject ? !currentState : currentState;
        if (changed) {	/* the poll succeeded, media state has changed */
            result = mediaStateHasChanged(currentState ? kIOMediaStateOnline
                                                       : kIOMediaStateOffline);
        }
    }

    return(result);
}

IOReturn
IOBlockStorageDriver::mediaStateHasChanged(IOMediaState state)
{
    IOReturn result;

    /* The media has changed state. See if it's just inserted or removed. */

    if (state == kIOMediaStateOnline) {		/* media is now present */

        if (_mediaObject) {
            return(kIOReturnBadArgument);
        }

        initMediaState();        /* clear all knowledge of the media */

        /* Allow a subclass to decide whether we accept the media. Such a
         * decision might be based on things like password-protection, etc.
         */

        if (validateNewMedia() == false) {	/* we're told to reject it */
            rejectMedia();			/* so let subclass do whatever it wants */
            return(kIOReturnSuccess);		/* pretend nothing happened */
        }

        result = recordMediaParameters();	/* learn about media */
        if (result != kIOReturnSuccess) {	/* couldn't record params */
	    IOLog("%s[IOBlockStorageDriver]::mediaStateHasChanged: err '%s' from recordMediaParameters\n",
			getName(),stringFromReturn(result));
            return(result);
        }

        /* Now we do what's necessary to make the new media
         * show up properly in the system.
         */

        result = acceptNewMedia();
        if (result != kIOReturnSuccess) {
	    IOLog("%s[IOBlockStorageDriver]::mediaStateHasChanged; err '%s' from acceptNewMedia\n",
            getName(),stringFromReturn(result));
        }

        return(result);		/* all done, new media is ready */

    } else {				/* media is now absent */

        result = decommissionMedia(true);	/* force a teardown */
        if (result != kIOReturnSuccess && result != kIOReturnNoMedia) {
	    IOLog("%s[IOBlockStorageDriver]::mediaStateHasChanged; err '%s' from decommissionNewMedia\n",
			getName(),stringFromReturn(result));
            return(result);
        }

        return(kIOReturnSuccess);		/* all done; media is gone */
    }
}

#if TARGET_OS_OSX && defined(__x86_64__)
UInt64
IOBlockStorageDriver::constrainByteCount(UInt64 /* requestedCount */ ,bool isWrite)
{
    if (isWrite) {
        return(_maxWriteByteTransfer);
    } else {
        return(_maxReadByteTransfer);
    }
}
#endif /* TARGET_OS_OSX && defined(__x86_64__) */

/* Decommission a piece of media that has become unavailable either due to
 * ejection or some outside force (e.g. the Giant Hand of the User).
 * (I prefer the term "decommission" rather than "abandon." The former implies
 * a well-determined procedure, whereas the latter implies leaving the media
 * in an orphaned state.)
 */
/* Tear down the stack above the specified object. Usually these objects will
 * be of type IOMedia, but they could be any IOService.
 */
IOReturn
IOBlockStorageDriver::decommissionMedia(bool forcible)
{
    IOMedia *m = NULL;
    IOReturn result;

    lockForArbitration();

    if (_mediaObject) {
        /* If this is a forcible decommission (i.e. media is gone), we
         * forget about the media.
         */
        if (forcible || !_openClients->containsObject(_mediaObject)) {
            m = _mediaObject;
            _mediaObject = 0;

            result = kIOReturnSuccess;
        } else {
            result = kIOReturnBusy;
        }
    } else {
        result = kIOReturnNoMedia;
    }

    unlockForArbitration();

    if (m) {
        IORegistryEntry * parent;

        /* Unwire the media object from the device tree. */

        if ( (parent = m->getParentEntry(gIODTPlane)) ) {
            m->detachFromParent(parent, gIODTPlane);
        }

        if (!m->isInactive()) {
            m->terminate();
        }

        m->release();
    }

    return(result);
}

IOReturn
IOBlockStorageDriver::ejectMedia(void)
{
    IOReturn result;

    if (_ejectable)
    {
        result = decommissionMedia(false);	/* try to teardown */
    }
    else
    {
        lockForArbitration();

        if (_mediaObject) {
            if (!_openClients->containsObject(_mediaObject)) {
                result = kIOReturnSuccess;
            } else {
                result = kIOReturnBusy;
            }
        } else {
            result = kIOReturnNoMedia;
        }

        unlockForArbitration();
    }

    if (result == kIOReturnSuccess) {	/* eject */
        if (!_writeProtected) {
            (void)getProvider()->doSynchronize(0,0);
        }

        result = getProvider()->doEjectMedia();
    }

    return(result);
}

void
IOBlockStorageDriver::executeRequest(UInt64                          byteStart,
                                     IOMemoryDescriptor *            buffer,
                                     IOStorageAttributes *           attributes,
                                     IOStorageCompletion *           completion,
                                     IOBlockStorageDriver::Context * context)
{
    UInt32 flags = 0;
    UInt64 block;
    UInt64 nblks;
    IOReturn result;

    if (!_mediaObject) {		/* no media? you lose */
        complete(completion,kIOReturnNoMedia,0);
        return;
    }

    /* We know that we are never called with a request too large,
     * nor one that is misaligned with a block.
     */
    assert((byteStart           % context->block.size) == 0);
    assert((buffer->getLength() % context->block.size) == 0);
    
    block = byteStart           / context->block.size;
    nblks = buffer->getLength() / context->block.size;

    /* Now the protocol-specific provider implements the actual
     * start of the data transfer: */

    if (attributes) {
        if (attributes->priority > kIOStoragePriorityDefault) {
            flags |= IO_PRIORITY_LOW;
        }
    }

    if (_solidState) {
        flags |= IO_MEDIUM_SOLID_STATE;
    }

    if (buffer->getDirection() == kIODirectionIn) {
        io_rate_update(flags, 1, 0, buffer->getLength(), 0);
    } else {
        io_rate_update(flags, 0, 1, 0, buffer->getLength());
    }

    // This is where we adjust the offset for this access to the nand layer.
    // We already maintain this buffer's file offset in attributes.fileOffset
    if (!attributes) {
        attributes = &context->request.attributes;
    }
    attributes->adjustedOffset = ((SInt64)byteStart - (SInt64)context->request.byteStart);

    result = getProvider()->doAsyncReadWrite(buffer,block,nblks,attributes,completion);

    if (result != kIOReturnSuccess) {		/* it failed to start */
        if (result != kIOReturnNotPermitted) {		/* expected error from content protection */
            IOLog("%s[IOBlockStorageDriver]; executeRequest: request failed to start!\n",getName());
        }
        complete(completion,result);
        return;
    }
}

IOReturn
IOBlockStorageDriver::formatMedia(UInt64 byteCapacity)
{
    IOReturn result;

    result = decommissionMedia(false);	/* try to teardown */

    if (result == kIOReturnSuccess) {	/* format */
        result = getProvider()->doFormatMedia(byteCapacity);

        if (result == kIOReturnSuccess) {
            result = mediaStateHasChanged(kIOMediaStateOnline);
        } else {
            (void)mediaStateHasChanged(kIOMediaStateOnline);
        }
    }

    return(result);
}

const char *
IOBlockStorageDriver::getDeviceTypeName(void)
{
    return(kIOBlockStorageDeviceTypeGeneric);
}

UInt32
IOBlockStorageDriver::getFormatCapacities(UInt64 * capacities,
                                            UInt32   capacitiesMaxCount) const
{
    return(getProvider()->doGetFormatCapacities(capacities,capacitiesMaxCount));
}

UInt64
IOBlockStorageDriver::getMediaBlockSize() const
{
    return(_mediaBlockSize);
}

IOMediaState
IOBlockStorageDriver::getMediaState() const
{
    if (_mediaObject) {
        return(kIOMediaStateOnline);
    } else {
        return(kIOMediaStateOffline);
    }
}

IOReturn
IOBlockStorageDriver::handlePowerEvent(void *target,void *refCon,
                                       UInt32 messageType,IOService *provider,
                                       void *messageArgument,vm_size_t argSize)
{
    IOBlockStorageDriver *driver = (IOBlockStorageDriver *)target;
    IOReturn result;

    switch (messageType) {
        case kIOMessageSystemWillPowerOff:
        case kIOMessageSystemWillRestart:
            if (driver->open(driver)) {
                if (driver->_mediaObject) {
                    if (!driver->_writeProtected) {
                        driver->getProvider()->doSynchronize(0,0);
                    }
                    if (!driver->_removable && (messageType == kIOMessageSystemWillPowerOff)) {
                        driver->getProvider()->doEjectMedia();
                    }
                }
                driver->close(driver);
            }
            result = kIOReturnSuccess;
            break;

        default:
            result = kIOReturnUnsupported;
            break;
    }

    return(result);
}

bool
IOBlockStorageDriver::handleStart(IOService * provider)
{
    OSObject *object;
    OSNumber *number;
    IOReturn result;

    /* The protocol-specific provider determines whether the media is removable. */

    result = getProvider()->reportRemovability(&_removable);
    if (result != kIOReturnSuccess) {
	IOLog("%s[IOBlockStorageDriver]::handleStart; err '%s' from reportRemovability\n",
			getName(),stringFromReturn(result));
        return(false);
    }

    if (_removable) {

        /* The protocol-specific provider determines whether the media is ejectable
         * under software control.
         */
        result = getProvider()->reportEjectability(&_ejectable);
        if (result != kIOReturnSuccess) {
	    IOLog("%s[IOBlockStorageDriver]::handleStart; err '%s' from reportEjectability\n",
			getName(),stringFromReturn(result));
            return(false);
        }

    } else {		/* fixed disk: not ejectable */
        OSDictionary *dictionary = OSDynamicCast(OSDictionary, getProvider()->getProperty(kIOPropertyDeviceCharacteristicsKey));

        if (dictionary) {
            OSString *string = OSDynamicCast(OSString, dictionary->getObject(kIOPropertyMediumTypeKey));

            if (string) {
                if (string->isEqualTo(kIOPropertyMediumTypeSolidStateKey)) {
                    _solidState = true;
                }
            }
        }

        _ejectable = false;
    }

    /* Obtain the constraint values for reads and writes. */

    object = copyProperty(kIOMaximumBlockCountReadKey, gIOServicePlane);
    if (object) {

        number = OSDynamicCast(OSNumber, object);
        if (number) {
            _maxReadBlockTransfer = number->unsigned64BitValue();
        }

        object->release();
    }

    object = copyProperty(kIOMaximumBlockCountWriteKey, gIOServicePlane);
    if (object) {

        number = OSDynamicCast(OSNumber, object);
        if (number) {
            _maxWriteBlockTransfer = number->unsigned64BitValue();
        }

        object->release();
    }

    object = copyProperty(kIOMaximumByteCountReadKey, gIOServicePlane);
    if (object) {

        number = OSDynamicCast(OSNumber, object);
        if (number) {
            _maxReadByteTransfer = number->unsigned64BitValue();
        }

        object->release();
    }

    object = copyProperty(kIOMaximumByteCountWriteKey, gIOServicePlane);
    if (object) {

        number = OSDynamicCast(OSNumber, object);
        if (number) {
            _maxWriteByteTransfer = number->unsigned64BitValue();
        }

        object->release();
    }

    object = copyProperty(kIOMaximumSegmentCountReadKey, gIOServicePlane);
    if (object) {

        number = OSDynamicCast(OSNumber, object);
        if (number) {
            _maxReadSegmentTransfer = number->unsigned64BitValue();
        }

        object->release();
    }

    object = copyProperty(kIOMaximumSegmentCountWriteKey, gIOServicePlane);
    if (object) {

        number = OSDynamicCast(OSNumber, object);
        if (number) {
            _maxWriteSegmentTransfer = number->unsigned64BitValue();
        }

        object->release();
    }

    object = copyProperty(kIOMaximumSegmentByteCountReadKey, gIOServicePlane);
    if (object) {

        number = OSDynamicCast(OSNumber, object);
        if (number) {
            _maxReadSegmentByteTransfer = number->unsigned64BitValue();
        }

        object->release();
    }

    object = copyProperty(kIOMaximumSegmentByteCountWriteKey, gIOServicePlane);
    if (object) {

        number = OSDynamicCast(OSNumber, object);
        if (number) {
            _maxWriteSegmentByteTransfer = number->unsigned64BitValue();
        }

        object->release();
    }

    object = copyProperty(kIOMinimumSegmentAlignmentByteCountKey, gIOServicePlane);
    if (object) {

        number = OSDynamicCast(OSNumber, object);
        if (number) {
            _minSegmentAlignmentByteTransfer = number->unsigned64BitValue();
        }

        object->release();
    } else {
        getProvider()->setProperty(kIOMinimumSegmentAlignmentByteCountKey, _minSegmentAlignmentByteTransfer, 64);
    }

    object = copyProperty(kIOMaximumSegmentAddressableBitCountKey, gIOServicePlane);
    if (object) {

        number = OSDynamicCast(OSNumber, object);
        if (number) {
            if (number->unsigned64BitValue()) {
                if (number->unsigned64BitValue() < 64) {
                    _maxSegmentWidthByteTransfer = 1ULL << number->unsigned64BitValue();
                }
            }
        }

        object->release();
    }

    object = copyProperty(kIOCommandPoolSizeKey, gIOServicePlane);
    if (object) {

        number = OSDynamicCast(OSNumber, object);
        if (number) {
            _contextsMaxCount = number->unsigned32BitValue();
        }

        object->release();
    }

    /* Check for the device being ready with media inserted: */

    result = checkForMedia();

    /* The poll should never fail for nonremovable media: */
    
    if (result != kIOReturnSuccess && !_removable) {
	IOLog("%s[IOBlockStorageDriver]::handleStart: err '%s' from checkForMedia\n",
			getName(),stringFromReturn(result));
        return(false);
    }

    /* Set up the power event handler for restarts and shutdowns: */

    _powerEventNotifier = registerPrioritySleepWakeInterest(handlePowerEvent,this);
    if (_powerEventNotifier) {
        retain();
    }

    return(true);
}

#if TARGET_OS_OSX && defined(__x86_64__)
bool
IOBlockStorageDriver::handleYield(IOService *  provider,
                                  IOOptionBits options,
                                  void *       argument)
{
    return false;
}
#endif /* TARGET_OS_OSX && defined(__x86_64__) */

void
IOBlockStorageDriver::initMediaState(void)
{
    _mediaType = 0;
}

IOMedia *
IOBlockStorageDriver::instantiateDesiredMediaObject(void)
{
    return(new IOMedia);
}

IOMedia *
IOBlockStorageDriver::instantiateMediaObject(UInt64 base,UInt64 byteSize,
                                        UInt32 blockSize,char *mediaName)
{
    IOMediaAttributeMask attributes = 0;
    const char *contentHint = "";
    IOMedia *m;
    bool result;

    OSString *providerHint = OSDynamicCast(OSString,
                                           getProvider()->getProperty(kIOMediaContentHintKey));
    if (providerHint && providerHint->getLength())
    {
        const char *hint = providerHint->getCStringNoCopy();
        if (hint)  contentHint = hint;
    }

    m = instantiateDesiredMediaObject();
    if (m == NULL) {
        return(NULL);
    }

    attributes |= _ejectable ? kIOMediaAttributeEjectableMask : 0;
    attributes |= _removable ? kIOMediaAttributeRemovableMask : 0;

    result = m->init(   base,			/* base byte offset */
                        byteSize,		/* byte size */
                        blockSize,		/* preferred block size */
                        attributes,		/* attributes */
                        true,			/* TRUE if whole physical media */
                        !_writeProtected,	/* TRUE if writable */
                        contentHint);			/* content hint */

    if (result) {
        const char *picture = "External.icns";

        if (_removable) {
            picture = "Removable.icns";
        } else {
            OSDictionary *dictionary = OSDynamicCast(OSDictionary, getProvider()->getProperty(kIOPropertyProtocolCharacteristicsKey));

            if (dictionary) {
                OSString *string = OSDynamicCast(OSString, dictionary->getObject(kIOPropertyPhysicalInterconnectLocationKey));

                if (string) {
                    if (string->isEqualTo(kIOPropertyExternalKey)) {
                        picture = "External.icns";
                    } else {
                        picture = "Internal.icns";
                    }
                }
            }
        }

        if (picture) {
            OSDictionary *dictionary = OSDictionary::withCapacity(2);
            OSString *identifier = OSString::withCString("com.apple.iokit.IOStorageFamily");
            OSString *resourceFile = OSString::withCString(picture);

            if (dictionary && identifier && resourceFile) {
                dictionary->setObject("CFBundleIdentifier", identifier);
                dictionary->setObject("IOBundleResourceFile", resourceFile);
            }

            m->setProperty(kIOMediaIconKey, dictionary);

            if (resourceFile) {
                resourceFile->release();
            }
            if (identifier) {
                identifier->release();
            }
            if (dictionary) {
                dictionary->release();
            }
        }

        if (mediaName) {
            m->setName(mediaName);
        }

        return(m);
        
    } else {					/* some init error */
        m->release();
        return(NULL);		/* beats me...call it this error */
    }
}

bool
IOBlockStorageDriver::isMediaEjectable(void) const
{
    return(_ejectable);
}

bool
IOBlockStorageDriver::isMediaRemovable(void) const
{
    return(_removable);
}

#if TARGET_OS_OSX && defined(__x86_64__)
bool
IOBlockStorageDriver::isMediaPollExpensive(void) const
{
    return(false);
}

bool
IOBlockStorageDriver::isMediaPollRequired(void) const
{
    return(false);
}
#endif /* TARGET_OS_OSX && defined(__x86_64__) */

bool
IOBlockStorageDriver::isMediaWritable(void) const
{
    return(!_writeProtected);
}

#if TARGET_OS_OSX && defined(__x86_64__)
IOReturn
IOBlockStorageDriver::lockMedia(bool locked)
{
    return(kIOReturnUnsupported);        
}

IOReturn
IOBlockStorageDriver::pollMedia(void)
{
    return(kIOReturnUnsupported);
}
#endif /* TARGET_OS_OSX && defined(__x86_64__) */

IOReturn
IOBlockStorageDriver::recordMediaParameters(void)
{
    IOReturn result;

    /* Determine the device's block size and max block number.
     * What should an unformatted device report? All zeroes.
     */

    result = getProvider()->reportBlockSize(&_mediaBlockSize);    
    if (result != kIOReturnSuccess) {
        goto err;
    }

    result = getProvider()->reportMaxValidBlock(&_maxBlockNumber);    
    if (result != kIOReturnSuccess) {
        goto err;
    }

    /* Is the media write-protected? */

    result = getProvider()->reportWriteProtection(&_writeProtected);
    if (result != kIOReturnSuccess) {
        goto err;
    }

    return(kIOReturnSuccess);		/* everything was successful */

    /* If we fall thru to here, we had some kind of error. Set everything to
     * a reasonable state since we haven't got any real information.
     */

err:
    return(result);
}

void
IOBlockStorageDriver::rejectMedia(void)
{
    (void)getProvider()->doEjectMedia();	/* eject it, ignoring any error */
}

void
IOBlockStorageDriver::stop(IOService * provider)
{
    if (_powerEventNotifier) {
        _powerEventNotifier->remove();
        _powerEventNotifier = NULL;
        release();
    }
    super::stop(provider);
}

IOReturn
IOBlockStorageDriver::synchronize(IOService *                 client,
                                  UInt64                      byteStart,
                                  UInt64                      byteCount,
                                  IOStorageSynchronizeOptions options)
{
    UInt64 blockStart;
    UInt64 blockCount;

#if TARGET_OS_OSX && defined(__x86_64__)
    if ( _respondsTo_synchronizeCache )
    {
        if ( options == _kIOStorageSynchronizeOption_super__synchronizeCache )
        {
            options = 0;
        }
        else
        {
            return IOStorage::synchronize( client, byteStart, byteCount, options );
        }
    }
#endif /* TARGET_OS_OSX && defined(__x86_64__) */

    if ( ( options & kIOStorageSynchronizeOptionReserved ) )
    {
        return kIOReturnBadArgument;
    }

    blockStart = byteStart / _mediaBlockSize;

    if ( byteCount )
    {
        blockCount = ( byteStart + byteCount + _mediaBlockSize - 1 ) / _mediaBlockSize - blockStart;
    }
    else
    {
        blockCount = 0;
    }

    return getProvider( )->doSynchronize( blockStart, blockCount, options );
}

IOReturn
IOBlockStorageDriver::unmap(IOService *           client,
                            IOStorageExtent *     extents,
                            UInt32                extentsCount,
                            IOStorageUnmapOptions options)
{
    UInt32                       extentsIndex;
    IOBlockStorageDeviceExtent * extentsOut;
    UInt32                       extentsOutCount;

    assert( sizeof( IOStorageExtent ) == sizeof( IOBlockStorageDeviceExtent ) );

    if ( ( options & kIOStorageUnmapOptionReserved ) )
    {
        return kIOReturnBadArgument;
    }

    extentsOut      = ( IOBlockStorageDeviceExtent * ) extents;
    extentsOutCount = 0;

    for ( extentsIndex = 0; extentsIndex < extentsCount; extentsIndex++ )
    {
        UInt64 blockStart;
        UInt64 blockCount;

        blockStart = ( extents[ extentsIndex ].byteStart + _mediaBlockSize - 1 ) / _mediaBlockSize;
        blockCount = ( extents[ extentsIndex ].byteStart + extents[ extentsIndex ].byteCount ) / _mediaBlockSize;

        if ( blockCount > blockStart )
        {
            blockCount = blockCount - blockStart;

            extentsOut[ extentsOutCount ].blockStart = blockStart;
            extentsOut[ extentsOutCount ].blockCount = blockCount;

            extentsOutCount++;
        }
    }

    if ( extentsOutCount )
    {
        return getProvider( )->doUnmap( extentsOut, extentsOutCount, options );
    }
    else
    {
        return kIOReturnSuccess;
    }
}

IOReturn
IOBlockStorageDriver::getProvisionStatus(IOService *                client,
                                         UInt64                     byteStart,
                                         UInt64                     byteCount,
                                         UInt32 *                   extentsCount,
                                         IOStorageProvisionExtent * extents,
                                         IOStorageGetProvisionStatusOptions options)
{
    UInt64                                blockStart;
    UInt64                                blockCount;
    UInt64                                maxByteSize;
    UInt32                                extentsCountIn;
    IOBlockStorageProvisionDeviceExtent * extentsOut;
    IOReturn                              result;

    if ( options != 0 )
    {
        return kIOReturnBadArgument;
    }

    maxByteSize = (_maxBlockNumber + 1 ) * _mediaBlockSize;
    if ( byteStart > maxByteSize )
    {
        return kIOReturnBadArgument;
    }

    byteCount = min ( byteCount, maxByteSize - byteStart );

    blockStart = byteStart / _mediaBlockSize;
    blockCount = ( byteStart + byteCount + _mediaBlockSize - 1 ) / _mediaBlockSize - blockStart;

    if ( ( extents == NULL ) || ( extentsCount == NULL ) || ( *extentsCount == 0 ) )
    {
        return kIOReturnBadArgument;
    }

    extentsOut = ( IOBlockStorageProvisionDeviceExtent * ) extents;
    extentsCountIn = *extentsCount;

    result = getProvider( )->doGetProvisionStatus( blockStart, blockCount, extentsCount, extentsOut, options );

    if ( result == kIOReturnSuccess )
    {
        UInt32                          extentsIndex;

        extentsCountIn = min ( extentsCountIn, *extentsCount );

        for ( extentsIndex = 0; extentsIndex < extentsCountIn; extentsIndex++ )
        {
            extents [ extentsIndex ].byteStart *= _mediaBlockSize;
            extents [ extentsIndex ].byteCount *= _mediaBlockSize;
        }
    }

    return result;
}

bool IOBlockStorageDriver::lockPhysicalExtents(IOService * client)
{
    //
    // Lock the contents of the storage object against relocation temporarily,
    // for the purpose of getting physical extents.
    //

    return(true);
}

IOStorage * IOBlockStorageDriver::copyPhysicalExtent(IOService * client,
                                                     UInt64 *    byteStart,
                                                     UInt64 *    byteCount)
{
    //
    // Convert the specified byte offset into a physical byte offset, relative
    // to a physical storage object.  This call should only be made within the
    // context of lockPhysicalExtents().
    //

    IOMedia *m;

    lockForArbitration();
    m = _mediaObject;
    if (m) {
        m->retain();
    }
    unlockForArbitration();

    return(m);
}

void IOBlockStorageDriver::unlockPhysicalExtents(IOService * client)
{
    //
    // Unlock the contents of the storage object for relocation again.  This
    // call must balance a successful call to lockPhysicalExtents().
    //

    return;
}

IOReturn
IOBlockStorageDriver::setPriority(IOService *       client,
                                  IOStorageExtent * extents,
                                  UInt32            extentsCount,
                                  IOStoragePriority priority)
{
    UInt32                       extentsIndex;
    IOBlockStorageDeviceExtent * extentsOut;

    assert( sizeof( IOStorageExtent ) == sizeof( IOBlockStorageDeviceExtent ) );

    extentsOut = ( IOBlockStorageDeviceExtent * ) extents;

    for ( extentsIndex = 0; extentsIndex < extentsCount; extentsIndex++ )
    {
        UInt64 blockStart;
        UInt64 blockCount;

        blockStart = extents[ extentsIndex ].byteStart / _mediaBlockSize;
        blockCount = extents[ extentsIndex ].byteCount / _mediaBlockSize;

        extentsOut[ extentsIndex ].blockStart = blockStart;
        extentsOut[ extentsIndex ].blockCount = blockCount;
    }

    return getProvider( )->doSetPriority( extentsOut, extentsCount, priority );
}

bool
IOBlockStorageDriver::validateNewMedia(void)
{
	int boot_arg_value = 0;

	bool boot_arg_found = PE_parse_boot_argn("disable_external_storage", &boot_arg_value, sizeof(boot_arg_value));

	if (!boot_arg_found) {
		return(true);
	}

	if (boot_arg_value == 0) {
		return(true);
	}

	if (_removable || _ejectable) {
		return(false);
	}

	OSDictionary *dictionary = OSDynamicCast(OSDictionary, getProvider()->getProperty(kIOPropertyProtocolCharacteristicsKey));

	if (dictionary) {
		OSString *string = OSDynamicCast(OSString, dictionary->getObject(kIOPropertyPhysicalInterconnectLocationKey));

		if (string) {
			if (string->isEqualTo(kIOPropertyInternalKey)) {
				return(true);
			}
		}
	}

	return(false);

}

// -----------------------------------------------------------------------------
// Deblocker Implementation

#include <IOKit/IOBufferMemoryDescriptor.h>

class IODeblocker : public IOMemoryDescriptor
{
    OSDeclareDefaultStructors(IODeblocker);

protected:

    UInt64                     _blockSize;

    struct
    {
        IOMemoryDescriptor * buffer;
        IOByteCount          offset;
        IOByteCount          length;
    }                          _chunks[3];
    UInt32                     _chunksCount;

    IOBufferMemoryDescriptor * _excessBuffer;
    UInt64                     _excessCountFinal;
    UInt64                     _excessCountStart;

    IOMemoryDescriptor *       _requestBuffer;
    IOStorageAttributes        _requestAttributes;
    IOStorageCompletion        _requestCompletion;
    void *                     _requestContext;
    UInt64                     _requestCount;
    bool                       _requestIsOneBlock;
    UInt64                     _requestStart;

    UInt64                     _byteStart;

    thread_call_t              _threadCallback;

    enum
    {
        kStageInit,
        kStagePrepareExcessStart,
        kStagePrepareExcessFinal,
        kStageLast,
        kStageDone
    } _stage;

    virtual void free() APPLE_KEXT_OVERRIDE;
    virtual uint64_t getPreparationID( void ) APPLE_KEXT_OVERRIDE;

public:

    static IODeblocker * withBlockSize(
                                  UInt64                blockSize,
                                  UInt64                withRequestStart,
                                  IOMemoryDescriptor *  withRequestBuffer,
                                  IOStorageAttributes * withRequestAttributes,
                                  IOStorageCompletion * withRequestCompletion,
                                  void *                withRequestContext );

    virtual bool initWithBlockSize(
                                  UInt64                blockSize,
                                  UInt64                withRequestStart,
                                  IOMemoryDescriptor *  withRequestBuffer,
                                  IOStorageAttributes * withRequestAttributes,
                                  IOStorageCompletion * withRequestCompletion,
                                  void *                withRequestContext );

    virtual addr64_t getPhysicalSegment( IOByteCount   offset,
                                         IOByteCount * length,
                                         IOOptionBits  options = 0 ) APPLE_KEXT_OVERRIDE;

    virtual IOReturn prepare(IODirection forDirection = kIODirectionNone) APPLE_KEXT_OVERRIDE;

    virtual IOReturn complete(IODirection forDirection = kIODirectionNone) APPLE_KEXT_OVERRIDE;

    virtual bool getNextStage();

    virtual IOStorageAttributes * getRequestAttributes();

    virtual IOStorageCompletion * getRequestCompletion(UInt64 * actualByteCount);

    virtual IOMemoryDescriptor * getRequestBuffer();

    virtual void * getRequestContext();

    virtual UInt64 getByteStart();

    virtual thread_call_t getThreadCallback();

    virtual bool setThreadCallback(thread_call_func_t callback);
};

#undef  super
#define super IOMemoryDescriptor
OSDefineMetaClassAndStructors(IODeblocker, IOMemoryDescriptor)

IODeblocker * IODeblocker::withBlockSize(
                                  UInt64                blockSize,
                                  UInt64                withRequestStart,
                                  IOMemoryDescriptor *  withRequestBuffer,
                                  IOStorageAttributes * withRequestAttributes,
                                  IOStorageCompletion * withRequestCompletion,
                                  void *                withRequestContext )
{
    //
    // Create a new IODeblocker.
    //

    IODeblocker * me = new IODeblocker;
    
    if ( me && me->initWithBlockSize(
                /* blockSize               */ blockSize,
                /* withRequestStart        */ withRequestStart,
                /* withRequestBuffer       */ withRequestBuffer,
                /* withRequestAttributes   */ withRequestAttributes,
                /* withRequestCompletion   */ withRequestCompletion,
                /* withRequestContext      */ withRequestContext ) == false )
    {
	    me->release();
	    me = 0;
    }

    return me;
}

bool IODeblocker::initWithBlockSize(
                                  UInt64                blockSize,
                                  UInt64                withRequestStart,
                                  IOMemoryDescriptor *  withRequestBuffer,
                                  IOStorageAttributes * withRequestAttributes,
                                  IOStorageCompletion * withRequestCompletion,
                                  void *                withRequestContext )
{
    //
    // Initialize an IODeblocker.
    //
    // _excessCountStart = byte count from media boundary to start of request
    // _excessCountFinal = byte count from end of request to a media boundary
    //

    IOByteCount excessBufferSize = 0;

    // Ask our superclass' opinion.

    if ( super::init() == false )  return false;

    // Initialize our minimal state.

    _blockSize         = blockSize;
    _chunksCount       = 0;
    _flags             = kIODirectionNone;
    _length            = 0;

    _requestBuffer     = withRequestBuffer;
    _requestBuffer->retain();

    if (withRequestAttributes)  _requestAttributes = *withRequestAttributes;
    if (withRequestCompletion)  _requestCompletion = *withRequestCompletion;

    _requestContext    = withRequestContext;
    _requestCount      = withRequestBuffer->getLength();
    _requestStart      = withRequestStart;

    _excessCountStart  = (withRequestStart                ) % blockSize;
    _excessCountFinal  = (withRequestStart + _requestCount) % blockSize;
    if ( _excessCountFinal )  _excessCountFinal = blockSize - _excessCountFinal;

    _requestIsOneBlock = (_excessCountStart + _requestCount <= blockSize);

    // Determine the necessary size for our scratch buffer.

    switch ( _requestBuffer->getDirection() )
    {
        case kIODirectionIn:                                           // (read)
        {
            excessBufferSize = max(_excessCountStart, _excessCountFinal);
        } break;

        case kIODirectionOut:                                         // (write)
        {
            if ( _excessCountStart )  excessBufferSize += blockSize;
            if ( _excessCountFinal )  excessBufferSize += blockSize;

            // If there is excess both ends of the original request, but both
            // ends reside within the same media block, then we could shorten
            // our buffer size to just one block.

            if ( _excessCountStart && _excessCountFinal && _requestIsOneBlock )
            {
                excessBufferSize -= blockSize;
            }
        } break;

        default:
        {
            assert(0);
        } break;
    }

    // Allocate our scratch buffer.

    if ( excessBufferSize )
    {
        _excessBuffer = IOBufferMemoryDescriptor::withCapacity(
                                         /* capacity      */ excessBufferSize,
                                         /* withDirection */ kIODirectionNone );
        if ( _excessBuffer == 0 )  return false;
    }

    return true;
}

void IODeblocker::free()
{
    //
    // Free all of this object's outstanding resources.
    //

    if ( _requestBuffer )  _requestBuffer->release();
    if ( _excessBuffer )  _excessBuffer->release();
    if ( _threadCallback )  thread_call_free(_threadCallback);

    super::free();
}

uint64_t
IODeblocker::getPreparationID( void )
{
    uint64_t pID = kIOPreparationIDUnsupported;

    if ( _chunksCount == 0 )
    {
        return (super::getPreparationID());
    }

    //
    // Walk through the list of _chunks, Calling buffer getPreparationID() on
    // each one of them will make sure all of them have a proper
    // preparationID assigned.
    //
    for ( unsigned index = 0; index < _chunksCount; index++ )
    {
        pID = _chunks[index].buffer->getPreparationID();
    }

    return pID;
}

IOReturn IODeblocker::prepare(IODirection forDirection)
{
    //
    // Prepare the memory for an I/O transfer.
    //
    // This involves paging in the memory and wiring it down for the duration
    // of the transfer.  The complete() method finishes the processing of the
    // memory after the I/O transfer finishes.
    //

    unsigned index;
    IOReturn status = kIOReturnInternalError;
    IOReturn statusUndo;

    if ( forDirection == kIODirectionNone )
    {
        forDirection = (IODirection) (_flags & kIOMemoryDirectionMask);
    }

    for ( index = 0; index < _chunksCount; index++ ) 
    {
        status = _chunks[index].buffer->prepare(forDirection);
        if ( status != kIOReturnSuccess )  break;
    }

    if ( status != kIOReturnSuccess )
    {
        for ( unsigned indexUndo = 0; indexUndo <= index; indexUndo++ )
        {
            statusUndo = _chunks[index].buffer->complete(forDirection);
            assert(statusUndo == kIOReturnSuccess);
        }
    }

    return status;
}

IOReturn IODeblocker::complete(IODirection forDirection)
{
    //
    // Complete processing of the memory after an I/O transfer finishes.
    //
    // This method shouldn't be called unless a prepare() was previously issued;
    // the prepare() and complete() must occur in pairs, before and after an I/O
    // transfer.
    //

    IOReturn status;
    IOReturn statusFinal = kIOReturnSuccess;

    if ( forDirection == kIODirectionNone )
    {
        forDirection = (IODirection) (_flags & kIOMemoryDirectionMask);
    }

    for ( unsigned index = 0; index < _chunksCount; index++ ) 
    {
        status = _chunks[index].buffer->complete(forDirection);
        if ( status != kIOReturnSuccess )  statusFinal = status;
        assert(status == kIOReturnSuccess);
    }

    return statusFinal;
}

addr64_t IODeblocker::getPhysicalSegment( IOByteCount   offset,
                                          IOByteCount * length,
                                          IOOptionBits  options )
{
    //
    // This method returns the physical address of the byte at the given offset
    // into the memory,  and optionally the length of the physically contiguous
    // segment from that offset.
    //

    assert(offset <= _length);

    for ( unsigned index = 0; index < _chunksCount; index++ ) 
    {
        if ( offset < _chunks[index].length )
        {
            addr64_t address;
            address = _chunks[index].buffer->getPhysicalSegment(
                                    /* offset  */ offset + _chunks[index].offset,
                                    /* length  */ length,
                                    /* options */ options );
            if ( length )  *length = min(*length, _chunks[index].length);
            return address;
        }
        offset -= _chunks[index].length;
    }

    if ( length )  *length = 0;

    return 0;
}

bool IODeblocker::getNextStage()
{
    //
    // Obtain the next stage of the transfer.   The transfer buffer will be the
    // deblocker object itself and the byte start will be returned in getByteStart().
    //
    // This method must not be called if the current stage failed with an error
    // or a short byte count, but instead getRequestCompletion() must be called
    // to adjust the status and actual byte count (with respect to the original
    // request) and return the original request's completion routine.  The same
    // call to getRequestCompletion() should also be done if the getNextStage()
    // method returns false.
    //

    _chunksCount = 0;
    _flags       = (_flags & ~kIOMemoryDirectionMask) | kIODirectionNone;
    _length      = 0;

    switch ( _requestBuffer->getDirection() )
    {
        case kIODirectionIn:                                           // (read)
        {
            switch ( _stage )
            {
                case kStageInit:
                {
                    _stage     = kStageLast;
                    _excessBuffer->setDirection(kIODirectionIn);
                    _flags     = (_flags & ~kIOMemoryDirectionMask) | kIODirectionIn;
                    _byteStart = _requestStart - _excessCountStart;

                    if ( _excessCountStart )
                    {
                        _chunks[_chunksCount].buffer = _excessBuffer;
                        _chunks[_chunksCount].offset = 0;
                        _chunks[_chunksCount].length = _excessCountStart;
                        _chunksCount++;
                    }

                    _chunks[_chunksCount].buffer = _requestBuffer;
                    _chunks[_chunksCount].offset = 0;
                    _chunks[_chunksCount].length = _requestBuffer->getLength();
                    _chunksCount++;

                    if ( _excessCountFinal )
                    {
                        _chunks[_chunksCount].buffer = _excessBuffer;
                        _chunks[_chunksCount].offset = 0;
                        _chunks[_chunksCount].length = _excessCountFinal;
                        _chunksCount++;
                    }
                } break;

                case kStageLast:
                {
                    _stage = kStageDone;
                } break;

                default:
                {
                    assert(0);
                } break;
            } // (switch)
        } break;

        case kIODirectionOut:                                         // (write)
        {
            switch ( _stage )
            {
                case kStageInit:
                {
                    if ( _excessCountStart )
                    {
                        _stage = kStagePrepareExcessStart;
                        _excessBuffer->setDirection(kIODirectionIn);
                        _flags     = (_flags & ~kIOMemoryDirectionMask) | kIODirectionIn;
                        _byteStart = _requestStart - _excessCountStart;

                        _chunks[_chunksCount].buffer = _excessBuffer;
                        _chunks[_chunksCount].offset = 0;
                        _chunks[_chunksCount].length = _blockSize;
                        _chunksCount++;
                        break;
                    } 
                } // (fall thru)

                case kStagePrepareExcessStart:
                {
                    if ( _excessCountFinal )
                    {
                        // We do not issue this stage if the original transfer
                        // resides within one media block, and we already read
                        // that block into our buffer in the previous stage.

                        if ( !_excessCountStart || !_requestIsOneBlock )
                        {
                            _stage = kStagePrepareExcessFinal;
                            _excessBuffer->setDirection(kIODirectionIn);
                            _flags     = (_flags & ~kIOMemoryDirectionMask) | kIODirectionIn;
                            _byteStart = _requestStart + _requestCount +
                                         _excessCountFinal - _blockSize;

                            _chunks[_chunksCount].buffer = _excessBuffer;
                            _chunks[_chunksCount].offset = (_requestIsOneBlock)
                                                           ? 0
                                                           : (_excessCountStart)
                                                             ? _blockSize
                                                             : 0;
                            _chunks[_chunksCount].length = _blockSize;
                            _chunksCount++;
                            break;
                        }
                    }
                } // (fall thru)

                case kStagePrepareExcessFinal:
                {
                    _stage     = kStageLast;
                    _excessBuffer->setDirection(kIODirectionOut);
                    _flags     = (_flags & ~kIOMemoryDirectionMask) | kIODirectionOut;
                    _byteStart = _requestStart - _excessCountStart;

                    if ( _excessCountStart )
                    {
                        _chunks[_chunksCount].buffer = _excessBuffer;
                        _chunks[_chunksCount].offset = 0;
                        _chunks[_chunksCount].length = _excessCountStart;
                        _chunksCount++;
                    }

                    _chunks[_chunksCount].buffer = _requestBuffer;
                    _chunks[_chunksCount].offset = 0;
                    _chunks[_chunksCount].length = _requestBuffer->getLength();
                    _chunksCount++;

                    if ( _excessCountFinal )
                    {
                        _chunks[_chunksCount].buffer = _excessBuffer;
                        _chunks[_chunksCount].offset = (_requestIsOneBlock)
                                                       ? 0
                                                       : (_excessCountStart)
                                                         ? _blockSize
                                                         : 0;
                        _chunks[_chunksCount].offset += ( _blockSize -
                                                          _excessCountFinal );
                        _chunks[_chunksCount].length = _excessCountFinal;
                        _chunksCount++;
                    }
                } break;

                case kStageLast:
                {
                    _stage = kStageDone;
                } break;

                default:
                {
                    assert(0);
                } break;
            } // (switch)
        } break;

        default:
        {
            assert(0);
        } break;
    } // (switch)

    // Determine whether we have an abort or completion condition.

    if ( _chunksCount == 0 )  return false;

    // Compute the total length of the descriptor over all chunks.

    for ( unsigned index = 0; index < _chunksCount; index++ )
    {
        _length += _chunks[index].length;
    }

    return true;
}

IOStorageAttributes * IODeblocker::getRequestAttributes()
{
    //
    // Obtain the attributes for the original request. 
    //

    return &_requestAttributes;
}

IOStorageCompletion * IODeblocker::getRequestCompletion(UInt64 * actualByteCount)
{
    //
    // Obtain the completion information for the original request, taking
    // into account the actual byte count of the current stage. 
    //

    switch ( _stage )
    {
        case kStageInit:                                       // (inital stage)
        {
            *actualByteCount = 0;
        } break;

        case kStagePrepareExcessStart:              // (write preparation stage)
        case kStagePrepareExcessFinal:
        {
            *actualByteCount = 0;
        } break;

        case kStageLast:                                         // (last stage)
        case kStageDone:
        {
            if ( *actualByteCount > _excessCountStart )
                *actualByteCount -= _excessCountStart;
            else
                *actualByteCount = 0;

            if ( *actualByteCount > _requestBuffer->getLength() )
                *actualByteCount = _requestBuffer->getLength();
        } break;

        default:
        {
            assert(0);
        } break;
    }

    return &_requestCompletion;
}

IOMemoryDescriptor * IODeblocker::getRequestBuffer()
{
    //
    // Obtain the buffer for the original request. 
    //

    return _requestBuffer;
}

void * IODeblocker::getRequestContext()
{
    //
    // Obtain the context for the original request.
    //

    return _requestContext;
}

UInt64 IODeblocker::getByteStart()
{
    //
    // Obtain the byte start for the current stage.
    //

    return _byteStart;
}

thread_call_t IODeblocker::getThreadCallback()
{
    //
    // Obtain the thread callback.
    //

    return _threadCallback;
}

bool IODeblocker::setThreadCallback(thread_call_func_t callback)
{
    //
    // Allocate a thread callback.
    //

    _threadCallback = thread_call_allocate(callback, this);

    return _threadCallback ? true : false;
}

void IOBlockStorageDriver::deblockRequest(
                                     UInt64                          byteStart,
                                     IOMemoryDescriptor *            buffer,
                                     IOStorageAttributes *           attributes,
                                     IOStorageCompletion *           completion,
                                     IOBlockStorageDriver::Context * context )
{
    //
    // The deblockRequest method checks to see if the incoming request rests
    // on the media's block boundaries, and if not, deblocks it.  Deblocking
    // involves rounding out the request to the nearest block boundaries and
    // transferring the excess bytes into a scratch buffer.
    //
    // This method is part of a sequence of methods invoked for each read/write
    // request.  The first is prepareRequest, which allocates and prepares some
    // context for the transfer; the second is deblockRequest, which aligns the
    // transfer at the media's block boundaries; third is breakUpRequest, which
    // breaks up the transfer into multiple sub-transfers when certain hardware
    // constraints are exceeded; fourth is executeRequest, which implements the
    // actual transfer from the block storage device.
    //
    // The current implementation of deblockRequest is asynchronous.
    //

    IODeblocker * deblocker;

    // If the request is aligned with the media's block boundaries, we
    // do short-circuit the deblocker and call breakUpRequest directly.

    if ( (byteStart           % context->block.size) == 0 &&
         (buffer->getLength() % context->block.size) == 0 )
    {
        breakUpRequest(byteStart, buffer, attributes, completion, context);
        return;
    }

    // Build a deblocker object.

    deblocker = IODeblocker::withBlockSize(
                                /* blockSize             */ context->block.size,
                                /* withRequestStart      */ byteStart,
                                /* withRequestBuffer     */ buffer,
                                /* withRequestAttributes */ attributes,
                                /* withRequestCompletion */ completion,
                                /* withRequestContext    */ context );

    if ( deblocker == 0 )
    {
        complete(completion, kIOReturnNoMemory);
        return;
    }

    // This implementation of the deblocker permits only one read-modify-write
    // at any given time.  Note that other write requests can, and do, proceed
    // simultaneously so long as they do not require the deblocker.

    if ( buffer->getDirection() == kIODirectionOut )
    {
        IOLockLock(_deblockRequestWriteLock);

        _deblockRequestWriteLockCount++;

        if ( _deblockRequestWriteLockCount > 1 )
        {
            while ( IOLockSleep( /* lock          */ _deblockRequestWriteLock, 
                                 /* event         */ _deblockRequestWriteLock,
                                 /* interruptible */ THREAD_UNINT ) );
        }

        IOLockUnlock(_deblockRequestWriteLock);
    }

    // Execute the transfer (for the next stage).

    deblockRequestCompletion(this, deblocker, kIOReturnSuccess, 0);
}

void IOBlockStorageDriver::deblockRequestCompletion( void *   target,
                                                     void *   parameter,
                                                     IOReturn status,
                                                     UInt64   actualByteCount )
{
    //
    // This is the completion routine for the aligned deblocker subrequests.
    // It verifies the success of the just-completed stage,  transitions to
    // the next stage, then builds and issues a transfer for the next stage.
    //

    thread_call_t          callback;
    IODeblocker *          deblocker = (IODeblocker          *) parameter;
    IOBlockStorageDriver * driver    = (IOBlockStorageDriver *) target;

    // Allocate a thread callback.

    callback = deblocker->getThreadCallback();

#if TARGET_OS_OSX
    if ( callback == 0 )
    {
        if ( deblocker->setThreadCallback(deblockRequestExecute) == false )
        {
            status = kIOReturnNoMemory;
        }
    }
#endif /* TARGET_OS_OSX */

    // Determine whether an error occurred or whether there are no more stages.

    if ( actualByteCount            < deblocker->getLength() ||
         status                    != kIOReturnSuccess       ||
         deblocker->getNextStage() == false                  )
    {
        IOStorageCompletion * completion;

        // Unlock the write-lock in order to allow the next write to proceed.

        if ( deblocker->getRequestBuffer()->getDirection() == kIODirectionOut )
        {
            IOLockLock(driver->_deblockRequestWriteLock);

            driver->_deblockRequestWriteLockCount--;

            if ( driver->_deblockRequestWriteLockCount > 0 )
            {
                IOLockWakeup( /* lock  */ driver->_deblockRequestWriteLock,
                              /* event */ driver->_deblockRequestWriteLock,
                              /* one   */ true );
            }

            IOLockUnlock(driver->_deblockRequestWriteLock);
        }

        // Obtain the completion information for the original request, taking
        // into account the actual byte count of the current stage. 

        completion = deblocker->getRequestCompletion(&actualByteCount);

        // Complete the original request.

        IOStorage::complete(completion, status, actualByteCount);

        // Release our resources.

        deblocker->release();
    }
    else
    {
        // Execute the transfer (for the next stage).

        if ( callback )
        {
            thread_call_enter1(callback, driver);
        }
        else
        {
            deblockRequestExecute(deblocker, driver);
        }
    }
}

void IOBlockStorageDriver::deblockRequestExecute(void * parameter, void * target)
{
    //
    // Execute the transfer (for the next stage).
    //

    IOStorageAttributes *  attributes;
    UInt64                 byteStart;
    IOStorageCompletion    completion;
    Context *              context;
    IODeblocker *          deblocker = (IODeblocker          *) parameter;
    IOBlockStorageDriver * driver    = (IOBlockStorageDriver *) target;

    attributes = deblocker->getRequestAttributes();

    byteStart = deblocker->getByteStart();

    completion.target    = driver;
    completion.action    = deblockRequestCompletion;
    completion.parameter = deblocker;

    context = (Context *) deblocker->getRequestContext();

    driver->breakUpRequest(byteStart, deblocker, attributes, &completion, context);
}

// -----------------------------------------------------------------------------
// Breaker Implementation

class IOBreaker : public IOSubMemoryDescriptor
{
    OSDeclareDefaultStructors(IOBreaker);

protected:

    UInt64                     _breakSize;

    UInt64                     _maximumBlockCount;
    UInt64                     _maximumByteCount;
    UInt64                     _maximumSegmentCount;
    UInt64                     _maximumSegmentByteCount;
    UInt64                     _minimumSegmentAlignmentByteCount;
    UInt64                     _maximumSegmentWidthByteCount;

    UInt64                     _requestBlockSize;
    IOMemoryDescriptor *       _requestBuffer;
    IOStorageAttributes        _requestAttributes;
    IOStorageCompletion        _requestCompletion;
    void *                     _requestContext;
    UInt64                     _requestCount;
    UInt64                     _requestStart;

    UInt64                     _byteStart;

    thread_call_t              _threadCallback;

    virtual void free() APPLE_KEXT_OVERRIDE;

public:

    static UInt64 getBreakSize(
                              UInt64               withMaximumBlockCount,
                              UInt64               withMaximumByteCount,
                              UInt64               withMaximumSegmentCount,
                              UInt64               withMaximumSegmentByteCount,
                              UInt64               withMinimumSegmentAlignmentByteCount,
                              UInt64               withMaximumSegmentWidthByteCount,
                              UInt64               withRequestBlockSize,
                              IOMemoryDescriptor * withRequestBuffer,
                              UInt64               withRequestBufferOffset );

    static IOBreaker * withBreakSize(
                              UInt64                breakSize,
                              UInt64                withMaximumBlockCount,
                              UInt64                withMaximumByteCount,
                              UInt64                withMaximumSegmentCount,
                              UInt64                withMaximumSegmentByteCount,
                              UInt64                withMinimumSegmentAlignmentByteCount,
                              UInt64                withMaximumSegmentWidthByteCount,
                              UInt64                withRequestBlockSize,
                              UInt64                withRequestStart,
                              IOMemoryDescriptor *  withRequestBuffer,
                              IOStorageAttributes * withRequestAttributes,
                              IOStorageCompletion * withRequestCompletion,
                              void *                withRequestContext );

    virtual bool initWithBreakSize(
                              UInt64                breakSize,
                              UInt64                withMaximumBlockCount,
                              UInt64                withMaximumByteCount,
                              UInt64                withMaximumSegmentCount,
                              UInt64                withMaximumSegmentByteCount,
                              UInt64                withMinimumSegmentAlignmentByteCount,
                              UInt64                withMaximumSegmentWidthByteCount,
                              UInt64                withRequestBlockSize,
                              UInt64                withRequestStart,
                              IOMemoryDescriptor *  withRequestBuffer,
                              IOStorageAttributes * withRequestAttributes,
                              IOStorageCompletion * withRequestCompletion,
                              void *                withRequestContext );

    virtual bool getNextStage();

    virtual IOStorageAttributes * getRequestAttributes();

    virtual IOStorageCompletion * getRequestCompletion(UInt64 * actualByteCount);

    virtual IOMemoryDescriptor * getRequestBuffer();

    virtual void * getRequestContext();

    virtual UInt64 getByteStart();

    virtual thread_call_t getThreadCallback();

    virtual bool setThreadCallback(thread_call_func_t callback);
};

#undef  super
#define super IOSubMemoryDescriptor
OSDefineMetaClassAndStructors(IOBreaker, IOSubMemoryDescriptor)

UInt64 IOBreaker::getBreakSize(
                              UInt64               withMaximumBlockCount,
                              UInt64               withMaximumByteCount,
                              UInt64               withMaximumSegmentCount,
                              UInt64               withMaximumSegmentByteCount,
                              UInt64               withMinimumSegmentAlignmentByteCount,
                              UInt64               withMaximumSegmentWidthByteCount,
                              UInt64               withRequestBlockSize,
                              IOMemoryDescriptor * withRequestBuffer,
                              UInt64               withRequestBufferOffset )
{
    //
    // Determine where the next break point is given our constraints.
    //

    UInt64               breakSize    = 0;
    IOMemoryDescriptor * buffer       = withRequestBuffer;
    IOByteCount          bufferLength = withRequestBuffer->getLength();
    IOByteCount          bufferOffset = withRequestBufferOffset;
    addr64_t             chunk        = 0;
    IOByteCount          chunkSize    = 0;
    addr64_t             segment      = 0;
    UInt32               segmentCount = 0;
    IOByteCount          segmentSize  = 0;

    // Prepare segment alignment mask.

    if ( withMinimumSegmentAlignmentByteCount )
    {
        withMinimumSegmentAlignmentByteCount--;
    }

    // Constrain block count.

    if ( withMaximumBlockCount )
    {
        UInt64 blockCountInBytes;

        blockCountInBytes = withMaximumBlockCount * withRequestBlockSize;

        if ( withMaximumByteCount )
        {
            withMaximumByteCount = min(blockCountInBytes, withMaximumByteCount);
        }
        else
        {
            withMaximumByteCount = blockCountInBytes;
        }
    }

    // Scan the buffer's segments.

    while ( segment || bufferOffset < bufferLength ) 
    {
        // Obtain a new segment.

        if ( segment == 0 )
        {
            segment = buffer->getPhysicalSegment(bufferOffset, &segmentSize, 0);

            assert(segment);
            assert(segmentSize);

            bufferOffset += segmentSize;
        }

        // Fold in a segment.

        if ( chunk == 0 )
        {
            breakSize  += segmentSize;

            chunk       = segment;
            chunkSize   = segmentSize;

            segment     = 0;
            segmentSize = 0;

            segmentCount++;
        }
		else if ( chunk + chunkSize == segment )
        {
            breakSize  += segmentSize;
            chunkSize  += segmentSize;

            segment     = 0;
            segmentSize = 0;
        }

        // Trim a complete segment.

        if ( segment == 0 )
        {
            // Constrain segment byte count.

            if ( withMaximumSegmentByteCount )
            {
                if ( chunkSize > withMaximumSegmentByteCount )
                {
                    segmentSize = chunkSize - withMaximumSegmentByteCount;
                }
            }

            // Constrain segment alignment byte count.

            if ( withMinimumSegmentAlignmentByteCount )
            {
                if ( ( chunk & withMinimumSegmentAlignmentByteCount ) || ( chunkSize & withMinimumSegmentAlignmentByteCount ) )
                {
                    if ( chunkSize > PAGE_SIZE )
                    {
                        segmentSize = max(chunkSize - PAGE_SIZE, segmentSize);
                    }
                }
            }

            // Constrain segment width byte count.

            if ( withMaximumSegmentWidthByteCount )
            {
                if ( chunk >= withMaximumSegmentWidthByteCount )
                {
                    if ( chunkSize > PAGE_SIZE )
                    {
                        segmentSize = max(chunkSize - PAGE_SIZE, segmentSize);
                    }
                }
                else if ( chunk + chunkSize > withMaximumSegmentWidthByteCount )
                {
                    segmentSize = max(chunk + chunkSize - withMaximumSegmentWidthByteCount, segmentSize);
                }
            }

            if ( segmentSize )
            {
                segment     = chunk + chunkSize - segmentSize;

                breakSize  -= segmentSize;
                chunkSize  -= segmentSize;
            }

            // Constrain byte count.

            if ( withMaximumByteCount )
            {
                if ( breakSize >= withMaximumByteCount )
                {
                    breakSize = withMaximumByteCount;
                    break;
                }
            }
        }

        // Commit a complete segment.

        if ( segment )
        {
            // Constrain segment count.

            if ( withMaximumSegmentCount )
            {
                if ( segmentCount == withMaximumSegmentCount )
                {
                    break;
                }
            }

            chunk     = 0;
            chunkSize = 0;
        }
    }

    breakSize = IOTrunc(breakSize, withRequestBlockSize);

    return breakSize;
}

IOBreaker * IOBreaker::withBreakSize(
                              UInt64                breakSize,
                              UInt64                withMaximumBlockCount,
                              UInt64                withMaximumByteCount,
                              UInt64                withMaximumSegmentCount,
                              UInt64                withMaximumSegmentByteCount,
                              UInt64                withMinimumSegmentAlignmentByteCount,
                              UInt64                withMaximumSegmentWidthByteCount,
                              UInt64                withRequestBlockSize,
                              UInt64                withRequestStart,
                              IOMemoryDescriptor *  withRequestBuffer,
                              IOStorageAttributes * withRequestAttributes,
                              IOStorageCompletion * withRequestCompletion,
                              void *                withRequestContext )
{
    //
    // Create a new IOBreaker.
    //

    IOBreaker * me = new IOBreaker;

    if ( me && me->initWithBreakSize(
              /* breakSize                            */ breakSize,
              /* withMaximumBlockCount                */ withMaximumBlockCount,
              /* withMaximumByteCount                 */ withMaximumByteCount,
              /* withMaximumSegmentCount              */ withMaximumSegmentCount,
              /* withMaximumSegmentByteCount          */ withMaximumSegmentByteCount,
              /* withMinimumSegmentAlignmentByteCount */ withMinimumSegmentAlignmentByteCount,
              /* withMaximumSegmentWidthByteCount     */ withMaximumSegmentWidthByteCount,
              /* withRequestBlockSize                 */ withRequestBlockSize,
              /* withRequestStart                     */ withRequestStart,
              /* withRequestBuffer                    */ withRequestBuffer,
              /* withRequestAttributes                */ withRequestAttributes,
              /* withRequestCompletion                */ withRequestCompletion,
              /* withRequestContext                   */ withRequestContext ) == false )
    {
	    me->release();
	    me = 0;
    }

    return me;
}

bool IOBreaker::initWithBreakSize(
                              UInt64                breakSize,
                              UInt64                withMaximumBlockCount,
                              UInt64                withMaximumByteCount,
                              UInt64                withMaximumSegmentCount,
                              UInt64                withMaximumSegmentByteCount,
                              UInt64                withMinimumSegmentAlignmentByteCount,
                              UInt64                withMaximumSegmentWidthByteCount,
                              UInt64                withRequestBlockSize,
                              UInt64                withRequestStart,
                              IOMemoryDescriptor *  withRequestBuffer,
                              IOStorageAttributes * withRequestAttributes,
                              IOStorageCompletion * withRequestCompletion,
                              void *                withRequestContext )
{
    //
    // Initialize an IOBreaker.
    //

    // Ask our superclass' opinion.

    if ( super::initSubRange( 
              /* parent        */ withRequestBuffer, 
              /* withOffset    */ 0,
              /* withLength    */ withRequestBuffer->getLength(),
              /* withDirection */ withRequestBuffer->getDirection() ) == false )
    {
        return false;
    }

    // Initialize our minimal state.

    _breakSize                        = breakSize;
    _length                           = 0;

    _maximumBlockCount                = withMaximumBlockCount;
    _maximumByteCount                 = withMaximumByteCount;
    _maximumSegmentCount              = withMaximumSegmentCount;
    _maximumSegmentByteCount          = withMaximumSegmentByteCount;
    _minimumSegmentAlignmentByteCount = withMinimumSegmentAlignmentByteCount;
    _maximumSegmentWidthByteCount     = withMaximumSegmentWidthByteCount;

    _requestBlockSize                 = withRequestBlockSize;
    _requestBuffer                    = withRequestBuffer;
    _requestBuffer->retain();

    if (withRequestAttributes)  _requestAttributes = *withRequestAttributes;
    if (withRequestCompletion)  _requestCompletion = *withRequestCompletion;

    _requestContext                   = withRequestContext;
    _requestCount                     = withRequestBuffer->getLength();
    _requestStart                     = withRequestStart;

    return true;
}

void IOBreaker::free()
{
    //
    // Free all of this object's outstanding resources.
    //

    if ( _requestBuffer )  _requestBuffer->release();
    if ( _threadCallback )  thread_call_free(_threadCallback);

    super::free();
}

bool IOBreaker::getNextStage()
{
    //
    // Obtain the next stage of the transfer.   The transfer buffer will be the
    // breaker object itself and the byte start will be returned in getByteStart().
    //
    // This method must not be called if the current stage failed with an error
    // or a short byte count, but instead getRequestCompletion() must be called
    // to adjust the status and actual byte count (with respect to the original
    // request) and return the original request's completion routine.  The same
    // call to getRequestCompletion() should also be done if the getNextStage()
    // method returns false.
    //

    if ( _start + _length < _requestCount )
    {
        _start += _length;
        _length = min(_breakSize, _requestCount - _start);

        _breakSize = getBreakSize(
              /* withMaximumBlockCount                */ _maximumBlockCount,
              /* withMaximumByteCount                 */ _maximumByteCount,
              /* withMaximumSegmentCount              */ _maximumSegmentCount,
              /* withMaximumSegmentByteCount          */ _maximumSegmentByteCount,
              /* withMinimumSegmentAlignmentByteCount */ _minimumSegmentAlignmentByteCount,
              /* withMaximumSegmentWidthByteCount     */ _maximumSegmentWidthByteCount,
              /* withRequestBlockSize                 */ _requestBlockSize,
              /* withRequestBuffer                    */ _requestBuffer,
              /* withRequestBufferOffset              */ _start + _length );
    }
    else
    {
        return false;
    }

    _byteStart = _requestStart + _start;

    return true;
}

IOStorageAttributes * IOBreaker::getRequestAttributes()
{
    //
    // Obtain the attributes for the original request. 
    //

    return &_requestAttributes;
}

IOStorageCompletion * IOBreaker::getRequestCompletion(UInt64 * actualByteCount)
{
    //
    // Obtain the completion information for the original request, taking
    // into account the actual byte count of the current stage. 
    //

    *actualByteCount += _start;

    return &_requestCompletion;
}

IOMemoryDescriptor * IOBreaker::getRequestBuffer()
{
    //
    // Obtain the buffer for the original request. 
    //

    return _requestBuffer;
}

void * IOBreaker::getRequestContext()
{
    //
    // Obtain the context for the original request.
    //

    return _requestContext;
}

UInt64 IOBreaker::getByteStart()
{
    //
    // Obtain the byte start for the current stage.
    //

    return _byteStart;
}

thread_call_t IOBreaker::getThreadCallback()
{
    //
    // Obtain the thread callback.
    //

    return _threadCallback;
}

bool IOBreaker::setThreadCallback(thread_call_func_t callback)
{
    //
    // Allocate a thread callback.
    //

    _threadCallback = thread_call_allocate(callback, this);

    return _threadCallback ? true : false;
}

void IOBlockStorageDriver::breakUpRequest(
                                     UInt64                          byteStart,
                                     IOMemoryDescriptor *            buffer,
                                     IOStorageAttributes *           attributes,
                                     IOStorageCompletion *           completion,
                                     IOBlockStorageDriver::Context * context )
{
    //
    // The breakUpRequest method checks to see if the incoming request exceeds
    // our transfer constraints, and if so, breaks up the request into smaller
    // sub-requests.
    //
    // This method is part of a sequence of methods invoked for each read/write
    // request.  The first is prepareRequest, which allocates and prepares some
    // context for the transfer; the second is deblockRequest, which aligns the
    // transfer at the media's block boundaries; third is breakUpRequest, which
    // breaks up the transfer into multiple sub-transfers when certain hardware
    // constraints are exceeded; fourth is executeRequest, which implements the
    // actual transfer from the block storage device.
    //
    // The current implementation of breakUpRequest is asynchronous.
    //

    IOBreaker * breaker;
    UInt64      breakSize;

    // State our assumptions.

    assert((byteStart           % context->block.size) == 0);
    assert((buffer->getLength() % context->block.size) == 0);

    // Determine the transfer constraint, based on direction.

    if ( buffer->getDirection() == kIODirectionIn )
    {
        breakSize = IOBreaker::getBreakSize(
              /* withMaximumBlockCount                */ _maxReadBlockTransfer,
              /* withMaximumByteCount                 */ _maxReadByteTransfer,
              /* withMaximumSegmentCount              */ _maxReadSegmentTransfer,
              /* withMaximumSegmentByteCount          */ _maxReadSegmentByteTransfer,
              /* withMinimumSegmentAlignmentByteCount */ _minSegmentAlignmentByteTransfer,
              /* withMaximumSegmentWidthByteCount     */ _maxSegmentWidthByteTransfer,
              /* withRequestBlockSize                 */ context->block.size,
              /* withRequestBuffer                    */ buffer,
              /* withRequestBufferOffset              */ 0 );
    }
    else
    {
        breakSize = IOBreaker::getBreakSize(
              /* withMaximumBlockCount                */ _maxWriteBlockTransfer,
              /* withMaximumByteCount                 */ _maxWriteByteTransfer,
              /* withMaximumSegmentCount              */ _maxWriteSegmentTransfer,
              /* withMaximumSegmentByteCount          */ _maxWriteSegmentByteTransfer,
              /* withMinimumSegmentAlignmentByteCount */ _minSegmentAlignmentByteTransfer,
              /* withMaximumSegmentWidthByteCount     */ _maxSegmentWidthByteTransfer,
              /* withRequestBlockSize                 */ context->block.size,
              /* withRequestBuffer                    */ buffer,
              /* withRequestBufferOffset              */ 0 );
    }

    if ( breakSize == 0 )
    {
        complete(completion, kIOReturnDMAError);
        return;
    }

    // If the request doesn't exceed our transfer constaints, we do
    // short-circuit the break-up and call executeRequest directly.

    if ( buffer->getLength() <= breakSize )
    {
        executeRequest(byteStart, buffer, attributes, completion, context);
        return;
    }

    // Build a breaker object.

    if ( buffer->getDirection() == kIODirectionIn )
    {
        breaker = IOBreaker::withBreakSize(
              /* breakSize                            */ breakSize,
              /* withMaximumBlockCount                */ _maxReadBlockTransfer,
              /* withMaximumByteCount                 */ _maxReadByteTransfer,
              /* withMaximumSegmentCount              */ _maxReadSegmentTransfer,
              /* withMaximumSegmentByteCount          */ _maxReadSegmentByteTransfer,
              /* withMinimumSegmentAlignmentByteCount */ _minSegmentAlignmentByteTransfer,
              /* withMaximumSegmentWidthByteCount     */ _maxSegmentWidthByteTransfer,
              /* withRequestBlockSize                 */ context->block.size,
              /* withRequestStart                     */ byteStart,
              /* withRequestBuffer                    */ buffer,
              /* withRequestAttributes                */ attributes,
              /* withRequestCompletion                */ completion,
              /* withRequestContext                   */ context );
    }
    else
    {
        breaker = IOBreaker::withBreakSize(
              /* breakSize                            */ breakSize,
              /* withMaximumBlockCount                */ _maxWriteBlockTransfer,
              /* withMaximumByteCount                 */ _maxWriteByteTransfer,
              /* withMaximumSegmentCount              */ _maxWriteSegmentTransfer,
              /* withMaximumSegmentByteCount          */ _maxWriteSegmentByteTransfer,
              /* withMinimumSegmentAlignmentByteCount */ _minSegmentAlignmentByteTransfer,
              /* withMaximumSegmentWidthByteCount     */ _maxSegmentWidthByteTransfer,
              /* withRequestBlockSize                 */ context->block.size,
              /* withRequestStart                     */ byteStart,
              /* withRequestBuffer                    */ buffer,
              /* withRequestAttributes                */ attributes,
              /* withRequestCompletion                */ completion,
              /* withRequestContext                   */ context );
    }

    if ( breaker == 0 )
    {
        complete(completion, kIOReturnNoMemory);
        return;
    }

    // Execute the transfer (for the next stage).

    breakUpRequestCompletion(this, breaker, kIOReturnSuccess, 0);
}

void IOBlockStorageDriver::breakUpRequestCompletion( void *   target,
                                                     void *   parameter,
                                                     IOReturn status,
                                                     UInt64   actualByteCount )
{
    //
    // This is the completion routine for the broken-up breaker subrequests.
    // It verifies the success of the just-completed stage,  transitions to
    // the next stage, then builds and issues a transfer for the next stage.
    //

    IOBreaker *            breaker = (IOBreaker            *) parameter;
    thread_call_t          callback;
    IOBlockStorageDriver * driver  = (IOBlockStorageDriver *) target;

    // Allocate a thread callback.

    callback = breaker->getThreadCallback();

#if TARGET_OS_OSX
    if ( callback == 0 )
    {
        if ( breaker->setThreadCallback(breakUpRequestExecute) == false )
        {
            status = kIOReturnNoMemory;
        }
    }
#endif /* TARGET_OS_OSX */

    // Determine whether an error occurred or whether there are no more stages.

    if ( actualByteCount          < breaker->getLength() ||
         status                  != kIOReturnSuccess     ||
         breaker->getNextStage() == false                )
    {
        IOStorageCompletion * completion;

        // Obtain the completion information for the original request, taking
        // into account the actual byte count of the current stage. 

        completion = breaker->getRequestCompletion(&actualByteCount);

        // Complete the original request.

        IOStorage::complete(completion, status, actualByteCount);

        // Release our resources.

        breaker->release();
    }
    else
    {
        // Execute the transfer (for the next stage).

        if ( callback )
        {
            thread_call_enter1(callback, driver);
        }
        else
        {
            breakUpRequestExecute(breaker, driver);
        }
    }
}

void IOBlockStorageDriver::breakUpRequestExecute(void * parameter, void * target)
{
    //
    // Execute the transfer (for the next stage).
    //

    IOStorageAttributes *  attributes;
    IOBreaker *            breaker = (IOBreaker            *) parameter;
    UInt64                 byteStart;
    IOStorageCompletion    completion;
    Context *              context;
    IOBlockStorageDriver * driver  = (IOBlockStorageDriver *) target;

    attributes = breaker->getRequestAttributes();

    byteStart = breaker->getByteStart();

    completion.target    = driver;
    completion.action    = breakUpRequestCompletion;
    completion.parameter = breaker;

    context = (Context *) breaker->getRequestContext();

    driver->executeRequest(byteStart, breaker, attributes, &completion, context);
}

void IOBlockStorageDriver::prepareRequest(UInt64                byteStart,
                                          IOMemoryDescriptor *  buffer,
                                          IOStorageAttributes * attributes,
                                          IOStorageCompletion * completion)
{
    //
    // The prepareRequest method allocates and prepares state for the transfer.
    //
    // This method is part of a sequence of methods invoked for each read/write
    // request.  The first is prepareRequest, which allocates and prepares some
    // context for the transfer; the second is deblockRequest, which aligns the
    // transfer at the media's block boundaries; third is breakUpRequest, which
    // breaks up the transfer into multiple sub-transfers when certain hardware
    // constraints are exceeded; fourth is executeRequest, which implements the
    // actual transfer from the block storage device.
    //

    IOStorageCompletion completionOut;
    Context *           context;

    // Determine whether the attributes are valid.

    if (attributes)
    {
        if ((attributes->options & kIOStorageOptionReserved))
        {
            complete(completion, kIOReturnBadArgument);
            return;
        }

        if (attributes->reserved0024)
        {
            complete(completion, kIOReturnBadArgument);
            return;
        }

        if (attributes->reserved0032 || attributes->reserved0064)
        {
            complete(completion, kIOReturnBadArgument);
            return;
        }
    }

    // Allocate a context structure to hold some of our state.

    context = allocateContext();

    if (context == 0)
    {
        complete(completion, kIOReturnNoMemory);
        return;
    }
    
    // Fill in the context structure with some of our state.

    context->block.size = (UInt32) getMediaBlockSize();
    context->block.type = kBlockTypeStandard;

    context->request.byteStart = byteStart;
    context->request.buffer    = buffer;
    context->request.buffer->retain();

    if (attributes)  context->request.attributes = *attributes;
    if (completion)  context->request.completion = *completion;

    clock_get_uptime(&context->timeStart);

    retain();
    completionOut.target    = this;
    completionOut.action    = prepareRequestCompletion;
    completionOut.parameter = context;

    // Deblock the transfer.

    deblockRequest(byteStart, buffer, attributes, &completionOut, context);
}

IOReturn
IOBlockStorageDriver::requestIdle(void)
{
    return(getProvider()->requestIdle());
}

OSMetaClassDefineReservedUnused(IOBlockStorageDriver,  0);
OSMetaClassDefineReservedUnused(IOBlockStorageDriver,  1);
OSMetaClassDefineReservedUnused(IOBlockStorageDriver,  2);
OSMetaClassDefineReservedUnused(IOBlockStorageDriver,  3);
OSMetaClassDefineReservedUnused(IOBlockStorageDriver,  4);
OSMetaClassDefineReservedUnused(IOBlockStorageDriver,  5);
OSMetaClassDefineReservedUnused(IOBlockStorageDriver,  6);
OSMetaClassDefineReservedUnused(IOBlockStorageDriver,  7);
OSMetaClassDefineReservedUnused(IOBlockStorageDriver,  8);
OSMetaClassDefineReservedUnused(IOBlockStorageDriver,  9);
OSMetaClassDefineReservedUnused(IOBlockStorageDriver, 10);
OSMetaClassDefineReservedUnused(IOBlockStorageDriver, 11);
OSMetaClassDefineReservedUnused(IOBlockStorageDriver, 12);
OSMetaClassDefineReservedUnused(IOBlockStorageDriver, 13);
OSMetaClassDefineReservedUnused(IOBlockStorageDriver, 14);
OSMetaClassDefineReservedUnused(IOBlockStorageDriver, 15);
OSMetaClassDefineReservedUnused(IOBlockStorageDriver, 16);
OSMetaClassDefineReservedUnused(IOBlockStorageDriver, 17);
OSMetaClassDefineReservedUnused(IOBlockStorageDriver, 18);
OSMetaClassDefineReservedUnused(IOBlockStorageDriver, 19);
OSMetaClassDefineReservedUnused(IOBlockStorageDriver, 20);
OSMetaClassDefineReservedUnused(IOBlockStorageDriver, 21);
OSMetaClassDefineReservedUnused(IOBlockStorageDriver, 22);
OSMetaClassDefineReservedUnused(IOBlockStorageDriver, 23);
OSMetaClassDefineReservedUnused(IOBlockStorageDriver, 24);
OSMetaClassDefineReservedUnused(IOBlockStorageDriver, 25);
OSMetaClassDefineReservedUnused(IOBlockStorageDriver, 26);
OSMetaClassDefineReservedUnused(IOBlockStorageDriver, 27);
OSMetaClassDefineReservedUnused(IOBlockStorageDriver, 28);
OSMetaClassDefineReservedUnused(IOBlockStorageDriver, 29);
OSMetaClassDefineReservedUnused(IOBlockStorageDriver, 30);
OSMetaClassDefineReservedUnused(IOBlockStorageDriver, 31);

#if TARGET_OS_OSX && defined(__x86_64__)
extern "C" void _ZN20IOBlockStorageDriver16synchronizeCacheEP9IOService( IOBlockStorageDriver * driver, IOService * client )
{
    driver->synchronize( client, 0, 0 );
}
#endif /* TARGET_OS_OSX && defined(__x86_64__) */
