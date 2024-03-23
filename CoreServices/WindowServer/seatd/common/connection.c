#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "connection.h"

static inline uint32_t connection_buffer_mask(const uint32_t idx) {
	return idx & (CONNECTION_BUFFER_SIZE - 1);
}

static inline uint32_t connection_buffer_size(const struct connection_buffer *b) {
	return b->head - b->tail;
}

static inline void connection_buffer_consume(struct connection_buffer *b, const size_t size) {
	b->tail += size;
}

static inline void connection_buffer_restore(struct connection_buffer *b, const size_t size) {
	b->tail -= size;
}

/*
 * connection_buffer_get_iov prepares I/O vectors pointing to our ring buffer.
 * Two may be used if the buffer has wrapped around.
 */
static void connection_buffer_get_iov(struct connection_buffer *b, struct iovec *iov, int *count) {
	uint32_t head = connection_buffer_mask(b->head);
	uint32_t tail = connection_buffer_mask(b->tail);
	if (tail < head) {
		iov[0].iov_base = b->data + tail;
		iov[0].iov_len = head - tail;
		*count = 1;
	} else if (head == 0) {
		iov[0].iov_base = b->data + tail;
		iov[0].iov_len = sizeof b->data - tail;
		*count = 1;
	} else {
		iov[0].iov_base = b->data + tail;
		iov[0].iov_len = sizeof b->data - tail;
		iov[1].iov_base = b->data;
		iov[1].iov_len = head;
		*count = 2;
	}
}

/*
 * connection_buffer_put_iov prepares I/O vectors pointing to our ring buffer.
 * Two may be used if the buffer has wrapped around.
 */
static void connection_buffer_put_iov(struct connection_buffer *b, struct iovec *iov, int *count) {
	uint32_t head = connection_buffer_mask(b->head);
	uint32_t tail = connection_buffer_mask(b->tail);
	if (head < tail) {
		iov[0].iov_base = b->data + head;
		iov[0].iov_len = tail - head;
		*count = 1;
	} else if (tail == 0) {
		iov[0].iov_base = b->data + head;
		iov[0].iov_len = sizeof b->data - head;
		*count = 1;
	} else {
		iov[0].iov_base = b->data + head;
		iov[0].iov_len = sizeof b->data - head;
		iov[1].iov_base = b->data;
		iov[1].iov_len = tail;
		*count = 2;
	}
}

/*
 * connection_buffer_copy copies from our ring buffer into a linear buffer.
 */
static void connection_buffer_copy(const struct connection_buffer *b, void *data, const size_t count) {
	uint32_t tail = connection_buffer_mask(b->tail);
	if (tail + count <= sizeof b->data) {
		memcpy(data, b->data + tail, count);
		return;
	}

	uint32_t size = sizeof b->data - tail;
	memcpy(data, b->data + tail, size);
	memcpy((char *)data + size, b->data, count - size);
}
/*
 * connection_buffer_copy copies from a linear buffer into our ring buffer.
 */
static int connection_buffer_put(struct connection_buffer *b, const void *data, const size_t count) {
	if (count > sizeof(b->data)) {
		errno = EOVERFLOW;
		return -1;
	}

	uint32_t head = connection_buffer_mask(b->head);
	if (head + count <= sizeof b->data) {
		memcpy(b->data + head, data, count);
	} else {
		uint32_t size = sizeof b->data - head;
		memcpy(b->data + head, data, size);
		memcpy(b->data, (const char *)data + size, count - size);
	}

	b->head += count;
	return 0;
}

/*
 * close_fds closes all fds within a connection_buffer
 */
static void connection_buffer_close_fds(struct connection_buffer *buffer) {
	size_t size = connection_buffer_size(buffer);
	if (size == 0) {
		return;
	}
	int fds[sizeof(buffer->data) / sizeof(int)];
	connection_buffer_copy(buffer, fds, size);
	int count = size / sizeof fds[0];
	size = count * sizeof fds[0];
	for (int idx = 0; idx < count; idx++) {
		close(fds[idx]);
	}
	connection_buffer_consume(buffer, size);
}

/*
 * build_cmsg prepares a cmsg from a buffer full of fds
 */
static void build_cmsg(struct connection_buffer *buffer, char *data, int *clen) {
	size_t size = connection_buffer_size(buffer);
	if (size > MAX_FDS * sizeof(int)) {
		size = MAX_FDS * sizeof(int);
	}

	if (size <= 0) {
		*clen = 0;
		return;
	}

	struct cmsghdr *cmsg = (struct cmsghdr *)data;
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	cmsg->cmsg_len = CMSG_LEN(size);
	connection_buffer_copy(buffer, CMSG_DATA(cmsg), size);
	*clen = cmsg->cmsg_len;
}

