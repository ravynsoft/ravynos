/*
 * AHCI register definitions for ravynOS from SATA AHCI 1.3.1 specification.
 *
 * Copyright (C) 2026 Zoe Knox. All rights reserved.
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

#ifndef _AHCI_H
#define _AHCI_H

#include <IOKit/IOTypes.h>

/* HBA Generic Host Control Registers (ABAR + 0x00) */
#define AHCI_CAP        0x00      /* Host Capabilities */
#define AHCI_GHC        0x04      /* Global Host Control */
#define AHCI_IS         0x08      /* Interrupt Status */
#define AHCI_PI         0x0C      /* Ports Implemented */
#define AHCI_VS         0x10      /* Version */
#define AHCI_CAP2       0x24      /* Host Capabilities Extended */

#define AHCI_GHC_AE     (1U<<31)  /* AHCI Enable */
#define AHCI_GHC_IE     (1U<<1)   /* Interrupt Enable */
#define AHCI_GHC_HR     (1U<<0)   /* HBA Reset */

#define AHCI_CAP_S64A   (1U<<31)  /* Supports 64-bit Addressing */
#define AHCI_CAP_NCS(c) (((c)>>8)&0x1f) /* Number of Command Slots (0-based) */
#define AHCI_CAP_NP(c)  ((c)&0x1f)      /* Number of Ports (0-based) */

/* Per port registers (ABAR + 0x100 + port*0x80) */
#define PORT_REGS_BASE  0x100
#define PORT_REGS_SIZE  0x80

#define PORT_CLB        0x00      /* Command List Base Address (low 32) */
#define PORT_CLBU       0x04      /* Command List Base Address (high 32) */
#define PORT_FB         0x08      /* FIS Base Address (low 32) */
#define PORT_FBU        0x0C      /* FIS Base Address (high 32) */
#define PORT_IS         0x10      /* Interrupt Status */
#define PORT_IE         0x14      /* Interrupt Enable */
#define PORT_CMD        0x18      /* Command and Status */
#define PORT_TFD        0x20      /* Task File Data */
#define PORT_SIG        0x24      /* Signature */
#define PORT_SSTS       0x28      /* SATA Status (SCR0) */
#define PORT_SCTL       0x2C      /* SATA Control (SCR2) */
#define PORT_SERR       0x30      /* SATA Error (SCR1) */
#define PORT_SACT       0x34      /* SATA Active */
#define PORT_CI         0x38      /* Command Issue */

#define PORT_CMD_ST      (1U<<0)   /* Start */
#define PORT_CMD_FRE     (1U<<4)   /* FIS Receive Enable */
#define PORT_CMD_FR      (1U<<14)  /* FIS Receive Running */
#define PORT_CMD_CR      (1U<<15)  /* Command List Running */
#define PORT_CMD_ACTIVE  (1U<<28)  /* Interface Active */
#define PORT_CMD_ICC_ACTIVE (1U<<28)

#define PORT_TFD_BSY     (1U<<7)   /* Busy */
#define PORT_TFD_DRQ     (1U<<3)   /* Data Request */
#define PORT_TFD_ERR     (1U<<0)   /* Error */

#define PORT_SSTS_DET(s) ((s)&0xf)
#define PORT_SSTS_DET_PRESENT 3   /* Device present and comms established */

#define PORT_SIG_SATA   0x00000101U
#define PORT_SIG_SATAPI 0xEB140101U

#define PORT_IS_DHRS    (1U << 0)
#define PORT_IS_PSS     (1U << 1)
#define PORT_IS_DS      (1U << 2)
#define PORT_IS_SDBS    (1U << 3)
#define PORT_IS_UFS     (1U << 4)
#define PORT_IS_DPS     (1U << 5)
#define PORT_IS_PCS     (1U << 6)
#define PORT_IS_DMPS    (1U << 7)
#define PORT_IS_PRCS    (1U << 22)
#define PORT_IS_IPMS    (1U << 23)
#define PORT_IS_OFS     (1U << 24)
#define PORT_IS_INFS    (1U << 26)
#define PORT_IS_IFS     (1U << 27)
#define PORT_IS_HBDS    (1U << 28)
#define PORT_IS_HBFS    (1U << 29)
#define PORT_IS_TFES    (1U << 30)
#define PORT_IS_CPDS    (1U << 31)

#define PORT_CMD_SUD    (1U << 1)
#define PORT_CMD_POD    (1U << 2)
#define PORT_CMD_CLO    (1U << 3)

#define PORT_SCTL_DET_MASK    0x0f
#define PORT_SCTL_DET_NONE    0x00
#define PORT_SCTL_DET_INIT    0x01

#define PORT_SSTS_IPM(s)      (((s) >> 8) & 0x0f)
#define PORT_SSTS_IPM_ACTIVE  1

#define ATA_STATUS_ERR  0x01
#define ATA_STATUS_DRQ  0x08
#define ATA_STATUS_DF   0x20
#define ATA_STATUS_DRDY 0x40
#define ATA_STATUS_BSY  0x80

/* Command List Header (32 bytes, 32 slots per port) */
struct AHCICmdHeader {
    uint16_t    cfl_flags;  /* CFL[4:0], A, W, P, R, B, C, rsvd[4:0] */
    uint16_t    prdtl;      /* PRD Table Length (entries) */
    uint32_t    prdbc;      /* PRD Byte Count (written by HBA) */
    uint32_t    ctba;       /* Command Table Descriptor Base (low, 7-byte aligned) */
    uint32_t    ctbau;      /* Command Table Descriptor Base (high) */
    uint32_t    rsvd[4];
} __attribute__((packed));

