/* Copyright 2022 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: AMD
 *
 */

#pragma once

#include "vpe_types.h"
#include "vpe_version.h"

#ifdef __cplusplus
extern "C" {
#endif

/* @brief Create the VPE lib instance.
 *
 * Caler provides the current asic info,
 * logging and system memory APIs.
 * It initializes all the necessary resources for the asic
 * and returns the general capabilities of the engines.
 *
 * For capabilities based on conditions,
 * shall be done by vpe->cap_funcs.*
 *
 *
 * @param[in] params  provide the asic version, APIs for logging and memory
 * @return            vpe instance if supported. NULL otherwise
 */
struct vpe *vpe_create(const struct vpe_init_data *params);

/* @brief Destroy the VPE lib instance and resources
 *
 * @param[in] vpe   the vpe instance created by vpe_create
 */
void vpe_destroy(struct vpe **vpe);

/**
 * @brief Check if the VPE operation is supported.
 *
 * Caller must call this to check if the VPE supports
 * the requested operation before calling vpe_build_commands().
 * If operation is supported, it returns the memory requirement.
 *
 * The caller has to prepare those required memories
 *  and pass them to the vpe_build_commands().
 *
 * @param[in]  vpe      vpe instance returned by vpe_initialize()
 * @param[in]  param    build params
 * @param[out] req      memory required for the command buffer and
                        embedded data if return VPE_OK.
                        caller has to alloc them and provide it to build_vpbilts API.
 * @return VPE_OK if supported
 */
enum vpe_status vpe_check_support(
    struct vpe *vpe, const struct vpe_build_param *param, struct vpe_bufs_req *req);

/************************************
 * Command building functions
 ************************************/
/**
 * Build the command descriptors for No-Op operation
 * @param[in]      vpe             vpe instance created by vpe_create()
 * @param[in]      num_dwords      number of noops
 * @param[in,out]  ppcmd_space     in: dword aligned command buffer start address
 *                                 out: dword aligned next good write address
 * @return status
 */
enum vpe_status vpe_build_noops(struct vpe *vpe, uint32_t num_dwords, uint32_t **ppcmd_space);

/**
 * build the command descriptors for the given param.
 * caller must call vpe_check_support() before this function,
 * unexpected result otherwise.
 *
 * @param[in]  vpe      vpe instance created by vpe_create()
 * @param[in]  param    vpe build params
 * @param[in,out]  bufs  [in]  memory allocated for the command buffer, embedded buffer and 3dlut.
 *                             If size is 0, it reports the required size for this checked
 * operation. [out] the next write address and the filled sizes.
 * @return status
 */
enum vpe_status vpe_build_commands(
    struct vpe *vpe, const struct vpe_build_param *param, struct vpe_build_bufs *bufs);

#ifdef __cplusplus
}
#endif
