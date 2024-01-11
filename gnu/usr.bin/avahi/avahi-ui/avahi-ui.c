/***
  This file is part of avahi.

  avahi is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  avahi is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General
  Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with avahi; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.
***/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdarg.h>
#include <net/if.h>

#include <gtk/gtk.h>

#include <avahi-glib/glib-watch.h>
#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-common/error.h>
#include <avahi-common/address.h>
#include <avahi-common/domain.h>
#include <avahi-common/i18n.h>

#include "avahi-ui.h"

#if defined(HAVE_GDBM) || defined(HAVE_DBM)
#include "../avahi-utils/stdb.h"
#endif

/* todo: i18n, HIGify */

struct _AuiServiceDialogPrivate {
    AvahiGLibPoll *glib_poll;
    AvahiClient *client;
    AvahiServiceBrowser **browsers;
    AvahiServiceResolver *resolver;
    AvahiDomainBrowser *domain_browser;

    gchar **browse_service_types;
    gchar *service_type;
    gchar *domain;
    gchar *service_name;
    AvahiProtocol address_family;

    AvahiAddress address;
    gchar *host_name;
    AvahiStringList *txt_data;
    guint16 port;

    gboolean resolve_service, resolve_service_done;
    gboolean resolve_host_name, resolve_host_name_done;

    GtkWidget *domain_label;
    GtkWidget *domain_button;
    GtkWidget *service_tree_view;
    GtkWidget *service_progress_bar;

    GtkListStore *service_list_store, *domain_list_store;
    GHashTable *service_type_names;

    guint service_pulse_timeout;
    guint domain_pulse_timeout;
    guint start_idle;

    AvahiIfIndex common_interface;
    AvahiProtocol common_protocol;

    GtkWidget *domain_dialog;
    GtkWidget *domain_entry;
    GtkWidget *domain_tree_view;
    GtkWidget *domain_progress_bar;
    GtkWidget *domain_ok_button;

    gint forward_response_id;
};

enum {
    PROP_0,
    PROP_BROWSE_SERVICE_TYPES,
    PROP_DOMAIN,
    PROP_SERVICE_TYPE,
    PROP_SERVICE_NAME,
    PROP_ADDRESS,
    PROP_PORT,
    PROP_HOST_NAME,
    PROP_TXT_DATA,
    PROP_RESOLVE_SERVICE,
    PROP_RESOLVE_HOST_NAME,
    PROP_ADDRESS_FAMILY
};

enum {
    SERVICE_COLUMN_IFACE,
    SERVICE_COLUMN_PROTO,
    SERVICE_COLUMN_TYPE,
    SERVICE_COLUMN_NAME,
    SERVICE_COLUMN_PRETTY_IFACE,
    SERVICE_COLUMN_PRETTY_TYPE,
    N_SERVICE_COLUMNS
};

enum {
    DOMAIN_COLUMN_NAME,
    DOMAIN_COLUMN_REF,
    N_DOMAIN_COLUMNS
};

static void aui_service_dialog_finalize(GObject *object);
static void aui_service_dialog_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void aui_service_dialog_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);

static int get_default_response(GtkDialog *dlg) {
    gint ret = GTK_RESPONSE_NONE;

    if (gtk_window_get_default_widget(GTK_WINDOW(dlg)))
        /* Use the response of the default widget, if possible */
        ret = gtk_dialog_get_response_for_widget(dlg, gtk_window_get_default_widget(GTK_WINDOW(dlg)));

    if (ret == GTK_RESPONSE_NONE) {
        /* Fall back to finding the first positive response */
        GList *children, *t;
        gint bad = GTK_RESPONSE_NONE;

        t = children = gtk_container_get_children(GTK_CONTAINER(gtk_dialog_get_action_area(dlg)));

        while (t) {
            GtkWidget *child = t->data;

            ret = gtk_dialog_get_response_for_widget(dlg, child);

            if (ret == GTK_RESPONSE_ACCEPT ||
                ret == GTK_RESPONSE_OK ||
                ret == GTK_RESPONSE_YES ||
                ret == GTK_RESPONSE_APPLY)
                break;

            if (ret != GTK_RESPONSE_NONE && bad == GTK_RESPONSE_NONE)
                bad = ret;

            t = t->next;
        }

        g_list_free (children);

        /* Fall back to finding the first negative response */
        if (ret == GTK_RESPONSE_NONE)
            ret = bad;
    }

    return ret;
}

G_DEFINE_TYPE(AuiServiceDialog, aui_service_dialog, GTK_TYPE_DIALOG)

static void aui_service_dialog_class_init(AuiServiceDialogClass *klass) {
    GObjectClass *object_class;

    avahi_init_i18n();

    object_class = (GObjectClass*) klass;

    object_class->finalize = aui_service_dialog_finalize;
    object_class->set_property = aui_service_dialog_set_property;
    object_class->get_property = aui_service_dialog_get_property;

    g_object_class_install_property(
            object_class,
            PROP_BROWSE_SERVICE_TYPES,
            g_param_spec_pointer("browse_service_types", _("Browse Service Types"), _("A NULL terminated list of service types to browse for"),
                                G_PARAM_READABLE | G_PARAM_WRITABLE));
    g_object_class_install_property(
            object_class,
            PROP_DOMAIN,
            g_param_spec_string("domain", _("Domain"), _("The domain to browse in, or NULL for the default domain"),
                                NULL,
                                G_PARAM_READABLE | G_PARAM_WRITABLE));
    g_object_class_install_property(
            object_class,
            PROP_SERVICE_TYPE,
            g_param_spec_string("service_type", _("Service Type"), _("The service type of the selected service"),
                                NULL,
                                G_PARAM_READABLE | G_PARAM_WRITABLE));
    g_object_class_install_property(
            object_class,
            PROP_SERVICE_NAME,
            g_param_spec_string("service_name", _("Service Name"), _("The service name of the selected service"),
                                NULL,
                                G_PARAM_READABLE | G_PARAM_WRITABLE));
    g_object_class_install_property(
            object_class,
            PROP_ADDRESS,
            g_param_spec_pointer("address", _("Address"), _("The address of the resolved service"),
                                G_PARAM_READABLE));
    g_object_class_install_property(
            object_class,
            PROP_PORT,
            g_param_spec_uint("port", _("Port"), _("The IP port number of the resolved service"),
                             0, 0xFFFF, 0,
                             G_PARAM_READABLE));
    g_object_class_install_property(
            object_class,
            PROP_HOST_NAME,
            g_param_spec_string("host_name", _("Host Name"), _("The host name of the resolved service"),
                                NULL,
                                G_PARAM_READABLE));
    g_object_class_install_property(
            object_class,
            PROP_TXT_DATA,
            g_param_spec_pointer("txt_data", _("TXT Data"), _("The TXT data of the resolved service"),
                                G_PARAM_READABLE));
    g_object_class_install_property(
            object_class,
            PROP_RESOLVE_SERVICE,
            g_param_spec_boolean("resolve_service", _("Resolve Service"), _("Resolve the selected service automatically before returning"),
                                 TRUE,
                                 G_PARAM_READABLE | G_PARAM_WRITABLE));
    g_object_class_install_property(
            object_class,
            PROP_RESOLVE_HOST_NAME,
            g_param_spec_boolean("resolve_host_name", _("Resolve Service Host Name"), _("Resolve the host name of the selected service automatically before returning"),
                                 TRUE,
                                 G_PARAM_READABLE | G_PARAM_WRITABLE));
    g_object_class_install_property(
            object_class,
            PROP_ADDRESS_FAMILY,
            g_param_spec_int("address_family", _("Address family"), _("The address family for host name resolution"),
                             AVAHI_PROTO_UNSPEC, AVAHI_PROTO_INET6, AVAHI_PROTO_UNSPEC,
                             G_PARAM_READABLE | G_PARAM_WRITABLE));
}


