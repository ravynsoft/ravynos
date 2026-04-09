/*
 * Copyright (c) 1998-2006 Apple Computer, Inc. All rights reserved.
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

#include <IOKit/IOLib.h>
#include <IOKit/IODeviceTreeSupport.h>

#include "AppleIntelPIIXATARoot.h"
#include "AppleIntelPIIXATAChannel.h"
#include "AppleIntelPIIXATAKeys.h"
#include "AppleIntelPIIXATAHW.h"

#define super IOService
OSDefineMetaClassAndStructors( AppleIntelPIIXATARoot, IOService )

/*
 * Given the map value (Bit 2:0) in the 'Port Mapping Register'
 * for the Serial ATA controller, return the (logical) channel
 * configuration for both primary and secondary channels.
 */
static const UInt8 gICH5ChannelModeMap[8 /*map value*/][2 /*channel*/] =
{
    /* Non-combined SATA only modes */
    { kChannelModeSATAPort0,  kChannelModeSATAPort1  },  /* 000 */
    { kChannelModeSATAPort1,  kChannelModeSATAPort0  },  /* 001 */

    /* Undefined modes in ICH5 PRM (April 2003) */
    { kChannelModeDisabled,   kChannelModeDisabled   },  /* 010 */
    { kChannelModeDisabled,   kChannelModeDisabled   },  /* 011 */

    /* Combined SATA/PATA modes */
    { kChannelModeSATAPort01, kChannelModePATA       },  /* 100 */
    { kChannelModeSATAPort10, kChannelModePATA       },  /* 101 */
    { kChannelModePATA,       kChannelModeSATAPort01 },  /* 110 */
    { kChannelModePATA,       kChannelModeSATAPort10 }   /* 111 */
};

/*
 * ICH6 / ICH6-R
 */
static const UInt8 gICH6ChannelModeMap[4 /*map value*/][2 /*channel*/] =
{
    /* Non-combined SATA only modes */
    { kChannelModeSATAPort02, kChannelModeSATAPort13 },  /* 00 */

    /* Combined SATA/PATA modes */
    { kChannelModePATA,       kChannelModeSATAPort13 },  /* 01 */
    { kChannelModeSATAPort02, kChannelModePATA       },  /* 10 */

    /* Reserved mode */
    { kChannelModeDisabled,   kChannelModeDisabled   }   /* 11 */
};

/*
 * ICH6-M (does not implement SATA ports 1 and 3)
 */
static const UInt8 gICH6MChannelModeMap[4 /*map value*/][2 /*channel*/] =
{
    /* Non-combined SATA only modes */
    { kChannelModeSATAPort02, kChannelModeDisabled   },  /* 00 */

    /* Combined SATA/PATA modes */
    { kChannelModePATA,       kChannelModeDisabled   },  /* 01 */
    { kChannelModeSATAPort02, kChannelModePATA       },  /* 10 */

    /* Reserved mode */
    { kChannelModeDisabled,   kChannelModeDisabled   }   /* 11 */
};

/*
 * ICH7 / ICH7-R
 */
static const UInt8 gICH7ChannelModeMap[4 /*map value*/][2 /*channel*/] =
{
    /* Non-combined SATA only modes */
    { kChannelModeSATAPort02, kChannelModeSATAPort13 },  /* 00 */

    /* Combined SATA/PATA modes */
    { kChannelModePATA,       kChannelModeSATAPort13 },  /* 01 */
    { kChannelModeSATAPort02, kChannelModePATA       },  /* 10 */

    /* Reserved mode */
    { kChannelModeDisabled,   kChannelModeDisabled   }   /* 11 */
};

/*
 * ICH7-M (does not implement SATA ports 1 and 3)
 */
