/*
 * Copyright © 2017 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "config.h"

#include "evdev-fallback.h"

/* Debounce cases to handle
     P ... button press
     R ... button release
     ---|  timeout duration

     'normal' .... event sent when it happens
     'filtered' .. event is not sent (but may be sent later)
     'delayed' ... event is sent with wall-clock delay

   1) P---| R		P normal, R normal
   2) R---| P		R normal, P normal
   3) P---R--| P	P normal, R filtered, delayed, P normal
   4) R---P--| R	R normal, P filtered, delayed, R normal
   4.1) P---| R--P--|	P normal, R filtered
   5) P--R-P-| R	P normal, R filtered, P filtered, R normal
   6) R--P-R-| P	R normal, P filtered, R filtered, P normal
   7) P--R--|
          ---P-|	P normal, R filtered, P filtered
   8) R--P--|
          ---R-|	R normal, P filtered, R filtered

   1, 2 are the normal click cases without debouncing taking effect
   3, 4 are fast clicks where the second event is delivered with a delay
   5, 6 are contact bounces, fast
   7, 8 are contact bounces, slow

   4.1 is a special case with the same event sequence as 4 but we want to
   filter the *release* event out, it's a button losing contact while being
   held down.

   7 and 8 are cases where the first event happens within the first timeout
   but the second event is outside that timeout (but within the timeout of
   the second event). These cases are handled by restarting the timer on every
   event that could be part of a bouncing sequence, which makes these cases
   indistinguishable from 5 and 6.
*/

enum debounce_event {
	DEBOUNCE_EVENT_PRESS = 50,
	DEBOUNCE_EVENT_RELEASE,
	DEBOUNCE_EVENT_TIMEOUT,
	DEBOUNCE_EVENT_TIMEOUT_SHORT,
	DEBOUNCE_EVENT_OTHERBUTTON,
};

static inline const char *
debounce_state_to_str(enum debounce_state state)
{
	switch(state) {
	CASE_RETURN_STRING(DEBOUNCE_STATE_IS_UP);
	CASE_RETURN_STRING(DEBOUNCE_STATE_IS_DOWN);
	CASE_RETURN_STRING(DEBOUNCE_STATE_IS_DOWN_WAITING);
	CASE_RETURN_STRING(DEBOUNCE_STATE_IS_UP_DELAYING);
	CASE_RETURN_STRING(DEBOUNCE_STATE_IS_UP_DELAYING_SPURIOUS);
	CASE_RETURN_STRING(DEBOUNCE_STATE_IS_UP_DETECTING_SPURIOUS);
	CASE_RETURN_STRING(DEBOUNCE_STATE_IS_DOWN_DETECTING_SPURIOUS);
	CASE_RETURN_STRING(DEBOUNCE_STATE_IS_UP_WAITING);
	CASE_RETURN_STRING(DEBOUNCE_STATE_IS_DOWN_DELAYING);
	CASE_RETURN_STRING(DEBOUNCE_STATE_DISABLED);
	}

	return NULL;
}

static inline const char*
debounce_event_to_str(enum debounce_event event)
{
	switch(event) {
	CASE_RETURN_STRING(DEBOUNCE_EVENT_PRESS);
	CASE_RETURN_STRING(DEBOUNCE_EVENT_RELEASE);
	CASE_RETURN_STRING(DEBOUNCE_EVENT_TIMEOUT);
	CASE_RETURN_STRING(DEBOUNCE_EVENT_TIMEOUT_SHORT);
	CASE_RETURN_STRING(DEBOUNCE_EVENT_OTHERBUTTON);
	}
	return NULL;
}

static inline void
log_debounce_bug(struct fallback_dispatch *fallback, enum debounce_event event)
{
	evdev_log_bug_libinput(fallback->device,
			       "invalid debounce event %s in state %s\n",
			       debounce_event_to_str(event),
			       debounce_state_to_str(fallback->debounce.state));

}

static inline void
debounce_set_state(struct fallback_dispatch *fallback,
		   enum debounce_state new_state)
{
	assert(new_state >= DEBOUNCE_STATE_IS_UP &&
	       new_state <= DEBOUNCE_STATE_IS_DOWN_DELAYING);

	fallback->debounce.state = new_state;
}

static inline void
debounce_set_timer(struct fallback_dispatch *fallback,
		   uint64_t time)
{
	const int DEBOUNCE_TIMEOUT_BOUNCE = ms2us(25);

	libinput_timer_set(&fallback->debounce.timer,
			   time + DEBOUNCE_TIMEOUT_BOUNCE);
}

