
#include <IOKit/IOUserClient.h>
#include <IOKit/IOBufferMemoryDescriptor.h>
#include <IOKit/pci/IOPCIDevice.h>

#if IOMEMORYDESCRIPTOR_SUPPORTS_DMACOMMAND
#include <IOKit/IODMACommand.h>
#endif

#include "AppleSamplePCIShared.h"

class AppleSamplePCI : public IOService
{
    /*
     * Declare the metaclass information that is used for runtime
     * typechecking of IOKit objects.
     */

    OSDeclareDefaultStructors( AppleSamplePCI );

private:
    IOPCIDevice *        fPCIDevice;
    IOMemoryDescriptor * fLowMemory;

public:
    /* IOService overrides */
    virtual bool start( IOService * provider );
    virtual void stop( IOService * provider );
    /* Other methods */
    IOMemoryDescriptor * copyGlobalMemory( void );
    IOReturn generateDMAAddresses( IOMemoryDescriptor * memDesc );
};

class AppleSamplePCIUserClient : public IOUserClient
{
    /*
     * Declare the metaclass information that is used for runtime
     * typechecking of IOKit objects.
     */

    OSDeclareDefaultStructors( AppleSamplePCIUserClient );

private:
    AppleSamplePCI *            fDriver;
    IOBufferMemoryDescriptor *  fClientSharedMemory;
    AppleSampleSharedMemory *   fClientShared;
    task_t                      fTask;
    SInt32                      fOpenCount;

public:
    /* IOService overrides */
    virtual bool start( IOService * provider );
    virtual void stop( IOService * provider );

    /* IOUserClient overrides */
    virtual bool initWithTask( task_t owningTask, void * securityID,
                                                UInt32 type,  OSDictionary * properties );
    virtual IOReturn clientClose( void );

    virtual IOExternalMethod * getTargetAndMethodForIndex(
                                            IOService ** targetP, UInt32 index );

    virtual IOReturn externalMethod( uint32_t selector, IOExternalMethodArguments * arguments,
                                        IOExternalMethodDispatch * dispatch = 0, OSObject * target = 0, void * reference = 0 );


    virtual IOReturn clientMemoryForType( UInt32 type,
                                            IOOptionBits * options,
                                            IOMemoryDescriptor ** memory );
    /* External methods */
    virtual IOReturn method1( UInt32 * dataIn, UInt32 * dataOut,
                                                IOByteCount inputCount, IOByteCount * outputCount );
    virtual IOReturn method2( AppleSampleStructForMethod2 * structIn, 
                                            AppleSampleResultsForMethod2 * structOut,
                                            IOByteCount inputSize, IOByteCount * outputSize );
};

