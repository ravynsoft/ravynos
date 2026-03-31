/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
 *
 * multiverse_efi_smc.h
 *
 * Definitions and types from EFI and SMC which are not yet
 * exported in headers.
 */

/*
 * TODO: replace this file with real include statements from real projects */

/* START: BridgeOSBootStateLib.h */
/* repo: MacEFIFirmware */
/* file: Vendor/Apple/EfiPkg/Include/Library/BridgeOSBootStateLib.h */

#define BOOT_STATE_INVALID          0xFF
#define BOOT_STATE_OFF              0
#define BOOT_STATE_PEI              1
#define BOOT_STATE_DXE              2
#define BOOT_STATE_EARLY_BDS        3
#define BOOT_STATE_FVAPP_BOOTPICKER 4
#define BOOT_STATE_FVAPP_TDM        5
#define BOOT_STATE_FVAPP_SLINGSHOT  6
#define BOOT_STATE_FVAPP_PASSWORDUI 7
#define BOOT_STATE_LATE_BDS         8
#define BOOT_STATE_RECOVERYOS       9
#define BOOT_STATE_DIAGS            10
#define BOOT_STATE_MACOS            11
#define BOOT_STATE_OTHEROS          12
// AEN security-related states end here
#define BOOT_STATE_EXITBS           13
#define BOOT_STATE_S3               14
#define BOOT_STATE_S4               15
// MacEFIManager-processed states end here
#define BOOT_STATE_VERSION_CHECK_QUERY          16
#define BOOT_STATE_VERSION_CHECK_COMMIT         17
#define BOOT_STATE_VERSION_CHECK_TDM_COMMIT     18
#define BOOT_STATE_UPDATE_UI_START              19
#define BOOT_STATE_VERSION_CHECK_VOLUME_UNLOCK  20
#define BOOT_STATE_VERSION_CHECK_RESET_REQUIRED 21

#define BOOT_STATE_KERNEL_AUDIT_LOAD            22
#define BOOT_STATE_KEXT_AUDIT_TEST              99

/* END: BridgeOSBootStateLib.h */

/* START: ExtendedBootPolicyLib.h */
/* repo: MacEFIFirmware */
/* file: Vendor/Apple/EfiPkg/Include/Library/ExtendedBootPolicyLib.h */

typedef enum {
	// Boot Policy not valid retry.
	BootPolicyNotReady,

	// Boot Selected macOS.
	BootPolicyOk,

	// Boot Recovery OS, update bridgeOS.
	BootPolicyUpdate,

	// Full system reboot, boot selected macOS.
	BootPolicyReboot,

	// Version unknown boot to recovery OS to get more info.
	BootPolicyUnknown,

	// Update failed take the failure path.
	BootPolicyBridgeOSUpdateFailed,

	// Boot Recovery OS to change security policy.
	BootPolicyRecoverySecurityPolicyUpdate,

	// Valid values will be less that this version.
	BootPolicyMaxValue
} BOOT_POLICY_ACTION;

/* END: ExtendedBootPolicyLib.h */


/* START: ssm.h */
/* repo: AppleSMCFirmware */
/* file: include/ssm.h */
typedef enum __attribute__((packed)) {
   SYS_STATE_RUN        = 0,
   SYS_STATE_SLEEP      = 1,
   SYS_STATE_STANDBY    = 2,
   SYS_STATE_SHUTDOWN   = 3,
   SYS_STATE_INVALID    = 0xff,
} SystemState_t;

/* END: ssm.h */

/*
 * Known SMC Keys
 */
#define SMCKEY_KEXT_AUDIT_IDENTITY        'EFKI'
#define SMCKEY_KEXT_AUDIT_IDENTITY_STR    "EFKI"
#define SMCKEY_KEXT_AUDIT_IDENTITY_SIZE   100

#define SMCKEY_EFI_BOOT_STATUS            'EFBS'
#define SMCKEY_EFI_BOOT_STATUS_STR        "EFBS"
#define SMCKEY_EFI_BOOT_STATUS_SIZE       1

#define SMCKEY_EFI_MULTIBOOT_STATUS       'EFMS'
#define SMCKEY_EFI_MULTIBOOT_STATUS_STR   "EFMS"
#define SMCKEY_EFI_MULTIBOOT_STATUS_SIZE  1

#define SMCKEY_ABSTRACT_SYSTEM_STATE      'MSPR'
#define SMCKEY_ABSTRACT_SYSTEM_STATE_STR  "MSPR"
#define SMCKEY_ABSTRACT_SYSTEM_STATE_SIZE 1
