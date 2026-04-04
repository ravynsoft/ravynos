Binary files pciutils-3.1.7.orig/.DS_Store and pciutils-3.1.7/.DS_Store differ
diff -rupN pciutils-3.1.7.orig/lib/Makefile pciutils-3.1.7/lib/Makefile
--- pciutils-3.1.7.orig/lib/Makefile	2009-07-04 09:11:04.000000000 -0700
+++ pciutils-3.1.7/lib/Makefile	2011-02-10 15:40:40.000000000 -0800
@@ -42,6 +42,10 @@ ifdef PCI_HAVE_PM_NBSD_LIBPCI
 OBJS += nbsd-libpci
 endif
 
+ifdef PCI_HAVE_PM_DARWIN_DEVICE
+OBJS += darwin-device
+endif
+
 all: $(PCILIB) $(PCILIBPC)
 
 ifeq ($(SHARED),no)
diff -rupN pciutils-3.1.7.orig/lib/configure pciutils-3.1.7/lib/configure
--- pciutils-3.1.7.orig/lib/configure	2009-07-04 09:11:04.000000000 -0700
+++ pciutils-3.1.7/lib/configure	2011-02-10 16:18:46.000000000 -0800
@@ -100,6 +100,14 @@ case $sys in
 		echo >>$c '#define PCI_PATH_OBSD_DEVICE "/dev/pci"'
 		LIBRESOLV=
 		;;
+
+        darwin)
+	        echo_n " darwin-device"
+		echo >>$c '#define PCI_HAVE_PM_DARWIN_DEVICE'
+		echo >>$m 'WITH_LIBS+=-framework CoreFoundation -framework IOKit'
+		echo >>$c '#define PCI_HAVE_64BIT_ADDRESS'
+		LIBRESOLV=
+		;;
 	aix)
 		echo_n " aix-device"
 		echo >>$c '#define PCI_HAVE_PM_AIX_DEVICE'
