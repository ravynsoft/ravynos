/*
 * Copyright 2021 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 */

#ifndef __AGX_IO_H
#define __AGX_IO_H

#include <stdbool.h>
#include "agx_bo.h"

#if __APPLE__
#include <IOKit/IODataQueueClient.h>
#include <mach/mach.h>
#endif

/*
 * This file contains necessary defines for the macOS (IOKit) interface to the
 * AGX accelerator, required to build a userspace graphics driver on macOS.
 *
 * They are not used under Linux.
 *
 * Information is this file was originally determined independently. More
 * recently, names have been augmented via the oob_timestamp code sample from
 * Project Zero [1]
 *
 * [1] https://bugs.chromium.org/p/project-zero/issues/detail?id=1986
 */

#define AGX_SERVICE_TYPE 0x100005

enum agx_selector {
   AGX_SELECTOR_GET_GLOBAL_IDS = 0x6,
   AGX_SELECTOR_SET_API = 0x7,
   AGX_SELECTOR_CREATE_COMMAND_QUEUE = 0x8,
   AGX_SELECTOR_FREE_COMMAND_QUEUE = 0x9,
   AGX_SELECTOR_ALLOCATE_MEM = 0xA,
   AGX_SELECTOR_FREE_MEM = 0xB,
   AGX_SELECTOR_CREATE_SHMEM = 0xF,
   AGX_SELECTOR_FREE_SHMEM = 0x10,
   AGX_SELECTOR_CREATE_NOTIFICATION_QUEUE = 0x11,
   AGX_SELECTOR_FREE_NOTIFICATION_QUEUE = 0x12,
   AGX_SELECTOR_SUBMIT_COMMAND_BUFFERS = 0x1E,
   AGX_SELECTOR_GET_VERSION = 0x23,
   AGX_NUM_SELECTORS = 0x32
};

static const char *selector_table[AGX_NUM_SELECTORS] = {
   "unk0",
   "unk1",
   "unk2",
   "unk3",
   "unk4",
   "unk5",
   "GET_GLOBAL_IDS",
   "SET_API",
   "CREATE_COMMAND_QUEUE",
   "FREE_COMMAND_QUEUE",
   "ALLOCATE_MEM",
   "FREE_MEM",
   "unkC",
   "unkD",
   "unkE",
   "CREATE_SHMEM",
   "FREE_SHMEM",
   "CREATE_NOTIFICATION_QUEUE",
   "FREE_NOTIFICATION_QUEUE",
   "unk13",
   "unk14",
   "unk15",
   "unk16",
   "unk17",
   "unk18",
   "unk19",
   "unk1A",
   "unk1B",
   "unk1C",
   "unk1D",
   "SUBMIT_COMMAND_BUFFERS",
   "unk1F",
   "unk20",
   "unk21",
   "unk22",
   "GET_VERSION",
   "unk24",
   "unk25",
   "unk26",
   "unk27",
   "unk28",
   "unk29",
   "unk2A",
   "unk2B",
   "unk2C",
   "unk2D",
   "unk2E",
   "unk2F",
   "unk30",
   "unk31"};

static inline const char *
wrap_selector_name(uint32_t selector)
{
   return (selector < AGX_NUM_SELECTORS) ? selector_table[selector] : "unk??";
}

struct agx_create_command_queue_resp {
   uint64_t id;
   uint32_t unk2; // 90 0A 08 27
   uint32_t unk3; // 0
} __attribute__((packed));

struct agx_create_shmem_resp {
   /* IOAccelDeviceShmemData */
   void *map;
   uint32_t size;
   uint32_t id;
} __attribute__((packed));

struct agx_create_notification_queue_resp {
#ifdef __APPLE__
   IODataQueueMemory *queue;
#else
   void *queue;
#endif
   uint32_t unk2; // 1
   uint32_t unk3; // 0
} __attribute__((packed));

struct IOAccelCommandQueueSubmitArgs_Header {
   uint32_t unk0;
   uint32_t count;
};

struct IOAccelCommandQueueSubmitArgs_Command {
   uint32_t command_buffer_shmem_id;
   uint32_t segment_list_shmem_id;
   uint64_t unk1B; // 0, new in 12.x
   uint64_t notify_1;
   uint64_t notify_2;
   uint32_t unk2;
   uint32_t unk3;
} __attribute__((packed));