static const UInt8 gICH7MChannelModeMap[4 /*map value*/][2 /*channel*/] =
{
    /* Non-combined SATA only modes */
    { kChannelModeSATAPort02, kChannelModeDisabled   },  /* 00 */

    /* Combined SATA/PATA modes */
    { kChannelModePATA,       kChannelModeDisabled   },  /* 01 */
    { kChannelModeSATAPort02, kChannelModePATA       },  /* 10 */

    /* Reserved mode */
    { kChannelModeDisabled,   kChannelModeDisabled   }   /* 11 */
};

//---------------------------------------------------------------------------
//
// Probe for PCI device and verify that I/O space decoding is enabled.
//

IOService * AppleIntelPIIXATARoot::probe( IOService * provider,
                                          SInt32 *    score )
{
    IOPCIDevice * pciDevice;

    kprintf("AppleIntelPIIXATARoot::probe provider=%p class=%s name=%s\n",
            provider,
            provider ? provider->getMetaClass()->getClassName() : "<null>",
            provider ? provider->getName() : "<null>");

    // Let our superclass probe first.

    if ( super::probe( provider, score ) == 0 )
    {
        return 0;
    }

    // Verify the provider type.

    pciDevice = OSDynamicCast( IOPCIDevice, provider );
    if ( pciDevice == 0 )
    {
        kprintf("AppleIntelPIIXATARoot::probe: provider is not IOPCIDevice\n");
        return 0;
    }

    // BIOS did not enable I/O space decoding.
    // For now assume the ATA controller is disabled.

    uint16_t command = pciDevice->configRead16( kIOPCIConfigCommand );
    kprintf("AppleIntelPIIXATARoot::probe: cmd=0x%04x\n", command);

    if ( (command &
          kIOPCICommandIOSpace) == 0 )
    {
        kprintf("AppleIntelPIIXATARoot::probe: I/O space disabled\n");
        return 0;
    }

    kprintf("AppleIntelPIIXATARoot::probe: matched\n");

    return this;
}

//---------------------------------------------------------------------------
//
// Start the Root PIIX ATA driver.
//

static void registerClientApplier( IOService * service, void * context )
{
    if ( service ) service->registerService();
}

bool AppleIntelPIIXATARoot::start( IOService * provider )
{
    if ( super::start(provider) != true )
        return false;
    
    // Allocate a mutex to serialize access to PCI config space.
    
    _pciConfigLock = IOLockAlloc();
    if ( _pciConfigLock == 0 )
        return false;

    _provider = OSDynamicCast( IOPCIDevice, provider );
    if ( _provider == 0 )
        return false;

    _provider->retain();

    _nubs = createATAChannelNubs();
    if ( _nubs == 0 )
        return false;

    _openNubs = OSSet::withCapacity( _nubs->getCount() );
    if ( _openNubs == 0 )
        return false;

    // Register channel nubs.

    applyToClients( registerClientApplier, 0 );
    
    kprintf("Successfully started the root PIIX ATA driver\n");

    return true;
}

//---------------------------------------------------------------------------
//
// Release allocated resources before this object is destroyed.
//

void AppleIntelPIIXATARoot::free( void )
{
    if ( _nubs )
    {
        _nubs->release();
        _nubs = 0;
    }

    if ( _openNubs )
    {
        _openNubs->release();
        _openNubs = 0;
    }

    if ( _provider )
    {
        _provider->release();
        _provider = 0;
    }

    if ( _pciConfigLock )
    {
        IOLockFree( _pciConfigLock );
        _pciConfigLock = 0;
    }

    super::free();
}

//---------------------------------------------------------------------------
//
// Locate an entry in the device tree that correspond to the channels
// behind the ATA controller. This allows discovery of the ACPI entry
// for ACPI method evaluation, and also uses the ACPI assigned device
// name for a persistent path to the root device.
//

IORegistryEntry * AppleIntelPIIXATARoot::getDTChannelEntry( int channelID )
{
    IORegistryEntry * entry = 0;
    const char *      location;

    OSIterator * iter = _provider->getChildIterator( gIODTPlane );
    if (iter == 0) return 0;

    while (( entry = (IORegistryEntry *) iter->getNextObject() ))
    {
        location = entry->getLocation();
        if ( location && strtol(location, 0, 10) == channelID )
        {
            entry->retain();
            break;
        }
    }

    iter->release();
    
    return entry;  // retain held on the entry
}

