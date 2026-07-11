/*
 * XHCI register/TRB/context definitions, from Intel xHCI spec rev 1.2.
 *
 * Copyright (C) 2026 ravynOS Project. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef _XHCI_H
#define _XHCI_H

#include <IOKit/IOTypes.h>

/* Capability registers (BAR0 + 0x00) */
#define XHCI_CAPLENGTH   0x00   /* u8: offset to operational regs */
#define XHCI_HCIVERSION  0x02   /* u16 */
#define XHCI_HCSPARAMS1  0x04
#define XHCI_HCSPARAMS2  0x08
#define XHCI_HCSPARAMS3  0x0C
#define XHCI_HCCPARAMS1  0x10
#define XHCI_DBOFF       0x14   /* offset to doorbell array */
#define XHCI_RTSOFF      0x18   /* offset to runtime regs */
#define XHCI_HCCPARAMS2  0x1C

#define XHCI_HCSP1_MAXSLOTS(v)   ((v) & 0xFF)
#define XHCI_HCSP1_MAXINTRS(v)   (((v) >> 8) & 0x7FF)
#define XHCI_HCSP1_MAXPORTS(v)   (((v) >> 24) & 0xFF)

#define XHCI_HCCP1_AC64(v)       ((v) & 1)              /* 64-bit addressing */
#define XHCI_HCCP1_CSZ(v)        (((v) >> 2) & 1)        /* context size: 0=32B 1=64B */
#define XHCI_HCCP1_XECP(v)       (((v) >> 16) & 0xFFFF)  /* extended caps ptr, in 32b words */

/* Operational registers (BAR0 + CAPLENGTH) */
#define XHCI_USBCMD      0x00
#define XHCI_USBSTS      0x04
#define XHCI_PAGESIZE    0x08
#define XHCI_DNCTRL      0x14
#define XHCI_CRCR        0x18   /* 64-bit */
#define XHCI_DCBAAP      0x30   /* 64-bit */
#define XHCI_CONFIG      0x38

#define XHCI_USBCMD_RS    (1U << 0)  /* Run/Stop */
#define XHCI_USBCMD_HCRST (1U << 1)  /* Host Controller Reset */
#define XHCI_USBCMD_INTE  (1U << 2)  /* Interrupter Enable */

#define XHCI_USBSTS_HCH   (1U << 0)  /* HC Halted */
#define XHCI_USBSTS_CNR   (1U << 11) /* Controller Not Ready */

#define XHCI_CRCR_RCS     (1ULL << 0) /* Ring Cycle State */
#define XHCI_CRCR_CS      (1ULL << 1) /* Command Stop */
#define XHCI_CRCR_CA      (1ULL << 2) /* Command Abort */
#define XHCI_CRCR_CRR     (1ULL << 3) /* Command Ring Running */

/* Per-port registers (BAR0 + CAPLENGTH + 0x400 + port*0x10), port is 0-based here
 * (spec numbers ports 1-based; we subtract 1 throughout). */
#define XHCI_PORTREGS_BASE 0x400
#define XHCI_PORTREGS_SIZE 0x10

#define XHCI_PORTSC   0x00
#define XHCI_PORTPMSC 0x04
#define XHCI_PORTLI   0x08

#define XHCI_PORTSC_CCS   (1U << 0)  /* Current Connect Status */
#define XHCI_PORTSC_PED   (1U << 1)  /* Port Enabled/Disabled */
#define XHCI_PORTSC_PR    (1U << 4)  /* Port Reset */
#define XHCI_PORTSC_PLS(v) (((v) >> 5) & 0xF) /* Port Link State */
#define XHCI_PORTSC_PP    (1U << 9)  /* Port Power */
#define XHCI_PORTSC_SPEED(v) (((v) >> 10) & 0xF)
#define XHCI_PORTSC_PRC   (1U << 21) /* Port Reset Change */
#define XHCI_PORTSC_CSC   (1U << 17) /* Connect Status Change */
#define XHCI_PORTSC_CAS   (1U << 24) /* Cold Attach Status: warm reset required */
#define XHCI_PORTSC_WPR   (1U << 31) /* Warm Port Reset (SuperSpeed only) */
/* RW1CS bits that must be preserved-as-0-on-write to avoid clearing on RMW */
#define XHCI_PORTSC_RW1CS (XHCI_PORTSC_CSC | (1U<<18) | (1U<<19) | (1U<<20) | \
                           XHCI_PORTSC_PRC | (1U<<22) | (1U<<23))
