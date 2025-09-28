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

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <locale.h>
#include <getopt.h>

#include <gtk/gtk.h>

#include <avahi-client/client.h>
#include <avahi-common/strlst.h>
#include <avahi-common/malloc.h>
#include <avahi-common/domain.h>
#include <avahi-common/i18n.h>

#include "avahi-ui.h"

typedef enum {
    COMMAND_HELP,
    COMMAND_SSH,
    COMMAND_VNC,
    COMMAND_SHELL
} Command;

typedef struct Config {
    char *domain;
    Command command;
} Config;

static void help(FILE *f, const char *argv0) {
    fprintf(f,
            _("%s [options]\n\n"
              "    -h --help            Show this help\n"
              "    -s --ssh             Browse SSH servers\n"
              "    -v --vnc             Browse VNC servers\n"
              "    -S --shell           Browse both SSH and VNC\n"
              "    -d --domain=DOMAIN   The domain to browse in\n"),
            argv0);
}

static int parse_command_line(Config *c, int argc, char *argv[]) {
    int o;

    static const struct option long_options[] = {
        { "help",           no_argument,       NULL, 'h' },
        { "ssh",            no_argument,       NULL, 's' },
        { "vnc",            no_argument,       NULL, 'v' },
        { "shell",          no_argument,       NULL, 'S' },
        { "domain",         required_argument, NULL, 'd' },
        { NULL, 0, NULL, 0 }
    };

    while ((o = getopt_long(argc, argv, "hVd:svS", long_options, NULL)) >= 0) {

        switch(o) {
            case 'h':
                c->command = COMMAND_HELP;
                break;
            case 's':
                c->command = COMMAND_SSH;
                break;
            case 'v':
                c->command = COMMAND_VNC;
                break;
            case 'S':
                c->command = COMMAND_SHELL;
                break;
            case 'd':
                avahi_free(c->domain);
                c->domain = avahi_strdup(optarg);
                break;
            default:
                return -1;
        }
    }

    if (optind < argc) {
        fprintf(stderr, _("Too many arguments\n"));
        return -1;
    }

    return 0;
}

