#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#if defined(__linux__)
#include <linux/kd.h>
#include <linux/vt.h>
#define K_ENABLE  K_UNICODE
#define K_DISABLE K_OFF
#define FRSIG     0
#elif defined(__FreeBSD__)
#include <sys/consio.h>
#include <sys/kbio.h>
#include <termios.h>
#define K_ENABLE  K_XLATE
#define K_DISABLE K_RAW
#define FRSIG     SIGIO
#elif defined(__NetBSD__)
#include <dev/wscons/wsdisplay_usl_io.h>
#define K_ENABLE  K_XLATE
#define K_DISABLE K_RAW
#define FRSIG     0 // unimplemented
#else
#error Unsupported platform
#endif

#include "log.h"
#include "terminal.h"

#define TTYPATHLEN 16

#if defined(__FreeBSD__)
static int get_tty_path(int tty, char path[static TTYPATHLEN]) {
	assert(tty >= 0);

	const char prefix[] = "/dev/ttyv";
	const size_t prefix_len = sizeof(prefix) - 1;
	strcpy(path, prefix);

	// The FreeBSD VT_GETACTIVE is implemented in the kernel as follows:
	//
	//	static int
	//	vtterm_ioctl(struct terminal *tm, u_long cmd, caddr_t data,
	// 		struct thread *td)
	//	{
	//		struct vt_window *vw = tm->tm_softc;
	//		struct vt_device *vd = vw->vw_device;
	//		...
	//		switch (cmd) {
	//		...
	//		case VT_GETACTIVE:
	//			*(int *)data = vd->vd_curwindow->vw_number + 1;
	//			return (0);
	//		...
	//		}
	//		...
	//	}
	//
	// The side-effect here being that the returned VT number is one
	// greater than the internal VT number. The internal number is what is
	// used to number the TTY device, while the external number is what we
	// use in e.g. VT switching.
	//
	// We subtract one from the requested TTY number to compensate. If the
	// user asked for TTY 0 (which is special on Linux), we just give them
	// the first tty.

	if (tty > 0) {
		tty--;
	}

	// The FreeBSD tty name is constructed in the kernel as follows:
	//
	//	static void
	//	vtterm_cnprobe(struct terminal *tm, struct consdev *cp)
	//	{
	//		...
	//		struct vt_window *vw = tm->tm_softc;
	//		...
	//		sprintf(cp->cn_name, "ttyv%r", VT_UNIT(vw));
	//		...
	//	}
	//
	// With %r being a FreeBSD-internal radix formatter (seemingly set to
	// base 32), and VT_UNIT expanding to the following to extract the
	// internal VT number (which is one less than the external VT number):
	//
	//	((vw)->vw_device->vd_unit * VT_MAXWINDOWS + (vw)->vw_number)
	//
	// As the %r formatter is kernel-internal, we implement the base 32
	// encoding ourselves below.

	size_t offset = prefix_len;
	if (tty == 0) {
		path[offset++] = '0';
		path[offset++] = '\0';
		return 0;
	}

	const int base = 32;
	for (int remaining = tty; remaining > 0; remaining /= base, offset++) {
		// Return early if the buffer is too small.
		if (offset + 1 >= TTYPATHLEN) {
			errno = ENOMEM;
			return -1;
		}

		const int value = remaining % base;
		if (value >= 10) {
			path[offset] = 'a' + value - 10;
		} else {
			path[offset] = '0' + value;
		}
	}

	const size_t num_len = offset - prefix_len;
	for (size_t i = 0; i < num_len / 2; i++) {
		const size_t p1 = prefix_len + i;
		const size_t p2 = offset - 1 - i;
		const char tmp = path[p1];
		path[p1] = path[p2];
		path[p2] = tmp;
	}

	path[offset++] = '\0';
	return 0;
}
#elif defined(__linux__)
static int get_tty_path(int tty, char path[static TTYPATHLEN]) {
	assert(tty >= 0);
	if (snprintf(path, TTYPATHLEN, "/dev/tty%d", tty) == -1) {
		return -1;
	}
	return 0;
}
#elif defined(__NetBSD__)
static int get_tty_path(int tty, char path[static TTYPATHLEN]) {
	assert(tty >= 0);
	if (snprintf(path, TTYPATHLEN, "/dev/ttyE%d", tty) == -1) {
		return -1;
	}
	return 0;
}
#else
#error Unsupported platform
#endif

