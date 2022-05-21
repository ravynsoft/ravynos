// SPDX-License-Identifier: GPL-2.0-only
#define _POSIX_C_SOURCE 200809L
#include "config.h"
#include <signal.h>
#include <sys/wait.h>
#include <wlr/types/wlr_data_control_v1.h>
#include <wlr/types/wlr_export_dmabuf_v1.h>
#include <wlr/types/wlr_gamma_control_v1.h>
#include <wlr/types/wlr_input_inhibitor.h>
#include <wlr/types/wlr_primary_selection_v1.h>
#include <wlr/types/wlr_screencopy_v1.h>
#include <wlr/types/wlr_viewporter.h>
#include "config/rcxml.h"
#include "config/session.h"
#include "labwc.h"
#include "layers.h"
#include "menu/menu.h"
#include "ssd.h"
#include "theme.h"

static struct wlr_compositor *compositor;
static struct wl_event_source *sighup_source;
static struct wl_event_source *sigint_source;
static struct wl_event_source *sigterm_source;

static struct server *g_server;

static void
reload_config_and_theme(void)
{
	/* TODO: use rc.config_path */
	rcxml_finish();
	rcxml_read(NULL);
	theme_finish(g_server->theme);
	theme_init(g_server->theme, g_server->renderer, rc.theme_name);

	struct view *view;
	wl_list_for_each (view, &g_server->views, link) {
		if (!view->mapped || !view->ssd.enabled) {
			continue;
		}
		view->margin = ssd_thickness(view);
		ssd_update_geometry(view, true);
	}

	menu_reconfigure(g_server, g_server->rootmenu);
	seat_reconfigure(g_server);
	damage_all_outputs(g_server);
}

static int
handle_sighup(int signal, void *data)
{
	session_environment_init();
	reload_config_and_theme();
	return 0;
}

static int
handle_sigterm(int signal, void *data)
{
	struct wl_display *display = data;

	wl_display_terminate(display);
	return 0;
}

static void
drop_permissions(void)
{
	if (getuid() != geteuid() || getgid() != getegid()) {
		if (setgid(getgid())) {
			wlr_log(WLR_ERROR, "unable to drop root group");
			exit(EXIT_FAILURE);
		}
		if (setuid(getuid())) {
			wlr_log(WLR_ERROR, "unable to drop root user");
			exit(EXIT_FAILURE);
		}
	}
	if (setgid(0) != -1 || setuid(0) != -1) {
		wlr_log(WLR_ERROR, "unable to drop root");
		exit(EXIT_FAILURE);
	}
}

static void
seat_inhibit_input(struct seat *seat,  struct wl_client *active_client)
{
	seat->active_client_while_inhibited = active_client;

	if (seat->focused_layer &&
	    (wl_resource_get_client(seat->focused_layer->resource) !=
	    active_client)) {
		seat_set_focus_layer(seat, NULL);
	}
	struct wlr_surface *previous_kb_surface =
		seat->seat->keyboard_state.focused_surface;
	if (previous_kb_surface &&
	    wl_resource_get_client(previous_kb_surface->resource) != active_client) {
		seat_focus_surface(seat, NULL);	  /* keyboard focus */
	}

	struct wlr_seat_client *previous_ptr_client =
		seat->seat->pointer_state.focused_client;
	if (previous_ptr_client &&
	    (previous_ptr_client->client != active_client)) {
		wlr_seat_pointer_clear_focus(seat->seat);
	}
}

static void
seat_disinhibit_input(struct seat *seat)
{
	seat->active_client_while_inhibited = NULL;

	/*
	 * Triggers a refocus of the topmost surface layer if necessary
	 * TODO: Make layer surface focus per-output based on cursor position
	 */
	struct output *output;
	wl_list_for_each(output, &seat->server->outputs, link) {
		arrange_layers(output);
	}
}



static void
handle_input_inhibit(struct wl_listener *listener, void *data)
{
	wlr_log(WLR_INFO, "activate input inhibit");

	struct server *server =
		wl_container_of(listener, server, input_inhibit_activate);
	seat_inhibit_input(&server->seat, server->input_inhibit->active_client);
}

static void
handle_input_disinhibit(struct wl_listener *listener, void *data)
{
	wlr_log(WLR_INFO, "deactivate input inhibit");

	struct server *server =
		wl_container_of(listener, server, input_inhibit_deactivate);
	seat_disinhibit_input(&server->seat);
}

