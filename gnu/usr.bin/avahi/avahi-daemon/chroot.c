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

#include <inttypes.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/un.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include <avahi-core/log.h>
#include <libdaemon/dfork.h>

#include "chroot.h"
#include "caps.h"
#include "setproctitle.h"

enum {
    AVAHI_CHROOT_SUCCESS = 0,
    AVAHI_CHROOT_FAILURE,
    AVAHI_CHROOT_GET_RESOLV_CONF,
#ifdef HAVE_DBUS
    AVAHI_CHROOT_GET_SERVER_INTROSPECT,
    AVAHI_CHROOT_GET_ENTRY_GROUP_INTROSPECT,
    AVAHI_CHROOT_GET_ADDRESS_RESOLVER_INTROSPECT,
    AVAHI_CHROOT_GET_DOMAIN_BROWSER_INTROSPECT,
    AVAHI_CHROOT_GET_HOST_NAME_RESOLVER_INTROSPECT,
    AVAHI_CHROOT_GET_SERVICE_BROWSER_INTROSPECT,
    AVAHI_CHROOT_GET_SERVICE_RESOLVER_INTROSPECT,
    AVAHI_CHROOT_GET_SERVICE_TYPE_BROWSER_INTROSPECT,
    AVAHI_CHROOT_GET_RECORD_BROWSER_INTROSPECT,
#endif
    AVAHI_CHROOT_UNLINK_PID,
    AVAHI_CHROOT_UNLINK_SOCKET,
    AVAHI_CHROOT_MAX
};

static const char* const get_file_name_table[AVAHI_CHROOT_MAX] = {
    NULL,
    NULL,
    "/etc/resolv.conf",
#ifdef HAVE_DBUS
    AVAHI_DBUS_INTROSPECTION_DIR"/org.freedesktop.Avahi.Server.xml",
    AVAHI_DBUS_INTROSPECTION_DIR"/org.freedesktop.Avahi.EntryGroup.xml",
    AVAHI_DBUS_INTROSPECTION_DIR"/org.freedesktop.Avahi.AddressResolver.xml",
    AVAHI_DBUS_INTROSPECTION_DIR"/org.freedesktop.Avahi.DomainBrowser.xml",
    AVAHI_DBUS_INTROSPECTION_DIR"/org.freedesktop.Avahi.HostNameResolver.xml",
    AVAHI_DBUS_INTROSPECTION_DIR"/org.freedesktop.Avahi.ServiceBrowser.xml",
    AVAHI_DBUS_INTROSPECTION_DIR"/org.freedesktop.Avahi.ServiceResolver.xml",
    AVAHI_DBUS_INTROSPECTION_DIR"/org.freedesktop.Avahi.ServiceTypeBrowser.xml",
    AVAHI_DBUS_INTROSPECTION_DIR"/org.freedesktop.Avahi.RecordBrowser.xml",
#endif
    NULL,
    NULL
};

static const char *const unlink_file_name_table[AVAHI_CHROOT_MAX] = {
    NULL,
    NULL,
    NULL,
#ifdef HAVE_DBUS
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
#endif
    AVAHI_DAEMON_RUNTIME_DIR"/pid",
    AVAHI_SOCKET
};

static int helper_fd = -1;

static int send_fd(int fd, int payload_fd) {
    uint8_t dummy = AVAHI_CHROOT_SUCCESS;
    struct iovec iov;
    struct msghdr msg;
    union {
        struct cmsghdr hdr;
        char buf[CMSG_SPACE(sizeof(int))];
    } cmsg;

    /* Send a file descriptor over the socket */

    memset(&iov, 0, sizeof(iov));
    memset(&msg, 0, sizeof(msg));
    memset(&cmsg, 0, sizeof(cmsg));

    iov.iov_base = &dummy;
    iov.iov_len = sizeof(dummy);

    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;

    msg.msg_control = &cmsg;
    msg.msg_controllen = sizeof(cmsg);
    msg.msg_flags = 0;

    cmsg.hdr.cmsg_len = CMSG_LEN(sizeof(int));
    cmsg.hdr.cmsg_level = SOL_SOCKET;
    cmsg.hdr.cmsg_type = SCM_RIGHTS;
    *((int*) CMSG_DATA(&cmsg.hdr)) = payload_fd;

    if (sendmsg(fd, &msg, 0) < 0) {
        avahi_log_error("sendmsg() failed: %s", strerror(errno));
        return -1;
    }

    return 0;
}

