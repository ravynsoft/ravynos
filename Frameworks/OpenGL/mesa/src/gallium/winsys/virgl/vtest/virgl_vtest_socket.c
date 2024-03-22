/*
 * Copyright 2014, 2015 Red Hat.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <unistd.h>

#include <util/format/u_format.h>
#include <util/u_process.h>

#include "virgl_vtest_winsys.h"
#include "virgl_vtest_public.h"

/* block read/write routines */
static int virgl_block_write(int fd, void *buf, int size)
{
   void *ptr = buf;
   int left;
   int ret;
   left = size;
   do {
      ret = write(fd, ptr, left);
      if (ret < 0)
         return -errno;
      left -= ret;
      ptr += ret;
   } while (left);
   return size;
}

static int virgl_block_read(int fd, void *buf, int size)
{
   void *ptr = buf;
   int left;
   int ret;
   left = size;
   do {
      ret = read(fd, ptr, left);
      if (ret <= 0) {
         fprintf(stderr,
                 "lost connection to rendering server on %d read %d %d\n",
                 size, ret, errno);
         abort();
         return ret < 0 ? -errno : 0;
      }
      left -= ret;
      ptr += ret;
   } while (left);
   return size;
}

static int virgl_vtest_receive_fd(int socket_fd)
{
    struct cmsghdr *cmsgh;
    struct msghdr msgh = { 0 };
    char buf[CMSG_SPACE(sizeof(int))], c;
    struct iovec iovec;

    iovec.iov_base = &c;
    iovec.iov_len = sizeof(char);

    msgh.msg_name = NULL;
    msgh.msg_namelen = 0;
    msgh.msg_iov = &iovec;
    msgh.msg_iovlen = 1;
    msgh.msg_control = buf;
    msgh.msg_controllen = sizeof(buf);
    msgh.msg_flags = 0;

    int size = recvmsg(socket_fd, &msgh, 0);
    if (size < 0) {
      fprintf(stderr, "Failed with %s\n", strerror(errno));
      return -1;
    }

    cmsgh = CMSG_FIRSTHDR(&msgh);
    if (!cmsgh) {
      fprintf(stderr, "No headers available\n");
      return -1;
    }

    if (cmsgh->cmsg_level != SOL_SOCKET) {
      fprintf(stderr, "invalid cmsg_level %d\n", cmsgh->cmsg_level);
      return -1;
    }

    if (cmsgh->cmsg_type != SCM_RIGHTS) {
      fprintf(stderr, "invalid cmsg_type %d\n", cmsgh->cmsg_type);
      return -1;
    }

    return *((int *) CMSG_DATA(cmsgh));
}

static int virgl_vtest_send_init(struct virgl_vtest_winsys *vws)
{
   uint32_t buf[VTEST_HDR_SIZE];
   const char *nstr = "virtest";
   char cmdline[64] = { 0 };
   const char *proc_name = util_get_process_name();

   if (proc_name)
      strncpy(cmdline, proc_name, 63);
   else
      strcpy(cmdline, nstr);
#if defined(HAVE_PROGRAM_INVOCATION_NAME)
   if (!strcmp(cmdline, "shader_runner")) {
      const char *name;
      /* hack to get better testname */
      name = program_invocation_short_name;
      name += strlen(name) + 1;
      strncpy(cmdline, name, 63);
   }
#endif
   buf[VTEST_CMD_LEN] = strlen(cmdline) + 1;
   buf[VTEST_CMD_ID] = VCMD_CREATE_RENDERER;

   virgl_block_write(vws->sock_fd, &buf, sizeof(buf));
   virgl_block_write(vws->sock_fd, (void *)cmdline, strlen(cmdline) + 1);
   return 0;
}