void
server_init(struct server *server)
{
	server->wl_display = wl_display_create();
	if (!server->wl_display) {
		wlr_log(WLR_ERROR, "cannot allocate a wayland display");
		exit(EXIT_FAILURE);
	}

	/* Catch SIGHUP */
	struct wl_event_loop *event_loop = NULL;
	event_loop = wl_display_get_event_loop(server->wl_display);
	sighup_source = wl_event_loop_add_signal(
		event_loop, SIGHUP, handle_sighup, NULL);
	sigint_source = wl_event_loop_add_signal(
		event_loop, SIGINT, handle_sigterm, server->wl_display);
	sigterm_source = wl_event_loop_add_signal(
		event_loop, SIGTERM, handle_sigterm, server->wl_display);

	/*
	 * The backend is a feature which abstracts the underlying input and
	 * output hardware. The autocreate option will choose the most suitable
	 * backend based on the current environment, such as opening an x11
	 * window if an x11 server is running.
	 */
	server->backend = wlr_backend_autocreate(server->wl_display);
	if (!server->backend) {
		wlr_log(WLR_ERROR, "unable to create backend");
		exit(EXIT_FAILURE);
	}

	/*
	 * The wlroots library makes use of systemd's logind to handle sessions
	 * and to allow compositors to run without elevated privileges.
	 * If running without logind or elogind, users may choose to set the
	 * setuid bit on the labwc executable despite associated security
	 * implications. In order to support this, but limit the elevated
	 * privileges as much as possible, we drop permissions at this point.
	 */
	drop_permissions();

	/*
	 * Autocreates a renderer, either Pixman, GLES2 or Vulkan for us. The
	 * user can also specify a renderer using the WLR_RENDERER env var.
	 * The renderer is responsible for defining the various pixel formats it
	 * supports for shared memory, this configures that for clients.
	 */
	server->renderer = wlr_renderer_autocreate(server->backend);
	if (!server->renderer) {
		wlr_log(WLR_ERROR, "unable to create renderer");
		exit(EXIT_FAILURE);
	}

	wlr_renderer_init_wl_display(server->renderer, server->wl_display);

	/*
	 * Autocreates an allocator for us. The allocator is the bridge between
	 * the renderer and the backend. It handles the buffer creation,
	 * allowing wlroots to render onto the screen
	 */
	server->allocator = wlr_allocator_autocreate(
		server->backend, server->renderer);
	if (!server->allocator) {
		wlr_log(WLR_ERROR, "unable to create allocator");
		exit(EXIT_FAILURE);
	}

	wl_list_init(&server->views);
	wl_list_init(&server->unmanaged_surfaces);

	/*
	 * Create some hands-off wlroots interfaces. The compositor is
	 * necessary for clients to allocate surfaces and the data device
	 * manager handles the clipboard. Each of these wlroots interfaces has
	 * room for you to dig your fingers in and play with their behavior if
	 * you want.
	 */
	compositor =
		wlr_compositor_create(server->wl_display, server->renderer);
	if (!compositor) {
		wlr_log(WLR_ERROR, "unable to create the wlroots compositor");
		exit(EXIT_FAILURE);
	}

	struct wlr_data_device_manager *device_manager = NULL;
	device_manager = wlr_data_device_manager_create(server->wl_display);
	if (!device_manager) {
		wlr_log(WLR_ERROR, "unable to create data device manager");
		exit(EXIT_FAILURE);
	}

	/*
	 * Empirically, primary selection doesn't work with Gtk apps unless the
	 * device manager is one of the earliest globals to be advertised. All
	 * credit to Wayfire for discovering this, though their symptoms
	 * (crash) are not the same as ours (silently does nothing). When adding
	 * more globals above this line it would be as well to check that
	 * middle-button paste still works with any Gtk app of your choice
	 *
	 * https://wayfire.org/2020/08/04/Wayfire-0-5.html
	 */
	wlr_primary_selection_v1_device_manager_create(server->wl_display);

	output_init(server);
	seat_init(server);

	/* Init xdg-shell */
	server->xdg_shell = wlr_xdg_shell_create(server->wl_display);
	if (!server->xdg_shell) {
		wlr_log(WLR_ERROR, "unable to create the XDG shell interface");
		exit(EXIT_FAILURE);
	}
	server->new_xdg_surface.notify = xdg_surface_new;
	wl_signal_add(&server->xdg_shell->events.new_surface,
		      &server->new_xdg_surface);

	/* Disable CSD */
	struct wlr_xdg_decoration_manager_v1 *xdg_deco_mgr = NULL;
	xdg_deco_mgr = wlr_xdg_decoration_manager_v1_create(server->wl_display);
	if (!xdg_deco_mgr) {
		wlr_log(WLR_ERROR, "unable to create the XDG deco manager");
		exit(EXIT_FAILURE);
	}
	wl_signal_add(&xdg_deco_mgr->events.new_toplevel_decoration,
		      &server->xdg_toplevel_decoration);
	server->xdg_toplevel_decoration.notify = xdg_toplevel_decoration;

	struct wlr_server_decoration_manager *deco_mgr = NULL;
	deco_mgr = wlr_server_decoration_manager_create(server->wl_display);
	if (!deco_mgr) {
		wlr_log(WLR_ERROR, "unable to create the server deco manager");
		exit(EXIT_FAILURE);
	}
	wlr_server_decoration_manager_set_default_mode(
		deco_mgr, rc.xdg_shell_server_side_deco ?
				  WLR_SERVER_DECORATION_MANAGER_MODE_SERVER :
				  WLR_SERVER_DECORATION_MANAGER_MODE_CLIENT);

	wlr_export_dmabuf_manager_v1_create(server->wl_display);
	wlr_screencopy_manager_v1_create(server->wl_display);
	wlr_data_control_manager_v1_create(server->wl_display);
	wlr_gamma_control_manager_v1_create(server->wl_display);
	wlr_viewporter_create(server->wl_display);

	server->relative_pointer_manager = wlr_relative_pointer_manager_v1_create(
		server->wl_display);
	server->constraints = wlr_pointer_constraints_v1_create(
		server->wl_display);

	server->new_constraint.notify = create_constraint;
	wl_signal_add(&server->constraints->events.new_constraint,
		&server->new_constraint);

	server->input_inhibit =
		wlr_input_inhibit_manager_create(server->wl_display);
	if (!server->input_inhibit) {
		wlr_log(WLR_ERROR, "unable to create input inhibit manager");
		exit(EXIT_FAILURE);
	}

	wl_signal_add(&server->input_inhibit->events.activate,
		&server->input_inhibit_activate);
	server->input_inhibit_activate.notify = handle_input_inhibit;

	wl_signal_add(&server->input_inhibit->events.deactivate,
		&server->input_inhibit_deactivate);
	server->input_inhibit_deactivate.notify = handle_input_disinhibit;

	server->foreign_toplevel_manager =
		wlr_foreign_toplevel_manager_v1_create(server->wl_display);

	layers_init(server);

#if HAVE_XWAYLAND
	/* Init xwayland */
	server->xwayland =
		wlr_xwayland_create(server->wl_display, compositor, true);
	if (!server->xwayland) {
		wlr_log(WLR_ERROR, "cannot create xwayland server");
		exit(EXIT_FAILURE);
	}
	server->new_xwayland_surface.notify = xwayland_surface_new;
	wl_signal_add(&server->xwayland->events.new_surface,
		      &server->new_xwayland_surface);

	if (setenv("DISPLAY", server->xwayland->display_name, true) < 0) {
		wlr_log_errno(WLR_ERROR, "unable to set DISPLAY for xwayland");
	} else {
		wlr_log(WLR_DEBUG, "xwayland is running on display %s",
			server->xwayland->display_name);
	}
#endif

	if (!wlr_xcursor_manager_load(server->seat.xcursor_manager, 1)) {
		wlr_log(WLR_ERROR, "cannot load xcursor theme");
	}

#if HAVE_XWAYLAND
	struct wlr_xcursor *xcursor;
	xcursor = wlr_xcursor_manager_get_xcursor(server->seat.xcursor_manager,
						  XCURSOR_DEFAULT, 1);
	if (xcursor) {
		struct wlr_xcursor_image *image = xcursor->images[0];
		wlr_xwayland_set_cursor(server->xwayland, image->buffer,
					image->width * 4, image->width,
					image->height, image->hotspot_x,
					image->hotspot_y);
	}
#endif

	/* used when handling SIGHUP */
	g_server = server;
}

