/*
 * Copyright © 2008 Kristian Høgsberg
 * Copyright © 2013 Jason Ekstrand
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define _GNU_SOURCE

#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <ffi.h>

#include "wayland-util.h"
#include "wayland-private.h"
#include "wayland-os.h"

static inline uint32_t
div_roundup(uint32_t n, size_t a)
{
	/* The cast to uint64_t is necessary to prevent overflow when rounding
	 * values close to UINT32_MAX. After the division it is again safe to
	 * cast back to uint32_t.
	 */
	return (uint32_t) (((uint64_t) n + (a - 1)) / a);
}

struct wl_ring_buffer {
	char data[4096];
	uint32_t head, tail;
};

#define MASK(i) ((i) & 4095)

#define MAX_FDS_OUT	28
#define CLEN		(CMSG_LEN(MAX_FDS_OUT * sizeof(int32_t)))

struct wl_connection {
	struct wl_ring_buffer in, out;
	struct wl_ring_buffer fds_in, fds_out;
	int fd;
	int want_flush;
};

static int
ring_buffer_put(struct wl_ring_buffer *b, const void *data, size_t count)
{
	uint32_t head, size;

	if (count > sizeof(b->data)) {
		wl_log("Data too big for buffer (%d > %d).\n",
		       count, sizeof(b->data));
		errno = E2BIG;
		return -1;
	}

	head = MASK(b->head);
	if (head + count <= sizeof b->data) {
		memcpy(b->data + head, data, count);
	} else {
		size = sizeof b->data - head;
		memcpy(b->data + head, data, size);
		memcpy(b->data, (const char *) data + size, count - size);
	}

	b->head += count;

	return 0;
}

