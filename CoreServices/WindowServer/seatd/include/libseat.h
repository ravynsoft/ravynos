#ifndef _LIBSEAT_H
#define _LIBSEAT_H

#include <stdarg.h>

/*
 * An opaque struct containing an opened seat, created by libseat_open_seat and
 * destroyed by libseat_close_seat.
 */
struct libseat;

/*
 * A seat event listener, given to libseat_open_seat.
 */
struct libseat_seat_listener {
	/*
	 * The seat has been enabled, and is now valid for use. Re-open all seat
	 * devices to ensure that they are operational, as existing fds may have
	 * had their functionality blocked or revoked.
	 *
	 * Must be non-NULL.
	 */
	void (*enable_seat)(struct libseat *seat, void *userdata);

	/*
	 * The seat has been disabled. This event signals that the application
	 * is going to lose its seat access. The event *must* be acknowledged
	 * with libseat_disable_seat shortly after receiving this event.
	 *
	 * If the recepient fails to acknowledge the event in time, seat devices
	 * may be forcibly revoked by the seat provider.
	 *
	 * Must be non-NULL.
	 */
	void (*disable_seat)(struct libseat *seat, void *userdata);
};

/*
 * Opens a seat, taking control of it if possible and returning a pointer to
 * the libseat instance. If LIBSEAT_BACKEND is set, the specified backend is
 * used. Otherwise, the first successful backend will be used.
 *
 * The seat listener specified is used to signal events on the seat, and must
 * be non-NULL. The userdata pointer will be provided in all calls to the seat
 * listener.
 *
 * The available backends, if enabled at compile-time, are: seatd, logind and
 * builtin.
 *
 * To use builtin, the process must have permission to open and use the seat's
 * devices at the time of the call. In the case of DRM devices, this includes
 * permission for drmSetMaster(3). These privileges can be dropped at any
 * point after the call.
 *
 * The returned pointer must be destroyed with libseat_close_seat.
 *
 * Returns a pointer to an opaque libseat struct on success. Returns NULL and
 * sets errno on error.
 */
struct libseat *libseat_open_seat(const struct libseat_seat_listener *listener, void *userdata);

/*
 * Disables a seat, used in response to a disable_seat event. After disabling
 * the seat, the seat devices must not be used until enable_seat is received,
 * and all requests on the seat will fail during this period.
 *
 * Returns 0 on success. -1 and sets errno on error.
 */
int libseat_disable_seat(struct libseat *seat);

/*
 * Closes the seat. This frees the libseat structure.
 *
 * Returns 0 on success. Returns -1 and sets errno on error.
 */
int libseat_close_seat(struct libseat *seat);

/*
 * Opens a device on the seat, returning its device ID and placing the fd in
 * the specified pointer.
 *
 * This will only succeed if the seat is active and the device is of a type
 * permitted for opening on the backend, such as drm and evdev.
 *
 * The device may be revoked in some situations, such as in situations where a
 * seat session switch is being forced.
 *
 * Returns the device id on success. Returns -1 and sets errno on error.
 */
int libseat_open_device(struct libseat *seat, const char *path, int *fd);

/*
 * Closes a device that has been opened on the seat using the device_id from
 * libseat_open_device.
 *
 * Returns 0 on success. Returns -1 and sets errno on error.
 */
int libseat_close_device(struct libseat *seat, int device_id);

/*
 * Retrieves the name of the seat that is currently made available through the
 * provided libseat instance.
 *
 * The returned string is owned by the libseat instance, and must not be
 * modified. It remains valid as long as the seat is open.
 */
const char *libseat_seat_name(struct libseat *seat);

/*
 * Requests that the seat switches session to the specified session number.
 * For seats that are VT-bound, the session number matches the VT number, and
 * switching session results in a VT switch.
 *
 * A call to libseat_switch_session does not imply that a switch will occur,
 * and the caller should assume that the session continues unaffected.
 *
 * Returns 0 on success. Returns -1 and sets errno on error.
 */
int libseat_switch_session(struct libseat *seat, int session);

/*
 * Retrieve the pollable connection fd for a given libseat instance. Used to
 * poll the libseat connection for events that need to be dispatched.
 *
 * Returns a pollable fd on success. Returns -1 and sets errno on error.
 */
int libseat_get_fd(struct libseat *seat);

/*
 * Reads and dispatches events on the libseat connection fd.
 *
 * The specified timeout dictates how long libseat might wait for data if none
 * is available: 0 means that no wait will occur, -1 means that libseat might
 * wait indefinitely for data to arrive, while > 0 is the maximum wait in
 * milliseconds that might occur.
 *
 * Returns a positive number signifying processed internal messages on success.
 * Returns 0 if no messages were processed. Returns -1 and sets errno on error.
 */
int libseat_dispatch(struct libseat *seat, int timeout);

/*
 * A log level.
 */
enum libseat_log_level {
	LIBSEAT_LOG_LEVEL_SILENT = 0,
	LIBSEAT_LOG_LEVEL_ERROR = 1,
	LIBSEAT_LOG_LEVEL_INFO = 2,
	LIBSEAT_LOG_LEVEL_DEBUG = 3,
	LIBSEAT_LOG_LEVEL_LAST,
};

/*
 * A function that handles log messages.
 */
typedef void (*libseat_log_func)(enum libseat_log_level level, const char *format, va_list args);

/*
 * Sets the handler for log messages.
 *
 * The handler will be called for each message whose level is lower or equal
 * to the current log level. If the handler is NULL, the handler is reset to
 * the default.
 */
void libseat_set_log_handler(libseat_log_func handler);

/*
 * Sets the libseat log level.
 *
 * Only log messages whose level is lower or equal than the current log level
 * will be processed, others will be ignored.
 */
void libseat_set_log_level(enum libseat_log_level level);

#endif