//---------------------------------------------------------------------------
//
// Create nubs based on the channel information in the driver personality.
//

OSSet * AppleIntelPIIXATARoot::createATAChannelNubs( void )
{
    OSSet *           nubSet;
    OSDictionary *    channelInfo;
    IORegistryEntry * dtEntry;
    UInt32            priChannelMode;
    UInt32            secChannelMode;
    UInt8             mapValue = 0;

    do {
        nubSet = OSSet::withCapacity(2);
        if ( nubSet == 0 )
            break;

        if ( _provider->open( this ) != true )
            break;

        priChannelMode = kChannelModePATA;
        secChannelMode = kChannelModePATA;

        // Determine SATA channel mode based on Port Mapping Register.

        if ( getProperty( kSerialATAKey ) == kOSBooleanTrue )
        {
            OSString * hwName;

            hwName = OSDynamicCast(OSString, getProperty(kControllerNameKey));
            mapValue = _provider->configRead8(kPIIX_PCI_MAP);
            setProperty( kPortMappingKey, mapValue, 8 );

            priChannelMode = kChannelModeDisabled;
            secChannelMode = kChannelModeDisabled;

            if (hwName)
            {
                if (hwName->isEqualTo("ICH7-M SATA"))
                {
                    mapValue &= 0x3;
                    priChannelMode = gICH7MChannelModeMap[mapValue][0];
                    secChannelMode = gICH7MChannelModeMap[mapValue][1];
                }
                else if (hwName->isEqualTo("ICH6 SATA") || hwName->isEqualTo("ESB2 SATA"))
                {
                    mapValue &= 0x3;
                    priChannelMode = gICH6ChannelModeMap[mapValue][0];
                    secChannelMode = gICH6ChannelModeMap[mapValue][1];
                }
                else if (hwName->isEqualTo("ICH6-M SATA"))
                {
                    mapValue &= 0x3;
                    priChannelMode = gICH6MChannelModeMap[mapValue][0];
                    secChannelMode = gICH6MChannelModeMap[mapValue][1];
                }
                else if (hwName->isEqualTo("ICH5 SATA"))
                {
                    mapValue &= 0x7;
                    priChannelMode = gICH5ChannelModeMap[mapValue][0];
                    secChannelMode = gICH5ChannelModeMap[mapValue][1];
                }
                else /* if (hwName->isEqualTo("ICH7 SATA")) */
                {
                    mapValue &= 0x3;
                    priChannelMode = gICH7ChannelModeMap[mapValue][0];
                    secChannelMode = gICH7ChannelModeMap[mapValue][1];
                }
            }
        }

        if ( priChannelMode == kChannelModeDisabled &&
             secChannelMode == kChannelModeDisabled )
        {
            kprintf("%s: bad value (%x) in Port Mapping register",
                  getName(), mapValue);
            _provider->close( this );
            break;
        }

        for ( UInt32 channelID = 0; channelID < 2; channelID++ )
        {
            UInt32 channelMode = (channelID ? secChannelMode : priChannelMode);

            // Create a dictionary for the channel info. Use native mode
            // settings if possible, else default to legacy mode.

            channelInfo = createNativeModeChannelInfo( channelID, channelMode );
            if (channelInfo == 0)
                channelInfo = createLegacyModeChannelInfo( channelID, channelMode );
            if (channelInfo == 0)
                continue;

            // Create a nub for each ATA channel.

            AppleIntelPIIXATAChannel * nub = new AppleIntelPIIXATAChannel;
            if ( nub )
            {
                dtEntry = getDTChannelEntry( channelID );

                if ( nub->init( this, channelInfo, dtEntry ) &&
                     nub->attach( this ) )
                {
                    nubSet->setObject( nub );
                }

                if ( dtEntry )
                {
                    dtEntry->release();
                }
                else
                {
                    // Platform did not create a device tree entry for
                    // this ATA channel. Do it here.

                    char channelName[5] = {'C','H','N','_','\0'};

                    channelName[3] = '0' + channelID;
                    nub->setName( channelName );

                    if ( _provider->inPlane(gIODTPlane) )
                    {
                        nub->attachToParent( _provider, gIODTPlane );
                    }
                }

                nub->release();
            }

            channelInfo->release();
        }
        
        _provider->close( this );
    }
    while ( false );

    // Release and invalidate an empty set.

    if ( nubSet && (nubSet->getCount() == 0) )
    {
        nubSet->release();
        nubSet = 0;
    }

    return nubSet;
}

