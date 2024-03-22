/* SCANNER TEST */

#ifndef SMALL_TEST_SERVER_PROTOCOL_H
#define SMALL_TEST_SERVER_PROTOCOL_H

#include <stdint.h>
#include <stddef.h>
#include "wayland-server-core.h"

#ifdef  __cplusplus
extern "C" {
#endif

struct wl_client;
struct wl_resource;

/**
 * @page page_small_test The small_test protocol
 * @section page_ifaces_small_test Interfaces
 * - @subpage page_iface_intf_A - the thing A
 * @section page_copyright_small_test Copyright
 * <pre>
 *
 * Copyright © 2016 Collabora, Ltd.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * </pre>
 */
struct another_intf;
struct intf_A;
struct intf_not_here;

#ifndef INTF_A_INTERFACE
#define INTF_A_INTERFACE
/**
 * @page page_iface_intf_A intf_A
 * @section page_iface_intf_A_desc Description
 *
 * A useless example trying to tickle the scanner.
 * @section page_iface_intf_A_api API
 * See @ref iface_intf_A.
 */
/**
 * @defgroup iface_intf_A The intf_A interface
 *
 * A useless example trying to tickle the scanner.
 */
extern const struct wl_interface intf_A_interface;
#endif

#ifndef INTF_A_FOO_ENUM
#define INTF_A_FOO_ENUM
enum intf_A_foo {
	/**
	 * this is the first
	 */
	INTF_A_FOO_FIRST = 0,
	/**
	 * this is the second
	 */
	INTF_A_FOO_SECOND = 1,
	/**
	 * this is the third
	 * @since 2
	 */
	INTF_A_FOO_THIRD = 2,
};
/**
 * @ingroup iface_intf_A
 */
#define INTF_A_FOO_THIRD_SINCE_VERSION 2
#endif /* INTF_A_FOO_ENUM */

/**
 * @ingroup iface_intf_A
 * @struct intf_A_interface
 */
struct intf_A_interface {
	/**
	 * @param interface name of the objects interface
	 * @param version version of the objects interface
	 */
	void (*rq1)(struct wl_client *client,
		    struct wl_resource *resource,
		    const char *interface, uint32_t version, uint32_t untyped_new);
	/**
	 */
	void (*rq2)(struct wl_client *client,
		    struct wl_resource *resource,
		    uint32_t typed_new,
		    const char *str,
		    int32_t i,
		    uint32_t u,
		    wl_fixed_t f,
		    int32_t fd,
		    struct wl_resource *obj);
	/**
	 */
	void (*destroy)(struct wl_client *client,
			struct wl_resource *resource);
};

#define INTF_A_HEY 0

/**
 * @ingroup iface_intf_A
 */
#define INTF_A_HEY_SINCE_VERSION 1

/**
 * @ingroup iface_intf_A
 */
#define INTF_A_RQ1_SINCE_VERSION 1
/**
 * @ingroup iface_intf_A
 */
#define INTF_A_RQ2_SINCE_VERSION 1
/**
 * @ingroup iface_intf_A
 */
#define INTF_A_DESTROY_SINCE_VERSION 1

/**
 * @ingroup iface_intf_A
 * Sends an hey event to the client owning the resource.
 * @param resource_ The client's resource
 */
static inline void
intf_A_send_hey(struct wl_resource *resource_)
{
	wl_resource_post_event(resource_, INTF_A_HEY);
}

#ifdef  __cplusplus
}
#endif

#endif