/* PED behaves like a write-1-to-clear bit too: per spec, software cannot
 * write a 1 to enable a port, only to disable one that's already enabled.
 * Any "preserve state, clear change bits" write that reads back portsc
 * with PED already 1 and writes that same 1 back will disable the port it
 * just enabled. Bits that must be forced to 0 on every such write. */
#define XHCI_PORTSC_WRITEBACK_CLEAR (XHCI_PORTSC_RW1CS | XHCI_PORTSC_PED)

/* Extended Capabilities (BAR0 + XHCI_HCCP1_XECP(HCCPARAMS1)*4), a linked
 * list of variable-format structures starting at the MMIO base (NOT
 * relative to CAPLENGTH like the operational registers). */
#define XHCI_XECP_ID(dw0)          ((dw0) & 0xFF)
#define XHCI_XECP_NEXT(dw0)        (((dw0) >> 8) & 0xFF)   /* in 32-bit DWORDs, 0 = end of list */
#define XHCI_XECP_ID_LEGACY_SUPPORT      1
#define XHCI_XECP_ID_SUPPORTED_PROTOCOL  2

/* USB Legacy Support capability */
#define XHCI_LEGACY_BIOS_OWNED     (1U << 16)
#define XHCI_LEGACY_OS_OWNED       (1U << 24)
#define XHCI_LEGACY_CONTROL_STATUS 0x04

/* Supported Protocol Capability (xHCI 1.2 spec 7.2), one per USB major
 * revision this controller implements, each covering a contiguous range of
 * root hub ports. */
#define XHCI_SP_MAJOR_REV(dw0)     (((dw0) >> 24) & 0xFF)  /* 2 = USB2, 3 = USB3 */
#define XHCI_SP_PORT_OFFSET(dw2)   ((dw2) & 0xFF)          /* 1-based */
#define XHCI_SP_PORT_COUNT(dw2)    (((dw2) >> 8) & 0xFF)

/* Runtime registers (BAR0 + RTSOFF) */
#define XHCI_RT_IR0        0x20   /* Interrupter Register Set 0 */
#define XHCI_IR_IMAN       0x00
#define XHCI_IR_IMOD       0x04
#define XHCI_IR_ERSTSZ     0x08
#define XHCI_IR_ERSTBA     0x10   /* 64-bit */
#define XHCI_IR_ERDP       0x18   /* 64-bit */

#define XHCI_IMAN_IP   (1U << 0) /* Interrupt Pending */
#define XHCI_IMAN_IE   (1U << 1) /* Interrupt Enable */

#define XHCI_ERDP_EHB  (1ULL << 3) /* Event Handler Busy */

/* Doorbell array (BAR0 + DBOFF), one u32 per slot (0 = command ring) */
#define XHCI_DB_TARGET_CONTROL_EP0  1  /* control endpoint doorbell target */

/* TRB (Transfer Request Block): 16 bytes */
typedef struct {
    UInt64 param;
    UInt32 status;
    UInt32 control;
} __attribute__((packed)) XHCITRB;

#define TRB_CYCLE          (1U << 0)
#define TRB_TC             (1U << 1)  /* Toggle Cycle (link TRB) / Evaluate Next TRB */
#define TRB_ENT             (1U << 1) /* Evaluate Next TRB (transfer TRBs) */
#define TRB_ISP            (1U << 2)  /* Interrupt on Short Packet */
#define TRB_CH             (1U << 4)  /* Chain bit */
#define TRB_IOC            (1U << 5)  /* Interrupt on Completion */
#define TRB_IDT            (1U << 6)  /* Immediate Data (setup stage) */
#define TRB_TYPE_SHIFT     10
#define TRB_TYPE(v)        (((v) >> TRB_TYPE_SHIFT) & 0x3F)
#define TRB_SET_TYPE(t)    ((UInt32)(t) << TRB_TYPE_SHIFT)

