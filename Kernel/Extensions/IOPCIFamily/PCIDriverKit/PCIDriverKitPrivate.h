//
//  PCIDriverKitPrivate.h
//  PCIDriverKit
//
//  Created by Kevin Strasberg on 10/23/19.
//

#ifndef PCIDriverKitPrivate_h
#define PCIDriverKitPrivate_h

enum kPCIDriverKitMemoryAccessOperation
{
    kPCIDriverKitMemoryAccessOperationDeviceMemoryIndexMask = 0x000000FF,
    kPCIDriverKitMemoryAccessOperationAccessTypeMask        = 0x0000FF00,
    kPCIDriverKitMemoryAccessOperationDeviceRead            = 0x00000100,
    kPCIDriverKitMemoryAccessOperationDeviceWrite           = 0x00000200,
    kPCIDriverKitMemoryAccessOperationConfigurationRead     = 0x00000400,
    kPCIDriverKitMemoryAccessOperationConfigurationWrite    = 0x00000800,
    kPCIDriverKitMemoryAccessOperationIORead                = 0x00001000,
    kPCIDriverKitMemoryAccessOperationIOWrite               = 0x00002000,

    kPCIDriverKitMemoryAccessOperationSizeMask = 0x000F0000,
    kPCIDriverKitMemoryAccessOperation8Bit     = 0x00010000,
    kPCIDriverKitMemoryAccessOperation16Bit    = 0x00020000,
    kPCIDriverKitMemoryAccessOperation32Bit    = 0x00040000,
    kPCIDriverKitMemoryAccessOperation64Bit    = 0x00080000
};

#endif /* PCIDriverKitPrivate_h */