int terminal_open(int vt) {
	char path[TTYPATHLEN];
	if (get_tty_path(vt, path) == -1) {
		log_errorf("Could not generate tty path: %s", strerror(errno));
		return -1;
	}
	int fd = open(path, O_RDWR | O_NOCTTY);
	if (fd == -1) {
		log_errorf("Could not open target tty: %s", strerror(errno));
		return -1;
	}
	return fd;
}

int terminal_current_vt(int fd) {
#if defined(__linux__) || defined(__NetBSD__)
	struct vt_stat st;
	int res = ioctl(fd, VT_GETSTATE, &st);
	close(fd);
	if (res == -1) {
		log_errorf("Could not retrieve VT state: %s", strerror(errno));
		return -1;
	}
	return st.v_active;
#elif defined(__FreeBSD__)
	int vt;
	int res = ioctl(fd, VT_GETACTIVE, &vt);
	close(fd);
	if (res == -1) {
		log_errorf("Could not retrieve VT state: %s", strerror(errno));
		return -1;
	}

	if (vt == -1) {
		log_errorf("Invalid VT: %d", vt);
		return -1;
	}
	return vt;
#else
#error Unsupported platform
#endif
}

int terminal_set_process_switching(int fd, bool enable) {
	log_debugf("Setting process switching to %d", enable);
	struct vt_mode mode = {
		.mode = enable ? VT_PROCESS : VT_AUTO,
		.waitv = 0,
		.relsig = enable ? SIGUSR1 : 0,
		.acqsig = enable ? SIGUSR2 : 0,
		.frsig = FRSIG,
	};

	if (ioctl(fd, VT_SETMODE, &mode) == -1) {
		log_errorf("Could not set VT mode to %s process switching: %s",
			   enable ? "enable" : "disable", strerror(errno));
		return -1;
	}
	return 0;
}

int terminal_switch_vt(int fd, int vt) {
	log_debugf("Switching to VT %d", vt);
	if (ioctl(fd, VT_ACTIVATE, vt) == -1) {
		log_errorf("Could not activate VT %d: %s", vt, strerror(errno));
		return -1;
	}

	return 0;
}

int terminal_ack_release(int fd) {
	log_debug("Acking VT release");
	if (ioctl(fd, VT_RELDISP, 1) == -1) {
		log_errorf("Could not ack VT release: %s", strerror(errno));
		return -1;
	}

	return 0;
}

int terminal_ack_acquire(int fd) {
	log_debug("Acking VT acquire");
	if (ioctl(fd, VT_RELDISP, VT_ACKACQ) == -1) {
		log_errorf("Could not ack VT acquire: %s", strerror(errno));
		return -1;
	}

	return 0;
}

int terminal_set_keyboard(int fd, bool enable) {
	log_debugf("Setting KD keyboard state to %d", enable);
	if (ioctl(fd, KDSKBMODE, enable ? K_ENABLE : K_DISABLE) == -1) {
		log_errorf("Could not set KD keyboard mode to %s: %s",
			   enable ? "enabled" : "disabled", strerror(errno));
		return -1;
	}
#if defined(__FreeBSD__)
	struct termios tios;
	if (tcgetattr(fd, &tios) == -1) {
		log_errorf("Could not set get terminal mode: %s", strerror(errno));
		return -1;
	}
	if (enable) {
		cfmakesane(&tios);
	} else {
		cfmakeraw(&tios);
	}
	if (tcsetattr(fd, TCSAFLUSH, &tios) == -1) {
		log_errorf("Could not set terminal mode to %s: %s", enable ? "sane" : "raw",
			   strerror(errno));
		return -1;
	}
#endif
	return 0;
}

int terminal_set_graphics(int fd, bool enable) {
	log_debugf("Setting KD graphics state to %d", enable);
	if (ioctl(fd, KDSETMODE, enable ? KD_GRAPHICS : KD_TEXT) == -1) {
		log_errorf("Could not set KD graphics mode to %s: %s", enable ? "graphics" : "text",
			   strerror(errno));
		return -1;
	}
	return 0;
}