static inline void
debounce_set_timer_short(struct fallback_dispatch *fallback,
			 uint64_t time)
{
	const int DEBOUNCE_TIMEOUT_SPURIOUS = ms2us(12);

	libinput_timer_set(&fallback->debounce.timer_short,
			   time + DEBOUNCE_TIMEOUT_SPURIOUS);
}

static inline void
debounce_cancel_timer(struct fallback_dispatch *fallback)
{
	libinput_timer_cancel(&fallback->debounce.timer);
}

static inline void
debounce_cancel_timer_short(struct fallback_dispatch *fallback)
{
	libinput_timer_cancel(&fallback->debounce.timer_short);
}

static inline void
debounce_enable_spurious(struct fallback_dispatch *fallback)
{
	if (fallback->debounce.spurious_enabled)
		evdev_log_bug_libinput(fallback->device,
				       "tried to enable spurious debouncing twice\n");

	fallback->debounce.spurious_enabled = true;
	evdev_log_info(fallback->device,
		       "Enabling spurious button debouncing, "
		       "see %s/button-debouncing.html for details\n",
		       HTTP_DOC_LINK);
}

static void
debounce_notify_button(struct fallback_dispatch *fallback,
		       enum libinput_button_state state)
{
	struct evdev_device *device = fallback->device;
	unsigned int code = fallback->debounce.button_code;
	uint64_t time = fallback->debounce.button_time;

	code = evdev_to_left_handed(device, code);

	fallback_notify_physical_button(fallback, device, time, code, state);
}

static void
debounce_is_up_handle_event(struct fallback_dispatch *fallback, enum debounce_event event, uint64_t time)
{
	switch (event) {
	case DEBOUNCE_EVENT_PRESS:
		fallback->debounce.button_time = time;
		debounce_set_timer(fallback, time);
		debounce_set_state(fallback, DEBOUNCE_STATE_IS_DOWN_WAITING);
		debounce_notify_button(fallback,
				       LIBINPUT_BUTTON_STATE_PRESSED);
		break;
	case DEBOUNCE_EVENT_RELEASE:
	case DEBOUNCE_EVENT_TIMEOUT:
	case DEBOUNCE_EVENT_TIMEOUT_SHORT:
		log_debounce_bug(fallback, event);
		break;
	case DEBOUNCE_EVENT_OTHERBUTTON:
		break;
	}
}

static void
debounce_is_down_handle_event(struct fallback_dispatch *fallback, enum debounce_event event, uint64_t time)
{
	switch (event) {
	case DEBOUNCE_EVENT_PRESS:
		log_debounce_bug(fallback, event);
		break;
	case DEBOUNCE_EVENT_RELEASE:
		fallback->debounce.button_time = time;
		debounce_set_timer(fallback, time);
		debounce_set_timer_short(fallback, time);
		if (fallback->debounce.spurious_enabled) {
			debounce_set_state(fallback, DEBOUNCE_STATE_IS_UP_DELAYING_SPURIOUS);
		} else {
			debounce_set_state(fallback, DEBOUNCE_STATE_IS_UP_DETECTING_SPURIOUS);
			debounce_notify_button(fallback,
					       LIBINPUT_BUTTON_STATE_RELEASED);
		}
		break;
	case DEBOUNCE_EVENT_TIMEOUT:
	case DEBOUNCE_EVENT_TIMEOUT_SHORT:
		log_debounce_bug(fallback, event);
		break;
	case DEBOUNCE_EVENT_OTHERBUTTON:
		break;
	}
}

static void
debounce_is_down_waiting_handle_event(struct fallback_dispatch *fallback, enum debounce_event event, uint64_t time)
{
	switch (event) {
	case DEBOUNCE_EVENT_PRESS:
		log_debounce_bug(fallback, event);
		break;
	case DEBOUNCE_EVENT_RELEASE:
		debounce_set_timer(fallback, time);
		debounce_set_state(fallback, DEBOUNCE_STATE_IS_UP_DELAYING);
		/* Note: In the debouncing RPR case, we use the last
		 * release's time stamp */
		fallback->debounce.button_time = time;
		break;
	case DEBOUNCE_EVENT_TIMEOUT:
		debounce_set_state(fallback, DEBOUNCE_STATE_IS_DOWN);
		break;
	case DEBOUNCE_EVENT_TIMEOUT_SHORT:
		log_debounce_bug(fallback, event);
		break;
	case DEBOUNCE_EVENT_OTHERBUTTON:
		debounce_set_state(fallback, DEBOUNCE_STATE_IS_DOWN);
		break;
	}
}

