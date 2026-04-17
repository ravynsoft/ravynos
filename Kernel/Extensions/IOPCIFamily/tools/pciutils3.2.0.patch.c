diff -Nur pciutils-3.2.0/lib/Makefile pciutils-3.2.0-darwin/lib/Makefile
--- pciutils-3.2.0/lib/Makefile	2011-01-07 13:04:28.000000000 -0800
+++ pciutils-3.2.0-darwin/lib/Makefile	2013-10-31 18:17:24.000000000 -0700
@@ -42,6 +42,10 @@
 OBJS += nbsd-libpci
 endif
 
+ifdef PCI_HAVE_PM_DARWIN_DEVICE
+OBJS += darwin-device
+endif
+
 all: $(PCILIB) $(PCILIBPC)
 
 ifeq ($(SHARED),no)
diff -Nur pciutils-3.2.0/lib/configure pciutils-3.2.0-darwin/lib/configure
--- pciutils-3.2.0/lib/configure	2013-04-01 12:47:38.000000000 -0700
+++ pciutils-3.2.0-darwin/lib/configure	2013-10-31 18:17:24.000000000 -0700
@@ -100,6 +100,14 @@
 		echo >>$c '#define PCI_PATH_OBSD_DEVICE "/dev/pci"'
 		LIBRESOLV=
 		;;
+
+        darwin)
+	        echo_n " darwin-device"
+		echo >>$c '#define PCI_HAVE_PM_DARWIN_DEVICE'
+		echo >>$m 'WITH_LIBS+=-lresolv -framework CoreFoundation -framework IOKit'
+		echo >>$c '#define PCI_HAVE_64BIT_ADDRESS'
+		LIBRESOLV=
+		;;
 	aix)
 		echo_n " aix-device"
 		echo >>$c '#define PCI_HAVE_PM_AIX_DEVICE'
diff -Nur pciutils-3.2.0/lib/darwin-device.c pciutils-3.2.0-darwin/lib/darwin-device.c
--- pciutils-3.2.0/lib/darwin-device.c	1969-12-31 16:00:00.000000000 -0800
+++ pciutils-3.2.0-darwin/lib/darwin-device.c	2013-11-01 13:46:41.000000000 -0700
@@ -0,0 +1,166 @@
+/*
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
+#include <mach/mach_error.h>
+#include <IOKit/IOKitLib.h>
+#include <IOKit/IOKitKeys.h>
+#include <IOKit/pci/IOPCIDevice.h>
+#include <IOKit/pci/IOPCIPrivate.h>
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
+																					IOServiceMatching("IOPCIBridge"));
+	if (service) 
+	{
+		status = IOServiceOpen(service, mach_task_self(), kIOPCIDiagnosticsClientType, &connect);
+		IOObjectRelease(service);
+	}
+
+  if (!service || (kIOReturnSuccess != status))
+	{
+		a->warning("Cannot open IOPCIBridge (add boot arg debug=0x144 & run as root)");
+		return 0;
+	}
+  a->debug("...using IOPCIBridge");
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
+    IOPCIDiagnosticsParameters param;
+    kern_return_t              status;
+
+    param.spaceType = kIOPCIConfigSpace;
+    param.bitWidth  = len * 8;
+    param.options   = 0;
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
+	status = IOConnectCallStructMethod(d->access->fd, kIOPCIDiagnosticsMethodRead,
+																					&param, sizeof(param),
+																					&param, &outSize);
+  if ((kIOReturnSuccess != status))
+	{
+		d->access->error("darwin_read: kIOPCIDiagnosticsMethodRead failed: %s",
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
+    IOPCIDiagnosticsParameters param;
+    kern_return_t              status;
+
+    param.spaceType = kIOPCIConfigSpace;
+    param.bitWidth  = len * 8;
+    param.options   = 0;
+    param.address.pci.offset   = pos;
+    param.address.pci.function = d->func;
+    param.address.pci.device   = d->dev;
+    param.address.pci.bus      = d->bus;
+    param.address.pci.segment  = d->domain;
+    param.address.pci.reserved = 0;
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
+	status = IOConnectCallStructMethod(d->access->fd, kIOPCIDiagnosticsMethodWrite,
+																					&param, sizeof(param),
+																					NULL, &outSize);
+  if ((kIOReturnSuccess != status))
+	{
+		d->access->error("darwin_read: kIOPCIDiagnosticsMethodWrite failed: %s",
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
diff -Nur pciutils-3.2.0/lib/init.c pciutils-3.2.0-darwin/lib/init.c
--- pciutils-3.2.0/lib/init.c	2011-01-07 13:04:28.000000000 -0800
+++ pciutils-3.2.0-darwin/lib/init.c	2013-10-31 18:17:24.000000000 -0700
@@ -57,6 +57,11 @@
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
diff -Nur pciutils-3.2.0/lib/internal.h pciutils-3.2.0-darwin/lib/internal.h
--- pciutils-3.2.0/lib/internal.h	2013-04-01 06:36:18.000000000 -0700
+++ pciutils-3.2.0-darwin/lib/internal.h	2013-10-31 18:17:24.000000000 -0700
@@ -69,4 +69,4 @@
 
 extern struct pci_methods pm_intel_conf1, pm_intel_conf2, pm_linux_proc,
 	pm_fbsd_device, pm_aix_device, pm_nbsd_libpci, pm_obsd_device,
-	pm_dump, pm_linux_sysfs;
+	pm_dump, pm_linux_sysfs, pm_darwin_device;
diff -Nur pciutils-3.2.0/lib/pci.h pciutils-3.2.0-darwin/lib/pci.h
--- pciutils-3.2.0/lib/pci.h	2013-04-01 06:35:24.000000000 -0700
+++ pciutils-3.2.0-darwin/lib/pci.h	2013-10-31 18:17:24.000000000 -0700
@@ -39,7 +39,8 @@
   PCI_ACCESS_AIX_DEVICE,		/* /dev/pci0, /dev/bus0, etc. */
   PCI_ACCESS_NBSD_LIBPCI,		/* NetBSD libpci */
   PCI_ACCESS_OBSD_DEVICE,		/* OpenBSD /dev/pci */
-  PCI_ACCESS_DUMP,			/* Dump file */
+  PCI_ACCESS_DUMP,			    /* Dump file */
+  PCI_ACCESS_DARWIN,			/* Darwin */
   PCI_ACCESS_MAX
 };
 