GtkWidget *aui_service_dialog_new_valist(
        const gchar *title,
        GtkWindow *parent,
        const gchar *first_button_text,
        va_list varargs) {

    const gchar *button_text;
    gint dr;

    GtkWidget *w = (GtkWidget*)g_object_new(
                                      AUI_TYPE_SERVICE_DIALOG,
#if !GTK_CHECK_VERSION (2,21,8)
                                      "has-separator", FALSE,
#endif
                                      "title", title,
                                      NULL);

    if (parent)
        gtk_window_set_transient_for(GTK_WINDOW(w), parent);

    button_text = first_button_text;
    while (button_text) {
        gint response_id;

        response_id = va_arg(varargs, gint);
        gtk_dialog_add_button(GTK_DIALOG(w), button_text, response_id);
        button_text = va_arg(varargs, const gchar *);
    }

    gtk_dialog_set_response_sensitive(GTK_DIALOG(w), GTK_RESPONSE_ACCEPT, FALSE);
    gtk_dialog_set_response_sensitive(GTK_DIALOG(w), GTK_RESPONSE_OK, FALSE);
    gtk_dialog_set_response_sensitive(GTK_DIALOG(w), GTK_RESPONSE_YES, FALSE);
    gtk_dialog_set_response_sensitive(GTK_DIALOG(w), GTK_RESPONSE_APPLY, FALSE);

    if ((dr = get_default_response(GTK_DIALOG(w))) != GTK_RESPONSE_NONE)
        gtk_dialog_set_default_response(GTK_DIALOG(w), dr);

    return w;
}

GtkWidget* aui_service_dialog_new(
        const gchar *title,
        GtkWindow *parent,
        const gchar *first_button_text,
        ...) {

    GtkWidget *w;

    va_list varargs;
    va_start(varargs, first_button_text);
    w = aui_service_dialog_new_valist(title, parent, first_button_text, varargs);
    va_end(varargs);

    return w;
}

static gboolean service_pulse_callback(gpointer data) {
    AuiServiceDialog *d = AUI_SERVICE_DIALOG(data);

    gtk_progress_bar_pulse(GTK_PROGRESS_BAR(d->priv->service_progress_bar));
    return TRUE;
}

static gboolean domain_pulse_callback(gpointer data) {
    AuiServiceDialog *d = AUI_SERVICE_DIALOG(data);

    gtk_progress_bar_pulse(GTK_PROGRESS_BAR(d->priv->domain_progress_bar));
    return TRUE;
}

static void client_callback(AvahiClient *c, AvahiClientState state, void *userdata) {
    AuiServiceDialog *d = AUI_SERVICE_DIALOG(userdata);

    if (state == AVAHI_CLIENT_FAILURE) {
        GtkWidget *m = gtk_message_dialog_new(GTK_WINDOW(d),
                                              GTK_DIALOG_DESTROY_WITH_PARENT,
                                              GTK_MESSAGE_ERROR,
                                              GTK_BUTTONS_CLOSE,
                                              _("Avahi client failure: %s"),
                                              avahi_strerror(avahi_client_errno(c)));
        gtk_dialog_run(GTK_DIALOG(m));
        gtk_widget_destroy(m);

        gtk_dialog_response(GTK_DIALOG(d), GTK_RESPONSE_CANCEL);
    }
}

static void resolve_callback(
        AvahiServiceResolver *r G_GNUC_UNUSED,
        AvahiIfIndex interface G_GNUC_UNUSED,
        AvahiProtocol protocol G_GNUC_UNUSED,
        AvahiResolverEvent event,
        const char *name,
        const char *type,
        const char *domain,
        const char *host_name,
        const AvahiAddress *a,
        uint16_t port,
        AvahiStringList *txt,
        AvahiLookupResultFlags flags G_GNUC_UNUSED,
        void *userdata) {

    AuiServiceDialog *d = AUI_SERVICE_DIALOG(userdata);

    switch (event) {
        case AVAHI_RESOLVER_FOUND:

            d->priv->resolve_service_done = 1;

            g_free(d->priv->service_name);
            d->priv->service_name = g_strdup(name);

            g_free(d->priv->service_type);
            d->priv->service_type = g_strdup(type);

            g_free(d->priv->domain);
            d->priv->domain = g_strdup(domain);

            g_free(d->priv->host_name);
            d->priv->host_name = g_strdup(host_name);

            d->priv->port = port;

            avahi_string_list_free(d->priv->txt_data);
            d->priv->txt_data = avahi_string_list_copy(txt);

            if (a) {
                d->priv->resolve_host_name_done = 1;
                d->priv->address = *a;
            }

            gtk_dialog_response(GTK_DIALOG(d), d->priv->forward_response_id);

            break;

        case AVAHI_RESOLVER_FAILURE: {
            GtkWidget *m = gtk_message_dialog_new(GTK_WINDOW(d),
                                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_CLOSE,
                                                  _("Avahi resolver failure: %s"),
                                                  avahi_strerror(avahi_client_errno(d->priv->client)));
            gtk_dialog_run(GTK_DIALOG(m));
            gtk_widget_destroy(m);

            gtk_dialog_response(GTK_DIALOG(d), GTK_RESPONSE_CANCEL);
            break;
        }
    }
}


