#include "util/blob.h"
#include "nv50_ir_driver.h"
#include "nv50_ir.h"
#include "nv50_ir_target.h"
#include "nv50_ir_driver.h"
#include "compiler/nir/nir_serialize.h"

enum FixupApplyFunc {
   APPLY_NV50,
   APPLY_NVC0,
   APPLY_GK110,
   APPLY_GM107,
   APPLY_GV100,
   FLIP_NVC0,
   FLIP_GK110,
   FLIP_GM107,
   FLIP_GV100,
};

extern bool
nv50_ir_prog_info_serialize(struct blob *blob, struct nv50_ir_prog_info *info)
{
   blob_write_uint32(blob, info->bin.smemSize);
   blob_write_uint16(blob, info->target);
   blob_write_uint8(blob, info->type);
   blob_write_uint8(blob, info->optLevel);
   blob_write_uint8(blob, info->dbgFlags);
   blob_write_uint8(blob, info->omitLineNum);

   nir_serialize(blob, info->bin.nir, true);

   if (info->type == PIPE_SHADER_COMPUTE)
      blob_write_bytes(blob, &info->prop.cp, sizeof(info->prop.cp));

   blob_write_bytes(blob, &info->io, sizeof(info->io));

   return true;
}

extern bool
nv50_ir_prog_info_out_serialize(struct blob *blob,
                                struct nv50_ir_prog_info_out *info_out)
{
   blob_write_uint16(blob, info_out->target);
   blob_write_uint8(blob, info_out->type);
   blob_write_uint8(blob, info_out->numPatchConstants);

   blob_write_uint16(blob, info_out->bin.maxGPR);
   blob_write_uint32(blob, info_out->bin.tlsSpace);
   blob_write_uint32(blob, info_out->bin.smemSize);
   blob_write_uint32(blob, info_out->bin.codeSize);
   blob_write_bytes(blob, info_out->bin.code, info_out->bin.codeSize);
   blob_write_uint32(blob, info_out->bin.instructions);

   if (!info_out->bin.relocData) {
      blob_write_uint32(blob, 0); // reloc count 0
   } else {
      nv50_ir::RelocInfo *reloc = (nv50_ir::RelocInfo *)info_out->bin.relocData;
      blob_write_uint32(blob, reloc->count);
      blob_write_uint32(blob, reloc->codePos);
      blob_write_uint32(blob, reloc->libPos);
      blob_write_uint32(blob, reloc->dataPos);
      blob_write_bytes(blob, reloc->entry, sizeof(*reloc->entry) * reloc->count);
   }

   if (!info_out->bin.fixupData) {
      blob_write_uint32(blob, 0); // fixup count 0
   } else {
      nv50_ir::FixupInfo *fixup = (nv50_ir::FixupInfo *)info_out->bin.fixupData;
      blob_write_uint32(blob, fixup->count);

      /* Going through each entry */
      for (uint32_t i = 0; i < fixup->count; i++) {
         blob_write_uint32(blob, fixup->entry[i].val);
         assert(fixup->entry[i].apply);
         /* Compare function pointers, for when at serializing
          * to know which function to apply */
         if (fixup->entry[i].apply == nv50_ir::nv50_interpApply)
            blob_write_uint8(blob, APPLY_NV50);
         else if (fixup->entry[i].apply == nv50_ir::nvc0_interpApply)
            blob_write_uint8(blob, APPLY_NVC0);
         else if (fixup->entry[i].apply == nv50_ir::gk110_interpApply)
            blob_write_uint8(blob, APPLY_GK110);
         else if (fixup->entry[i].apply == nv50_ir::gm107_interpApply)
            blob_write_uint8(blob, APPLY_GM107);
         else if (fixup->entry[i].apply == nv50_ir::gv100_interpApply)
            blob_write_uint8(blob, APPLY_GV100);
         else if (fixup->entry[i].apply == nv50_ir::nvc0_selpFlip)
            blob_write_uint8(blob, FLIP_NVC0);
         else if (fixup->entry[i].apply == nv50_ir::gk110_selpFlip)
            blob_write_uint8(blob, FLIP_GK110);
         else if (fixup->entry[i].apply == nv50_ir::gm107_selpFlip)
            blob_write_uint8(blob, FLIP_GM107);
         else if (fixup->entry[i].apply == nv50_ir::gv100_selpFlip)
            blob_write_uint8(blob, FLIP_GV100);
         else {
            ERROR("unhandled fixup apply function pointer\n");
            assert(false);
            return false;
         }
      }
   }

   blob_write_uint8(blob, info_out->numInputs);
   blob_write_uint8(blob, info_out->numOutputs);
   blob_write_uint8(blob, info_out->numSysVals);
   blob_write_bytes(blob, info_out->sv, info_out->numSysVals * sizeof(info_out->sv[0]));
   blob_write_bytes(blob, info_out->in, info_out->numInputs * sizeof(info_out->in[0]));
   blob_write_bytes(blob, info_out->out, info_out->numOutputs * sizeof(info_out->out[0]));

   switch(info_out->type) {
      case PIPE_SHADER_VERTEX:
         blob_write_bytes(blob, &info_out->prop.vp, sizeof(info_out->prop.vp));
         break;
      case PIPE_SHADER_TESS_CTRL:
      case PIPE_SHADER_TESS_EVAL:
         blob_write_bytes(blob, &info_out->prop.tp, sizeof(info_out->prop.tp));
         break;
      case PIPE_SHADER_GEOMETRY:
         blob_write_bytes(blob, &info_out->prop.gp, sizeof(info_out->prop.gp));
         break;
      case PIPE_SHADER_FRAGMENT:
         blob_write_bytes(blob, &info_out->prop.fp, sizeof(info_out->prop.fp));
         break;
      case PIPE_SHADER_COMPUTE:
         blob_write_bytes(blob, &info_out->prop.cp, sizeof(info_out->prop.cp));
         break;
      default:
         break;
   }
   blob_write_bytes(blob, &info_out->io, sizeof(info_out->io));
   blob_write_uint8(blob, info_out->numBarriers);