//---------------------------------------------------------------------------

OSDictionary *
AppleIntelPIIXATARoot::createNativeModeChannelInfo( UInt32 ataChannel,
                                                    UInt32 channelMode )
{
    UInt8  pi = _provider->configRead8( kPIIX_PCI_PI );
    UInt16 cmdPort = 0;
    UInt16 ctrPort = 0;

    switch ( ataChannel )
    {
        case kPIIX_CHANNEL_PRIMARY:
            if ((pi & kPIIX_PCI_PRI_NATIVE_MASK) == kPIIX_PCI_PRI_NATIVE_MASK)
            {
                // Primary channel native mode supported and enabled.

                cmdPort = _provider->configRead16( kIOPCIConfigBaseAddress0 );
                ctrPort = _provider->configRead16( kIOPCIConfigBaseAddress1 );
                
                cmdPort &= ~0x1;  // clear PCI I/O space indicator bit
                ctrPort &= ~0x1;
                
                // Programming interface byte indicate that native mode
                // is supported and active, but the controller has been
                // assigned legacy ranges. Force legacy mode configuration
                // which is safest. PCI INT# interrupts are not wired
                // properly for some machines in this state.

                if ( cmdPort == kPIIX_P_CMD_ADDR &&
                     ctrPort == kPIIX_P_CTL_ADDR )
                {
                     cmdPort = ctrPort = 0;
                }
            }
            break;

        case kPIIX_CHANNEL_SECONDARY:
            if ((pi & kPIIX_PCI_SEC_NATIVE_MASK) == kPIIX_PCI_SEC_NATIVE_MASK)
            {
                cmdPort = _provider->configRead16( kIOPCIConfigBaseAddress2 );
                ctrPort = _provider->configRead16( kIOPCIConfigBaseAddress3 );

                cmdPort &= ~0x1;  // clear PCI I/O space indicator bit
                ctrPort &= ~0x1;

                if ( cmdPort == kPIIX_S_CMD_ADDR &&
                     ctrPort == kPIIX_S_CTL_ADDR )
                {
                     cmdPort = ctrPort = 0;
                }
            }
            break;
    }

    if ( cmdPort && ctrPort )
        return createChannelInfo( ataChannel, channelMode, cmdPort, ctrPort,
                     _provider->configRead8( kIOPCIConfigInterruptLine ) );
    else
        return 0;
}

//---------------------------------------------------------------------------

OSDictionary *
AppleIntelPIIXATARoot::createLegacyModeChannelInfo( UInt32 ataChannel,
                                                    UInt32 channelMode )
{
    UInt16  cmdPort = 0;
    UInt16  ctrPort = 0;
    UInt8   irq = 0;

    switch ( ataChannel )
    {
        case kPIIX_CHANNEL_PRIMARY:
            cmdPort = kPIIX_P_CMD_ADDR;
            ctrPort = kPIIX_P_CTL_ADDR;
            irq     = kPIIX_P_IRQ;
            break;
        
        case kPIIX_CHANNEL_SECONDARY:
            cmdPort = kPIIX_S_CMD_ADDR;
            ctrPort = kPIIX_S_CTL_ADDR;
            irq     = kPIIX_S_IRQ;
            break;
    }

    return createChannelInfo( ataChannel, channelMode,
                              cmdPort, ctrPort, irq );
}

