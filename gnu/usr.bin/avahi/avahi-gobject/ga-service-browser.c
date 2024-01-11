/*
 * ga-service-browser.c - Source for GaServiceBrowser
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-common/error.h>

#include "ga-service-browser.h"
#include "signals-marshal.h"
#include "ga-error.h"
#include "ga-enums-enumtypes.h"

G_DEFINE_TYPE(GaServiceBrowser, ga_service_browser, G_TYPE_OBJECT)

/* signal enum */
enum {
    NEW,
    REMOVED,
    CACHE_EXHAUSTED,
    ALL_FOR_NOW,
    FAILURE,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

/* properties */
enum {
    PROP_PROTOCOL = 1,
    PROP_IFINDEX,
    PROP_TYPE,
    PROP_DOMAIN,
    PROP_FLAGS
};

/* private structure */
typedef struct _GaServiceBrowserPrivate GaServiceBrowserPrivate;

struct _GaServiceBrowserPrivate {
    GaClient *client;
    AvahiServiceBrowser *browser;
    AvahiIfIndex interface;
    AvahiProtocol protocol;
    char *type;
    char *domain;
    AvahiLookupFlags flags;
    gboolean dispose_has_run;
};

#define GA_SERVICE_BROWSER_GET_PRIVATE(o)     (G_TYPE_INSTANCE_GET_PRIVATE ((o), GA_TYPE_SERVICE_BROWSER, GaServiceBrowserPrivate))

static void ga_service_browser_init(GaServiceBrowser * obj) {
    GaServiceBrowserPrivate *priv = GA_SERVICE_BROWSER_GET_PRIVATE(obj);

    /* allocate any data required by the object here */
    priv->client = NULL;
    priv->browser = NULL;
    priv->type = NULL;
    priv->domain = NULL;

}

static void ga_service_browser_dispose(GObject * object);
static void ga_service_browser_finalize(GObject * object);

static void ga_service_browser_set_property(GObject * object,
                                guint property_id,
                                const GValue * value, GParamSpec * pspec) {
    GaServiceBrowser *browser = GA_SERVICE_BROWSER(object);
    GaServiceBrowserPrivate *priv = GA_SERVICE_BROWSER_GET_PRIVATE(browser);

    g_assert(priv->browser == NULL);
    switch (property_id) {
        case PROP_PROTOCOL:
            priv->protocol = g_value_get_enum(value);
            break;
        case PROP_IFINDEX:
            priv->interface = g_value_get_int(value);
            break;
        case PROP_TYPE:
            priv->type = g_strdup(g_value_get_string(value));
            break;
        case PROP_DOMAIN:
            priv->domain = g_strdup(g_value_get_string(value));
            break;
        case PROP_FLAGS:
            priv->flags = g_value_get_enum(value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

static void ga_service_browser_get_property(GObject * object,
                                guint property_id,
                                GValue * value, GParamSpec * pspec) {
    GaServiceBrowser *browser = GA_SERVICE_BROWSER(object);
    GaServiceBrowserPrivate *priv = GA_SERVICE_BROWSER_GET_PRIVATE(browser);

    switch (property_id) {
        case PROP_PROTOCOL:
            g_value_set_int(value, priv->protocol);
            break;
        case PROP_IFINDEX:
            g_value_set_int(value, priv->interface);
            break;
        case PROP_TYPE:
            g_value_set_string(value, priv->type);
            break;
        case PROP_DOMAIN:
            g_value_set_string(value, priv->domain);
            break;
        case PROP_FLAGS:
            g_value_set_enum(value, priv->flags);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}


static void ga_service_browser_class_init(GaServiceBrowserClass *
                              ga_service_browser_class) {
    GObjectClass *object_class = G_OBJECT_CLASS(ga_service_browser_class);
    GParamSpec *param_spec;

    g_type_class_add_private(ga_service_browser_class,
                             sizeof (GaServiceBrowserPrivate));

    object_class->dispose = ga_service_browser_dispose;
    object_class->finalize = ga_service_browser_finalize;

    object_class->set_property = ga_service_browser_set_property;
    object_class->get_property = ga_service_browser_get_property;

    signals[NEW] =
            g_signal_new("new-service",
                         G_OBJECT_CLASS_TYPE(ga_service_browser_class),
                         G_SIGNAL_RUN_LAST,
                         0,
                         NULL, NULL,
                         _ga_signals_marshal_VOID__INT_ENUM_STRING_STRING_STRING_UINT,
                         G_TYPE_NONE, 6,
                         G_TYPE_INT,
                         GA_TYPE_PROTOCOL,
                         G_TYPE_STRING,
                         G_TYPE_STRING,
                         G_TYPE_STRING, GA_TYPE_LOOKUP_RESULT_FLAGS);

    signals[REMOVED] =
            g_signal_new("removed-service",
                         G_OBJECT_CLASS_TYPE(ga_service_browser_class),
                         G_SIGNAL_RUN_LAST,
                         0,
                         NULL, NULL,
                         _ga_signals_marshal_VOID__INT_ENUM_STRING_STRING_STRING_UINT,
                         G_TYPE_NONE, 6,
                         G_TYPE_INT,
                         GA_TYPE_PROTOCOL,
                         G_TYPE_STRING,
                         G_TYPE_STRING,
                         G_TYPE_STRING, GA_TYPE_LOOKUP_RESULT_FLAGS);

    signals[ALL_FOR_NOW] =
            g_signal_new("all-for-now",
                         G_OBJECT_CLASS_TYPE(ga_service_browser_class),
                         G_SIGNAL_RUN_LAST,
                         0,
                         NULL, NULL,
                         g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

    signals[CACHE_EXHAUSTED] =
            g_signal_new("cache-exhausted",
                         G_OBJECT_CLASS_TYPE(ga_service_browser_class),
                         G_SIGNAL_RUN_LAST,
                         0,
                         NULL, NULL,
                         g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

    signals[FAILURE] =
            g_signal_new("failure",
                         G_OBJECT_CLASS_TYPE(ga_service_browser_class),
                         G_SIGNAL_RUN_LAST,
                         0,
                         NULL, NULL,
                         g_cclosure_marshal_VOID__POINTER,
                         G_TYPE_NONE, 1, G_TYPE_POINTER);

    param_spec = g_param_spec_enum("protocol", "Avahi protocol to browse",
                                   "Avahi protocol to browse",
                                   GA_TYPE_PROTOCOL,
                                   GA_PROTOCOL_UNSPEC,
                                   G_PARAM_READWRITE |
                                   G_PARAM_STATIC_NAME |
                                   G_PARAM_STATIC_BLURB);
    g_object_class_install_property(object_class, PROP_PROTOCOL, param_spec);

    param_spec = g_param_spec_int("interface", "interface index",
                                  "Interface use for browser",
                                  AVAHI_IF_UNSPEC,
                                  G_MAXINT,
                                  AVAHI_IF_UNSPEC,
                                  G_PARAM_READWRITE |
                                  G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB);
    g_object_class_install_property(object_class, PROP_IFINDEX, param_spec);

    param_spec = g_param_spec_string("type", "service type",
                                     "Service type to browse for",
                                     NULL,
                                     G_PARAM_READWRITE |
                                     G_PARAM_STATIC_NAME |
                                     G_PARAM_STATIC_BLURB);
    g_object_class_install_property(object_class, PROP_TYPE, param_spec);

    param_spec = g_param_spec_string("domain", "service domain",
                                     "Domain to browse in",
                                     NULL,
                                     G_PARAM_READWRITE |
                                     G_PARAM_STATIC_NAME |
                                     G_PARAM_STATIC_BLURB);
    g_object_class_install_property(object_class, PROP_DOMAIN, param_spec);

    param_spec = g_param_spec_enum("flags", "Lookup flags for the browser",
                                   "Browser lookup flags",
                                   GA_TYPE_LOOKUP_FLAGS,
                                   GA_LOOKUP_NO_FLAGS,
                                   G_PARAM_READWRITE |
                                   G_PARAM_STATIC_NAME |
                                   G_PARAM_STATIC_BLURB);
    g_object_class_install_property(object_class, PROP_FLAGS, param_spec);
}

void ga_service_browser_dispose(GObject * object) {
    GaServiceBrowser *self = GA_SERVICE_BROWSER(object);
    GaServiceBrowserPrivate *priv = GA_SERVICE_BROWSER_GET_PRIVATE(self);

    if (priv->dispose_has_run)
        return;

    priv->dispose_has_run = TRUE;

    if (priv->browser)
        avahi_service_browser_free(priv->browser);
    priv->browser = NULL;
    if (priv->client)
        g_object_unref(priv->client);
    priv->client = NULL;

    /* release any references held by the object here */

    if (G_OBJECT_CLASS(ga_service_browser_parent_class)->dispose)
        G_OBJECT_CLASS(ga_service_browser_parent_class)->dispose(object);
}

void ga_service_browser_finalize(GObject * object) {
    GaServiceBrowser *self = GA_SERVICE_BROWSER(object);
    GaServiceBrowserPrivate *priv = GA_SERVICE_BROWSER_GET_PRIVATE(self);

    /* free any data held directly by the object here */
    g_free(priv->type);
    priv->type = NULL;
    g_free(priv->domain);
    priv->domain = NULL;

    G_OBJECT_CLASS(ga_service_browser_parent_class)->finalize(object);
}

static void _avahi_service_browser_cb(AvahiServiceBrowser * b, AvahiIfIndex interface,
                          AvahiProtocol protocol, AvahiBrowserEvent event,
                          const char *name, const char *type,
                          const char *domain, AvahiLookupResultFlags flags,
                          void *userdata) {
    GaServiceBrowser *self = GA_SERVICE_BROWSER(userdata);
    GaServiceBrowserPrivate *priv = GA_SERVICE_BROWSER_GET_PRIVATE(self);
    if (priv->browser == NULL) {
        priv->browser = b;
    }
    g_assert(priv->browser == b);

    switch (event) {
        case AVAHI_BROWSER_NEW:
        case AVAHI_BROWSER_REMOVE:{
                guint signalid;
                signalid = (event == AVAHI_BROWSER_NEW ? NEW : REMOVED);
                g_signal_emit(self, signals[signalid], 0,
                              interface, protocol, name, type, domain, flags);
                break;
            }
        case AVAHI_BROWSER_CACHE_EXHAUSTED:
            g_signal_emit(self, signals[CACHE_EXHAUSTED], 0);
            break;
        case AVAHI_BROWSER_ALL_FOR_NOW:
            g_signal_emit(self, signals[ALL_FOR_NOW], 0);
            break;
        case AVAHI_BROWSER_FAILURE:{
                GError *error;
                int aerrno = avahi_client_errno(priv->client->avahi_client);
                error = g_error_new(GA_ERROR, aerrno,
                                    "Browsing failed: %s",
                                    avahi_strerror(aerrno));
                g_signal_emit(self, signals[FAILURE], 0, error);
                g_error_free(error);
                break;
            }
    }
}

GaServiceBrowser *ga_service_browser_new(const gchar * type) {
    return ga_service_browser_new_full(AVAHI_IF_UNSPEC,
                                       AVAHI_PROTO_UNSPEC, type, NULL, 0);
}

GaServiceBrowser *ga_service_browser_new_full(AvahiIfIndex interface,
                                              AvahiProtocol protocol,
                                              const gchar * type, gchar * domain,
                                              GaLookupFlags flags) {
    return g_object_new(GA_TYPE_SERVICE_BROWSER,
                        "interface", interface,
                        "protocol", protocol,
                        "type", type, "domain", domain, "flags", flags, NULL);
}

gboolean ga_service_browser_attach(GaServiceBrowser * browser,
                          GaClient * client, GError ** error) {
    GaServiceBrowserPrivate *priv = GA_SERVICE_BROWSER_GET_PRIVATE(browser);

    g_object_ref(client);
    priv->client = client;

    priv->browser = avahi_service_browser_new(client->avahi_client,
                                              priv->interface,
                                              priv->protocol,
                                              priv->type, priv->domain,
                                              priv->flags,
                                              _avahi_service_browser_cb,
                                              browser);
    if (priv->browser == NULL) {
        if (error != NULL) {
            int aerrno = avahi_client_errno(client->avahi_client);
            *error = g_error_new(GA_ERROR, aerrno,
                                 "Attaching group failed: %s",
                                 avahi_strerror(aerrno));
        }
        return FALSE;
    }
    return TRUE;
}
