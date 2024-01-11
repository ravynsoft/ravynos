/*
 * ga-service-browser.h - Header for GaServiceBrowser
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

#ifndef __GA_SERVICE_BROWSER_H__
#define __GA_SERVICE_BROWSER_H__

#include <glib-object.h>
#include <avahi-client/lookup.h>
#include <avahi-common/defs.h>
#include "ga-client.h"
#include "ga-enums.h"

G_BEGIN_DECLS

typedef struct _GaServiceBrowser GaServiceBrowser;
typedef struct _GaServiceBrowserClass GaServiceBrowserClass;

struct _GaServiceBrowserClass {
    GObjectClass parent_class;
};

struct _GaServiceBrowser {
    GObject parent;
};

GType ga_service_browser_get_type(void);

/* TYPE MACROS */
#define GA_TYPE_SERVICE_BROWSER \
  (ga_service_browser_get_type())
#define GA_SERVICE_BROWSER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), GA_TYPE_SERVICE_BROWSER, GaServiceBrowser))
#define GA_SERVICE_BROWSER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass), GA_TYPE_SERVICE_BROWSER, GaServiceBrowserClass))
#define IS_GA_SERVICE_BROWSER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), GA_TYPE_SERVICE_BROWSER))
#define IS_GA_SERVICE_BROWSER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass), GA_TYPE_SERVICE_BROWSER))
#define GA_SERVICE_BROWSER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), GA_TYPE_SERVICE_BROWSER, GaServiceBrowserClass))

GaServiceBrowser *ga_service_browser_new(const gchar * type);

GaServiceBrowser *ga_service_browser_new_full(AvahiIfIndex interface,
                                              AvahiProtocol protocol,
                                              const gchar * type, gchar * domain,
                                              GaLookupFlags flags);

gboolean
ga_service_browser_attach(GaServiceBrowser * browser,
                          GaClient * client, GError ** error);


G_END_DECLS
#endif /* #ifndef __GA_SERVICE_BROWSER_H__ */
