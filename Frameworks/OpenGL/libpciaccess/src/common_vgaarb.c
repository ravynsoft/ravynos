/*
 * Copyright (c) 2007 Paulo R. Zanoni, Tiago Vignatti
 *               2009 Tiago Vignatti
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>

#include "pciaccess.h"
#include "pciaccess_private.h"

#define BUFSIZE 64

static int
parse_string_to_decodes_rsrc(char *input, int *vga_count, struct pci_slot_match *match)
{
    char *tok;
    char *input_sp = NULL, *count_sp, *pci_sp;
    char tmp[32];

    tok = strtok_r(input,",",&input_sp);
    if (!tok)
        goto fail;

    strncpy(tmp, input, 15);
    tmp[15] = 0;

    tok = strtok_r(tmp,":",&count_sp);
    if (!tok)
        goto fail;
    tok = strtok_r(NULL, ":",&count_sp);
    if (!tok)
        goto fail;

    *vga_count = strtoul(tok, NULL, 10);
    if (*vga_count == LONG_MAX)
        goto fail;

#ifdef DEBUG
    fprintf(stderr,"vga count is %d\n", *vga_count);
#endif

    tok = strtok_r(NULL, ",",&input_sp);
    if (!tok)
        goto fail;

    if (match) {
        strncpy(tmp, tok, 32);
        tmp[31] = 0;
        tok = strtok_r(tmp, ":", &pci_sp);
        if (!tok)
            goto fail;
        tok = strtok_r(NULL, ":", &pci_sp);
        if (!tok)
            goto fail;
        match->domain = strtoul(tok, NULL, 16);

        tok = strtok_r(NULL, ":", &pci_sp);
        if (!tok)
            goto fail;
        match->bus = strtoul(tok, NULL, 16);

        tok = strtok_r(NULL, ".", &pci_sp);
        if (!tok)
            goto fail;
        match->dev = strtoul(tok, NULL, 16);

        tok = strtok_r(NULL, ".", &pci_sp);
        if (!tok)
            goto fail;
        match->func = strtoul(tok, NULL, 16);
    }

    tok = strtok_r(NULL, ",",&input_sp);
    if (!tok)
        goto fail;
    tok = strtok_r(tok, "=", &input_sp);
    if (!tok)
        goto fail;
    tok = strtok_r(NULL, "=", &input_sp);
    if (!tok)
        goto fail;

    if (!strncmp(tok, "io+mem", 6))
        return VGA_ARB_RSRC_LEGACY_IO | VGA_ARB_RSRC_LEGACY_MEM;
    if (!strncmp(tok, "io", 2))
        return VGA_ARB_RSRC_LEGACY_IO;
    if (!strncmp(tok, "mem", 3))
        return VGA_ARB_RSRC_LEGACY_MEM;
fail:
    return VGA_ARB_RSRC_NONE;
}

int
pci_device_vgaarb_init(void)
{
    struct pci_slot_match match;
    char buf[BUFSIZE + 1]; /* reading BUFSIZE characters, + 1 for NULL */
    int ret, rsrc;

    if (!pci_sys)
        return -1;

    if ((pci_sys->vgaarb_fd = open ("/dev/vga_arbiter", O_RDWR | O_CLOEXEC)) < 0) {
        return errno;
    }

    ret = read(pci_sys->vgaarb_fd, buf, BUFSIZE);
    if (ret <= 0)
        return -1;

    buf[ret] = 0; /* ret will never be greater than BUFSIZE */

    memset(&match, 0xff, sizeof(match));
    /* need to find the device to go back to and what it was decoding */
    rsrc = parse_string_to_decodes_rsrc(buf, &pci_sys->vga_count, &match);

    pci_sys->vga_default_dev = pci_device_find_by_slot(match.domain, match.bus, match.dev, match.func);

    if (pci_sys->vga_default_dev)
        pci_sys->vga_default_dev->vgaarb_rsrc = rsrc;
    return 0;
}

void
pci_device_vgaarb_fini(void)
{
    if (!pci_sys)
        return;

    close(pci_sys->vgaarb_fd);
}

/**
 * Writes message on vga device. The messages are defined by the kernel
 * implementation.
 *
 * \param fd    vga arbiter device.
 * \param buf   message itself.
 * \param len   message length.
 *
 * \return
 * Zero on success, 1 if something gets wrong and 2 if fd is busy (only for
 * 'trylock')
 */