int main(int argc, char*argv[]) {
    GtkWidget *d;
    Config config;
    const char *argv0;

    avahi_init_i18n();
    setlocale(LC_ALL, "");

    if ((argv0 = strrchr(argv[0], '/')))
        argv0++;
    else
        argv0 = argv[0];

    if (g_str_has_suffix(argv[0], "bshell"))
        config.command = COMMAND_SHELL;
    else if (g_str_has_suffix(argv[0], "bvnc"))
        config.command = COMMAND_VNC;
    else
        config.command = COMMAND_SSH;

    /* defaults to local */
    config.domain = NULL;

    if (parse_command_line(&config, argc, argv) < 0) {
        help(stderr, argv0);
        return 1;
    }

    bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);

    gtk_init(&argc, &argv);

    switch (config.command) {
        case COMMAND_HELP:
            help(stdout, argv0);
            return 0;
            break;

        case COMMAND_SHELL:
            d = aui_service_dialog_new(_("Choose Shell Server"), NULL, _("_Cancel"), GTK_RESPONSE_CANCEL, _("C_onnect"), GTK_RESPONSE_ACCEPT, NULL);
            aui_service_dialog_set_browse_service_types(AUI_SERVICE_DIALOG(d), "_rfb._tcp", "_ssh._tcp", NULL);
            aui_service_dialog_set_service_type_name(AUI_SERVICE_DIALOG(d), "_rfb._tcp", _("Desktop"));
            aui_service_dialog_set_service_type_name(AUI_SERVICE_DIALOG(d), "_ssh._tcp", _("Terminal"));
            break;

        case COMMAND_VNC:
            d = aui_service_dialog_new(_("Choose VNC server"), NULL, _("_Cancel"), GTK_RESPONSE_CANCEL, _("C_onnect"), GTK_RESPONSE_ACCEPT, NULL);
            aui_service_dialog_set_browse_service_types(AUI_SERVICE_DIALOG(d), "_rfb._tcp", NULL);
            break;

        case COMMAND_SSH:
            d = aui_service_dialog_new(_("Choose SSH server"), NULL, _("_Cancel"), GTK_RESPONSE_CANCEL, _("C_onnect"), GTK_RESPONSE_ACCEPT, NULL);
            aui_service_dialog_set_browse_service_types(AUI_SERVICE_DIALOG(d), "_ssh._tcp", NULL);
            break;
    }

    aui_service_dialog_set_domain (AUI_SERVICE_DIALOG(d), config.domain);
    aui_service_dialog_set_resolve_service(AUI_SERVICE_DIALOG(d), TRUE);
    aui_service_dialog_set_resolve_host_name(AUI_SERVICE_DIALOG(d), !avahi_nss_support());

    gtk_window_present(GTK_WINDOW(d));

    if (gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_ACCEPT) {
        char a[AVAHI_ADDRESS_STR_MAX], *u = NULL, *n = NULL;
        char *h = NULL, *t = NULL;
        const AvahiStringList *txt;

        t = g_strdup(aui_service_dialog_get_service_type(AUI_SERVICE_DIALOG(d)));
        n = g_strdup(aui_service_dialog_get_service_name(AUI_SERVICE_DIALOG(d)));

        if (avahi_nss_support())
            h = g_strdup(aui_service_dialog_get_host_name(AUI_SERVICE_DIALOG(d)));
        else
            h = g_strdup(avahi_address_snprint(a, sizeof(a), aui_service_dialog_get_address(AUI_SERVICE_DIALOG(d))));

        g_print(_("Connecting to '%s' ...\n"), n);

        if (avahi_domain_equal(t, "_rfb._tcp")) {
            char p[AVAHI_DOMAIN_NAME_MAX+16];
            snprintf(p, sizeof(p), "%s:%u", h, aui_service_dialog_get_port(AUI_SERVICE_DIALOG(d))-5900);

            gtk_widget_destroy(d);

            g_print("vncviewer %s\n", p);
            execlp("xvncviewer", "xvncviewer", p, NULL);
            execlp("vncviewer", "vncviewer", p, NULL);

        } else {
            char p[16];

            snprintf(p, sizeof(p), "%u", aui_service_dialog_get_port(AUI_SERVICE_DIALOG(d)));

            for (txt = aui_service_dialog_get_txt_data(AUI_SERVICE_DIALOG(d)); txt; txt = txt->next) {
                char *key, *value;

                if (avahi_string_list_get_pair((AvahiStringList*) txt, &key, &value, NULL) < 0)
                    break;

                if (strcmp(key, "u") == 0)
                    u = g_strdup(value);

                avahi_free(key);
                avahi_free(value);
            }

            gtk_widget_destroy(d);

            if (u) {
                g_print("ssh -p %s -l %s %s\n", p, u, h);

                if (isatty(0) || !getenv("DISPLAY"))
                    execlp("ssh", "ssh", "-p", p, "-l", u, h, NULL);
                else {
                    execlp("x-terminal-emulator", "x-terminal-emulator", "-T", n, "-e", "ssh", "-p", p, "-l", u, h, NULL);
                    execlp("gnome-terminal", "gnome-terminal", "-t", n, "-x", "ssh", "-p", p, "-l", u, h, NULL);
                    execlp("xterm", "xterm", "-T", n, "-e", "ssh", "-p", p, "-l", u, h, NULL);
                }
            } else {
                g_print("ssh -p %s %s\n", p, h);

                if (isatty(0) || !getenv("DISPLAY"))
                    execlp("ssh", "ssh", "-p", p, h, NULL);
                else {
                    execlp("x-terminal-emulator", "x-terminal-emulator", "-T", n, "-e", "ssh", "-p", p, h, NULL);
                    execlp("gnome-terminal", "gnome-terminal", "-t", n, "-x", "ssh", "-p", p, h, NULL);
                    execlp("xterm", "xterm", "-T", n, "-e", "ssh", "-p", p, h, NULL);
                }
            }
        }

        g_warning(_("execlp() failed: %s\n"), strerror(errno));

        g_free(h);
        g_free(u);
        g_free(t);
        g_free(n);

    } else {
        gtk_widget_destroy(d);

        g_print(_("Canceled.\n"));
    }

    g_free(config.domain);

    return 1;
}
