//
//  PEX8733Definitions.h
//  PEX8733
//
//  Created by Kevin Strasberg on 12/10/19.
//

#ifndef PEX8733Definitions_h
#define PEX8733Definitions_h


#define PEX8733Bit(bit)                     ((uint32_t)(1) << bit)
#define PEX8733Range(start, end)         (~(((uint32_t)(1) << start) - 1) & (((uint32_t)(1) << end) | (((uint32_t)(1) << end) - 1)))

// 4.3.2 PCI Express Memory-Mapped Configuration Space
#define descriptorSpaceOffset(channel)    (0x20000 + 0x400 * (channel))
#define configurationSpaceOffset(channel) (0x21000 + 0x1000 * (channel))

enum
{
    kPEX8733VendorID       = 0x10b5,
    kPEX8733DMADeviceID    = 0x87d0,
    kPEX8733SwitchDeviceID = 0x8733,

    kPEX8733DefaultTestTransferSizeNumPages = 2,
    kPEX8733DefaultDescriptorSize           = 16,

    kPEX8733DefaultNumDMADescriptors          = 4,
    kPEX8733DefaultDMADescriptorPrefetchLimit = 1,
    kPEX8733MemoryIndex                       = 0,
    kPEX8733DefaultChannel                    = 0,

    kPEX8733DMAChannelGlobalControlModeFour = 0,
    kPEX8733DMAChannelGlobalControlModeOne = 1,
    kPEX8733DMAChannelGlobalControlModeTwo = 2,


    // Section 19.12 of PEX_8733 Data Book
    kPEX8733DMAChannelCapabilitiesOffset                             = 0x1F0,
    kPEX8733DMAChannelGlobalControlOffset                            = kPEX8733DMAChannelCapabilitiesOffset + 0x8,
    kPEX8733DMAChannelSourceAddressLowerOffset                       = kPEX8733DMAChannelCapabilitiesOffset + 0x10,
    kPEX8733DMAChannelSourceAddressUpperOffset                       = kPEX8733DMAChannelCapabilitiesOffset + 0x14,
    kPEX8733DMAChannelDestinationAddressLowerOffset                  = kPEX8733DMAChannelCapabilitiesOffset + 0x18,
    kPEX8733DMAChannelDestinationAddressUpperOffset                  = kPEX8733DMAChannelCapabilitiesOffset + 0x1C,
    kPEX8733DMAChannelTransferSizeOffset                             = kPEX8733DMAChannelCapabilitiesOffset + 0x20,
    kPEX8733DMAChannelDescriptorRingAddressLowerOffset               = kPEX8733DMAChannelCapabilitiesOffset + 0x24,
    kPEX8733DMAChannelDescriptorRingAddressUpperOffset               = kPEX8733DMAChannelCapabilitiesOffset + 0x28,
    kPEX8733DMAChannelDescriptorRingNextDescriptorAddressLowerOffset = kPEX8733DMAChannelCapabilitiesOffset + 0x2C,
    kPEX8733DMAChannelDescriptorRingRingSizeOffset                   = kPEX8733DMAChannelCapabilitiesOffset + 0x30,
    kPEX8733DMAChannelDescriptorRingLastDescriptorAddressLowerOffset = kPEX8733DMAChannelCapabilitiesOffset + 0x34,
    kPEX8733DMAChannelDescriptorRingLastDescriptorTransferSizeOffset = kPEX8733DMAChannelCapabilitiesOffset + 0x38,
    kPEX8733DMAChannelMaximumPrefetchLimitOffset                     = kPEX8733DMAChannelCapabilitiesOffset + 0x44,
    kPEX8733DMAChannelControlStatusRegisterOffset                    = kPEX8733DMAChannelCapabilitiesOffset + 0x48,
    kPEX8733DMAChannelInterruptControlStatusRegisterOffset           = kPEX8733DMAChannelCapabilitiesOffset + 0x4C,




    kPEX8733DMAChannelSize         = 0x1000,
    kPEX8733DMAChannelCapabilityID = 0xB,

};



enum
{
    kPEX8733DMAChannelControlStatusGracefulPause                   = PEX8733Bit(0),
    kPEX8733DMAChannelControlStatusAbort                           = PEX8733Bit(1),
    kPEX8733DMAChannelControlStatusCompletionStatusWriteBackEnable = PEX8733Bit(2),
    kPEX8733DMAChannelControlStatusStart                           = PEX8733Bit(3),
    kPEX8733DMAChannelControlStatusRingStopMode                    = PEX8733Bit(4),

    kPEX8733DMAChannelControlStatusDescriptorModeSelectRange     = PEX8733Range(5, 6),
    kPEX8733DMAChannelControlStatusDescriptorModeSelectBlockMode = (0 << 5),
    kPEX8733DMAChannelControlStatusDescriptorModeSelectOnChip    = (1 << 5),
    kPEX8733DMAChannelControlStatusDescriptorModeSelectOffChip   = (2 << 5),

    // Write 1 to clear status bits
    kPEX8733DMAChannelControlStatusRange                    = PEX8733Range(8, 12),
    kPEX8733DMAChannelControlStatusInvalidStatus            = PEX8733Bit(8),
    kPEX8733DMAChannelControlStatusGracefulPauseDoneStatus  = PEX8733Bit(9),
    kPEX8733DMAChannelControlStatusAbortDoneStatus          = PEX8733Bit(10),
    kPEX8733DMAChannelControlStatusImmediatePuaseDoneStatus = PEX8733Bit(12),