static int recv_fd(int fd) {
    uint8_t dummy;
    struct iovec iov;
    struct msghdr msg;
    union {
        struct cmsghdr hdr;
        char buf[CMSG_SPACE(sizeof(int))];
    } cmsg;

    /* Receive a file descriptor from a socket */

    memset(&iov, 0, sizeof(iov));
    memset(&msg, 0, sizeof(msg));
    memset(&cmsg, 0, sizeof(cmsg));

    iov.iov_base = &dummy;
    iov.iov_len = sizeof(dummy);

    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;

    msg.msg_control = cmsg.buf;
    msg.msg_controllen = sizeof(cmsg);
    msg.msg_flags = 0;

    cmsg.hdr.cmsg_len = CMSG_LEN(sizeof(int));
    cmsg.hdr.cmsg_level = SOL_SOCKET;
    cmsg.hdr.cmsg_type = SCM_RIGHTS;
    *((int*) CMSG_DATA(&cmsg.hdr)) = -1;

    if (recvmsg(fd, &msg, 0) <= 0) {
        avahi_log_error("recvmsg() failed: %s", strerror(errno));
        return -1;
    } else {
        struct cmsghdr* h;

        if (dummy != AVAHI_CHROOT_SUCCESS) {
            errno = EINVAL;
            return -1;
        }

        if (!(h = CMSG_FIRSTHDR(&msg))) {
            avahi_log_error("recvmsg() sent no fd.");
            errno = EINVAL;
            return -1;
        }

        assert(h->cmsg_len = CMSG_LEN(sizeof(int)));
        assert(h->cmsg_level = SOL_SOCKET);
        assert(h->cmsg_type == SCM_RIGHTS);

        return *((int*)CMSG_DATA(h));
    }
}

static int helper_main(int fd) {
    int ret = 1;
    assert(fd >= 0);

    /* This is the main function of our helper process which is forked
     * off to access files outside the chroot environment. Keep in
     * mind that this code is security sensitive! */

    avahi_log_debug(__FILE__": chroot() helper started");

    for (;;) {
        uint8_t command;
        ssize_t r;

        if ((r = read(fd, &command, sizeof(command))) <= 0) {

            /* EOF? */
            if (r == 0)
                break;

            avahi_log_error(__FILE__": read() failed: %s", strerror(errno));
            goto fail;
        }

        assert(r == sizeof(command));

        avahi_log_debug(__FILE__": chroot() helper got command %02x", command);

        switch (command) {
#ifdef HAVE_DBUS
            case AVAHI_CHROOT_GET_SERVER_INTROSPECT:
            case AVAHI_CHROOT_GET_ENTRY_GROUP_INTROSPECT:
            case AVAHI_CHROOT_GET_ADDRESS_RESOLVER_INTROSPECT:
            case AVAHI_CHROOT_GET_DOMAIN_BROWSER_INTROSPECT:
            case AVAHI_CHROOT_GET_HOST_NAME_RESOLVER_INTROSPECT:
            case AVAHI_CHROOT_GET_SERVICE_BROWSER_INTROSPECT:
            case AVAHI_CHROOT_GET_SERVICE_RESOLVER_INTROSPECT:
            case AVAHI_CHROOT_GET_SERVICE_TYPE_BROWSER_INTROSPECT:
            case AVAHI_CHROOT_GET_RECORD_BROWSER_INTROSPECT:
#endif
            case AVAHI_CHROOT_GET_RESOLV_CONF: {
                int payload;

                if ((payload = open(get_file_name_table[(int) command], O_RDONLY)) < 0) {
                    uint8_t c = AVAHI_CHROOT_FAILURE;

                    avahi_log_error(__FILE__": open() failed: %s", strerror(errno));

                    if (write(fd, &c, sizeof(c)) != sizeof(c)) {
                        avahi_log_error(__FILE__": write() failed: %s\n", strerror(errno));
                        goto fail;
                    }

                    break;
                }

                if (send_fd(fd, payload) < 0)
                    goto fail;

                close(payload);

                break;
            }

            case AVAHI_CHROOT_UNLINK_SOCKET:
            case AVAHI_CHROOT_UNLINK_PID: {
                uint8_t c = AVAHI_CHROOT_SUCCESS;

                unlink(unlink_file_name_table[(int) command]);

                if (write(fd, &c, sizeof(c)) != sizeof(c)) {
                    avahi_log_error(__FILE__": write() failed: %s\n", strerror(errno));
                    goto fail;
                }

                break;
            }

            default:
                avahi_log_error(__FILE__": Unknown command %02x.", command);
                break;
        }
    }

    ret = 0;

fail:

    avahi_log_debug(__FILE__": chroot() helper exiting with return value %i", ret);

    return ret;
}