void
server_start(struct server *server)
{
	/* Add a Unix socket to the Wayland display. */
	const char *socket = wl_display_add_socket_auto(server->wl_display);
	if (!socket) {
		wlr_log_errno(WLR_ERROR, "unable to open wayland socket");
		exit(EXIT_FAILURE);
	}

	/*
	 * Start the backend. This will enumerate outputs and inputs, become
	 * the DRM master, etc
	 */
	if (!wlr_backend_start(server->backend)) {
		wlr_log(WLR_ERROR, "unable to start the wlroots backend");
		exit(EXIT_FAILURE);
	}

	setenv("WAYLAND_DISPLAY", socket, true);
	if (setenv("WAYLAND_DISPLAY", socket, true) < 0) {
		wlr_log_errno(WLR_ERROR, "unable to set WAYLAND_DISPLAY");
	} else {
		wlr_log(WLR_DEBUG, "WAYLAND_DISPLAY=%s", socket);
	}

#if HAVE_XWAYLAND
	wlr_xwayland_set_seat(server->xwayland, server->seat.seat);
#endif
}

void
server_finish(struct server *server)
{
#if HAVE_XWAYLAND
	wlr_xwayland_destroy(server->xwayland);
#endif
	if (sighup_source) {
		wl_event_source_remove(sighup_source);
	}
	wl_display_destroy_clients(server->wl_display);

	seat_finish(server);
	wlr_output_layout_destroy(server->output_layout);

	wl_display_destroy(server->wl_display);
}