static int decode_cmsg(struct connection_buffer *buffer, struct msghdr *msg) {
	bool overflow = false;
	struct cmsghdr *cmsg;
	for (cmsg = CMSG_FIRSTHDR(msg); cmsg != NULL; cmsg = CMSG_NXTHDR(msg, cmsg)) {
		if (cmsg->cmsg_level != SOL_SOCKET || cmsg->cmsg_type != SCM_RIGHTS) {
			continue;
		}

		size_t size = cmsg->cmsg_len - CMSG_LEN(0);
		size_t max = sizeof(buffer->data) - connection_buffer_size(buffer);
		if (size > max || overflow) {
			overflow = true;
			size /= sizeof(int);
			for (size_t idx = 0; idx < size; idx++) {
				close(((int *)CMSG_DATA(cmsg))[idx]);
			}
		} else if (connection_buffer_put(buffer, CMSG_DATA(cmsg), size) < 0) {
			return -1;
		}
	}

	if (overflow) {
		errno = EOVERFLOW;
		return -1;
	}
	return 0;
}

int connection_read(struct connection *connection) {
	if (connection_buffer_size(&connection->in) >= sizeof(connection->in.data)) {
		errno = EOVERFLOW;
		return -1;
	}

	int count;
	struct iovec iov[2];
	connection_buffer_put_iov(&connection->in, iov, &count);

	char cmsg[CMSG_LEN(CONNECTION_BUFFER_SIZE)];
	struct msghdr msg = {
		.msg_name = NULL,
		.msg_namelen = 0,
		.msg_iov = iov,
		.msg_iovlen = count,
		.msg_control = cmsg,
		.msg_controllen = sizeof cmsg,
		.msg_flags = 0,
	};

	int len;
	do {
		len = recvmsg(connection->fd, &msg, MSG_DONTWAIT | MSG_CMSG_CLOEXEC);
		if (len == -1 && errno != EINTR)
			return -1;
	} while (len == -1);

	if (decode_cmsg(&connection->fds_in, &msg) != 0) {
		return -1;
	}
	connection->in.head += len;

	return connection_buffer_size(&connection->in);
}

int connection_flush(struct connection *connection) {
	if (!connection->want_flush) {
		return 0;
	}

	uint32_t tail = connection->out.tail;
	while (connection->out.head - connection->out.tail > 0) {
		int count;
		struct iovec iov[2];
		connection_buffer_get_iov(&connection->out, iov, &count);

		int clen;
		char cmsg[CMSG_LEN(CONNECTION_BUFFER_SIZE)];
		build_cmsg(&connection->fds_out, cmsg, &clen);
		struct msghdr msg = {
			.msg_name = NULL,
			.msg_namelen = 0,
			.msg_iov = iov,
			.msg_iovlen = count,
			.msg_control = (clen > 0) ? cmsg : NULL,
			.msg_controllen = clen,
			.msg_flags = 0,
		};

		int len;
		do {
			len = sendmsg(connection->fd, &msg, MSG_NOSIGNAL | MSG_DONTWAIT);
			if (len == -1 && errno != EINTR)
				return -1;
		} while (len == -1);
		connection_buffer_close_fds(&connection->fds_out);
		connection->out.tail += len;
	}
	connection->want_flush = 0;
	return connection->out.head - tail;
}

int connection_put(struct connection *connection, const void *data, size_t count) {
	if (connection_buffer_size(&connection->out) + count > CONNECTION_BUFFER_SIZE) {
		connection->want_flush = 1;
		if (connection_flush(connection) == -1) {
			return -1;
		}
	}

	if (connection_buffer_put(&connection->out, data, count) == -1) {
		return -1;
	}

	connection->want_flush = 1;
	return 0;
}

int connection_put_fd(struct connection *connection, int fd) {
	if (connection_buffer_size(&connection->fds_out) == MAX_FDS * sizeof fd) {
		errno = EOVERFLOW;
		return -1;
	}

	return connection_buffer_put(&connection->fds_out, &fd, sizeof fd);
}

int connection_get(struct connection *connection, void *dst, size_t count) {
	if (count > connection_buffer_size(&connection->in)) {
		errno = EAGAIN;
		return -1;
	}
	connection_buffer_copy(&connection->in, dst, count);
	connection_buffer_consume(&connection->in, count);
	return count;
}

int connection_get_fd(struct connection *connection, int *fd) {
	if (sizeof(int) > connection_buffer_size(&connection->fds_in)) {
		errno = EAGAIN;
		return -1;
	}
	connection_buffer_copy(&connection->fds_in, fd, sizeof(int));
	connection_buffer_consume(&connection->fds_in, sizeof(int));
	return 0;
}

void connection_close_fds(struct connection *connection) {
	connection_buffer_close_fds(&connection->fds_in);
	connection_buffer_close_fds(&connection->fds_out);
}

size_t connection_pending(struct connection *connection) {
	return connection_buffer_size(&connection->in);
}

void connection_restore(struct connection *connection, size_t count) {
	connection_buffer_restore(&connection->in, count);
}
