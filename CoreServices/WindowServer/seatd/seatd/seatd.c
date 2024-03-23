#include <errno.h>
#include <grp.h>
#include <poll.h>
#include <pwd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

#include "client.h"
#include "log.h"
#include "poller.h"
#include "server.h"

#define LISTEN_BACKLOG 16

static int open_socket(const char *path, int uid, int gid) {
	union {
		struct sockaddr_un unix;
		struct sockaddr generic;
	} addr = {{0}};
	int fd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
	if (fd == -1) {
		log_errorf("Could not create socket: %s", strerror(errno));
		return -1;
	}

	addr.unix.sun_family = AF_UNIX;
	strncpy(addr.unix.sun_path, path, sizeof addr.unix.sun_path - 1);
	socklen_t size = offsetof(struct sockaddr_un, sun_path) + strlen(addr.unix.sun_path);
	if (bind(fd, &addr.generic, size) == -1) {
		log_errorf("Could not bind socket: %s", strerror(errno));
		goto error;
	}
	if (listen(fd, LISTEN_BACKLOG) == -1) {
		log_errorf("Could not listen on socket: %s", strerror(errno));
		goto error;
	}
	if (uid != -1 || gid != -1) {
		if (chmod(path, 0770) == -1) {
			log_errorf("Could not chmod socket: %s", strerror(errno));
			goto error;
		}
		if (chown(path, uid, gid) == -1) {
			log_errorf("Could not chown socket to uid %d, gid %d: %s", uid, gid,
				   strerror(errno));
			goto error;
		}
	}
	return fd;
error:
	close(fd);
	return -1;
}

int main(int argc, char *argv[]) {
	const char *usage = "Usage: seatd [options]\n"
			    "\n"
			    "  -h		Show this help message\n"
			    "  -n <fd>	FD to notify readiness on\n"
			    "  -u <user>	User to own the seatd socket\n"
			    "  -g <group>	Group to own the seatd socket\n"
			    "  -l <loglevel>	Log-level, one of debug, info, error or silent\n"
			    "  -v		Show the version number\n"
			    "\n";

	int c;
	int uid = -1, gid = -1;
	int readiness = -1;
	bool unlink_existing_socket = true;
	bool chown_socket = true;
	enum libseat_log_level level = LIBSEAT_LOG_LEVEL_INFO;
	while ((c = getopt(argc, argv, "vhn:g:u:l:z")) != -1) {
		switch (c) {
		case 'n':
			readiness = atoi(optarg);
			if (readiness < 0) {
				fprintf(stderr, "Invalid readiness fd: %s\n", optarg);
				return 1;
			}
			break;
		case 'u': {
			if (!chown_socket) {
				fprintf(stderr, "-u/-g and -z are mutually exclusive\n");
				return 1;
			}
			struct passwd *pw = getpwnam(optarg);
			if (pw == NULL) {
				fprintf(stderr, "Could not find user by name '%s'.\n", optarg);
				return 1;
			} else {
				uid = pw->pw_uid;
			}
			break;
		}
		case 'g': {
			if (!chown_socket) {
				fprintf(stderr, "-u/-g and -z are mutually exclusive\n");
				return 1;
			}
			struct group *gr = getgrnam(optarg);
			if (gr == NULL) {
				fprintf(stderr, "Could not find group by name '%s'.\n", optarg);
				return 1;
			} else {
				gid = gr->gr_gid;
			}
			break;
		}
		case 'l':
			if (strcmp(optarg, "debug") == 0) {
				level = LIBSEAT_LOG_LEVEL_DEBUG;
			} else if (strcmp(optarg, "info") == 0) {
				level = LIBSEAT_LOG_LEVEL_INFO;
			} else if (strcmp(optarg, "error") == 0) {
				level = LIBSEAT_LOG_LEVEL_ERROR;
			} else if (strcmp(optarg, "silent") == 0) {
				level = LIBSEAT_LOG_LEVEL_SILENT;
			} else {
				fprintf(stderr, "Invalid loglevel: %s\n", optarg);
				return 1;
			}
			break;
		case 'z':
			// Running under seatd-launch. We do not unlink files
			// to protect against multiple instances, and
			// seatd-launch takes care of ownership.
			if (uid != -1 || gid != -1) {
				fprintf(stderr, "-u/-g and -z are mutually exclusive\n");
				return 1;
			}
			unlink_existing_socket = false;
			chown_socket = false;
			break;
		case 'v':
			printf("seatd version %s\n", SEATD_VERSION);
			return 0;
		case 'h':
			printf("%s", usage);
			return 0;
		case '?':
			fprintf(stderr, "Try '%s -h' for more information.\n", argv[0]);
			return 1;
		default:
			abort();
		}
	}

	log_init();
	libseat_set_log_level(level);

	struct stat st;
	if (lstat(SEATD_DEFAULTPATH, &st) == 0) {
		if (!S_ISSOCK(st.st_mode)) {
			log_errorf("Non-socket file found at socket path %s, refusing to start",
				   SEATD_DEFAULTPATH);
			return 1;
		} else if (!unlink_existing_socket) {
			log_errorf("Socket file found at socket path %s, refusing to start",
				   SEATD_DEFAULTPATH);
			return 1;
		} else {
			// We only do this if the socket path is not user specified
			log_infof("Removing leftover socket at %s", SEATD_DEFAULTPATH);
			if (unlink(SEATD_DEFAULTPATH) == -1) {
				log_errorf("Could not remove leftover socket: %s", strerror(errno));
				return 1;
			}
		}
	}

	struct server server = {0};
	if (server_init(&server) == -1) {
		log_errorf("server_init failed: %s", strerror(errno));
		return 1;
	}

	int ret = 1;
	int socket_fd = open_socket(SEATD_DEFAULTPATH, uid, gid);
	if (socket_fd == -1) {
		log_error("Could not create server socket");
		goto error_server;
	}
	if (poller_add_fd(&server.poller, socket_fd, EVENT_READABLE, server_handle_connection,
			  &server) == NULL) {
		log_errorf("Could not add socket to poller: %s", strerror(errno));
		close(socket_fd);
		goto error_socket;
	}

	log_info("seatd started");

	if (readiness != -1) {
		if (write(readiness, "\n", 1) == -1) {
			log_errorf("Could not write readiness signal: %s\n", strerror(errno));
		}
		close(readiness);
	}

	while (server.running) {
		if (poller_poll(&server.poller) == -1) {
			log_errorf("Poller failed: %s", strerror(errno));
			goto error_socket;
		}
	}

	ret = 0;

error_socket:
	if (unlink(SEATD_DEFAULTPATH) == -1) {
		log_errorf("Could not remove socket: %s", strerror(errno));
	}
error_server:
	server_finish(&server);
	log_info("seatd stopped");
	return ret;
}
