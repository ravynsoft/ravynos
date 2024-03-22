/**************************************************************************
 *
 * Copyright 2009 VMware, Inc.
 * Copyright 2016 Axel Davy <axel.davy@ens.fr>
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

#ifndef _NINE_BUFFER_UPLOAD_H_
#define _NINE_BUFFER_UPLOAD_H_

#include "pipe/p_defines.h"

struct nine_buffer_upload;
struct nine_subbuffer;

struct nine_buffer_upload *
nine_upload_create(struct pipe_context *pipe, unsigned buffers_size,
                   unsigned num_buffers);

void
nine_upload_destroy(struct nine_buffer_upload *upload);

struct nine_subbuffer *
nine_upload_create_buffer(struct nine_buffer_upload *upload,
                          unsigned buffer_size);

void
nine_upload_release_buffer(struct nine_buffer_upload *upload,
                           struct nine_subbuffer *buf);

uint8_t *
nine_upload_buffer_get_map(struct nine_subbuffer *buf);

struct pipe_resource *
nine_upload_buffer_resource_and_offset(struct nine_subbuffer *buf,
                                       unsigned *offset);

#endif /* _NINE_BUFFER_UPLOAD_H_ */
