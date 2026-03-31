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

import enum
import uuid
cimport xpc
from libc.stdint cimport *


class XPCType(enum.IntEnum):
    CONNECTION = xpc.XPC_TYPE_CONNECTION
    ENDPOINT = xpc.XPC_TYPE_ENDPOINT
    NONE = xpc.XPC_TYPE_NULL
    BOOL = xpc.XPC_TYPE_BOOL
    INT64 = xpc.XPC_TYPE_INT64
    UINT64 = xpc.XPC_TYPE_UINT64
    DOUBLE = xpc.XPC_TYPE_DOUBLE
    DATE = xpc.XPC_TYPE_DATE
    DATA = xpc.XPC_TYPE_DATA
    STRING = xpc.XPC_TYPE_STRING
    UUID = xpc.XPC_TYPE_UUID
    FD = xpc.XPC_TYPE_FD
    SHMEM = xpc.XPC_TYPE_SHMEM
    ARRAY = xpc.XPC_TYPE_ARRAY
    DICTIONARY = xpc.XPC_TYPE_DICTIONARY
    ERROR = xpc.XPC_TYPE_ERROR


class XPCConnectionFlags(enum.IntEnum):
    MACH_SERVICE_LISTENER = xpc.XPC_CONNECTION_MACH_SERVICE_LISTENER


cdef class XPCObject(object):
    cdef xpc.xpc_object_t obj

    def __init__(self, value=None, type=None, ptr=0):
        if type:
            if type == XPCType.ARRAY:
                self.obj = xpc.xpc_array_create(NULL, 0)

            if type == XPCType.DICTIONARY:
                self.obj = xpc.xpc_dictionary_create(NULL, NULL, 0)

        if value is not None:
            self.value = value

        if ptr:
            self.obj = <xpc.xpc_object_t><uintptr_t>ptr

    def __dealloc__(self):
        #if <void*>self.obj != NULL:
        #    xpc.xpc_release(self.obj)
        pass

    property ptr:
        def __get__(self):
            return <uintptr_t>self.obj

    property type:
        def __get__(self):
            return XPCType(xpc.xpc_get_type(self.obj))

    property value:
        def __get__(self):
            if self.type == XPCType.NONE:
                return None

            if self.type == XPCType.BOOL:
                return xpc.xpc_bool_get_value(self.obj)

            if self.type == XPCType.DOUBLE:
                return xpc.xpc_double_get_value(self.obj)

            if self.type == XPCType.INT64:
                return xpc.xpc_int64_get_value(self.obj)

            if self.type == XPCType.UINT64:
                return xpc.xpc_uint64_get_value(self.obj)

            if self.type == XPCType.STRING:
                return xpc.xpc_string_get_string_ptr(self.obj)

            if self.type == XPCType.UUID:
                return uuid.UUID(bytes=xpc.xpc_uuid_get_bytes(self.obj))

            if self.type == XPCType.CONNECTION:
                return XPCConnection(ptr=<uintptr_t>self.obj)

        def __set__(self, value):
            if value is None:
                self.obj = xpc.xpc_null_create()

            if isinstance(value, XPCObject):
                self.obj = xpc.xpc_copy(<xpc_object_t><uintptr_t>value.ptr)

            if isinstance(value, basestring):
                self.obj = xpc.xpc_string_create(value)

            if isinstance(value, (int, long)):
                self.obj = xpc.xpc_int64_create(value)

            if isinstance(value, bool):
                self.obj = xpc.xpc_bool_create(value)

            if isinstance(value, (list, tuple)):
                self.obj = xpc.xpc_array_create(NULL, 0)
                for i in value:
                    self.append(i)

            if isinstance(value, dict):
                self.obj = xpc.xpc_dictionary_create(NULL, NULL, 0)
                for k, v in value.items():
                    self[k] = v

    def __repr__(self):
        return xpc.xpc_copy_description(self.obj)

    def __getitem__(self, item):
        cdef xpc.xpc_object_t obj

        if self.type == XPCType.ARRAY:
            obj = xpc.xpc_array_get_value(self.obj, item)
            return XPCObject(ptr=<uintptr_t>obj)

        if self.type in (XPCType.DICTIONARY, XPCType.ERROR):
            obj = xpc.xpc_dictionary_get_value(self.obj, item)
            return XPCObject(ptr=<uintptr_t>obj)

        raise NotImplementedError()

    def __setitem__(self, key, value):
        if self.type == XPCType.ARRAY:
            xpc.xpc_array_set_value(self.obj, key, value.obj)
            return

        if self.type == XPCType.DICTIONARY:
            xpc.xpc_dictionary_set_value(
                self.obj,
                key,
                <xpc.xpc_object_t><uintptr_t>XPCObject(value).ptr)

            return


        raise NotImplementedError()

    def keys(self):
        if self.type != XPCType.DICTIONARY:
            raise NotImplementedError()

    def values(self):
        if self.type != XPCType.DICTIONARY:
            raise NotImplementedError()

    def items(self):
        if self.type != XPCType.DICTIONARY:
            raise NotImplementedError()


    def append(self, value):
        if self.type != XPCType.ARRAY:
            raise NotImplementedError()

        xpc.xpc_array_append_value(
            self.obj,
            <xpc.xpc_object_t><uintptr_t>XPCObject(value).ptr)

    def update(self, other):
        pass


cdef class XPCConnection(object):
    cdef xpc.xpc_connection_t conn
    cdef readonly children
    cdef readonly listener
    cdef public object on_event

    def __init__(self, name=None, listener=False, ptr=0):
        if ptr:
            self.conn = <xpc.xpc_connection_t><uintptr_t>ptr
        else:
            flags = XPCConnectionFlags.MACH_SERVICE_LISTENER if listener else 0
            self.conn = xpc.xpc_connection_create_mach_service(name, <dispatch_queue_t>NULL, flags)

        self.listener = listener
        self.on_event = None
        self.children = []
        xpc.xpc_connection_set_event_handler_f(self.conn, self.event_handler, <void*>self)

    @staticmethod
    cdef void event_handler(xpc.xpc_object_t obj, void* context) with gil:
        self = <object>context
        if self.listener:
            conn = XPCConnection(ptr=<uintptr_t>obj)
            self.children.append(conn)
            self.on_event(self, conn)
        else:
            msg = XPCObject(ptr=<uintptr_t>obj)
            self.on_event(self, msg)

    property remote_uid:
        def __get__(self):
            return xpc.xpc_connection_get_euid(self.conn)

#    property remote_gid:
#        def __get__(self):
#            return xpc.xpc_connection_get_gid(self.conn)

    property remote_pid:
        def __get__(self):
            return xpc.xpc_connection_get_pid(self.conn)

    def resume(self):
        if not self.on_event:
            raise RuntimeError('Set event handler first')

        xpc.xpc_connection_resume(self.conn)

    def send(self, obj):
        if not isinstance(obj, XPCObject):
            raise ValueError('Only XPCObject instances can be sent')

        if obj.type != XPCType.DICTIONARY:
            raise ValueError('Only XPCObjects of type DICTIONARY can be sent')

        xpc.xpc_connection_send_message(self.conn, <xpc_object_t><uintptr_t>obj.ptr)

    def send_with_reply(self, obj, callback):
        pass

    def send_with_reply_sync(self, obj):
        pass


def main():
    with nogil:
        xpc.xpc_main(NULL);
