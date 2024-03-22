/*
 * Copyright 2021-2022 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 */
#include <assert.h>
#include <dlfcn.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include <IOKit/IOKitLib.h>
#include <mach/mach.h>

#include "util/compiler.h"
#include "util/u_hexdump.h"
#include "agx_iokit.h"
#include "decode.h"
#include "dyld_interpose.h"
#include "util.h"

/*
 * Wrap IOKit entrypoints to intercept communication between the AGX kernel
 * extension and userspace clients. IOKit prototypes are public from the IOKit
 * source release.
 */

mach_port_t metal_connection = 0;

kern_return_t
wrap_Method(mach_port_t connection, uint32_t selector, const uint64_t *input,
            uint32_t inputCnt, const void *inputStruct, size_t inputStructCnt,
            uint64_t *output, uint32_t *outputCnt, void *outputStruct,
            size_t *outputStructCntP)
{
   /* Heuristic guess which connection is Metal, skip over I/O from everything
    * else. This is technically wrong but it works in practice, and reduces the
    * surface area we need to wrap.
    */
   if (selector == AGX_SELECTOR_SET_API) {
      metal_connection = connection;
   } else if (metal_connection != connection) {
      return IOConnectCallMethod(connection, selector, input, inputCnt,
                                 inputStruct, inputStructCnt, output, outputCnt,
                                 outputStruct, outputStructCntP);
   }

   printf("Selector %u, %X, %X\n", selector, connection, metal_connection);

   /* Check the arguments make sense */
   assert((input != NULL) == (inputCnt != 0));
   assert((inputStruct != NULL) == (inputStructCnt != 0));
   assert((output != NULL) == (outputCnt != 0));
   assert((outputStruct != NULL) == (outputStructCntP != 0));

   /* Dump inputs */
   switch (selector) {
   case AGX_SELECTOR_SET_API:
      assert(input == NULL && output == NULL && outputStruct == NULL);
      assert(inputStruct != NULL && inputStructCnt == 16);
      assert(((uint8_t *)inputStruct)[15] == 0x0);

      printf("%X: SET_API(%s)\n", connection, (const char *)inputStruct);
      break;

   case AGX_SELECTOR_ALLOCATE_MEM: {
      const struct agx_allocate_resource_req *req = inputStruct;
      struct agx_allocate_resource_req *req2 = (void *)inputStruct;
      req2->mode = (req->mode & 0x800) | 0x430;

      bool suballocated = req->mode & 0x800;

      printf("Resource allocation:\n");
      printf("  Mode: 0x%X%s\n", req->mode & ~0x800,
             suballocated ? " (suballocated) " : "");
      printf("  CPU fixed: 0x%" PRIx64 "\n", req->cpu_fixed);
      printf("  CPU fixed (parent): 0x%" PRIx64 "\n", req->cpu_fixed_parent);
      printf("  Size: 0x%X\n", req->size);
      printf("  Flags: 0x%X\n", req->flags);

      if (suballocated) {
         printf("  Parent: %u\n", req->parent);
      } else {
         assert(req->parent == 0);
      }

      for (unsigned i = 0; i < ARRAY_SIZE(req->unk0); ++i) {
         if (req->unk0[i])
            printf("  UNK%u: 0x%X\n", 0 + i, req->unk0[i]);
      }

      for (unsigned i = 0; i < ARRAY_SIZE(req->unk6); ++i) {
         if (req->unk6[i])
            printf("  UNK%u: 0x%X\n", 6 + i, req->unk6[i]);
      }

      if (req->unk17)
         printf("  UNK17: 0x%X\n", req->unk17);

      if (req->unk19)
         printf("  UNK19: 0x%X\n", req->unk19);

      for (unsigned i = 0; i < ARRAY_SIZE(req->unk21); ++i) {
         if (req->unk21[i])
            printf("  UNK%u: 0x%X\n", 21 + i, req->unk21[i]);
      }

      break;
   }

   case AGX_SELECTOR_SUBMIT_COMMAND_BUFFERS:
      assert(output == NULL && outputStruct == NULL);
      assert(inputCnt == 1);

      printf("%X: SUBMIT_COMMAND_BUFFERS command queue id:%llx %p\n",
             connection, input[0], inputStruct);

      const struct IOAccelCommandQueueSubmitArgs_Header *hdr = inputStruct;
      const struct IOAccelCommandQueueSubmitArgs_Command *cmds =
         (void *)(hdr + 1);

      for (unsigned i = 0; i < hdr->count; ++i) {
         const struct IOAccelCommandQueueSubmitArgs_Command *req = &cmds[i];
         agxdecode_cmdstream(req->command_buffer_shmem_id,
                             req->segment_list_shmem_id, true);
         if (getenv("ASAHI_DUMP"))
            agxdecode_dump_mappings(req->segment_list_shmem_id);
      }

      agxdecode_next_frame();
      FALLTHROUGH;

   default:
      printf("%X: call %s (out %p, %zu)", connection,
             wrap_selector_name(selector), outputStructCntP,
             outputStructCntP ? *outputStructCntP : 0);

      for (uint64_t u = 0; u < inputCnt; ++u)
         printf(" %llx", input[u]);

      if (inputStructCnt) {
         printf(", struct:\n");
         u_hexdump(stdout, inputStruct, inputStructCnt, true);
      } else {
         printf("\n");
      }

      break;
   }

   /* Invoke the real method */
   kern_return_t ret = IOConnectCallMethod(
      connection, selector, input, inputCnt, inputStruct, inputStructCnt,
      output, outputCnt, outputStruct, outputStructCntP);

   if (ret != 0)
      printf("return %u\n", ret);

   /* Track allocations for later analysis (dumping, disassembly, etc) */
   switch (selector) {
   case AGX_SELECTOR_CREATE_SHMEM: {
      assert(inputCnt == 2);
      assert((*outputStructCntP) == 0x10);
      uint64_t *inp = (uint64_t *)input;

      uint8_t type = inp[1];

      assert(type <= 2);
      if (type == 2)
         printf("(cmdbuf with error reporting)\n");

      uint64_t *ptr = (uint64_t *)outputStruct;
      uint32_t *words = (uint32_t *)(ptr + 1);

      agxdecode_track_alloc(&(struct agx_bo){
         .handle = words[1],
         .ptr.cpu = (void *)*ptr,
         .size = words[0],
         .type = inp[1] ? AGX_ALLOC_CMDBUF : AGX_ALLOC_MEMMAP});

      break;
   }

   case AGX_SELECTOR_ALLOCATE_MEM: {
      assert((*outputStructCntP) == 0x50);
      const struct agx_allocate_resource_req *req = inputStruct;
      struct agx_allocate_resource_resp *resp = outputStruct;
      if (resp->cpu && req->cpu_fixed)
         assert(resp->cpu == req->cpu_fixed);
      printf("Response:\n");
      printf("  GPU VA: 0x%" PRIx64 "\n", resp->gpu_va);
      printf("  CPU VA: 0x%" PRIx64 "\n", resp->cpu);
      printf("  Handle: %u\n", resp->handle);
      printf("  Root size: 0x%" PRIx64 "\n", resp->root_size);
      printf("  Suballocation size: 0x%" PRIx64 "\n", resp->sub_size);
      printf("  GUID: 0x%X\n", resp->guid);
      for (unsigned i = 0; i < ARRAY_SIZE(resp->unk4); ++i) {
         if (resp->unk4[i])
            printf("  UNK%u: 0x%X\n", 4 + i, resp->unk4[i]);
      }
      for (unsigned i = 0; i < ARRAY_SIZE(resp->unk11); ++i) {
         if (resp->unk11[i])
            printf("  UNK%u: 0x%X\n", 11 + i, resp->unk11[i]);
      }

      if (req->parent)
         assert(resp->sub_size <= resp->root_size);
      else
         assert(resp->sub_size == resp->root_size);

      agxdecode_track_alloc(&(struct agx_bo){
         .type = AGX_ALLOC_REGULAR,
         .size = resp->sub_size,
         .handle = resp->handle,
         .ptr.gpu = resp->gpu_va,
         .ptr.cpu = (void *)resp->cpu,
      });

      break;
   }

   case AGX_SELECTOR_FREE_MEM: {
      assert(inputCnt == 1);
      assert(inputStruct == NULL);
      assert(output == NULL);
      assert(outputStruct == NULL);

      agxdecode_track_free(
         &(struct agx_bo){.type = AGX_ALLOC_REGULAR, .handle = input[0]});

      break;
   }

   case AGX_SELECTOR_FREE_SHMEM: {
      assert(inputCnt == 1);
      assert(inputStruct == NULL);
      assert(output == NULL);
      assert(outputStruct == NULL);

      agxdecode_track_free(
         &(struct agx_bo){.type = AGX_ALLOC_CMDBUF, .handle = input[0]});

      break;
   }

   default:
      /* Dump the outputs */
      if (outputCnt) {
         printf("%u scalars: ", *outputCnt);

         for (uint64_t u = 0; u < *outputCnt; ++u)
            printf("%llx ", output[u]);

         printf("\n");
      }

      if (outputStructCntP) {
         printf(" struct\n");
         u_hexdump(stdout, outputStruct, *outputStructCntP, true);

         if (selector == 2) {
            /* Dump linked buffer as well */
            void **o = outputStruct;
            u_hexdump(stdout, *o, 64, true);
         }
      }

      printf("\n");
      break;
   }

   return ret;
}

