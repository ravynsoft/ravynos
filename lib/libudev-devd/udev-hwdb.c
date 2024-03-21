/*
 * Copyright (c) 2021 Vladimir Kondratyev <vladimir@kondratyev.su>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "utils.h"

#include <stddef.h>

#include "libudev.h"
#include "udev-utils.h"

LIBUDEV_EXPORT struct udev_hwdb *
udev_hwdb_new(struct udev *udev)
{
	TRC("(%p", udev);
	UNIMPL();
	return (NULL);
}

LIBUDEV_EXPORT struct udev_hwdb *
udev_hwdb_ref(struct udev_hwdb *uh)
{
	TRC("(%p", uh);
	UNIMPL();
	return (NULL);
}

LIBUDEV_EXPORT struct udev_hwdb *
udev_hwdb_unref(struct udev_hwdb *uh)
{
	TRC("(%p", uh);
	UNIMPL();
	return (NULL);
}

LIBUDEV_EXPORT struct udev_list_entry *
udev_hwdb_get_properties_list_entry(struct udev_hwdb *uh, const char *modalias,
    unsigned int flags)
{
	TRC("(%p, %s, %u", uh, modalias, flags);
	UNIMPL();
	return (NULL);
}