int avahi_chroot_helper_start(const char *argv0) {
    int sock[2];
    pid_t pid;

    assert(helper_fd < 0);

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sock) < 0) {
        avahi_log_error("socketpair() failed: %s", strerror(errno));
        return -1;
    }

    if ((pid = fork()) < 0) {
        close(sock[0]);
        close(sock[1]);
        avahi_log_error(__FILE__": fork() failed: %s", strerror(errno));
        return -1;
    } else if (pid == 0) {

        /* Drop all remaining capabilities */
        avahi_caps_drop_all();

        avahi_set_proc_title(argv0, "%s: chroot helper", argv0);

        daemon_retval_done();

        close(sock[0]);
        helper_main(sock[1]);
        _exit(0);
    }

    close(sock[1]);
    helper_fd = sock[0];

    return 0;
}

void avahi_chroot_helper_shutdown(void) {

    if (helper_fd <= 0)
        return;

    close(helper_fd);
    helper_fd = -1;
}

int avahi_chroot_helper_get_fd(const char *fname) {

    if (helper_fd >= 0) {
        uint8_t command;

        for (command = 2; command < AVAHI_CHROOT_MAX; command++)
            if (get_file_name_table[(int) command] &&
                strcmp(fname, get_file_name_table[(int) command]) == 0)
                break;

        if (command >= AVAHI_CHROOT_MAX) {
            avahi_log_error("chroot() helper accessed for invalid file name");
            errno = EACCES;
            return -1;
        }

        assert(get_file_name_table[(int) command]);

        if (write(helper_fd, &command, sizeof(command)) < 0) {
            avahi_log_error("write() failed: %s\n", strerror(errno));
            return -1;
        }

        return recv_fd(helper_fd);

    } else
        return open(fname, O_RDONLY);
}


FILE *avahi_chroot_helper_get_file(const char *fname) {
    FILE *f;
    int fd;

    if ((fd = avahi_chroot_helper_get_fd(fname)) < 0)
        return NULL;

    f = fdopen(fd, "r");
    assert(f);

    return f;
}

int avahi_chroot_helper_unlink(const char *fname) {

    if (helper_fd >= 0) {
        uint8_t c, command;
        ssize_t r;

        for (command = 2; command < AVAHI_CHROOT_MAX; command++)
            if (unlink_file_name_table[(int) command] &&
                strcmp(fname, unlink_file_name_table[(int) command]) == 0)
                break;

        if (command >= AVAHI_CHROOT_MAX) {
            avahi_log_error("chroot() helper accessed for invalid file name");
            errno = EACCES;
            return -1;
        }

        if (write(helper_fd, &command, sizeof(command)) < 0 &&
            (errno != EPIPE && errno != ECONNRESET)) {
            avahi_log_error("write() failed: %s\n", strerror(errno));
            return -1;
        }

        if ((r = read(helper_fd, &c, sizeof(c))) < 0 &&
            (errno != EPIPE && errno != ECONNRESET)) {
            avahi_log_error("read() failed: %s\n", r < 0 ? strerror(errno) : "EOF");
            return -1;
        }

        return 0;

    } else

        return unlink(fname);

}
