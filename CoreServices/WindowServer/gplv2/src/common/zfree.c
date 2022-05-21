// SPDX-License-Identifier: GPL-2.0-only
#include <stdlib.h>
#include "common/zfree.h"

void __zfree(void **ptr)
{
	if (!ptr || !*ptr) {
		return;
	}
	free(*ptr);
	*ptr = NULL;
}
