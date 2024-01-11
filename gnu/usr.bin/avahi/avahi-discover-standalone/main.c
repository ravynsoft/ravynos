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

#include <sys/ioctl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <unistd.h>

#include <gtk/gtk.h>

#include <avahi-core/core.h>
#include <avahi-core/lookup.h>

#include <avahi-common/strlst.h>
#include <avahi-common/domain.h>
#include <avahi-common/error.h>

#include <avahi-glib/glib-watch.h>
#include <avahi-glib/glib-malloc.h>

struct ServiceType;

struct Service {
    struct ServiceType *service_type;
    gchar *service_name;
    gchar *domain_name;

    AvahiIfIndex interface;
    AvahiProtocol protocol;

    GtkTreeRowReference *tree_ref;
};

struct ServiceType {
    gchar *service_type;
    AvahiSServiceBrowser *browser;

    GList *services;
    GtkTreeRowReference *tree_ref;
};

static GtkWidget *main_window = NULL;
static GtkTreeView *tree_view = NULL;
static GtkTreeStore *tree_store = NULL;
static GtkLabel *info_label = NULL;
static AvahiServer *server = NULL;
static AvahiSServiceTypeBrowser *service_type_browser = NULL;
static GHashTable *service_type_hash_table = NULL;
static AvahiSServiceResolver *service_resolver = NULL;
static struct Service *current_service = NULL;

static struct Service *get_service(const gchar *service_type, const gchar *service_name, const gchar*domain_name, AvahiIfIndex interface, AvahiProtocol protocol) {
    struct ServiceType *st;
    GList *l;

    if (!(st = g_hash_table_lookup(service_type_hash_table, service_type)))
        return NULL;

    for (l = st->services; l; l = l->next) {
        struct Service *s = l->data;

        if (s->interface == interface &&
            s->protocol == protocol &&
            avahi_domain_equal(s->service_name, service_name) &&
            avahi_domain_equal(s->domain_name, domain_name))
            return s;
    }

    return NULL;
}

static void free_service(struct Service *s) {
    GtkTreeIter iter;
    GtkTreePath *path;

    if (current_service == s) {
        current_service = NULL;

        if (service_resolver) {
            avahi_s_service_resolver_free(service_resolver);
            service_resolver = NULL;
        }

        gtk_label_set_text(info_label, "<i>Service removed</i>");
    }

    s->service_type->services = g_list_remove(s->service_type->services, s);

    if ((path = gtk_tree_row_reference_get_path(s->tree_ref))) {
        gtk_tree_model_get_iter(GTK_TREE_MODEL(tree_store), &iter, path);
        gtk_tree_path_free(path);
    }

    gtk_tree_store_remove(tree_store, &iter);

    gtk_tree_row_reference_free(s->tree_ref);

    g_free(s->service_name);
    g_free(s->domain_name);
    g_free(s);
}

static void service_browser_callback(
    AVAHI_GCC_UNUSED AvahiSServiceBrowser *b,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char *service_name,
    const char *service_type,
    const char *domain_name,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    AVAHI_GCC_UNUSED void* userdata) {

    if (event == AVAHI_BROWSER_NEW) {
        struct Service *s;
        GtkTreeIter iter, piter;
        GtkTreePath *path, *ppath;
        gchar iface[256];
	char name[IF_NAMESIZE];

        s = g_new(struct Service, 1);
        s->service_name = g_strdup(service_name);
        s->domain_name = g_strdup(domain_name);
        s->interface = interface;
        s->protocol = protocol;
        s->service_type = g_hash_table_lookup(service_type_hash_table, service_type);
        g_assert(s->service_type);

        s->service_type->services = g_list_prepend(s->service_type->services, s);

        ppath = gtk_tree_row_reference_get_path(s->service_type->tree_ref);
        gtk_tree_model_get_iter(GTK_TREE_MODEL(tree_store), &piter, ppath);

        snprintf(iface, sizeof(iface), "%s %s", if_indextoname(interface, name), avahi_proto_to_string(protocol));

        gtk_tree_store_append(tree_store, &iter, &piter);
        gtk_tree_store_set(tree_store, &iter, 0, s->service_name, 1, iface, 2, s, -1);

        path = gtk_tree_model_get_path(GTK_TREE_MODEL(tree_store), &iter);
        s->tree_ref = gtk_tree_row_reference_new(GTK_TREE_MODEL(tree_store), path);
        gtk_tree_path_free(path);

        gtk_tree_view_expand_row(tree_view, ppath, FALSE);
        gtk_tree_path_free(ppath);


    } else if (event == AVAHI_BROWSER_REMOVE) {
        struct Service* s;

        if ((s = get_service(service_type, service_name, domain_name, interface, protocol)))
            free_service(s);
    }
}

