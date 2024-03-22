/*
 * Copyright 2011 Christoph Bumiller
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __NV50_IR_DRIVER_H__
#define __NV50_IR_DRIVER_H__

#include "compiler/shader_enums.h"
#include "util/macros.h"
#include "util/blob.h"

#define NV50_CODEGEN_MAX_VARYINGS 80
struct nir_shader;
struct nir_shader_compiler_options;

/*
 * This struct constitutes linkage information in TGSI terminology.
 *
 * It is created by the code generator and handed to the pipe driver
 * for input/output slot assignment.
 */
struct nv50_ir_varying
{
   uint8_t slot[4]; /* native slots for xyzw (addresses in 32-bit words) */

   unsigned mask     : 4; /* vec4 mask */
   unsigned linear   : 1; /* linearly interpolated if true (and not flat) */
   unsigned flat     : 1;
   unsigned sc       : 1; /* special colour interpolation mode (SHADE_MODEL) */
   unsigned centroid : 1;
   unsigned patch    : 1; /* patch constant value */
   unsigned regular  : 1; /* driver-specific meaning (e.g. input in sreg) */
   unsigned input    : 1; /* indicates direction of system values */
   unsigned oread    : 1; /* true if output is read from parallel TCP */

   uint8_t id; /* TGSI register index */
   uint8_t sn; /* TGSI semantic name */
   uint8_t si; /* TGSI semantic index */
};

struct nv50_ir_sysval
{
   gl_system_value sn;
   uint8_t slot[4]; /* for nv50: native slots for xyzw (addresses in 32-bit words) */
};

#ifndef NDEBUG
# define NV50_IR_DEBUG_BASIC     (1 << 0)
# define NV50_IR_DEBUG_VERBOSE   (2 << 0)
# define NV50_IR_DEBUG_REG_ALLOC (1 << 2)
#else
# define NV50_IR_DEBUG_BASIC     0
# define NV50_IR_DEBUG_VERBOSE   0
# define NV50_IR_DEBUG_REG_ALLOC 0
#endif

struct nv50_ir_prog_symbol
{
   uint32_t label;
   uint32_t offset;
};

#define NVISA_G80_CHIPSET      0x50
#define NVISA_GF100_CHIPSET    0xc0
#define NVISA_GK104_CHIPSET    0xe0
#define NVISA_GK20A_CHIPSET    0xea
#define NVISA_GM107_CHIPSET    0x110
#define NVISA_GM200_CHIPSET    0x120
#define NVISA_GV100_CHIPSET    0x140

struct nv50_ir_prog_info_out;

/* used for the input data and assignSlot interface */
struct nv50_ir_prog_info
{
   uint16_t target; /* chipset (0x50, 0x84, 0xc0, ...) */

   uint8_t type; /* PIPE_SHADER */

   uint8_t optLevel; /* optimization level (0 to 4). Level 4 enables MemoryOpt
                      * which does not work well with NVK */
   uint8_t dbgFlags;
   bool omitLineNum; /* only used for printing the prog when dbgFlags is set */

   struct {
      uint32_t smemSize;  /* required shared memory per block */
      struct nir_shader *nir;
   } bin;

   union {
      struct {
         uint32_t inputOffset; /* base address for user args */
         uint32_t gridInfoBase;  /* base address for NTID,NCTAID */
         uint16_t numThreads[3]; /* max number of threads */
      } cp;
   } prop;

   struct {
      int8_t genUserClip;        /* request user clip planes for ClipVertex */
      uint8_t auxCBSlot;         /* driver constant buffer slot */
      uint16_t ucpBase;          /* base address for UCPs */
      uint16_t drawInfoBase;     /* base address for draw parameters */
      uint16_t alphaRefBase;     /* base address for alpha test values */
      int8_t viewportId;         /* output index of ViewportIndex */
      bool mul_zero_wins;        /* program wants for x*0 = 0 */
      bool nv50styleSurfaces;    /* generate gX[] access for raw buffers */
      uint16_t texBindBase;      /* base address for tex handles (nve4) */
      uint16_t fbtexBindBase;    /* base address for fbtex handle (nve4) */
      uint16_t suInfoBase;       /* base address for surface info (nve4) */
      uint16_t bindlessBase;     /* base address for bindless image info (nve4) */
      uint16_t bufInfoBase;      /* base address for buffer info */
      uint16_t sampleInfoBase;   /* base address for sample positions */
      uint8_t msInfoCBSlot;      /* cX[] used for multisample info */
      uint16_t msInfoBase;       /* base address for multisample info */
      uint16_t uboInfoBase;      /* base address for compute UBOs (gk104+) */

      uint16_t membarOffset;     /* base address for membar reads (nv50) */
      uint8_t gmemMembar;        /* gX[] on which to perform membar reads (nv50) */
   } io;

   /* driver callback to assign input/output locations */
   int (*assignSlots)(struct nv50_ir_prog_info_out *);
};

/* the produced binary with metadata */
struct nv50_ir_prog_info_out
{
   uint16_t target; /* chipset (0x50, 0x84, 0xc0, ...) */

   uint8_t type; /* PIPE_SHADER */

   struct {
      int16_t maxGPR;     /* may be -1 if none used */
      uint32_t tlsSpace;  /* required local memory per thread */
      uint32_t smemSize;  /* required shared memory per block */
      uint32_t *code;
      uint32_t codeSize;
      uint32_t instructions;
      void *relocData;
      void *fixupData;
   } bin;

