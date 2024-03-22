/**************************************************************************
 *
 * Copyright 2008 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

/**
 * @file
 * Trace data dumping primitives.
 */

#ifndef TR_DUMP_H
#define TR_DUMP_H


#include "util/compiler.h"
#include "util/format/u_formats.h"

struct pipe_resource;
struct pipe_surface;
struct pipe_transfer;
struct pipe_box;

/*
 * Low level dumping controls.
 *
 * Opening the trace file and checking if that is opened.
 */
bool trace_dump_trace_begin(void);
bool trace_dump_trace_enabled(void);
void trace_dump_trace_flush(void);

/*
 * Lock and unlock the call mutex.
 *
 * It used by the none locked version of dumping control
 * and begin/end call dump functions.
 *
 * Begin takes the lock while end unlocks it. Use the _locked
 * version to avoid locking/unlocking it.
 */
void trace_dump_call_lock(void);
void trace_dump_call_unlock(void);

/*
 * High level dumping control.
 */
void trace_dumping_start_locked(void);
void trace_dumping_stop_locked(void);
bool trace_dumping_enabled_locked(void);
void trace_dumping_start(void);
void trace_dumping_stop(void);
bool trace_dumping_enabled(void);

void trace_dump_call_begin_locked(const char *klass, const char *method);
void trace_dump_call_end_locked(void);
void trace_dump_call_begin(const char *klass, const char *method);
void trace_dump_call_end(void);

void trace_dump_arg_begin(const char *name);
void trace_dump_arg_end(void);
void trace_dump_ret_begin(void);
void trace_dump_ret_end(void);
void trace_dump_bool(bool value);
void trace_dump_int(int64_t value);
void trace_dump_uint(uint64_t value);
void trace_dump_float(double value);
void trace_dump_bytes(const void *data, size_t size);
void trace_dump_box_bytes(const void *data,
                          struct pipe_resource *resource,
			  const struct pipe_box *box,
			  unsigned stride,
			  uint64_t slice_stride);
void trace_dump_string(const char *str);
void trace_dump_enum(const char *value);
void trace_dump_array_begin(void);
void trace_dump_array_end(void);
void trace_dump_elem_begin(void);
void trace_dump_elem_end(void);
void trace_dump_struct_begin(const char *name);
void trace_dump_struct_end(void);
void trace_dump_member_begin(const char *name);
void trace_dump_member_end(void);
void trace_dump_null(void);
void trace_dump_ptr(const void *value);
/* will turn a wrapped object into the real one and dump ptr */
void trace_dump_surface_ptr(struct pipe_surface *_surface);
void trace_dump_transfer_ptr(struct pipe_transfer *_transfer);
void trace_dump_nir(void *nir);

void trace_dump_trigger_active(bool active);
void trace_dump_check_trigger(void);
bool trace_dump_is_triggered(void);

/*
 * Code saving macros.
 */

#define trace_dump_arg(_type, _arg) \
   do { \
      trace_dump_arg_begin(#_arg); \
      trace_dump_##_type(_arg); \
      trace_dump_arg_end(); \
   } while(0)

#define trace_dump_arg_enum(_type, _arg) \
   do { \
      trace_dump_arg_begin(#_arg); \
      trace_dump_enum(tr_util_##_type##_name(_arg)); \
      trace_dump_arg_end(); \
   } while(0)

#define trace_dump_arg_struct(_type, _arg) \
   do { \
      trace_dump_arg_begin(#_arg); \
      trace_dump_##_type(&_arg); \
      trace_dump_arg_end(); \
   } while(0)

#define trace_dump_ret(_type, _arg) \
   do { \
      trace_dump_ret_begin(); \
      trace_dump_##_type(_arg); \
      trace_dump_ret_end(); \
   } while(0)

#define trace_dump_array_impl(_type, _obj, _size, _prefix) \
   do { \
      if (_obj) { \
         size_t idx; \
         trace_dump_array_begin(); \
         for(idx = 0; idx < (_size); ++idx) { \
            trace_dump_elem_begin(); \
            trace_dump_##_type(_prefix(_obj)[idx]); \
            trace_dump_elem_end(); \
         } \
         trace_dump_array_end(); \
      } else { \
         trace_dump_null(); \
      } \
   } while(0)

#define trace_dump_array(_type, _obj, _size) \
   trace_dump_array_impl(_type, _obj, _size, );

#define trace_dump_array_val(_type, _obj, _size) \
   trace_dump_array_impl(_type, _obj, _size, *);

#define trace_dump_struct_array(_type, _obj, _size) \
   do { \
      if (_obj) { \
         size_t idx; \
         trace_dump_array_begin(); \
         for(idx = 0; idx < (_size); ++idx) { \
            trace_dump_elem_begin(); \
            trace_dump_##_type(&(_obj)[idx]); \
            trace_dump_elem_end(); \
         } \
         trace_dump_array_end(); \
      } else { \
         trace_dump_null(); \
      } \
   } while(0)

#define trace_dump_member(_type, _obj, _member) \
   do { \
      trace_dump_member_begin(#_member); \
      trace_dump_##_type((_obj)->_member); \
      trace_dump_member_end(); \
   } while(0)


#define trace_dump_member_enum(_type, _obj, _member) \
   do { \
      trace_dump_member_begin(#_member); \
      trace_dump_enum(tr_util_##_type##_name((_obj)->_member)); \
      trace_dump_member_end(); \
   } while(0)

#define trace_dump_member_struct(_type, _obj, _member) \
   do { \
      trace_dump_member_begin(#_member); \
      trace_dump_##_type(&((_obj)->_member)); \
      trace_dump_member_end(); \
   } while(0)

#define trace_dump_arg_array(_type, _arg, _size) \
   do { \
      trace_dump_arg_begin(#_arg); \
      trace_dump_array(_type, _arg, _size); \
      trace_dump_arg_end(); \
   } while(0)

#define trace_dump_arg_array_val(_type, _arg, _size) \
   do { \
      trace_dump_arg_begin(#_arg); \
      trace_dump_array_val(_type, _arg, _size); \
      trace_dump_arg_end(); \
   } while(0)

#define trace_dump_ret_array(_type, _arg, _size) \
   do { \
      trace_dump_ret_begin(); \
      trace_dump_array(_type, _arg, _size); \
      trace_dump_ret_end(); \
   } while(0)

#define trace_dump_ret_array_val(_type, _arg, _size) \
   do { \
      trace_dump_ret_begin(); \
      trace_dump_array_val(_type, _arg, _size); \
      trace_dump_ret_end(); \
   } while(0)

#define trace_dump_member_array(_type, _obj, _member) \
   do { \
      trace_dump_member_begin(#_member); \
      trace_dump_array(_type, (_obj)->_member, sizeof((_obj)->_member)/sizeof((_obj)->_member[0])); \
      trace_dump_member_end(); \
   } while(0)


#endif /* TR_DUMP_H */
