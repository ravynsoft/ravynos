/*
 * Copyright 2023 Advanced Micro Devices, Inc.
 * SPDX-License-Identifier: MIT
 */

#ifndef AC_HW_STAGE_H
#define AC_HW_STAGE_H

/**
 * Shader stages as understood by AMD HW.
 *
 * Every HW generation has a dedicated stage for compute and PS,
 * but it varies greatly over other geometry processing stages.
 *
 * Valid graphics shader configurations:
 * (-> = merged with the next stage)
 *
 * -------------------------|-----|-----|----|----|----
 * API shaders:          VS | TCS | TES | GS |copy| FS
 * Are compiled as:         |     |     |    |    |
 * GFX6-8 ------------------|-----|-----|----|----|----
 *          - VS & PS:   VS |     |     |    |    | PS
 *          - with GS:   ES |     |     | GS | VS | PS
 *          - with tess: LS | HS  | VS  |    |    | PS
 *          - with both: LS | HS  | ES  | GS | VS | PS
 * GFX9-10.3/legacy --------|-----|-----|----|----|----
 *          - VS & PS:   VS |     |     |    |    | PS
 *          - with GS:   -> |     |     | GS | VS | PS
 *          - with tess: -> | HS  | VS  |    |    | PS
 *          - with both: -> | HS  | ->  | GS | VS | PS
 * GFX10+/NGG --------------|-----|-----|----|----|----
 *          - VS & PS:   GS |     |     |    |    | PS
 *          - with GS:   -> |     |     | GS |    | PS
 *          - with tess: -> | HS  | GS  |    |    | PS
 *          - with both: -> | HS  | ->  | GS |    | PS
 * -------------------------|-----|-----|----|----|----
 *
 * Valid mesh shading graphics pipeline configurations:
 *
 * -------------------------------|---------------|----
 * API shaders:                TS |            MS | FS
 * Are compiled as:               |               |
 * GFX10.3+/NGG ------------------|---------------|----
 *        - mesh only:            |            GS | PS
 *        - task & mesh:       CS |            GS | PS
 * -------------------------------|---------------|----
 *
 */
enum ac_hw_stage
{
   /* GFX6-8 only, merged into HS on GFX9+:
    *    - vertex shader (when tess is used)
    */
   AC_HW_LOCAL_SHADER,

   /* GFX6-8:
    *    - tess control shader
    *
    * GFX9+:
    * Also known as surface shader.
    *    - merged vertex and tess control shader
    */
   AC_HW_HULL_SHADER,

   /* GFX6-8 only, merged into GS on GFX9+:
    *    - vertex shader before GS (when tess is not used)
    *    - tess eval shader before GS (when tess is used)
    */
   AC_HW_EXPORT_SHADER,

   /* GFX6-8:
    *    - geometry shader
    * GFX9-10/legacy:
    *    - merged vertex + geometry (when tess is not used)
    *    - merged tess eval + geometry (when tess is used)
    */
   AC_HW_LEGACY_GEOMETRY_SHADER,

   /* GFX6-10/legacy only:
    *    - vertex shader (when tess and GS are not used)
    *    - tess eval shader (when GS is not used),
    *    - "GS copy" shader (always when GS is used)
    */
   AC_HW_VERTEX_SHADER,

   /* GFX10+/NGG:
    * All pre-rasterization stages (after tess). Also known as primitive shader.
    *    - vertex shader (when tess and GS are not used)
    *    - tess eval shader (when GS is not used)
    *    - merged vertex + geometry shader (when GS is used but tess is not)
    *    - merged tess eval + geometry shader (when both tess and GS are used)
    *    - mesh shader
    */
   AC_HW_NEXT_GEN_GEOMETRY_SHADER,

   /* Fragment shader.
    * Call it "pixel shader" because that is how HW docs call it.
    */
   AC_HW_PIXEL_SHADER,

   /* Compute and compute-like shaders, such as task shader and ray tracing. */
   AC_HW_COMPUTE_SHADER,
};

#endif
