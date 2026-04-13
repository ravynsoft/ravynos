/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#include <mach/mach_types.h>
#include <sys/mount.h>
#include <libkern/libkern.h>
#include <IOKit/IOService.h>
#include <IOKit/IOBSD.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IOPlatformExpert.h>
#include <pexpert/pexpert.h>

#include "hfs_iokit.h"
#include "hfs.h"
#include "hfs_dbg.h"
#include "hfs_cnode.h"

#ifndef panic_on_assert
bool panic_on_assert;
#endif

#if DEBUG
bool hfs_corruption_panics = true;
#endif

class com_apple_filesystems_hfs : public IOService {
    OSDeclareDefaultStructors(com_apple_filesystems_hfs)

public:

	bool start(IOService *provider) override;
	void stop(IOService *provider) override;

protected:
	vfstable_t vfs_handle;
};

#define super IOService
OSDefineMetaClassAndStructors(com_apple_filesystems_hfs, IOService)

extern struct vnodeopv_desc hfs_vnodeop_opv_desc;
#if CONFIG_HFS_STD
extern struct vnodeopv_desc hfs_std_vnodeop_opv_desc;
#endif
extern struct vnodeopv_desc hfs_specop_opv_desc;
extern struct vnodeopv_desc hfs_fifoop_opv_desc;
extern struct vfsops hfs_vfsops;

bool com_apple_filesystems_hfs::start(IOService *provider)
{
	if (!super::start(provider))
		return false;

#ifndef panic_on_assert
	panic_on_assert = true;
#endif

#if DEBUG
	PE_parse_boot_argn("hfs_corruption_panics", &hfs_corruption_panics, sizeof(hfs_corruption_panics));
#endif

	struct vnodeopv_desc *op_descs[] = {
		&hfs_vnodeop_opv_desc,
#if CONFIG_HFS_STD
		&hfs_std_vnodeop_opv_desc,
#endif
		&hfs_specop_opv_desc,
#if FIFO
		&hfs_fifoop_opv_desc,
#endif
	};

#define lengthof(x) (sizeof(x)/sizeof(*x))

#ifndef VFS_TBLVNOP_SECLUDE_RENAME
#define VFS_TBLVNOP_SECLUDE_RENAME 0
#endif

	struct vfs_fsentry vfe = {
		.vfe_vfsops = &hfs_vfsops,
		.vfe_vopcnt = lengthof(op_descs),
		.vfe_opvdescs = op_descs,
		.vfe_fsname = "hfs",
		.vfe_flags = (VFS_TBLNOTYPENUM | VFS_TBLLOCALVOL | VFS_TBLREADDIR_EXTENDED
					  | VFS_TBL64BITREADY | VFS_TBLVNOP_PAGEOUTV2 | VFS_TBLVNOP_PAGEINV2
					  | VFS_TBLTHREADSAFE | VFS_TBLCANMOUNTROOT | VFS_TBLVNOP_SECLUDE_RENAME
					  | VFS_TBLNATIVEXATTR)
	};

	int ret = vfs_fsadd(&vfe, &vfs_handle);

	if (ret) {
		printf("hfs: vfs_fsadd failed: %d!\n", ret);
		vfs_handle = NULL;
		return false;
	}

	hfs_init_zones();
	
	hfs_sysctl_register();

	uint32_t num_cpus;
	size_t sz = sizeof(num_cpus);

	if (sysctlbyname("hw.physicalcpu", &num_cpus,  &sz, NULL, 0) == 0) {
		if ((2 * num_cpus) > MAX_CACHED_ORIGINS_DEFAULT) {
			_hfs_max_origins = 2 * num_cpus;
			_hfs_max_file_origins = 2 * num_cpus;
		} else if ((2 * num_cpus) > MAX_CACHED_FILE_ORIGINS_DEFAULT) {
			_hfs_max_file_origins = 2 * num_cpus;
		}
	}

	return true;
}

void com_apple_filesystems_hfs::stop(IOService *provider)
{
	if (vfs_handle) {
		vfs_fsremove(vfs_handle);
		hfs_sysctl_unregister();
		vfs_handle = NULL;
	}

	super::stop(provider);
}

int hfs_is_ejectable(const char *cdev_name)
{
	int ret = 0;
	OSDictionary *dictionary;
	OSString *dev_name;

	if (strncmp(cdev_name, "/dev/", 5) == 0) {
		cdev_name += 5;
	}

	dictionary = IOService::serviceMatching("IOMedia");
	if( dictionary ) {
		dev_name = OSString::withCString( cdev_name );
		if( dev_name ) {
			IOService *service;
			mach_timespec_t tv = { 5, 0 };    // wait up to "timeout" seconds for the device

			dictionary->setObject(kIOBSDNameKey, dev_name);
			dictionary->retain();
			service = IOService::waitForService(dictionary, &tv);
			if( service ) {
				OSBoolean *ejectable = (OSBoolean *)service->getProperty("Ejectable");

				if( ejectable ) {
					ret = (int)ejectable->getValue();
				}

			}
			dev_name->release();
		}
		dictionary->release();
	}

	return ret;
}