static int virgl_vtest_negotiate_version(struct virgl_vtest_winsys *vws)
{
   uint32_t vtest_hdr[VTEST_HDR_SIZE];
   uint32_t version_buf[VCMD_PROTOCOL_VERSION_SIZE];
   uint32_t busy_wait_buf[VCMD_BUSY_WAIT_SIZE];
   uint32_t busy_wait_result[1];
   ASSERTED int ret;

   vtest_hdr[VTEST_CMD_LEN] = VCMD_PING_PROTOCOL_VERSION_SIZE;
   vtest_hdr[VTEST_CMD_ID] = VCMD_PING_PROTOCOL_VERSION;
   virgl_block_write(vws->sock_fd, &vtest_hdr, sizeof(vtest_hdr));

   vtest_hdr[VTEST_CMD_LEN] = VCMD_BUSY_WAIT_SIZE;
   vtest_hdr[VTEST_CMD_ID] = VCMD_RESOURCE_BUSY_WAIT;
   busy_wait_buf[VCMD_BUSY_WAIT_HANDLE] = 0;
   busy_wait_buf[VCMD_BUSY_WAIT_FLAGS] = 0;
   virgl_block_write(vws->sock_fd, &vtest_hdr, sizeof(vtest_hdr));
   virgl_block_write(vws->sock_fd, &busy_wait_buf, sizeof(busy_wait_buf));

   ret = virgl_block_read(vws->sock_fd, vtest_hdr, sizeof(vtest_hdr));
   assert(ret);

   if (vtest_hdr[VTEST_CMD_ID] == VCMD_PING_PROTOCOL_VERSION) {
     /* Read dummy busy_wait response */
     ret = virgl_block_read(vws->sock_fd, vtest_hdr, sizeof(vtest_hdr));
     assert(ret);
     ret = virgl_block_read(vws->sock_fd, busy_wait_result, sizeof(busy_wait_result));
     assert(ret);

     vtest_hdr[VTEST_CMD_LEN] = VCMD_PROTOCOL_VERSION_SIZE;
     vtest_hdr[VTEST_CMD_ID] = VCMD_PROTOCOL_VERSION;
     version_buf[VCMD_PROTOCOL_VERSION_VERSION] = VTEST_PROTOCOL_VERSION;
     virgl_block_write(vws->sock_fd, &vtest_hdr, sizeof(vtest_hdr));
     virgl_block_write(vws->sock_fd, &version_buf, sizeof(version_buf));

     ret = virgl_block_read(vws->sock_fd, vtest_hdr, sizeof(vtest_hdr));
     assert(ret);
     ret = virgl_block_read(vws->sock_fd, version_buf, sizeof(version_buf));
     assert(ret);
     return version_buf[VCMD_PROTOCOL_VERSION_VERSION];
   }

   /* Read dummy busy_wait response */
   assert(vtest_hdr[VTEST_CMD_ID] == VCMD_RESOURCE_BUSY_WAIT);
   ret = virgl_block_read(vws->sock_fd, busy_wait_result, sizeof(busy_wait_result));
   assert(ret);

   /* Old server, return version 0 */
   return 0;
}

int virgl_vtest_connect(struct virgl_vtest_winsys *vws)
{
   struct sockaddr_un un;
   int sock, ret;
   const char* socket_name = os_get_option("VTEST_SOCKET_NAME");

   sock = socket(PF_UNIX, SOCK_STREAM, 0);
   if (sock < 0)
      return -1;

   memset(&un, 0, sizeof(un));
   un.sun_family = AF_UNIX;
   snprintf(un.sun_path, sizeof(un.sun_path), "%s", socket_name ?
      socket_name : VTEST_DEFAULT_SOCKET_NAME);

   do {
      ret = 0;
      if (connect(sock, (struct sockaddr *)&un, sizeof(un)) < 0) {
         ret = -errno;
      }
   } while (ret == -EINTR);

   vws->sock_fd = sock;
   virgl_vtest_send_init(vws);
   vws->protocol_version = virgl_vtest_negotiate_version(vws);

   /* Version 1 is deprecated. */
   if (vws->protocol_version == 1)
      vws->protocol_version = 0;

   return 0;
}

int virgl_vtest_send_get_caps(struct virgl_vtest_winsys *vws,
                              struct virgl_drm_caps *caps)
{
   uint32_t get_caps_buf[VTEST_HDR_SIZE * 2];
   uint32_t resp_buf[VTEST_HDR_SIZE];
   uint32_t caps_size = sizeof(struct virgl_caps_v2);
   int ret;
   get_caps_buf[VTEST_CMD_LEN] = 0;
   get_caps_buf[VTEST_CMD_ID] = VCMD_GET_CAPS2;
   get_caps_buf[VTEST_CMD_LEN + 2] = 0;
   get_caps_buf[VTEST_CMD_ID + 2] = VCMD_GET_CAPS;

   virgl_block_write(vws->sock_fd, &get_caps_buf, sizeof(get_caps_buf));

   ret = virgl_block_read(vws->sock_fd, resp_buf, sizeof(resp_buf));
   if (ret <= 0)
      return 0;