static void
debounce_is_up_delaying_handle_event(struct fallback_dispatch *fallback, enum debounce_event event, uint64_t time)
{
	switch (event) {
	case DEBOUNCE_EVENT_PRESS:
		debounce_set_timer(fallback, time);
		debounce_set_state(fallback, DEBOUNCE_STATE_IS_DOWN_WAITING);
		break;
	case DEBOUNCE_EVENT_RELEASE:
	case DEBOUNCE_EVENT_TIMEOUT_SHORT:
		log_debounce_bug(fallback, event);
		break;
	case DEBOUNCE_EVENT_TIMEOUT:
	case DEBOUNCE_EVENT_OTHERBUTTON:
		debounce_set_state(fallback, DEBOUNCE_STATE_IS_UP);
		debounce_notify_button(fallback,
				       LIBINPUT_BUTTON_STATE_RELEASED);
		break;
	}
}

static void
debounce_is_up_delaying_spurious_handle_event(struct fallback_dispatch *fallback, enum debounce_event event, uint64_t time)
{
	switch (event) {
	case DEBOUNCE_EVENT_PRESS:
		debounce_set_state(fallback, DEBOUNCE_STATE_IS_DOWN);
		debounce_cancel_timer(fallback);
		debounce_cancel_timer_short(fallback);
		break;
	case DEBOUNCE_EVENT_RELEASE:
	case DEBOUNCE_EVENT_TIMEOUT:
		log_debounce_bug(fallback, event);
		break;
	case DEBOUNCE_EVENT_TIMEOUT_SHORT:
		debounce_set_state(fallback, DEBOUNCE_STATE_IS_UP_WAITING);
		debounce_notify_button(fallback,
				       LIBINPUT_BUTTON_STATE_RELEASED);
		break;
	case DEBOUNCE_EVENT_OTHERBUTTON:
		debounce_set_state(fallback, DEBOUNCE_STATE_IS_UP);
		debounce_notify_button(fallback,
				       LIBINPUT_BUTTON_STATE_RELEASED);
		break;
	}
}

static void
debounce_is_up_detecting_spurious_handle_event(struct fallback_dispatch *fallback, enum debounce_event event, uint64_t time)
{
	switch (event) {
	case DEBOUNCE_EVENT_PRESS:
		debounce_set_timer(fallback, time);
		debounce_set_timer_short(fallback, time);
		/* Note: in a bouncing PRP case, we use the last press
		 * event time */
		fallback->debounce.button_time = time;
		debounce_set_state(fallback, DEBOUNCE_STATE_IS_DOWN_DETECTING_SPURIOUS);
		break;
	case DEBOUNCE_EVENT_RELEASE:
		log_debounce_bug(fallback, event);
		break;
	case DEBOUNCE_EVENT_TIMEOUT:
		debounce_set_state(fallback, DEBOUNCE_STATE_IS_UP);
		break;
	case DEBOUNCE_EVENT_TIMEOUT_SHORT:
		debounce_set_state(fallback, DEBOUNCE_STATE_IS_UP_WAITING);
		break;
	case DEBOUNCE_EVENT_OTHERBUTTON:
		debounce_set_state(fallback, DEBOUNCE_STATE_IS_UP);
		break;
	}
}

static void
debounce_is_down_detecting_spurious_handle_event(struct fallback_dispatch *fallback, enum debounce_event event, uint64_t time)
{
	switch (event) {
	case DEBOUNCE_EVENT_PRESS:
		log_debounce_bug(fallback, event);
		break;
	case DEBOUNCE_EVENT_RELEASE:
		debounce_set_timer(fallback, time);
		debounce_set_timer_short(fallback, time);
		debounce_set_state(fallback, DEBOUNCE_STATE_IS_UP_DETECTING_SPURIOUS);
		break;
	case DEBOUNCE_EVENT_TIMEOUT_SHORT:
		debounce_cancel_timer(fallback);
		debounce_set_state(fallback, DEBOUNCE_STATE_IS_DOWN);
		debounce_enable_spurious(fallback);
		debounce_notify_button(fallback,
				       LIBINPUT_BUTTON_STATE_PRESSED);
		break;
	case DEBOUNCE_EVENT_TIMEOUT:
	case DEBOUNCE_EVENT_OTHERBUTTON:
		debounce_set_state(fallback, DEBOUNCE_STATE_IS_DOWN);
		debounce_notify_button(fallback,
				       LIBINPUT_BUTTON_STATE_PRESSED);
		break;
	}
}