    kPEX8733DMAChannelControlStatusDelayForRingWrapAroundContinuous = (0 << 13),
    kPEX8733DMAChannelControlStatusDelayForRingWrapAround1uS        = (1 << 13),
    kPEX8733DMAChannelControlStatusDelayForRingWrapAround2uS        = (2 << 13),
    kPEX8733DMAChannelControlStatusDelayForRingWrapAround8uS        = (3 << 13),
    kPEX8733DMAChannelControlStatusDelayForRingWrapAround32uS       = (4 << 13),
    kPEX8733DMAChannelControlStatusDelayForRingWrapAround128uS      = (5 << 13),
    kPEX8733DMAChannelControlStatusDelayForRingWrapAround512uS      = (6 << 13),
    kPEX8733DMAChannelControlStatusDelayForRingWrapAround1mS        = (7 << 13),

    kPEX8733DMAControlStatusMaximumTransferSize64B  = (0 << 16),
    kPEX8733DMAControlStatusMaximumTransferSize128B = (1 << 16),
    kPEX8733DMAControlStatusMaximumTransferSize256B = (2 << 16),
    kPEX8733DMAControlStatusMaximumTransferSize512B = (3 << 16),
    kPEX8733DMAControlStatusMaximumTransferSize1KB  = (4 << 16),
    kPEX8733DMAControlStatusMaximumTransferSize2KB  = (5 << 16),
    kPEX8733DMAControlStatusMaximumTransferSize4B   = (7 << 16), // potential typo in spec?

    kPEX8733DMAChannelControlStatusTrafficClassMask = (7 << 19),

    kPEX8733DMAChannelControlStatusRelaxedOrderingDescriptorReadRequest  = PEX8733Bit(22),
    kPEX8733DMAChannelControlStatusRelaxedOrderingDataReadRequest        = PEX8733Bit(23),
    kPEX8733DMAChannelControlStatusRelaxedOrderingDescriptorWriteRequest = PEX8733Bit(24),
    kPEX8733DMAChannelControlStatusRelaxedOrderingDataWriteRequest       = PEX8733Bit(25),

    kPEX8733DMAChannelControlStatusNoSnoopDescriptorReadRequest  = PEX8733Bit(26),
    kPEX8733DMAChannelControlStatusNoSnoopDataReadRequest        = PEX8733Bit(27),
    kPEX8733DMAChannelControlStatusNoSnoopDescriptorWriteRequest = PEX8733Bit(28),
    kPEX8733DMAChannelControlStatusNoSnoopDataWriteRequest       = PEX8733Bit(29),

    kPEX8733DMAChannelControlStatusInProgress         = PEX8733Bit(30),
    kPEX8733DMAChannelControlStatusHeaderLoggingValid = PEX8733Bit(31),

};

enum
{
    kPEX8733DMAChannelInterruptControlStatusEnableRange                       = PEX8733Range(0, 5),
    kPEX8733DMAChannelInterruptControlStatusErrorInterruptEnable              = PEX8733Bit(0),
    kPEX8733DMAChannelInterruptControlStatusInvalidDescriptorInterruptEnable  = PEX8733Bit(1),
    kPEX8733DMAChannelInterruptControlStatusAbortDoneInterruptEnable          = PEX8733Bit(3),
    kPEX8733DMAChannelInterruptControlStatusGracefulPauseDoneInterruptEnable  = PEX8733Bit(4),
    kPEX8733DMAChannelInterruptControlStatusImmediatePauseDoneInterruptEnable = PEX8733Bit(5),
    kPEX8733DMAChannelInterruptControlStatusIRQPinInterruptEnable             = PEX8733Bit(15),

    // Write 1 to clear status bits
    kPEX8733DMAChannelInterruptControlStatusInterruptStatusRange              = PEX8733Range(16, 21),
    kPEX8733DMAChannelInterruptControlStatusErrorInterruptStatus              = PEX8733Bit(16),
    kPEX8733DMAChannelInterruptControlStatusInvalidDescriptorInterruptStatus  = PEX8733Bit(17),
    kPEX8733DMAChannelInterruptControlStatusDescriptorDoneInterruptStatus     = PEX8733Bit(18),
    kPEX8733DMAChannelInterruptControlStatusAbortDoneInterruptStatus          = PEX8733Bit(19),
    kPEX8733DMAChannelInterruptControlStatusGracefulPauseDoneInterruptStatus  = PEX8733Bit(20),
    kPEX8733DMAChannelInterruptControlStatusImmediatePauseDoneInterruptStatus = PEX8733Bit(21),
};

enum
{
    kPEX8733DMAChannelTransferSizeRegisterTransferSizeRange    = PEX8733Range(0, 26),
    kPEX8733DMAChannelTransferSizeRegisterWriteAddressConstant = PEX8733Bit(28),
    kPEX8733DMAChannelTransferSizeRegisterReadAddressConstant  = PEX8733Bit(29),
    kPEX8733DMAChannelTransferSizeRegisterDoneInterruptEnable  = PEX8733Bit(30),
    kPEX8733DMAChannelTransferSizeRegisterDMAValid             = PEX8733Bit(31),

};

// Section 8.7.1 of PEX_8733 Data Book
typedef struct DMAStandardDescriptor
{
    uint32_t transferSize                    : 27;
    uint8_t  descriptorFormat                : 1;
    uint8_t  holdWriteAddressConstant        : 1;
    uint8_t  holdReadAddressConstant         : 1;
    uint8_t  interruptWhenDoneWithDescriptor : 1;
    uint8_t  descriptorValid                 : 1;
    uint16_t upperDestinationAddress;
    uint16_t upperSourceAddress;
    uint32_t lowerSourceAddress;
    uint32_t lowerDestinationAddress;
} __attribute__((packed)) DMAStandardDescriptor;

#endif /* PEX8733Definitions_h */