kern_return_t
wrap_AsyncMethod(mach_port_t connection, uint32_t selector,
                 mach_port_t wakePort, uint64_t *reference,
                 uint32_t referenceCnt, const uint64_t *input,
                 uint32_t inputCnt, const void *inputStruct,
                 size_t inputStructCnt, uint64_t *output, uint32_t *outputCnt,
                 void *outputStruct, size_t *outputStructCntP)
{
   /* Check the arguments make sense */
   assert((input != NULL) == (inputCnt != 0));
   assert((inputStruct != NULL) == (inputStructCnt != 0));
   assert((output != NULL) == (outputCnt != 0));
   assert((outputStruct != NULL) == (outputStructCntP != 0));

   printf("%X: call %X, wake port %X (out %p, %zu)", connection, selector,
          wakePort, outputStructCntP, outputStructCntP ? *outputStructCntP : 0);

   for (uint64_t u = 0; u < inputCnt; ++u)
      printf(" %llx", input[u]);

   if (inputStructCnt) {
      printf(", struct:\n");
      u_hexdump(stdout, inputStruct, inputStructCnt, true);
   } else {
      printf("\n");
   }

   printf(", references: ");
   for (unsigned i = 0; i < referenceCnt; ++i)
      printf(" %llx", reference[i]);
   printf("\n");

   kern_return_t ret = IOConnectCallAsyncMethod(
      connection, selector, wakePort, reference, referenceCnt, input, inputCnt,
      inputStruct, inputStructCnt, output, outputCnt, outputStruct,
      outputStructCntP);

   printf("return %u", ret);

   if (outputCnt) {
      printf("%u scalars: ", *outputCnt);

      for (uint64_t u = 0; u < *outputCnt; ++u)
         printf("%llx ", output[u]);

      printf("\n");
   }

   if (outputStructCntP) {
      printf(" struct\n");
      u_hexdump(stdout, outputStruct, *outputStructCntP, true);

      if (selector == 2) {
         /* Dump linked buffer as well */
         void **o = outputStruct;
         u_hexdump(stdout, *o, 64, true);
      }
   }

   printf("\n");
   return ret;
}

