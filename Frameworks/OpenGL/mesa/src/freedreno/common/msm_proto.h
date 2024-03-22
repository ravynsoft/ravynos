/*
 * Copyright 2022 Google LLC
 * SPDX-License-Identifier: MIT
 */

#ifndef MSM_PROTO_H_
#define MSM_PROTO_H_

/**
 * General protocol notes:
 * 1) Request (req) messages are generally sent over DRM_VIRTGPU_EXECBUFFER
 *    but can also be sent via DRM_VIRTGPU_RESOURCE_CREATE_BLOB (in which
 *    case they are processed by the host before ctx->get_blob())
 * 2) Response (rsp) messages are returned via shmem->rsp_mem, at an offset
 *    specified by the guest in the req message.  Not all req messages have
 *    a rsp.
 * 3) Host and guest could have different pointer sizes, ie. 32b guest and
 *    64b host, or visa versa, so similar to kernel uabi, req and rsp msgs
 *    should be explicitly padded to avoid 32b vs 64b struct padding issues
 */

/**
 * Defines the layout of shmem buffer used for host->guest communication.
 */
struct msm_shmem {
   struct vdrm_shmem base;

   /**
    * Counter that is incremented on asynchronous errors, like SUBMIT
    * or GEM_NEW failures.  The guest should treat errors as context-
    * lost.
    */
   uint32_t async_error;

   /**
    * Counter that is incremented on global fault (see MSM_PARAM_FAULTS)
    */
   uint32_t global_faults;
};
DEFINE_CAST(vdrm_shmem, msm_shmem)

/*
 * Possible cmd types for "command stream", ie. payload of EXECBUF ioctl:
 */
enum msm_ccmd {
   MSM_CCMD_NOP = 1,         /* No payload, can be used to sync with host */
   MSM_CCMD_IOCTL_SIMPLE,
   MSM_CCMD_GEM_NEW,
   MSM_CCMD_GEM_SET_IOVA,
   MSM_CCMD_GEM_CPU_PREP,
   MSM_CCMD_GEM_SET_NAME,
   MSM_CCMD_GEM_SUBMIT,
   MSM_CCMD_GEM_UPLOAD,
   MSM_CCMD_SUBMITQUEUE_QUERY,
   MSM_CCMD_WAIT_FENCE,
   MSM_CCMD_SET_DEBUGINFO,
   MSM_CCMD_LAST,
};

#ifdef __cplusplus
#define MSM_CCMD(_cmd, _len) {                      \
       .cmd = MSM_CCMD_##_cmd,                      \
       .len = (_len),                               \
   }
#else
#define MSM_CCMD(_cmd, _len) (struct vdrm_ccmd_req){ \
       .cmd = MSM_CCMD_##_cmd,                      \
       .len = (_len),                               \
   }
#endif

/*
 * MSM_CCMD_NOP
 */
struct msm_ccmd_nop_req {
   struct vdrm_ccmd_req hdr;
};

/*
 * MSM_CCMD_IOCTL_SIMPLE
 *
 * Forward simple/flat IOC_RW or IOC_W ioctls.  Limited ioctls are supported.
 */
struct msm_ccmd_ioctl_simple_req {
   struct vdrm_ccmd_req hdr;

   uint32_t cmd;
   uint8_t payload[];
};
DEFINE_CAST(vdrm_ccmd_req, msm_ccmd_ioctl_simple_req)

struct msm_ccmd_ioctl_simple_rsp {
   struct vdrm_ccmd_rsp hdr;

   /* ioctl return value, interrupted syscalls are handled on the host without
    * returning to the guest.
    */
   int32_t ret;

   /* The output payload for IOC_RW ioctls, the payload is the same size as
    * msm_context_cmd_ioctl_simple_req.
    *
    * For IOC_W ioctls (userspace writes, kernel reads) this is zero length.
    */
   uint8_t payload[];
};

/*
 * MSM_CCMD_GEM_NEW
 *
 * GEM buffer allocation, maps to DRM_MSM_GEM_NEW plus DRM_MSM_GEM_INFO to
 * set the BO's iova (to avoid extra guest -> host trip)
 *
 * No response.
 */
struct msm_ccmd_gem_new_req {
   struct vdrm_ccmd_req hdr;

   uint64_t iova;
   uint64_t size;
   uint32_t flags;
   uint32_t blob_id;
};
DEFINE_CAST(vdrm_ccmd_req, msm_ccmd_gem_new_req)

/*
 * MSM_CCMD_GEM_SET_IOVA
 *
 * Set the buffer iova (for imported BOs).  Also used to release the iova
 * (by setting it to zero) when a BO is freed.
 */
struct msm_ccmd_gem_set_iova_req {
   struct vdrm_ccmd_req hdr;

   uint64_t iova;
   uint32_t res_id;
};
DEFINE_CAST(vdrm_ccmd_req, msm_ccmd_gem_set_iova_req)

/*
 * MSM_CCMD_GEM_CPU_PREP
 *
 * Maps to DRM_MSM_GEM_CPU_PREP
 *
 * Note: Since we don't want to block the single threaded host, this returns
 * immediately with -EBUSY if the fence is not yet signaled.  The guest
 * should poll if needed.
 */
