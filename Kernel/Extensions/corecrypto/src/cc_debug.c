#include <stddef.h>
#include <corecrypto/cc.h>
#include <corecrypto/cc_debug.h>

void cc_print(const char *label, unsigned long count, const uint8_t *s) {
	size_t prefix_length = 0;

	if (label != NULL) {
		cc_printf("%s: ", label);
		prefix_length = strlen(label) + 2;
	} else {
		prefix_length = 0;
	}

	for (unsigned long index = 0; index < count; index++) {
		cc_printf("0x%02X, ", s[index]);

		if ((index % 16) == 0) {
			cc_printf("\n");

			if (prefix_length != 0) {
				for (size_t j = 0; j < prefix_length; j++) cc_printf(" ");
			}
		}
	}
}
