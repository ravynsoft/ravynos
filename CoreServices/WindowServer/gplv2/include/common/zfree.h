/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef __LABWC_ZFREE_H
#define __LABWC_ZFREE_H

void __zfree(void **ptr);

#define zfree(ptr) __zfree((void **)&(ptr))

#endif /* __LABWC_ZFREE_H */