static void browse_callback(
        AvahiServiceBrowser *b G_GNUC_UNUSED,
        AvahiIfIndex interface,
        AvahiProtocol protocol,
        AvahiBrowserEvent event,
        const char *name,
        const char *type,
        const char *domain,
        AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
        void* userdata) {

    AuiServiceDialog *d = AUI_SERVICE_DIALOG(userdata);

    switch (event) {

        case AVAHI_BROWSER_NEW: {
            gchar *ifs;
            const gchar *pretty_type = NULL;
            char ifname[IFNAMSIZ];
            GtkTreeIter iter;
            GtkTreeSelection *selection;

            if (!(if_indextoname(interface, ifname)))
                g_snprintf(ifname, sizeof(ifname), "%i", interface);

            ifs = g_strdup_printf("%s %s", ifname, protocol == AVAHI_PROTO_INET ? "IPv4" : "IPv6");

            if (d->priv->service_type_names)
                pretty_type = g_hash_table_lookup (d->priv->service_type_names, type);

            if (!pretty_type) {
#if defined(HAVE_GDBM) || defined(HAVE_DBM)
                pretty_type = stdb_lookup(type);
#else
                pretty_type = type;
#endif
            }

            gtk_list_store_append(d->priv->service_list_store, &iter);

            gtk_list_store_set(d->priv->service_list_store, &iter,
                               SERVICE_COLUMN_IFACE, interface,
                               SERVICE_COLUMN_PROTO, protocol,
                               SERVICE_COLUMN_NAME, name,
                               SERVICE_COLUMN_TYPE, type,
                               SERVICE_COLUMN_PRETTY_IFACE, ifs,
                               SERVICE_COLUMN_PRETTY_TYPE, pretty_type,
                               -1);

            g_free(ifs);

            if (d->priv->common_protocol == AVAHI_PROTO_UNSPEC)
                d->priv->common_protocol = protocol;

            if (d->priv->common_interface == AVAHI_IF_UNSPEC)
                d->priv->common_interface = interface;

            if (d->priv->common_interface != interface || d->priv->common_protocol != protocol) {
                gtk_tree_view_column_set_visible(gtk_tree_view_get_column(GTK_TREE_VIEW(d->priv->service_tree_view), 0), TRUE);
                gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(d->priv->service_tree_view), TRUE);
            }

            selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(d->priv->service_tree_view));
            if (!gtk_tree_selection_get_selected(selection, NULL, NULL)) {

                if (!d->priv->service_type ||
                    !d->priv->service_name ||
                    (avahi_domain_equal(d->priv->service_type, type) && strcasecmp(d->priv->service_name, name) == 0)) {
                    GtkTreePath *path;

                    gtk_tree_selection_select_iter(selection, &iter);

                    path = gtk_tree_model_get_path(GTK_TREE_MODEL(d->priv->service_list_store), &iter);
                    gtk_tree_view_set_cursor(GTK_TREE_VIEW(d->priv->service_tree_view), path, NULL, FALSE);
                    gtk_tree_path_free(path);
                }

            }

            break;
        }

        case AVAHI_BROWSER_REMOVE: {
            GtkTreeIter iter;
            gboolean valid;

            valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(d->priv->service_list_store), &iter);
            while (valid) {
                gint _interface, _protocol;
                gchar *_name, *_type;
                gboolean found;

                gtk_tree_model_get(GTK_TREE_MODEL(d->priv->service_list_store), &iter,
                                   SERVICE_COLUMN_IFACE, &_interface,
                                   SERVICE_COLUMN_PROTO, &_protocol,
                                   SERVICE_COLUMN_NAME, &_name,
                                   SERVICE_COLUMN_TYPE, &_type,
                                   -1);

                found = _interface == interface && _protocol == protocol && strcasecmp(_name, name) == 0 && avahi_domain_equal(_type, type);
                g_free(_name);

                if (found) {
                    gtk_list_store_remove(d->priv->service_list_store, &iter);
                    break;
                }

                valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(d->priv->service_list_store), &iter);
            }

            break;
        }

        case AVAHI_BROWSER_FAILURE: {
            GtkWidget *m = gtk_message_dialog_new(GTK_WINDOW(d),
                                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_CLOSE,
                                                  _("Browsing for service type %s in domain %s failed: %s"),
                                                  type, domain ? domain : _("n/a"),
                                                  avahi_strerror(avahi_client_errno(d->priv->client)));
            gtk_dialog_run(GTK_DIALOG(m));
            gtk_widget_destroy(m);

            /* Fall through */
        }

        case AVAHI_BROWSER_ALL_FOR_NOW:
            if (d->priv->service_pulse_timeout > 0) {
                g_source_remove(d->priv->service_pulse_timeout);
                d->priv->service_pulse_timeout = 0;
                gtk_widget_hide(d->priv->service_progress_bar);
            }
            break;

        case AVAHI_BROWSER_CACHE_EXHAUSTED:
            ;
    }
}

static void domain_make_default_selection(AuiServiceDialog *d, const gchar *name, GtkTreeIter *iter) {
    GtkTreeSelection *selection;

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(d->priv->domain_tree_view));
    if (!gtk_tree_selection_get_selected(selection, NULL, NULL)) {

        if (avahi_domain_equal(gtk_entry_get_text(GTK_ENTRY(d->priv->domain_entry)), name)) {
            GtkTreePath *path;

            gtk_tree_selection_select_iter(selection, iter);

            path = gtk_tree_model_get_path(GTK_TREE_MODEL(d->priv->domain_list_store), iter);
            gtk_tree_view_set_cursor(GTK_TREE_VIEW(d->priv->domain_tree_view), path, NULL, FALSE);
            gtk_tree_path_free(path);
        }

    }
}

static void domain_browse_callback(
        AvahiDomainBrowser *b G_GNUC_UNUSED,
        AvahiIfIndex interface G_GNUC_UNUSED,
        AvahiProtocol protocol G_GNUC_UNUSED,
        AvahiBrowserEvent event,
        const char *name,
        AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
        void* userdata) {

    AuiServiceDialog *d = AUI_SERVICE_DIALOG(userdata);

    switch (event) {

        case AVAHI_BROWSER_NEW: {
            GtkTreeIter iter;
            gboolean found = FALSE, valid;
            gint ref;

            valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(d->priv->domain_list_store), &iter);
            while (valid) {
                gchar *_name;

                gtk_tree_model_get(GTK_TREE_MODEL(d->priv->domain_list_store), &iter,
                                   DOMAIN_COLUMN_NAME, &_name,
                                   DOMAIN_COLUMN_REF, &ref,
                                   -1);

                found = avahi_domain_equal(_name, name);
                g_free(_name);

                if (found)
                    break;

                valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(d->priv->domain_list_store), &iter);
            }

            if (found)
                gtk_list_store_set(d->priv->domain_list_store, &iter, DOMAIN_COLUMN_REF, ref + 1, -1);
            else {
                gtk_list_store_append(d->priv->domain_list_store, &iter);

                gtk_list_store_set(d->priv->domain_list_store, &iter,
                                   DOMAIN_COLUMN_NAME, name,
                                   DOMAIN_COLUMN_REF, 1,
                                   -1);
            }

            domain_make_default_selection(d, name, &iter);

            break;
        }

        case AVAHI_BROWSER_REMOVE: {
            gboolean valid;
            GtkTreeIter iter;

            valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(d->priv->domain_list_store), &iter);
            while (valid) {
                gint ref;
                gchar *_name;
                gboolean found;

                gtk_tree_model_get(GTK_TREE_MODEL(d->priv->domain_list_store), &iter,
                                   DOMAIN_COLUMN_NAME, &_name,
                                   DOMAIN_COLUMN_REF, &ref,
                                   -1);

                found = avahi_domain_equal(_name, name);
                g_free(_name);

                if (found) {
                    if (ref <= 1)
                        gtk_list_store_remove(d->priv->service_list_store, &iter);
                    else
                        gtk_list_store_set(d->priv->domain_list_store, &iter, DOMAIN_COLUMN_REF, ref - 1, -1);
                    break;
                }

                valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(d->priv->domain_list_store), &iter);
            }

            break;
        }


        case AVAHI_BROWSER_FAILURE: {
            GtkWidget *m = gtk_message_dialog_new(GTK_WINDOW(d),
                                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_CLOSE,
                                                  _("Avahi domain browser failure: %s"),
                                                  avahi_strerror(avahi_client_errno(d->priv->client)));
            gtk_dialog_run(GTK_DIALOG(m));
            gtk_widget_destroy(m);

            /* Fall through */
        }

        case AVAHI_BROWSER_ALL_FOR_NOW:
            if (d->priv->domain_pulse_timeout > 0) {
                g_source_remove(d->priv->domain_pulse_timeout);
                d->priv->domain_pulse_timeout = 0;
                gtk_widget_hide(d->priv->domain_progress_bar);
            }
            break;

        case AVAHI_BROWSER_CACHE_EXHAUSTED:
            ;
    }
}