kern_return_t
wrap_StructMethod(mach_port_t connection, uint32_t selector,
                  const void *inputStruct, size_t inputStructCnt,
                  void *outputStruct, size_t *outputStructCntP)
{
   return wrap_Method(connection, selector, NULL, 0, inputStruct,
                      inputStructCnt, NULL, NULL, outputStruct,
                      outputStructCntP);
}

kern_return_t
wrap_AsyncStructMethod(mach_port_t connection, uint32_t selector,
                       mach_port_t wakePort, uint64_t *reference,
                       uint32_t referenceCnt, const void *inputStruct,
                       size_t inputStructCnt, void *outputStruct,
                       size_t *outputStructCnt)
{
   return wrap_AsyncMethod(connection, selector, wakePort, reference,
                           referenceCnt, NULL, 0, inputStruct, inputStructCnt,
                           NULL, NULL, outputStruct, outputStructCnt);
}

kern_return_t
wrap_ScalarMethod(mach_port_t connection, uint32_t selector,
                  const uint64_t *input, uint32_t inputCnt, uint64_t *output,
                  uint32_t *outputCnt)
{
   return wrap_Method(connection, selector, input, inputCnt, NULL, 0, output,
                      outputCnt, NULL, NULL);
}

kern_return_t
wrap_AsyncScalarMethod(mach_port_t connection, uint32_t selector,
                       mach_port_t wakePort, uint64_t *reference,
                       uint32_t referenceCnt, const uint64_t *input,
                       uint32_t inputCnt, uint64_t *output, uint32_t *outputCnt)
{
   return wrap_AsyncMethod(connection, selector, wakePort, reference,
                           referenceCnt, input, inputCnt, NULL, 0, output,
                           outputCnt, NULL, NULL);
}

