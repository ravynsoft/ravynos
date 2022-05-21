#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_surface.h>
#include <wlr/types/wlr_xdg_activation_v1.h>
#include <wlr/util/log.h>
#include "util/signal.h"
#include "util/token.h"
#include "xdg-activation-v1-protocol.h"

#define XDG_ACTIVATION_V1_VERSION 1

static const struct xdg_activation_token_v1_interface token_impl;

static struct wlr_xdg_activation_token_v1 *token_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource,
		&xdg_activation_token_v1_interface, &token_impl));
	return wl_resource_get_user_data(resource);
}

void wlr_xdg_activation_token_v1_destroy(
		struct wlr_xdg_activation_token_v1 *token) {
	if (token == NULL) {
		return;
	}
	if (token->resource != NULL) {
		wl_resource_set_user_data(token->resource, NULL); // make inert
	}
	if (token->timeout != NULL) {
		wl_event_source_remove(token->timeout);
	}

	wlr_signal_emit_safe(&token->events.destroy, NULL);

	wl_list_remove(&token->link);
	wl_list_remove(&token->seat_destroy.link);
	wl_list_remove(&token->surface_destroy.link);
	free(token->app_id);
	free(token->token);
	free(token);
}

static int token_handle_timeout(void *data) {
	struct wlr_xdg_activation_token_v1 *token = data;
	wlr_log(WLR_DEBUG, "Activation token '%s' has expired", token->token);
	wlr_xdg_activation_token_v1_destroy(token);
	return 0;
}

static void token_handle_resource_destroy(struct wl_resource *resource) {
	struct wlr_xdg_activation_token_v1 *token = token_from_resource(resource);
	wlr_xdg_activation_token_v1_destroy(token);
}

static void token_handle_destroy(struct wl_client *client,
		struct wl_resource *token_resource) {
	wl_resource_destroy(token_resource);
}

static bool token_init( struct wlr_xdg_activation_token_v1 *token) {
	char token_str[TOKEN_STRLEN + 1] = {0};
	if (!generate_token(token_str)) {
		return false;
	}

	token->token = strdup(token_str);
	if (token->token == NULL) {
		return false;
	}

	if (token->activation->token_timeout_msec > 0) {
		// Needs wayland > 1.19
		// struct wl_display *display = wl_global_get_display(activation->global);
		struct wl_display *display = token->activation->display;
		struct wl_event_loop *loop = wl_display_get_event_loop(display);
		token->timeout =
			wl_event_loop_add_timer(loop, token_handle_timeout, token);
		if (token->timeout == NULL) {
			return false;
		}
		wl_event_source_timer_update(token->timeout,
			token->activation->token_timeout_msec);
	}

	assert(wl_list_empty(&token->link));
	wl_list_insert(&token->activation->tokens, &token->link);
	return true;
}

static void token_handle_commit(struct wl_client *client,
		struct wl_resource *token_resource) {
	struct wlr_xdg_activation_token_v1 *token =
		token_from_resource(token_resource);
	if (token == NULL) {
		wl_resource_post_error(token_resource,
			XDG_ACTIVATION_TOKEN_V1_ERROR_ALREADY_USED,
			"The activation token has already been used");
		return;
	}

	// Make the token resource inert
	wl_resource_set_user_data(token->resource, NULL);
	token->resource = NULL;

	if (token->seat != NULL) {
		struct wlr_seat_client *seat_client =
			wlr_seat_client_for_wl_client(token->seat, client);
		if (seat_client == NULL ||
				!wlr_seat_client_validate_event_serial(seat_client, token->serial)) {
			wlr_log(WLR_DEBUG, "Rejecting token commit request: "
				"serial %"PRIu32" was never given to client", token->serial);
			goto error;
		}

		if (token->surface != NULL &&
				token->surface != token->seat->keyboard_state.focused_surface) {
			wlr_log(WLR_DEBUG, "Rejecting token commit request: "
				"surface doesn't have keyboard focus");
			goto error;
		}
	}

	if (!token_init(token)) {
		wl_client_post_no_memory(client);
		return;
	}

	xdg_activation_token_v1_send_done(token_resource, token->token);

	// TODO: consider emitting a new_token event

	return;

error:;
	// Here we send a generated token, but it's invalid and can't be used to
	// request activation.
	char token_str[TOKEN_STRLEN + 1] = {0};
	if (!generate_token(token_str)) {
		wl_client_post_no_memory(client);
		return;
	}

	xdg_activation_token_v1_send_done(token_resource, token_str);
	wlr_xdg_activation_token_v1_destroy(token);
}

