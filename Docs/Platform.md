# Notes on ravynOS PlatformExpert https://music.apple.com/ca/song/lebenslinien/1810750529

### Device tree /defaults entries
| Entry                     | Description                                                       |
|---------------------------|-------------------------------------------------------------------|
| kern.vm_compressor        | Compressed pager configuration (default 0x4,                      |
|                           |    VM_PAGER_COMPRESSOR_WITH_SWAP)                                 |

```
    kern.maxvnodes
    kern.jetsam_delta
    kern.jetsam_critical_threshold
    kern.jetsam_idle_offset
    kern.jetsam_pressure_threshold
    kern.jetsam_freeze_threshold
    kern.jetsam_aging_policy
    kern.jetsam_idle_snapshot
    kern.io_throttle_window_tier1
    kern.io_throttle_window_tier2
    kern.io_throttle_window_tier3
    kern.io_throttle_period_tier1
    kern.io_throttle_period_tier2
    kern.io_throttle_period_tier3
    kern.maxkfsevents
    hw.memsize
    kern.disable_atm
    kern.atm_diagnostic_config
    kern.preferred_cpu_type
    kern.preferred_cpu_subtype
    kern.sched_pri_decay_limit
    kern.max_task_pmem
    kern.max_cpumon_percentage
    kern.max_cpumon_interval
    kern.vm_compressor
    kern.secluded_mem_mb
    progress-dy
```

### Device tree /chosen/memory-map entries
| Entry           | Description                                                                 |
|-----------------|-----------------------------------------------------------------------------|
| BootCLUT        | Color table for boot graphics framebuffer (OPTIONAL)                        |
| Pict-FailedBoot | Override the progress element for a failed boot (OPTIONAL)                  |
| FailedImage     | Override the icon for a failed boot (OPTIONAL)                              |
| FailedCLUT      | Override the color table for a failed boot (OPTIONAL)                       |
| DeviceTree      | Virtual address of the device tree blob. This is used during IOKit startup. |

A CLUT is a length byte followed by up to 256 24-bit RGB values corresponding to the colors numbered 0x00 to 0xFF.
Image data has a small header with dimension info and then is a pixel map where each entry is a color from the CLUT.  

In `pexpert/i386/boot_images.h`:
```
struct boot_icon_element {
	unsigned int    width;
	unsigned int    height;
	int             y_offset_from_center;
	unsigned int    data_size;
	unsigned int    __reserved1[4];
	unsigned char   data[0];
};
```

In `pexpert/pe_images.h`:
```
struct boot_progress_element {
	unsigned int        width;
	unsigned int        height;
	int                 yOffset;
	unsigned int        res[5];
	unsigned char       data[0];
};

```

### IOKit Registry properties

See `iokit/IOKit/IOKitKeys.h`

```
kDIRootImageResultKey
kDIRootImageDevTKey
kDIRootImageDevNameKey
kIOBSDNameKey
kIOBSDMajorKey
kIOBSDMinorKey
kIOPlatformUUIDKey
kIOReportLegendKey
kIOPMDeepSleepEnabledKey
kIOPMDestroyFVKeyOnStandbyKey
kIOHibernateMachineSignatureKey
kIOHibernatePreviewActiveKey
kIOScreenLockStateKey
kIOHibernateSMCVariablesKey
kIOPlatformMapperPresentKey
kIOPMRootDomainWakeTypeKey
kIOPMRootDomainWakeReasonKey
kIOHibernateOptionsKey
kRootDomainSupportedFeatures
kIOUserClientSharedInstanceKey
kIOUserClientMessageAppSuspendedKey
kIOUserClientCrossEndianCompatibleKey

"top-level"
"AAPL,ignore"
"AAPL,phandle"
"AAPL,slot-name"
"serial-number"
"unique-chip-id" 
"chip-id" 
"system-id" 
"platform-uuid" 
"apple-coprocessor-version"
"boot-device-path"
"boot-file-path"
"boot-ramdmg-size"
"boot-ramdmg-extents"
"boot-uuid" 
"acpi-device"
"no-idle"
"root-matching" 
"RAMDisk"
"reg" 
"interrupt-controller"
"interrupt-map"
"interrupt-map-mask" 
"interrupts"
"slot-names"
"dma-parent"
"iommu-id"
"Platform Memory Ranges"
"halt-restart-timeout" 
"IOPMStrictTreeOrder"
"IOPMUnattendedWakePowerState"
"IOPCITunnelled"
```