/*
 * Copyright 2015 Patrick Rudolph <siro@das-labor.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include <stdio.h>
#include <string.h>
#include "adapter9.h"

#define DBG_CHANNEL DBG_ADAPTER

/* prototypes */
void
d3d_match_vendor_id( D3DADAPTER_IDENTIFIER9* drvid,
        unsigned fallback_ven,
        unsigned fallback_dev,
        const char* fallback_name );
void d3d_fill_driver_version(D3DADAPTER_IDENTIFIER9* drvid);
void d3d_fill_cardname(D3DADAPTER_IDENTIFIER9* drvid);

enum d3d_vendor_id
{
    HW_VENDOR_SOFTWARE              = 0x0000,
    HW_VENDOR_AMD                   = 0x1002,
    HW_VENDOR_NVIDIA                = 0x10de,
    HW_VENDOR_VMWARE                = 0x15ad,
    HW_VENDOR_INTEL                 = 0x8086,
};

struct card_lookup_table {
    const char *mesaname;
    const char *d3d9name;
}
cards_amd[] = {
    {"HAWAII",                      "AMD Radeon R9 290"},
    {"KAVERI",                      "AMD Radeon(TM) R7 Graphics"},
    {"KABINI",                      "AMD Radeon HD 8400 / R3 Series"},
    {"BONAIRE",                     "AMD Radeon HD 8770"},
    {"OLAND",                       "AMD Radeon HD 8670"},
    {"HAINAN",                      "AMD Radeon HD 8600M Series"},
    {"TAHITI",                      "AMD Radeon HD 7900 Series"},
    {"PITCAIRN",                    "AMD Radeon HD 7800 Series"},
    {"CAPE VERDE",                  "AMD Radeon HD 7700 Series"},
    {"ARUBA",                       "AMD Radeon HD 7660D"},
    {"CAYMAN",                      "AMD Radeon HD 6900 Series"},
    {"BARTS",                       "AMD Radeon HD 6800 Series"},
    {"TURKS",                       "AMD Radeon HD 6600 Series"},
    {"SUMO2",                       "AMD Radeon HD 6410D"},
    {"SUMO",                        "AMD Radeon HD 6550D"},
    {"CAICOS",                      "AMD Radeon HD 6400 Series"},
    {"PALM",                        "AMD Radeon HD 6300 series Graphics"},
    {"HEMLOCK",                     "ATI Radeon HD 5900 Series"},
    {"CYPRESS",                     "ATI Radeon HD 5800 Series"},
    {"JUNIPER",                     "ATI Radeon HD 5700 Series"},
    {"REDWOOD",                     "ATI Radeon HD 5600 Series"},
    {"CEDAR",                       "ATI Radeon HD 5500 Series"},
    {"R700",                        "ATI Radeon HD 4800 Series"},
    {"RV790",                       "ATI Radeon HD 4800 Series"},
    {"RV770",                       "ATI Radeon HD 4800 Series"},
    {"RV740",                       "ATI Radeon HD 4700 Series"},
    {"RV730",                       "ATI Radeon HD 4600 Series"},
    {"RV710",                       "ATI Radeon HD 4350"},
    {"RS880",                       "ATI Mobility Radeon HD 4200"},
    {"RS780",                       "ATI Radeon HD 3200 Graphics"},
    {"R680",                        "ATI Radeon HD 2900 XT"},
    {"R600",                        "ATI Radeon HD 2900 XT"},
    {"RV670",                       "ATI Radeon HD 2900 XT"},
    {"RV635",                       "ATI Mobility Radeon HD 2600"},
    {"RV630",                       "ATI Mobility Radeon HD 2600"},
    {"RV620",                       "ATI Mobility Radeon HD 2350"},
    {"RV610",                       "ATI Mobility Radeon HD 2350"},
    {"R580",                        "ATI Radeon X1600 Series"},
    {"R520",                        "ATI Radeon X1600 Series"},
    {"RV570",                       "ATI Radeon X1600 Series"},
    {"RV560",                       "ATI Radeon X1600 Series"},
    {"RV535",                       "ATI Radeon X1600 Series"},
    {"RV530",                       "ATI Radeon X1600 Series"},
    {"RV516",                       "ATI Radeon X700 SE"},
    {"RV515",                       "ATI Radeon X700 SE"},
    {"R481",                        "ATI Radeon X700 SE"},
    {"R480",                        "ATI Radeon X700 SE"},
    {"R430",                        "ATI Radeon X700 SE"},
    {"R423",                        "ATI Radeon X700 SE"},
    {"R420",                        "ATI Radeon X700 SE"},
    {"R410",                        "ATI Radeon X700 SE"},
    {"RV410",                       "ATI Radeon X700 SE"},
    {"RS740",                       "ATI RADEON XPRESS 200M Series"},
    {"RS690",                       "ATI RADEON XPRESS 200M Series"},
    {"RS600",                       "ATI RADEON XPRESS 200M Series"},
    {"RS485",                       "ATI RADEON XPRESS 200M Series"},
    {"RS482",                       "ATI RADEON XPRESS 200M Series"},
    {"RS480",                       "ATI RADEON XPRESS 200M Series"},
    {"RS400",                       "ATI RADEON XPRESS 200M Series"},
    {"R360",                        "ATI Radeon 9500"},
    {"R350",                        "ATI Radeon 9500"},
    {"R300",                        "ATI Radeon 9500"},
    {"RV370",                       "ATI Radeon 9500"},
    {"RV360",                       "ATI Radeon 9500"},
    {"RV351",                       "ATI Radeon 9500"},
    {"RV350",                       "ATI Radeon 9500"},
},
cards_nvidia[] =
{
    {"NV124",                       "NVIDIA GeForce GTX 970"},
    {"NV117",                       "NVIDIA GeForce GTX 750"},
    {"NVF1",                        "NVIDIA GeForce GTX 780 Ti"},
    {"NVF0",                        "NVIDIA GeForce GTX 780"},
    {"NVE6",                        "NVIDIA GeForce GTX 770M"},
    {"NVE4",                        "NVIDIA GeForce GTX 680"},
    {"NVD9",                        "NVIDIA GeForce GT 520"},
    {"NVCF",                        "NVIDIA GeForce GTX 550 Ti"},
    {"NVCE",                        "NVIDIA GeForce GTX 560"},
    {"NVC8",                        "NVIDIA GeForce GTX 570"},
    {"NVC4",                        "NVIDIA GeForce GTX 460"},
    {"NVC3",                        "NVIDIA GeForce GT 440"},
    {"NVC1",                        "NVIDIA GeForce GT 420"},
    {"NVC0",                        "NVIDIA GeForce GTX 480"},
    {"NVAF",                        "NVIDIA GeForce GT 320M"},
    {"NVAC",                        "NVIDIA GeForce 8200"},
    {"NVAA",                        "NVIDIA GeForce 8200"},
    {"NVA8",                        "NVIDIA GeForce 210"},
    {"NVA5",                        "NVIDIA GeForce GT 220"},
    {"NVA3",                        "NVIDIA GeForce GT 240"},
    {"NVA0",                        "NVIDIA GeForce GTX 280"},
    {"NV98",                        "NVIDIA GeForce 9200"},
    {"NV96",                        "NVIDIA GeForce 9400 GT"},
    {"NV94",                        "NVIDIA GeForce 9600 GT"},
    {"NV92",                        "NVIDIA GeForce 9800 GT"},
    {"NV86",                        "NVIDIA GeForce 8500 GT"},
    {"NV84",                        "NVIDIA GeForce 8600 GT"},
    {"NV50",                        "NVIDIA GeForce 8800 GTX"},
    {"NV68",                        "NVIDIA GeForce 6200"},
    {"NV67",                        "NVIDIA GeForce 6200"},
    {"NV63",                        "NVIDIA GeForce 6200"},
    {"NV4E",                        "NVIDIA GeForce 6200"},
    {"NV4C",                        "NVIDIA GeForce 6200"},
    {"NV4B",                        "NVIDIA GeForce 7600 GT"},
    {"NV4A",                        "NVIDIA GeForce 6200"},
    {"NV49",                        "NVIDIA GeForce 7800 GT"},
    {"NV47",                        "NVIDIA GeForce 7800 GT"},
    {"NV46",                        "NVIDIA GeForce Go 7400",},
    {"NV45",                        "NVIDIA GeForce 6800"},
    {"NV44",                        "NVIDIA GeForce 6200"},
    {"NV43",                        "NVIDIA GeForce 6600 GT"},
    {"NV42",                        "NVIDIA GeForce 6800"},
    {"NV41",                        "NVIDIA GeForce 6800"},
    {"NV40",                        "NVIDIA GeForce 6800"},
    {"NV38",                        "NVIDIA GeForce FX 5800"},
    {"NV36",                        "NVIDIA GeForce FX 5800"},
    {"NV35",                        "NVIDIA GeForce FX 5800"},
    {"NV34",                        "NVIDIA GeForce FX 5200"},
    {"NV31",                        "NVIDIA GeForce FX 5600"},
    {"NV30",                        "NVIDIA GeForce FX 5800"},
    {"nv28",                        "NVIDIA GeForce4 Ti 4200"},
    {"nv25",                        "NVIDIA GeForce4 Ti 4200"},
    {"nv20",                        "NVIDIA GeForce3"},
    {"nv1F",                        "NVIDIA GeForce4 MX 460"},
    {"nv1A",                        "NVIDIA GeForce2 GTS/GeForce2 Pro"},
    {"nv18",                        "NVIDIA GeForce4 MX 460"},
    {"nv17",                        "NVIDIA GeForce4 MX 460"},
    {"nv16",                        "NVIDIA GeForce2 GTS/GeForce2 Pro"},
    {"nv15",                        "NVIDIA GeForce2 GTS/GeForce2 Pro"},
    {"nv11",                        "NVIDIA GeForce2 MX/MX 400"},
    {"nv10",                        "NVIDIA GeForce 256"},
},
cards_vmware[] =
{
    {"SVGA3D",                      "VMware SVGA 3D (Microsoft Corporation - WDDM)"},
},
cards_intel[] =
{
    {"Haswell Mobile",              "Intel(R) Haswell Mobile"},
    {"Ivybridge Server",            "Intel(R) Ivybridge Server"},
    {"Ivybridge Mobile",            "Intel(R) Ivybridge Mobile"},
    {"Ivybridge Desktop",           "Intel(R) Ivybridge Desktop"},
    {"Sandybridge Server",          "Intel(R) Sandybridge Server"},
    {"Sandybridge Mobile",          "Intel(R) Sandybridge Mobile"},
    {"Sandybridge Desktop",         "Intel(R) Sandybridge Desktop"},
    {"Ironlake Mobile",             "Intel(R) Ironlake Mobile"},
    {"Ironlake Desktop",            "Intel(R) Ironlake Desktop"},
    {"B43",                         "Intel(R) B43"},
    {"G41",                         "Intel(R) G41"},
    {"G45",                         "Intel(R) G45/G43"},
    {"Q45",                         "Intel(R) Q45/Q43"},
    {"Integrated Graphics Device",  "Intel(R) Integrated Graphics Device"},
    {"GM45",                        "Mobile Intel(R) GM45 Express Chipset Family"},
    {"965GME",                      "Intel(R) 965GME"},
    {"965GM",                       "Mobile Intel(R) 965 Express Chipset Family"},
    {"946GZ",                       "Intel(R) 946GZ"},
    {"965G",                        "Intel(R) 965G"},
    {"965Q",                        "Intel(R) 965Q"},
    {"Pineview M",                  "Intel(R) IGD"},
    {"Pineview G",                  "Intel(R) IGD"},
    {"IGD",                         "Intel(R) IGD"},
    {"Q33",                         "Intel(R) Q33"},
    {"G33",                         "Intel(R) G33"},
    {"Q35",                         "Intel(R) Q35"},
    {"945GME",                      "Intel(R) 945GME"},
    {"945GM",                       "Mobile Intel(R) 945GM Express Chipset Family"},
    {"945G",                        "Intel(R) 945G"},
    {"915GM",                       "Mobile Intel(R) 915GM/GMS,910GML Express Chipset Family"},
    {"E7221G",                      "Intel(R) E7221G"},
    {"915G",                        "Intel(R) 82915G/GV/910GL Express Chipset Family"},
    {"865G",                        "Intel(R) 82865G Graphics Controller"},
    {"845G",                        "Intel(R) 845G"},
    {"855GM",                       "Intel(R) 82852/82855 GM/GME Graphics Controller"},
    {"830M",                        "Intel(R) 82830M Graphics Controller"},
};