static void
debounce_is_up_waiting_handle_event(struct fallback_dispatch *fallback, enum debounce_event event, uint64_t time)
{
	switch (event) {
	case DEBOUNCE_EVENT_PRESS:
		debounce_set_timer(fallback, time);
		/* Note: in a debouncing PRP case, we use the last press'
		 * time */
		fallback->debounce.button_time = time;
		debounce_set_state(fallback, DEBOUNCE_STATE_IS_DOWN_DELAYING);
		break;
	case DEBOUNCE_EVENT_RELEASE:
	case DEBOUNCE_EVENT_TIMEOUT_SHORT:
		log_debounce_bug(fallback, event);
		break;
	case DEBOUNCE_EVENT_TIMEOUT:
	case DEBOUNCE_EVENT_OTHERBUTTON:
		debounce_set_state(fallback, DEBOUNCE_STATE_IS_UP);
		break;
	}
}

static void
debounce_is_down_delaying_handle_event(struct fallback_dispatch *fallback, enum debounce_event event, uint64_t time)
{
	switch (event) {
	case DEBOUNCE_EVENT_PRESS:
		log_debounce_bug(fallback, event);
		break;
	case DEBOUNCE_EVENT_RELEASE:
		debounce_set_timer(fallback, time);
		debounce_set_state(fallback, DEBOUNCE_STATE_IS_UP_WAITING);
		break;
	case DEBOUNCE_EVENT_TIMEOUT_SHORT:
		log_debounce_bug(fallback, event);
		break;
	case DEBOUNCE_EVENT_TIMEOUT:
	case DEBOUNCE_EVENT_OTHERBUTTON:
		debounce_set_state(fallback, DEBOUNCE_STATE_IS_DOWN);
		debounce_notify_button(fallback,
				       LIBINPUT_BUTTON_STATE_PRESSED);
		break;
	}
}

static void
debounce_disabled_handle_event(struct fallback_dispatch *fallback,
			enum debounce_event event,
			uint64_t time)
{
	switch (event) {
	case DEBOUNCE_EVENT_PRESS:
		fallback->debounce.button_time = time;
		debounce_notify_button(fallback,
				       LIBINPUT_BUTTON_STATE_PRESSED);
		break;
	case DEBOUNCE_EVENT_RELEASE:
		fallback->debounce.button_time = time;
		debounce_notify_button(fallback,
				       LIBINPUT_BUTTON_STATE_RELEASED);
		break;
	case DEBOUNCE_EVENT_TIMEOUT_SHORT:
	case DEBOUNCE_EVENT_TIMEOUT:
		log_debounce_bug(fallback, event);
		break;
	case DEBOUNCE_EVENT_OTHERBUTTON:
		break;
	}
}

static void
debounce_handle_event(struct fallback_dispatch *fallback,
		      enum debounce_event event,
		      uint64_t time)
{
	enum debounce_state current = fallback->debounce.state;

	if (event == DEBOUNCE_EVENT_OTHERBUTTON) {
		debounce_cancel_timer(fallback);
		debounce_cancel_timer_short(fallback);
	}

	switch(current) {
	case DEBOUNCE_STATE_IS_UP:
		debounce_is_up_handle_event(fallback, event, time);
		break;
	case DEBOUNCE_STATE_IS_DOWN:
		debounce_is_down_handle_event(fallback, event, time);
		break;
	case DEBOUNCE_STATE_IS_DOWN_WAITING:
		debounce_is_down_waiting_handle_event(fallback, event, time);
		break;
	case DEBOUNCE_STATE_IS_UP_DELAYING:
		debounce_is_up_delaying_handle_event(fallback, event, time);
		break;
	case DEBOUNCE_STATE_IS_UP_DELAYING_SPURIOUS:
		debounce_is_up_delaying_spurious_handle_event(fallback, event, time);
		break;
	case DEBOUNCE_STATE_IS_UP_DETECTING_SPURIOUS:
		debounce_is_up_detecting_spurious_handle_event(fallback, event, time);
		break;
	case DEBOUNCE_STATE_IS_DOWN_DETECTING_SPURIOUS:
		debounce_is_down_detecting_spurious_handle_event(fallback, event, time);
		break;
	case DEBOUNCE_STATE_IS_UP_WAITING:
		debounce_is_up_waiting_handle_event(fallback, event, time);
		break;
	case DEBOUNCE_STATE_IS_DOWN_DELAYING:
		debounce_is_down_delaying_handle_event(fallback, event, time);
		break;
	case DEBOUNCE_STATE_DISABLED:
		debounce_disabled_handle_event(fallback, event, time);
		break;
	}

	evdev_log_debug(fallback->device,
			"debounce state: %s → %s → %s\n",
			debounce_state_to_str(current),
			debounce_event_to_str(event),
			debounce_state_to_str(fallback->debounce.state));
}