//---------------------------------------------------------------------------

OSDictionary *
AppleIntelPIIXATARoot::createChannelInfo( UInt32 ataChannel,
                                          UInt32 channelMode,
                                          UInt16 commandPort,
                                          UInt16 controlPort,
                                          UInt8  interruptVector )
{
    OSDictionary * dict = OSDictionary::withCapacity( 4 );
    OSNumber *     num;

    if ( dict == 0 || commandPort == 0 || controlPort == 0 || 
         interruptVector == 0 || interruptVector == 0xFF )
    {
        if ( dict ) dict->release();
        return 0;
    }

    num = OSNumber::withNumber( ataChannel, 32 );
    if (num)
    {
        dict->setObject( kChannelNumberKey, num );
        num->release();
    }
    
    num = OSNumber::withNumber( commandPort, 16 );
    if (num)
    {
        dict->setObject( kCommandBlockAddressKey, num );
        num->release();
    }

    num = OSNumber::withNumber( controlPort, 16 );
    if (num)
    {
        dict->setObject( kControlBlockAddressKey, num );
        num->release();
    }

    num = OSNumber::withNumber( interruptVector, 8 );
    if (num)
    {
        dict->setObject( kInterruptVectorKey, num );
        num->release();
    }

    num = OSNumber::withNumber( channelMode, 32 );
    if (num)
    {
        dict->setObject( kChannelModeKey, num );
        num->release();
    }

    return dict;
}

//---------------------------------------------------------------------------
//
// Handle an open request from a client. Several clients can call this
// function and hold an open on us.
//

bool AppleIntelPIIXATARoot::handleOpen( IOService *  client,
                                        IOOptionBits options,
                                        void *       arg )
{
    bool ret = true;

    // Reject open request from unknown clients, or if the client
    // already holds an open.

    if ( ( _nubs->containsObject( client ) == false ) ||
         ( _openNubs->containsObject( client ) == true ) )
        return false;

    // First client open will trigger an open to our provider.

    if ( _openNubs->getCount() == 0 )
        ret = _provider->open( this );

    if ( ret )
    {
        _openNubs->setObject( client );
        if ( arg ) *((IOService **) arg) = _provider;
    }

    return ret;
}

//---------------------------------------------------------------------------
//
// Handle a close request from a client.
//

void AppleIntelPIIXATARoot::handleClose( IOService *  client,
                                         IOOptionBits options )
{
    // Reject close request from clients that do not hold an open.

    if ( _openNubs->containsObject( client ) == false ) return;

    _openNubs->removeObject( client );

    // Last client close will trigger a close to our provider.

    if ( _openNubs->getCount() == 0 )
        _provider->close( this );
}

//---------------------------------------------------------------------------
//
// Report if the specified client has an open on us.
//

bool AppleIntelPIIXATARoot::handleIsOpen( const IOService * client ) const
{
    if ( client )
        return _openNubs->containsObject( client );
    else
        return ( _openNubs->getCount() != 0 );
}

//---------------------------------------------------------------------------
//
// Helpers for non 4-byte aligned PCI config space writes.
// WARNING: These will not work on a big-endian machine.
//

void AppleIntelPIIXATARoot::pciConfigWrite8( UInt8 offset,
                                             UInt8 data,
                                             UInt8 mask )
{
    UInt8 u8;

    IOLockLock( _pciConfigLock );

    u8 = _provider->configRead8( offset );
    u8 &= ~mask;
    u8 |= (mask & data);
    _provider->configWrite8( offset, u8 );

    IOLockUnlock( _pciConfigLock );
}