#define TRB_SLOT_SHIFT     24
#define TRB_SET_SLOT(s)    ((UInt32)(s) << TRB_SLOT_SHIFT)
#define TRB_GET_SLOT(v)    (((v) >> TRB_SLOT_SHIFT) & 0xFF)
#define TRB_EP_SHIFT       16
#define TRB_SET_EP(e)      ((UInt32)(e) << TRB_EP_SHIFT)

/* TRB types */
enum {
    TRB_TYPE_NORMAL           = 1,
    TRB_TYPE_SETUP_STAGE      = 2,
    TRB_TYPE_DATA_STAGE       = 3,
    TRB_TYPE_STATUS_STAGE     = 4,
    TRB_TYPE_LINK             = 6,
    TRB_TYPE_ENABLE_SLOT      = 9,
    TRB_TYPE_DISABLE_SLOT     = 10,
    TRB_TYPE_ADDRESS_DEVICE   = 11,
    TRB_TYPE_CONFIGURE_EP     = 12,
    TRB_TYPE_EVALUATE_CONTEXT = 13,
    TRB_TYPE_RESET_EP         = 14,
    TRB_TYPE_NOOP_CMD         = 23,
    TRB_TYPE_TRANSFER_EVENT   = 32,
    TRB_TYPE_CMD_COMPLETION   = 33,
    TRB_TYPE_PORT_STATUS_CHG  = 34,
};

/* Completion codes (event TRB status[31:24]) */
#define TRB_CC(status)       (((status) >> 24) & 0xFF)
#define TRB_CC_SUCCESS        1
#define TRB_CC_SHORT_PACKET   13

/* Setup stage TRT (transfer type) field, control[17:16] */
#define TRB_SETUP_TRT_NO_DATA     0
#define TRB_SETUP_TRT_OUT_DATA    2
#define TRB_SETUP_TRT_IN_DATA     3
#define TRB_SETUP_TRT_SHIFT       16

/* Data/status stage direction bit, control[16] */
#define TRB_DIR_IN         (1U << 16)

/* Address Device Command TRB, BSR (Block Set Address Request) bit, control[9].
 * BSR=1 sets up the default control endpoint (so 8-byte GET_DESCRIPTOR probes
 * work) without sending SET_ADDRESS on the wire; the real address assignment
 * happens on a second Address Device command with BSR=0, once we know the
 * device's actual bMaxPacketSize0. */
#define TRB_ADDR_DEV_BSR   (1U << 9)

/* Slot Context (first 32B of a 32B-context device context) */
typedef struct {
    UInt32 dword0;   /* route string[19:0], speed[23:20], ctx entries[31:27] */
    UInt32 dword1;   /* max exit latency[15:0], root hub port[23:16], num ports[31:24] */
    UInt32 dword2;   /* target slot? interrupter target[31:22] */
    UInt32 dword3;   /* usb device address[7:0], slot state[31:27] */
    UInt32 reserved[4];
} __attribute__((packed)) XHCISlotContext;

#define SLOT_CTX_ENTRIES_SHIFT   27
#define SLOT_CTX_SPEED_SHIFT     20
#define SLOT_CTX_ROOTPORT_SHIFT  16
/* dword0: Hub/MTT flags for devices that are themselves USB hubs. */
#define SLOT_CTX_MTT_BIT         (1U << 25)
#define SLOT_CTX_HUB_BIT         (1U << 26)
/* dword1[31:24]: Number of Ports (hub descriptor's bNbrPorts), only
 * meaningful when SLOT_CTX_HUB_BIT is set. */
#define SLOT_CTX_NUMPORTS_SHIFT  24
/* dword2: Parent Hub Slot ID[7:0]/Parent Port Number[15:8] (only needed for
 * LS/FS devices behind a HS hub, for Transaction Translator routing) and
 * TT Think Time[17:16] (only meaningful when this slot IS a hub). */
#define SLOT_CTX_PARENT_SLOT_SHIFT 0
#define SLOT_CTX_PARENT_PORT_SHIFT 8
#define SLOT_CTX_TTT_SHIFT         16