   struct nv50_ir_sysval sv[NV50_CODEGEN_MAX_VARYINGS];
   struct nv50_ir_varying in[NV50_CODEGEN_MAX_VARYINGS];
   struct nv50_ir_varying out[NV50_CODEGEN_MAX_VARYINGS];
   uint8_t numInputs;
   uint8_t numOutputs;
   uint8_t numPatchConstants; /* also included in numInputs/numOutputs */
   uint8_t numSysVals;
   uint32_t loops;

   union {
      struct {
         bool usesDrawParameters;
      } vp;
      struct {
         uint8_t outputPatchSize;
         uint8_t partitioning;    /* PIPE_TESS_PART */
         int8_t winding;          /* +1 (clockwise) / -1 (counter-clockwise) */
         uint8_t domain;          /* MESA_PRIM_{QUADS,TRIANGLES,LINES} */
         uint8_t outputPrim;      /* MESA_PRIM_{TRIANGLES,LINES,POINTS} */
      } tp;
      struct {
         uint8_t outputPrim;
         unsigned instanceCount;
         unsigned maxVertices;
      } gp;
      struct {
         unsigned numColourResults;
         bool writesDepth           : 1;
         bool earlyFragTests        : 1;
         bool postDepthCoverage     : 1;
         bool usesDiscard           : 1;
         bool usesSampleMaskIn      : 1;
         bool readsFramebuffer      : 1;
         bool readsSampleLocations  : 1;
         bool separateFragData      : 1;
      } fp;
      struct {
         struct {
            unsigned valid : 1;
            unsigned image : 1;
            unsigned slot  : 6;
         } gmem[16]; /* nv50 only */
      } cp;
   } prop;

   struct {
      uint8_t clipDistances;     /* number of clip distance outputs */
      uint8_t cullDistances;     /* number of cull distance outputs */
      int8_t genUserClip;        /* request user clip planes for ClipVertex */
      uint8_t instanceId;        /* system value index of InstanceID */
      uint8_t vertexId;          /* system value index of VertexID */
      uint8_t edgeFlagIn;
      uint8_t edgeFlagOut;
      uint8_t fragDepth;         /* output index of FragDepth */
      uint8_t sampleMask;        /* output index of SampleMask */
      uint8_t globalAccess;      /* 1 for read, 2 for wr, 3 for rw */
      bool fp64;                 /* program uses fp64 math */
      bool layer_viewport_relative;
   } io;

   uint8_t numBarriers;

   void *driverPriv;
};

#ifdef __cplusplus
extern "C" {
#endif

const struct nir_shader_compiler_options *
nv50_ir_nir_shader_compiler_options(int chipset, uint8_t shader_type);

extern int nv50_ir_generate_code(struct nv50_ir_prog_info *,
                                 struct nv50_ir_prog_info_out *);

extern void nv50_ir_relocate_code(void *relocData, uint32_t *code,
                                  uint32_t codePos,
                                  uint32_t libPos,
                                  uint32_t dataPos);

extern void
nv50_ir_apply_fixups(void *fixupData, uint32_t *code,
                     bool force_per_sample, bool flatshade,
                     uint8_t alphatest, bool msaa);

/* obtain code that will be shared among programs */
extern void nv50_ir_get_target_library(uint32_t chipset,
                                       const uint32_t **code, uint32_t *size);


#ifdef __cplusplus
namespace nv50_ir
{
   struct FixupEntry;
   struct FixupData;

   void
   gk110_interpApply(const nv50_ir::FixupEntry *entry, uint32_t *code,
                     const nv50_ir::FixupData& data);
   void
   gm107_interpApply(const nv50_ir::FixupEntry *entry, uint32_t *code,
                     const nv50_ir::FixupData& data);
   void
   nv50_interpApply(const nv50_ir::FixupEntry *entry, uint32_t *code,
                    const nv50_ir::FixupData& data);
   void
   nvc0_interpApply(const nv50_ir::FixupEntry *entry, uint32_t *code,
                    const nv50_ir::FixupData& data);
   void
   gv100_interpApply(const nv50_ir::FixupEntry *entry, uint32_t *code,
                     const nv50_ir::FixupData& data);
   void
   gk110_selpFlip(const nv50_ir::FixupEntry *entry, uint32_t *code,
                  const nv50_ir::FixupData& data);
   void
   gm107_selpFlip(const nv50_ir::FixupEntry *entry, uint32_t *code,
                  const nv50_ir::FixupData& data);
   void
   nvc0_selpFlip(const nv50_ir::FixupEntry *entry, uint32_t *code,
                 const nv50_ir::FixupData& data);
   void
   gv100_selpFlip(const nv50_ir::FixupEntry *entry, uint32_t *code,
                  const nv50_ir::FixupData& data);
}
#endif

extern void
nv50_ir_prog_info_out_print(struct nv50_ir_prog_info_out *);

/* Serialize a nv50_ir_prog_info structure and save it into blob */
extern bool
nv50_ir_prog_info_serialize(struct blob *, struct nv50_ir_prog_info *);

/* Serialize a nv50_ir_prog_info_out structure and save it into blob */
extern bool MUST_CHECK
nv50_ir_prog_info_out_serialize(struct blob *, struct nv50_ir_prog_info_out *);

/* Deserialize from data and save into a nv50_ir_prog_info_out structure
 * using a pointer. Size is a total size of the serialized data.
 * Offset points to where info_out in data is located. */
extern bool MUST_CHECK
nv50_ir_prog_info_out_deserialize(void *data, size_t size, size_t offset,
                                  struct nv50_ir_prog_info_out *);

#ifdef __cplusplus
}
#endif

#endif // __NV50_IR_DRIVER_H__