static const gchar *get_domain_name(AuiServiceDialog *d) {
    const gchar *domain;

    g_return_val_if_fail(d, NULL);
    g_return_val_if_fail(AUI_IS_SERVICE_DIALOG(d), NULL);

    if (d->priv->domain)
        return d->priv->domain;

    if (!(domain = avahi_client_get_domain_name(d->priv->client))) {
        GtkWidget *m = gtk_message_dialog_new(GTK_WINDOW(d),
                                              GTK_DIALOG_DESTROY_WITH_PARENT,
                                              GTK_MESSAGE_ERROR,
                                              GTK_BUTTONS_CLOSE,
                                              _("Failed to read Avahi domain: %s"),
                                              avahi_strerror(avahi_client_errno(d->priv->client)));
        gtk_dialog_run(GTK_DIALOG(m));
        gtk_widget_destroy(m);

        return NULL;
    }

    return domain;
}

static gboolean start_callback(gpointer data) {
    int error;
    AuiServiceDialog *d = AUI_SERVICE_DIALOG(data);
    gchar **st;
    AvahiServiceBrowser **sb;
    unsigned i;
    const char *domain;

    d->priv->start_idle = 0;

    if (!d->priv->browse_service_types || !*d->priv->browse_service_types) {
        g_warning(_("Browse service type list is empty!"));
        return FALSE;
    }

    if (!d->priv->client) {
        if (!(d->priv->client = avahi_client_new(avahi_glib_poll_get(d->priv->glib_poll), 0, client_callback, d, &error))) {

            GtkWidget *m = gtk_message_dialog_new(GTK_WINDOW(d),
                                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_CLOSE,
                                                  _("Failed to connect to Avahi server: %s"),
                                                  avahi_strerror(error));
            gtk_dialog_run(GTK_DIALOG(m));
            gtk_widget_destroy(m);

            gtk_dialog_response(GTK_DIALOG(d), GTK_RESPONSE_CANCEL);
            return FALSE;
        }
    }

    if (!(domain = get_domain_name(d))) {
        gtk_dialog_response(GTK_DIALOG(d), GTK_RESPONSE_CANCEL);
        return FALSE;
    }

    g_assert(domain);

    if (avahi_domain_equal(domain, "local."))
        gtk_label_set_markup(GTK_LABEL(d->priv->domain_label), _("Browsing for services on <b>local network</b>:"));
    else {
        gchar *t = g_strdup_printf(_("Browsing for services in domain <b>%s</b>:"), domain);
        gtk_label_set_markup(GTK_LABEL(d->priv->domain_label), t);
        g_free(t);
    }

    if (d->priv->browsers) {
        for (sb = d->priv->browsers; *sb; sb++)
            avahi_service_browser_free(*sb);

        g_free(d->priv->browsers);
        d->priv->browsers = NULL;
    }

    gtk_list_store_clear(GTK_LIST_STORE(d->priv->service_list_store));
    d->priv->common_interface = AVAHI_IF_UNSPEC;
    d->priv->common_protocol = AVAHI_PROTO_UNSPEC;

    gtk_tree_view_column_set_visible(gtk_tree_view_get_column(GTK_TREE_VIEW(d->priv->service_tree_view), 0), FALSE);
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(d->priv->service_tree_view), FALSE);
    gtk_widget_show(d->priv->service_progress_bar);

    if (d->priv->service_pulse_timeout <= 0)
        d->priv->service_pulse_timeout = g_timeout_add(100, service_pulse_callback, d);

    for (i = 0; d->priv->browse_service_types[i]; i++)
        ;
    g_assert(i > 0);

    d->priv->browsers = g_new0(AvahiServiceBrowser*, i+1);
    for (st = d->priv->browse_service_types, sb = d->priv->browsers; *st; st++, sb++) {

        if (!(*sb = avahi_service_browser_new(d->priv->client, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, *st, d->priv->domain, 0, browse_callback, d))) {
            GtkWidget *m = gtk_message_dialog_new(GTK_WINDOW(d),
                                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_CLOSE,
                                                  _("Failed to create browser for %s: %s"),
                                                  *st,
                                                  avahi_strerror(avahi_client_errno(d->priv->client)));
            gtk_dialog_run(GTK_DIALOG(m));
            gtk_widget_destroy(m);

            gtk_dialog_response(GTK_DIALOG(d), GTK_RESPONSE_CANCEL);
            return FALSE;

        }
    }

    return FALSE;
}

static void aui_service_dialog_finalize(GObject *object) {
    AuiServiceDialog *d = AUI_SERVICE_DIALOG(object);

    if (d->priv->domain_pulse_timeout > 0)
        g_source_remove(d->priv->domain_pulse_timeout);

    if (d->priv->service_pulse_timeout > 0)
        g_source_remove(d->priv->service_pulse_timeout);

    if (d->priv->start_idle > 0)
        g_source_remove(d->priv->start_idle);

    g_free(d->priv->host_name);
    g_free(d->priv->domain);
    g_free(d->priv->service_name);

    avahi_string_list_free(d->priv->txt_data);

    g_strfreev(d->priv->browse_service_types);

    if (d->priv->domain_browser)
        avahi_domain_browser_free(d->priv->domain_browser);

    if (d->priv->resolver)
        avahi_service_resolver_free(d->priv->resolver);

    if (d->priv->browsers) {
        AvahiServiceBrowser **sb;

        for (sb = d->priv->browsers; *sb; sb++)
            avahi_service_browser_free(*sb);

        g_free(d->priv->browsers);
    }

    if (d->priv->client)
        avahi_client_free(d->priv->client);

    if (d->priv->glib_poll)
        avahi_glib_poll_free(d->priv->glib_poll);

    if (d->priv->service_list_store)
        g_object_unref(d->priv->service_list_store);
    if (d->priv->domain_list_store)
        g_object_unref(d->priv->domain_list_store);
    if (d->priv->service_type_names)
        g_hash_table_unref (d->priv->service_type_names);

    g_free(d->priv);
    d->priv = NULL;

    G_OBJECT_CLASS(aui_service_dialog_parent_class)->finalize(object);
}