/* Endpoint Context */
typedef struct {
    UInt32 dword0;   /* ep state[2:0], interval[23:16] */
    UInt32 dword1;   /* ep type[5:3], max packet size[31:16], max burst[15:8] */
    UInt64 trDequeuePtr; /* + DCS in bit0 */
    UInt32 avgTrbLen_maxEsitLo;
    UInt32 reserved[3];
} __attribute__((packed)) XHCIEndpointContext;

#define EP_TYPE_CONTROL       4
#define EP_TYPE_BULK_OUT      2
#define EP_TYPE_BULK_IN       6
#define EP_CTX_CERR_SHIFT     1
#define EP_CTX_CERR(c)        ((UInt32)(c) << EP_CTX_CERR_SHIFT)
#define EP_CTX_TYPE_SHIFT     3
#define EP_CTX_MAXPKT_SHIFT   16

/* Input Control Context (first 32B of an Input Context) */
typedef struct {
    UInt32 dropFlags;
    UInt32 addFlags;
    UInt32 reserved[5];
    UInt32 configInfo; /* configuration value[7:0], interface number[15:8], alt setting[23:16] */
} __attribute__((packed)) XHCIInputControlContext;

/* A 32-byte-context Input Context: control + slot + 31 endpoint contexts.
 * We only ever populate slot + EP0 + one bulk IN + one bulk OUT. */
typedef struct {
    XHCIInputControlContext control;
    XHCISlotContext         slot;
    XHCIEndpointContext     ep[31];
} __attribute__((packed)) XHCIInputContext;

typedef struct {
    XHCISlotContext         slot;
    XHCIEndpointContext     ep[31];
} __attribute__((packed)) XHCIDeviceContext;

/* USB standard descriptors/requests (subset) */
typedef struct {
    UInt8  bmRequestType;
    UInt8  bRequest;
    UInt16 wValue;
    UInt16 wIndex;
    UInt16 wLength;
} __attribute__((packed)) USBSetupPacket;

#define USB_REQ_GET_DESCRIPTOR   6
#define USB_REQ_SET_CONFIGURATION 9
#define USB_REQ_SET_ADDRESS      5

#define USB_DESC_DEVICE          1
#define USB_DESC_CONFIGURATION   2

typedef struct {
    UInt8  bLength;
    UInt8  bDescriptorType;
    UInt16 bcdUSB;
    UInt8  bDeviceClass;
    UInt8  bDeviceSubClass;
    UInt8  bDeviceProtocol;
    UInt8  bMaxPacketSize0;
    UInt16 idVendor;
    UInt16 idProduct;
    UInt16 bcdDevice;
    UInt8  iManufacturer;
    UInt8  iProduct;
    UInt8  iSerialNumber;
    UInt8  bNumConfigurations;
} __attribute__((packed)) USBDeviceDescriptor;

typedef struct {
    UInt8  bLength;
    UInt8  bDescriptorType;
    UInt16 wTotalLength;
    UInt8  bNumInterfaces;
    UInt8  bConfigurationValue;
    UInt8  iConfiguration;
    UInt8  bmAttributes;
    UInt8  bMaxPower;
} __attribute__((packed)) USBConfigDescriptor;

typedef struct {
    UInt8  bLength;
    UInt8  bDescriptorType; /* 4 */
    UInt8  bInterfaceNumber;
    UInt8  bAlternateSetting;
    UInt8  bNumEndpoints;
    UInt8  bInterfaceClass;
    UInt8  bInterfaceSubClass;
    UInt8  bInterfaceProtocol;
    UInt8  iInterface;
} __attribute__((packed)) USBInterfaceDescriptor;

typedef struct {
    UInt8  bLength;
    UInt8  bDescriptorType; /* 5 */
    UInt8  bEndpointAddress; /* bit7: dir, bits3:0: number */
    UInt8  bmAttributes;     /* bits1:0: transfer type (2=bulk) */
    UInt16 wMaxPacketSize;
    UInt8  bInterval;
} __attribute__((packed)) USBEndpointDescriptor;