/* override VendorId, DeviceId and Description for unknown vendors */
void
d3d_match_vendor_id( D3DADAPTER_IDENTIFIER9* drvid,
        unsigned fallback_ven,
        unsigned fallback_dev,
        const char* fallback_name )
{
    if (drvid->VendorId == HW_VENDOR_INTEL ||
        drvid->VendorId == HW_VENDOR_VMWARE ||
        drvid->VendorId == HW_VENDOR_AMD ||
        drvid->VendorId == HW_VENDOR_NVIDIA)
        return;

    DBG("unknown vendor 0x4%x, emulating 0x4%x\n", drvid->VendorId, fallback_ven);
    drvid->VendorId = fallback_ven;
    drvid->DeviceId = fallback_dev;
    snprintf(drvid->Description, sizeof(drvid->Description), "%s", fallback_name);
}

/* fill in driver name and version */
void d3d_fill_driver_version(D3DADAPTER_IDENTIFIER9* drvid) {
    switch (drvid->VendorId) {
    case HW_VENDOR_INTEL:
        drvid->DriverVersionLowPart = 0x000A0682;
        drvid->DriverVersionHighPart = 0x0006000F;
        strncpy(drvid->Driver, "igdumd32.dll", sizeof(drvid->Driver));
        break;
    case HW_VENDOR_VMWARE:
        drvid->DriverVersionLowPart = 0x0001046E;
        drvid->DriverVersionHighPart = 0x0006000E;
        strncpy(drvid->Driver, "vm3dum.dll", sizeof(drvid->Driver));
        break;
    case HW_VENDOR_AMD:
        drvid->DriverVersionLowPart = 0x000A0500;
        drvid->DriverVersionHighPart = 0x00060011;
        strncpy(drvid->Driver, "atiumdag.dll", sizeof(drvid->Driver));
        break;
    case HW_VENDOR_NVIDIA:
        drvid->DriverVersionLowPart = 0x000D0FD4;
        drvid->DriverVersionHighPart = 0x00060012;
        strncpy(drvid->Driver, "nvd3dum.dll", sizeof(drvid->Driver));
        break;
    default:
        break;
    }
}