static void token_handle_set_app_id(struct wl_client *client,
		struct wl_resource *token_resource, const char *app_id) {
	struct wlr_xdg_activation_token_v1 *token =
		token_from_resource(token_resource);
	if (token == NULL) {
		wl_resource_post_error(token_resource,
			XDG_ACTIVATION_TOKEN_V1_ERROR_ALREADY_USED,
			"The activation token has already been used");
		return;
	}

	free(token->app_id);
	token->app_id = strdup(app_id);
}

static void token_handle_seat_destroy(struct wl_listener *listener, void *data) {
	struct wlr_xdg_activation_token_v1 *token =
		wl_container_of(listener, token, seat_destroy);
	wl_list_remove(&token->seat_destroy.link);
	wl_list_init(&token->seat_destroy.link);
	token->serial = 0;
	token->seat = NULL;
}

static void token_handle_set_serial(struct wl_client *client,
		struct wl_resource *token_resource, uint32_t serial,
		struct wl_resource *seat_resource) {
	struct wlr_xdg_activation_token_v1 *token =
		token_from_resource(token_resource);
	if (token == NULL) {
		wl_resource_post_error(token_resource,
			XDG_ACTIVATION_TOKEN_V1_ERROR_ALREADY_USED,
			"The activation token has already been used");
		return;
	}

	struct wlr_seat_client *seat_client =
		wlr_seat_client_from_resource(seat_resource);
	if (seat_client == NULL) {
		wlr_log(WLR_DEBUG, "Rejecting token set_serial request: seat is inert");
		return;
	}

	token->seat = seat_client->seat;
	token->serial = serial;

	token->seat_destroy.notify = token_handle_seat_destroy;
	wl_list_remove(&token->seat_destroy.link);
	wl_signal_add(&token->seat->events.destroy, &token->seat_destroy);
}

static void token_handle_surface_destroy(struct wl_listener *listener, void *data) {
	struct wlr_xdg_activation_token_v1 *token =
		wl_container_of(listener, token, surface_destroy);
	wl_list_remove(&token->surface_destroy.link);
	wl_list_init(&token->surface_destroy.link);
	token->surface = NULL;
}

static void token_handle_set_surface(struct wl_client *client,
		struct wl_resource *token_resource,
		struct wl_resource *surface_resource) {
	struct wlr_xdg_activation_token_v1 *token =
		token_from_resource(token_resource);
	struct wlr_surface *surface = wlr_surface_from_resource(surface_resource);
	if (token == NULL) {
		wl_resource_post_error(token_resource,
			XDG_ACTIVATION_TOKEN_V1_ERROR_ALREADY_USED,
			"The activation token has already been used");
		return;
	}

	token->surface = surface;

	token->surface_destroy.notify = token_handle_surface_destroy;
	wl_list_remove(&token->surface_destroy.link);
	wl_signal_add(&surface->events.destroy, &token->surface_destroy);
}

static const struct xdg_activation_token_v1_interface token_impl = {
	.destroy = token_handle_destroy,
	.commit = token_handle_commit,
	.set_app_id = token_handle_set_app_id,
	.set_serial = token_handle_set_serial,
	.set_surface = token_handle_set_surface,
};

static const struct xdg_activation_v1_interface activation_impl;

static struct wlr_xdg_activation_v1 *activation_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource,
		&xdg_activation_v1_interface, &activation_impl));
	return wl_resource_get_user_data(resource);
}

static void activation_handle_destroy(struct wl_client *client,
		struct wl_resource *activation_resource) {
	wl_resource_destroy(activation_resource);
}

static void activation_handle_get_activation_token(struct wl_client *client,
		struct wl_resource *activation_resource, uint32_t id) {
	struct wlr_xdg_activation_v1 *activation =
		activation_from_resource(activation_resource);

	struct wlr_xdg_activation_token_v1 *token = calloc(1, sizeof(*token));
	if (token == NULL) {
		wl_client_post_no_memory(client);
		return;
	}
	wl_list_init(&token->link);
	wl_list_init(&token->seat_destroy.link);
	wl_list_init(&token->surface_destroy.link);
	wl_signal_init(&token->events.destroy);

	token->activation = activation;

	uint32_t version = wl_resource_get_version(activation_resource);
	token->resource = wl_resource_create(client,
		&xdg_activation_token_v1_interface, version, id);
	if (token->resource == NULL) {
		free(token);
		wl_client_post_no_memory(client);
		return;
	}
	wl_resource_set_implementation(token->resource, &token_impl, token,
		token_handle_resource_destroy);
}