static void service_row_activated_callback(GtkTreeView *tree_view G_GNUC_UNUSED, GtkTreePath *path G_GNUC_UNUSED, GtkTreeViewColumn *column G_GNUC_UNUSED, gpointer user_data) {
    AuiServiceDialog *d = AUI_SERVICE_DIALOG(user_data);

    gtk_dialog_response(GTK_DIALOG(d), get_default_response(GTK_DIALOG(d)));
}

static void service_selection_changed_callback(GtkTreeSelection *selection, gpointer user_data) {
    AuiServiceDialog *d = AUI_SERVICE_DIALOG(user_data);
    gboolean b;

    b = gtk_tree_selection_get_selected(selection, NULL, NULL);
    gtk_dialog_set_response_sensitive(GTK_DIALOG(d), GTK_RESPONSE_ACCEPT, b);
    gtk_dialog_set_response_sensitive(GTK_DIALOG(d), GTK_RESPONSE_OK, b);
    gtk_dialog_set_response_sensitive(GTK_DIALOG(d), GTK_RESPONSE_YES, b);
    gtk_dialog_set_response_sensitive(GTK_DIALOG(d), GTK_RESPONSE_APPLY, b);
}

static void response_callback(GtkDialog *dialog, gint response, gpointer user_data) {
    AuiServiceDialog *d = AUI_SERVICE_DIALOG(user_data);

    if ((response == GTK_RESPONSE_ACCEPT ||
        response == GTK_RESPONSE_OK ||
        response == GTK_RESPONSE_YES ||
        response == GTK_RESPONSE_APPLY) &&
        ((d->priv->resolve_service && !d->priv->resolve_service_done) ||
         (d->priv->resolve_host_name && !d->priv->resolve_host_name_done))) {

        GtkTreeIter iter;
        gint interface, protocol;
        gchar *name, *type;
        GdkCursor *cursor;

        g_signal_stop_emission(dialog, g_signal_lookup("response", gtk_dialog_get_type()), 0);
        d->priv->forward_response_id = response;

        if (d->priv->resolver)
            return;

        g_return_if_fail(gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(d->priv->service_tree_view)), NULL, &iter));

        gtk_tree_model_get(GTK_TREE_MODEL(d->priv->service_list_store), &iter,
                           SERVICE_COLUMN_IFACE, &interface,
                           SERVICE_COLUMN_PROTO, &protocol,
                           SERVICE_COLUMN_NAME, &name,
                           SERVICE_COLUMN_TYPE, &type, -1);

        g_return_if_fail(d->priv->client);

        gtk_widget_set_sensitive(GTK_WIDGET(dialog), FALSE);
        cursor = gdk_cursor_new(GDK_WATCH);
        gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(dialog)), cursor);
#if GTK_CHECK_VERSION(3,0,0)
        g_object_unref(cursor);
#else
        gdk_cursor_unref(cursor);
#endif

        if (!(d->priv->resolver = avahi_service_resolver_new(
                      d->priv->client, interface, protocol, name, type, d->priv->domain,
                      d->priv->address_family, !d->priv->resolve_host_name ? AVAHI_LOOKUP_NO_ADDRESS : 0, resolve_callback, d))) {

            GtkWidget *m = gtk_message_dialog_new(GTK_WINDOW(d),
                                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_CLOSE,
                                                  _("Failed to create resolver for %s of type %s in domain %s: %s"),
                                                  name, type, d->priv->domain,
                                                  avahi_strerror(avahi_client_errno(d->priv->client)));
            gtk_dialog_run(GTK_DIALOG(m));
            gtk_widget_destroy(m);

            gtk_dialog_response(GTK_DIALOG(d), GTK_RESPONSE_CANCEL);
            return;
        }
    }
}

static gboolean is_valid_domain_suffix(const gchar *n) {
    gchar label[AVAHI_LABEL_MAX];

    if (!avahi_is_valid_domain_name(n))
        return FALSE;

    if (!avahi_unescape_label(&n, label, sizeof(label)))
        return FALSE;

    /* At least one label */

    return !!label[0];
}

static void domain_row_activated_callback(GtkTreeView *tree_view G_GNUC_UNUSED, GtkTreePath *path G_GNUC_UNUSED, GtkTreeViewColumn *column G_GNUC_UNUSED, gpointer user_data) {
    AuiServiceDialog *d = AUI_SERVICE_DIALOG(user_data);

    if (is_valid_domain_suffix(gtk_entry_get_text(GTK_ENTRY(d->priv->domain_entry))))
        gtk_dialog_response(GTK_DIALOG(d->priv->domain_dialog), GTK_RESPONSE_ACCEPT);
}

static void domain_selection_changed_callback(GtkTreeSelection *selection G_GNUC_UNUSED, gpointer user_data) {
    GtkTreeIter iter;
    AuiServiceDialog *d = AUI_SERVICE_DIALOG(user_data);
    gchar *name;

    g_return_if_fail(gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(d->priv->domain_tree_view)), NULL, &iter));

    gtk_tree_model_get(GTK_TREE_MODEL(d->priv->domain_list_store), &iter,
                       DOMAIN_COLUMN_NAME, &name, -1);

    gtk_entry_set_text(GTK_ENTRY(d->priv->domain_entry), name);
}

static void domain_entry_changed_callback(GtkEditable *editable G_GNUC_UNUSED, gpointer user_data) {
    AuiServiceDialog *d = AUI_SERVICE_DIALOG(user_data);

    gtk_widget_set_sensitive(d->priv->domain_ok_button, is_valid_domain_suffix(gtk_entry_get_text(GTK_ENTRY(d->priv->domain_entry))));
}