   if (resp_buf[1] == 2) {
       struct virgl_caps_v1 dummy;
       uint32_t resp_size = resp_buf[0] - 1;
       uint32_t dummy_size = 0;
       if (resp_size > caps_size) {
	   dummy_size = resp_size - caps_size;
	   resp_size = caps_size;
       }

       ret = virgl_block_read(vws->sock_fd, &caps->caps, resp_size);

       while (dummy_size) {
           ret = virgl_block_read(vws->sock_fd, &dummy,
                    dummy_size < sizeof(dummy) ? dummy_size : sizeof(dummy));
           if (ret <= 0)
               break;
           dummy_size -= ret;
       }

       /* now read back the pointless caps v1 we requested */
       ret = virgl_block_read(vws->sock_fd, resp_buf, sizeof(resp_buf));
       if (ret <= 0)
	   return 0;
       ret = virgl_block_read(vws->sock_fd, &dummy, sizeof(struct virgl_caps_v1));
   } else
       ret = virgl_block_read(vws->sock_fd, &caps->caps, sizeof(struct virgl_caps_v1));

   return 0;
}

static int virgl_vtest_send_resource_create2(struct virgl_vtest_winsys *vws,
                                             uint32_t handle,
                                             enum pipe_texture_target target,
                                             uint32_t format,
                                             uint32_t bind,
                                             uint32_t width,
                                             uint32_t height,
                                             uint32_t depth,
                                             uint32_t array_size,
                                             uint32_t last_level,
                                             uint32_t nr_samples,
                                             uint32_t size,
                                             int *out_fd)
{
   uint32_t res_create_buf[VCMD_RES_CREATE2_SIZE], vtest_hdr[VTEST_HDR_SIZE];

   vtest_hdr[VTEST_CMD_LEN] = VCMD_RES_CREATE2_SIZE;
   vtest_hdr[VTEST_CMD_ID] = VCMD_RESOURCE_CREATE2;

   res_create_buf[VCMD_RES_CREATE2_RES_HANDLE] = handle;
   res_create_buf[VCMD_RES_CREATE2_TARGET] = target;
   res_create_buf[VCMD_RES_CREATE2_FORMAT] = format;
   res_create_buf[VCMD_RES_CREATE2_BIND] = bind;
   res_create_buf[VCMD_RES_CREATE2_WIDTH] = width;
   res_create_buf[VCMD_RES_CREATE2_HEIGHT] = height;
   res_create_buf[VCMD_RES_CREATE2_DEPTH] = depth;
   res_create_buf[VCMD_RES_CREATE2_ARRAY_SIZE] = array_size;
   res_create_buf[VCMD_RES_CREATE2_LAST_LEVEL] = last_level;
   res_create_buf[VCMD_RES_CREATE2_NR_SAMPLES] = nr_samples;
   res_create_buf[VCMD_RES_CREATE2_DATA_SIZE] = size;

   virgl_block_write(vws->sock_fd, &vtest_hdr, sizeof(vtest_hdr));
   virgl_block_write(vws->sock_fd, &res_create_buf, sizeof(res_create_buf));

   /* Multi-sampled textures have no backing store attached. */
   if (size == 0)
      return 0;

   *out_fd = virgl_vtest_receive_fd(vws->sock_fd);
   if (*out_fd < 0) {
      fprintf(stderr, "failed to get fd\n");
      return -1;
   }

   return 0;
}

int virgl_vtest_send_resource_create(struct virgl_vtest_winsys *vws,
                                     uint32_t handle,
                                     enum pipe_texture_target target,
                                     uint32_t format,
                                     uint32_t bind,
                                     uint32_t width,
                                     uint32_t height,
                                     uint32_t depth,
                                     uint32_t array_size,
                                     uint32_t last_level,
                                     uint32_t nr_samples,
                                     uint32_t size,
                                     int *out_fd)
{
   uint32_t res_create_buf[VCMD_RES_CREATE_SIZE], vtest_hdr[VTEST_HDR_SIZE];

   if (vws->protocol_version >= 2)
      return virgl_vtest_send_resource_create2(vws, handle, target, format,
                                               bind, width, height, depth,
                                               array_size, last_level,
                                               nr_samples, size, out_fd);

   vtest_hdr[VTEST_CMD_LEN] = VCMD_RES_CREATE_SIZE;
   vtest_hdr[VTEST_CMD_ID] = VCMD_RESOURCE_CREATE;

   res_create_buf[VCMD_RES_CREATE_RES_HANDLE] = handle;
   res_create_buf[VCMD_RES_CREATE_TARGET] = target;
   res_create_buf[VCMD_RES_CREATE_FORMAT] = format;
   res_create_buf[VCMD_RES_CREATE_BIND] = bind;
   res_create_buf[VCMD_RES_CREATE_WIDTH] = width;
   res_create_buf[VCMD_RES_CREATE_HEIGHT] = height;
   res_create_buf[VCMD_RES_CREATE_DEPTH] = depth;
   res_create_buf[VCMD_RES_CREATE_ARRAY_SIZE] = array_size;
   res_create_buf[VCMD_RES_CREATE_LAST_LEVEL] = last_level;
   res_create_buf[VCMD_RES_CREATE_NR_SAMPLES] = nr_samples;

   virgl_block_write(vws->sock_fd, &vtest_hdr, sizeof(vtest_hdr));
   virgl_block_write(vws->sock_fd, &res_create_buf, sizeof(res_create_buf));

   return 0;
}

