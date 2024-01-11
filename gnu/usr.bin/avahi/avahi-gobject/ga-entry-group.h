/*
 * ga-entry-group.h - Header for GaEntryGroup
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

#ifndef __GA_ENTRY_GROUP_H__
#define __GA_ENTRY_GROUP_H__

#include <glib-object.h>
#include <avahi-client/publish.h>
#include <avahi-client/client.h>

#include "ga-client.h"

G_BEGIN_DECLS

typedef enum {
    GA_ENTRY_GROUP_STATE_UNCOMMITED = AVAHI_ENTRY_GROUP_UNCOMMITED,
    GA_ENTRY_GROUP_STATE_REGISTERING = AVAHI_ENTRY_GROUP_REGISTERING,
    GA_ENTRY_GROUP_STATE_ESTABLISHED = AVAHI_ENTRY_GROUP_ESTABLISHED,
    GA_ENTRY_GROUP_STATE_COLLISTION = AVAHI_ENTRY_GROUP_COLLISION,
    GA_ENTRY_GROUP_STATE_FAILURE = AVAHI_ENTRY_GROUP_FAILURE
} GaEntryGroupState;

typedef struct _GaEntryGroupService GaEntryGroupService;
typedef struct _GaEntryGroup GaEntryGroup;
typedef struct _GaEntryGroupClass GaEntryGroupClass;

struct _GaEntryGroupService {
    AvahiIfIndex interface;
    AvahiProtocol protocol;
    AvahiPublishFlags flags;
    gchar *name;
    gchar *type;
    gchar *domain;
    gchar *host;
    guint16 port;
};

struct _GaEntryGroupClass {
    GObjectClass parent_class;
};

struct _GaEntryGroup {
    GObject parent;
};

GType ga_entry_group_get_type(void);

/* TYPE MACROS */
#define GA_TYPE_ENTRY_GROUP \
  (ga_entry_group_get_type())
#define GA_ENTRY_GROUP(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), GA_TYPE_ENTRY_GROUP, GaEntryGroup))
#define GA_ENTRY_GROUP_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass), GA_TYPE_ENTRY_GROUP, GaEntryGroupClass))
#define IS_GA_ENTRY_GROUP(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), GA_TYPE_ENTRY_GROUP))
#define IS_GA_ENTRY_GROUP_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass), GA_TYPE_ENTRY_GROUP))
#define GA_ENTRY_GROUP_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), GA_TYPE_ENTRY_GROUP, GaEntryGroupClass))

GaEntryGroup *ga_entry_group_new(void);

gboolean ga_entry_group_attach(GaEntryGroup * group,
                      GaClient * client, GError ** error);

GaEntryGroupService *ga_entry_group_add_service_strlist(GaEntryGroup * group,
                                                        const gchar * name,
                                                        const gchar * type,
                                                        guint16 port,
                                                        GError ** error,
                                                        AvahiStringList *
                                                        txt);

GaEntryGroupService *ga_entry_group_add_service_full_strlist(GaEntryGroup *
                                                             group,
                                                             AvahiIfIndex
                                                             interface,
                                                             AvahiProtocol
                                                             protocol,
                                                             AvahiPublishFlags
                                                             flags,
                                                             const gchar *
                                                             name,
                                                             const gchar *
                                                             type,
                                                             const gchar *
                                                             domain,
                                                             const gchar *
                                                             host,
                                                             guint16 port,
                                                             GError ** error,
                                                             AvahiStringList *
                                                             txt);
GaEntryGroupService *ga_entry_group_add_service(GaEntryGroup * group,
                                                const gchar * name,
                                                const gchar * type,
                                                guint16 port, GError ** error,
                                                ...);

GaEntryGroupService *ga_entry_group_add_service_full(GaEntryGroup * group,
                                                     AvahiIfIndex interface,
                                                     AvahiProtocol protocol,
                                                     AvahiPublishFlags flags,
                                                     const gchar * name,
                                                     const gchar * type,
                                                     const gchar * domain,
                                                     const gchar * host,
                                                     guint16 port,
                                                     GError ** error, ...);

/* Add raw record */
gboolean ga_entry_group_add_record(GaEntryGroup * group,
                          AvahiPublishFlags flags,
                          const gchar * name,
                          guint16 type,
                          guint32 ttl,
                          const void *rdata, gsize size, GError ** error);
gboolean ga_entry_group_add_record_full(GaEntryGroup * group,
                               AvahiIfIndex interface,
                               AvahiProtocol protocol,
                               AvahiPublishFlags flags,
                               const gchar * name,
                               guint16 clazz,
                               guint16 type,
                               guint32 ttl,
                               const void *rdata,
                               gsize size, GError ** error);



void ga_entry_group_service_freeze(GaEntryGroupService * service);

/* Set a key in the service record. If the service isn't frozen it's committed
 * immediately */
gboolean ga_entry_group_service_set(GaEntryGroupService * service,
                           const gchar * key, const gchar * value,
                           GError ** error);

gboolean ga_entry_group_service_set_arbitrary(GaEntryGroupService * service,
                                     const gchar * key, const guint8 * value,
                                     gsize size, GError ** error);

/* Remove one key from the service record */
gboolean ga_entry_group_service_remove_key(GaEntryGroupService * service,
                                  const gchar * key, GError ** error);

/* Update the txt record of the frozen service */
gboolean ga_entry_group_service_thaw(GaEntryGroupService * service, GError ** error);

/* Commit all newly added services */
gboolean ga_entry_group_commit(GaEntryGroup * group, GError ** error);

/* Invalidated all GaEntryGroupServices */
gboolean ga_entry_group_reset(GaEntryGroup * group, GError ** error);

G_END_DECLS
#endif /* #ifndef __GA_ENTRY_GROUP_H__ */
