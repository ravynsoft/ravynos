#include "waybox/output.h"

void output_frame_notify(struct wl_listener *listener, void *data) {
	/* This function is called every time an output is ready to display a frame,
	 * generally at the output's refresh rate (e.g. 60Hz). */
	struct wb_output *output = wl_container_of(listener, output, frame);
	struct wlr_scene *scene = output->server->scene;

	struct wlr_scene_output *scene_output = wlr_scene_get_scene_output(
		scene, output->wlr_output);

#if WLR_CHECK_VERSION(0, 16, 0)
	wlr_output_layout_get_box(output->server->output_layout,
			output->wlr_output, &output->geometry);
#else
	output->geometry = *wlr_output_layout_get_box(
			output->server->output_layout, output->wlr_output);
#endif
	/* Update the background for the current output size. */
	wlr_scene_rect_set_size(output->background,
			output->geometry.width, output->geometry.height);

	/* Render the scene if needed and commit the output */
	wlr_scene_output_commit(scene_output);

	/* This lets the client know that we've displayed that frame and it can
	 * prepare another one now if it likes. */
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	wlr_scene_output_send_frame_done(scene_output, &now);
}

void output_destroy_notify(struct wl_listener *listener, void *data) {
       	struct wb_output *output = wl_container_of(listener, output, destroy);

	wl_list_remove(&output->destroy.link);
	wl_list_remove(&output->frame.link);
	wl_list_remove(&output->link);

	/* Frees the layers */
	size_t num_layers = sizeof(output->layers) / sizeof(struct wlr_scene_node *);
	for (size_t i = 0; i < num_layers; i++) {
		struct wlr_scene_node *node =
			((struct wlr_scene_node **) &output->layers)[i];
		wlr_scene_node_destroy(node);
	}

	free(output);
}

void new_output_notify(struct wl_listener *listener, void *data) {
	struct wb_server *server = wl_container_of(
			listener, server, new_output
			);
	struct wlr_output *wlr_output = data;
	wlr_log(WLR_INFO, "%s: %s", _("New output device detected"), wlr_output->name);

	/* Configures the output created by the backend to use our allocator
         * and our renderer */
	wlr_output_init_render(wlr_output, server->allocator, server->renderer);

	if (!wl_list_empty(&wlr_output->modes)) {
		struct wlr_output_mode *mode = wlr_output_preferred_mode(wlr_output);
		wlr_output_set_mode(wlr_output, mode);
		wlr_output_enable(wlr_output, true);

		if (!wlr_output_commit(wlr_output)) {
			wlr_log_errno(WLR_ERROR, "%s", _("Couldn't commit pending frame to output"));
			return;
		}
	}

	struct wb_output *output = calloc(1, sizeof(struct wb_output));
	output->server = server;
	output->wlr_output = wlr_output;
	wlr_output->data = output;

	/* Set the background color */
	float color[4] = {0.1875, 0.1875, 0.1875, 1.0};
	output->background = wlr_scene_rect_create(&server->scene->node, 0, 0, color);
	wlr_scene_node_lower_to_bottom(&output->background->node);

	/* Initializes the layers */
	size_t num_layers = sizeof(output->layers) / sizeof(struct wlr_scene_node *);
	for (size_t i = 0; i < num_layers; i++) {
		((struct wlr_scene_node **) &output->layers)[i] =
			&wlr_scene_tree_create(&server->scene->node)->node;
	}

	wl_list_insert(&server->outputs, &output->link);

	output->destroy.notify = output_destroy_notify;
	wl_signal_add(&wlr_output->events.destroy, &output->destroy);
	output->frame.notify = output_frame_notify;
	wl_signal_add(&wlr_output->events.frame, &output->frame);

	/* Adds this to the output layout. The add_auto function arranges outputs
	 * from left-to-right in the order they appear. A more sophisticated
	 * compositor would let the user configure the arrangement of outputs in the
	 * layout.
	 *
	 * The output layout utility automatically adds a wl_output global to the
	 * display, which Wayland clients can see to find out information about the
	 * output (such as DPI, scale factor, manufacturer, etc).
	 */
	wlr_output_layout_add_auto(server->output_layout, wlr_output);
}
