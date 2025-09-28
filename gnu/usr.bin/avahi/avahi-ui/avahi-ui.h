#ifndef fooavahiuihfoo
#define fooavahiuihfoo

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

#include <gtk/gtk.h>

#include <avahi-client/client.h>

/** \file avahi-ui.h A Gtk dialog for browsing for services */

G_BEGIN_DECLS

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#define AUI_TYPE_SERVICE_DIALOG            (aui_service_dialog_get_type())
#define AUI_SERVICE_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), AUI_TYPE_SERVICE_DIALOG, AuiServiceDialog))
#define AUI_SERVICE_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), AUI_TYPE_SERVICE_DIALOG, AuiServiceDialogClass))
#define AUI_IS_SERVICE_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), AUI_TYPE_SERVICE_DIALOG))
#define AUI_IS_SERVICE_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), AUI_TYPE_SERVICE_DIALOG))
#define AUI_SERVICE_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), AUI_TYPE_SERVICE_DIALOG, AuiServiceDialogClass))

typedef struct _AuiServiceDialogPrivate AuiServiceDialogPrivate;
typedef struct _AuiServiceDialogClass  AuiServiceDialogClass;

struct _AuiServiceDialogClass {
    GtkDialogClass parent_class;

    /* Padding for future expansion */
    void (*_aui_reserved1)(void);
    void (*_aui_reserved2)(void);
    void (*_aui_reserved3)(void);
    void (*_aui_reserved4)(void);
};

struct _AuiServiceDialog {
    GtkDialog parent_instance;
    AuiServiceDialogPrivate *priv;
};

/* ServiceDialog */
GType aui_service_dialog_get_type(void) G_GNUC_CONST;

#endif

/** The GTK service dialog structure */
typedef struct _AuiServiceDialog AuiServiceDialog;

/** @{ \name Construction */

/** Create a new service browser dialog with the specific title,
 * parent window and the speicified buttons. The buttons are specified
 * in a similar way to GtkFileChooserDialog. Please note that at least
 * one button has to respond GTK_RESPONSE_ACCEPT. */
GtkWidget* aui_service_dialog_new(
        const gchar *title,
        GtkWindow *parent,
        const gchar *first_button_text, ...) G_GNUC_NULL_TERMINATED;

/** \cond fulldocs */
GtkWidget *aui_service_dialog_new_valist(
        const gchar *title,
        GtkWindow *parent,
        const gchar *first_button_text,
        va_list varargs);
/** \endcond */

/** @} */

/** @{ \name Service types to browse for */

/** Select the service types to browse for. Takes a NULL terminated list of DNS-SD service types. i.e. _http._tcp */
void aui_service_dialog_set_browse_service_types(AuiServiceDialog *d, const gchar *type, ...) G_GNUC_NULL_TERMINATED;
/** Same as aui_service_dialog_set_browse_service_types() but take a NULL terminated array */
void aui_service_dialog_set_browse_service_typesv(AuiServiceDialog *d, const gchar *const*type);
/** Return the service types currently browsed for. i.e. what was previously set with aui_service_dialog_set_browse_service_types() */
const gchar*const* aui_service_dialog_get_browse_service_types(AuiServiceDialog *d);
/** Overwrite the pretty name shown in the service type column. \since 0.6.22 */
void aui_service_dialog_set_service_type_name(AuiServiceDialog *d, const gchar *type, const gchar *name);

/** @} */

/** @{ \name Domain to browse in */

/** Set the domain to browse in */
void aui_service_dialog_set_domain(AuiServiceDialog *d, const gchar *domain);
/** Query the domain that is browsed in */
const gchar* aui_service_dialog_get_domain(AuiServiceDialog *d);

/** @} */

/** @{ \name Selected service item */

/** Set the service type for the service to select */
void aui_service_dialog_set_service_type(AuiServiceDialog *d, const gchar *name);

/** Query the service type of the currently selected service */
const gchar* aui_service_dialog_get_service_type(AuiServiceDialog *d);

/** Set the service name for the service to select */
void aui_service_dialog_set_service_name(AuiServiceDialog *d, const gchar *name);

/** Query the service name of the currently select service */
const gchar* aui_service_dialog_get_service_name(AuiServiceDialog *d);

/** @} */

/** @{ \name Resolved service information */

/** Return the IP address of the selected service. (Only valid if host name resolving has not been disabled via aui_service_dialog_set_resolve_host_name()) */
const AvahiAddress* aui_service_dialog_get_address(AuiServiceDialog *d);

/** Return the IP port number of the selected service */
guint16 aui_service_dialog_get_port(AuiServiceDialog *d);

/** Return the host name of the selected service */
const gchar* aui_service_dialog_get_host_name(AuiServiceDialog *d);

/** Return the TXT metadata of the selected service */
const AvahiStringList *aui_service_dialog_get_txt_data(AuiServiceDialog *d);

/** @} */

/** @{ \name Resolving settings */

/** Disable/Enable automatic service resolving. Disabling this feature
 * will require you to resolve the selected service on our own. I.e. the port
 * number, the TXT data and the host name/IP address will not be
 * available after a service has been selected. This functionality
 * offers a certain optimization in the traffic imposed on the
 * network. Most people will not want to touch this. */
void aui_service_dialog_set_resolve_service(AuiServiceDialog *d, gboolean resolve);

/** Query the last status of aui_service_dialog_set_resolve_service() */
gboolean aui_service_dialog_get_resolve_service(AuiServiceDialog *d);

/** Disable/Enable automatic host name resolving. Disabling this
 * feature will cause aui_service_dialog_get_address() return NULL in
 * all case because avahi-ui will not resolve the host name of the
 * selected service to an address. This is a slight optimization
 * regarding the traffic imposed by this query to the network. By
 * default, avahi-ui will resolve the host names of selected services. */
void aui_service_dialog_set_resolve_host_name(AuiServiceDialog *d, gboolean resolve);

/** Query the last status of aui_service_dialog_set_resolve_host_name() */
gboolean aui_service_dialog_get_resolve_host_name(AuiServiceDialog *d);

/** @} */

/** @{ \name Address family */

/** Select the address family to look for services of. This can be
used to look only for IPv6 services or only for IPv4 services. By
default avahi-ui will browse for both IPv4 and IPv6 services.*/
void aui_service_dialog_set_address_family(AuiServiceDialog *d, AvahiProtocol proto);

/** Query the address family we're looking for. */
AvahiProtocol aui_service_dialog_get_address_family(AuiServiceDialog *d);

/** @} */

G_END_DECLS

#endif