int virgl_vtest_submit_cmd(struct virgl_vtest_winsys *vws,
                           struct virgl_vtest_cmd_buf *cbuf)
{
   uint32_t vtest_hdr[VTEST_HDR_SIZE];

   vtest_hdr[VTEST_CMD_LEN] = cbuf->base.cdw;
   vtest_hdr[VTEST_CMD_ID] = VCMD_SUBMIT_CMD;

   virgl_block_write(vws->sock_fd, &vtest_hdr, sizeof(vtest_hdr));
   virgl_block_write(vws->sock_fd, cbuf->buf, cbuf->base.cdw * 4);
   return 0;
}

int virgl_vtest_send_resource_unref(struct virgl_vtest_winsys *vws,
                                    uint32_t handle)
{
   uint32_t vtest_hdr[VTEST_HDR_SIZE];
   uint32_t cmd[1];
   vtest_hdr[VTEST_CMD_LEN] = 1;
   vtest_hdr[VTEST_CMD_ID] = VCMD_RESOURCE_UNREF;

   cmd[0] = handle;
   virgl_block_write(vws->sock_fd, &vtest_hdr, sizeof(vtest_hdr));
   virgl_block_write(vws->sock_fd, &cmd, sizeof(cmd));
   return 0;
}

static int virgl_vtest_send_transfer_cmd(struct virgl_vtest_winsys *vws,
                                  uint32_t vcmd,
                                  uint32_t handle,
                                  uint32_t level, uint32_t stride,
                                  uint32_t layer_stride,
                                  const struct pipe_box *box,
                                  uint32_t data_size)
{
   uint32_t vtest_hdr[VTEST_HDR_SIZE];
   uint32_t cmd[VCMD_TRANSFER_HDR_SIZE];
   vtest_hdr[VTEST_CMD_LEN] = VCMD_TRANSFER_HDR_SIZE;
   vtest_hdr[VTEST_CMD_ID] = vcmd;

   /* The host expects the size in dwords so calculate the rounded up
    * value here. */
   if (vcmd == VCMD_TRANSFER_PUT)
      vtest_hdr[VTEST_CMD_LEN] += (data_size + 3) / 4;

   cmd[0] = handle;
   cmd[1] = level;
   cmd[2] = stride;
   cmd[3] = layer_stride;
   cmd[4] = box->x;
   cmd[5] = box->y;
   cmd[6] = box->z;
   cmd[7] = box->width;
   cmd[8] = box->height;
   cmd[9] = box->depth;
   cmd[10] = data_size;
   virgl_block_write(vws->sock_fd, &vtest_hdr, sizeof(vtest_hdr));
   virgl_block_write(vws->sock_fd, &cmd, sizeof(cmd));

   return 0;
}

