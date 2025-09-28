/*
 * ga-service-resolver.h - Header for GaServiceResolver
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

#ifndef __GA_SERVICE_RESOLVER_H__
#define __GA_SERVICE_RESOLVER_H__


#include <avahi-common/address.h>

#include <glib-object.h>
#include "ga-client.h"
#include "ga-enums.h"

G_BEGIN_DECLS

typedef struct _GaServiceResolver GaServiceResolver;
typedef struct _GaServiceResolverClass GaServiceResolverClass;

struct _GaServiceResolverClass {
    GObjectClass parent_class;
};

struct _GaServiceResolver {
    GObject parent;
};

GType ga_service_resolver_get_type(void);

/* TYPE MACROS */
#define GA_TYPE_SERVICE_RESOLVER \
  (ga_service_resolver_get_type())
#define GA_SERVICE_RESOLVER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), GA_TYPE_SERVICE_RESOLVER, GaServiceResolver))
#define GA_SERVICE_RESOLVER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass), GA_TYPE_SERVICE_RESOLVER, GaServiceResolverClass))
#define IS_GA_SERVICE_RESOLVER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), GA_TYPE_SERVICE_RESOLVER))
#define IS_GA_SERVICE_RESOLVER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass), GA_TYPE_SERVICE_RESOLVER))
#define GA_SERVICE_RESOLVER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), GA_TYPE_SERVICE_RESOLVER, GaServiceResolverClass))

GaServiceResolver *ga_service_resolver_new(AvahiIfIndex interface,
                                           AvahiProtocol protocol,
                                           const gchar * name,
                                           const gchar * type,
                                           const gchar * domain,
                                           AvahiProtocol address_protocol,
                                           GaLookupFlags flags);

gboolean
ga_service_resolver_attach(GaServiceResolver * resolver,
                           GaClient * client, GError ** error);

gboolean
ga_service_resolver_get_address(GaServiceResolver * resolver,
                                AvahiAddress * address, uint16_t * port);
G_END_DECLS
#endif /* #ifndef __GA_SERVICE_RESOLVER_H__ */
