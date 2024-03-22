#ifndef _PCI_ID_DRIVER_MAP_H_
#define _PCI_ID_DRIVER_MAP_H_

#include <stdbool.h>
#include <stddef.h>

#ifndef __IS_LOADER
#  error "Only include from loader.c"
#endif

static const int i915_chip_ids[] = {
#define CHIPSET(chip, desc, name) chip,
#include "pci_ids/i915_pci_ids.h"
#undef CHIPSET
};

static const int crocus_chip_ids[] = {
#define CHIPSET(chip, family, family_str, name) chip,
#include "pci_ids/crocus_pci_ids.h"
#undef CHIPSET
};

static const int r300_chip_ids[] = {
#define CHIPSET(chip, name, family) chip,
#include "pci_ids/r300_pci_ids.h"
#undef CHIPSET
};

static const int r600_chip_ids[] = {
#define CHIPSET(chip, name, family) chip,
#include "pci_ids/r600_pci_ids.h"
#undef CHIPSET
};

static const int virtio_gpu_chip_ids[] = {
#define CHIPSET(chip, name, family) chip,
#include "pci_ids/virtio_gpu_pci_ids.h"
#undef CHIPSET
};

static const int vmwgfx_chip_ids[] = {
#define CHIPSET(chip, name, family) chip,
#include "pci_ids/vmwgfx_pci_ids.h"
#undef CHIPSET
};

bool iris_predicate(int fd);

static const struct {
   int vendor_id;
   const char *driver;
   const int *chip_ids;
   int num_chips_ids;
   bool (*predicate)(int fd);
} driver_map[] = {
   { 0x8086, "i915", i915_chip_ids, ARRAY_SIZE(i915_chip_ids) },
   { 0x8086, "crocus", crocus_chip_ids, ARRAY_SIZE(crocus_chip_ids) },
   { 0x8086, "iris", NULL, -1, iris_predicate },
   { 0x1002, "r300", r300_chip_ids, ARRAY_SIZE(r300_chip_ids) },
   { 0x1002, "r600", r600_chip_ids, ARRAY_SIZE(r600_chip_ids) },
   { 0x1002, "radeonsi", NULL, -1 },
   { 0x10de, "nouveau", NULL, -1, },
   { 0x1af4, "virtio_gpu", virtio_gpu_chip_ids, ARRAY_SIZE(virtio_gpu_chip_ids) },
   { 0x15ad, "vmwgfx", vmwgfx_chip_ids, ARRAY_SIZE(vmwgfx_chip_ids) },
};

#endif /* _PCI_ID_DRIVER_MAP_H_ */