void AppleIntelPIIXATARoot::pciConfigWrite16( UInt8  offset,
                                              UInt16 data,
                                              UInt16 mask )
{
    UInt16 u16;

    IOLockLock( _pciConfigLock );

    u16 = _provider->configRead16( offset );
    u16 &= ~mask;
    u16 |= (mask & data);
    _provider->configWrite16( offset, u16 );

    IOLockUnlock( _pciConfigLock );
}

//---------------------------------------------------------------------------

bool AppleIntelPIIXATARoot::serializeProperties( OSSerialize * s ) const
{
    AppleIntelPIIXATARoot * self;
    char timingString[80];

    if ( _provider )
    {
        // Dump the timing registers for debugging.
        snprintf( timingString, sizeof ( timingString ), "0x40=%08x 0x44=%08x 0x48=%08x 0x54=%04x",
                 (int)_provider->configRead32( 0x40 ),
                 (int)_provider->configRead32( 0x44 ),
                 (int)_provider->configRead32( 0x48 ),
                 (int)_provider->configRead16( 0x54 ) );

        self = (AppleIntelPIIXATARoot *) this;
        self->setProperty( "PCI Timing Registers", timingString );
    }

    return super::serializeProperties(s);
}

//---------------------------------------------------------------------------

struct PCSPortMap
{
    UInt8 enableOffset;
    UInt8 enableMask;
    UInt8 presenceOffset;
    UInt8 presenceMask;
};

static const PCSPortMap gDefaultPortMap[4] =
{
    { kPIIX_PCI_PCS, kPIIX_PCI_PCS_P0E, kPIIX_PCI_PCS, kPIIX_PCI_PCS_P0P },
    { kPIIX_PCI_PCS, kPIIX_PCI_PCS_P1E, kPIIX_PCI_PCS, kPIIX_PCI_PCS_P1P },
    { kPIIX_PCI_PCS, kPIIX_PCI_PCS_P2E, kPIIX_PCI_PCS, kPIIX_PCI_PCS_P2P },
    { kPIIX_PCI_PCS, kPIIX_PCI_PCS_P3E, kPIIX_PCI_PCS, kPIIX_PCI_PCS_P3P }
};

static bool
getPCSPortMapping(
    IORegistryEntry * root,
    UInt32     portNum,
    UInt8 *    enableOffset,
    UInt8 *    enableMask,
    UInt8 *    presenceOffset,
    UInt8 *    presenceMask )
{
    OSObject *  prop;
    OSData *    data;

    const PCSPortMap * portMap = gDefaultPortMap;

    if (!root || (portNum > kSerialATAPort3) ||
        root->getProperty( kSerialATAKey ) != kOSBooleanTrue)
        return false;

    prop = root->copyProperty( kPCSPortMapKey );
    data = OSDynamicCast(OSData, prop);

    if (data && (data->getLength() == sizeof(gDefaultPortMap)))
    {
        portMap = (const PCSPortMap *) data->getBytesNoCopy();
    }

    if (enableOffset)
        *enableOffset = portMap[portNum].enableOffset;
    if (enableMask)
        *enableMask = portMap[portNum].enableMask;
    if (presenceOffset)
        *presenceOffset = portMap[portNum].presenceOffset;
    if (presenceMask)
        *presenceMask = portMap[portNum].presenceMask;

    if (prop) prop->release();

    return true;
}

void AppleIntelPIIXATARoot::setSerialATAPortEnable( UInt32 port, bool enable )
{
    UInt8    offset;
    UInt8    mask;

    if (getPCSPortMapping( this, port, &offset, &mask, NULL, NULL ))
    {
        pciConfigWrite8( offset, enable ? mask : 0, mask );
    }
}

bool AppleIntelPIIXATARoot::getSerialATAPortPresentStatus( UInt32 port )
{
    UInt8    offset;
    UInt8    mask = 0;
    UInt8    pcs  = 0;

    if (getPCSPortMapping( this, port, NULL, NULL, &offset, &mask ))
    {
        pcs = _provider->configRead8( offset );
    }

    return ( pcs & mask );
}