static void
ring_buffer_put_iov(struct wl_ring_buffer *b, struct iovec *iov, int *count)
{
	uint32_t head, tail;

	head = MASK(b->head);
	tail = MASK(b->tail);
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

static void
ring_buffer_get_iov(struct wl_ring_buffer *b, struct iovec *iov, int *count)
{
	uint32_t head, tail;

	head = MASK(b->head);
	tail = MASK(b->tail);
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

static void
ring_buffer_copy(struct wl_ring_buffer *b, void *data, size_t count)
{
	uint32_t tail, size;

	tail = MASK(b->tail);
	if (tail + count <= sizeof b->data) {
		memcpy(data, b->data + tail, count);
	} else {
		size = sizeof b->data - tail;
		memcpy(data, b->data + tail, size);
		memcpy((char *) data + size, b->data, count - size);
	}
}

static uint32_t
ring_buffer_size(struct wl_ring_buffer *b)
{
	return b->head - b->tail;
}

struct wl_connection *
wl_connection_create(int fd)
{
	struct wl_connection *connection;

	connection = zalloc(sizeof *connection);
	if (connection == NULL)
		return NULL;

	connection->fd = fd;

	return connection;
}

static void
close_fds(struct wl_ring_buffer *buffer, int max)
{
	int32_t fds[sizeof(buffer->data) / sizeof(int32_t)], i, count;
	size_t size;

	size = ring_buffer_size(buffer);
	if (size == 0)
		return;

	ring_buffer_copy(buffer, fds, size);
	count = size / sizeof fds[0];
	if (max > 0 && max < count)
		count = max;
	size = count * sizeof fds[0];
	for (i = 0; i < count; i++)
		close(fds[i]);
	buffer->tail += size;
}

void
wl_connection_close_fds_in(struct wl_connection *connection, int max)
{
	close_fds(&connection->fds_in, max);
}

int
wl_connection_destroy(struct wl_connection *connection)
{
	int fd = connection->fd;

	close_fds(&connection->fds_out, -1);
	close_fds(&connection->fds_in, -1);
	free(connection);

	return fd;
}

void
wl_connection_copy(struct wl_connection *connection, void *data, size_t size)
{
	ring_buffer_copy(&connection->in, data, size);
}

void
wl_connection_consume(struct wl_connection *connection, size_t size)
{
	connection->in.tail += size;
}

static void
build_cmsg(struct wl_ring_buffer *buffer, char *data, size_t *clen)
{
	struct cmsghdr *cmsg;
	size_t size;

	size = ring_buffer_size(buffer);
	if (size > MAX_FDS_OUT * sizeof(int32_t))
		size = MAX_FDS_OUT * sizeof(int32_t);

	if (size > 0) {
		cmsg = (struct cmsghdr *) data;
		cmsg->cmsg_level = SOL_SOCKET;
		cmsg->cmsg_type = SCM_RIGHTS;
		cmsg->cmsg_len = CMSG_LEN(size);
		ring_buffer_copy(buffer, CMSG_DATA(cmsg), size);
		*clen = cmsg->cmsg_len;
	} else {
		*clen = 0;
	}
}

static int
decode_cmsg(struct wl_ring_buffer *buffer, struct msghdr *msg)
{
	struct cmsghdr *cmsg;
	size_t size, max, i;
	int overflow = 0;

	for (cmsg = CMSG_FIRSTHDR(msg); cmsg != NULL;
	     cmsg = CMSG_NXTHDR(msg, cmsg)) {
		if (cmsg->cmsg_level != SOL_SOCKET ||
		    cmsg->cmsg_type != SCM_RIGHTS)
			continue;

		size = cmsg->cmsg_len - CMSG_LEN(0);
		max = sizeof(buffer->data) - ring_buffer_size(buffer);
		if (size > max || overflow) {
			overflow = 1;
			size /= sizeof(int32_t);
			for (i = 0; i < size; i++)
				close(((int*)CMSG_DATA(cmsg))[i]);
		} else if (ring_buffer_put(buffer, CMSG_DATA(cmsg), size) < 0) {
				return -1;
		}
	}

	if (overflow) {
		errno = EOVERFLOW;
		return -1;
	}

	return 0;
}

int
wl_connection_flush(struct wl_connection *connection)
{
	struct iovec iov[2];
	struct msghdr msg = {0};
	char cmsg[CLEN];
	int len = 0, count;
	size_t clen;
	uint32_t tail;

	if (!connection->want_flush)
		return 0;

	tail = connection->out.tail;
	while (connection->out.head - connection->out.tail > 0) {
		ring_buffer_get_iov(&connection->out, iov, &count);

		build_cmsg(&connection->fds_out, cmsg, &clen);

		msg.msg_iov = iov;
		msg.msg_iovlen = count;
		msg.msg_control = (clen > 0) ? cmsg : NULL;
		msg.msg_controllen = clen;

		do {
			len = sendmsg(connection->fd, &msg,
				      MSG_NOSIGNAL | MSG_DONTWAIT);
		} while (len == -1 && errno == EINTR);

		if (len == -1)
			return -1;

		close_fds(&connection->fds_out, MAX_FDS_OUT);

		connection->out.tail += len;
	}

	connection->want_flush = 0;

	return connection->out.head - tail;
}

uint32_t
wl_connection_pending_input(struct wl_connection *connection)
{
	return ring_buffer_size(&connection->in);
}

int
wl_connection_read(struct wl_connection *connection)
{
	struct iovec iov[2];
	struct msghdr msg;
	char cmsg[CLEN];
	int len, count, ret;

	if (ring_buffer_size(&connection->in) >= sizeof(connection->in.data)) {
		errno = EOVERFLOW;
		return -1;
	}

	ring_buffer_put_iov(&connection->in, iov, &count);

	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = iov;
	msg.msg_iovlen = count;
	msg.msg_control = cmsg;
	msg.msg_controllen = sizeof cmsg;
	msg.msg_flags = 0;

	do {
		len = wl_os_recvmsg_cloexec(connection->fd, &msg, MSG_DONTWAIT);
	} while (len < 0 && errno == EINTR);

	if (len <= 0)
		return len;

	ret = decode_cmsg(&connection->fds_in, &msg);
	if (ret)
		return -1;

	connection->in.head += len;

	return wl_connection_pending_input(connection);
}

int
wl_connection_write(struct wl_connection *connection,
		    const void *data, size_t count)
{
	if (connection->out.head - connection->out.tail +
	    count > ARRAY_LENGTH(connection->out.data)) {
		connection->want_flush = 1;
		if (wl_connection_flush(connection) < 0)
			return -1;
	}

	if (ring_buffer_put(&connection->out, data, count) < 0)
		return -1;

	connection->want_flush = 1;

	return 0;
}

int
wl_connection_queue(struct wl_connection *connection,
		    const void *data, size_t count)
{
	if (connection->out.head - connection->out.tail +
	    count > ARRAY_LENGTH(connection->out.data)) {
		connection->want_flush = 1;
		if (wl_connection_flush(connection) < 0)
			return -1;
	}

	return ring_buffer_put(&connection->out, data, count);
}

int
wl_message_count_arrays(const struct wl_message *message)
{
	int i, arrays;

	for (i = 0, arrays = 0; message->signature[i]; i++) {
		if (message->signature[i] == 'a')
			arrays++;
	}

	return arrays;
}

int
wl_connection_get_fd(struct wl_connection *connection)
{
	return connection->fd;
}

static int
wl_connection_put_fd(struct wl_connection *connection, int32_t fd)
{
	if (ring_buffer_size(&connection->fds_out) == MAX_FDS_OUT * sizeof fd) {
		connection->want_flush = 1;
		if (wl_connection_flush(connection) < 0)
			return -1;
	}

	return ring_buffer_put(&connection->fds_out, &fd, sizeof fd);
}

const char *
get_next_argument(const char *signature, struct argument_details *details)
{
	details->nullable = 0;
	for(; *signature; ++signature) {
		switch(*signature) {
		case 'i':
		case 'u':
		case 'f':
		case 's':
		case 'o':
		case 'n':
		case 'a':
		case 'h':
			details->type = *signature;
			return signature + 1;
		case '?':
			details->nullable = 1;
		}
	}
	details->type = '\0';
	return signature;
}

int
arg_count_for_signature(const char *signature)
{
	int count = 0;
	for(; *signature; ++signature) {
		switch(*signature) {
		case 'i':
		case 'u':
		case 'f':
		case 's':
		case 'o':
		case 'n':
		case 'a':
		case 'h':
			++count;
		}
	}
	return count;
}

int
wl_message_get_since(const struct wl_message *message)
{
	int since;

	since = atoi(message->signature);

	if (since == 0)
		since = 1;

	return since;
}

void
wl_argument_from_va_list(const char *signature, union wl_argument *args,
			 int count, va_list ap)
{
	int i;
	const char *sig_iter;
	struct argument_details arg;

	sig_iter = signature;
	for (i = 0; i < count; i++) {
		sig_iter = get_next_argument(sig_iter, &arg);

		switch(arg.type) {
		case 'i':
			args[i].i = va_arg(ap, int32_t);
			break;
		case 'u':
			args[i].u = va_arg(ap, uint32_t);
			break;
		case 'f':
			args[i].f = va_arg(ap, wl_fixed_t);
			break;
		case 's':
			args[i].s = va_arg(ap, const char *);
			break;
		case 'o':
			args[i].o = va_arg(ap, struct wl_object *);
			break;
		case 'n':
			args[i].o = va_arg(ap, struct wl_object *);
			break;
		case 'a':
			args[i].a = va_arg(ap, struct wl_array *);
			break;
		case 'h':
			args[i].h = va_arg(ap, int32_t);
			break;
		case '\0':
			return;
		}
	}
}

static void
wl_closure_clear_fds(struct wl_closure *closure)
{
	const char *signature = closure->message->signature;
	struct argument_details arg;
	int i;

	for (i = 0; i < closure->count; i++) {
		signature = get_next_argument(signature, &arg);
		if (arg.type == 'h')
			closure->args[i].h = -1;
	}
}

static struct wl_closure *
wl_closure_init(const struct wl_message *message, uint32_t size,
                int *num_arrays, union wl_argument *args)
{
	struct wl_closure *closure;
	int count;

	count = arg_count_for_signature(message->signature);
	if (count > WL_CLOSURE_MAX_ARGS) {
		wl_log("too many args (%d)\n", count);
		errno = EINVAL;
		return NULL;
	}

	if (size) {
		*num_arrays = wl_message_count_arrays(message);
		closure = zalloc(sizeof *closure + size +
				 *num_arrays * sizeof(struct wl_array));
	} else {
		closure = zalloc(sizeof *closure);
	}

	if (!closure) {
		errno = ENOMEM;
		return NULL;
	}

	if (args)
		memcpy(closure->args, args, count * sizeof *args);

	closure->message = message;
	closure->count = count;

	/* Set these all to -1 so we can close any that have been
	 * set to a real value during wl_closure_destroy().
	 * We may have copied a bunch of fds into the closure with
	 * memcpy previously, but those are undup()d client fds
	 * that we would have replaced anyway.
	 */
	wl_closure_clear_fds(closure);

	return closure;
}

struct wl_closure *
wl_closure_marshal(struct wl_object *sender, uint32_t opcode,
		   union wl_argument *args,
		   const struct wl_message *message)
{
	struct wl_closure *closure;
	struct wl_object *object;
	int i, count, fd, dup_fd;
	const char *signature;
	struct argument_details arg;

	closure = wl_closure_init(message, 0, NULL, args);
	if (closure == NULL)
		return NULL;

	count = closure->count;

	signature = message->signature;
	for (i = 0; i < count; i++) {
		signature = get_next_argument(signature, &arg);

		switch (arg.type) {
		case 'f':
		case 'u':
		case 'i':
			break;
		case 's':
			if (!arg.nullable && args[i].s == NULL)
				goto err_null;
			break;
		case 'o':
			if (!arg.nullable && args[i].o == NULL)
				goto err_null;
			break;
		case 'n':
			object = args[i].o;
			if (object == NULL)
				goto err_null;

			closure->args[i].n = object ? object->id : 0;
			break;
		case 'a':
			if (args[i].a == NULL)
				goto err_null;
			break;
		case 'h':
			fd = args[i].h;
			dup_fd = wl_os_dupfd_cloexec(fd, 0);
			if (dup_fd < 0) {
				wl_closure_destroy(closure);
				wl_log("error marshalling arguments for %s: dup failed: %s\n",
				       message->name, strerror(errno));
				return NULL;
			}
			closure->args[i].h = dup_fd;
			break;
		default:
			wl_abort("unhandled format code: '%c'\n", arg.type);
			break;
		}
	}

	closure->sender_id = sender->id;
	closure->opcode = opcode;

	return closure;

err_null:
	wl_closure_destroy(closure);
	wl_log("error marshalling arguments for %s (signature %s): "
	       "null value passed for arg %i\n", message->name,
	       message->signature, i);
	errno = EINVAL;
	return NULL;
}

struct wl_closure *
wl_closure_vmarshal(struct wl_object *sender, uint32_t opcode, va_list ap,
		    const struct wl_message *message)
{
	union wl_argument args[WL_CLOSURE_MAX_ARGS];

	wl_argument_from_va_list(message->signature, args,
				 WL_CLOSURE_MAX_ARGS, ap);

	return wl_closure_marshal(sender, opcode, args, message);
}

struct wl_closure *
wl_connection_demarshal(struct wl_connection *connection,
			uint32_t size,
			struct wl_map *objects,
			const struct wl_message *message)
{
	uint32_t *p, *next, *end, length, length_in_u32, id;
	int fd;
	char *s;
	int i, count, num_arrays;
	const char *signature;
	struct argument_details arg;
	struct wl_closure *closure;
	struct wl_array *array_extra;

	/* Space for sender_id and opcode */
	if (size < 2 * sizeof *p) {
		wl_log("message too short, invalid header\n");
		wl_connection_consume(connection, size);
		errno = EINVAL;
		return NULL;
	}

	closure = wl_closure_init(message, size, &num_arrays, NULL);
	if (closure == NULL) {
		wl_connection_consume(connection, size);
		return NULL;
	}

	count = closure->count;

	array_extra = closure->extra;
	p = (uint32_t *)(closure->extra + num_arrays);
	end = p + size / sizeof *p;

	wl_connection_copy(connection, p, size);
	closure->sender_id = *p++;
	closure->opcode = *p++ & 0x0000ffff;

	signature = message->signature;
	for (i = 0; i < count; i++) {
		signature = get_next_argument(signature, &arg);

		if (arg.type != 'h' && p + 1 > end) {
			wl_log("message too short, "
			       "object (%d), message %s(%s)\n",
			       closure->sender_id, message->name,
			       message->signature);
			errno = EINVAL;
			goto err;
		}

		switch (arg.type) {
		case 'u':
			closure->args[i].u = *p++;
			break;
		case 'i':
			closure->args[i].i = *p++;
			break;
		case 'f':
			closure->args[i].f = *p++;
			break;
		case 's':
			length = *p++;

			if (length == 0 && !arg.nullable) {
				wl_log("NULL string received on non-nullable "
				       "type, message %s(%s)\n", message->name,
				       message->signature);
				errno = EINVAL;
				goto err;
			}
			if (length == 0) {
				closure->args[i].s = NULL;
				break;
			}

			length_in_u32 = div_roundup(length, sizeof *p);
			if ((uint32_t) (end - p) < length_in_u32) {
				wl_log("message too short, "
				       "object (%d), message %s(%s)\n",
				       closure->sender_id, message->name,
				       message->signature);
				errno = EINVAL;
				goto err;
			}
			next = p + length_in_u32;

			s = (char *) p;

			if (length > 0 && s[length - 1] != '\0') {
				wl_log("string not nul-terminated, "
				       "message %s(%s)\n",
				       message->name, message->signature);
				errno = EINVAL;
				goto err;
			}

			closure->args[i].s = s;
			p = next;
			break;
		case 'o':
			id = *p++;
			closure->args[i].n = id;

			if (id == 0 && !arg.nullable) {
				wl_log("NULL object received on non-nullable "
				       "type, message %s(%s)\n", message->name,
				       message->signature);
				errno = EINVAL;
				goto err;
			}
			break;
		case 'n':
			id = *p++;
			closure->args[i].n = id;

			if (id == 0) {
				wl_log("NULL new ID received on non-nullable "
				       "type, message %s(%s)\n", message->name,
				       message->signature);
				errno = EINVAL;
				goto err;
			}

			if (wl_map_reserve_new(objects, id) < 0) {
				if (errno == EINVAL) {
					wl_log("not a valid new object id (%u), "
					       "message %s(%s)\n", id,
					       message->name,
					       message->signature);
				}
				goto err;
			}

			break;
		case 'a':
			length = *p++;

			length_in_u32 = div_roundup(length, sizeof *p);
			if ((uint32_t) (end - p) < length_in_u32) {
				wl_log("message too short, "
				       "object (%d), message %s(%s)\n",
				       closure->sender_id, message->name,
				       message->signature);
				errno = EINVAL;
				goto err;
			}
			next = p + length_in_u32;

			array_extra->size = length;
			array_extra->alloc = 0;
			array_extra->data = p;

			closure->args[i].a = array_extra++;
			p = next;
			break;
		case 'h':
			if (connection->fds_in.tail == connection->fds_in.head) {
				wl_log("file descriptor expected, "
				       "object (%d), message %s(%s)\n",
				       closure->sender_id, message->name,
				       message->signature);
				errno = EINVAL;
				goto err;
			}

			ring_buffer_copy(&connection->fds_in, &fd, sizeof fd);
			connection->fds_in.tail += sizeof fd;
			closure->args[i].h = fd;
			break;
		default:
			wl_abort("unknown type\n");
			break;
		}
	}

	wl_connection_consume(connection, size);

	return closure;

 err:
	wl_closure_destroy(closure);
	wl_connection_consume(connection, size);

	return NULL;
}

bool
wl_object_is_zombie(struct wl_map *map, uint32_t id)
{
	uint32_t flags;

	/* Zombie objects only exist on the client side. */
	if (map->side == WL_MAP_SERVER_SIDE)
		return false;

	/* Zombie objects can only have been created by the client. */
	if (id >= WL_SERVER_ID_START)
		return false;

	flags = wl_map_lookup_flags(map, id);
	return !!(flags & WL_MAP_ENTRY_ZOMBIE);
}

int
wl_closure_lookup_objects(struct wl_closure *closure, struct wl_map *objects)
{
	struct wl_object *object;
	const struct wl_message *message;
	const char *signature;
	struct argument_details arg;
	int i, count;
	uint32_t id;

	message = closure->message;
	signature = message->signature;
	count = arg_count_for_signature(signature);
	for (i = 0; i < count; i++) {
		signature = get_next_argument(signature, &arg);
		switch (arg.type) {
		case 'o':
			id = closure->args[i].n;
			closure->args[i].o = NULL;

			object = wl_map_lookup(objects, id);
			if (wl_object_is_zombie(objects, id)) {
				/* references object we've already
				 * destroyed client side */
				object = NULL;
			} else if (object == NULL && id != 0) {
				wl_log("unknown object (%u), message %s(%s)\n",
				       id, message->name, message->signature);
				errno = EINVAL;
				return -1;
			}

			if (object != NULL && message->types[i] != NULL &&
			    !wl_interface_equal((object)->interface,
						message->types[i])) {
				wl_log("invalid object (%u), type (%s), "
				       "message %s(%s)\n",
				       id, (object)->interface->name,
				       message->name, message->signature);
				errno = EINVAL;
				return -1;
			}
			closure->args[i].o = object;
		}
	}

	return 0;
}

static void
convert_arguments_to_ffi(const char *signature, uint32_t flags,
			 union wl_argument *args,
			 int count, ffi_type **ffi_types, void** ffi_args)
{
	int i;
	const char *sig_iter;
	struct argument_details arg;

	sig_iter = signature;
	for (i = 0; i < count; i++) {
		sig_iter = get_next_argument(sig_iter, &arg);

		switch(arg.type) {
		case 'i':
			ffi_types[i] = &ffi_type_sint32;
			ffi_args[i] = &args[i].i;
			break;
		case 'u':
			ffi_types[i] = &ffi_type_uint32;
			ffi_args[i] = &args[i].u;
			break;
		case 'f':
			ffi_types[i] = &ffi_type_sint32;
			ffi_args[i] = &args[i].f;
			break;
		case 's':
			ffi_types[i] = &ffi_type_pointer;
			ffi_args[i] = &args[i].s;
			break;
		case 'o':
			ffi_types[i] = &ffi_type_pointer;
			ffi_args[i] = &args[i].o;
			break;
		case 'n':
			if (flags & WL_CLOSURE_INVOKE_CLIENT) {
				ffi_types[i] = &ffi_type_pointer;
				ffi_args[i] = &args[i].o;
			} else {
				ffi_types[i] = &ffi_type_uint32;
				ffi_args[i] = &args[i].n;
			}
			break;
		case 'a':
			ffi_types[i] = &ffi_type_pointer;
			ffi_args[i] = &args[i].a;
			break;
		case 'h':
			ffi_types[i] = &ffi_type_sint32;
			ffi_args[i] = &args[i].h;
			break;
		default:
			wl_abort("unknown type\n");
			break;
		}
	}
}

void
wl_closure_invoke(struct wl_closure *closure, uint32_t flags,
		  struct wl_object *target, uint32_t opcode, void *data)
{
	int count;
	ffi_cif cif;
	ffi_type *ffi_types[WL_CLOSURE_MAX_ARGS + 2];
	void * ffi_args[WL_CLOSURE_MAX_ARGS + 2];
	void (* const *implementation)(void);

	count = arg_count_for_signature(closure->message->signature);

	ffi_types[0] = &ffi_type_pointer;
	ffi_args[0] = &data;
	ffi_types[1] = &ffi_type_pointer;
	ffi_args[1] = &target;

	convert_arguments_to_ffi(closure->message->signature, flags, closure->args,
				 count, ffi_types + 2, ffi_args + 2);

	ffi_prep_cif(&cif, FFI_DEFAULT_ABI,
		     count + 2, &ffi_type_void, ffi_types);

	implementation = target->implementation;
	if (!implementation[opcode]) {
		wl_abort("listener function for opcode %u of %s is NULL\n",
			 opcode, target->interface->name);
	}
	ffi_call(&cif, implementation[opcode], NULL, ffi_args);

	wl_closure_clear_fds(closure);
}

void
wl_closure_dispatch(struct wl_closure *closure, wl_dispatcher_func_t dispatcher,
		    struct wl_object *target, uint32_t opcode)
{
	dispatcher(target->implementation, target, opcode, closure->message,
		   closure->args);

	wl_closure_clear_fds(closure);
}

static int
copy_fds_to_connection(struct wl_closure *closure,
		       struct wl_connection *connection)
{
	const struct wl_message *message = closure->message;
	uint32_t i, count;
	struct argument_details arg;
	const char *signature = message->signature;
	int fd;

	count = arg_count_for_signature(signature);
	for (i = 0; i < count; i++) {
		signature = get_next_argument(signature, &arg);
		if (arg.type != 'h')
			continue;

		fd = closure->args[i].h;
		if (wl_connection_put_fd(connection, fd)) {
			wl_log("request could not be marshaled: "
			       "can't send file descriptor\n");
			return -1;
		}
		closure->args[i].h = -1;
	}

	return 0;
}


static uint32_t
buffer_size_for_closure(struct wl_closure *closure)
{
	const struct wl_message *message = closure->message;
	int i, count;
	struct argument_details arg;
	const char *signature;
	uint32_t size, buffer_size = 0;

	signature = message->signature;
	count = arg_count_for_signature(signature);
	for (i = 0; i < count; i++) {
		signature = get_next_argument(signature, &arg);

		switch (arg.type) {
		case 'h':
			break;
		case 'u':
		case 'i':
		case 'f':
		case 'o':
		case 'n':
			buffer_size++;
			break;
		case 's':
			if (closure->args[i].s == NULL) {
				buffer_size++;
				break;
			}

			size = strlen(closure->args[i].s) + 1;
			buffer_size += 1 + div_roundup(size, sizeof(uint32_t));
			break;
		case 'a':
			if (closure->args[i].a == NULL) {
				buffer_size++;
				break;
			}

			size = closure->args[i].a->size;
			buffer_size += (1 + div_roundup(size, sizeof(uint32_t)));
			break;
		default:
			break;
		}
	}

	return buffer_size + 2;
}

static int
serialize_closure(struct wl_closure *closure, uint32_t *buffer,
		  size_t buffer_count)
{
	const struct wl_message *message = closure->message;
	unsigned int i, count, size;
	uint32_t *p, *end;
	struct argument_details arg;
	const char *signature;

	if (buffer_count < 2)
		goto overflow;

	p = buffer + 2;
	end = buffer + buffer_count;

	signature = message->signature;
	count = arg_count_for_signature(signature);
	for (i = 0; i < count; i++) {
		signature = get_next_argument(signature, &arg);

		if (arg.type == 'h')
			continue;

		if (p + 1 > end)
			goto overflow;

		switch (arg.type) {
		case 'u':
			*p++ = closure->args[i].u;
			break;
		case 'i':
			*p++ = closure->args[i].i;
			break;
		case 'f':
			*p++ = closure->args[i].f;
			break;
		case 'o':
			*p++ = closure->args[i].o ? closure->args[i].o->id : 0;
			break;
		case 'n':
			*p++ = closure->args[i].n;
			break;
		case 's':
			if (closure->args[i].s == NULL) {
				*p++ = 0;
				break;
			}

			size = strlen(closure->args[i].s) + 1;
			*p++ = size;

			if (p + div_roundup(size, sizeof *p) > end)
				goto overflow;

			memcpy(p, closure->args[i].s, size);
			p += div_roundup(size, sizeof *p);
			break;
		case 'a':
			if (closure->args[i].a == NULL) {
				*p++ = 0;
				break;
			}

			size = closure->args[i].a->size;
			*p++ = size;

			if (p + div_roundup(size, sizeof *p) > end)
				goto overflow;

			memcpy(p, closure->args[i].a->data, size);
			p += div_roundup(size, sizeof *p);
			break;
		default:
			break;
		}
	}

	size = (p - buffer) * sizeof *p;

	buffer[0] = closure->sender_id;
	buffer[1] = size << 16 | (closure->opcode & 0x0000ffff);

	return size;

overflow:
	errno = ERANGE;
	return -1;
}

int
wl_closure_send(struct wl_closure *closure, struct wl_connection *connection)
{
	int size;
	uint32_t buffer_size;
	uint32_t *buffer;
	int result;

	if (copy_fds_to_connection(closure, connection))
		return -1;

	buffer_size = buffer_size_for_closure(closure);
	buffer = zalloc(buffer_size * sizeof buffer[0]);
	if (buffer == NULL)
		return -1;

	size = serialize_closure(closure, buffer, buffer_size);
	if (size < 0) {
		free(buffer);
		return -1;
	}

	result = wl_connection_write(connection, buffer, size);
	free(buffer);

	return result;
}

int
wl_closure_queue(struct wl_closure *closure, struct wl_connection *connection)
{
	int size;
	uint32_t buffer_size;
	uint32_t *buffer;
	int result;

	if (copy_fds_to_connection(closure, connection))
		return -1;

	buffer_size = buffer_size_for_closure(closure);
	buffer = malloc(buffer_size * sizeof buffer[0]);
	if (buffer == NULL)
		return -1;

	size = serialize_closure(closure, buffer, buffer_size);
	if (size < 0) {
		free(buffer);
		return -1;
	}

	result = wl_connection_queue(connection, buffer, size);
	free(buffer);

	return result;
}

void
wl_closure_print(struct wl_closure *closure, struct wl_object *target,
		 int send, int discarded, uint32_t (*n_parse)(union wl_argument *arg))
{
	int i;
	struct argument_details arg;
	const char *signature = closure->message->signature;
	struct timespec tp;
	unsigned int time;
	uint32_t nval;
	FILE *f;
	char *buffer;
	size_t buffer_length;

	f = open_memstream(&buffer, &buffer_length);
	if (f == NULL)
		return;

	clock_gettime(CLOCK_REALTIME, &tp);
	time = (tp.tv_sec * 1000000L) + (tp.tv_nsec / 1000);

	fprintf(f, "[%7u.%03u] %s%s%s@%u.%s(",
		time / 1000, time % 1000,
		discarded ? "discarded " : "",
		send ? " -> " : "",
		target->interface->name, target->id,
		closure->message->name);

	for (i = 0; i < closure->count; i++) {
		signature = get_next_argument(signature, &arg);
		if (i > 0)
			fprintf(f, ", ");

		switch (arg.type) {
		case 'u':
			fprintf(f, "%u", closure->args[i].u);
			break;
		case 'i':
			fprintf(f, "%d", closure->args[i].i);
			break;
		case 'f':
			/* The magic number 390625 is 1e8 / 256 */
			if (closure->args[i].f >= 0) {
				fprintf(f, "%d.%08d",
					closure->args[i].f / 256,
					390625 * (closure->args[i].f % 256));
			} else {

				fprintf(f, "-%d.%08d",
					closure->args[i].f / -256,
					-390625 * (closure->args[i].f % 256));
			}
			break;
		case 's':
			if (closure->args[i].s)
				fprintf(f, "\"%s\"", closure->args[i].s);
			else
				fprintf(f, "nil");
			break;
		case 'o':
			if (closure->args[i].o)
				fprintf(f, "%s@%u",
					closure->args[i].o->interface->name,
					closure->args[i].o->id);
			else
				fprintf(f, "nil");
			break;
		case 'n':
			if (n_parse)
				nval = n_parse(&closure->args[i]);
			else
				nval = closure->args[i].n;

			fprintf(f, "new id %s@",
				(closure->message->types[i]) ?
				 closure->message->types[i]->name :
				  "[unknown]");
			if (nval != 0)
				fprintf(f, "%u", nval);
			else
				fprintf(f, "nil");
			break;
		case 'a':
			fprintf(f, "array[%zu]", closure->args[i].a->size);
			break;
		case 'h':
			fprintf(f, "fd %d", closure->args[i].h);
			break;
		}
	}

	fprintf(f, ")\n");

	if (fclose(f) == 0) {
		fprintf(stderr, "%s", buffer);
		free(buffer);
	}
}

static int
wl_closure_close_fds(struct wl_closure *closure)
{
	int i;
	struct argument_details arg;
	const char *signature = closure->message->signature;

	for (i = 0; i < closure->count; i++) {
		signature = get_next_argument(signature, &arg);
		if (arg.type == 'h' && closure->args[i].h != -1)
			close(closure->args[i].h);
	}

	return 0;
}

void
wl_closure_destroy(struct wl_closure *closure)
{
	/* wl_closure_destroy has free() semantics */
	if (!closure)
		return;

	wl_closure_close_fds(closure);
	free(closure);
}