void
fallback_debounce_handle_state(struct fallback_dispatch *dispatch,
			       uint64_t time)
{
	unsigned int changed[16] = {0}; /* event codes of changed buttons */
	size_t nchanged = 0;
	bool flushed = false;

	for (unsigned int code = 0; code <= KEY_MAX; code++) {
		if (get_key_type(code) != KEY_TYPE_BUTTON)
			continue;

		if (hw_key_has_changed(dispatch, code))
			changed[nchanged++] = code;

		/* If you manage to press more than 16 buttons in the same
		 * frame, we just quietly ignore the rest of them */
		if (nchanged == ARRAY_LENGTH(changed))
			break;
	}

	/* If we have more than one button this frame or a different button,
	 * flush the state machine with otherbutton */
	if (nchanged > 1 ||
	    changed[0] != dispatch->debounce.button_code) {
		debounce_handle_event(dispatch,
				      DEBOUNCE_EVENT_OTHERBUTTON,
				      time);
		flushed = true;
	}

	/* The state machine has some pre-conditions:
	 * - the IS_DOWN and IS_UP states are neutral entry states without
	 *   any timeouts
	 * - a OTHERBUTTON event always flushes the state to IS_DOWN or
	 *   IS_UP
	 */

	for (size_t i = 0; i < nchanged; i++) {
		bool is_down = hw_is_key_down(dispatch, changed[i]);

		if (flushed &&
		    dispatch->debounce.state != DEBOUNCE_STATE_DISABLED) {
			debounce_set_state(dispatch,
					   !is_down ?
						   DEBOUNCE_STATE_IS_DOWN :
						   DEBOUNCE_STATE_IS_UP);
			flushed = false;
		}

		dispatch->debounce.button_code = changed[i];
		debounce_handle_event(dispatch,
				      is_down ?
					      DEBOUNCE_EVENT_PRESS :
					      DEBOUNCE_EVENT_RELEASE,
				      time);

		/* if we have more than one event, we flush the state
		 * machine immediately after the event itself */
		if (nchanged > 1) {
			debounce_handle_event(dispatch,
					      DEBOUNCE_EVENT_OTHERBUTTON,
					      time);
			flushed = true;
		}

	}
}

static void
debounce_timeout(uint64_t now, void *data)
{
	struct evdev_device *device = data;
	struct fallback_dispatch *dispatch =
		fallback_dispatch(device->dispatch);

	debounce_handle_event(dispatch, DEBOUNCE_EVENT_TIMEOUT, now);
}

static void
debounce_timeout_short(uint64_t now, void *data)
{
	struct evdev_device *device = data;
	struct fallback_dispatch *dispatch =
		fallback_dispatch(device->dispatch);

	debounce_handle_event(dispatch, DEBOUNCE_EVENT_TIMEOUT_SHORT, now);
}

void
fallback_init_debounce(struct fallback_dispatch *dispatch)
{
	struct evdev_device *device = dispatch->device;
	char timer_name[64];

	if (evdev_device_has_model_quirk(device, QUIRK_MODEL_BOUNCING_KEYS)) {
		dispatch->debounce.state = DEBOUNCE_STATE_DISABLED;
		return;
	}

	dispatch->debounce.state = DEBOUNCE_STATE_IS_UP;

	snprintf(timer_name,
		 sizeof(timer_name),
		 "%s debounce short",
		 evdev_device_get_sysname(device));
	libinput_timer_init(&dispatch->debounce.timer_short,
			    evdev_libinput_context(device),
			    timer_name,
			    debounce_timeout_short,
			    device);

	snprintf(timer_name,
		 sizeof(timer_name),
		 "%s debounce",
		 evdev_device_get_sysname(device));
	libinput_timer_init(&dispatch->debounce.timer,
			    evdev_libinput_context(device),
			    timer_name,
			    debounce_timeout,
			    device);
}
