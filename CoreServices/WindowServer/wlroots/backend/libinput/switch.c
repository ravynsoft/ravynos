#include <assert.h>
#include <libinput.h>
#include <stdlib.h>
#include <wlr/backend/session.h>
#include <wlr/interfaces/wlr_switch.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/util/log.h>
#include "backend/libinput.h"
#include "util/signal.h"

struct wlr_switch *create_libinput_switch(
		struct libinput_device *libinput_dev) {
	assert(libinput_dev);
	struct wlr_switch *wlr_switch = calloc(1, sizeof(struct wlr_switch));
	if (!wlr_switch) {
		wlr_log(WLR_ERROR, "Unable to allocate wlr_switch");
		return NULL;
	}
	wlr_switch_init(wlr_switch, NULL);
	wlr_log(WLR_DEBUG, "Created switch for device %s", libinput_device_get_name(libinput_dev));
	return wlr_switch;
}

void handle_switch_toggle(struct libinput_event *event,
		struct libinput_device *libinput_dev) {
	struct wlr_input_device *wlr_dev =
		get_appropriate_device(WLR_INPUT_DEVICE_SWITCH, libinput_dev);
	if (!wlr_dev) {
		wlr_log(WLR_DEBUG, "Got a switch event for a device with no switch?");
		return;
	}
	struct libinput_event_switch *sevent =
		libinput_event_get_switch_event	(event);
	struct wlr_event_switch_toggle wlr_event = { 0 };
	wlr_event.device = wlr_dev;
	switch (libinput_event_switch_get_switch(sevent)) {
	case LIBINPUT_SWITCH_LID:
		wlr_event.switch_type = WLR_SWITCH_TYPE_LID;
		break;
	case LIBINPUT_SWITCH_TABLET_MODE:
		wlr_event.switch_type = WLR_SWITCH_TYPE_TABLET_MODE;
		break;
	}
	switch (libinput_event_switch_get_switch_state(sevent)) {
	case LIBINPUT_SWITCH_STATE_OFF:
		wlr_event.switch_state = WLR_SWITCH_STATE_OFF;
		break;
	case LIBINPUT_SWITCH_STATE_ON:
		wlr_event.switch_state = WLR_SWITCH_STATE_ON;
		break;
	}
	wlr_event.time_msec =
		usec_to_msec(libinput_event_switch_get_time_usec(sevent));
	wlr_signal_emit_safe(&wlr_dev->switch_device->events.toggle, &wlr_event);
}
