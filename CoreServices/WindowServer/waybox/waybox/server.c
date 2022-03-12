#include "waybox/server.h"
#include "waybox/xdg_shell.h"

bool wb_create_backend(struct wb_server* server) {
	/* The Wayland display is managed by libwayland. It handles accepting
	 * clients from the Unix socket, manging Wayland globals, and so on. */
	server->wl_display = wl_display_create();
	if (server->wl_display == NULL) {
		wlr_log(WLR_ERROR, "%s", _("Failed to connect to a Wayland display"));
		return false;
	}

	/* The backend is a wlroots feature which abstracts the underlying input and
	 * output hardware. The autocreate option will choose the most suitable
	 * backend based on the current environment, such as opening an X11 window
	 * if an X11 server is running. */
	server->backend = wlr_backend_autocreate(server->wl_display);
	if (server->backend == NULL) {
		return false;
	}

	/* Autocreates a renderer, either Pixman, GLES2 or Vulkan for us. The user
         * can also specify a renderer using the WLR_RENDERER env var.
         * The renderer is responsible for defining the various pixel formats it
         * supports for shared memory, this configures that for clients. */
	server->renderer = wlr_renderer_autocreate(server->backend);
	wlr_renderer_init_wl_display(server->renderer, server->wl_display);

        /* Autocreates an allocator for us.
         * The allocator is the bridge between the renderer and the backend. It
         * handles the buffer creation, allowing wlroots to render onto the
         * screen */
        server->allocator = wlr_allocator_autocreate(server->backend, server->renderer);

	server->compositor = wlr_compositor_create(server->wl_display,
			server->renderer);
#if WLR_CHECK_VERSION(0, 16, 0)
	server->subcompositor = wlr_subcompositor_create(server->wl_display);
#endif
	server->output_layout = wlr_output_layout_create();
	server->seat = wb_seat_create(server);
	server->cursor = wb_cursor_create(server);

	server->output_manager = wlr_xdg_output_manager_v1_create(server->wl_display,
				server->output_layout);

	return true;
}

bool wb_start_server(struct wb_server* server) {
	init_config(server);
	wl_list_init(&server->outputs);

	server->new_output.notify = new_output_notify;
	wl_signal_add(&server->backend->events.new_output, &server->new_output);

	/* Create a scene graph. This is a wlroots abstraction that handles all
	 * rendering and damage tracking. All the compositor author needs to do
	 * is add things that should be rendered to the scene graph at the proper
	 * positions and then call wlr_scene_output_commit() to render a frame if
	 * necessary.
	 */
	server->scene = wlr_scene_create();
	wlr_scene_attach_output_layout(server->scene, server->output_layout);

	const char *socket = wl_display_add_socket_auto(server->wl_display);
	if (!socket) {
		wlr_backend_destroy(server->backend);
		return false;
	}

	if (!wlr_backend_start(server->backend)) {
		wlr_log(WLR_ERROR, "%s", _("Failed to start backend"));
		wlr_backend_destroy(server->backend);
		wl_display_destroy(server->wl_display);
		return false;
	}

	wlr_log(WLR_INFO, "%s: WAYLAND_DISPLAY=%s", _("Running Wayland compositor on Wayland display"), socket);
	setenv("WAYLAND_DISPLAY", socket, true);

	wlr_gamma_control_manager_v1_create(server->wl_display);
	wlr_screencopy_manager_v1_create(server->wl_display);
	wlr_idle_create(server->wl_display);

	wlr_data_device_manager_create(server->wl_display);
	wl_list_init(&server->views);
	init_xdg_decoration(server);
	init_layer_shell(server);

	/* Set up the xdg-shell. The xdg-shell is a Wayland protocol which is used
	 * for application windows. For more detail on shells, refer to Drew
	 * DeVault's article:
	 *
	 * https://drewdevault.com/2018/07/29/Wayland-shells.html
	 */
	init_xdg_shell(server);

	return true;
}

bool wb_terminate(struct wb_server* server) {
	wb_cursor_destroy(server->cursor);
	wl_list_remove(&server->new_xdg_decoration.link); /* wb_decoration_destroy */
	wb_seat_destroy(server->seat);
	deinit_config(server->config);
	wl_display_destroy_clients(server->wl_display);
	wl_display_destroy(server->wl_display);
	wlr_output_layout_destroy(server->output_layout);

	wlr_log(WLR_INFO, "%s", _("Display destroyed"));

	return true;
}