diff -rupN pciutils-3.1.7.orig/lib/darwin-device.c pciutils-3.1.7/lib/darwin-device.c
--- pciutils-3.1.7.orig/lib/darwin-device.c	1969-12-31 16:00:00.000000000 -0800
+++ pciutils-3.1.7/lib/darwin-device.c	2011-02-11 10:45:06.000000000 -0800
@@ -0,0 +1,226 @@
+/*
+ *	The PCI Library -- FreeBSD /dev/pci access
+ *
+ *	Copyright (c) 1999 Jari Kirma <kirma@cs.hut.fi>
+ *	Updated in 2003 by Samy Al Bahra <samy@kerneled.com>
+ *
+ *	Can be freely distributed and used under the terms of the GNU GPL.
+ */
+
+#include <errno.h>
+#include <fcntl.h>
+#include <stdio.h>
+#include <string.h>
+#include <unistd.h>
+#include <stdint.h>
+
+#include "internal.h"
+
+
+#include <CoreFoundation/CoreFoundation.h>
+#include <IOKit/IOKitLib.h>
+#include <IOKit/IOKitKeys.h>
+
+
+enum {
+	kACPIMethodAddressSpaceRead		= 0,
+	kACPIMethodAddressSpaceWrite	= 1,
+	kACPIMethodDebuggerCommand		= 2,
+	kACPIMethodCount
+};
+
+#pragma pack(1)
+
+typedef UInt32 IOACPIAddressSpaceID;
+
+enum {
+    kIOACPIAddressSpaceIDSystemMemory       = 0,
+    kIOACPIAddressSpaceIDSystemIO           = 1,
+    kIOACPIAddressSpaceIDPCIConfiguration   = 2,
+    kIOACPIAddressSpaceIDEmbeddedController = 3,
+    kIOACPIAddressSpaceIDSMBus              = 4
+};
+
+/*
+ * 64-bit ACPI address
+ */
+union IOACPIAddress {
+    UInt64 addr64;
+    struct {
+        unsigned int offset     :16;
+        unsigned int function   :3;
+        unsigned int device     :5;
+        unsigned int bus        :8;
+        unsigned int segment    :16;
+        unsigned int reserved   :16;
+    } pci;
+};
+typedef union IOACPIAddress IOACPIAddress;
+
+#pragma pack()
+
+struct AddressSpaceParam {
+	UInt64			value;
+	UInt32			spaceID;
+	IOACPIAddress	address;
+	UInt32			bitWidth;
+	UInt32			bitOffset;
+	UInt32			options;
+};
+typedef struct AddressSpaceParam AddressSpaceParam;
+
+
+static void
+darwin_config(struct pci_access *a UNUSED)
+{
+}
+
+static int
+darwin_detect(struct pci_access *a)
+{
+	io_registry_entry_t    service;
+	io_connect_t           connect;
+	kern_return_t          status;
+
+	service = IOServiceGetMatchingService(kIOMasterPortDefault, 
+																					IOServiceMatching("AppleACPIPlatformExpert"));
+	if (service) 
+	{
+		status = IOServiceOpen(service, mach_task_self(), 0, &connect);
+		IOObjectRelease(service);
+	}
+
+  if (!service || (kIOReturnSuccess != status))
+	{
+		a->warning("Cannot open AppleACPIPlatformExpert (add boot arg debug=0x144 & run as root)");
+		return 0;
+	}
+  a->debug("...using AppleACPIPlatformExpert");
+  a->fd = connect;
+  return 1;
+}
+
+static void
+darwin_init(struct pci_access *a UNUSED)
+{
+}
+
+static void
+darwin_cleanup(struct pci_access *a UNUSED)
+{
+}
+
+static int
+darwin_read(struct pci_dev *d, int pos, byte *buf, int len)
+{
+  if (!(len == 1 || len == 2 || len == 4))
+    return pci_generic_block_read(d, pos, buf, len);
+
+  if (pos >= 256)
+    return 0;
+
+	AddressSpaceParam param;
+	kern_return_t     status;
+
+	param.spaceID   = kIOACPIAddressSpaceIDPCIConfiguration;
+	param.bitWidth  = len * 8;
+	param.bitOffset = 0;
+	param.options   = 0;
+
+	param.address.pci.offset   = pos;
+	param.address.pci.function = d->func;
+	param.address.pci.device   = d->dev;
+	param.address.pci.bus      = d->bus;
+	param.address.pci.segment  = d->domain;
+	param.address.pci.reserved = 0;
+	param.value                = -1ULL;
+
+	size_t outSize = sizeof(param);
+	status = IOConnectCallStructMethod(d->access->fd, kACPIMethodAddressSpaceRead,
+																					&param, sizeof(param),
+																					&param, &outSize);
+  if ((kIOReturnSuccess != status))
+	{
+		d->access->error("darwin_read: kACPIMethodAddressSpaceRead failed: %s",
+							mach_error_string(status));
+	}
+
+  switch (len)
+	{
+    case 1:
+      buf[0] = (u8) param.value;
+      break;
+    case 2:
+      ((u16 *) buf)[0] = cpu_to_le16((u16) param.value);
+      break;
+    case 4:
+      ((u32 *) buf)[0] = cpu_to_le32((u32) param.value);
+      break;
+	}
+  return 1;
+}
+
+static int
+darwin_write(struct pci_dev *d, int pos, byte *buf, int len)
+{
+  if (!(len == 1 || len == 2 || len == 4))
+    return pci_generic_block_write(d, pos, buf, len);
+
+  if (pos >= 256)
+    return 0;
+
+	AddressSpaceParam param;
+	kern_return_t     status;
+
+	param.spaceID   = kIOACPIAddressSpaceIDPCIConfiguration;
+	param.bitWidth  = len * 8;
+	param.bitOffset = 0;
+	param.options   = 0;
+
+	param.address.pci.offset   = pos;
+	param.address.pci.function = d->func;
+	param.address.pci.device   = d->dev;
+	param.address.pci.bus      = d->bus;
+	param.address.pci.segment  = d->domain;
+	param.address.pci.reserved = 0;
+  switch (len)
+	{
+    case 1:
+      param.value = buf[0];
+      break;
+    case 2:
+      param.value = le16_to_cpu(((u16 *) buf)[0]);
+      break;
+    case 4:
+      param.value = le32_to_cpu(((u32 *) buf)[0]);
+      break;
+	}
+
+	size_t outSize = 0;
+	status = IOConnectCallStructMethod(d->access->fd, kACPIMethodAddressSpaceWrite,
+																					&param, sizeof(param),
+																					NULL, &outSize);
+  if ((kIOReturnSuccess != status))
+	{
+		d->access->error("darwin_read: kACPIMethodAddressSpaceWrite failed: %s",
+							mach_error_string(status));
+	}
+
+  return 1;
+}
+
+struct pci_methods pm_darwin_device = {
+  "darwin-device",
+  "Darwin device",
+  darwin_config,
+  darwin_detect,
+  darwin_init,
+  darwin_cleanup,
+  pci_generic_scan,
+  pci_generic_fill_info,
+  darwin_read,
+  darwin_write,
+  NULL,                                 /* read_vpd */
+  NULL,                                 /* dev_init */
+  NULL                                  /* dev_cleanup */
+};
diff -rupN pciutils-3.1.7.orig/lib/init.c pciutils-3.1.7/lib/init.c
--- pciutils-3.1.7.orig/lib/init.c	2008-11-10 15:11:51.000000000 -0800
+++ pciutils-3.1.7/lib/init.c	2011-02-10 15:37:42.000000000 -0800
@@ -57,6 +57,11 @@ static struct pci_methods *pci_methods[P
 #else
   NULL,
 #endif
+#ifdef PCI_HAVE_PM_DARWIN_DEVICE
+  &pm_darwin_device,
+#else
+  NULL,
+#endif
 };
 
 void *
diff -rupN pciutils-3.1.7.orig/lib/internal.h pciutils-3.1.7/lib/internal.h
--- pciutils-3.1.7.orig/lib/internal.h	2009-07-04 09:11:04.000000000 -0700
+++ pciutils-3.1.7/lib/internal.h	2011-02-10 15:38:37.000000000 -0800
@@ -68,4 +68,4 @@ void pci_free_caps(struct pci_dev *);
 
 extern struct pci_methods pm_intel_conf1, pm_intel_conf2, pm_linux_proc,
 	pm_fbsd_device, pm_aix_device, pm_nbsd_libpci, pm_obsd_device,
-	pm_dump, pm_linux_sysfs;
+	pm_dump, pm_linux_sysfs, pm_darwin_device;
diff -rupN pciutils-3.1.7.orig/lib/pci.h pciutils-3.1.7/lib/pci.h
--- pciutils-3.1.7.orig/lib/pci.h	2009-07-04 09:11:04.000000000 -0700
+++ pciutils-3.1.7/lib/pci.h	2011-02-10 16:15:58.000000000 -0800
@@ -39,7 +39,8 @@ enum pci_access_type {
   PCI_ACCESS_AIX_DEVICE,		/* /dev/pci0, /dev/bus0, etc. */
   PCI_ACCESS_NBSD_LIBPCI,		/* NetBSD libpci */
   PCI_ACCESS_OBSD_DEVICE,		/* OpenBSD /dev/pci */
-  PCI_ACCESS_DUMP,			/* Dump file */
+  PCI_ACCESS_DUMP,			    /* Dump file */
+  PCI_ACCESS_DARWIN,			/* Darwin */
   PCI_ACCESS_MAX
 };
 
