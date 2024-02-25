/* SCANNER TEST */

#ifndef SMALL_TEST_CLIENT_PROTOCOL_H
#define SMALL_TEST_CLIENT_PROTOCOL_H

#include <stdint.h>
#include <stddef.h>
#include "wayland-client-core.h"

#ifdef  __cplusplus
extern "C" {
#endif

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
 * @struct intf_A_listener
 */
struct intf_A_listener {
	/**
	 */
	void (*hey)(void *data,
		    struct intf_A *intf_A);
};

/**
 * @ingroup iface_intf_A
 */
static inline int
intf_A_add_listener(struct intf_A *intf_A,
		    const struct intf_A_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) intf_A,
				     (void (**)(void)) listener, data);
}

#define INTF_A_RQ1 0
#define INTF_A_RQ2 1
#define INTF_A_DESTROY 2

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

/** @ingroup iface_intf_A */
static inline void
intf_A_set_user_data(struct intf_A *intf_A, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) intf_A, user_data);
}

/** @ingroup iface_intf_A */
static inline void *
intf_A_get_user_data(struct intf_A *intf_A)
{
	return wl_proxy_get_user_data((struct wl_proxy *) intf_A);
}

static inline uint32_t
intf_A_get_version(struct intf_A *intf_A)
{
	return wl_proxy_get_version((struct wl_proxy *) intf_A);
}

/**
 * @ingroup iface_intf_A
 */
static inline void *
intf_A_rq1(struct intf_A *intf_A, const struct wl_interface *interface, uint32_t version)
{
	struct wl_proxy *untyped_new;

	untyped_new = wl_proxy_marshal_flags((struct wl_proxy *) intf_A,
			 INTF_A_RQ1, interface, version, 0, interface->name, version, NULL);

	return (void *) untyped_new;
}

/**
 * @ingroup iface_intf_A
 */
static inline struct intf_not_here *
intf_A_rq2(struct intf_A *intf_A, const char *str, int32_t i, uint32_t u, wl_fixed_t f, int32_t fd, struct another_intf *obj)
{
	struct wl_proxy *typed_new;

	typed_new = wl_proxy_marshal_flags((struct wl_proxy *) intf_A,
			 INTF_A_RQ2, &intf_not_here_interface, wl_proxy_get_version((struct wl_proxy *) intf_A), 0, NULL, str, i, u, f, fd, obj);

	return (struct intf_not_here *) typed_new;
}

/**
 * @ingroup iface_intf_A
 */
static inline void
intf_A_destroy(struct intf_A *intf_A)
{
	wl_proxy_marshal_flags((struct wl_proxy *) intf_A,
			 INTF_A_DESTROY, NULL, wl_proxy_get_version((struct wl_proxy *) intf_A), WL_MARSHAL_FLAG_DESTROY);
}

#ifdef  __cplusplus
}
#endif

#endif
