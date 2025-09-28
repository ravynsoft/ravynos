/*
 * ga-entry-group.c - Source for GaEntryGroup
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avahi-common/malloc.h>

#include "ga-error.h"
#include "ga-entry-group.h"
#include "ga-entry-group-enumtypes.h"

G_DEFINE_TYPE(GaEntryGroup, ga_entry_group, G_TYPE_OBJECT)

static void _free_service(gpointer data);

/* signal enum */
enum {
    STATE_CHANGED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

/* properties */
enum {
    PROP_STATE = 1
};

/* private structures */
typedef struct _GaEntryGroupPrivate GaEntryGroupPrivate;

struct _GaEntryGroupPrivate {
    GaEntryGroupState state;
    GaClient *client;
    AvahiEntryGroup *group;
    GHashTable *services;
    gboolean dispose_has_run;
};

typedef struct _GaEntryGroupServicePrivate GaEntryGroupServicePrivate;

struct _GaEntryGroupServicePrivate {
    GaEntryGroupService public;
    GaEntryGroup *group;
    gboolean frozen;
    GHashTable *entries;
};

typedef struct _GaEntryGroupServiceEntry GaEntryGroupServiceEntry;

struct _GaEntryGroupServiceEntry {
    guint8 *value;
    gsize size;
};


#define GA_ENTRY_GROUP_GET_PRIVATE(o)     (G_TYPE_INSTANCE_GET_PRIVATE ((o), GA_TYPE_ENTRY_GROUP, GaEntryGroupPrivate))

static void ga_entry_group_init(GaEntryGroup * obj) {
    GaEntryGroupPrivate *priv = GA_ENTRY_GROUP_GET_PRIVATE(obj);
    /* allocate any data required by the object here */
    priv->state = GA_ENTRY_GROUP_STATE_UNCOMMITED;
    priv->client = NULL;
    priv->group = NULL;
    priv->services = g_hash_table_new_full(g_direct_hash,
                                           g_direct_equal,
                                           NULL, _free_service);
}

static void ga_entry_group_dispose(GObject * object);
static void ga_entry_group_finalize(GObject * object);

static void ga_entry_group_get_property(GObject * object,
                            guint property_id,
                            GValue * value, GParamSpec * pspec) {
    GaEntryGroup *group = GA_ENTRY_GROUP(object);
    GaEntryGroupPrivate *priv = GA_ENTRY_GROUP_GET_PRIVATE(group);

    switch (property_id) {
        case PROP_STATE:
            g_value_set_enum(value, priv->state);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

static void ga_entry_group_class_init(GaEntryGroupClass * ga_entry_group_class) {
    GObjectClass *object_class = G_OBJECT_CLASS(ga_entry_group_class);
    GParamSpec *param_spec;

    g_type_class_add_private(ga_entry_group_class,
                             sizeof (GaEntryGroupPrivate));

    object_class->dispose = ga_entry_group_dispose;
    object_class->finalize = ga_entry_group_finalize;
    object_class->get_property = ga_entry_group_get_property;

    param_spec = g_param_spec_enum("state", "Entry Group state",
                                   "The state of the avahi entry group",
                                   GA_TYPE_ENTRY_GROUP_STATE,
                                   GA_ENTRY_GROUP_STATE_UNCOMMITED,
                                   G_PARAM_READABLE |
                                   G_PARAM_STATIC_NAME |
                                   G_PARAM_STATIC_BLURB);
    g_object_class_install_property(object_class, PROP_STATE, param_spec);

    signals[STATE_CHANGED] =
            g_signal_new("state-changed",
                         G_OBJECT_CLASS_TYPE(ga_entry_group_class),
                         G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
                         0,
                         NULL, NULL,
                         g_cclosure_marshal_VOID__ENUM,
                         G_TYPE_NONE, 1, GA_TYPE_ENTRY_GROUP_STATE);
}

void ga_entry_group_dispose(GObject * object) {
    GaEntryGroup *self = GA_ENTRY_GROUP(object);
    GaEntryGroupPrivate *priv = GA_ENTRY_GROUP_GET_PRIVATE(self);

    if (priv->dispose_has_run)
        return;
    priv->dispose_has_run = TRUE;

    /* release any references held by the object here */
    if (priv->group) {
        avahi_entry_group_free(priv->group);
        priv->group = NULL;
    }

    if (priv->client) {
        g_object_unref(priv->client);
        priv->client = NULL;
    }

    if (G_OBJECT_CLASS(ga_entry_group_parent_class)->dispose)
        G_OBJECT_CLASS(ga_entry_group_parent_class)->dispose(object);
}

void ga_entry_group_finalize(GObject * object) {
    GaEntryGroup *self = GA_ENTRY_GROUP(object);
    GaEntryGroupPrivate *priv = GA_ENTRY_GROUP_GET_PRIVATE(self);

    /* free any data held directly by the object here */
    g_hash_table_destroy(priv->services);
    priv->services = NULL;

    G_OBJECT_CLASS(ga_entry_group_parent_class)->finalize(object);
}

static void _free_service(gpointer data) {
    GaEntryGroupService *s = (GaEntryGroupService *) data;
    GaEntryGroupServicePrivate *p = (GaEntryGroupServicePrivate *) s;
    g_free(s->name);
    g_free(s->type);
    g_free(s->domain);
    g_free(s->host);
    g_hash_table_destroy(p->entries);
    g_free(s);
}

static GQuark detail_for_state(AvahiEntryGroupState state) {
    static struct {
        AvahiEntryGroupState state;
        const gchar *name;
        GQuark quark;
    } states[] = {
        { AVAHI_ENTRY_GROUP_UNCOMMITED, "uncommited", 0},
        { AVAHI_ENTRY_GROUP_REGISTERING, "registering", 0},
        { AVAHI_ENTRY_GROUP_ESTABLISHED, "established", 0},
        { AVAHI_ENTRY_GROUP_COLLISION, "collistion", 0},
        { AVAHI_ENTRY_GROUP_FAILURE, "failure", 0},
        { 0, NULL, 0}
    };
    int i;

    for (i = 0; states[i].name != NULL; i++) {
        if (state != states[i].state)
            continue;

        if (!states[i].quark)
            states[i].quark = g_quark_from_static_string(states[i].name);
        return states[i].quark;
    }
    g_assert_not_reached();
}

static void _avahi_entry_group_cb(AvahiEntryGroup * g,
                      AvahiEntryGroupState state, void *data) {
    GaEntryGroup *self = GA_ENTRY_GROUP(data);
    GaEntryGroupPrivate *priv = GA_ENTRY_GROUP_GET_PRIVATE(self);

    /* Avahi can call the callback before return from _client_new */
    if (priv->group == NULL)
        priv->group = g;

    g_assert(g == priv->group);
    priv->state = state;
    g_signal_emit(self, signals[STATE_CHANGED],
                  detail_for_state(state), state);
}

GaEntryGroup *ga_entry_group_new(void) {
    return g_object_new(GA_TYPE_ENTRY_GROUP, NULL);
}

static guint _entry_hash(gconstpointer v) {
    const GaEntryGroupServiceEntry *entry =
            (const GaEntryGroupServiceEntry *) v;
    guint32 h = 0;
    guint i;

    for (i = 0; i < entry->size; i++) {
        h = (h << 5) - h + entry->value[i];
    }

    return h;
}

static gboolean _entry_equal(gconstpointer a, gconstpointer b) {
    const GaEntryGroupServiceEntry *aentry =
            (const GaEntryGroupServiceEntry *) a;
    const GaEntryGroupServiceEntry *bentry =
            (const GaEntryGroupServiceEntry *) b;

    if (aentry->size != bentry->size) {
        return FALSE;
    }

    return memcmp(aentry->value, bentry->value, aentry->size) == 0;
}

static GaEntryGroupServiceEntry *_new_entry(const guint8 * value, gsize size) {
    GaEntryGroupServiceEntry *entry;

    if (value == NULL) {
        return NULL;
    }

    entry = g_slice_new(GaEntryGroupServiceEntry);
    entry->value = g_malloc(size + 1);
    memcpy(entry->value, value, size);
    /* for string keys, make sure it's NUL-terminated too */
    entry->value[size] = 0;
    entry->size = size;

    return entry;
}

static void _set_entry(GHashTable * table, const guint8 * key, gsize ksize,
           const guint8 * value, gsize vsize) {

    g_hash_table_insert(table, _new_entry(key, ksize),
                        _new_entry(value, vsize));
}

static void _free_entry(gpointer data) {
    GaEntryGroupServiceEntry *entry = (GaEntryGroupServiceEntry *) data;

    if (entry == NULL) {
        return;
    }

    g_free(entry->value);
    g_slice_free(GaEntryGroupServiceEntry, entry);
}

static GHashTable *_string_list_to_hash(AvahiStringList * list) {
    GHashTable *ret;
    AvahiStringList *t;

    ret = g_hash_table_new_full(_entry_hash,
                                _entry_equal, _free_entry, _free_entry);

    for (t = list; t != NULL; t = avahi_string_list_get_next(t)) {
        gchar *key;
        gchar *value;
        gsize size;
        int r;

        /* list_get_pair only fails if if memory allocation fails. Normal glib
         * behaviour is to assert/abort when that happens */
        r = avahi_string_list_get_pair(t, &key, &value, &size);
        g_assert(r == 0);

        if (value == NULL) {
            _set_entry(ret, t->text, t->size, NULL, 0);
        } else {
            _set_entry(ret, (const guint8 *) key, strlen(key),
                       (const guint8 *) value, size);
        }
        avahi_free(key);
        avahi_free(value);
    }
    return ret;
}

static void _hash_to_string_list_foreach(gpointer key, gpointer value, gpointer data) {
    AvahiStringList **list = (AvahiStringList **) data;
    GaEntryGroupServiceEntry *kentry = (GaEntryGroupServiceEntry *) key;
    GaEntryGroupServiceEntry *ventry = (GaEntryGroupServiceEntry *) value;

    if (value != NULL) {
        *list = avahi_string_list_add_pair_arbitrary(*list,
                                                     (gchar *) kentry->value,
                                                     ventry->value,
                                                     ventry->size);
    } else {
        *list = avahi_string_list_add_arbitrary(*list,
                                                kentry->value, kentry->size);
    }
}

static AvahiStringList *_hash_to_string_list(GHashTable * table) {
    AvahiStringList *list = NULL;
    g_hash_table_foreach(table, _hash_to_string_list_foreach,
                         (gpointer) & list);
    return list;
}

GaEntryGroupService *ga_entry_group_add_service_strlist(GaEntryGroup * group,
                                                        const gchar * name,
                                                        const gchar * type,
                                                        guint16 port,
                                                        GError ** error,
                                                        AvahiStringList *
                                                        txt) {
    return ga_entry_group_add_service_full_strlist(group, AVAHI_IF_UNSPEC,
                                                   AVAHI_PROTO_UNSPEC, 0,
                                                   name, type, NULL, NULL,
                                                   port, error, txt);
}

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
                                                             txt) {
    GaEntryGroupPrivate *priv = GA_ENTRY_GROUP_GET_PRIVATE(group);
    GaEntryGroupServicePrivate *service = NULL;
    int ret;

    ret = avahi_entry_group_add_service_strlst(priv->group,
                                               interface, protocol,
                                               flags,
                                               name, type,
                                               domain, host, port, txt);
    if (ret) {
        if (error != NULL) {
            *error = g_error_new(GA_ERROR, ret,
                                 "Adding service to group failed: %s",
                                 avahi_strerror(ret));
        }
        goto out;
    }

    service = g_new0(GaEntryGroupServicePrivate, 1);
    service->public.interface = interface;
    service->public.protocol = protocol;
    service->public.flags = flags;
    service->public.name = g_strdup(name);
    service->public.type = g_strdup(type);
    service->public.domain = g_strdup(domain);
    service->public.host = g_strdup(host);
    service->public.port = port;
    service->group = group;
    service->frozen = FALSE;
    service->entries = _string_list_to_hash(txt);
    g_hash_table_insert(priv->services, group, service);
  out:
    return (GaEntryGroupService *) service;
}

GaEntryGroupService *ga_entry_group_add_service(GaEntryGroup * group,
                                                const gchar * name,
                                                const gchar * type,
                                                guint16 port,
                                                GError ** error, ...) {
    GaEntryGroupService *ret;
    AvahiStringList *txt = NULL;
    va_list va;
    va_start(va, error);
    txt = avahi_string_list_new_va(va);

    ret = ga_entry_group_add_service_full_strlist(group,
                                                  AVAHI_IF_UNSPEC,
                                                  AVAHI_PROTO_UNSPEC,
                                                  0,
                                                  name, type,
                                                  NULL, NULL,
                                                  port, error, txt);
    avahi_string_list_free(txt);
    va_end(va);
    return ret;
}

GaEntryGroupService *ga_entry_group_add_service_full(GaEntryGroup * group,
                                                     AvahiIfIndex interface,
                                                     AvahiProtocol protocol,
                                                     AvahiPublishFlags flags,
                                                     const gchar * name,
                                                     const gchar * type,
                                                     const gchar * domain,
                                                     const gchar * host,
                                                     guint16 port,
                                                     GError ** error, ...) {
    GaEntryGroupService *ret;
    AvahiStringList *txt = NULL;
    va_list va;

    va_start(va, error);
    txt = avahi_string_list_new_va(va);

    ret = ga_entry_group_add_service_full_strlist(group,
                                                  interface, protocol,
                                                  flags,
                                                  name, type,
                                                  domain, host,
                                                  port, error, txt);
    avahi_string_list_free(txt);
    va_end(va);
    return ret;
}

gboolean ga_entry_group_add_record(GaEntryGroup * group,
                          AvahiPublishFlags flags,
                          const gchar * name,
                          guint16 type,
                          guint32 ttl,
                          const void *rdata, gsize size, GError ** error) {
    return ga_entry_group_add_record_full(group,
                                          AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC,
                                          flags, name, AVAHI_DNS_CLASS_IN,
                                          type, ttl, rdata, size, error);
}

gboolean ga_entry_group_add_record_full(GaEntryGroup * group,
                               AvahiIfIndex interface,
                               AvahiProtocol protocol,
                               AvahiPublishFlags flags,
                               const gchar * name,
                               guint16 clazz,
                               guint16 type,
                               guint32 ttl,
                               const void *rdata,
                               gsize size, GError ** error) {
    int ret;
    GaEntryGroupPrivate *priv = GA_ENTRY_GROUP_GET_PRIVATE(group);
    g_assert(group != NULL && priv->group != NULL);

    ret = avahi_entry_group_add_record(priv->group, interface, protocol,
                                       flags, name, clazz, type, ttl, rdata,
                                       size);
    if (ret) {
        if (error != NULL) {
            *error = g_error_new(GA_ERROR, ret,
                                 "Setting raw record failed: %s",
                                 avahi_strerror(ret));
        }
        return FALSE;
    }
    return TRUE;
}


void ga_entry_group_service_freeze(GaEntryGroupService * service) {
    GaEntryGroupServicePrivate *p = (GaEntryGroupServicePrivate *) service;
    p->frozen = TRUE;
}

gboolean ga_entry_group_service_thaw(GaEntryGroupService * service, GError ** error) {
    GaEntryGroupServicePrivate *priv = (GaEntryGroupServicePrivate *) service;
    int ret;
    gboolean result = TRUE;

    AvahiStringList *txt = _hash_to_string_list(priv->entries);
    ret = avahi_entry_group_update_service_txt_strlst
            (GA_ENTRY_GROUP_GET_PRIVATE(priv->group)->group,
             service->interface, service->protocol, service->flags,
             service->name, service->type, service->domain, txt);
    if (ret) {
        if (error != NULL) {
            *error = g_error_new(GA_ERROR, ret,
                                 "Updating txt record failed: %s",
                                 avahi_strerror(ret));
        }
        result = FALSE;
    }

    avahi_string_list_free(txt);
    priv->frozen = FALSE;
    return result;
}

gboolean ga_entry_group_service_set(GaEntryGroupService * service,
                           const gchar * key, const gchar * value,
                           GError ** error) {
    return ga_entry_group_service_set_arbitrary(service, key,
                                                (const guint8 *) value,
                                                strlen(value), error);

}

gboolean ga_entry_group_service_set_arbitrary(GaEntryGroupService * service,
                                     const gchar * key, const guint8 * value,
                                     gsize size, GError ** error) {
    GaEntryGroupServicePrivate *priv = (GaEntryGroupServicePrivate *) service;

    _set_entry(priv->entries, (const guint8 *) key, strlen(key), value, size);

    if (!priv->frozen)
        return ga_entry_group_service_thaw(service, error);
    else
        return TRUE;
}

gboolean ga_entry_group_service_remove_key(GaEntryGroupService * service,
                                  const gchar * key, GError ** error) {
    GaEntryGroupServicePrivate *priv = (GaEntryGroupServicePrivate *) service;
    GaEntryGroupServiceEntry entry;

    entry.value = (void*) key;
    entry.size = strlen(key);

    g_hash_table_remove(priv->entries, &entry);

    if (!priv->frozen)
        return ga_entry_group_service_thaw(service, error);
    else
        return TRUE;
}


gboolean ga_entry_group_attach(GaEntryGroup * group,
                      GaClient * client, GError ** error) {
    GaEntryGroupPrivate *priv = GA_ENTRY_GROUP_GET_PRIVATE(group);

    g_return_val_if_fail(client->avahi_client, FALSE);
    g_assert(priv->client == NULL || priv->client == client);
    g_assert(priv->group == NULL);

    priv->client = client;
    g_object_ref(client);

    priv->group = avahi_entry_group_new(client->avahi_client,
                                        _avahi_entry_group_cb, group);
    if (priv->group == NULL) {
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

gboolean ga_entry_group_commit(GaEntryGroup * group, GError ** error) {
    GaEntryGroupPrivate *priv = GA_ENTRY_GROUP_GET_PRIVATE(group);
    int ret;
    ret = avahi_entry_group_commit(priv->group);
    if (ret) {
        if (error != NULL) {
            *error = g_error_new(GA_ERROR, ret,
                                 "Committing group failed: %s",
                                 avahi_strerror(ret));
        }
        return FALSE;
    }
    return TRUE;
}

gboolean ga_entry_group_reset(GaEntryGroup * group, GError ** error) {
    GaEntryGroupPrivate *priv = GA_ENTRY_GROUP_GET_PRIVATE(group);
    int ret;
    ret = avahi_entry_group_reset(priv->group);
    if (ret) {
        if (error != NULL) {
            *error = g_error_new(GA_ERROR, ret,
                                 "Resetting group failed: %s",
                                 avahi_strerror(ret));
        }
        return FALSE;
    }
    return TRUE;
}
