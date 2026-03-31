#+
# Copyright 2015 iXsystems, Inc.
# All rights reserved
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted providing that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES LOSS OF USE, DATA, OR PROFITS OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
# IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
#####################################################################

from libc.stdint cimport *
from posix.types cimport gid_t, pid_t, uid_t


cdef extern from "uuid/uuid.h":
    ctypedef struct uuid_t:
        pass


cdef extern from "dispatch/dispatch.h":
    ctypedef struct dispatch_queue_t:
        pass

    ctypedef struct dispatch_data_t:
        pass

    ctypedef struct dispatch_block_t:
        pass


cdef extern from "blocks_wrapper.h" nogil:
    ctypedef int (*xpc_dictionary_applier_func_t)(const char *, xpc_object_t, void *)
    ctypedef int (*xpc_array_applier_func_t)(size_t, xpc_object_t, void *)
    ctypedef void (*xpc_handler_func_t)(xpc_object_t, void *)

    cdef int xpc_dictionary_apply_f(xpc_object_t, xpc_dictionary_applier_func_t,
        void *)
    cdef int xpc_array_apply_f(xpc_object_t, xpc_array_applier_func_t, void *)
    cdef void xpc_connection_set_event_handler_f(xpc_connection_t, xpc_handler_func_t,
        void *)