static int virgl_vtest_send_transfer_cmd2(struct virgl_vtest_winsys *vws,
                                  uint32_t vcmd,
                                  uint32_t handle,
                                  uint32_t level,
                                  const struct pipe_box *box,
                                  uint32_t data_size,
                                  uint32_t offset)
{
   uint32_t vtest_hdr[VTEST_HDR_SIZE];
   uint32_t cmd[VCMD_TRANSFER2_HDR_SIZE];
   vtest_hdr[VTEST_CMD_LEN] = VCMD_TRANSFER2_HDR_SIZE;
   vtest_hdr[VTEST_CMD_ID] = vcmd;

   /* The host expects the size in dwords so calculate the rounded up
    * value here. */
   if (vcmd == VCMD_TRANSFER_PUT2)
      vtest_hdr[VTEST_CMD_LEN] += (data_size + 3) / 4;

   cmd[VCMD_TRANSFER2_RES_HANDLE] = handle;
   cmd[VCMD_TRANSFER2_LEVEL] = level;
   cmd[VCMD_TRANSFER2_X] = box->x;
   cmd[VCMD_TRANSFER2_Y] = box->y;
   cmd[VCMD_TRANSFER2_Z] = box->z;
   cmd[VCMD_TRANSFER2_WIDTH] = box->width;
   cmd[VCMD_TRANSFER2_HEIGHT] = box->height;
   cmd[VCMD_TRANSFER2_DEPTH] = box->depth;
   cmd[VCMD_TRANSFER2_DATA_SIZE] = data_size;
   cmd[VCMD_TRANSFER2_OFFSET] = offset;
   virgl_block_write(vws->sock_fd, &vtest_hdr, sizeof(vtest_hdr));
   virgl_block_write(vws->sock_fd, &cmd, sizeof(cmd));

   return 0;
}

int virgl_vtest_send_transfer_get(struct virgl_vtest_winsys *vws,
                                  uint32_t handle,
                                  uint32_t level, uint32_t stride,
                                  uint32_t layer_stride,
                                  const struct pipe_box *box,
                                  uint32_t data_size,
                                  uint32_t offset)
{
   if (vws->protocol_version < 2)
      return virgl_vtest_send_transfer_cmd(vws, VCMD_TRANSFER_GET, handle,
                                           level, stride, layer_stride, box,
                                           data_size);

   return virgl_vtest_send_transfer_cmd2(vws, VCMD_TRANSFER_GET2, handle,
                                        level, box, data_size, offset);
}

int virgl_vtest_send_transfer_put(struct virgl_vtest_winsys *vws,
                                  uint32_t handle,
                                  uint32_t level, uint32_t stride,
                                  uint32_t layer_stride,
                                  const struct pipe_box *box,
                                  uint32_t data_size,
                                  uint32_t offset)
{
   if (vws->protocol_version < 2)
      return virgl_vtest_send_transfer_cmd(vws, VCMD_TRANSFER_PUT, handle,
                                           level, stride, layer_stride, box,
                                           data_size);

   return virgl_vtest_send_transfer_cmd2(vws, VCMD_TRANSFER_PUT2, handle,
                                        level, box, data_size, offset);
}

int virgl_vtest_send_transfer_put_data(struct virgl_vtest_winsys *vws,
                                       void *data,
                                       uint32_t data_size)
{
   return virgl_block_write(vws->sock_fd, data, data_size);
}

int virgl_vtest_recv_transfer_get_data(struct virgl_vtest_winsys *vws,
                                       void *data,
                                       uint32_t data_size,
                                       uint32_t stride,
                                       const struct pipe_box *box,
                                       uint32_t format)
{
   void *line;
   void *ptr = data;
   int hblocks = util_format_get_nblocksy(format, box->height);

   line = malloc(stride);
   while (hblocks) {
      virgl_block_read(vws->sock_fd, line, stride);
      memcpy(ptr, line, util_format_get_stride(format, box->width));
      ptr += stride;
      hblocks--;
   }
   free(line);
   return 0;
}

int virgl_vtest_busy_wait(struct virgl_vtest_winsys *vws, int handle,
                          int flags)
{
   uint32_t vtest_hdr[VTEST_HDR_SIZE];
   uint32_t cmd[VCMD_BUSY_WAIT_SIZE];
   uint32_t result[1];
   ASSERTED int ret;
   vtest_hdr[VTEST_CMD_LEN] = VCMD_BUSY_WAIT_SIZE;
   vtest_hdr[VTEST_CMD_ID] = VCMD_RESOURCE_BUSY_WAIT;
   cmd[VCMD_BUSY_WAIT_HANDLE] = handle;
   cmd[VCMD_BUSY_WAIT_FLAGS] = flags;

   virgl_block_write(vws->sock_fd, &vtest_hdr, sizeof(vtest_hdr));
   virgl_block_write(vws->sock_fd, &cmd, sizeof(cmd));

   ret = virgl_block_read(vws->sock_fd, vtest_hdr, sizeof(vtest_hdr));
   assert(ret);
   ret = virgl_block_read(vws->sock_fd, result, sizeof(result));
   assert(ret);
   return result[0];
}
