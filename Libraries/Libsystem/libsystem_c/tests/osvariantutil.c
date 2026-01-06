#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <os/variant_private.h>

#include "../libdarwin/variant.c"

#define bool2str(b) (b ? "true" : "false")

static void __dead2
usage(void)
{
	printf("osvariantutil status\n");
	printf("osvariantutil parse <kern.osvariant_status>\n");
	exit(1);
}

int
main(int argc, char *argv[]) {
	// Warm up the dispatch_once
	_check_disabled(VP_CONTENT);

	if (argc == 2 && strcmp(argv[1], "status") == 0) {
		uint64_t status = _get_cached_check_status();
		printf("Cached status: %llx\n", status);
	} else if (argc == 3 && strcmp(argv[1], "parse") == 0) {
		uint64_t status = strtoull(argv[2], NULL, 0);
		if ((status & STATUS_INITIAL_BITS) != STATUS_INITIAL_BITS) {
			printf("Invalid status: 0x%llx\n", status);
			exit(1);
		}
		_restore_cached_check_status(status);
		printf("Using status: %llx\n", status);
	} else {
		usage();
	}

	printf("\nOS Variants:\n");
	printf("\tos_variant_has_internal_content: %s\n",
			bool2str(os_variant_has_internal_content("com.apple.osvariantutil")));
	printf("\tos_variant_has_internal_diagnostics: %s\n",
			bool2str(os_variant_has_internal_diagnostics("com.apple.osvariantutil")));
	printf("\tos_variant_has_internal_ui: %s\n",
			bool2str(os_variant_has_internal_ui("com.apple.osvariantutil")));
	printf("\tos_variant_allows_internal_security_properties: %s\n",
		   	bool2str(os_variant_allows_internal_security_policies("com.apple.osvariantutil")));
	printf("\tos_variant_has_factory_content: %s\n",
			bool2str(os_variant_has_factory_content("com.apple.osvariantutil")));
	printf("\tos_variant_is_darwinos: %s\n",
			bool2str(os_variant_is_darwinos("com.apple.osvariantutil")));
	printf("\tos_variant_uses_ephemeral_storage: %s\n",
			bool2str(os_variant_uses_ephemeral_storage("com.apple.osvariantutil")));
	printf("\tos_variant_is_recovery: %s\n",
			bool2str(os_variant_is_recovery("com.apple.osvariantutil")));

	printf("\nOS Variant Overrides:\n");
	printf("\tCONTENT: %s\n", bool2str(_check_disabled(VP_CONTENT)));
	printf("\tDIAGNOSTICS: %s\n", bool2str(_check_disabled(VP_DIAGNOSTICS)));
	printf("\tUI: %s\n", bool2str(_check_disabled(VP_UI)));
	printf("\tSECURITY: %s\n", bool2str(_check_disabled(VP_SECURITY)));

	printf("\nOS Variant Inputs:\n");
#if !TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
	printf("\tInternal Content: %s\n", bool2str(_check_internal_content()));
#endif
#if TARGET_OS_IPHONE
	printf("\tInternal Release Type: %s\n", bool2str(_check_internal_release_type()));
	printf("\tFactory Release Type: %s\n", bool2str(_check_factory_release_type()));
	printf("\tDarwin Release Type: %s\n", bool2str(_check_darwin_release_type()));
	printf("\tRecovery Release Type: %s\n", bool2str(_check_recovery_release_type()));
	printf("\tDevelopment Kernel: %s\n", bool2str(_check_development_kernel()));
#else
	printf("\tInternal Diags Profile: %s\n", bool2str(_check_internal_diags_profile()));
	printf("\tFactory Content: %s\n", bool2str(_check_factory_content()));
	printf("\tBaseSystem Content: %s\n", bool2str(_check_base_system_content()));
#endif
	printf("\tCan Has Debugger: %s\n", bool2str(_check_can_has_debugger()));

	return 0;
}