struct msm_ccmd_gem_cpu_prep_req {
   struct vdrm_ccmd_req hdr;

   uint32_t res_id;
   uint32_t op;
};
DEFINE_CAST(vdrm_ccmd_req, msm_ccmd_gem_cpu_prep_req)

struct msm_ccmd_gem_cpu_prep_rsp {
   struct vdrm_ccmd_rsp hdr;

   int32_t ret;
};

/*
 * MSM_CCMD_GEM_SET_NAME
 *
 * Maps to DRM_MSM_GEM_INFO:MSM_INFO_SET_NAME
 *
 * No response.
 */
struct msm_ccmd_gem_set_name_req {
   struct vdrm_ccmd_req hdr;

   uint32_t res_id;
   /* Note: packet size aligned to 4 bytes, so the string name may
    * be shorter than the packet header indicates.
    */
   uint32_t len;
   uint8_t  payload[];
};
DEFINE_CAST(vdrm_ccmd_req, msm_ccmd_gem_set_name_req)

/*
 * MSM_CCMD_GEM_SUBMIT
 *
 * Maps to DRM_MSM_GEM_SUBMIT
 *
 * The actual for-reals cmdstream submission.  Note this intentionally
 * does not support relocs, since we already require a non-ancient
 * kernel.
 *
 * Note, no in/out fence-fd, that synchronization is handled on guest
 * kernel side (ugg).. need to come up with a better story for fencing.
 * We probably need to sort something out for that to handle syncobjs.
 *
 * No response.
 */
struct msm_ccmd_gem_submit_req {
   struct vdrm_ccmd_req hdr;

   uint32_t flags;
   uint32_t queue_id;
   uint32_t nr_bos;
   uint32_t nr_cmds;

   /**
    * The fence "seqno" assigned by the guest userspace.  The host SUBMIT
    * ioctl uses the MSM_SUBMIT_FENCE_SN_IN flag to let the guest assign
    * the sequence #, to avoid the guest needing to wait for a response
    * from the host.
    */
   uint32_t fence;

   /**
    * Payload is first an array of 'struct drm_msm_gem_submit_bo' of
    * length determined by nr_bos (note that handles are guest resource
    * ids which are translated to host GEM handles by the host VMM),
    * followed by an array of 'struct drm_msm_gem_submit_cmd' of length
    * determined by nr_cmds
    */
   int8_t   payload[];
};
DEFINE_CAST(vdrm_ccmd_req, msm_ccmd_gem_submit_req)

/*
 * MSM_CCMD_GEM_UPLOAD
 *
 * Upload data to a GEM buffer
 *
 * No response.
 */
struct msm_ccmd_gem_upload_req {
   struct vdrm_ccmd_req hdr;

   uint32_t res_id;
   uint32_t pad;
   uint32_t off;

   /* Note: packet size aligned to 4 bytes, so the payload may
    * be shorter than the packet header indicates.
    */
   uint32_t len;
   uint8_t  payload[];
};
DEFINE_CAST(vdrm_ccmd_req, msm_ccmd_gem_upload_req)

/*
 * MSM_CCMD_SUBMITQUEUE_QUERY
 *
 * Maps to DRM_MSM_SUBMITQUEUE_QUERY
 */
struct msm_ccmd_submitqueue_query_req {
   struct vdrm_ccmd_req hdr;

   uint32_t queue_id;
   uint32_t param;
   uint32_t len;   /* size of payload in rsp */
};
DEFINE_CAST(vdrm_ccmd_req, msm_ccmd_submitqueue_query_req)

struct msm_ccmd_submitqueue_query_rsp {
   struct vdrm_ccmd_rsp hdr;

   int32_t  ret;
   uint32_t out_len;
   uint8_t  payload[];
};

/*
 * MSM_CCMD_WAIT_FENCE
 *
 * Maps to DRM_MSM_WAIT_FENCE
 *
 * Note: Since we don't want to block the single threaded host, this returns
 * immediately with -ETIMEDOUT if the fence is not yet signaled.  The guest
 * should poll if needed.
 */
struct msm_ccmd_wait_fence_req {
   struct vdrm_ccmd_req hdr;

   uint32_t queue_id;
   uint32_t fence;
};
DEFINE_CAST(vdrm_ccmd_req, msm_ccmd_wait_fence_req)

struct msm_ccmd_wait_fence_rsp {
   struct vdrm_ccmd_rsp hdr;

   int32_t ret;
};

/*
 * MSM_CCMD_SET_DEBUGINFO
 *
 * Set per-guest-process debug info (comm and cmdline).  For GPU faults/
 * crashes, it isn't too useful to see the crosvm (for ex.) comm/cmdline,
 * since the host process is only a proxy.  This allows the guest to
 * pass through the guest process comm and commandline for debugging
 * purposes.
 *
 * No response.
 */
struct msm_ccmd_set_debuginfo_req {
   struct vdrm_ccmd_req hdr;

   uint32_t comm_len;
   uint32_t cmdline_len;

   /**
    * Payload is first the comm string followed by cmdline string, padded
    * out to a multiple of 4.
    */
   int8_t   payload[];
};
DEFINE_CAST(vdrm_ccmd_req, msm_ccmd_set_debuginfo_req)

#endif /* MSM_PROTO_H_ */
