#ifndef _SEATD_SEAT_H
#define _SEATD_SEAT_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#include "linked_list.h"

struct client;

enum seat_device_type {
	SEAT_DEVICE_TYPE_NORMAL,
	SEAT_DEVICE_TYPE_EVDEV,
	SEAT_DEVICE_TYPE_DRM,
	SEAT_DEVICE_TYPE_WSCONS,
};

struct seat_device {
	struct linked_list link; // client::devices
	int device_id;
	int fd;
	int ref_cnt;
	bool active;
	char *path;
	enum seat_device_type type;
};

struct seat {
	struct linked_list link; // server::seats
	char *seat_name;
	struct linked_list clients;
	struct client *active_client;
	struct client *next_client;

	bool vt_bound;
	int cur_vt;
	int session_cnt;
};

struct seat *seat_create(const char *name, bool vt_bound);
void seat_destroy(struct seat *seat);

int seat_add_client(struct seat *seat, struct client *client);
int seat_remove_client(struct client *client);
int seat_open_client(struct seat *seat, struct client *client);
int seat_ack_disable_client(struct client *client);

struct seat_device *seat_open_device(struct client *client, const char *path);
int seat_close_device(struct client *client, struct seat_device *seat_device);
struct seat_device *seat_find_device(struct client *client, int device_id);

int seat_set_next_session(struct client *client, int session);
int seat_vt_activate(struct seat *seat);
int seat_vt_release(struct seat *seat);

#endif
