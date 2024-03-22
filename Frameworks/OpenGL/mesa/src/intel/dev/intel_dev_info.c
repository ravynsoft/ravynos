/*
 * Copyright © 2020 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <getopt.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "util/libdrm.h"

#include "intel_device_info.h"
#include "intel_hwconfig.h"

static int
error(char *fmt, ...)
{
   va_list ap;
   va_start(ap, fmt);
   vfprintf(stderr, fmt, ap);
   va_end(ap);

   return EXIT_FAILURE;
}

static void
print_base_devinfo(const struct intel_device_info *devinfo)
{
   fprintf(stdout, "devinfo struct size = %zu\n", sizeof(*devinfo));

   fprintf(stdout, "   name: %s\n", devinfo->name);
   fprintf(stdout, "   gen: %u\n", devinfo->ver);
   fprintf(stdout, "   PCI device id: 0x%x\n", devinfo->pci_device_id);
   fprintf(stdout, "   PCI domain: 0x%x\n", devinfo->pci_domain);
   fprintf(stdout, "   PCI bus: 0x%x\n", devinfo->pci_bus);
   fprintf(stdout, "   PCI dev: 0x%x\n", devinfo->pci_dev);
   fprintf(stdout, "   PCI function: 0x%x\n", devinfo->pci_func);
   fprintf(stdout, "   PCI revision id: 0x%x\n", devinfo->pci_revision_id);
   fprintf(stdout, "   revision: %u\n", devinfo->revision);

   const char *subslice_name = devinfo->ver >= 12 ? "dualsubslice" : "subslice";
   uint32_t n_s = 0, n_ss = 0, n_eus = 0;
   for (unsigned s = 0; s < devinfo->max_slices; s++) {
      n_s += (devinfo->slice_masks & (1u << s)) ? 1 : 0;
      for (unsigned ss = 0; ss < devinfo->max_subslices_per_slice; ss++) {
         fprintf(stdout, "   slice%u.%s%u: ", s, subslice_name, ss);
         if (intel_device_info_subslice_available(devinfo, s, ss)) {
            n_ss++;
            for (unsigned eu = 0; eu < devinfo->max_eus_per_subslice; eu++) {
               n_eus += intel_device_info_eu_available(devinfo, s, ss, eu) ? 1 : 0;
               fprintf(stdout, "%s", intel_device_info_eu_available(devinfo, s, ss, eu) ? "1" : "0");
            }
         } else {
            fprintf(stdout, "fused");
         }
         fprintf(stdout, "\n");
      }
   }
   for (uint32_t pp = 0; pp < ARRAY_SIZE(devinfo->ppipe_subslices); pp++) {
      fprintf(stdout, "   pixel pipe %02u: %u\n",
              pp, devinfo->ppipe_subslices[pp]);
   }

   fprintf(stdout, "   slices: %u\n", n_s);
   fprintf(stdout, "   %s: %u\n", subslice_name, n_ss);
   fprintf(stdout, "   EUs: %u\n", n_eus);
   fprintf(stdout, "   EU threads: %u\n", n_eus * devinfo->num_thread_per_eu);

   fprintf(stdout, "   LLC: %u\n", devinfo->has_llc);
   fprintf(stdout, "   threads per EU: %u\n", devinfo->num_thread_per_eu);
   fprintf(stdout, "   L3 banks: %u\n", devinfo->l3_banks);
   fprintf(stdout, "   max VS  threads: %u\n", devinfo->max_vs_threads);
   fprintf(stdout, "   max TCS threads: %u\n", devinfo->max_tcs_threads);
   fprintf(stdout, "   max TES threads: %u\n", devinfo->max_tes_threads);
   fprintf(stdout, "   max GS  threads: %u\n", devinfo->max_gs_threads);
   fprintf(stdout, "   max WM  threads: %u\n", devinfo->max_wm_threads);
   fprintf(stdout, "   max CS  threads: %u\n", devinfo->max_cs_threads);
   fprintf(stdout, "   timestamp frequency: %" PRIu64 " / %.4f ns\n",
           devinfo->timestamp_frequency, 1000000000.0 / devinfo->timestamp_frequency);

   fprintf(stdout, "   URB size: %u\n", devinfo->urb.size);
   static const char *stage_names[4] = {
      "VS", "HS", "DS", "GS",
   };
   for (unsigned s = 0; s < ARRAY_SIZE(devinfo->urb.min_entries); s++) {
      fprintf(stdout, "      URB.entries[%s] = [%4u, %4u]\n",
              stage_names[s],
              devinfo->urb.min_entries[s],
              devinfo->urb.max_entries[s]);
   }
}

static void
print_regions_info(const struct intel_device_info *devinfo)
{
   if (devinfo->mem.sram.mappable.size > 0 ||
       devinfo->mem.sram.unmappable.size > 0) {
      fprintf(stdout, "   sram:\n");
      if (devinfo->mem.use_class_instance) {
         fprintf(stdout, "      class: %d; instance: %d\n",
                 devinfo->mem.sram.mem.klass, devinfo->mem.sram.mem.instance);
      }
      fprintf(stdout, "      mappable: %" PRId64 "; ",
              devinfo->mem.sram.mappable.size);
      fprintf(stdout, "free: %" PRId64 "\n",
              devinfo->mem.sram.mappable.free);
      if (devinfo->mem.sram.unmappable.size > 0) {
         fprintf(stdout, "      unmappable: %" PRId64 "; ",
                 devinfo->mem.sram.unmappable.size);
         fprintf(stdout, "free: %" PRId64 "\n",
                 devinfo->mem.sram.unmappable.free);
      }
   }

   if (devinfo->mem.vram.mappable.size > 0 ||
       devinfo->mem.vram.unmappable.size > 0) {
      fprintf(stdout, "   vram:\n");
      if (devinfo->mem.use_class_instance) {
         fprintf(stdout, "      class: %d; instance: %d\n",
                 devinfo->mem.vram.mem.klass, devinfo->mem.vram.mem.instance);
      }
      fprintf(stdout, "      mappable: %" PRId64 "; ",
              devinfo->mem.vram.mappable.size);
      fprintf(stdout, "free: %" PRId64 "\n",
              devinfo->mem.vram.mappable.free);
      if (devinfo->mem.vram.unmappable.size > 0) {
         fprintf(stdout, "      unmappable: %" PRId64 "; ",
                 devinfo->mem.vram.unmappable.size);
         fprintf(stdout, "free: %" PRId64 "\n",
                 devinfo->mem.vram.unmappable.free);
      }
   }
}

#define INTEL_WA( X ) "WA_"#X
static void
print_wa_info(const struct intel_device_info *devinfo)
{
   static const char* all_wa[] = { INTEL_ALL_WA };
   fprintf(stdout, "   required workarounds:\n");
   for (enum intel_workaround_id id = 0; id < INTEL_WA_NUM; ++id) {
      if (BITSET_TEST(devinfo->workarounds, id)) {
         fprintf(stdout, "      %s\n", all_wa[id]);
      }
   }
   fprintf(stdout, "\n");
}
#undef INTEL_WA

int
main(int argc, char *argv[])
{
   drmDevicePtr devices[8];
   int max_devices, i;
   char c;
   bool help = false, print_hwconfig = false, all = false, print_workarounds = false;
   const char *platform = NULL;
   const struct option opts[] = {
      { "help",              no_argument,  (int *) &help,              true },
      { "platform",    required_argument,  NULL,                       false },
      { "hwconfig",          no_argument,  (int *) &print_hwconfig,    true },
      { "workarounds",       no_argument,  (int *) &print_workarounds, true },
      { "all",               no_argument,  (int *) &all,               true },
   };
   while ((c = getopt_long(argc, argv, "hap:", opts, &i)) != -1) {
      switch (c) {
      case 'h':
         help = true;
         break;
      case 'a':
         all = true;
         break;
      case 'p':
         platform = optarg;
         break;
      default:
         break;
      }
   }

   if (help) {
      fprintf(stdout,
              "Usage: intel_dev_info [OPTION]\n"
              "Print device info for the current system.\n"
              "      --help / h        display this help and exit\n"
              "      --platform <name> print a given platform's info (skl, icl, tgl, etc...)\n"
              "      --hwconfig        print the hwconfig table\n"
              "      --workarounds     print the list of hardware workarounds for the system\n"
              "      --all / -a        print all optional details\n");
      exit(0);
   }

   if (all) {
      print_workarounds = true;
      print_hwconfig = true;
   }

   if (platform) {
      int pci_id;

      if (strstr(platform, "0x") == platform)
         pci_id = strtol(platform, NULL, 16);
      else
         pci_id = intel_device_name_to_pci_device_id(platform);

      struct intel_device_info devinfo;
      if (!intel_get_device_info_from_pci_id(pci_id, &devinfo))
         return error("No platform found with name: %s", platform);

      print_base_devinfo(&devinfo);
      if (print_workarounds)
         print_wa_info(&devinfo);
   } else {
      max_devices = drmGetDevices2(0, devices, ARRAY_SIZE(devices));
      if (max_devices < 1)
         return error("Not device found");

      for (int i = 0; i < max_devices; i++) {
         struct intel_device_info devinfo;
         const char *path = devices[i]->nodes[DRM_NODE_RENDER];
         int fd = open(path, O_RDWR | O_CLOEXEC);

         if (fd < 0)
            continue;

         bool success = intel_get_device_info_from_fd(fd, &devinfo);
         close(fd);

         if (!success)
            continue;

         fprintf(stdout, "%s:\n", path);

         print_base_devinfo(&devinfo);
         print_regions_info(&devinfo);
         if (print_hwconfig)
            intel_get_and_print_hwconfig_table(fd, &devinfo);
         if (print_workarounds)
            print_wa_info(&devinfo);
      }
   }

   return EXIT_SUCCESS;
}