#define USB_IF_CLASS_MASS_STORAGE   0x08
#define USB_IF_SUBCLASS_SCSI        0x06
#define USB_IF_PROTOCOL_BULK_ONLY   0x50

/* USB Hub class (device class 0x09): downstream port traversal so devices
 * behind an internal/onboard hub (very common on real hardware - the actual
 * boot USB stick usually isn't wired straight to a root xHCI port) can be
 * enumerated. bDeviceProtocol distinguishes a classic USB2 hub (0/1/2:
 * full-speed only / single-TT / multi-TT) from a USB3.x SuperSpeed hub (3);
 * the hub/port class requests below are numerically identical between the
 * two, only the GET_DESCRIPTOR class code and exact port-status bit layout
 * differ, and we only need the bits both share (connection + reset). */
#define USB_DEV_CLASS_HUB           0x09
#define USB_HUB_PROTO_SUPERSPEED    3

#define USB_DESC_HUB                0x29 /* USB2 Hub Descriptor */
#define USB_DESC_HUB_SS             0x2A /* USB3 SuperSpeed Hub Descriptor */

/* bmRequestType for hub class requests: 0xA0/0x23/0xA3 = class request,
 * device/other(port) recipient, in/out. */
#define USB_HUB_REQTYPE_GET_HUB_DESC   0xA0
#define USB_HUB_REQTYPE_SET_PORT_FEAT  0x23
#define USB_HUB_REQTYPE_CLEAR_PORT_FEAT 0x23
#define USB_HUB_REQTYPE_GET_PORT_STATUS 0xA3

#define USB_HUB_FEAT_PORT_CONNECTION   0
#define USB_HUB_FEAT_PORT_RESET        4
#define USB_HUB_FEAT_PORT_POWER        8
#define USB_HUB_FEAT_C_PORT_CONNECTION 16
#define USB_HUB_FEAT_C_PORT_RESET      20
#define USB_HUB_FEAT_C_BH_PORT_RESET   29 /* SS hub: warm-reset change */

/* GET_PORT_STATUS's 4-byte reply: wPortStatus | (wPortChange << 16). Only
 * the bits common to USB2 and USB3 hub port status words that we actually
 * use are named here. */
#define USB_PORTSTATUS_CONNECTION      (1U << 0)
#define USB_PORTSTATUS_ENABLE          (1U << 1)
#define USB_PORTSTATUS_PLS(v)          (((v) >> 5) & 0xF) /* SS hubs only: Port Link State */
#define USB_PORTSTATUS_LOW_SPEED       (1U << 9)  /* USB2 hubs only */
#define USB_PORTSTATUS_HIGH_SPEED      (1U << 10) /* USB2 hubs only */
#define USB_PORTCHANGE_C_RESET         (1U << 20)
#define USB_PORTCHANGE_C_BH_RESET      (1U << 21) /* SS hubs only: warm-reset change */
#define USB_PORT_LINK_STATE_U0         0

#define USB_EP_DIR_IN    0x80
#define USB_EP_ADDR_MASK 0x0F
#define USB_EP_TYPE_MASK 0x03
#define USB_EP_TYPE_BULK 0x02

/* USB Bulk-Only Transport (Mass Storage) */
typedef struct {
    UInt32 dCBWSignature;      /* 0x43425355 "USBC" */
    UInt32 dCBWTag;
    UInt32 dCBWDataTransferLength;
    UInt8  bmCBWFlags;         /* bit7: 1=IN, 0=OUT */
    UInt8  bCBWLUN;            /* bits3:0 */
    UInt8  bCBWCBLength;       /* bits4:0, 1..16 */
    UInt8  CBWCB[16];
} __attribute__((packed)) USBBOTCommandBlockWrapper;

#define BOT_CBW_SIGNATURE 0x43425355U
#define BOT_CSW_SIGNATURE 0x53425355U

typedef struct {
    UInt32 dCSWSignature;
    UInt32 dCSWTag;
    UInt32 dCSWDataResidue;
    UInt8  bCSWStatus;         /* 0=pass 1=fail 2=phase error */
} __attribute__((packed)) USBBOTCommandStatusWrapper;

#endif /* _XHCI_H */