static void domain_button_clicked(GtkButton *button G_GNUC_UNUSED, gpointer user_data) {
    GtkWidget *vbox, *vbox2, *scrolled_window;
    GtkTreeSelection *selection;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    AuiServiceDialog *d = AUI_SERVICE_DIALOG(user_data);
    AuiServiceDialogPrivate *p = d->priv;
    const gchar *domain;
    GtkTreeIter iter;

    g_return_if_fail(!p->domain_dialog);
    g_return_if_fail(!p->domain_browser);

    if (!(domain = get_domain_name(d))) {
        gtk_dialog_response(GTK_DIALOG(d), GTK_RESPONSE_CANCEL);
        return;
    }

    if (!(p->domain_browser = avahi_domain_browser_new(p->client, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, NULL, AVAHI_DOMAIN_BROWSER_BROWSE, 0, domain_browse_callback, d))) {
        GtkWidget *m = gtk_message_dialog_new(GTK_WINDOW(d),
                                              GTK_DIALOG_DESTROY_WITH_PARENT,
                                              GTK_MESSAGE_ERROR,
                                              GTK_BUTTONS_CLOSE,
                                              _("Failed to create domain browser: %s"),
                                              avahi_strerror(avahi_client_errno(p->client)));
        gtk_dialog_run(GTK_DIALOG(m));
        gtk_widget_destroy(m);

        gtk_dialog_response(GTK_DIALOG(d), GTK_RESPONSE_CANCEL);
        return;
    }

    p->domain_dialog = gtk_dialog_new();
    gtk_container_set_border_width(GTK_CONTAINER(p->domain_dialog), 5);
    gtk_window_set_title(GTK_WINDOW(p->domain_dialog), _("Change domain"));
#if !GTK_CHECK_VERSION(2,21,8)
    gtk_dialog_set_has_separator(GTK_DIALOG(p->domain_dialog), FALSE);
#endif

#if GTK_CHECK_VERSION(3,0,0)
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
#else
    vbox = gtk_vbox_new(FALSE, 8);
#endif
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 8);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(p->domain_dialog))), vbox, TRUE, TRUE, 0);

    p->domain_entry = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(p->domain_entry), AVAHI_DOMAIN_NAME_MAX);
    gtk_entry_set_text(GTK_ENTRY(p->domain_entry), domain);
    gtk_entry_set_activates_default(GTK_ENTRY(p->domain_entry), TRUE);
    g_signal_connect(p->domain_entry, "changed", G_CALLBACK(domain_entry_changed_callback), d);
    gtk_box_pack_start(GTK_BOX(vbox), p->domain_entry, FALSE, FALSE, 0);

#if GTK_CHECK_VERSION(3,0,0)
    vbox2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
#else
    vbox2 = gtk_vbox_new(FALSE, 8);
#endif
    gtk_box_pack_start(GTK_BOX(vbox), vbox2, TRUE, TRUE, 0);

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW (scrolled_window), GTK_SHADOW_ETCHED_IN);
    gtk_box_pack_start(GTK_BOX(vbox2), scrolled_window, TRUE, TRUE, 0);

    p->domain_list_store = gtk_list_store_new(N_DOMAIN_COLUMNS, G_TYPE_STRING, G_TYPE_INT);

    p->domain_tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(p->domain_list_store));
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(p->domain_tree_view), FALSE);
    g_signal_connect(p->domain_tree_view, "row-activated", G_CALLBACK(domain_row_activated_callback), d);
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(p->domain_tree_view));
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_BROWSE);
    g_signal_connect(selection, "changed", G_CALLBACK(domain_selection_changed_callback), d);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(_("Service Name"), renderer, "text", DOMAIN_COLUMN_NAME, NULL);
    gtk_tree_view_column_set_expand(column, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(p->domain_tree_view), column);

    gtk_tree_view_set_search_column(GTK_TREE_VIEW(p->domain_tree_view), DOMAIN_COLUMN_NAME);
    gtk_container_add(GTK_CONTAINER(scrolled_window), p->domain_tree_view);

    p->domain_progress_bar = gtk_progress_bar_new();
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(p->domain_progress_bar), _("Browsing..."));
    gtk_progress_bar_set_pulse_step(GTK_PROGRESS_BAR(p->domain_progress_bar), 0.1);
    gtk_box_pack_end(GTK_BOX(vbox2), p->domain_progress_bar, FALSE, FALSE, 0);

    gtk_dialog_add_button(GTK_DIALOG(p->domain_dialog), _("_Cancel"), GTK_RESPONSE_CANCEL);
    p->domain_ok_button = GTK_WIDGET(gtk_dialog_add_button(GTK_DIALOG(p->domain_dialog), _("_OK"), GTK_RESPONSE_ACCEPT));
    gtk_dialog_set_default_response(GTK_DIALOG(p->domain_dialog), GTK_RESPONSE_ACCEPT);
    gtk_widget_set_sensitive(p->domain_ok_button, is_valid_domain_suffix(gtk_entry_get_text(GTK_ENTRY(p->domain_entry))));

    gtk_widget_grab_default(p->domain_ok_button);
    gtk_widget_grab_focus(p->domain_entry);

    gtk_window_set_default_size(GTK_WINDOW(p->domain_dialog), 300, 300);

    gtk_widget_show_all(vbox);

    gtk_list_store_append(p->domain_list_store, &iter);
    gtk_list_store_set(p->domain_list_store, &iter, DOMAIN_COLUMN_NAME, "local", DOMAIN_COLUMN_REF, 1, -1);
    domain_make_default_selection(d, "local", &iter);

    p->domain_pulse_timeout = g_timeout_add(100, domain_pulse_callback, d);

    if (gtk_dialog_run(GTK_DIALOG(p->domain_dialog)) == GTK_RESPONSE_ACCEPT)
        aui_service_dialog_set_domain(d, gtk_entry_get_text(GTK_ENTRY(p->domain_entry)));

    gtk_widget_destroy(p->domain_dialog);
    p->domain_dialog = NULL;

    if (p->domain_pulse_timeout > 0) {
        g_source_remove(p->domain_pulse_timeout);
        p->domain_pulse_timeout = 0;
    }

    avahi_domain_browser_free(p->domain_browser);
    p->domain_browser = NULL;
}

static void aui_service_dialog_init(AuiServiceDialog *d) {
    GtkWidget *vbox, *vbox2, *scrolled_window;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkTreeSelection *selection;
    AuiServiceDialogPrivate *p;

    p = d->priv = g_new(AuiServiceDialogPrivate, 1);

    p->host_name = NULL;
    p->domain = NULL;
    p->service_name = NULL;
    p->service_type = NULL;
    p->txt_data = NULL;
    p->browse_service_types = NULL;
    memset(&p->address, 0, sizeof(p->address));
    p->port = 0;
    p->resolve_host_name = p->resolve_service = TRUE;
    p->resolve_host_name_done = p->resolve_service_done = FALSE;
    p->address_family = AVAHI_PROTO_UNSPEC;

    p->glib_poll = NULL;
    p->client = NULL;
    p->browsers = NULL;
    p->resolver = NULL;
    p->domain_browser = NULL;

    p->service_pulse_timeout = 0;
    p->domain_pulse_timeout = 0;
    p->start_idle = 0;
    p->common_interface = AVAHI_IF_UNSPEC;
    p->common_protocol = AVAHI_PROTO_UNSPEC;

    p->domain_dialog = NULL;
    p->domain_entry = NULL;
    p->domain_tree_view = NULL;
    p->domain_progress_bar = NULL;
    p->domain_ok_button = NULL;

    p->forward_response_id = GTK_RESPONSE_NONE;

    p->service_list_store = p->domain_list_store = NULL;
    p->service_type_names = NULL;

    gtk_container_set_border_width(GTK_CONTAINER(d), 5);

#if GTK_CHECK_VERSION(3,0,0)
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
#else
    vbox = gtk_vbox_new(FALSE, 8);
#endif
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 8);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(d))), vbox, TRUE, TRUE, 0);

    p->domain_label = gtk_label_new(_("Initializing..."));
    gtk_label_set_ellipsize(GTK_LABEL(p->domain_label), TRUE);
    gtk_misc_set_alignment(GTK_MISC(p->domain_label), 0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), p->domain_label, FALSE, FALSE, 0);