mach_port_t
wrap_DataQueueAllocateNotificationPort()
{
   mach_port_t ret = IODataQueueAllocateNotificationPort();
   printf("Allocated notif port %X\n", ret);
   return ret;
}

kern_return_t
wrap_SetNotificationPort(io_connect_t connect, uint32_t type, mach_port_t port,
                         uintptr_t reference)
{
   printf(
      "Set noficiation port connect=%X, type=%X, port=%X, reference=%" PRIx64
      "\n",
      connect, type, port, (uint64_t)reference);

   return IOConnectSetNotificationPort(connect, type, port, reference);
}

IOReturn
wrap_DataQueueWaitForAvailableData(IODataQueueMemory *dataQueue,
                                   mach_port_t notificationPort)
{
   printf("Waiting for data queue at notif port %X\n", notificationPort);
   IOReturn ret = IODataQueueWaitForAvailableData(dataQueue, notificationPort);
   printf("ret=%X\n", ret);
   return ret;
}

IODataQueueEntry *
wrap_DataQueuePeek(IODataQueueMemory *dataQueue)
{
   printf("Peeking data queue\n");
   return IODataQueuePeek(dataQueue);
}

IOReturn
wrap_DataQueueDequeue(IODataQueueMemory *dataQueue, void *data,
                      uint32_t *dataSize)
{
   printf("Dequeueing (dataQueue=%p, data=%p, buffer %u)\n", dataQueue, data,
          *dataSize);
   IOReturn ret = IODataQueueDequeue(dataQueue, data, dataSize);
   printf("Return \"%s\", got %u bytes\n", mach_error_string(ret), *dataSize);

   uint8_t *data8 = data;
   for (unsigned i = 0; i < *dataSize; ++i) {
      printf("%02X ", data8[i]);
   }
   printf("\n");

   return ret;
}

DYLD_INTERPOSE(wrap_Method, IOConnectCallMethod);
DYLD_INTERPOSE(wrap_AsyncMethod, IOConnectCallAsyncMethod);
DYLD_INTERPOSE(wrap_StructMethod, IOConnectCallStructMethod);
DYLD_INTERPOSE(wrap_AsyncStructMethod, IOConnectCallAsyncStructMethod);
DYLD_INTERPOSE(wrap_ScalarMethod, IOConnectCallScalarMethod);
DYLD_INTERPOSE(wrap_AsyncScalarMethod, IOConnectCallAsyncScalarMethod);
DYLD_INTERPOSE(wrap_SetNotificationPort, IOConnectSetNotificationPort);
DYLD_INTERPOSE(wrap_DataQueueAllocateNotificationPort,
               IODataQueueAllocateNotificationPort);
DYLD_INTERPOSE(wrap_DataQueueWaitForAvailableData,
               IODataQueueWaitForAvailableData);
DYLD_INTERPOSE(wrap_DataQueuePeek, IODataQueuePeek);
DYLD_INTERPOSE(wrap_DataQueueDequeue, IODataQueueDequeue);