cdef extern from "xpc/xpc.h" nogil:
    ctypedef struct xpc_connection_t:
        pass

    ctypedef struct xpc_endpoint_t:
        pass

    ctypedef struct xpc_object_t:
        pass

    ctypedef enum xpc_type_t:
        XPC_TYPE_CONNECTION
        XPC_TYPE_ENDPOINT
        XPC_TYPE_NULL
        XPC_TYPE_BOOL
        XPC_TYPE_INT64
        XPC_TYPE_UINT64
        XPC_TYPE_DOUBLE
        XPC_TYPE_DATE
        XPC_TYPE_DATA
        XPC_TYPE_STRING
        XPC_TYPE_UUID
        XPC_TYPE_FD
        XPC_TYPE_SHMEM
        XPC_TYPE_ARRAY
        XPC_TYPE_DICTIONARY
        XPC_TYPE_ERROR

    enum:
        XPC_CONNECTION_MACH_SERVICE_LISTENER

    cdef void xpc_retain(xpc_object_t object)
    cdef void xpc_release(xpc_object_t object)
    cdef xpc_object_t xpc_copy(xpc_object_t object)
    cdef xpc_type_t xpc_get_type(xpc_object_t object);
    cdef int xpc_equal(xpc_object_t object1, xpc_object_t object2)
    cdef size_t xpc_hash(xpc_object_t object)
    cdef char* xpc_copy_description(xpc_object_t object)

    cdef xpc_object_t xpc_null_create()
    cdef xpc_object_t xpc_int_create(int value)
    cdef int xpc_int_get_value(xpc_object_t xint)
    cdef xpc_object_t xpc_int64_create(int64_t value)
    cdef int64_t xpc_int64_get_value(xpc_object_t xint)
    cdef xpc_object_t xpc_uint64_create(uint64_t value)
    cdef uint64_t xpc_uint64_get_value(xpc_object_t xuint)
    cdef xpc_object_t xpc_double_create(double value)
    cdef double xpc_double_get_value(xpc_object_t xdouble)
    cdef xpc_object_t xpc_date_create(int64_t interval)
    cdef xpc_object_t xpc_date_create_from_current()
    cdef int64_t xpc_date_get_value(xpc_object_t xdate)
    cdef xpc_object_t xpc_data_create(const void *bytes, size_t length)
    cdef xpc_object_t xpc_data_create_with_dispatch_data(dispatch_data_t ddata)
    cdef size_t xpc_data_get_length(xpc_object_t xdata)
    cdef const void * xpc_data_get_bytes_ptr(xpc_object_t xdata)
    cdef size_t xpc_data_get_bytes(xpc_object_t xdata, void *buffer, size_t off, size_t length)
    cdef xpc_object_t xpc_string_create(const char *string)
    cdef size_t xpc_string_get_length(xpc_object_t xstring)
    cdef const char * xpc_string_get_string_ptr(xpc_object_t xstring)
    cdef xpc_object_t xpc_uuid_create(const uuid_t uuid)
    cdef const uint8_t * xpc_uuid_get_bytes(xpc_object_t xuuid)
    cdef xpc_object_t xpc_fd_create(int fd)
    cdef intxpc_fd_dup(xpc_object_t xfd)
    cdef xpc_object_t xpc_shmem_create(void *region, size_t length)
    cdef size_t xpc_shmem_map(xpc_object_t xshmem, void **region)

    cdef xpc_object_t xpc_dictionary_create(const char * const *keys, const xpc_object_t *values, size_t count)
    cdef xpc_object_t xpc_dictionary_create_reply(xpc_object_t original)
    cdef void xpc_dictionary_set_value(xpc_object_t dictionary, const char *key, xpc_object_t value)
    cdef xpc_object_t xpc_dictionary_get_value(xpc_object_t dictionary, const char *key)
    cdef size_t xpc_dictionary_get_count(xpc_object_t dictionary)
    cdef xpc_connection_t xpc_dictionary_get_remote_connection(xpc_object_t dictionary)
    cdef void xpc_dictionary_set_int(xpc_object_t dictionary, const char *key, int value)
    cdef void xpc_dictionary_set_int64(xpc_object_t dictionary, const char *key, int64_t value)
    cdef void xpc_dictionary_set_uint64(xpc_object_t dictionary, const char *key, uint64_t value)
    cdef void xpc_dictionary_set_double(xpc_object_t dictionary, const char *key, double value)
    cdef void xpc_dictionary_set_date(xpc_object_t dictionary, const char *key, int64_t value)
    cdef void xpc_dictionary_set_data(xpc_object_t dictionary, const char *key, const void *value, size_t length)
    cdef void xpc_dictionary_set_string(xpc_object_t dictionary, const char *key, const char *value)
    cdef void xpc_dictionary_set_uuid(xpc_object_t dictionary, const char *key, const uuid_t value)
    cdef void xpc_dictionary_set_fd(xpc_object_t dictionary, const char *key, int value)
    cdef void xpc_dictionary_set_connection(xpc_object_t dictionary, const char *key, xpc_connection_t connection)
    cdef int xpc_dictionary_get_int(xpc_object_t dictionary, const char *key)
    cdef int64_t xpc_dictionary_get_int64(xpc_object_t dictionary, const char *key)
    cdef uint64_t xpc_dictionary_get_uint64(xpc_object_t dictionary, const char *key)
    cdef double xpc_dictionary_get_double(xpc_object_t dictionary, const char *key)
    cdef int64_t xpc_dictionary_get_date(xpc_object_t dictionary, const char *key)
    cdef const void *xpc_dictionary_get_data(xpc_object_t dictionary, const char *key, size_t *length)
    cdef const uint8_t *xpc_dictionary_get_uuid(xpc_object_t dictionary, const char *key)
    cdef const char *xpc_dictionary_get_string(xpc_object_t dictionary, const char *key)
    cdef int xpc_dictionary_dup_fd(xpc_object_t dictionary, const char *key)
    cdef xpc_connection_t xpc_dictionary_get_connection(xpc_object_t dictionary, const char *key)

    cdef xpc_object_t xpc_array_create(const xpc_object_t *objects, size_t count)
    cdef void xpc_array_set_value(xpc_object_t array, size_t index, xpc_object_t value)
    cdef void xpc_array_append_value(xpc_object_t array, xpc_object_t value)
    cdef xpc_object_t xpc_array_get_value(xpc_object_t array, size_t index)
    cdef size_t xpc_array_get_count(xpc_object_t array)
    cdef void xpc_array_set_int(xpc_object_t array, size_t index, int value)
    cdef void xpc_array_set_int64(xpc_object_t array, size_t index, int64_t value)
    cdef void xpc_array_set_uint64(xpc_object_t array, size_t index, uint64_t value)
    cdef void xpc_array_set_double(xpc_object_t array, size_t index, double value)
    cdef void xpc_array_set_date(xpc_object_t array, size_t index, int64_t value)
    cdef void xpc_array_set_data(xpc_object_t array, size_t index, const void *bytes, size_t length)
    cdef void xpc_array_set_string(xpc_object_t array, size_t index, const char *value)
    cdef void xpc_array_set_uuid(xpc_object_t array, size_t index, const uuid_t value)
    cdef void xpc_array_set_fd(xpc_object_t array, size_t index, int value)
    cdef void xpc_array_set_connection(xpc_object_t array, size_t index, xpc_connection_t value)
    cdef int xpc_array_get_int(xpc_object_t array, size_t index)
    cdef int64_t xpc_array_get_int64(xpc_object_t array, size_t index)
    cdef uint64_t xpc_array_get_uint64(xpc_object_t array, size_t index)
    cdef double xpc_array_get_double(xpc_object_t array, size_t index)
    cdef int64_t xpc_array_get_date(xpc_object_t array, size_t index)
    cdef const void * xpc_array_get_data(xpc_object_t array, size_t index, size_t *length)
    cdef const uint8_t * xpc_array_get_uuid(xpc_object_t array, size_t index)
    cdef const char * xpc_array_get_string(xpc_object_t array, size_t index)
    cdef int xpc_array_get_fd(xpc_object_t array, size_t index)
    cdef xpc_connection_t xpc_array_get_connection(xpc_object_t array, size_t index)

    cdef xpc_connection_t xpc_connection_create(const char *name, dispatch_queue_t targetq)
    cdef xpc_connection_t xpc_connection_create_mach_service(const char *name, dispatch_queue_t targetq, uint64_t flags)
    cdef xpc_connection_t xpc_connection_create_from_endpoint(xpc_endpoint_t endpoint)
    cdef void xpc_connection_set_target_queue(xpc_connection_t connection, dispatch_queue_t targetq)
    cdef void xpc_connection_suspend(xpc_connection_t connection)
    cdef void xpc_connection_resume(xpc_connection_t connection)
    cdef void xpc_connection_send_message(xpc_connection_t connection, xpc_object_t message)
    cdef void xpc_connection_send_barrier(xpc_connection_t connection, dispatch_block_t barrier)
    cdef xpc_object_t xpc_connection_send_message_with_reply_sync(xpc_connection_t connection, xpc_object_t message)
    cdef void xpc_connection_cancel(xpc_connection_t connection)
    cdef const char* xpc_connection_get_name(xpc_connection_t connection)
    cdef uid_t xpc_connection_get_euid(xpc_connection_t connection)
#    cdef gid_t xpc_connection_get_gid(xpc_connection_t connection)
    cdef pid_t xpc_connection_get_pid(xpc_connection_t connection)
#    cdef au_asid_t xpc_connection_get_asid(xpc_connection_t connection)
    cdef void xpc_connection_set_context(xpc_connection_t connection, void *ctx)
    cdef void* xpc_connection_get_context(xpc_connection_t connection)
#    cdef void xpc_connection_set_finalizer_f(xpc_connection_t connection, xpc_finalizer_t finalizer)
    cdef xpc_endpoint_t xpc_endpoint_create(xpc_connection_t connection)
    cdef void xpc_main(void *handler)