   return true;
}

extern bool
nv50_ir_prog_info_out_deserialize(void *data, size_t size, size_t offset,
                                  struct nv50_ir_prog_info_out *info_out)
{
   struct blob_reader reader;
   blob_reader_init(&reader, data, size);
   blob_skip_bytes(&reader, offset);

   info_out->target = blob_read_uint16(&reader);
   info_out->type = blob_read_uint8(&reader);
   info_out->numPatchConstants = blob_read_uint8(&reader);

   info_out->bin.maxGPR = blob_read_uint16(&reader);
   info_out->bin.tlsSpace = blob_read_uint32(&reader);
   info_out->bin.smemSize = blob_read_uint32(&reader);
   info_out->bin.codeSize = blob_read_uint32(&reader);
   info_out->bin.code = (uint32_t *)MALLOC(info_out->bin.codeSize);
   blob_copy_bytes(&reader, info_out->bin.code, info_out->bin.codeSize);
   info_out->bin.instructions = blob_read_uint32(&reader);

   info_out->bin.relocData = NULL;
   /*  Check if data contains RelocInfo */
   uint32_t count = blob_read_uint32(&reader);
   if (count) {
      nv50_ir::RelocInfo *reloc =
                  CALLOC_VARIANT_LENGTH_STRUCT(nv50_ir::RelocInfo,
                                               count * sizeof(*reloc->entry));
      reloc->codePos = blob_read_uint32(&reader);
      reloc->libPos = blob_read_uint32(&reader);
      reloc->dataPos = blob_read_uint32(&reader);
      reloc->count = count;

      blob_copy_bytes(&reader, reloc->entry, sizeof(*reloc->entry) * reloc->count);
      info_out->bin.relocData = reloc;
   }

   info_out->bin.fixupData = NULL;
   /* Check if data contains FixupInfo */
   count = blob_read_uint32(&reader);
   if (count) {
      nv50_ir::FixupInfo *fixup =
                  CALLOC_VARIANT_LENGTH_STRUCT(nv50_ir::FixupInfo,
                                               count * sizeof(*fixup->entry));
      fixup->count = count;

      for (uint32_t i = 0; i < count; i++) {
         fixup->entry[i].val = blob_read_uint32(&reader);

         /* Assign back function pointer depending on stored enum */
         enum FixupApplyFunc apply = (enum FixupApplyFunc)blob_read_uint8(&reader);
         switch(apply) {
            case APPLY_NV50:
               fixup->entry[i].apply = nv50_ir::nv50_interpApply;
               break;
            case APPLY_NVC0:
               fixup->entry[i].apply = nv50_ir::nvc0_interpApply;
               break;
            case APPLY_GK110:
               fixup->entry[i].apply = nv50_ir::gk110_interpApply;
               break;
            case APPLY_GM107:
               fixup->entry[i].apply = nv50_ir::gm107_interpApply;
               break;
            case APPLY_GV100:
               fixup->entry[i].apply = nv50_ir::gv100_interpApply;
               break;
            case FLIP_NVC0:
               fixup->entry[i].apply = nv50_ir::nvc0_selpFlip;
               break;
            case FLIP_GK110:
               fixup->entry[i].apply = nv50_ir::gk110_selpFlip;
               break;
            case FLIP_GM107:
               fixup->entry[i].apply = nv50_ir::gm107_selpFlip;
               break;
            case FLIP_GV100:
               fixup->entry[i].apply = nv50_ir::gv100_selpFlip;
               break;
            default:
               ERROR("unhandled fixup apply function switch case");
               assert(false);
               return false;
         }
      }
      info_out->bin.fixupData = fixup;
   }

   info_out->numInputs = blob_read_uint8(&reader);
   info_out->numOutputs = blob_read_uint8(&reader);
   info_out->numSysVals = blob_read_uint8(&reader);
   blob_copy_bytes(&reader, info_out->sv, info_out->numSysVals * sizeof(info_out->sv[0]));
   blob_copy_bytes(&reader, info_out->in, info_out->numInputs * sizeof(info_out->in[0]));
   blob_copy_bytes(&reader, info_out->out, info_out->numOutputs * sizeof(info_out->out[0]));

   switch(info_out->type) {
      case PIPE_SHADER_VERTEX:
         blob_copy_bytes(&reader, &info_out->prop.vp, sizeof(info_out->prop.vp));
         break;
      case PIPE_SHADER_TESS_CTRL:
      case PIPE_SHADER_TESS_EVAL:
         blob_copy_bytes(&reader, &info_out->prop.tp, sizeof(info_out->prop.tp));
         break;
      case PIPE_SHADER_GEOMETRY:
         blob_copy_bytes(&reader, &info_out->prop.gp, sizeof(info_out->prop.gp));
         break;
      case PIPE_SHADER_FRAGMENT:
         blob_copy_bytes(&reader, &info_out->prop.fp, sizeof(info_out->prop.fp));
         break;
      case PIPE_SHADER_COMPUTE:
         blob_copy_bytes(&reader, &info_out->prop.cp, sizeof(info_out->prop.cp));
         break;
      default:
         break;
   }
   blob_copy_bytes(&reader, &(info_out->io), sizeof(info_out->io));
   info_out->numBarriers = blob_read_uint8(&reader);

   return true;
}
