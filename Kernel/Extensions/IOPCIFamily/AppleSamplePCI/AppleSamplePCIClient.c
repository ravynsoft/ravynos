/*
cc AppleSamplePCIClient.c -o /tmp/sampleclient -framework IOKit -framework CoreFoundation -Wno-four-char-constants -Wall -g -arch ppc -arch i386 -arch ppc64 -arch x86_64
*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <inttypes.h>
#include <IOKit/IOKitLib.h>

#include "AppleSamplePCIShared.h"

void Test( mach_port_t masterPort, io_service_t service );
void TestSharedMemory( io_connect_t connect );


int main( int argc, char * argv[] )
{
    mach_port_t                 masterPort;
    io_iterator_t               iter;
    io_service_t                service;
    kern_return_t               kr;
    CFMutableDictionaryRef      properties;
    CFStringRef                 cfStr;

    kr = IOMasterPort( MACH_PORT_NULL, &masterPort);
    assert( KERN_SUCCESS == kr );

    // Look up the object we wish to open. This example uses simple class
    // matching (IOServiceMatching()) to look up the object that is the
    // AppleSamplePCI driver class instantiated by the kext.

    kr = IOServiceGetMatchingServices( masterPort,
                        IOServiceMatching( kAppleSamplePCIClassName ), &iter);
    assert( KERN_SUCCESS == kr );

    for( ;
      (service = IOIteratorNext(iter));
      IOObjectRelease(service)) {

        io_string_t path;
        kr = IORegistryEntryGetPath(service, kIOServicePlane, path);
        assert( KERN_SUCCESS == kr );
        printf("Found a device of class "kAppleSamplePCIClassName": %s\n", path);

        // print the value of kIONameMatchedKey property, as an example of 
        // getting properties from the registry. Property based access
        // doesn't require a user client connection.

        // grab a copy of the properties
        kr = IORegistryEntryCreateCFProperties( service, &properties,
                    kCFAllocatorDefault, kNilOptions );
        assert( KERN_SUCCESS == kr );
        
        cfStr = CFDictionaryGetValue( properties, CFSTR(kIONameMatchedKey) );
        if( cfStr) {
            const char * c = NULL;
            char * buffer = NULL;
            c = CFStringGetCStringPtr(cfStr, kCFStringEncodingMacRoman);
            if(!c) {
                CFIndex bufferSize = CFStringGetLength(cfStr) + 1;
                buffer = malloc(bufferSize);
                if(buffer) {
                    if(CFStringGetCString(cfStr, buffer, bufferSize, kCFStringEncodingMacRoman))
                        c = buffer;
                }
            }
            if(c)
                printf("it matched on name \"%s\"\n", c);
            if(buffer)
                free(buffer);
        }
        CFRelease( properties );

        // test out the user client
        Test( masterPort, service );
    }
    IOObjectRelease(iter);

    exit(0);
    return(0);
}

#define arrayCnt(var) (sizeof(var) / sizeof(var[0]))

void Test( mach_port_t masterPort, io_service_t service )
{
    kern_return_t               kr;
    io_connect_t                connect;
    size_t                      structureOutputSize;
    AppleSampleStructForMethod2  method2Param;
    AppleSampleResultsForMethod2 method2Results;
    uint32_t                    varStructParam[3] = { 1, 2, 3 };
    IOByteCount                 bigBufferLen;
    uint32_t *                  bigBuffer;

    kr = IOServiceOpen( service, mach_task_self(), kAppleSamplePCIConnectType, &connect );
    assert( KERN_SUCCESS == kr );

    // test a simple struct in/out method
    structureOutputSize = sizeof(varStructParam);


#if MAC_OS_X_VERSION_10_5
    kr = IOConnectCallStructMethod( connect, kAppleSampleMethod1,
                            // inputStructure
                            &varStructParam, sizeof(varStructParam),
                            // ouputStructure
                            &varStructParam, &structureOutputSize );
#else
    kr = IOConnectMethodStructureIStructureO( connect, kAppleSampleMethod1,
                                                sizeof(varStructParam), /* structureInputSize */
                                                &structureOutputSize,   /* structureOutputSize */
                                                &varStructParam,        /* inputStructure */
                                                &varStructParam);       /* ouputStructure */
#endif

    assert( KERN_SUCCESS == kr );
    printf("kAppleSampleMethod1 results 0x%08" PRIx32 ", 0x%08" PRIx32 ", 0x%08" PRIx32 "\n",
            varStructParam[0], varStructParam[1], varStructParam[2]);

    // test shared memory
    TestSharedMemory( connect );

    // test method with out of line memory
    bigBufferLen = 0x4321;
    bigBuffer = malloc( bigBufferLen );

    strcpy( (char *) (bigBuffer + (32 / 4)), "some out of line data");

    method2Param.parameter1   = 0x12345678;
    method2Param.data_pointer = (uintptr_t) bigBuffer;
    method2Param.data_length  = bigBufferLen;

    structureOutputSize = sizeof(method2Results);
#if MAC_OS_X_VERSION_10_5
    kr = IOConnectCallStructMethod( connect, kAppleSampleMethod2,
                            // inputStructure
                            &method2Param, sizeof(method2Param),
                            // ouputStructure
                            &method2Results, &structureOutputSize );
#else
    kr = IOConnectMethodStructureIStructureO( connect, kAppleSampleMethod2,
                                                sizeof(method2Param),   /* structureInputSize */
                                                &structureOutputSize,   /* structureOutputSize */
                                                &method2Param,          /* inputStructure */
                                                &method2Results);       /* ouputStructure */
#endif

    assert( KERN_SUCCESS == kr );
    printf("kAppleSampleMethod2 result 0x%" PRIx64 "\n", method2Results.results1);

    free( bigBuffer );

}

void TestSharedMemory( io_connect_t connect )
{
    kern_return_t               kr;
    AppleSampleSharedMemory *   shared;

#if __LP64__
    mach_vm_address_t           addr;
    mach_vm_size_t              size;
#else
    vm_address_t                addr;
    vm_size_t           size;
#endif
    
    kr = IOConnectMapMemory( connect, kAppleSamplePCIMemoryType1,
                            mach_task_self(), &addr, &size,
                            kIOMapAnywhere | kIOMapDefaultCache );
    assert( KERN_SUCCESS == kr );
    assert( size == sizeof( AppleSampleSharedMemory ));

    shared = (AppleSampleSharedMemory *) addr;

    printf("From AppleSampleSharedMemory: %08" PRIx32 ", %08" PRIx32 ", %08" PRIx32 ", \"%s\"\n",
            shared->field1, shared->field2, shared->field3, shared->string);

    strcpy( shared->string, "some other data" );
}



