#include <stddef.h>
#include <corecrypto/cc.h>

int cc_cmp_safe(size_t num, const void *ptr1, const void *ptr2) {
	if (num == 0) return 1;

	volatile const unsigned char *buffer1 = ptr1;
	volatile const unsigned char *buffer2 = ptr2;
	unsigned char result = 0;

	while (num != 0) {
		result |= *buffer1 ^ *buffer2;
		buffer1++;
		buffer2++;
		num--;
	}

	return !!result;
}
