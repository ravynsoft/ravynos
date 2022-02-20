/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_TYPES_WLR_OUTPUT_MANAGEMENT_V1_H
#define WLR_TYPES_WLR_OUTPUT_MANAGEMENT_V1_H

#include <stdbool.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_output.h>

struct wlr_output_manager_v1 {
	struct wl_display *display;
	struct wl_global *global;
	struct wl_list resources; // wl_resource_get_link

	struct wl_list heads; // wlr_output_head_v1::link
	uint32_t serial;
	bool current_configuration_dirty;

	struct {
		/**
		 * The `apply` and `test` events are emitted when a client requests a
		 * configuration to be applied or tested. The compositor should send
		 * feedback with `wlr_output_configuration_v1_send_succeeded` xor
		 * `wlr_output_configuration_v1_send_failed`.
		 *
		 * The compositor gains ownership over the configuration (passed as the
		 * event data). That is, the compositor is responsible for destroying
		 * the configuration.
		 */
		struct wl_signal apply; // wlr_output_configuration_v1
		struct wl_signal test; // wlr_output_configuration_v1

		struct wl_signal destroy;
	} events;

	struct wl_listener display_destroy;

	void *data;
};

struct wlr_output_head_v1_state {
	struct wlr_output *output;

	bool enabled;
	struct wlr_output_mode *mode;
	struct {
		int32_t width, height;
		int32_t refresh;
	} custom_mode;
	int32_t x, y;
	enum wl_output_transform transform;
	float scale;
};

struct wlr_output_head_v1 {
	struct wlr_output_head_v1_state state;
	struct wlr_output_manager_v1 *manager;
	struct wl_list link; // wlr_output_manager_v1::heads

	struct wl_list resources; // wl_resource_get_link
	struct wl_list mode_resources; // wl_resource_get_link

	struct wl_listener output_destroy;
};

struct wlr_output_configuration_v1 {
	struct wl_list heads; // wlr_output_configuration_head_v1::link

	// client state
	struct wlr_output_manager_v1 *manager;
	uint32_t serial;
	bool finalized; // client has requested to apply the config
	bool finished; // feedback has been sent by the compositor
	struct wl_resource *resource; // can be NULL if destroyed early
};

struct wlr_output_configuration_head_v1 {
	struct wlr_output_head_v1_state state;
	struct wlr_output_configuration_v1 *config;
	struct wl_list link; // wlr_output_configuration_v1::heads

	// client state
	struct wl_resource *resource; // can be NULL if finalized or disabled

	struct wl_listener output_destroy;
};

/**
 * Create a new output manager. The compositor is responsible for calling
 * `wlr_output_manager_v1_set_configuration` whenever the current output
 * configuration changes.
 */
struct wlr_output_manager_v1 *wlr_output_manager_v1_create(
	struct wl_display *display);
/**
 * Updates the output manager's current configuration. This will broadcast any
 * changes to all clients.
 *
 * This function takes ownership over `config`. That is, the compositor must not
 * access the configuration anymore.
 */
void wlr_output_manager_v1_set_configuration(
	struct wlr_output_manager_v1 *manager,
	struct wlr_output_configuration_v1 *config);

/**
 * Create a new, empty output configuration. Compositors should add current head
 * status with `wlr_output_configuration_head_v1_create`. They can then call
 * `wlr_output_manager_v1_set_configuration`.
 */
struct wlr_output_configuration_v1 *wlr_output_configuration_v1_create(void);
void wlr_output_configuration_v1_destroy(
	struct wlr_output_configuration_v1 *config);
/**
 * If the configuration comes from a client request, this sends positive
 * feedback to the client (configuration has been applied).
 */
void wlr_output_configuration_v1_send_succeeded(
	struct wlr_output_configuration_v1 *config);
/**
 * If the configuration comes from a client request, this sends negative
 * feedback to the client (configuration has not been applied).
 */
void wlr_output_configuration_v1_send_failed(
	struct wlr_output_configuration_v1 *config);

/**
 * Create a new configuration head for the given output. This adds the head to
 * the provided output configuration.
 *
 * The configuration head will be pre-filled with data from `output`. The
 * compositor should adjust this data according to its current internal state.
 */
struct wlr_output_configuration_head_v1 *
	wlr_output_configuration_head_v1_create(
	struct wlr_output_configuration_v1 *config, struct wlr_output *output);

#endif
