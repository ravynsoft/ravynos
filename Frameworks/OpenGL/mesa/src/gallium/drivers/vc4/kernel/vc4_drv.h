/*
 * Copyright © 2014 Broadcom
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef VC4_DRV_H
#define VC4_DRV_H

#include "vc4_simulator_validate.h"

struct vc4_exec_info {
	/* Sequence number for this bin/render job. */
	uint64_t seqno;

	/* Kernel-space copy of the ioctl arguments */
	struct drm_vc4_submit_cl *args;

	/* This is the array of BOs that were looked up at the start of exec.
	 * Command validation will use indices into this array.
	 */
	struct drm_gem_cma_object **bo;
	uint32_t bo_count;

	/* List of other BOs used in the job that need to be released
	 * once the job is complete.
	 */
	struct list_head unref_list;

	/* Current unvalidated indices into @bo loaded by the non-hardware
	 * VC4_PACKET_GEM_HANDLES.
	 */
	uint32_t bo_index[2];

	/* This is the BO where we store the validated command lists, shader
	 * records, and uniforms.
	 */
	struct drm_gem_cma_object *exec_bo;

	/**
	 * This tracks the per-shader-record state (packet 64) that
	 * determines the length of the shader record and the offset
	 * it's expected to be found at.  It gets read in from the
	 * command lists.
	 */
	struct vc4_shader_state {
		uint32_t addr;
		/* Maximum vertex index referenced by any primitive using this
		 * shader state.
		 */
		uint32_t max_index;
	} *shader_state;

	/** How many shader states the user declared they were using. */
	uint32_t shader_state_size;
	/** How many shader state records the validator has seen. */
	uint32_t shader_state_count;

	bool found_tile_binning_mode_config_packet;
	bool found_start_tile_binning_packet;
	bool found_increment_semaphore_packet;
	bool found_flush;
	uint8_t bin_tiles_x, bin_tiles_y;
	struct drm_gem_cma_object *tile_bo;
	uint32_t tile_alloc_offset;

	uint32_t tile_width, tile_height;

	/**
	 * Computed addresses pointing into exec_bo where we start the
	 * bin thread (ct0) and render thread (ct1).
	 */
	uint32_t ct0ca, ct0ea;
	uint32_t ct1ca, ct1ea;

	/* Pointer to the unvalidated bin CL (if present). */
	void *bin_u;

	/* Pointers to the shader recs.  These paddr gets incremented as CL
	 * packets are relocated in validate_gl_shader_state, and the vaddrs
	 * (u and v) get incremented and size decremented as the shader recs
	 * themselves are validated.
	 */
	void *shader_rec_u;
	void *shader_rec_v;
	uint32_t shader_rec_p;
	uint32_t shader_rec_size;

	/* Pointers to the uniform data.  These pointers are incremented, and
	 * size decremented, as each batch of uniforms is uploaded.
	 */
	void *uniforms_u;
	void *uniforms_v;
	uint32_t uniforms_p;
	uint32_t uniforms_size;
};

/**
 * struct vc4_texture_sample_info - saves the offsets into the UBO for texture
 * setup parameters.
 *
 * This will be used at draw time to relocate the reference to the texture
 * contents in p0, and validate that the offset combined with
 * width/height/stride/etc. from p1 and p2/p3 doesn't sample outside the BO.
 * Note that the hardware treats unprovided config parameters as 0, so not all
 * of them need to be set up for every texture sample, and we'll store ~0 as
 * the offset to mark the unused ones.
 *
 * See the VC4 3D architecture guide page 41 ("Texture and Memory Lookup Unit
 * Setup") for definitions of the texture parameters.
 */
struct vc4_texture_sample_info {
	bool is_direct;
	uint32_t p_offset[4];
};

/**
 * struct vc4_validated_shader_info - information about validated shaders that
 * needs to be used from command list validation.
 *
 * For a given shader, each time a shader state record references it, we need
 * to verify that the shader doesn't read more uniforms than the shader state
 * record's uniform BO pointer can provide, and we need to apply relocations
 * and validate the shader state record's uniforms that define the texture
 * samples.
 */
struct vc4_validated_shader_info
{
	uint32_t uniforms_size;
	uint32_t uniforms_src_size;
	uint32_t num_texture_samples;
	struct vc4_texture_sample_info *texture_samples;

	uint32_t num_uniform_addr_offsets;
	uint32_t *uniform_addr_offsets;

	bool is_threaded;
};

/* vc4_validate.c */
int
vc4_validate_bin_cl(struct drm_device *dev,
		    void *validated,
		    void *unvalidated,
		    struct vc4_exec_info *exec);

int
vc4_validate_shader_recs(struct drm_device *dev, struct vc4_exec_info *exec);

struct vc4_validated_shader_info *
vc4_validate_shader(struct drm_gem_cma_object *shader_obj);

struct drm_gem_cma_object *vc4_use_bo(struct vc4_exec_info *exec,
				      uint32_t hindex);

int vc4_get_rcl(struct drm_device *dev, struct vc4_exec_info *exec);

bool vc4_check_tex_size(struct vc4_exec_info *exec,
			struct drm_gem_cma_object *fbo,
			uint32_t offset, uint8_t tiling_format,
			uint32_t width, uint32_t height, uint8_t cpp);

#endif /* VC4_DRV_H */