/* Memory allocation isn't really understood yet. By comparing SHADER/CMDBUF_32
 * vs everything else, it appears the 0x40000000 bit indicates the GPU VA must
 * be be in the first 4GiB */

enum agx_memory_type {
   AGX_MEMORY_TYPE_NORMAL = 0x00000000,    /* used for user allocations */
   AGX_MEMORY_TYPE_UNK = 0x08000000,       /* unknown */
   AGX_MEMORY_TYPE_CMDBUF_64 = 0x18000000, /* used for command buffer storage */
   AGX_MEMORY_TYPE_SHADER =
      0x48000000, /* used for shader memory, with VA = 0 */
   AGX_MEMORY_TYPE_CMDBUF_32 =
      0x58000000, /* used for command buffers, with VA < 32-bit */
   AGX_MEMORY_TYPE_FRAMEBUFFER = 0x00888F00, /* used for framebuffer backing */
};

static inline const char *
agx_memory_type_name(uint32_t type)
{
   switch (type) {
   case AGX_MEMORY_TYPE_NORMAL:
      return "normal";
   case AGX_MEMORY_TYPE_UNK:
      return "unk";
   case AGX_MEMORY_TYPE_CMDBUF_64:
      return "cmdbuf_64";
   case AGX_MEMORY_TYPE_SHADER:
      return "shader";
   case AGX_MEMORY_TYPE_CMDBUF_32:
      return "cmdbuf_32";
   case AGX_MEMORY_TYPE_FRAMEBUFFER:
      return "framebuffer";
   default:
      return NULL;
   }
}

struct agx_allocate_resource_req {
   uint32_t unk0[5];
   uint32_t mode;
   uint32_t unk6[6];
   uint64_t cpu_fixed;
   uint64_t cpu_fixed_parent;
   uint32_t size;
   uint32_t unk17;

   /* Handle of the parent resource when a suballocation is requested.
    * Based on an assertion failure, this corresponds to:
    *
    * -[IOGPUMetalBuffer
    * initWithPrimaryBuffer:heapIndex:bufferIndex:bufferOffset:length:args:argsSize:]
    */
   uint32_t parent;

   uint32_t unk19;
   uint32_t flags;
   uint32_t unk21[3];
} __attribute__((packed));

struct agx_allocate_resource_resp {
   /* Returned GPU virtual address */
   uint64_t gpu_va;

   /* Returned CPU virtual address */
   uint64_t cpu;

   uint32_t unk4[3];

   /* Handle used to identify the resource in the segment list */
   uint32_t handle;

   /* Size of the root resource from which we are allocated. If this is not a
    * suballocation, this is equal to the size.
    */
   uint64_t root_size;

   /* Globally unique identifier for the resource, shown in Instruments */
   uint32_t guid;

   uint32_t unk11[7];

   /* Maximum size of the suballocation. For a suballocation, this equals:
    *
    *    sub_size = root_size - (sub_cpu - root_cpu)
    *
    * For root allocations, this equals the size.
    */
   uint64_t sub_size;
} __attribute__((packed));

struct agx_notification_queue {
#ifdef __APPLE__
   mach_port_t port;
   IODataQueueMemory *queue;
#else
   unsigned port;
   void *queue;
#endif
   unsigned id;
};

struct agx_command_queue {
   unsigned id;
   struct agx_notification_queue notif;
};

struct agx_map_header {
   /* IOAccelSegmentListHeader */
   uint64_t cmdbuf_id; // GUID
   uint32_t segment_count;
   uint16_t length;
   uint16_t unk;        // 0x8000
   uint64_t encoder_id; // GUID

   /* IOAccelSegmentResourceListHeader */
   uint32_t kernel_commands_start_offset;
   uint32_t kernel_commands_end_offset;
   uint32_t padding[2];
   uint32_t total_resources;
   uint32_t resource_group_count;
} __attribute__((packed));

/* IOAccelSegmentResourceList_ResourceGroup */
struct agx_map_entry {
   uint32_t resource_id[6];
   uint32_t resource_unk[6];
   uint16_t resource_flags[6];
   uint16_t unka; // ff ff
   uint16_t resource_count;
} __attribute__((packed));

uint64_t agx_get_global_id(struct agx_device *dev);

#endif