static int
vgaarb_write(int fd, char *buf, int len)
{
    int ret;


    buf[len] = '\0';

    ret = write(fd, buf, len);
    if (ret == -1) {
        /* the user may have called "trylock" and didn't get the lock */
        if (errno == EBUSY)
            return 2;

#ifdef DEBUG
        fprintf(stderr, "write error");
#endif
        return 1;
    }
    else if (ret != len) {
        /* it's need to receive the exactly amount required. */
#ifdef DEBUG
        fprintf(stderr, "write error: wrote different than expected\n");
#endif
        return 1;
    }

#ifdef DEBUG
    fprintf(stderr, "%s: successfully wrote: '%s'\n", __FUNCTION__, buf);
#endif

    return 0;
}


static const char *
rsrc_to_str(int iostate)
{
    switch (iostate) {
    case VGA_ARB_RSRC_LEGACY_IO | VGA_ARB_RSRC_LEGACY_MEM:
        return "io+mem";
    case VGA_ARB_RSRC_LEGACY_IO:
        return "io";
    case VGA_ARB_RSRC_LEGACY_MEM:
        return "mem";
    }

    return "none";
}

int
pci_device_vgaarb_set_target(struct pci_device *dev)
{
    int len;
    char buf[BUFSIZE + 1]; /* reading BUFSIZE characters, + 1 for NULL */
    int ret;

    if (!dev)
        dev = pci_sys->vga_default_dev;
    if (!dev)
        return -1;

    len = snprintf(buf, BUFSIZE, "target PCI:%04x:%02x:%02x.%x",
                   dev->domain, dev->bus, dev->dev, dev->func);

    ret = vgaarb_write(pci_sys->vgaarb_fd, buf, len);
    if (ret)
        return ret;

    ret = read(pci_sys->vgaarb_fd, buf, BUFSIZE);
    if (ret <= 0)
        return -1;

    buf[ret] = 0; /* ret will never be greater than BUFSIZE */

    dev->vgaarb_rsrc = parse_string_to_decodes_rsrc(buf, &pci_sys->vga_count, NULL);
    pci_sys->vga_target = dev;
    return 0;
}

int
pci_device_vgaarb_decodes(int new_vgaarb_rsrc)
{
    int len;
    char buf[BUFSIZE + 1]; /* reading BUFSIZE characters, + 1 for NULL */
    int ret;
    struct pci_device *dev = pci_sys->vga_target;

    if (!dev)
        return -1;
    if (dev->vgaarb_rsrc == new_vgaarb_rsrc)
        return 0;

    len = snprintf(buf, BUFSIZE, "decodes %s", rsrc_to_str(new_vgaarb_rsrc));
    ret = vgaarb_write(pci_sys->vgaarb_fd, buf, len);
    if (ret == 0)
        dev->vgaarb_rsrc = new_vgaarb_rsrc;

    ret = read(pci_sys->vgaarb_fd, buf, BUFSIZE);
    if (ret <= 0)
        return -1;

    buf[ret] = 0; /* ret will never be greater than BUFSIZE */

    parse_string_to_decodes_rsrc(buf, &pci_sys->vga_count, NULL);

    return ret;
}

int
pci_device_vgaarb_lock(void)
{
    int len;
    char buf[BUFSIZE];
    struct pci_device *dev = pci_sys->vga_target;

    if (!dev)
	return -1;

    if (dev->vgaarb_rsrc == 0 || pci_sys->vga_count == 1)
        return 0;

    len = snprintf(buf, BUFSIZE, "lock %s", rsrc_to_str(dev->vgaarb_rsrc));

    return vgaarb_write(pci_sys->vgaarb_fd, buf, len);
}

int
pci_device_vgaarb_trylock(void)
{
    int len;
    char buf[BUFSIZE];
    struct pci_device *dev = pci_sys->vga_target;

    if (!dev)
        return -1;

    if (dev->vgaarb_rsrc == 0 || pci_sys->vga_count == 1)
        return 0;

    len = snprintf(buf, BUFSIZE, "trylock %s", rsrc_to_str(dev->vgaarb_rsrc));

    return vgaarb_write(pci_sys->vgaarb_fd, buf, len);
}

int
pci_device_vgaarb_unlock(void)
{
    int len;
    char buf[BUFSIZE];
    struct pci_device *dev = pci_sys->vga_target;

    if (!dev)
        return -1;

    if (dev->vgaarb_rsrc == 0 || pci_sys->vga_count == 1)
        return 0;

    len = snprintf(buf, BUFSIZE, "unlock %s", rsrc_to_str(dev->vgaarb_rsrc));

    return vgaarb_write(pci_sys->vgaarb_fd, buf, len);
}

int pci_device_vgaarb_get_info(struct pci_device *dev, int *vga_count, int *rsrc_decodes)
{
    *vga_count = pci_sys->vga_count;
    if (!dev)
        return 0;

    *rsrc_decodes = dev->vgaarb_rsrc;
        return 0;
}
