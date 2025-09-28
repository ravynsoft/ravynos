/*
 * ga-record-browser.h - Header for GaRecordBrowser
 * Copyright (C) 2007 Collabora Ltd.
 * @author Sjoerd Simons <sjoerd@luon.net>
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

#ifndef __GA_RECORD_BROWSER_H__
#define __GA_RECORD_BROWSER_H__

#include <glib-object.h>
#include <avahi-client/lookup.h>
#include <avahi-common/defs.h>
#include "ga-client.h"
#include "ga-enums.h"

G_BEGIN_DECLS

typedef struct _GaRecordBrowser GaRecordBrowser;
typedef struct _GaRecordBrowserClass GaRecordBrowserClass;

struct _GaRecordBrowserClass {
    GObjectClass parent_class;
};

struct _GaRecordBrowser {
    GObject parent;
};

GType ga_record_browser_get_type(void);

/* TYPE MACROS */
#define GA_TYPE_RECORD_BROWSER \
  (ga_record_browser_get_type())
#define GA_RECORD_BROWSER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), GA_TYPE_RECORD_BROWSER, GaRecordBrowser))
#define GA_RECORD_BROWSER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass), GA_TYPE_RECORD_BROWSER, GaRecordBrowserClass))
#define IS_GA_RECORD_BROWSER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), GA_TYPE_RECORD_BROWSER))
#define IS_GA_RECORD_BROWSER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass), GA_TYPE_RECORD_BROWSER))
#define GA_RECORD_BROWSER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), GA_TYPE_RECORD_BROWSER, GaRecordBrowserClass))

GaRecordBrowser *ga_record_browser_new(const gchar * name, guint16 type);

GaRecordBrowser *ga_record_browser_new_full(AvahiIfIndex interface,
                                            AvahiProtocol protocol,
                                            const gchar * name,
                                            guint16 clazz,
                                            guint16 type,
                                            GaLookupFlags flags);

gboolean
ga_record_browser_attach(GaRecordBrowser * browser,
                         GaClient * client, GError ** error);


G_END_DECLS
#endif /* #ifndef __GA_RECORD_BROWSER_H__ */
