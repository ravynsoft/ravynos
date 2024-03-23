#ifndef _SEATD_CONSTANTS_H
#define _SEATD_CONSTANTS_H

#define MAX_PATH_LEN     256
#define MAX_SEAT_LEN     64
#define MAX_SEAT_DEVICES 128
#define MAX_SESSION_LEN  64

#define CLIENT_EVENT(opcode) (opcode)
#define SERVER_EVENT(opcode) ((opcode) + (1 << 15))

#define CLIENT_OPEN_SEAT      CLIENT_EVENT(1)
#define CLIENT_CLOSE_SEAT     CLIENT_EVENT(2)
#define CLIENT_OPEN_DEVICE    CLIENT_EVENT(3)
#define CLIENT_CLOSE_DEVICE   CLIENT_EVENT(4)
#define CLIENT_DISABLE_SEAT   CLIENT_EVENT(5)
#define CLIENT_SWITCH_SESSION CLIENT_EVENT(6)
#define CLIENT_PING           CLIENT_EVENT(7)

#define SERVER_SEAT_OPENED   SERVER_EVENT(1)
#define SERVER_SEAT_CLOSED   SERVER_EVENT(2)
#define SERVER_DEVICE_OPENED SERVER_EVENT(3)
#define SERVER_DEVICE_CLOSED SERVER_EVENT(4)
#define SERVER_DISABLE_SEAT  SERVER_EVENT(5)
#define SERVER_ENABLE_SEAT   SERVER_EVENT(6)
#define SERVER_PONG          SERVER_EVENT(7)
#define SERVER_ERROR         SERVER_EVENT(0x7FFF)

#include <stdint.h>

struct proto_header {
	uint16_t opcode;
	uint16_t size;
};

struct proto_client_open_device {
	uint16_t path_len;
	// NULL-terminated byte-sequence path_len long follows
};

struct proto_client_close_device {
	int device_id;
};

struct proto_client_switch_session {
	int session;
};

struct proto_server_seat_opened {
	uint16_t seat_name_len;
	// NULL-terminated byte-sequence seat_name_len long follows
};

struct proto_server_device_opened {
	int device_id;
	// One fd in auxillary data
};

struct proto_server_error {
	int error_code;
};

#endif