#define CMD_HDR_CFL(n)   ((n)&0x1f)        /* Command FIS Length in DWORDs */
#define CMD_HDR_WRITE    (1U<<6)           /* Write direction */
#define CMD_HDR_PREFETCH (1U<<5)           /* Prefetch */
#define CMD_HDR_ATAPI    (1U<<5)           /* ATAPI command */

/* Physical Region Descriptor */
struct AHCIPhysRegionDesc {
    uint32_t    dba;   /* Data Base Address (low) */
    uint32_t    dbau;  /* Data Base Address (high) */
    uint32_t    rsvd;
    uint32_t    dbc;   /* Byte count (22-bit) | rsvd[8:1] | I[0] */
} __attribute__((packed));

#define PRD_DBC_INT     (1U<<31)          /* Interrupt on completion */
#define PRD_MAX_BYTES   (4*1024*1024)     /* 4 MiB per PRD entry */

/* Command Table */
#define AHCI_CMD_TABLE_CFIS_SIZE    64
#define AHCI_CMD_TABLE_ACMD_SIZE    16
#define AHCI_CMD_TABLE_RSVD_SIZE    48
#define AHCI_CMD_TABLE_PRDT_OFFSET  (AHCI_CMD_TABLE_CFIS_SIZE \
                                   + AHCI_CMD_TABLE_ACMD_SIZE \
                                   + AHCI_CMD_TABLE_RSVD_SIZE)

/* We use 1 PRD per command during bring-up */
#define AHCI_CMD_TABLE_SIZE         (AHCI_CMD_TABLE_PRDT_OFFSET \
                                   + sizeof(AHCIPhysRegionDesc))

/* H2D Register FIS */
struct AHCIFIS_H2D {
    uint8_t     type;       /* 0x27 */
    uint8_t     pmport_c;   /* pm_port[3:0], rsvd[2:0], C */
    uint8_t     command;
    uint8_t     featurel;
    uint8_t     lba0;
    uint8_t     lba1;
    uint8_t     lba2;
    uint8_t     device;
    uint8_t     lba3;
    uint8_t     lba4;
    uint8_t     lba5;
    uint8_t     featureh;
    uint8_t     countl;
    uint8_t     counth;
    uint8_t     icc;
    uint8_t     control;
    uint8_t     rsvd[4];
} __attribute__((packed));

#define FIS_TYPE_H2D         0x27
#define FIS_H2D_C            (1U<<7)   /* Command (vs. Control) */

/* ATA Commands */
#define ATA_CMD_IDENTIFY     0xEC
#define ATA_CMD_READ_DMA_EX  0x25
#define ATA_CMD_WRITE_DMA_EX 0x35
#define ATA_CMD_FLUSH_EXT    0xEA
#define ATA_DEV_LBA          (1U<<6)   /* Device LBA bit */

/* IDENTIFY offsets (in uint16_t words) */
#define IDENTIFY_SECTORS_48   100   /* Total sectors (48-bit LBA), 4 words */
#define IDENTIFY_SECTORS_28   60    /* Total sectors (28-bit LBA), 2 words */
#define IDENTIFY_CAP_83       83    /* Features supported: bit 10 = 48-bit LBA */
#define IDENTIFY_MODEL_START  27    /* Model string (40 bytes, 20 words) */
#define IDENTIFY_FIRMWARE_REV 23    /* Firmware revision (8 bytes, 4 words) */
#define IDENTIFY_SERIAL_START 10    /* Serial number (20 bytes, 10 words) */

/* Memory layout per port */
/* Command List: 32 × 32-byte headers = 1024 bytes (1K-aligned) */
#define AHCI_CLB_SIZE       (32 * sizeof(AHCICmdHeader))
/* FIS Receive Area: 256 bytes (256-byte aligned) */
#define AHCI_FIS_SIZE       256
/* Command Table: 1 table for slot 0, 128-byte aligned */
#define AHCI_CTBL_SIZE      AHCI_CMD_TABLE_SIZE
/* DMA bounce buffer for IDENTIFY and small I/O */
#define AHCI_DMA_SIZE       (512 * 256)  /* 128 KiB */

/* Total per-port allocation (2 MiB aligned is overkill but safe) */
#define AHCI_PORT_MEM_SIZE  (AHCI_CLB_SIZE + AHCI_FIS_SIZE + AHCI_CTBL_SIZE + AHCI_DMA_SIZE + 4096)

/* Port memory offsets within the per-port buffer */
enum {
    kPortCLBOffset = 0x000,    // Command List: 1 KiB
    kPortFISOffset = 0x400,    // FIS Receive Area: 256 B
    kPortCTOffset  = 0x800,    // Command Table: aligned to 128 B
    kPortDMAOffset = 0x1000,   // DMA bounce buffer: page-aligned
    kPortMemBytes  = 0x21000   // 0x1000 fixed overhead + 128 KiB DMA = 132 KiB
};

#define AHCI_TIMEOUT_MS       5000  /* 5-second command timeout */
#define AHCI_RESET_TIMEOUT_MS 1000

#endif /* _AHCI_H */