static void service_type_browser_callback(
    AVAHI_GCC_UNUSED AvahiSServiceTypeBrowser *b,
    AVAHI_GCC_UNUSED AvahiIfIndex interface,
    AVAHI_GCC_UNUSED AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char *service_type,
    const char *domain,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    AVAHI_GCC_UNUSED void * userdata) {

    struct ServiceType *st;
    GtkTreePath *path;
    GtkTreeIter iter;

    if (event != AVAHI_BROWSER_NEW)
        return;

    if (g_hash_table_lookup(service_type_hash_table, service_type))
        return;

    st = g_new(struct ServiceType, 1);
    st->service_type = g_strdup(service_type);
    st->services = NULL;

    gtk_tree_store_append(tree_store, &iter, NULL);
    gtk_tree_store_set(tree_store, &iter, 0, st->service_type, 1, "", 2, NULL, -1);

    path = gtk_tree_model_get_path(GTK_TREE_MODEL(tree_store), &iter);
    st->tree_ref = gtk_tree_row_reference_new(GTK_TREE_MODEL(tree_store), path);
    gtk_tree_path_free(path);

    g_hash_table_insert(service_type_hash_table, st->service_type, st);

    st->browser = avahi_s_service_browser_new(server, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, st->service_type, domain, 0, service_browser_callback, NULL);
}

static void update_label(struct Service *s, const gchar *hostname, const AvahiAddress *a, guint16 port, AvahiStringList *txt) {
    gchar t[512], address[64], *txt_s;
    char name[IF_NAMESIZE];

    if (a && hostname) {
        char na[AVAHI_ADDRESS_STR_MAX];
        avahi_address_snprint(na, sizeof(na), a);
        snprintf(address, sizeof(address), "%s/%s:%u", hostname, na, port);

        if (txt)
            txt_s = avahi_string_list_to_string(txt);
        else
            txt_s = g_strdup("<i>empty</i>");
    } else {
        snprintf(address, sizeof(address), "<i>n/a</i>");
        txt_s = g_strdup("<i>n/a</i>");
    }

    snprintf(t, sizeof(t),
             "<b>Service Type:</b> %s\n"
             "<b>Service Name:</b> %s\n"
             "<b>Domain Name:</b> %s\n"
             "<b>Interface:</b> %s %s\n"
             "<b>Address:</b> %s\n"
             "<b>TXT Data:</b> %s",
             s->service_type->service_type,
             s->service_name,
             s->domain_name,
             if_indextoname(s->interface,name), avahi_proto_to_string(s->protocol),
             address,
             txt_s);

    gtk_label_set_markup(info_label, t);

    g_free(txt_s);
}

static struct Service *get_service_on_cursor(void) {
    GtkTreePath *path;
    struct Service *s;
    GtkTreeIter iter;

    gtk_tree_view_get_cursor(tree_view, &path, NULL);

    if (!path)
        return NULL;

    gtk_tree_model_get_iter(GTK_TREE_MODEL(tree_store), &iter, path);
    gtk_tree_model_get(GTK_TREE_MODEL(tree_store), &iter, 2, &s, -1);
    gtk_tree_path_free(path);

    return s;
}

