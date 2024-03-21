/*
 * ga-client.h - Header for GaClient
 * Copyright (C) 2006-2007 Collabora Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __GA_CLIENT_H__
#define __GA_CLIENT_H__

#include <glib-object.h>
#include <avahi-client/client.h>

G_BEGIN_DECLS

typedef enum {
    GA_CLIENT_STATE_NOT_STARTED = -1,
    GA_CLIENT_STATE_S_REGISTERING = AVAHI_CLIENT_S_REGISTERING,
    GA_CLIENT_STATE_S_RUNNING = AVAHI_CLIENT_S_RUNNING,
    GA_CLIENT_STATE_S_COLLISION = AVAHI_CLIENT_S_COLLISION,
    GA_CLIENT_STATE_FAILURE = AVAHI_CLIENT_FAILURE,
    GA_CLIENT_STATE_CONNECTING = AVAHI_CLIENT_CONNECTING
} GaClientState;

typedef enum {
    GA_CLIENT_FLAG_NO_FLAGS = 0,
    GA_CLIENT_FLAG_IGNORE_USER_CONFIG = AVAHI_CLIENT_IGNORE_USER_CONFIG,
    GA_CLIENT_FLAG_NO_FAIL = AVAHI_CLIENT_NO_FAIL
} GaClientFlags;

typedef struct _GaClient GaClient;
typedef struct _GaClientClass GaClientClass;

struct _GaClientClass {
    GObjectClass parent_class;
};

struct _GaClient {
    GObject parent;
    /* Raw avahi_client handle, only reuse if you have reffed this instance */
    AvahiClient *avahi_client;
};

GType ga_client_get_type(void);

/* TYPE MACROS */
#define GA_TYPE_CLIENT \
    (ga_client_get_type())
#define GA_CLIENT(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), GA_TYPE_CLIENT, GaClient))
#define GA_CLIENT_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), GA_TYPE_CLIENT, GaClientClass))
#define IS_GA_CLIENT(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), GA_TYPE_CLIENT))
#define IS_GA_CLIENT_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), GA_TYPE_CLIENT))
#define GA_CLIENT_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), GA_TYPE_CLIENT, GaClientClass))

GaClient *ga_client_new(GaClientFlags flags);

gboolean ga_client_start(GaClient * client, GError ** error);

gboolean ga_client_start_in_context(GaClient * client, GMainContext * context, GError ** error);

G_END_DECLS

#endif /* #ifndef __GA_CLIENT_H__ */