void hfs_iterate_media_with_content(const char *content_uuid_cstring,
									int (*func)(const char *device,
												const char *uuid_str,
												void *arg),
									void *arg)
{
	OSDictionary *dictionary;
	OSString *content_uuid_string;

	dictionary = IOService::serviceMatching("IOMedia");
	if (dictionary) {
		content_uuid_string = OSString::withCString(content_uuid_cstring);
		if (content_uuid_string) {
			IOService *service;
			OSIterator *iter;

			dictionary->setObject("Content", content_uuid_string);
			dictionary->retain();

			iter = IOService::getMatchingServices(dictionary);
			while (iter && (service = (IOService *)iter->getNextObject())) {
				if (service) {
					OSString *iostr = (OSString *) service->getProperty(kIOBSDNameKey);
					OSString *uuidstr = (OSString *)service->getProperty("UUID");
					const char *uuid;

					if (iostr) {
						if (uuidstr) {
							uuid = uuidstr->getCStringNoCopy();
						} else {
							uuid = "00000000-0000-0000-0000-000000000000";
						}

						if (!func(iostr->getCStringNoCopy(), uuid, arg))
							break;
					}
				}
			}
			if (iter)
				iter->release();

			content_uuid_string->release();
		}
		dictionary->release();
	}
}

kern_return_t hfs_get_platform_serial_number(char *serial_number_str,
											 uint32_t len)
{
	OSDictionary * platform_dict;
	IOService *platform;
	OSString *  string;

	if (len < 1) {
		return 0;
	}
	serial_number_str[0] = '\0';

	platform_dict = IOService::serviceMatching( "IOPlatformExpertDevice" );
	if (platform_dict == NULL) {
		return KERN_NOT_SUPPORTED;
	}

	platform = IOService::waitForService( platform_dict );
	if (platform) {
		string = (OSString *)platform->getProperty(kIOPlatformSerialNumberKey);
		if (string == 0) {
			return KERN_NOT_SUPPORTED;
		} else {
			strlcpy( serial_number_str, string->getCStringNoCopy(), len);
		}
	}

	return KERN_SUCCESS;
}

// Interface with AKS

static aks_file_system_key_services_t *
key_services(void)
{
	static aks_file_system_key_services_t *g_key_services;

	if (!g_key_services) {
		IOService *platform = IOService::getPlatform();
		if (platform) {
			IOReturn ret = platform->callPlatformFunction
				(kAKSFileSystemKeyServices, true, &g_key_services, NULL, NULL, NULL);
			if (ret)
				printf("hfs: unable to get " kAKSFileSystemKeyServices " (0x%x)\n", ret);
		}
	}

	return g_key_services;
}

int hfs_unwrap_key(aks_cred_t access, const aks_wrapped_key_t wrapped_key_in,
				   aks_raw_key_t key_out)
{
	aks_file_system_key_services_t *ks = key_services();
	if (!ks || !ks->unwrap_key)
		return ENXIO;
	return ks->unwrap_key(access, wrapped_key_in, key_out);
}

int hfs_rewrap_key(aks_cred_t access, cp_key_class_t dp_class,
				   const aks_wrapped_key_t wrapped_key_in,
				   aks_wrapped_key_t wrapped_key_out)
{
	aks_file_system_key_services_t *ks = key_services();
	if (!ks || !ks->rewrap_key)
		return ENXIO;
	return ks->rewrap_key(access, dp_class, wrapped_key_in, wrapped_key_out);
}

int hfs_new_key(aks_cred_t access, cp_key_class_t dp_class,
				aks_raw_key_t key_out, aks_wrapped_key_t wrapped_key_out)
{
	aks_file_system_key_services_t *ks = key_services();
	if (!ks || !ks->new_key)
		return ENXIO;
	return ks->new_key(access, dp_class, key_out, wrapped_key_out);
}

int hfs_backup_key(aks_cred_t access, const aks_wrapped_key_t wrapped_key_in,
				   aks_wrapped_key_t wrapped_key_out)
{
	aks_file_system_key_services_t *ks = key_services();
	if (!ks || !ks->backup_key)
		return ENXIO;
	return ks->backup_key(access, wrapped_key_in, wrapped_key_out);
}