#if GTK_CHECK_VERSION(3,0,0)
    vbox2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
#else
    vbox2 = gtk_vbox_new(FALSE, 8);
#endif
    gtk_box_pack_start(GTK_BOX(vbox), vbox2, TRUE, TRUE, 0);

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW (scrolled_window), GTK_SHADOW_ETCHED_IN);
    gtk_box_pack_start(GTK_BOX(vbox2), scrolled_window, TRUE, TRUE, 0);

    p->service_list_store = gtk_list_store_new(N_SERVICE_COLUMNS, G_TYPE_INT, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

    p->service_tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(p->service_list_store));
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(p->service_tree_view), FALSE);
    g_signal_connect(p->service_tree_view, "row-activated", G_CALLBACK(service_row_activated_callback), d);
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(p->service_tree_view));
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_BROWSE);
    g_signal_connect(selection, "changed", G_CALLBACK(service_selection_changed_callback), d);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(_("Location"), renderer, "text", SERVICE_COLUMN_PRETTY_IFACE, NULL);
    gtk_tree_view_column_set_visible(column, FALSE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(p->service_tree_view), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(_("Name"), renderer, "text", SERVICE_COLUMN_NAME, NULL);
    gtk_tree_view_column_set_expand(column, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(p->service_tree_view), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(_("Type"), renderer, "text", SERVICE_COLUMN_PRETTY_TYPE, NULL);
    gtk_tree_view_column_set_visible(column, FALSE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(p->service_tree_view), column);

    gtk_tree_view_set_search_column(GTK_TREE_VIEW(p->service_tree_view), SERVICE_COLUMN_NAME);
    gtk_container_add(GTK_CONTAINER(scrolled_window), p->service_tree_view);

    p->service_progress_bar = gtk_progress_bar_new();
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(p->service_progress_bar), _("Browsing..."));
    gtk_progress_bar_set_pulse_step(GTK_PROGRESS_BAR(p->service_progress_bar), 0.1);
    gtk_box_pack_end(GTK_BOX(vbox2), p->service_progress_bar, FALSE, FALSE, 0);

    p->domain_button = gtk_button_new_with_mnemonic(_("_Domain..."));
    gtk_button_set_image(GTK_BUTTON(p->domain_button), gtk_image_new_from_icon_name("network-workgroup", GTK_ICON_SIZE_BUTTON));
    g_signal_connect(p->domain_button, "clicked", G_CALLBACK(domain_button_clicked), d);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_action_area(GTK_DIALOG(d))), p->domain_button, FALSE, TRUE, 0);
    gtk_button_box_set_child_secondary(GTK_BUTTON_BOX(gtk_dialog_get_action_area(GTK_DIALOG(d))), p->domain_button, TRUE);
    gtk_widget_show(p->domain_button);

    gtk_dialog_set_default_response(GTK_DIALOG(d), GTK_RESPONSE_ACCEPT);

    gtk_widget_grab_focus(p->service_tree_view);

    gtk_window_set_default_size(GTK_WINDOW(d), 400, 300);

    gtk_widget_show_all(vbox);

    p->glib_poll = avahi_glib_poll_new(NULL, G_PRIORITY_DEFAULT);

    p->service_pulse_timeout = g_timeout_add(100, service_pulse_callback, d);
    p->start_idle = g_idle_add(start_callback, d);

    g_signal_connect(d, "response", G_CALLBACK(response_callback), d);
}

static void restart_browsing(AuiServiceDialog *d) {
    g_return_if_fail(AUI_IS_SERVICE_DIALOG(d));

    if (d->priv->start_idle <= 0)
        d->priv->start_idle = g_idle_add(start_callback, d);
}

void aui_service_dialog_set_browse_service_types(AuiServiceDialog *d, const char *type, ...) {
    va_list ap;
    const char *t;
    unsigned u;

    g_return_if_fail(AUI_IS_SERVICE_DIALOG(d));
    g_return_if_fail(type);

    g_strfreev(d->priv->browse_service_types);

    va_start(ap, type);
    for (u = 1; va_arg(ap, const char *); u++)
        ;
    va_end(ap);

    d->priv->browse_service_types = g_new0(gchar*, u+1);
    d->priv->browse_service_types[0] = g_strdup(type);

    va_start(ap, type);
    for (u = 1; (t = va_arg(ap, const char*)); u++)
        d->priv->browse_service_types[u] = g_strdup(t);
    va_end(ap);

    if (d->priv->browse_service_types[0] && d->priv->browse_service_types[1]) {
        /* Multiple service types, show type-column */
        gtk_tree_view_column_set_visible(gtk_tree_view_get_column(GTK_TREE_VIEW(d->priv->service_tree_view), 2), TRUE);
    }

    restart_browsing(d);
}

void aui_service_dialog_set_browse_service_typesv(AuiServiceDialog *d, const char *const*types) {

    g_return_if_fail(AUI_IS_SERVICE_DIALOG(d));
    g_return_if_fail(types);
    g_return_if_fail(*types);

    g_strfreev(d->priv->browse_service_types);
    d->priv->browse_service_types = g_strdupv((char**) types);

    if (d->priv->browse_service_types[0] && d->priv->browse_service_types[1]) {
        /* Multiple service types, show type-column */
        gtk_tree_view_column_set_visible(gtk_tree_view_get_column(GTK_TREE_VIEW(d->priv->service_tree_view), 2), TRUE);
    }

    restart_browsing(d);
}

const gchar*const* aui_service_dialog_get_browse_service_types(AuiServiceDialog *d) {
    g_return_val_if_fail(AUI_IS_SERVICE_DIALOG(d), NULL);

    return (const char* const*) d->priv->browse_service_types;
}

void aui_service_dialog_set_service_type_name(AuiServiceDialog *d, const gchar *type, const gchar *name) {
    GtkTreeModel *m = NULL;
    GtkTreeIter iter;

    g_return_if_fail(AUI_IS_SERVICE_DIALOG(d));
    g_return_if_fail(NULL != type);
    g_return_if_fail(NULL != name);

    if (NULL == d->priv->service_type_names)
        d->priv->service_type_names = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

    g_hash_table_insert(d->priv->service_type_names, g_strdup(type), g_strdup(name));

    if (d->priv->service_list_store)
        m = GTK_TREE_MODEL(d->priv->service_list_store);

    if (m && gtk_tree_model_get_iter_first(m, &iter)) {
        do {
            char *stored_type = NULL;

            gtk_tree_model_get(m, &iter, SERVICE_COLUMN_TYPE, &stored_type, -1);

            if (stored_type && g_str_equal(stored_type, type))
                gtk_list_store_set(d->priv->service_list_store, &iter, SERVICE_COLUMN_PRETTY_TYPE, name, -1);
        } while (gtk_tree_model_iter_next(m, &iter));
    }
}

