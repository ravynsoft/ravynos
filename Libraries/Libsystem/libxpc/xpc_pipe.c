/*
 * Compatibility stubs for private xpc_pipe APIs using regular unix pipes
 */

#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "xpc/launchd.h"

struct xpc_pipe_state {
    xpc_pipe_t handle;
    int read_fd;
    int write_fd;
    uint64_t flags;
    int valid;
    struct xpc_pipe_state *next;
};

static pthread_mutex_t s_pipe_lock = PTHREAD_MUTEX_INITIALIZER;
static struct xpc_pipe_state *s_pipes;

static struct xpc_pipe_state *
xpc_pipe_state_find_nolock(xpc_pipe_t handle)
{
    struct xpc_pipe_state *it = s_pipes;
    while (it != NULL) {
        if (it->handle == handle)
            return it;
        it = it->next;
    }
    return NULL;
}

xpc_pipe_t
xpc_pipe_create(const char *name, uint64_t flags)
{
    int fds[2];
    struct xpc_pipe_state *state;
    xpc_pipe_t handle;

    (void)name;

    if (pipe(fds) != 0)
        return NULL;

    handle = xpc_dictionary_create(NULL, NULL, 0);
    if (handle == NULL) {
        close(fds[0]);
        close(fds[1]);
        return NULL;
    }

    state = calloc(1, sizeof(*state));
    if (state == NULL) {
        close(fds[0]);
        close(fds[1]);
        xpc_release(handle);
        return NULL;
    }

    state->handle = handle;
    state->read_fd = fds[0];
    state->write_fd = fds[1];
    state->flags = flags;
    state->valid = 1;

    pthread_mutex_lock(&s_pipe_lock);
    state->next = s_pipes;
    s_pipes = state;
    pthread_mutex_unlock(&s_pipe_lock);

    return handle;
}

void
xpc_pipe_invalidate(xpc_pipe_t pipe)
{
    pthread_mutex_lock(&s_pipe_lock);
    struct xpc_pipe_state *prev = NULL;
    struct xpc_pipe_state *state = s_pipes;
    while (state != NULL) {
        if (state->handle == pipe)
            break;
        prev = state;
        state = state->next;
    }
    if (state != NULL) {
        if (prev == NULL)
            s_pipes = state->next;
        else
            prev->next = state->next;

        if (state->valid) {
            state->valid = 0;
            close(state->read_fd);
            close(state->write_fd);
        }
        free(state);
    }
    pthread_mutex_unlock(&s_pipe_lock);
}

int
xpc_pipe_routine(xpc_pipe_t pipe, xpc_object_t msg, xpc_object_t *reply)
{
    struct xpc_pipe_state *state;
    char byte = 0;
    ssize_t n;

    if (reply != NULL)
        *reply = NULL;

    if (pipe == NULL || msg == NULL)
        return EINVAL;

    pthread_mutex_lock(&s_pipe_lock);
    state = xpc_pipe_state_find_nolock(pipe);
    if (state == NULL || !state->valid) {
        pthread_mutex_unlock(&s_pipe_lock);
        return EPIPE;
    }
    int write_fd = state->write_fd;
    int read_fd = state->read_fd;
    pthread_mutex_unlock(&s_pipe_lock);

    /* Check for transport failure */
    n = write(write_fd, &byte, 1);
    if (n != 1)
        return EPIPE;
    n = read(read_fd, &byte, 1);
    if (n != 1)
        return EPIPE;

    if (reply != NULL)
        *reply = xpc_dictionary_create(NULL, NULL, 0);

    return 0;
}