static void service_resolver_callback(
    AvahiSServiceResolver *r,
    AVAHI_GCC_UNUSED AvahiIfIndex interface,
    AVAHI_GCC_UNUSED AvahiProtocol protocol,
    AvahiResolverEvent event,
    AVAHI_GCC_UNUSED const char *name,
    AVAHI_GCC_UNUSED const char *type,
    AVAHI_GCC_UNUSED const char *domain,
    const char *host_name,
    const AvahiAddress *a,
    uint16_t port,
    AvahiStringList *txt,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    void* userdata) {

    struct Service *s;
    g_assert(r);

    if (!(s = get_service_on_cursor()) || userdata != s) {
        g_assert(r == service_resolver);
        avahi_s_service_resolver_free(service_resolver);
        service_resolver = NULL;
        return;
    }

    if (event == AVAHI_RESOLVER_FAILURE) {
        char t[256];
        snprintf(t, sizeof(t), "<i>Failed to resolve: %s.</i>", avahi_strerror(avahi_server_errno(server)));
        gtk_label_set_markup(info_label, t);
    } else if (event == AVAHI_RESOLVER_FOUND)
        update_label(s, host_name, a, port, txt);
}

static void tree_view_on_cursor_changed(AVAHI_GCC_UNUSED GtkTreeView *tv, AVAHI_GCC_UNUSED gpointer userdata) {
    struct Service *s;

    if (!(s = get_service_on_cursor()))
        return;

    if (service_resolver)
        avahi_s_service_resolver_free(service_resolver);

    update_label(s, NULL, NULL, 0, NULL);

    service_resolver = avahi_s_service_resolver_new(server, s->interface, s->protocol, s->service_name, s->service_type->service_type, s->domain_name, AVAHI_PROTO_UNSPEC, 0, service_resolver_callback, s);
}

static gboolean main_window_on_delete_event(AVAHI_GCC_UNUSED GtkWidget *widget, AVAHI_GCC_UNUSED GdkEvent *event, AVAHI_GCC_UNUSED gpointer user_data) {
    gtk_main_quit();
    return FALSE;
}

int main(int argc, char *argv[]) {
    GtkBuilder *ui;
    AvahiServerConfig config;
    GtkTreeViewColumn *c;
    gint error;
    AvahiGLibPoll *poll_api;

    gtk_init(&argc, &argv);

    avahi_set_allocator(avahi_glib_allocator());

    poll_api = avahi_glib_poll_new(NULL, G_PRIORITY_DEFAULT);

    ui = gtk_builder_new();
    gtk_builder_set_translation_domain(ui, "avahi");
    gtk_builder_add_from_file(ui, AVAHI_INTERFACES_DIR"avahi-discover.ui", NULL);
    main_window = GTK_WIDGET(gtk_builder_get_object(ui, "main_window"));
    g_signal_connect(main_window, "delete-event", (GCallback) main_window_on_delete_event, NULL);

    tree_view = GTK_TREE_VIEW(gtk_builder_get_object(ui, "tree_view"));
    g_signal_connect(GTK_WIDGET(tree_view), "cursor-changed", (GCallback) tree_view_on_cursor_changed, NULL);

    info_label = GTK_LABEL(gtk_builder_get_object(ui, "info_label"));

    tree_store = gtk_tree_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
    gtk_tree_view_set_model(tree_view, GTK_TREE_MODEL(tree_store));
    gtk_tree_view_insert_column_with_attributes(tree_view, -1, "Name", gtk_cell_renderer_text_new(), "text", 0, NULL);
    gtk_tree_view_insert_column_with_attributes(tree_view, -1, "Interface", gtk_cell_renderer_text_new(), "text", 1, NULL);

    gtk_tree_view_column_set_resizable(c = gtk_tree_view_get_column(tree_view, 0), TRUE);
    gtk_tree_view_column_set_sizing(c, GTK_TREE_VIEW_COLUMN_GROW_ONLY);
    gtk_tree_view_column_set_expand(c, TRUE);

    service_type_hash_table = g_hash_table_new((GHashFunc) avahi_domain_hash, (GEqualFunc) avahi_domain_equal);

    avahi_server_config_init(&config);
    config.publish_hinfo = config.publish_addresses = config.publish_domain = config.publish_workstation = FALSE;
    server = avahi_server_new(avahi_glib_poll_get(poll_api), &config, NULL, NULL, &error);
    avahi_server_config_free(&config);

    g_assert(server);

    service_type_browser = avahi_s_service_type_browser_new(server, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, argc >= 2 ? argv[1] : NULL, 0, service_type_browser_callback, NULL);

    gtk_main();

    avahi_server_free(server);
    avahi_glib_poll_free(poll_api);

    return 0;
}