void aui_service_dialog_set_domain(AuiServiceDialog *d, const char *domain) {
    g_return_if_fail(AUI_IS_SERVICE_DIALOG(d));
    g_return_if_fail(!domain || is_valid_domain_suffix(domain));

    g_free(d->priv->domain);
    d->priv->domain = domain ? avahi_normalize_name_strdup(domain) : NULL;

    restart_browsing(d);
}

const char* aui_service_dialog_get_domain(AuiServiceDialog *d) {
    g_return_val_if_fail(AUI_IS_SERVICE_DIALOG(d), NULL);

    return d->priv->domain;
}

void aui_service_dialog_set_service_name(AuiServiceDialog *d, const char *name) {
    g_return_if_fail(AUI_IS_SERVICE_DIALOG(d));

    g_free(d->priv->service_name);
    d->priv->service_name = g_strdup(name);
}

const char* aui_service_dialog_get_service_name(AuiServiceDialog *d) {
    g_return_val_if_fail(AUI_IS_SERVICE_DIALOG(d), NULL);

    return d->priv->service_name;
}

void aui_service_dialog_set_service_type(AuiServiceDialog *d, const char*stype) {
    g_return_if_fail(AUI_IS_SERVICE_DIALOG(d));

    g_free(d->priv->service_type);
    d->priv->service_type = g_strdup(stype);
}

const char* aui_service_dialog_get_service_type(AuiServiceDialog *d) {
    g_return_val_if_fail(AUI_IS_SERVICE_DIALOG(d), NULL);

    return d->priv->service_type;
}

const AvahiAddress* aui_service_dialog_get_address(AuiServiceDialog *d) {
    g_return_val_if_fail(AUI_IS_SERVICE_DIALOG(d), NULL);
    g_return_val_if_fail(d->priv->resolve_service_done && d->priv->resolve_host_name_done, NULL);

    return &d->priv->address;
}

guint16 aui_service_dialog_get_port(AuiServiceDialog *d) {
    g_return_val_if_fail(AUI_IS_SERVICE_DIALOG(d), 0);
    g_return_val_if_fail(d->priv->resolve_service_done, 0);

    return d->priv->port;
}

const char* aui_service_dialog_get_host_name(AuiServiceDialog *d) {
    g_return_val_if_fail(AUI_IS_SERVICE_DIALOG(d), NULL);
    g_return_val_if_fail(d->priv->resolve_service_done, NULL);

    return d->priv->host_name;
}

const AvahiStringList *aui_service_dialog_get_txt_data(AuiServiceDialog *d) {
    g_return_val_if_fail(AUI_IS_SERVICE_DIALOG(d), NULL);
    g_return_val_if_fail(d->priv->resolve_service_done, NULL);

    return d->priv->txt_data;
}

void aui_service_dialog_set_resolve_service(AuiServiceDialog *d, gboolean resolve) {
    g_return_if_fail(AUI_IS_SERVICE_DIALOG(d));

    d->priv->resolve_service = resolve;
}

gboolean aui_service_dialog_get_resolve_service(AuiServiceDialog *d) {
    g_return_val_if_fail(AUI_IS_SERVICE_DIALOG(d), FALSE);

    return d->priv->resolve_service;
}

void aui_service_dialog_set_resolve_host_name(AuiServiceDialog *d, gboolean resolve) {
    g_return_if_fail(AUI_IS_SERVICE_DIALOG(d));

    d->priv->resolve_host_name = resolve;
}

gboolean aui_service_dialog_get_resolve_host_name(AuiServiceDialog *d) {
    g_return_val_if_fail(AUI_IS_SERVICE_DIALOG(d), FALSE);

    return d->priv->resolve_host_name;
}

void aui_service_dialog_set_address_family(AuiServiceDialog *d, AvahiProtocol proto) {
    g_return_if_fail(AUI_IS_SERVICE_DIALOG(d));
    g_return_if_fail(proto == AVAHI_PROTO_UNSPEC || proto == AVAHI_PROTO_INET || proto == AVAHI_PROTO_INET6);

    d->priv->address_family = proto;
}

AvahiProtocol aui_service_dialog_get_address_family(AuiServiceDialog *d) {
    g_return_val_if_fail(AUI_IS_SERVICE_DIALOG(d), AVAHI_PROTO_UNSPEC);

    return d->priv->address_family;
}

static void aui_service_dialog_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    AuiServiceDialog *d = AUI_SERVICE_DIALOG(object);

    switch (prop_id) {
        case PROP_BROWSE_SERVICE_TYPES:
            aui_service_dialog_set_browse_service_typesv(d, g_value_get_pointer(value));
            break;

        case PROP_DOMAIN:
            aui_service_dialog_set_domain(d, g_value_get_string(value));
            break;

        case PROP_SERVICE_TYPE:
            aui_service_dialog_set_service_type(d, g_value_get_string(value));
            break;

        case PROP_SERVICE_NAME:
            aui_service_dialog_set_service_name(d, g_value_get_string(value));
            break;

        case PROP_RESOLVE_SERVICE:
            aui_service_dialog_set_resolve_service(d, g_value_get_boolean(value));
            break;

        case PROP_RESOLVE_HOST_NAME:
            aui_service_dialog_set_resolve_host_name(d, g_value_get_boolean(value));
            break;

        case PROP_ADDRESS_FAMILY:
            aui_service_dialog_set_address_family(d, g_value_get_int(value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void aui_service_dialog_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    AuiServiceDialog *d = AUI_SERVICE_DIALOG(object);

    switch (prop_id) {
        case PROP_BROWSE_SERVICE_TYPES:
            g_value_set_pointer(value, (gpointer) aui_service_dialog_get_browse_service_types(d));
            break;

        case PROP_DOMAIN:
            g_value_set_string(value, aui_service_dialog_get_domain(d));
            break;

        case PROP_SERVICE_TYPE:
            g_value_set_string(value, aui_service_dialog_get_service_type(d));
            break;

        case PROP_SERVICE_NAME:
            g_value_set_string(value, aui_service_dialog_get_service_name(d));
            break;

        case PROP_ADDRESS:
            g_value_set_pointer(value, (gpointer) aui_service_dialog_get_address(d));
            break;

        case PROP_PORT:
            g_value_set_uint(value, aui_service_dialog_get_port(d));
            break;

        case PROP_HOST_NAME:
            g_value_set_string(value, aui_service_dialog_get_host_name(d));
            break;

        case PROP_TXT_DATA:
            g_value_set_pointer(value, (gpointer) aui_service_dialog_get_txt_data(d));
            break;

        case PROP_RESOLVE_SERVICE:
            g_value_set_boolean(value, aui_service_dialog_get_resolve_service(d));
            break;

        case PROP_RESOLVE_HOST_NAME:
            g_value_set_boolean(value, aui_service_dialog_get_resolve_host_name(d));
            break;

        case PROP_ADDRESS_FAMILY:
            g_value_set_int(value, aui_service_dialog_get_address_family(d));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}