static void activation_handle_activate(struct wl_client *client,
		struct wl_resource *activation_resource, const char *token_str,
		struct wl_resource *surface_resource) {
	struct wlr_xdg_activation_v1 *activation =
		activation_from_resource(activation_resource);
	struct wlr_surface *surface = wlr_surface_from_resource(surface_resource);

	struct wlr_xdg_activation_token_v1 *token;
	bool found = false;
	wl_list_for_each(token, &activation->tokens, link) {
		if (strcmp(token_str, token->token) == 0) {
			found = true;
			break;
		}
	}
	if (!found) {
		wlr_log(WLR_DEBUG, "Rejecting activate request: unknown token");
		return;
	}

	struct wlr_xdg_activation_v1_request_activate_event event = {
		.activation = activation,
		.token = token,
		.surface = surface,
	};
	wlr_signal_emit_safe(&activation->events.request_activate, &event);

	wlr_xdg_activation_token_v1_destroy(token);
}

static const struct xdg_activation_v1_interface activation_impl = {
	.destroy = activation_handle_destroy,
	.get_activation_token = activation_handle_get_activation_token,
	.activate = activation_handle_activate,
};

static void activation_bind(struct wl_client *client, void *data,
		uint32_t version, uint32_t id) {
	struct wlr_xdg_activation_v1 *activation = data;

	struct wl_resource *resource = wl_resource_create(client,
		&xdg_activation_v1_interface, version, id);
	if (resource == NULL) {
		wl_client_post_no_memory(client);
		return;
	}
	wl_resource_set_implementation(resource, &activation_impl, activation, NULL);
}

static void handle_display_destroy(struct wl_listener *listener, void *data) {
	struct wlr_xdg_activation_v1 *activation =
		wl_container_of(listener, activation, display_destroy);
	wlr_signal_emit_safe(&activation->events.destroy, NULL);

	struct wlr_xdg_activation_token_v1 *token, *token_tmp;
	wl_list_for_each_safe(token, token_tmp, &activation->tokens, link) {
		wlr_xdg_activation_token_v1_destroy(token);
	}

	wl_list_remove(&activation->display_destroy.link);
	wl_global_destroy(activation->global);
	free(activation);
}

struct wlr_xdg_activation_v1 *wlr_xdg_activation_v1_create(
		struct wl_display *display) {
	struct wlr_xdg_activation_v1 *activation = calloc(1, sizeof(*activation));
	if (activation == NULL) {
		return NULL;
	}

	activation->token_timeout_msec = 30000; // 30s
	wl_list_init(&activation->tokens);
	wl_signal_init(&activation->events.destroy);
	wl_signal_init(&activation->events.request_activate);

	activation->global = wl_global_create(display,
		&xdg_activation_v1_interface, XDG_ACTIVATION_V1_VERSION, activation,
		activation_bind);
	if (activation->global == NULL) {
		free(activation);
		return NULL;
	}

	activation->display = display;

	activation->display_destroy.notify = handle_display_destroy;
	wl_display_add_destroy_listener(display, &activation->display_destroy);

	return activation;
}

struct wlr_xdg_activation_token_v1 *wlr_xdg_activation_token_v1_create(
		struct wlr_xdg_activation_v1 *activation) {
	struct wlr_xdg_activation_token_v1 *token = calloc(1, sizeof(*token));
	if (token == NULL) {
		return NULL;
	}

	wl_list_init(&token->link);
	// Currently no way to set seat/surface
	wl_list_init(&token->seat_destroy.link);
	wl_list_init(&token->surface_destroy.link);
	wl_signal_init(&token->events.destroy);

	token->activation = activation;

	if (!token_init(token)) {
		wlr_xdg_activation_token_v1_destroy(token);
		return NULL;
	}

	return token;
}

struct wlr_xdg_activation_token_v1 *wlr_xdg_activation_v1_find_token(
		struct wlr_xdg_activation_v1 *activation, const char *token_str) {
	struct wlr_xdg_activation_token_v1 *token;
	wl_list_for_each(token, &activation->tokens, link) {
		if (strcmp(token_str, token->token) == 0) {
			return token;
		}
	}
	return NULL;
}

const char *wlr_xdg_activation_token_v1_get_name(
		struct wlr_xdg_activation_token_v1 *token) {
	return token->token;
}

struct wlr_xdg_activation_token_v1 *wlr_xdg_activation_v1_add_token(
		struct wlr_xdg_activation_v1 *activation, const char *token_str) {
	assert(token_str);

	struct wlr_xdg_activation_token_v1 *token = calloc(1, sizeof(*token));
	if (token == NULL) {
		return NULL;
	}
	wl_list_init(&token->link);
	wl_list_init(&token->seat_destroy.link);
	wl_list_init(&token->surface_destroy.link);

	token->activation = activation;
	token->token = strdup(token_str);

	wl_list_insert(&activation->tokens, &token->link);

	return token;
}
