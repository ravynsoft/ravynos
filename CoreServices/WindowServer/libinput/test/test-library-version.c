#include <assert.h>
#include <stdio.h>

int main(void) {
	const char *version = LIBINPUT_LT_VERSION;
	int C, R, A;
	int rc;

	rc = sscanf(version, "%d:%d:%d", &C, &R, &A);
	assert(rc == 3);

	assert(C >= 17);
	assert(R >= 0);
	assert(A >= 7);

	/* Binary compatibility broken? */
	assert(R != 0 || A != 0);

	/* The first stable API in 0.12 had 10:0:0  */
	assert(C - A == 10);

	return 0;
}