/* try to match the device name and override it with Windows like device names */
void d3d_fill_cardname(D3DADAPTER_IDENTIFIER9* drvid) {
    unsigned i;
    switch (drvid->VendorId) {
    case HW_VENDOR_INTEL:
        for (i = 0; i < sizeof(cards_intel) / sizeof(cards_intel[0]); i++) {
            if (strstr(drvid->Description, cards_intel[i].mesaname)) {
                snprintf(drvid->Description, sizeof(drvid->Description),
                         "%s", cards_intel[i].d3d9name);
                return;
            }
        }
        /* use a fall-back if nothing matches */
        DBG("Unknown card name %s!\n", drvid->DeviceName);
        snprintf(drvid->Description, sizeof(drvid->Description),
                 "%s", cards_intel[0].d3d9name);
        break;
    case HW_VENDOR_VMWARE:
        for (i = 0; i < sizeof(cards_vmware) / sizeof(cards_vmware[0]); i++) {
            if (strstr(drvid->Description, cards_vmware[i].mesaname)) {
                snprintf(drvid->Description, sizeof(drvid->Description),
                         "%s", cards_vmware[i].d3d9name);
                return;
            }
        }
        /* use a fall-back if nothing matches */
        DBG("Unknown card name %s!\n", drvid->DeviceName);
        snprintf(drvid->Description, sizeof(drvid->Description),
                 "%s", cards_vmware[0].d3d9name);
        break;
    case HW_VENDOR_AMD:
        for (i = 0; i < sizeof(cards_amd) / sizeof(cards_amd[0]); i++) {
            if (strstr(drvid->Description, cards_amd[i].mesaname)) {
                snprintf(drvid->Description, sizeof(drvid->Description),
                         "%s", cards_amd[i].d3d9name);
                return;
            }
        }
        /* use a fall-back if nothing matches */
        DBG("Unknown card name %s!\n", drvid->DeviceName);
        snprintf(drvid->Description, sizeof(drvid->Description),
                 "%s", cards_amd[0].d3d9name);
        break;
    case HW_VENDOR_NVIDIA:
        for (i = 0; i < sizeof(cards_nvidia) / sizeof(cards_nvidia[0]); i++) {
            if (strstr(drvid->Description, cards_nvidia[i].mesaname)) {
                snprintf(drvid->Description, sizeof(drvid->Description),
                         "%s", cards_nvidia[i].d3d9name);
                return;
            }
        }
        /* use a fall-back if nothing matches */
        DBG("Unknown card name %s!\n", drvid->DeviceName);
        snprintf(drvid->Description, sizeof(drvid->Description),
                 "%s", cards_nvidia[0].d3d9name);
        break;
    default:
        break;
    }
}
