/**********************************************************
 * Copyright 2009 VMware, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************/

/**
 * @file
 * Dump SVGA commands.
 *
 * Generated automatically from svga3d_reg.h by svga_dump.py.
 */

#include "../svga_format.h"
#include "svga_types.h"
#include "svga_shader_dump.h"
#include "svga3d_reg.h"

#include "util/u_debug.h"
#include "svga_dump.h"

static const char *
shader_name(unsigned type)
{
   switch (type) {
   case SVGA3D_SHADERTYPE_VS:
      return "SVGA3D_SHADERTYPE_VS";
   case SVGA3D_SHADERTYPE_PS:
      return "SVGA3D_SHADERTYPE_PS";
   case SVGA3D_SHADERTYPE_GS:
      return "SVGA3D_SHADERTYPE_GS";
   default:
      return "unknown shader type!";
   }
}


static void
dump_SVGA3dVertexDecl(const SVGA3dVertexDecl *cmd)
{
   switch((*cmd).identity.type) {
   case SVGA3D_DECLTYPE_FLOAT1:
      _debug_printf("\t\t.identity.type = SVGA3D_DECLTYPE_FLOAT1\n");
      break;
   case SVGA3D_DECLTYPE_FLOAT2:
      _debug_printf("\t\t.identity.type = SVGA3D_DECLTYPE_FLOAT2\n");
      break;
   case SVGA3D_DECLTYPE_FLOAT3:
      _debug_printf("\t\t.identity.type = SVGA3D_DECLTYPE_FLOAT3\n");
      break;
   case SVGA3D_DECLTYPE_FLOAT4:
      _debug_printf("\t\t.identity.type = SVGA3D_DECLTYPE_FLOAT4\n");
      break;
   case SVGA3D_DECLTYPE_D3DCOLOR:
      _debug_printf("\t\t.identity.type = SVGA3D_DECLTYPE_D3DCOLOR\n");
      break;
   case SVGA3D_DECLTYPE_UBYTE4:
      _debug_printf("\t\t.identity.type = SVGA3D_DECLTYPE_UBYTE4\n");
      break;
   case SVGA3D_DECLTYPE_SHORT2:
      _debug_printf("\t\t.identity.type = SVGA3D_DECLTYPE_SHORT2\n");
      break;
   case SVGA3D_DECLTYPE_SHORT4:
      _debug_printf("\t\t.identity.type = SVGA3D_DECLTYPE_SHORT4\n");
      break;
   case SVGA3D_DECLTYPE_UBYTE4N:
      _debug_printf("\t\t.identity.type = SVGA3D_DECLTYPE_UBYTE4N\n");
      break;
   case SVGA3D_DECLTYPE_SHORT2N:
      _debug_printf("\t\t.identity.type = SVGA3D_DECLTYPE_SHORT2N\n");
      break;
   case SVGA3D_DECLTYPE_SHORT4N:
      _debug_printf("\t\t.identity.type = SVGA3D_DECLTYPE_SHORT4N\n");
      break;
   case SVGA3D_DECLTYPE_USHORT2N:
      _debug_printf("\t\t.identity.type = SVGA3D_DECLTYPE_USHORT2N\n");
      break;
   case SVGA3D_DECLTYPE_USHORT4N:
      _debug_printf("\t\t.identity.type = SVGA3D_DECLTYPE_USHORT4N\n");
      break;
   case SVGA3D_DECLTYPE_UDEC3:
      _debug_printf("\t\t.identity.type = SVGA3D_DECLTYPE_UDEC3\n");
      break;
   case SVGA3D_DECLTYPE_DEC3N:
      _debug_printf("\t\t.identity.type = SVGA3D_DECLTYPE_DEC3N\n");
      break;
   case SVGA3D_DECLTYPE_FLOAT16_2:
      _debug_printf("\t\t.identity.type = SVGA3D_DECLTYPE_FLOAT16_2\n");
      break;
   case SVGA3D_DECLTYPE_FLOAT16_4:
      _debug_printf("\t\t.identity.type = SVGA3D_DECLTYPE_FLOAT16_4\n");
      break;
   case SVGA3D_DECLTYPE_MAX:
      _debug_printf("\t\t.identity.type = SVGA3D_DECLTYPE_MAX\n");
      break;
   default:
      _debug_printf("\t\t.identity.type = %i\n", (*cmd).identity.type);
      break;
   }
   switch((*cmd).identity.method) {
   case SVGA3D_DECLMETHOD_DEFAULT:
      _debug_printf("\t\t.identity.method = SVGA3D_DECLMETHOD_DEFAULT\n");
      break;
   case SVGA3D_DECLMETHOD_PARTIALU:
      _debug_printf("\t\t.identity.method = SVGA3D_DECLMETHOD_PARTIALU\n");
      break;
   case SVGA3D_DECLMETHOD_PARTIALV:
      _debug_printf("\t\t.identity.method = SVGA3D_DECLMETHOD_PARTIALV\n");
      break;
   case SVGA3D_DECLMETHOD_CROSSUV:
      _debug_printf("\t\t.identity.method = SVGA3D_DECLMETHOD_CROSSUV\n");
      break;
   case SVGA3D_DECLMETHOD_UV:
      _debug_printf("\t\t.identity.method = SVGA3D_DECLMETHOD_UV\n");
      break;
   case SVGA3D_DECLMETHOD_LOOKUP:
      _debug_printf("\t\t.identity.method = SVGA3D_DECLMETHOD_LOOKUP\n");
      break;
   case SVGA3D_DECLMETHOD_LOOKUPPRESAMPLED:
      _debug_printf("\t\t.identity.method = SVGA3D_DECLMETHOD_LOOKUPPRESAMPLED\n");
      break;
   default:
      _debug_printf("\t\t.identity.method = %i\n", (*cmd).identity.method);
      break;
   }
   switch((*cmd).identity.usage) {
   case SVGA3D_DECLUSAGE_POSITION:
      _debug_printf("\t\t.identity.usage = SVGA3D_DECLUSAGE_POSITION\n");
      break;
   case SVGA3D_DECLUSAGE_BLENDWEIGHT:
      _debug_printf("\t\t.identity.usage = SVGA3D_DECLUSAGE_BLENDWEIGHT\n");
      break;
   case SVGA3D_DECLUSAGE_BLENDINDICES:
      _debug_printf("\t\t.identity.usage = SVGA3D_DECLUSAGE_BLENDINDICES\n");
      break;
   case SVGA3D_DECLUSAGE_NORMAL:
      _debug_printf("\t\t.identity.usage = SVGA3D_DECLUSAGE_NORMAL\n");
      break;
   case SVGA3D_DECLUSAGE_PSIZE:
      _debug_printf("\t\t.identity.usage = SVGA3D_DECLUSAGE_PSIZE\n");
      break;
   case SVGA3D_DECLUSAGE_TEXCOORD:
      _debug_printf("\t\t.identity.usage = SVGA3D_DECLUSAGE_TEXCOORD\n");
      break;
   case SVGA3D_DECLUSAGE_TANGENT:
      _debug_printf("\t\t.identity.usage = SVGA3D_DECLUSAGE_TANGENT\n");
      break;
   case SVGA3D_DECLUSAGE_BINORMAL:
      _debug_printf("\t\t.identity.usage = SVGA3D_DECLUSAGE_BINORMAL\n");
      break;
   case SVGA3D_DECLUSAGE_TESSFACTOR:
      _debug_printf("\t\t.identity.usage = SVGA3D_DECLUSAGE_TESSFACTOR\n");
      break;
   case SVGA3D_DECLUSAGE_POSITIONT:
      _debug_printf("\t\t.identity.usage = SVGA3D_DECLUSAGE_POSITIONT\n");
      break;
   case SVGA3D_DECLUSAGE_COLOR:
      _debug_printf("\t\t.identity.usage = SVGA3D_DECLUSAGE_COLOR\n");
      break;
   case SVGA3D_DECLUSAGE_FOG:
      _debug_printf("\t\t.identity.usage = SVGA3D_DECLUSAGE_FOG\n");
      break;
   case SVGA3D_DECLUSAGE_DEPTH:
      _debug_printf("\t\t.identity.usage = SVGA3D_DECLUSAGE_DEPTH\n");
      break;
   case SVGA3D_DECLUSAGE_SAMPLE:
      _debug_printf("\t\t.identity.usage = SVGA3D_DECLUSAGE_SAMPLE\n");
      break;
   case SVGA3D_DECLUSAGE_MAX:
      _debug_printf("\t\t.identity.usage = SVGA3D_DECLUSAGE_MAX\n");
      break;
   default:
      _debug_printf("\t\t.identity.usage = %i\n", (*cmd).identity.usage);
      break;
   }
   _debug_printf("\t\t.identity.usageIndex = %u\n", (*cmd).identity.usageIndex);
   _debug_printf("\t\t.array.surfaceId = %u\n", (*cmd).array.surfaceId);
   _debug_printf("\t\t.array.offset = %u\n", (*cmd).array.offset);
   _debug_printf("\t\t.array.stride = %u\n", (*cmd).array.stride);
   _debug_printf("\t\t.rangeHint.first = %u\n", (*cmd).rangeHint.first);
   _debug_printf("\t\t.rangeHint.last = %u\n", (*cmd).rangeHint.last);
}

static void
dump_SVGA3dTextureState(const SVGA3dTextureState *cmd)
{
   _debug_printf("\t\t.stage = %u\n", (*cmd).stage);
   switch((*cmd).name) {
   case SVGA3D_TS_INVALID:
      _debug_printf("\t\t.name = SVGA3D_TS_INVALID\n");
      break;
   case SVGA3D_TS_BIND_TEXTURE:
      _debug_printf("\t\t.name = SVGA3D_TS_BIND_TEXTURE\n");
      break;
   case SVGA3D_TS_COLOROP:
      _debug_printf("\t\t.name = SVGA3D_TS_COLOROP\n");
      break;
   case SVGA3D_TS_COLORARG1:
      _debug_printf("\t\t.name = SVGA3D_TS_COLORARG1\n");
      break;
   case SVGA3D_TS_COLORARG2:
      _debug_printf("\t\t.name = SVGA3D_TS_COLORARG2\n");
      break;
   case SVGA3D_TS_ALPHAOP:
      _debug_printf("\t\t.name = SVGA3D_TS_ALPHAOP\n");
      break;
   case SVGA3D_TS_ALPHAARG1:
      _debug_printf("\t\t.name = SVGA3D_TS_ALPHAARG1\n");
      break;
   case SVGA3D_TS_ALPHAARG2:
      _debug_printf("\t\t.name = SVGA3D_TS_ALPHAARG2\n");
      break;
   case SVGA3D_TS_ADDRESSU:
      _debug_printf("\t\t.name = SVGA3D_TS_ADDRESSU\n");
      break;
   case SVGA3D_TS_ADDRESSV:
      _debug_printf("\t\t.name = SVGA3D_TS_ADDRESSV\n");
      break;
   case SVGA3D_TS_MIPFILTER:
      _debug_printf("\t\t.name = SVGA3D_TS_MIPFILTER\n");
      break;
   case SVGA3D_TS_MAGFILTER:
      _debug_printf("\t\t.name = SVGA3D_TS_MAGFILTER\n");
      break;
   case SVGA3D_TS_MINFILTER:
      _debug_printf("\t\t.name = SVGA3D_TS_MINFILTER\n");
      break;
   case SVGA3D_TS_BORDERCOLOR:
      _debug_printf("\t\t.name = SVGA3D_TS_BORDERCOLOR\n");
      break;
   case SVGA3D_TS_TEXCOORDINDEX:
      _debug_printf("\t\t.name = SVGA3D_TS_TEXCOORDINDEX\n");
      break;
   case SVGA3D_TS_TEXTURETRANSFORMFLAGS:
      _debug_printf("\t\t.name = SVGA3D_TS_TEXTURETRANSFORMFLAGS\n");
      break;
   case SVGA3D_TS_TEXCOORDGEN:
      _debug_printf("\t\t.name = SVGA3D_TS_TEXCOORDGEN\n");
      break;
   case SVGA3D_TS_BUMPENVMAT00:
      _debug_printf("\t\t.name = SVGA3D_TS_BUMPENVMAT00\n");
      break;
   case SVGA3D_TS_BUMPENVMAT01:
      _debug_printf("\t\t.name = SVGA3D_TS_BUMPENVMAT01\n");
      break;
   case SVGA3D_TS_BUMPENVMAT10:
      _debug_printf("\t\t.name = SVGA3D_TS_BUMPENVMAT10\n");
      break;
   case SVGA3D_TS_BUMPENVMAT11:
      _debug_printf("\t\t.name = SVGA3D_TS_BUMPENVMAT11\n");
      break;
   case SVGA3D_TS_TEXTURE_MIPMAP_LEVEL:
      _debug_printf("\t\t.name = SVGA3D_TS_TEXTURE_MIPMAP_LEVEL\n");
      break;
   case SVGA3D_TS_TEXTURE_LOD_BIAS:
      _debug_printf("\t\t.name = SVGA3D_TS_TEXTURE_LOD_BIAS\n");
      break;
   case SVGA3D_TS_TEXTURE_ANISOTROPIC_LEVEL:
      _debug_printf("\t\t.name = SVGA3D_TS_TEXTURE_ANISOTROPIC_LEVEL\n");
      break;
   case SVGA3D_TS_ADDRESSW:
      _debug_printf("\t\t.name = SVGA3D_TS_ADDRESSW\n");
      break;
   case SVGA3D_TS_GAMMA:
      _debug_printf("\t\t.name = SVGA3D_TS_GAMMA\n");
      break;
   case SVGA3D_TS_BUMPENVLSCALE:
      _debug_printf("\t\t.name = SVGA3D_TS_BUMPENVLSCALE\n");
      break;
   case SVGA3D_TS_BUMPENVLOFFSET:
      _debug_printf("\t\t.name = SVGA3D_TS_BUMPENVLOFFSET\n");
      break;
   case SVGA3D_TS_COLORARG0:
      _debug_printf("\t\t.name = SVGA3D_TS_COLORARG0\n");
      break;
   case SVGA3D_TS_ALPHAARG0:
      _debug_printf("\t\t.name = SVGA3D_TS_ALPHAARG0\n");
      break;
   case SVGA3D_TS_MAX:
      _debug_printf("\t\t.name = SVGA3D_TS_MAX\n");
      break;
   default:
      _debug_printf("\t\t.name = %i\n", (*cmd).name);
      break;
   }
   _debug_printf("\t\t.value = %u\n", (*cmd).value);
   _debug_printf("\t\t.floatValue = %f\n", (*cmd).floatValue);
}

static void
dump_SVGA3dViewport(const SVGA3dViewport *cmd)
{
   _debug_printf("\t\t.x = %f\n", (*cmd).x);
   _debug_printf("\t\t.y = %f\n", (*cmd).y);
   _debug_printf("\t\t.width = %f\n", (*cmd).width);
   _debug_printf("\t\t.height = %f\n", (*cmd).height);
   _debug_printf("\t\t.minDepth = %f\n", (*cmd).minDepth);
   _debug_printf("\t\t.maxDepth = %f\n", (*cmd).maxDepth);
}

static void
dump_SVGA3dSamplerId(const SVGA3dSamplerId *cmd)
{
   _debug_printf("\t\t.id = %u\n", *cmd);
}

static void
dump_SVGA3dSoTarget(const SVGA3dSoTarget *cmd)
{
   _debug_printf("\t\t.sid = %u\n", (*cmd).sid);
   _debug_printf("\t\t.offset = %u\n", (*cmd).offset);
}

static void
dump_SVGA3dInputElementDesc(const SVGA3dInputElementDesc *cmd)
{
   _debug_printf("\t\t.inputSlot = %u\n", (*cmd).inputSlot);
   _debug_printf("\t\t.alignedByteOffset = %u\n", (*cmd).alignedByteOffset);
   _debug_printf("\t\t.format = %s\n", svga_format_name((*cmd).format));
   _debug_printf("\t\t.inputSlotClass = %u\n", (*cmd).inputSlotClass);
   _debug_printf("\t\t.instanceDataStepRate = %u\n", (*cmd).instanceDataStepRate);
   _debug_printf("\t\t.inputRegister = %u\n", (*cmd).inputRegister);
}

static void
dump_SVGA3dVertexBuffer(const SVGA3dVertexBuffer *cmd)
{
   _debug_printf("\t\t.sid = %u\n", (*cmd).sid);
   _debug_printf("\t\t.stride = %u\n", (*cmd).stride);
   _debug_printf("\t\t.offset = %u\n", (*cmd).offset);
}

static void
dump_SVGA3dCopyBox(const SVGA3dCopyBox *cmd)
{
   _debug_printf("\t\t.x = %u\n", (*cmd).x);
   _debug_printf("\t\t.y = %u\n", (*cmd).y);
   _debug_printf("\t\t.z = %u\n", (*cmd).z);
   _debug_printf("\t\t.w = %u\n", (*cmd).w);
   _debug_printf("\t\t.h = %u\n", (*cmd).h);
   _debug_printf("\t\t.d = %u\n", (*cmd).d);
   _debug_printf("\t\t.srcx = %u\n", (*cmd).srcx);
   _debug_printf("\t\t.srcy = %u\n", (*cmd).srcy);
   _debug_printf("\t\t.srcz = %u\n", (*cmd).srcz);
}

static void
dump_SVGA3dShaderResourceViewId(const SVGA3dShaderResourceViewId *id)
{
   _debug_printf("\t\t.id = %u\n", *id);
}

static void
dump_SVGA3dCmdSetClipPlane(const SVGA3dCmdSetClipPlane *cmd)
{
   _debug_printf("\t\t.cid = %u\n", (*cmd).cid);
   _debug_printf("\t\t.index = %u\n", (*cmd).index);
   _debug_printf("\t\t.plane[0] = %f\n", (*cmd).plane[0]);
   _debug_printf("\t\t.plane[1] = %f\n", (*cmd).plane[1]);
   _debug_printf("\t\t.plane[2] = %f\n", (*cmd).plane[2]);
   _debug_printf("\t\t.plane[3] = %f\n", (*cmd).plane[3]);
}

static void
dump_SVGA3dCmdWaitForQuery(const SVGA3dCmdWaitForQuery *cmd)
{
   _debug_printf("\t\t.cid = %u\n", (*cmd).cid);
   switch((*cmd).type) {
   case SVGA3D_QUERYTYPE_OCCLUSION:
      _debug_printf("\t\t.type = SVGA3D_QUERYTYPE_OCCLUSION\n");
      break;
   case SVGA3D_QUERYTYPE_MAX:
      _debug_printf("\t\t.type = SVGA3D_QUERYTYPE_MAX\n");
      break;
   default:
      _debug_printf("\t\t.type = %i\n", (*cmd).type);
      break;
   }
   _debug_printf("\t\t.guestResult.gmrId = %u\n", (*cmd).guestResult.gmrId);
   _debug_printf("\t\t.guestResult.offset = %u\n", (*cmd).guestResult.offset);
}

static void
dump_SVGA3dCmdSetRenderTarget(const SVGA3dCmdSetRenderTarget *cmd)
{
   _debug_printf("\t\t.cid = %u\n", (*cmd).cid);
   switch((*cmd).type) {
   case SVGA3D_RT_DEPTH:
      _debug_printf("\t\t.type = SVGA3D_RT_DEPTH\n");
      break;
   case SVGA3D_RT_STENCIL:
      _debug_printf("\t\t.type = SVGA3D_RT_STENCIL\n");
      break;
   default:
      _debug_printf("\t\t.type = SVGA3D_RT_COLOR%u\n", (*cmd).type - SVGA3D_RT_COLOR0);
      break;
   }
   _debug_printf("\t\t.target.sid = %u\n", (*cmd).target.sid);
   _debug_printf("\t\t.target.face = %u\n", (*cmd).target.face);
   _debug_printf("\t\t.target.mipmap = %u\n", (*cmd).target.mipmap);
}

static void
dump_SVGA3dCmdSetTextureState(const SVGA3dCmdSetTextureState *cmd)
{
   _debug_printf("\t\t.cid = %u\n", (*cmd).cid);
}

static void
dump_SVGA3dCmdSurfaceCopy(const SVGA3dCmdSurfaceCopy *cmd)
{
   _debug_printf("\t\t.src.sid = %u\n", (*cmd).src.sid);
   _debug_printf("\t\t.src.face = %u\n", (*cmd).src.face);
   _debug_printf("\t\t.src.mipmap = %u\n", (*cmd).src.mipmap);
   _debug_printf("\t\t.dest.sid = %u\n", (*cmd).dest.sid);
   _debug_printf("\t\t.dest.face = %u\n", (*cmd).dest.face);
   _debug_printf("\t\t.dest.mipmap = %u\n", (*cmd).dest.mipmap);
}

static void
dump_SVGA3dCmdSetMaterial(const SVGA3dCmdSetMaterial *cmd)
{
   _debug_printf("\t\t.cid = %u\n", (*cmd).cid);
   switch((*cmd).face) {
   case SVGA3D_FACE_INVALID:
      _debug_printf("\t\t.face = SVGA3D_FACE_INVALID\n");
      break;
   case SVGA3D_FACE_NONE:
      _debug_printf("\t\t.face = SVGA3D_FACE_NONE\n");
      break;
   case SVGA3D_FACE_FRONT:
      _debug_printf("\t\t.face = SVGA3D_FACE_FRONT\n");
      break;
   case SVGA3D_FACE_BACK:
      _debug_printf("\t\t.face = SVGA3D_FACE_BACK\n");
      break;
   case SVGA3D_FACE_FRONT_BACK:
      _debug_printf("\t\t.face = SVGA3D_FACE_FRONT_BACK\n");
      break;
   case SVGA3D_FACE_MAX:
      _debug_printf("\t\t.face = SVGA3D_FACE_MAX\n");
      break;
   default:
      _debug_printf("\t\t.face = %i\n", (*cmd).face);
      break;
   }
   _debug_printf("\t\t.material.diffuse[0] = %f\n", (*cmd).material.diffuse[0]);
   _debug_printf("\t\t.material.diffuse[1] = %f\n", (*cmd).material.diffuse[1]);
   _debug_printf("\t\t.material.diffuse[2] = %f\n", (*cmd).material.diffuse[2]);
   _debug_printf("\t\t.material.diffuse[3] = %f\n", (*cmd).material.diffuse[3]);
   _debug_printf("\t\t.material.ambient[0] = %f\n", (*cmd).material.ambient[0]);
   _debug_printf("\t\t.material.ambient[1] = %f\n", (*cmd).material.ambient[1]);
   _debug_printf("\t\t.material.ambient[2] = %f\n", (*cmd).material.ambient[2]);
   _debug_printf("\t\t.material.ambient[3] = %f\n", (*cmd).material.ambient[3]);
   _debug_printf("\t\t.material.specular[0] = %f\n", (*cmd).material.specular[0]);
   _debug_printf("\t\t.material.specular[1] = %f\n", (*cmd).material.specular[1]);
   _debug_printf("\t\t.material.specular[2] = %f\n", (*cmd).material.specular[2]);
   _debug_printf("\t\t.material.specular[3] = %f\n", (*cmd).material.specular[3]);
   _debug_printf("\t\t.material.emissive[0] = %f\n", (*cmd).material.emissive[0]);
   _debug_printf("\t\t.material.emissive[1] = %f\n", (*cmd).material.emissive[1]);
   _debug_printf("\t\t.material.emissive[2] = %f\n", (*cmd).material.emissive[2]);
   _debug_printf("\t\t.material.emissive[3] = %f\n", (*cmd).material.emissive[3]);
   _debug_printf("\t\t.material.shininess = %f\n", (*cmd).material.shininess);
}

static void
dump_SVGA3dCmdSetLightData(const SVGA3dCmdSetLightData *cmd)
{
   _debug_printf("\t\t.cid = %u\n", (*cmd).cid);
   _debug_printf("\t\t.index = %u\n", (*cmd).index);
   switch((*cmd).data.type) {
   case SVGA3D_LIGHTTYPE_INVALID:
      _debug_printf("\t\t.data.type = SVGA3D_LIGHTTYPE_INVALID\n");
      break;
   case SVGA3D_LIGHTTYPE_POINT:
      _debug_printf("\t\t.data.type = SVGA3D_LIGHTTYPE_POINT\n");
      break;
   case SVGA3D_LIGHTTYPE_SPOT1:
      _debug_printf("\t\t.data.type = SVGA3D_LIGHTTYPE_SPOT1\n");
      break;
   case SVGA3D_LIGHTTYPE_SPOT2:
      _debug_printf("\t\t.data.type = SVGA3D_LIGHTTYPE_SPOT2\n");
      break;
   case SVGA3D_LIGHTTYPE_DIRECTIONAL:
      _debug_printf("\t\t.data.type = SVGA3D_LIGHTTYPE_DIRECTIONAL\n");
      break;
   case SVGA3D_LIGHTTYPE_MAX:
      _debug_printf("\t\t.data.type = SVGA3D_LIGHTTYPE_MAX\n");
      break;
   default:
      _debug_printf("\t\t.data.type = %i\n", (*cmd).data.type);
      break;
   }
   _debug_printf("\t\t.data.inWorldSpace = %u\n", (*cmd).data.inWorldSpace);
   _debug_printf("\t\t.data.diffuse[0] = %f\n", (*cmd).data.diffuse[0]);
   _debug_printf("\t\t.data.diffuse[1] = %f\n", (*cmd).data.diffuse[1]);
   _debug_printf("\t\t.data.diffuse[2] = %f\n", (*cmd).data.diffuse[2]);
   _debug_printf("\t\t.data.diffuse[3] = %f\n", (*cmd).data.diffuse[3]);
   _debug_printf("\t\t.data.specular[0] = %f\n", (*cmd).data.specular[0]);
   _debug_printf("\t\t.data.specular[1] = %f\n", (*cmd).data.specular[1]);
   _debug_printf("\t\t.data.specular[2] = %f\n", (*cmd).data.specular[2]);
   _debug_printf("\t\t.data.specular[3] = %f\n", (*cmd).data.specular[3]);
   _debug_printf("\t\t.data.ambient[0] = %f\n", (*cmd).data.ambient[0]);
   _debug_printf("\t\t.data.ambient[1] = %f\n", (*cmd).data.ambient[1]);
   _debug_printf("\t\t.data.ambient[2] = %f\n", (*cmd).data.ambient[2]);
   _debug_printf("\t\t.data.ambient[3] = %f\n", (*cmd).data.ambient[3]);
   _debug_printf("\t\t.data.position[0] = %f\n", (*cmd).data.position[0]);
   _debug_printf("\t\t.data.position[1] = %f\n", (*cmd).data.position[1]);
   _debug_printf("\t\t.data.position[2] = %f\n", (*cmd).data.position[2]);
   _debug_printf("\t\t.data.position[3] = %f\n", (*cmd).data.position[3]);
   _debug_printf("\t\t.data.direction[0] = %f\n", (*cmd).data.direction[0]);
   _debug_printf("\t\t.data.direction[1] = %f\n", (*cmd).data.direction[1]);
   _debug_printf("\t\t.data.direction[2] = %f\n", (*cmd).data.direction[2]);
   _debug_printf("\t\t.data.direction[3] = %f\n", (*cmd).data.direction[3]);
   _debug_printf("\t\t.data.range = %f\n", (*cmd).data.range);
   _debug_printf("\t\t.data.falloff = %f\n", (*cmd).data.falloff);
   _debug_printf("\t\t.data.attenuation0 = %f\n", (*cmd).data.attenuation0);
   _debug_printf("\t\t.data.attenuation1 = %f\n", (*cmd).data.attenuation1);
   _debug_printf("\t\t.data.attenuation2 = %f\n", (*cmd).data.attenuation2);
   _debug_printf("\t\t.data.theta = %f\n", (*cmd).data.theta);
   _debug_printf("\t\t.data.phi = %f\n", (*cmd).data.phi);
}

static void
dump_SVGA3dCmdSetViewport(const SVGA3dCmdSetViewport *cmd)
{
   _debug_printf("\t\t.cid = %u\n", (*cmd).cid);
   _debug_printf("\t\t.rect.x = %u\n", (*cmd).rect.x);
   _debug_printf("\t\t.rect.y = %u\n", (*cmd).rect.y);
   _debug_printf("\t\t.rect.w = %u\n", (*cmd).rect.w);
   _debug_printf("\t\t.rect.h = %u\n", (*cmd).rect.h);
}

static void
dump_SVGA3dCmdSetScissorRect(const SVGA3dCmdSetScissorRect *cmd)
{
   _debug_printf("\t\t.cid = %u\n", (*cmd).cid);
   _debug_printf("\t\t.rect.x = %u\n", (*cmd).rect.x);
   _debug_printf("\t\t.rect.y = %u\n", (*cmd).rect.y);
   _debug_printf("\t\t.rect.w = %u\n", (*cmd).rect.w);
   _debug_printf("\t\t.rect.h = %u\n", (*cmd).rect.h);
}

static void
dump_SVGA3dCopyRect(const SVGA3dCopyRect *cmd)
{
   _debug_printf("\t\t.x = %u\n", (*cmd).x);
   _debug_printf("\t\t.y = %u\n", (*cmd).y);
   _debug_printf("\t\t.w = %u\n", (*cmd).w);
   _debug_printf("\t\t.h = %u\n", (*cmd).h);
   _debug_printf("\t\t.srcx = %u\n", (*cmd).srcx);
   _debug_printf("\t\t.srcy = %u\n", (*cmd).srcy);
}

static void
dump_SVGA3dCmdSetShader(const SVGA3dCmdSetShader *cmd)
{
   _debug_printf("\t\t.cid = %u\n", (*cmd).cid);
   _debug_printf("\t\t.type = %s\n", shader_name(cmd->type));
   _debug_printf("\t\t.shid = %u\n", (*cmd).shid);
}

static void
dump_SVGA3dCmdEndQuery(const SVGA3dCmdEndQuery *cmd)
{
   _debug_printf("\t\t.cid = %u\n", (*cmd).cid);
   switch((*cmd).type) {
   case SVGA3D_QUERYTYPE_OCCLUSION:
      _debug_printf("\t\t.type = SVGA3D_QUERYTYPE_OCCLUSION\n");
      break;
   case SVGA3D_QUERYTYPE_MAX:
      _debug_printf("\t\t.type = SVGA3D_QUERYTYPE_MAX\n");
      break;
   default:
      _debug_printf("\t\t.type = %i\n", (*cmd).type);
      break;
   }
   _debug_printf("\t\t.guestResult.gmrId = %u\n", (*cmd).guestResult.gmrId);
   _debug_printf("\t\t.guestResult.offset = %u\n", (*cmd).guestResult.offset);
}

static void
dump_SVGA3dSize(const SVGA3dSize *cmd)
{
   _debug_printf("\t\t.width = %u\n", (*cmd).width);
   _debug_printf("\t\t.height = %u\n", (*cmd).height);
   _debug_printf("\t\t.depth = %u\n", (*cmd).depth);
}

static void
dump_SVGA3dCmdDestroySurface(const SVGA3dCmdDestroySurface *cmd)
{
   _debug_printf("\t\t.sid = %u\n", (*cmd).sid);
}

static void
dump_SVGA3dCmdDefineContext(const SVGA3dCmdDefineContext *cmd)
{
   _debug_printf("\t\t.cid = %u\n", (*cmd).cid);
}

static void
dump_SVGA3dRect(const SVGA3dRect *cmd)
{
   _debug_printf("\t\t.x = %u\n", (*cmd).x);
   _debug_printf("\t\t.y = %u\n", (*cmd).y);
   _debug_printf("\t\t.w = %u\n", (*cmd).w);
   _debug_printf("\t\t.h = %u\n", (*cmd).h);
}

static void
dump_SVGA3dCmdBeginQuery(const SVGA3dCmdBeginQuery *cmd)
{
   _debug_printf("\t\t.cid = %u\n", (*cmd).cid);
   switch((*cmd).type) {
   case SVGA3D_QUERYTYPE_OCCLUSION:
      _debug_printf("\t\t.type = SVGA3D_QUERYTYPE_OCCLUSION\n");
      break;
   case SVGA3D_QUERYTYPE_MAX:
      _debug_printf("\t\t.type = SVGA3D_QUERYTYPE_MAX\n");
      break;
   default:
      _debug_printf("\t\t.type = %i\n", (*cmd).type);
      break;
   }
}

static void
dump_SVGA3dRenderState(const SVGA3dRenderState *cmd)
{
   switch((*cmd).state) {
   case SVGA3D_RS_INVALID:
      _debug_printf("\t\t.state = SVGA3D_RS_INVALID\n");
      break;
   case SVGA3D_RS_ZENABLE:
      _debug_printf("\t\t.state = SVGA3D_RS_ZENABLE\n");
      break;
   case SVGA3D_RS_ZWRITEENABLE:
      _debug_printf("\t\t.state = SVGA3D_RS_ZWRITEENABLE\n");
      break;
   case SVGA3D_RS_ALPHATESTENABLE:
      _debug_printf("\t\t.state = SVGA3D_RS_ALPHATESTENABLE\n");
      break;
   case SVGA3D_RS_DITHERENABLE:
      _debug_printf("\t\t.state = SVGA3D_RS_DITHERENABLE\n");
      break;
   case SVGA3D_RS_BLENDENABLE:
      _debug_printf("\t\t.state = SVGA3D_RS_BLENDENABLE\n");
      break;
   case SVGA3D_RS_FOGENABLE:
      _debug_printf("\t\t.state = SVGA3D_RS_FOGENABLE\n");
      break;
   case SVGA3D_RS_SPECULARENABLE:
      _debug_printf("\t\t.state = SVGA3D_RS_SPECULARENABLE\n");
      break;
   case SVGA3D_RS_STENCILENABLE:
      _debug_printf("\t\t.state = SVGA3D_RS_STENCILENABLE\n");
      break;
   case SVGA3D_RS_LIGHTINGENABLE:
      _debug_printf("\t\t.state = SVGA3D_RS_LIGHTINGENABLE\n");
      break;
   case SVGA3D_RS_NORMALIZENORMALS:
      _debug_printf("\t\t.state = SVGA3D_RS_NORMALIZENORMALS\n");
      break;
   case SVGA3D_RS_POINTSPRITEENABLE:
      _debug_printf("\t\t.state = SVGA3D_RS_POINTSPRITEENABLE\n");
      break;
   case SVGA3D_RS_POINTSCALEENABLE:
      _debug_printf("\t\t.state = SVGA3D_RS_POINTSCALEENABLE\n");
      break;
   case SVGA3D_RS_STENCILREF:
      _debug_printf("\t\t.state = SVGA3D_RS_STENCILREF\n");
      break;
   case SVGA3D_RS_STENCILMASK:
      _debug_printf("\t\t.state = SVGA3D_RS_STENCILMASK\n");
      break;
   case SVGA3D_RS_STENCILWRITEMASK:
      _debug_printf("\t\t.state = SVGA3D_RS_STENCILWRITEMASK\n");
      break;
   case SVGA3D_RS_FOGSTART:
      _debug_printf("\t\t.state = SVGA3D_RS_FOGSTART\n");
      break;
   case SVGA3D_RS_FOGEND:
      _debug_printf("\t\t.state = SVGA3D_RS_FOGEND\n");
      break;
   case SVGA3D_RS_FOGDENSITY:
      _debug_printf("\t\t.state = SVGA3D_RS_FOGDENSITY\n");
      break;
   case SVGA3D_RS_POINTSIZE:
      _debug_printf("\t\t.state = SVGA3D_RS_POINTSIZE\n");
      break;
   case SVGA3D_RS_POINTSIZEMIN:
      _debug_printf("\t\t.state = SVGA3D_RS_POINTSIZEMIN\n");
      break;
   case SVGA3D_RS_POINTSIZEMAX:
      _debug_printf("\t\t.state = SVGA3D_RS_POINTSIZEMAX\n");
      break;
   case SVGA3D_RS_POINTSCALE_A:
      _debug_printf("\t\t.state = SVGA3D_RS_POINTSCALE_A\n");
      break;
   case SVGA3D_RS_POINTSCALE_B:
      _debug_printf("\t\t.state = SVGA3D_RS_POINTSCALE_B\n");
      break;
   case SVGA3D_RS_POINTSCALE_C:
      _debug_printf("\t\t.state = SVGA3D_RS_POINTSCALE_C\n");
      break;
   case SVGA3D_RS_FOGCOLOR:
      _debug_printf("\t\t.state = SVGA3D_RS_FOGCOLOR\n");
      break;
   case SVGA3D_RS_AMBIENT:
      _debug_printf("\t\t.state = SVGA3D_RS_AMBIENT\n");
      break;
   case SVGA3D_RS_CLIPPLANEENABLE:
      _debug_printf("\t\t.state = SVGA3D_RS_CLIPPLANEENABLE\n");
      break;
   case SVGA3D_RS_FOGMODE:
      _debug_printf("\t\t.state = SVGA3D_RS_FOGMODE\n");
      break;
   case SVGA3D_RS_FILLMODE:
      _debug_printf("\t\t.state = SVGA3D_RS_FILLMODE\n");
      break;
   case SVGA3D_RS_SHADEMODE:
      _debug_printf("\t\t.state = SVGA3D_RS_SHADEMODE\n");
      break;
   case SVGA3D_RS_LINEPATTERN:
      _debug_printf("\t\t.state = SVGA3D_RS_LINEPATTERN\n");
      break;
   case SVGA3D_RS_SRCBLEND:
      _debug_printf("\t\t.state = SVGA3D_RS_SRCBLEND\n");
      break;
   case SVGA3D_RS_DSTBLEND:
      _debug_printf("\t\t.state = SVGA3D_RS_DSTBLEND\n");
      break;
   case SVGA3D_RS_BLENDEQUATION:
      _debug_printf("\t\t.state = SVGA3D_RS_BLENDEQUATION\n");
      break;
   case SVGA3D_RS_CULLMODE:
      _debug_printf("\t\t.state = SVGA3D_RS_CULLMODE\n");
      break;
   case SVGA3D_RS_ZFUNC:
      _debug_printf("\t\t.state = SVGA3D_RS_ZFUNC\n");
      break;
   case SVGA3D_RS_ALPHAFUNC:
      _debug_printf("\t\t.state = SVGA3D_RS_ALPHAFUNC\n");
      break;
   case SVGA3D_RS_STENCILFUNC:
      _debug_printf("\t\t.state = SVGA3D_RS_STENCILFUNC\n");
      break;
   case SVGA3D_RS_STENCILFAIL:
      _debug_printf("\t\t.state = SVGA3D_RS_STENCILFAIL\n");
      break;
   case SVGA3D_RS_STENCILZFAIL:
      _debug_printf("\t\t.state = SVGA3D_RS_STENCILZFAIL\n");
      break;
   case SVGA3D_RS_STENCILPASS:
      _debug_printf("\t\t.state = SVGA3D_RS_STENCILPASS\n");
      break;
   case SVGA3D_RS_ALPHAREF:
      _debug_printf("\t\t.state = SVGA3D_RS_ALPHAREF\n");
      break;
   case SVGA3D_RS_FRONTWINDING:
      _debug_printf("\t\t.state = SVGA3D_RS_FRONTWINDING\n");
      break;
   case SVGA3D_RS_COORDINATETYPE:
      _debug_printf("\t\t.state = SVGA3D_RS_COORDINATETYPE\n");
      break;
   case SVGA3D_RS_ZBIAS:
      _debug_printf("\t\t.state = SVGA3D_RS_ZBIAS\n");
      break;
   case SVGA3D_RS_RANGEFOGENABLE:
      _debug_printf("\t\t.state = SVGA3D_RS_RANGEFOGENABLE\n");
      break;
   case SVGA3D_RS_COLORWRITEENABLE:
      _debug_printf("\t\t.state = SVGA3D_RS_COLORWRITEENABLE\n");
      break;
   case SVGA3D_RS_VERTEXMATERIALENABLE:
      _debug_printf("\t\t.state = SVGA3D_RS_VERTEXMATERIALENABLE\n");
      break;
   case SVGA3D_RS_DIFFUSEMATERIALSOURCE:
      _debug_printf("\t\t.state = SVGA3D_RS_DIFFUSEMATERIALSOURCE\n");
      break;
   case SVGA3D_RS_SPECULARMATERIALSOURCE:
      _debug_printf("\t\t.state = SVGA3D_RS_SPECULARMATERIALSOURCE\n");
      break;
   case SVGA3D_RS_AMBIENTMATERIALSOURCE:
      _debug_printf("\t\t.state = SVGA3D_RS_AMBIENTMATERIALSOURCE\n");
      break;
   case SVGA3D_RS_EMISSIVEMATERIALSOURCE:
      _debug_printf("\t\t.state = SVGA3D_RS_EMISSIVEMATERIALSOURCE\n");
      break;
   case SVGA3D_RS_TEXTUREFACTOR:
      _debug_printf("\t\t.state = SVGA3D_RS_TEXTUREFACTOR\n");
      break;
   case SVGA3D_RS_LOCALVIEWER:
      _debug_printf("\t\t.state = SVGA3D_RS_LOCALVIEWER\n");
      break;
   case SVGA3D_RS_SCISSORTESTENABLE:
      _debug_printf("\t\t.state = SVGA3D_RS_SCISSORTESTENABLE\n");
      break;
   case SVGA3D_RS_BLENDCOLOR:
      _debug_printf("\t\t.state = SVGA3D_RS_BLENDCOLOR\n");
      break;
   case SVGA3D_RS_STENCILENABLE2SIDED:
      _debug_printf("\t\t.state = SVGA3D_RS_STENCILENABLE2SIDED\n");
      break;
   case SVGA3D_RS_CCWSTENCILFUNC:
      _debug_printf("\t\t.state = SVGA3D_RS_CCWSTENCILFUNC\n");
      break;
   case SVGA3D_RS_CCWSTENCILFAIL:
      _debug_printf("\t\t.state = SVGA3D_RS_CCWSTENCILFAIL\n");
      break;
   case SVGA3D_RS_CCWSTENCILZFAIL:
      _debug_printf("\t\t.state = SVGA3D_RS_CCWSTENCILZFAIL\n");
      break;
   case SVGA3D_RS_CCWSTENCILPASS:
      _debug_printf("\t\t.state = SVGA3D_RS_CCWSTENCILPASS\n");
      break;
   case SVGA3D_RS_VERTEXBLEND:
      _debug_printf("\t\t.state = SVGA3D_RS_VERTEXBLEND\n");
      break;
   case SVGA3D_RS_SLOPESCALEDEPTHBIAS:
      _debug_printf("\t\t.state = SVGA3D_RS_SLOPESCALEDEPTHBIAS\n");
      break;
   case SVGA3D_RS_DEPTHBIAS:
      _debug_printf("\t\t.state = SVGA3D_RS_DEPTHBIAS\n");
      break;
   case SVGA3D_RS_OUTPUTGAMMA:
      _debug_printf("\t\t.state = SVGA3D_RS_OUTPUTGAMMA\n");
      break;
   case SVGA3D_RS_ZVISIBLE:
      _debug_printf("\t\t.state = SVGA3D_RS_ZVISIBLE\n");
      break;
   case SVGA3D_RS_LASTPIXEL:
      _debug_printf("\t\t.state = SVGA3D_RS_LASTPIXEL\n");
      break;
   case SVGA3D_RS_CLIPPING:
      _debug_printf("\t\t.state = SVGA3D_RS_CLIPPING\n");
      break;
   case SVGA3D_RS_WRAP0:
      _debug_printf("\t\t.state = SVGA3D_RS_WRAP0\n");
      break;
   case SVGA3D_RS_WRAP1:
      _debug_printf("\t\t.state = SVGA3D_RS_WRAP1\n");
      break;
   case SVGA3D_RS_WRAP2:
      _debug_printf("\t\t.state = SVGA3D_RS_WRAP2\n");
      break;
   case SVGA3D_RS_WRAP3:
      _debug_printf("\t\t.state = SVGA3D_RS_WRAP3\n");
      break;
   case SVGA3D_RS_WRAP4:
      _debug_printf("\t\t.state = SVGA3D_RS_WRAP4\n");
      break;
   case SVGA3D_RS_WRAP5:
      _debug_printf("\t\t.state = SVGA3D_RS_WRAP5\n");
      break;
   case SVGA3D_RS_WRAP6:
      _debug_printf("\t\t.state = SVGA3D_RS_WRAP6\n");
      break;
   case SVGA3D_RS_WRAP7:
      _debug_printf("\t\t.state = SVGA3D_RS_WRAP7\n");
      break;
   case SVGA3D_RS_WRAP8:
      _debug_printf("\t\t.state = SVGA3D_RS_WRAP8\n");
      break;
   case SVGA3D_RS_WRAP9:
      _debug_printf("\t\t.state = SVGA3D_RS_WRAP9\n");
      break;
   case SVGA3D_RS_WRAP10:
      _debug_printf("\t\t.state = SVGA3D_RS_WRAP10\n");
      break;
   case SVGA3D_RS_WRAP11:
      _debug_printf("\t\t.state = SVGA3D_RS_WRAP11\n");
      break;
   case SVGA3D_RS_WRAP12:
      _debug_printf("\t\t.state = SVGA3D_RS_WRAP12\n");
      break;
   case SVGA3D_RS_WRAP13:
      _debug_printf("\t\t.state = SVGA3D_RS_WRAP13\n");
      break;
   case SVGA3D_RS_WRAP14:
      _debug_printf("\t\t.state = SVGA3D_RS_WRAP14\n");
      break;
   case SVGA3D_RS_WRAP15:
      _debug_printf("\t\t.state = SVGA3D_RS_WRAP15\n");
      break;
   case SVGA3D_RS_MULTISAMPLEANTIALIAS:
      _debug_printf("\t\t.state = SVGA3D_RS_MULTISAMPLEANTIALIAS\n");
      break;
   case SVGA3D_RS_MULTISAMPLEMASK:
      _debug_printf("\t\t.state = SVGA3D_RS_MULTISAMPLEMASK\n");
      break;
   case SVGA3D_RS_INDEXEDVERTEXBLENDENABLE:
      _debug_printf("\t\t.state = SVGA3D_RS_INDEXEDVERTEXBLENDENABLE\n");
      break;
   case SVGA3D_RS_TWEENFACTOR:
      _debug_printf("\t\t.state = SVGA3D_RS_TWEENFACTOR\n");
      break;
   case SVGA3D_RS_ANTIALIASEDLINEENABLE:
      _debug_printf("\t\t.state = SVGA3D_RS_ANTIALIASEDLINEENABLE\n");
      break;
   case SVGA3D_RS_COLORWRITEENABLE1:
      _debug_printf("\t\t.state = SVGA3D_RS_COLORWRITEENABLE1\n");
      break;
   case SVGA3D_RS_COLORWRITEENABLE2:
      _debug_printf("\t\t.state = SVGA3D_RS_COLORWRITEENABLE2\n");
      break;
   case SVGA3D_RS_COLORWRITEENABLE3:
      _debug_printf("\t\t.state = SVGA3D_RS_COLORWRITEENABLE3\n");
      break;
   case SVGA3D_RS_SEPARATEALPHABLENDENABLE:
      _debug_printf("\t\t.state = SVGA3D_RS_SEPARATEALPHABLENDENABLE\n");
      break;
   case SVGA3D_RS_SRCBLENDALPHA:
      _debug_printf("\t\t.state = SVGA3D_RS_SRCBLENDALPHA\n");
      break;
   case SVGA3D_RS_DSTBLENDALPHA:
      _debug_printf("\t\t.state = SVGA3D_RS_DSTBLENDALPHA\n");
      break;
   case SVGA3D_RS_BLENDEQUATIONALPHA:
      _debug_printf("\t\t.state = SVGA3D_RS_BLENDEQUATIONALPHA\n");
      break;
   case SVGA3D_RS_MAX:
      _debug_printf("\t\t.state = SVGA3D_RS_MAX\n");
      break;
   default:
      _debug_printf("\t\t.state = %i\n", (*cmd).state);
      break;
   }
   _debug_printf("\t\t.uintValue = %u\n", (*cmd).uintValue);
   _debug_printf("\t\t.floatValue = %f\n", (*cmd).floatValue);
}

static void
dump_SVGA3dVertexDivisor(const SVGA3dVertexDivisor *cmd)
{
   _debug_printf("\t\t.value = %u\n", (*cmd).value);
   _debug_printf("\t\t.count = %u\n", (*cmd).count);
   _debug_printf("\t\t.indexedData = %u\n", (*cmd).indexedData);
   _debug_printf("\t\t.instanceData = %u\n", (*cmd).instanceData);
}

static void
dump_SVGA3dCmdDefineShader(const SVGA3dCmdDefineShader *cmd)
{
   _debug_printf("\t\t.cid = %u\n", (*cmd).cid);
   _debug_printf("\t\t.shid = %u\n", (*cmd).shid);
   switch((*cmd).type) {
   case SVGA3D_SHADERTYPE_VS:
      _debug_printf("\t\t.type = SVGA3D_SHADERTYPE_VS\n");
      break;
   case SVGA3D_SHADERTYPE_PS:
      _debug_printf("\t\t.type = SVGA3D_SHADERTYPE_PS\n");
      break;
   default:
      _debug_printf("\t\t.type = %i\n", (*cmd).type);
      break;
   }
}

static void
dump_constants(SVGA3dShaderConstType type, unsigned start,
               unsigned numConsts, const void *buf)
{
   unsigned i;
   const float (*fvalues)[4];
   const int32 (*ivalues)[4];

   switch (type) {
   case SVGA3D_CONST_TYPE_FLOAT:
      _debug_printf("\t\t.ctype = SVGA3D_CONST_TYPE_FLOAT\n");
      fvalues = (const float (*)[4]) buf;
      for (i = 0; i < numConsts; ++i) {
         _debug_printf("\t\t.values[%u] = {%f, %f, %f, %f}\n",
                       start + i, 
                       fvalues[i][0],
                       fvalues[i][1],
                       fvalues[i][2],
                       fvalues[i][3]);
      }
      break;
   case SVGA3D_CONST_TYPE_INT:
      _debug_printf("\t\t.ctype = SVGA3D_CONST_TYPE_INT\n");
      ivalues = (const int32 (*)[4]) buf;
      for (i = 0; i < numConsts; ++i) {
         _debug_printf("\t\t.values[%u] = {%i, %i, %i, %i}\n",
                       start + i,
                       ivalues[i][0],
                       ivalues[i][1],
                       ivalues[i][2],
                       ivalues[i][3]);
      }
      break;
   case SVGA3D_CONST_TYPE_BOOL:
      _debug_printf("\t\t.ctype = SVGA3D_CONST_TYPE_BOOL\n");
      ivalues = (const int32 (*)[4]) buf;
      for (i = 0; i < numConsts; ++i) {
         _debug_printf("\t\t.values[%u] = {%i, %i, %i, %i}\n",
                       start + i,
                       ivalues[i][0],
                       ivalues[i][1],
                       ivalues[i][2],
                       ivalues[i][3]);
      }
      break;
   default:
      _debug_printf("\t\t.ctype = %i\n", type);
      ivalues = (const int32 (*)[4]) buf;
      for (i = 0; i < numConsts; ++i) {
         _debug_printf("\t\t.values[%u] = {%i, %i, %i, %i}\n",
                       start + i,
                       ivalues[i][0],
                       ivalues[i][1],
                       ivalues[i][2],
                       ivalues[i][3]);
      }
      break;
   }
}

static void
dump_SVGA3dCmdSetShaderConst(const SVGA3dCmdSetShaderConst *cmd, uint32 numConsts)
{
   _debug_printf("\t\t.cid = %u\n", (*cmd).cid);
   _debug_printf("\t\t.reg = %u\n", (*cmd).reg);
   _debug_printf("\t\t.type = %s\n", shader_name((*cmd).type));
   dump_constants((*cmd).ctype, cmd->reg, numConsts, cmd->values);
}

static void
dump_SVGA3dCmdSetGBShaderConstInline(const SVGA3dCmdSetGBShaderConstInline *cmd, uint32 numConsts)
{
   _debug_printf("\t\t.cid = %u\n", (*cmd).cid);
   _debug_printf("\t\t.reg = %u\n", (*cmd).regStart);
   _debug_printf("\t\t.type = %s\n", shader_name((*cmd).shaderType));
   dump_constants((*cmd).constType, cmd->regStart, numConsts, &cmd[1]);
}


static void
dump_SVGA3dCmdSetZRange(const SVGA3dCmdSetZRange *cmd)
{
   _debug_printf("\t\t.cid = %u\n", (*cmd).cid);
   _debug_printf("\t\t.zRange.min = %f\n", (*cmd).zRange.min);
   _debug_printf("\t\t.zRange.max = %f\n", (*cmd).zRange.max);
}

static void
dump_SVGA3dCmdDrawPrimitives(const SVGA3dCmdDrawPrimitives *cmd)
{
   _debug_printf("\t\t.cid = %u\n", (*cmd).cid);
   _debug_printf("\t\t.numVertexDecls = %u\n", (*cmd).numVertexDecls);
   _debug_printf("\t\t.numRanges = %u\n", (*cmd).numRanges);
}

static void
dump_SVGA3dCmdSetLightEnabled(const SVGA3dCmdSetLightEnabled *cmd)
{
   _debug_printf("\t\t.cid = %u\n", (*cmd).cid);
   _debug_printf("\t\t.index = %u\n", (*cmd).index);
   _debug_printf("\t\t.enabled = %u\n", (*cmd).enabled);
}

static void
dump_SVGA3dPrimitiveRange(const SVGA3dPrimitiveRange *cmd)
{
   switch((*cmd).primType) {
   case SVGA3D_PRIMITIVE_INVALID:
      _debug_printf("\t\t.primType = SVGA3D_PRIMITIVE_INVALID\n");
      break;
   case SVGA3D_PRIMITIVE_TRIANGLELIST:
      _debug_printf("\t\t.primType = SVGA3D_PRIMITIVE_TRIANGLELIST\n");
      break;
   case SVGA3D_PRIMITIVE_POINTLIST:
      _debug_printf("\t\t.primType = SVGA3D_PRIMITIVE_POINTLIST\n");
      break;
   case SVGA3D_PRIMITIVE_LINELIST:
      _debug_printf("\t\t.primType = SVGA3D_PRIMITIVE_LINELIST\n");
      break;
   case SVGA3D_PRIMITIVE_LINESTRIP:
      _debug_printf("\t\t.primType = SVGA3D_PRIMITIVE_LINESTRIP\n");
      break;
   case SVGA3D_PRIMITIVE_TRIANGLESTRIP:
      _debug_printf("\t\t.primType = SVGA3D_PRIMITIVE_TRIANGLESTRIP\n");
      break;
   case SVGA3D_PRIMITIVE_TRIANGLEFAN:
      _debug_printf("\t\t.primType = SVGA3D_PRIMITIVE_TRIANGLEFAN\n");
      break;
   case SVGA3D_PRIMITIVE_MAX:
      _debug_printf("\t\t.primType = SVGA3D_PRIMITIVE_MAX\n");
      break;
   default:
      _debug_printf("\t\t.primType = %i\n", (*cmd).primType);
      break;
   }
   _debug_printf("\t\t.primitiveCount = %u\n", (*cmd).primitiveCount);
   _debug_printf("\t\t.indexArray.surfaceId = %u\n", (*cmd).indexArray.surfaceId);
   _debug_printf("\t\t.indexArray.offset = %u\n", (*cmd).indexArray.offset);
   _debug_printf("\t\t.indexArray.stride = %u\n", (*cmd).indexArray.stride);
   _debug_printf("\t\t.indexWidth = %u\n", (*cmd).indexWidth);
   _debug_printf("\t\t.indexBias = %i\n", (*cmd).indexBias);
}

static void
dump_SVGA3dCmdPresent(const SVGA3dCmdPresent *cmd)
{
   _debug_printf("\t\t.sid = %u\n", (*cmd).sid);
}

static void
dump_SVGA3dCmdSetRenderState(const SVGA3dCmdSetRenderState *cmd)
{
   _debug_printf("\t\t.cid = %u\n", (*cmd).cid);
}

static void
dump_SVGA3dCmdSurfaceStretchBlt(const SVGA3dCmdSurfaceStretchBlt *cmd)
{
   _debug_printf("\t\t.src.sid = %u\n", (*cmd).src.sid);
   _debug_printf("\t\t.src.face = %u\n", (*cmd).src.face);
   _debug_printf("\t\t.src.mipmap = %u\n", (*cmd).src.mipmap);
   _debug_printf("\t\t.dest.sid = %u\n", (*cmd).dest.sid);
   _debug_printf("\t\t.dest.face = %u\n", (*cmd).dest.face);
   _debug_printf("\t\t.dest.mipmap = %u\n", (*cmd).dest.mipmap);
   _debug_printf("\t\t.boxSrc.x = %u\n", (*cmd).boxSrc.x);
   _debug_printf("\t\t.boxSrc.y = %u\n", (*cmd).boxSrc.y);
   _debug_printf("\t\t.boxSrc.z = %u\n", (*cmd).boxSrc.z);
   _debug_printf("\t\t.boxSrc.w = %u\n", (*cmd).boxSrc.w);
   _debug_printf("\t\t.boxSrc.h = %u\n", (*cmd).boxSrc.h);
   _debug_printf("\t\t.boxSrc.d = %u\n", (*cmd).boxSrc.d);
   _debug_printf("\t\t.boxDest.x = %u\n", (*cmd).boxDest.x);
   _debug_printf("\t\t.boxDest.y = %u\n", (*cmd).boxDest.y);
   _debug_printf("\t\t.boxDest.z = %u\n", (*cmd).boxDest.z);
   _debug_printf("\t\t.boxDest.w = %u\n", (*cmd).boxDest.w);
   _debug_printf("\t\t.boxDest.h = %u\n", (*cmd).boxDest.h);
   _debug_printf("\t\t.boxDest.d = %u\n", (*cmd).boxDest.d);
   switch((*cmd).mode) {
   case SVGA3D_STRETCH_BLT_POINT:
      _debug_printf("\t\t.mode = SVGA3D_STRETCH_BLT_POINT\n");
      break;
   case SVGA3D_STRETCH_BLT_LINEAR:
      _debug_printf("\t\t.mode = SVGA3D_STRETCH_BLT_LINEAR\n");
      break;
   case SVGA3D_STRETCH_BLT_MAX:
      _debug_printf("\t\t.mode = SVGA3D_STRETCH_BLT_MAX\n");
      break;
   default:
      _debug_printf("\t\t.mode = %i\n", (*cmd).mode);
      break;
   }
}

static void
dump_SVGA3dCmdSurfaceDMA(const SVGA3dCmdSurfaceDMA *cmd)
{
   _debug_printf("\t\t.guest.ptr.gmrId = %u\n", (*cmd).guest.ptr.gmrId);
   _debug_printf("\t\t.guest.ptr.offset = %u\n", (*cmd).guest.ptr.offset);
   _debug_printf("\t\t.guest.pitch = %u\n", (*cmd).guest.pitch);
   _debug_printf("\t\t.host.sid = %u\n", (*cmd).host.sid);
   _debug_printf("\t\t.host.face = %u\n", (*cmd).host.face);
   _debug_printf("\t\t.host.mipmap = %u\n", (*cmd).host.mipmap);
   switch((*cmd).transfer) {
   case SVGA3D_WRITE_HOST_VRAM:
      _debug_printf("\t\t.transfer = SVGA3D_WRITE_HOST_VRAM\n");
      break;
   case SVGA3D_READ_HOST_VRAM:
      _debug_printf("\t\t.transfer = SVGA3D_READ_HOST_VRAM\n");
      break;
   default:
      _debug_printf("\t\t.transfer = %i\n", (*cmd).transfer);
      break;
   }
}

static void
dump_SVGA3dCmdSurfaceDMASuffix(const SVGA3dCmdSurfaceDMASuffix *cmd)
{
   _debug_printf("\t\t.suffixSize = %u\n", (*cmd).suffixSize);
   _debug_printf("\t\t.maximumOffset = %u\n", (*cmd).maximumOffset);
   _debug_printf("\t\t.flags.discard = %u\n", (*cmd).flags.discard);
   _debug_printf("\t\t.flags.unsynchronized = %u\n", (*cmd).flags.unsynchronized);
}

static void
dump_SVGA3dCmdSetTransform(const SVGA3dCmdSetTransform *cmd)
{
   _debug_printf("\t\t.cid = %u\n", (*cmd).cid);
   switch((*cmd).type) {
   case SVGA3D_TRANSFORM_INVALID:
      _debug_printf("\t\t.type = SVGA3D_TRANSFORM_INVALID\n");
      break;
   case SVGA3D_TRANSFORM_WORLD:
      _debug_printf("\t\t.type = SVGA3D_TRANSFORM_WORLD\n");
      break;
   case SVGA3D_TRANSFORM_VIEW:
      _debug_printf("\t\t.type = SVGA3D_TRANSFORM_VIEW\n");
      break;
   case SVGA3D_TRANSFORM_PROJECTION:
      _debug_printf("\t\t.type = SVGA3D_TRANSFORM_PROJECTION\n");
      break;
   case SVGA3D_TRANSFORM_TEXTURE0:
      _debug_printf("\t\t.type = SVGA3D_TRANSFORM_TEXTURE0\n");
      break;
   case SVGA3D_TRANSFORM_TEXTURE1:
      _debug_printf("\t\t.type = SVGA3D_TRANSFORM_TEXTURE1\n");
      break;
   case SVGA3D_TRANSFORM_TEXTURE2:
      _debug_printf("\t\t.type = SVGA3D_TRANSFORM_TEXTURE2\n");
      break;
   case SVGA3D_TRANSFORM_TEXTURE3:
      _debug_printf("\t\t.type = SVGA3D_TRANSFORM_TEXTURE3\n");
      break;
   case SVGA3D_TRANSFORM_TEXTURE4:
      _debug_printf("\t\t.type = SVGA3D_TRANSFORM_TEXTURE4\n");
      break;
   case SVGA3D_TRANSFORM_TEXTURE5:
      _debug_printf("\t\t.type = SVGA3D_TRANSFORM_TEXTURE5\n");
      break;
   case SVGA3D_TRANSFORM_TEXTURE6:
      _debug_printf("\t\t.type = SVGA3D_TRANSFORM_TEXTURE6\n");
      break;
   case SVGA3D_TRANSFORM_TEXTURE7:
      _debug_printf("\t\t.type = SVGA3D_TRANSFORM_TEXTURE7\n");
      break;
   case SVGA3D_TRANSFORM_WORLD1:
      _debug_printf("\t\t.type = SVGA3D_TRANSFORM_WORLD1\n");
      break;
   case SVGA3D_TRANSFORM_WORLD2:
      _debug_printf("\t\t.type = SVGA3D_TRANSFORM_WORLD2\n");
      break;
   case SVGA3D_TRANSFORM_WORLD3:
      _debug_printf("\t\t.type = SVGA3D_TRANSFORM_WORLD3\n");
      break;
   case SVGA3D_TRANSFORM_MAX:
      _debug_printf("\t\t.type = SVGA3D_TRANSFORM_MAX\n");
      break;
   default:
      _debug_printf("\t\t.type = %i\n", (*cmd).type);
      break;
   }
   _debug_printf("\t\t.matrix[0] = %f\n", (*cmd).matrix[0]);
   _debug_printf("\t\t.matrix[1] = %f\n", (*cmd).matrix[1]);
   _debug_printf("\t\t.matrix[2] = %f\n", (*cmd).matrix[2]);
   _debug_printf("\t\t.matrix[3] = %f\n", (*cmd).matrix[3]);
   _debug_printf("\t\t.matrix[4] = %f\n", (*cmd).matrix[4]);
   _debug_printf("\t\t.matrix[5] = %f\n", (*cmd).matrix[5]);
   _debug_printf("\t\t.matrix[6] = %f\n", (*cmd).matrix[6]);
   _debug_printf("\t\t.matrix[7] = %f\n", (*cmd).matrix[7]);
   _debug_printf("\t\t.matrix[8] = %f\n", (*cmd).matrix[8]);
   _debug_printf("\t\t.matrix[9] = %f\n", (*cmd).matrix[9]);
   _debug_printf("\t\t.matrix[10] = %f\n", (*cmd).matrix[10]);
   _debug_printf("\t\t.matrix[11] = %f\n", (*cmd).matrix[11]);
   _debug_printf("\t\t.matrix[12] = %f\n", (*cmd).matrix[12]);
   _debug_printf("\t\t.matrix[13] = %f\n", (*cmd).matrix[13]);
   _debug_printf("\t\t.matrix[14] = %f\n", (*cmd).matrix[14]);
   _debug_printf("\t\t.matrix[15] = %f\n", (*cmd).matrix[15]);
}

static void
dump_SVGA3dCmdDestroyShader(const SVGA3dCmdDestroyShader *cmd)
{
   _debug_printf("\t\t.cid = %u\n", (*cmd).cid);
   _debug_printf("\t\t.shid = %u\n", (*cmd).shid);
   switch((*cmd).type) {
   case SVGA3D_SHADERTYPE_VS:
      _debug_printf("\t\t.type = SVGA3D_SHADERTYPE_VS\n");
      break;
   case SVGA3D_SHADERTYPE_PS:
      _debug_printf("\t\t.type = SVGA3D_SHADERTYPE_PS\n");
      break;
   default:
      _debug_printf("\t\t.type = %i\n", (*cmd).type);
      break;
   }
}

static void
dump_SVGA3dCmdDestroyContext(const SVGA3dCmdDestroyContext *cmd)
{
   _debug_printf("\t\t.cid = %u\n", (*cmd).cid);
}

static void
dump_SVGA3dCmdClear(const SVGA3dCmdClear *cmd)
{
   _debug_printf("\t\t.cid = %u\n", (*cmd).cid);
   switch((*cmd).clearFlag) {
   case SVGA3D_CLEAR_COLOR:
      _debug_printf("\t\t.clearFlag = SVGA3D_CLEAR_COLOR\n");
      break;
   case SVGA3D_CLEAR_DEPTH:
      _debug_printf("\t\t.clearFlag = SVGA3D_CLEAR_DEPTH\n");
      break;
   case SVGA3D_CLEAR_STENCIL:
      _debug_printf("\t\t.clearFlag = SVGA3D_CLEAR_STENCIL\n");
      break;
   default:
      _debug_printf("\t\t.clearFlag = %i\n", (*cmd).clearFlag);
      break;
   }
   _debug_printf("\t\t.color = %u\n", (*cmd).color);
   _debug_printf("\t\t.depth = %f\n", (*cmd).depth);
   _debug_printf("\t\t.stencil = %u\n", (*cmd).stencil);
}

static void
dump_SVGA3dCmdDefineSurface(const SVGA3dCmdDefineSurface *cmd)
{
   _debug_printf("\t\t.sid = %u\n", (*cmd).sid);
   switch((*cmd).surfaceFlags) {
   case SVGA3D_SURFACE_CUBEMAP:
      _debug_printf("\t\t.surfaceFlags = SVGA3D_SURFACE_CUBEMAP\n");
      break;
   case SVGA3D_SURFACE_HINT_STATIC:
      _debug_printf("\t\t.surfaceFlags = SVGA3D_SURFACE_HINT_STATIC\n");
      break;
   case SVGA3D_SURFACE_HINT_DYNAMIC:
      _debug_printf("\t\t.surfaceFlags = SVGA3D_SURFACE_HINT_DYNAMIC\n");
      break;
   case SVGA3D_SURFACE_HINT_INDEXBUFFER:
      _debug_printf("\t\t.surfaceFlags = SVGA3D_SURFACE_HINT_INDEXBUFFER\n");
      break;
   case SVGA3D_SURFACE_HINT_VERTEXBUFFER:
      _debug_printf("\t\t.surfaceFlags = SVGA3D_SURFACE_HINT_VERTEXBUFFER\n");
      break;
   default:
      _debug_printf("\t\t.surfaceFlags = %i\n", (*cmd).surfaceFlags);
      break;
   }
   _debug_printf("\t\t.format = %s\n", svga_format_name((*cmd).format));
   _debug_printf("\t\t.face[0].numMipLevels = %u\n", (*cmd).face[0].numMipLevels);
   _debug_printf("\t\t.face[1].numMipLevels = %u\n", (*cmd).face[1].numMipLevels);
   _debug_printf("\t\t.face[2].numMipLevels = %u\n", (*cmd).face[2].numMipLevels);
   _debug_printf("\t\t.face[3].numMipLevels = %u\n", (*cmd).face[3].numMipLevels);
   _debug_printf("\t\t.face[4].numMipLevels = %u\n", (*cmd).face[4].numMipLevels);
   _debug_printf("\t\t.face[5].numMipLevels = %u\n", (*cmd).face[5].numMipLevels);
}

static void
dump_SVGASignedRect(const SVGASignedRect *cmd)
{
   _debug_printf("\t\t.left = %i\n", (*cmd).left);
   _debug_printf("\t\t.top = %i\n", (*cmd).top);
   _debug_printf("\t\t.right = %i\n", (*cmd).right);
   _debug_printf("\t\t.bottom = %i\n", (*cmd).bottom);
}

static void
dump_SVGA3dBox(const SVGA3dBox *box)
{
   _debug_printf("\t\t.box = %u, %u, %u  %u x %u x %u\n",
                 box->x, box->y, box->z, 
                 box->w, box->h, box->d);
}

static void
dump_SVGA3dCmdBlitSurfaceToScreen(const SVGA3dCmdBlitSurfaceToScreen *cmd)
{
   _debug_printf("\t\t.srcImage.sid = %u\n", (*cmd).srcImage.sid);
   _debug_printf("\t\t.srcImage.face = %u\n", (*cmd).srcImage.face);
   _debug_printf("\t\t.srcImage.mipmap = %u\n", (*cmd).srcImage.mipmap);
   _debug_printf("\t\t.srcRect.left = %i\n", (*cmd).srcRect.left);
   _debug_printf("\t\t.srcRect.top = %i\n", (*cmd).srcRect.top);
   _debug_printf("\t\t.srcRect.right = %i\n", (*cmd).srcRect.right);
   _debug_printf("\t\t.srcRect.bottom = %i\n", (*cmd).srcRect.bottom);
   _debug_printf("\t\t.destScreenId = %u\n", (*cmd).destScreenId);
   _debug_printf("\t\t.destRect.left = %i\n", (*cmd).destRect.left);
   _debug_printf("\t\t.destRect.top = %i\n", (*cmd).destRect.top);
   _debug_printf("\t\t.destRect.right = %i\n", (*cmd).destRect.right);
   _debug_printf("\t\t.destRect.bottom = %i\n", (*cmd).destRect.bottom);
}

static void
dump_SVGA3dCmdDefineGBContext(const SVGA3dCmdDefineGBContext *cmd)
{
   _debug_printf("\t\t.cid = %u\n", cmd->cid);
}
         
static void
dump_SVGA3dCmdBindGBContext(const SVGA3dCmdBindGBContext *cmd)
{
   _debug_printf("\t\t.cid = %u\n", cmd->cid);
   _debug_printf("\t\t.mobid = %u\n", cmd->mobid);
   _debug_printf("\t\t.validContents = %u\n", cmd->validContents);
}
         
static void
dump_SVGA3dCmdDestroyGBContext(const SVGA3dCmdDestroyGBContext *cmd)
{
   _debug_printf("\t\t.cid = %u\n", cmd->cid);
}

static void
dump_SVGA3dCmdDefineGBShader(const SVGA3dCmdDefineGBShader *cmd)
{
   _debug_printf("\t\t.shid = %u\n", cmd->shid);
   _debug_printf("\t\t.type = %s\n", shader_name(cmd->type));
   _debug_printf("\t\t.sizeInBytes = %u\n", cmd->sizeInBytes);
}

static void
dump_SVGA3dCmdBindGBShader(const SVGA3dCmdBindGBShader *cmd)
{
   _debug_printf("\t\t.shid = %u\n", cmd->shid);
   _debug_printf("\t\t.mobid = %u\n", cmd->mobid);
   _debug_printf("\t\t.offsetInBytes = %u\n", cmd->offsetInBytes);
}

static void
dump_SVGA3dCmdDestroyGBShader(const SVGA3dCmdDestroyGBShader *cmd)
{
   _debug_printf("\t\t.shid = %u\n", cmd->shid);
}

static void
dump_SVGA3dCmdBindGBSurface(const SVGA3dCmdBindGBSurface *cmd)
{
   _debug_printf("\t\t.sid = %u\n", cmd->sid);
   _debug_printf("\t\t.mobid = %u\n", cmd->mobid);
}

static void
dump_SVGA3dCmdUpdateGBSurface(const SVGA3dCmdUpdateGBSurface *cmd)
{
   _debug_printf("\t\t.sid = %u\n", cmd->sid);
}

static void
dump_SVGA3dCmdUpdateGBImage(const SVGA3dCmdUpdateGBImage *cmd)
{
   _debug_printf("\t\t.image.sid = %u\n", cmd->image.sid);
   _debug_printf("\t\t.image.face = %u\n", cmd->image.face);
   _debug_printf("\t\t.image.mipmap = %u\n", cmd->image.mipmap);
   dump_SVGA3dBox(&cmd->box);
}

static void
dump_SVGA3dCmdReadbackGBImage(const SVGA3dCmdReadbackGBImage *cmd)
{
   _debug_printf("\t\t.image.sid = %u\n", cmd->image.sid);
   _debug_printf("\t\t.image.face = %u\n", cmd->image.face);
   _debug_printf("\t\t.image.mipmap = %u\n", cmd->image.mipmap);
}

static void
dump_SVGA3dCmdInvalidateGBImage(const SVGA3dCmdInvalidateGBImage *cmd)
{
   _debug_printf("\t\t.image.sid = %u\n", cmd->image.sid);
   _debug_printf("\t\t.image.face = %u\n", cmd->image.face);
   _debug_printf("\t\t.image.mipmap = %u\n", cmd->image.mipmap);
}

static void
dump_SVGA3dCmdInvalidateGBImagePartial(const SVGA3dCmdInvalidateGBImagePartial *cmd)
{
   _debug_printf("\t\t.image.sid = %u\n", cmd->image.sid);
   _debug_printf("\t\t.image.face = %u\n", cmd->image.face);
   _debug_printf("\t\t.image.mipmap = %u\n", cmd->image.mipmap);
   dump_SVGA3dBox(&cmd->box);
   _debug_printf("\t\t.invertBox = %u\n", cmd->invertBox);
}

/// SVGA_3D_CMD_DX

#define __SVGA3D_DUMP_STRINGIFY(a) #a
#define SVGA3D_DUMP_STRINGIFY(a) __SVGA3D_DUMP_STRINGIFY(a)

#define SVGA3D_DUMP_HEADER(CommandName) \
static void \
dump_SVGA3dCmdDX##CommandName(const SVGA3dCmdDX##CommandName *cmd)

#define SVGA3D_DUMP_PARAMETER(ParameterName, ParameterType) \
_debug_printf(SVGA3D_DUMP_STRINGIFY(\t\t.ParameterName = %ParameterType\n), cmd->ParameterName)

#define SVGA3D_DUMP_TYPE_CASE(TypeVariableName, CaseName) \
case CaseName: \
  _debug_printf(SVGA3D_DUMP_STRINGIFY(\t\t.TypeVariableName = CaseName) "\n"); \
   break;

#define SVGA3D_DUMP_TYPE_DEFAULT(TypeVariableName) \
default: \
   _debug_printf(SVGA3D_DUMP_STRINGIFY(\t\t.TypeVariableName = %i\n), (*cmd).TypeVariableName); \
   break;

SVGA3D_DUMP_HEADER(SetShader)
{
   SVGA3D_DUMP_PARAMETER(shaderId, u);
   debug_printf("\t\t.type = %s\n", shader_name(cmd->type));
}

SVGA3D_DUMP_HEADER(SetSamplers)
{
   SVGA3D_DUMP_PARAMETER(startSampler, u);
   debug_printf("\t\t.type = %s\n", shader_name(cmd->type));
   /* XXX: note we're not printing the sampler IDs at this time */
}

SVGA3D_DUMP_HEADER(Draw)
{
   SVGA3D_DUMP_PARAMETER(vertexCount, u);
   SVGA3D_DUMP_PARAMETER(startVertexLocation, u);
}

SVGA3D_DUMP_HEADER(DrawIndexed)
{
   SVGA3D_DUMP_PARAMETER(indexCount, u);
   SVGA3D_DUMP_PARAMETER(startIndexLocation, u);
   SVGA3D_DUMP_PARAMETER(baseVertexLocation, i);
}

SVGA3D_DUMP_HEADER(DrawInstanced)
{
   SVGA3D_DUMP_PARAMETER(vertexCountPerInstance, u);
   SVGA3D_DUMP_PARAMETER(instanceCount, u);
   SVGA3D_DUMP_PARAMETER(startVertexLocation, u);
   SVGA3D_DUMP_PARAMETER(startInstanceLocation, u);
}

SVGA3D_DUMP_HEADER(DrawIndexedInstanced)
{
   SVGA3D_DUMP_PARAMETER(indexCountPerInstance, u);
   SVGA3D_DUMP_PARAMETER(instanceCount, u);
   SVGA3D_DUMP_PARAMETER(startIndexLocation, u);
   SVGA3D_DUMP_PARAMETER(baseVertexLocation, i);
   SVGA3D_DUMP_PARAMETER(startInstanceLocation, u);
}

SVGA3D_DUMP_HEADER(DrawAuto)
{
}

SVGA3D_DUMP_HEADER(SetBlendState)
{
   SVGA3D_DUMP_PARAMETER(blendId, u);
   _debug_printf("\t\t.blendFactor[4] = %f %f %f %f\n", cmd->blendFactor[0],
                                                        cmd->blendFactor[1],
                                                        cmd->blendFactor[2],
                                                        cmd->blendFactor[3]);
  SVGA3D_DUMP_PARAMETER(sampleMask, u);
}

SVGA3D_DUMP_HEADER(SetDepthStencilState)
{
   SVGA3D_DUMP_PARAMETER(depthStencilId, u);
   SVGA3D_DUMP_PARAMETER(stencilRef, u);
}

SVGA3D_DUMP_HEADER(SetRasterizerState)
{
   SVGA3D_DUMP_PARAMETER(rasterizerId, u);
}

SVGA3D_DUMP_HEADER(DefineQuery)
{
   SVGA3D_DUMP_PARAMETER(queryId, u);
   switch (cmd->type)
   {
      SVGA3D_DUMP_TYPE_CASE(type, SVGA3D_QUERYTYPE_OCCLUSION);
      SVGA3D_DUMP_TYPE_CASE(type, SVGA3D_QUERYTYPE_MAX);
      SVGA3D_DUMP_TYPE_DEFAULT(type);
   }
   switch (cmd->flags)
   {
      SVGA3D_DUMP_TYPE_CASE(flags, SVGA3D_DXQUERY_FLAG_PREDICATEHINT);
      SVGA3D_DUMP_TYPE_DEFAULT(flags);
   }
}

SVGA3D_DUMP_HEADER(DestroyQuery)
{
   SVGA3D_DUMP_PARAMETER(queryId, u);
}

SVGA3D_DUMP_HEADER(BindAllQuery)
{
   SVGA3D_DUMP_PARAMETER(cid, u);
   SVGA3D_DUMP_PARAMETER(mobid, u);
}

SVGA3D_DUMP_HEADER(BindQuery)
{
   SVGA3D_DUMP_PARAMETER(queryId, u);
   SVGA3D_DUMP_PARAMETER(mobid, u);
}

SVGA3D_DUMP_HEADER(MoveQuery)
{
   SVGA3D_DUMP_PARAMETER(queryId, u);
   SVGA3D_DUMP_PARAMETER(mobid, u);
   SVGA3D_DUMP_PARAMETER(mobOffset, u);
}

SVGA3D_DUMP_HEADER(ReadbackAllQuery)
{
   SVGA3D_DUMP_PARAMETER(cid, u);
}

SVGA3D_DUMP_HEADER(SetQueryOffset)
{
   SVGA3D_DUMP_PARAMETER(queryId, u);
   SVGA3D_DUMP_PARAMETER(mobOffset, u);
}

SVGA3D_DUMP_HEADER(BeginQuery)
{
   SVGA3D_DUMP_PARAMETER(queryId, u);
}

SVGA3D_DUMP_HEADER(EndQuery)
{
   SVGA3D_DUMP_PARAMETER(queryId, u);
}

SVGA3D_DUMP_HEADER(SetPredication)
{
   SVGA3D_DUMP_PARAMETER(queryId, u);
   SVGA3D_DUMP_PARAMETER(predicateValue, u);
}

SVGA3D_DUMP_HEADER(SetSOTargets)
{
}


SVGA3D_DUMP_HEADER(BindContext)
{
   SVGA3D_DUMP_PARAMETER(mobid, u);
   SVGA3D_DUMP_PARAMETER(validContents, u);
}

SVGA3D_DUMP_HEADER(SetViewports)
{

   /* XXX: note we're not printing the SVGA3dViewport list at this time */
}

SVGA3D_DUMP_HEADER(SetScissorRects)
{
 
   /* XXX: note we're not printing the SVGASignedRect list at this time */
}

SVGA3D_DUMP_HEADER(ClearRenderTargetView)
{
   SVGA3D_DUMP_PARAMETER(renderTargetViewId, u);
   SVGA3D_DUMP_PARAMETER(rgba.r, f);
   SVGA3D_DUMP_PARAMETER(rgba.g, f);
   SVGA3D_DUMP_PARAMETER(rgba.b, f);
   SVGA3D_DUMP_PARAMETER(rgba.a, f);
}

SVGA3D_DUMP_HEADER(ClearDepthStencilView)
{
   SVGA3D_DUMP_PARAMETER(flags, u);
   SVGA3D_DUMP_PARAMETER(stencil, u);
   SVGA3D_DUMP_PARAMETER(depthStencilViewId, u);
   SVGA3D_DUMP_PARAMETER(depth, f);
}

SVGA3D_DUMP_HEADER(DefineShaderResourceView)
{
   SVGA3D_DUMP_PARAMETER(shaderResourceViewId, u);
   SVGA3D_DUMP_PARAMETER(sid, u);
   _debug_printf("\t\t.format = %s\n", svga_format_name(cmd->format));
   switch (cmd->resourceDimension)
   {
      SVGA3D_DUMP_TYPE_CASE(resourceDimension, SVGA3D_RESOURCE_BUFFER);
      SVGA3D_DUMP_TYPE_CASE(resourceDimension, SVGA3D_RESOURCE_TEXTURE1D);
      SVGA3D_DUMP_TYPE_CASE(resourceDimension, SVGA3D_RESOURCE_TEXTURE2D);
      SVGA3D_DUMP_TYPE_CASE(resourceDimension, SVGA3D_RESOURCE_TEXTURE3D);
      SVGA3D_DUMP_TYPE_CASE(resourceDimension, SVGA3D_RESOURCE_TEXTURECUBE);
      SVGA3D_DUMP_TYPE_CASE(resourceDimension, SVGA3D_RESOURCE_TYPE_MAX);
      SVGA3D_DUMP_TYPE_DEFAULT(resourceDimension);
   }
   if (cmd->resourceDimension == SVGA3D_RESOURCE_BUFFER) {
      SVGA3D_DUMP_PARAMETER(desc.buffer.firstElement, u);
      SVGA3D_DUMP_PARAMETER(desc.buffer.numElements, u);
   } 
   else {
      SVGA3D_DUMP_PARAMETER(desc.tex.mostDetailedMip, u);
      SVGA3D_DUMP_PARAMETER(desc.tex.firstArraySlice, u);
      SVGA3D_DUMP_PARAMETER(desc.tex.mipLevels, u);
      SVGA3D_DUMP_PARAMETER(desc.tex.arraySize, u);
   }
}

SVGA3D_DUMP_HEADER(SetShaderResources)
{
   SVGA3D_DUMP_PARAMETER(startView, u);
   debug_printf("\t\t.type = %s\n", shader_name(cmd->type));
}


SVGA3D_DUMP_HEADER(DestroyShaderResourceView)
{
   SVGA3D_DUMP_PARAMETER(shaderResourceViewId, u);
}

SVGA3D_DUMP_HEADER(DefineRenderTargetView)
{
   SVGA3D_DUMP_PARAMETER(renderTargetViewId, u);
   SVGA3D_DUMP_PARAMETER(sid, u);
   _debug_printf("\t\t.format = %s\n", svga_format_name(cmd->format));
   switch (cmd->resourceDimension)
   {
      SVGA3D_DUMP_TYPE_CASE(resourceDimension, SVGA3D_RESOURCE_BUFFER);
      SVGA3D_DUMP_TYPE_CASE(resourceDimension, SVGA3D_RESOURCE_TEXTURE1D);
      SVGA3D_DUMP_TYPE_CASE(resourceDimension, SVGA3D_RESOURCE_TEXTURE2D);
      SVGA3D_DUMP_TYPE_CASE(resourceDimension, SVGA3D_RESOURCE_TEXTURE3D);
      SVGA3D_DUMP_TYPE_CASE(resourceDimension, SVGA3D_RESOURCE_TEXTURECUBE);
      SVGA3D_DUMP_TYPE_CASE(resourceDimension, SVGA3D_RESOURCE_TYPE_MAX);
      SVGA3D_DUMP_TYPE_DEFAULT(resourceDimension);
   }
   SVGA3D_DUMP_PARAMETER(desc.buffer.firstElement, u);
   SVGA3D_DUMP_PARAMETER(desc.buffer.numElements, u);
   SVGA3D_DUMP_PARAMETER(desc.tex.mipSlice, u);
   SVGA3D_DUMP_PARAMETER(desc.tex.firstArraySlice, u);
   SVGA3D_DUMP_PARAMETER(desc.tex.arraySize, u);
   SVGA3D_DUMP_PARAMETER(desc.tex3D.mipSlice, u);
   SVGA3D_DUMP_PARAMETER(desc.tex3D.firstW, u);
   SVGA3D_DUMP_PARAMETER(desc.tex3D.wSize, u);
}

SVGA3D_DUMP_HEADER(DestroyRenderTargetView)
{
   SVGA3D_DUMP_PARAMETER(renderTargetViewId, u);
}

SVGA3D_DUMP_HEADER(DefineDepthStencilView)
{
   SVGA3D_DUMP_PARAMETER(depthStencilViewId, u);
   SVGA3D_DUMP_PARAMETER(sid, u);
   _debug_printf("\t\t.format = %s\n", svga_format_name(cmd->format));
   switch (cmd->resourceDimension)
   {
      SVGA3D_DUMP_TYPE_CASE(resourceDimension, SVGA3D_RESOURCE_BUFFER);
      SVGA3D_DUMP_TYPE_CASE(resourceDimension, SVGA3D_RESOURCE_TEXTURE1D);
      SVGA3D_DUMP_TYPE_CASE(resourceDimension, SVGA3D_RESOURCE_TEXTURE2D);
      SVGA3D_DUMP_TYPE_CASE(resourceDimension, SVGA3D_RESOURCE_TEXTURE3D);
      SVGA3D_DUMP_TYPE_CASE(resourceDimension, SVGA3D_RESOURCE_TEXTURECUBE);
      SVGA3D_DUMP_TYPE_CASE(resourceDimension, SVGA3D_RESOURCE_TYPE_MAX);
      SVGA3D_DUMP_TYPE_DEFAULT(resourceDimension);
   }
   SVGA3D_DUMP_PARAMETER(mipSlice, u);
   SVGA3D_DUMP_PARAMETER(firstArraySlice, u);
   SVGA3D_DUMP_PARAMETER(arraySize, u);
}

SVGA3D_DUMP_HEADER(DestroyDepthStencilView)
{
   SVGA3D_DUMP_PARAMETER(depthStencilViewId, u);
}

SVGA3D_DUMP_HEADER(DefineElementLayout)
{
   SVGA3D_DUMP_PARAMETER(elementLayoutId, u);
}

SVGA3D_DUMP_HEADER(DestroyElementLayout)
{
   SVGA3D_DUMP_PARAMETER(elementLayoutId, u);
}

static void
dump_SVGA3dCmdDXDefineBlendState(const SVGA3dCmdDXDefineBlendState *cmd)
{
   unsigned i;

   SVGA3D_DUMP_PARAMETER(blendId, u);
   SVGA3D_DUMP_PARAMETER(alphaToCoverageEnable, u);
   SVGA3D_DUMP_PARAMETER(independentBlendEnable, u);
   for (i = 0; i < SVGA3D_DX_MAX_RENDER_TARGETS; i++) {
      const SVGA3dDXBlendStatePerRT *rt = cmd->perRT + i;
      _debug_printf("\t\t.perRT[%u].blendEnable = %u\n", i, rt->blendEnable);
      if (rt->blendEnable) {
         _debug_printf("\t\t.perRT[%u].srcBlend = %u\n", i, rt->srcBlend);
         _debug_printf("\t\t.perRT[%u].destBlend = %u\n", i, rt->destBlend);
         _debug_printf("\t\t.perRT[%u].blendOp = %u\n", i, rt->blendOp);
         _debug_printf("\t\t.perRT[%u].srcBlendAlpha = %u\n", i, rt->srcBlendAlpha);
         _debug_printf("\t\t.perRT[%u].destBlendAlpha = %u\n", i, rt->destBlendAlpha);
         _debug_printf("\t\t.perRT[%u].blendOpAlpha = %u\n", i, rt->blendOpAlpha);
      }
      _debug_printf("\t\t.perRT[%u].renderTargetWriteMask = %u\n", i, rt->renderTargetWriteMask);
      _debug_printf("\t\t.perRT[%u].logicOpEnable = %u\n", i, rt->logicOpEnable);
      if (rt->logicOpEnable) {
         _debug_printf("\t\t.perRT[%u].logicOp = %u\n", i, rt->logicOp);
      }
   }
}

SVGA3D_DUMP_HEADER(DestroyBlendState)
{
   SVGA3D_DUMP_PARAMETER(blendId, u);
}

SVGA3D_DUMP_HEADER(DefineDepthStencilState)
{
   SVGA3D_DUMP_PARAMETER(depthStencilId, u);
   SVGA3D_DUMP_PARAMETER(depthEnable, u);
   SVGA3D_DUMP_PARAMETER(depthWriteMask, u);
   SVGA3D_DUMP_PARAMETER(depthFunc, u);
   SVGA3D_DUMP_PARAMETER(stencilEnable, u);
   SVGA3D_DUMP_PARAMETER(frontEnable, u);
   SVGA3D_DUMP_PARAMETER(backEnable, u);
   SVGA3D_DUMP_PARAMETER(stencilReadMask, u);
   SVGA3D_DUMP_PARAMETER(stencilWriteMask, u);
   SVGA3D_DUMP_PARAMETER(frontStencilFailOp, u);
   SVGA3D_DUMP_PARAMETER(frontStencilDepthFailOp, u);
   SVGA3D_DUMP_PARAMETER(frontStencilPassOp, u);
   SVGA3D_DUMP_PARAMETER(frontStencilFunc, u);
   SVGA3D_DUMP_PARAMETER(backStencilFailOp, u);
   SVGA3D_DUMP_PARAMETER(backStencilDepthFailOp, u);
   SVGA3D_DUMP_PARAMETER(backStencilPassOp, u);
   SVGA3D_DUMP_PARAMETER(backStencilFunc, u);
}

SVGA3D_DUMP_HEADER(DestroyDepthStencilState)
{
   SVGA3D_DUMP_PARAMETER(depthStencilId, u);
}

SVGA3D_DUMP_HEADER(DefineRasterizerState)
{
   SVGA3D_DUMP_PARAMETER(rasterizerId, u);
   SVGA3D_DUMP_PARAMETER(fillMode, u);
   SVGA3D_DUMP_PARAMETER(cullMode, u);
   SVGA3D_DUMP_PARAMETER(frontCounterClockwise, u);
   SVGA3D_DUMP_PARAMETER(depthBias, u);
   SVGA3D_DUMP_PARAMETER(depthBiasClamp, f);
   SVGA3D_DUMP_PARAMETER(slopeScaledDepthBias, f);
   SVGA3D_DUMP_PARAMETER(depthClipEnable, u);
   SVGA3D_DUMP_PARAMETER(scissorEnable, u);
   SVGA3D_DUMP_PARAMETER(multisampleEnable, u);
   SVGA3D_DUMP_PARAMETER(antialiasedLineEnable, u);
   SVGA3D_DUMP_PARAMETER(lineWidth, f);
   SVGA3D_DUMP_PARAMETER(lineStippleEnable, u);
   SVGA3D_DUMP_PARAMETER(lineStippleFactor, u);
   SVGA3D_DUMP_PARAMETER(lineStipplePattern, u);
   SVGA3D_DUMP_PARAMETER(provokingVertexLast, u);
}

SVGA3D_DUMP_HEADER(DestroyRasterizerState)
{
   SVGA3D_DUMP_PARAMETER(rasterizerId, u);
}

SVGA3D_DUMP_HEADER(DefineSamplerState)
{
   SVGA3D_DUMP_PARAMETER(samplerId, u);
   SVGA3D_DUMP_PARAMETER(filter, u);
   SVGA3D_DUMP_PARAMETER(addressU, u);
   SVGA3D_DUMP_PARAMETER(addressV, u);
   SVGA3D_DUMP_PARAMETER(addressW, u);
   SVGA3D_DUMP_PARAMETER(mipLODBias, f);
   SVGA3D_DUMP_PARAMETER(maxAnisotropy, u);
   SVGA3D_DUMP_PARAMETER(comparisonFunc, u);
   SVGA3D_DUMP_PARAMETER(borderColor.r, f);
   SVGA3D_DUMP_PARAMETER(borderColor.g, f);
   SVGA3D_DUMP_PARAMETER(borderColor.b, f);
   SVGA3D_DUMP_PARAMETER(borderColor.a, f);
   SVGA3D_DUMP_PARAMETER(minLOD, f);
   SVGA3D_DUMP_PARAMETER(maxLOD, f);
}

SVGA3D_DUMP_HEADER(DestroySamplerState)
{
   SVGA3D_DUMP_PARAMETER(samplerId, u);
}

SVGA3D_DUMP_HEADER(DefineShader)
{
   SVGA3D_DUMP_PARAMETER(shaderId, u);
   debug_printf("\t\t.type = %s\n", shader_name(cmd->type));
   SVGA3D_DUMP_PARAMETER(sizeInBytes, u);
}

SVGA3D_DUMP_HEADER(DestroyShader)
{
   SVGA3D_DUMP_PARAMETER(shaderId, u);
}

SVGA3D_DUMP_HEADER(BindShader)
{
   SVGA3D_DUMP_PARAMETER(cid, u);
   SVGA3D_DUMP_PARAMETER(shid, u);
   SVGA3D_DUMP_PARAMETER(mobid, u);
   SVGA3D_DUMP_PARAMETER(offsetInBytes, u);
}

SVGA3D_DUMP_HEADER(DefineStreamOutput)
{
   int i;
   SVGA3D_DUMP_PARAMETER(soid, u);
   SVGA3D_DUMP_PARAMETER(numOutputStreamEntries, u);
   for (i = 0; i < SVGA3D_DX_MAX_SOTARGETS; i++) {
      _debug_printf("\t\t.streamOutputStrideInBytes[%d] = %u\n",
                    i, cmd->streamOutputStrideInBytes[i]);
   }
   for (i = 0; i < 16; i++)
   {
      _debug_printf("\t\t.decl[%d].outputSlot = %u\n", i, cmd->decl[i].outputSlot);
      _debug_printf("\t\t.decl[%d].registerIndex = %u\n", i, cmd->decl[i].registerIndex);
      _debug_printf("\t\t.decl[%d].registerMask = %u\n", i, cmd->decl[i].registerMask);
   }
}

SVGA3D_DUMP_HEADER(DestroyStreamOutput)
{
   SVGA3D_DUMP_PARAMETER(soid, u);
}

SVGA3D_DUMP_HEADER(SetStreamOutput)
{
   SVGA3D_DUMP_PARAMETER(soid, u);
}

SVGA3D_DUMP_HEADER(SetSingleConstantBuffer)
{
   SVGA3D_DUMP_PARAMETER(slot, u);
   SVGA3D_DUMP_PARAMETER(sid, u);
   debug_printf("\t\t.type = %s\n", shader_name(cmd->type));
   SVGA3D_DUMP_PARAMETER(offsetInBytes, u);
   SVGA3D_DUMP_PARAMETER(sizeInBytes, u);
}

SVGA3D_DUMP_HEADER(SetInputLayout)
{
   SVGA3D_DUMP_PARAMETER(elementLayoutId, u);
}

SVGA3D_DUMP_HEADER(SetVertexBuffers)
{
   SVGA3D_DUMP_PARAMETER(startBuffer, u);

   /* XXX: note we're not printing the SVGA3dVertexBuffer list at this time */
}

SVGA3D_DUMP_HEADER(SetTopology)
{
   switch (cmd->topology)
   {
      SVGA3D_DUMP_TYPE_CASE(topology, SVGA3D_PRIMITIVE_INVALID);
      SVGA3D_DUMP_TYPE_CASE(topology, SVGA3D_PRIMITIVE_TRIANGLELIST);
      SVGA3D_DUMP_TYPE_CASE(topology, SVGA3D_PRIMITIVE_POINTLIST);
      SVGA3D_DUMP_TYPE_CASE(topology, SVGA3D_PRIMITIVE_LINELIST);
      SVGA3D_DUMP_TYPE_CASE(topology, SVGA3D_PRIMITIVE_LINESTRIP);
      SVGA3D_DUMP_TYPE_CASE(topology, SVGA3D_PRIMITIVE_TRIANGLESTRIP);
      SVGA3D_DUMP_TYPE_CASE(topology, SVGA3D_PRIMITIVE_TRIANGLEFAN);
      SVGA3D_DUMP_TYPE_CASE(topology, SVGA3D_PRIMITIVE_MAX);
      SVGA3D_DUMP_TYPE_DEFAULT(topology);
   }
}

SVGA3D_DUMP_HEADER(SetIndexBuffer)
{
   SVGA3D_DUMP_PARAMETER(sid, u);
   _debug_printf("\t\t.format = %s\n", svga_format_name(cmd->format));
   SVGA3D_DUMP_PARAMETER(offset, u);
}

SVGA3D_DUMP_HEADER(PredCopyRegion)
{
   SVGA3D_DUMP_PARAMETER(dstSid, u);
   SVGA3D_DUMP_PARAMETER(dstSubResource, u);
   SVGA3D_DUMP_PARAMETER(srcSid, u);
   SVGA3D_DUMP_PARAMETER(srcSubResource, u);
   dump_SVGA3dCopyBox(&cmd->box);
}

SVGA3D_DUMP_HEADER(PredCopy)
{
   SVGA3D_DUMP_PARAMETER(dstSid, u);
   SVGA3D_DUMP_PARAMETER(srcSid, u);
}

static void
dump_SVGA3dCmdDXUpdateSubResource(const SVGA3dCmdDXUpdateSubResource *cmd)
{
   SVGA3D_DUMP_PARAMETER(sid, u);
   SVGA3D_DUMP_PARAMETER(subResource, u);
   dump_SVGA3dBox(&cmd->box);
}

static void
dump_SVGA3dCmdDXReadbackSubResource(const SVGA3dCmdDXReadbackSubResource *cmd)
{
   SVGA3D_DUMP_PARAMETER(sid, u);
   SVGA3D_DUMP_PARAMETER(subResource, u);
}

SVGA3D_DUMP_HEADER(BufferCopy)
{
   SVGA3D_DUMP_PARAMETER(dest, u);
   SVGA3D_DUMP_PARAMETER(src, u);
   SVGA3D_DUMP_PARAMETER(destX, u);
   SVGA3D_DUMP_PARAMETER(srcX, u);
   SVGA3D_DUMP_PARAMETER(width, u);

}

SVGA3D_DUMP_HEADER(BufferUpdate)
{
   SVGA3D_DUMP_PARAMETER(sid, u);
   SVGA3D_DUMP_PARAMETER(x, u);
   SVGA3D_DUMP_PARAMETER(width, u);

}

SVGA3D_DUMP_HEADER(GenMips)
{
   SVGA3D_DUMP_PARAMETER(shaderResourceViewId, u);
}

SVGA3D_DUMP_HEADER(TransferFromBuffer)
{
   SVGA3D_DUMP_PARAMETER(srcSid, u);
   SVGA3D_DUMP_PARAMETER(srcOffset, u);
   SVGA3D_DUMP_PARAMETER(srcPitch, u);
   SVGA3D_DUMP_PARAMETER(srcSlicePitch, u);
   SVGA3D_DUMP_PARAMETER(destSid, u);
   SVGA3D_DUMP_PARAMETER(destSubResource, u);
   dump_SVGA3dBox(&cmd->destBox);
}

static void
dump_SVGA3dCmdIntraSurfaceCopy(const SVGA3dCmdIntraSurfaceCopy *cmd)
{
   SVGA3D_DUMP_PARAMETER(surface.sid, u);
   SVGA3D_DUMP_PARAMETER(surface.face, u);
   SVGA3D_DUMP_PARAMETER(surface.mipmap, u);
   dump_SVGA3dCopyBox(&cmd->box);
}

static void
dump_SVGA3dCmdInvalidateGBSurface(const SVGA3dCmdInvalidateGBSurface *cmd)
{
   SVGA3D_DUMP_PARAMETER(sid, u);
}

#define SVGA3D_DUMP_CASE_BASIC(CommandName, CommandCode) \
case SVGA_3D_CMD_DX_##CommandCode: \
   _debug_printf(SVGA3D_DUMP_STRINGIFY(\tSVGA_3D_CMD_DX_##CommandCode) "\n"); \
   { \
      const SVGA3dCmdDX##CommandName *cmd = (const SVGA3dCmdDX##CommandName *)body; \
      dump_SVGA3dCmdDX##CommandName(cmd); \
      body = (const uint8_t *)&cmd[1]; \
   } \
   break

#define SVGA3D_DUMP_CASE_LIST(CommandName, CommandCode, ElementType) \
case SVGA_3D_CMD_DX_##CommandCode: \
   _debug_printf(SVGA3D_DUMP_STRINGIFY(\tSVGA_3D_CMD_DX_##CommandCode) "\n"); \
   { \
      const SVGA3dCmdDX##CommandName *cmd = (const SVGA3dCmdDX##CommandName *)body; \
      dump_SVGA3dCmdDX##CommandName(cmd); \
      body = (const uint8_t *)&cmd[1]; \
      while (body + sizeof(ElementType) <= next) \
      { \
         dump_##ElementType((const ElementType *)body); \
         body += sizeof(ElementType); \
      } \
   } \
   break

void            
svga_dump_command(uint32_t cmd_id, const void *data, uint32_t size)
{
   const uint8_t *body = (const uint8_t *)data;
   const uint8_t *next = body + size;
  
   switch(cmd_id) {
   SVGA3D_DUMP_CASE_BASIC(BindContext, BIND_CONTEXT);
   SVGA3D_DUMP_CASE_LIST(SetViewports, SET_VIEWPORTS, SVGA3dViewport);
   SVGA3D_DUMP_CASE_BASIC(SetShader, SET_SHADER);
   SVGA3D_DUMP_CASE_LIST(SetSamplers, SET_SAMPLERS, SVGA3dSamplerId);
   SVGA3D_DUMP_CASE_BASIC(SetBlendState, SET_BLEND_STATE);
   SVGA3D_DUMP_CASE_BASIC(SetDepthStencilState, SET_DEPTHSTENCIL_STATE);
   SVGA3D_DUMP_CASE_BASIC(SetRasterizerState, SET_RASTERIZER_STATE);
   SVGA3D_DUMP_CASE_BASIC(SetPredication, SET_PREDICATION);
   SVGA3D_DUMP_CASE_LIST(SetSOTargets, SET_SOTARGETS, SVGA3dSoTarget);
   SVGA3D_DUMP_CASE_LIST(SetScissorRects, SET_SCISSORRECTS, SVGASignedRect);
   SVGA3D_DUMP_CASE_BASIC(SetStreamOutput, SET_STREAMOUTPUT);
   SVGA3D_DUMP_CASE_BASIC(SetSingleConstantBuffer, SET_SINGLE_CONSTANT_BUFFER);
   SVGA3D_DUMP_CASE_BASIC(Draw, DRAW);
   SVGA3D_DUMP_CASE_BASIC(DrawIndexed, DRAW_INDEXED);
   SVGA3D_DUMP_CASE_BASIC(DrawInstanced, DRAW_INSTANCED);
   SVGA3D_DUMP_CASE_BASIC(DrawIndexedInstanced, DRAW_INDEXED_INSTANCED);
   SVGA3D_DUMP_CASE_BASIC(DrawAuto, DRAW_AUTO);
   SVGA3D_DUMP_CASE_BASIC(DefineQuery, DEFINE_QUERY);
   SVGA3D_DUMP_CASE_BASIC(DestroyQuery, DESTROY_QUERY);
   SVGA3D_DUMP_CASE_BASIC(BindAllQuery, BIND_ALL_QUERY);
   SVGA3D_DUMP_CASE_BASIC(BindQuery, BIND_QUERY);
   SVGA3D_DUMP_CASE_BASIC(MoveQuery, MOVE_QUERY);
   SVGA3D_DUMP_CASE_BASIC(ReadbackAllQuery, READBACK_ALL_QUERY);
   SVGA3D_DUMP_CASE_BASIC(SetQueryOffset, SET_QUERY_OFFSET);
   SVGA3D_DUMP_CASE_BASIC(BeginQuery, BEGIN_QUERY);
   SVGA3D_DUMP_CASE_BASIC(EndQuery, END_QUERY);
   SVGA3D_DUMP_CASE_BASIC(ClearRenderTargetView, CLEAR_RENDERTARGET_VIEW);
   SVGA3D_DUMP_CASE_BASIC(ClearDepthStencilView, CLEAR_DEPTHSTENCIL_VIEW);
   SVGA3D_DUMP_CASE_BASIC(DefineShaderResourceView, DEFINE_SHADERRESOURCE_VIEW);
   SVGA3D_DUMP_CASE_LIST(SetShaderResources, SET_SHADER_RESOURCES, SVGA3dShaderResourceViewId);
   SVGA3D_DUMP_CASE_BASIC(DestroyShaderResourceView, DESTROY_SHADERRESOURCE_VIEW);
   SVGA3D_DUMP_CASE_BASIC(DefineRenderTargetView, DEFINE_RENDERTARGET_VIEW);
   SVGA3D_DUMP_CASE_BASIC(DestroyRenderTargetView, DESTROY_RENDERTARGET_VIEW);
   SVGA3D_DUMP_CASE_BASIC(DefineDepthStencilView, DEFINE_DEPTHSTENCIL_VIEW);
   SVGA3D_DUMP_CASE_BASIC(DestroyDepthStencilView, DESTROY_DEPTHSTENCIL_VIEW);
   SVGA3D_DUMP_CASE_LIST(DefineElementLayout, DEFINE_ELEMENTLAYOUT, SVGA3dInputElementDesc);
   SVGA3D_DUMP_CASE_BASIC(DestroyElementLayout, DESTROY_ELEMENTLAYOUT);
   SVGA3D_DUMP_CASE_BASIC(DefineBlendState, DEFINE_BLEND_STATE);
   SVGA3D_DUMP_CASE_BASIC(DestroyBlendState, DESTROY_BLEND_STATE);
   SVGA3D_DUMP_CASE_BASIC(DefineDepthStencilState, DEFINE_DEPTHSTENCIL_STATE);
   SVGA3D_DUMP_CASE_BASIC(DestroyDepthStencilState, DESTROY_DEPTHSTENCIL_STATE);
   SVGA3D_DUMP_CASE_BASIC(DefineRasterizerState, DEFINE_RASTERIZER_STATE);
   SVGA3D_DUMP_CASE_BASIC(DestroyRasterizerState, DESTROY_RASTERIZER_STATE);
   SVGA3D_DUMP_CASE_BASIC(DefineSamplerState, DEFINE_SAMPLER_STATE);
   SVGA3D_DUMP_CASE_BASIC(DestroySamplerState, DESTROY_SAMPLER_STATE);
   SVGA3D_DUMP_CASE_BASIC(DefineShader, DEFINE_SHADER);
   SVGA3D_DUMP_CASE_BASIC(DestroyShader, DESTROY_SHADER);
   SVGA3D_DUMP_CASE_BASIC(BindShader, BIND_SHADER);
   SVGA3D_DUMP_CASE_BASIC(DefineStreamOutput, DEFINE_STREAMOUTPUT);
   SVGA3D_DUMP_CASE_BASIC(DestroyStreamOutput, DESTROY_STREAMOUTPUT);
   SVGA3D_DUMP_CASE_BASIC(SetInputLayout, SET_INPUT_LAYOUT);
   SVGA3D_DUMP_CASE_LIST(SetVertexBuffers, SET_VERTEX_BUFFERS, SVGA3dVertexBuffer);
   SVGA3D_DUMP_CASE_BASIC(SetTopology, SET_TOPOLOGY);
   SVGA3D_DUMP_CASE_BASIC(SetIndexBuffer, SET_INDEX_BUFFER);

   SVGA3D_DUMP_CASE_BASIC(PredCopy, PRED_COPY);
   SVGA3D_DUMP_CASE_BASIC(UpdateSubResource, UPDATE_SUBRESOURCE);
   SVGA3D_DUMP_CASE_BASIC(ReadbackSubResource, READBACK_SUBRESOURCE);
   SVGA3D_DUMP_CASE_BASIC(PredCopyRegion, PRED_COPY_REGION);
   SVGA3D_DUMP_CASE_BASIC(BufferCopy, BUFFER_COPY);
   SVGA3D_DUMP_CASE_BASIC(BufferUpdate, BUFFER_UPDATE);
   SVGA3D_DUMP_CASE_BASIC(GenMips, GENMIPS);
   SVGA3D_DUMP_CASE_BASIC(TransferFromBuffer, TRANSFER_FROM_BUFFER);

   case SVGA_3D_CMD_DX_SET_RENDERTARGETS:
      _debug_printf("\tSVGA_3D_CMD_DX_SET_RENDERTARGETS\n");
      {
         const SVGA3dCmdDXSetRenderTargets *cmd =
            (const SVGA3dCmdDXSetRenderTargets *) body;
         _debug_printf("\t\t.depthStencilViewId = %u\n",
                       cmd->depthStencilViewId);
         body = (const uint8_t *) &cmd[1];
         while (body + sizeof(SVGA3dRenderTargetViewId) <= next) {
            _debug_printf("\t\t.renderTargetViewId = %u\n",
                          *((SVGA3dRenderTargetViewId *) body));
            body += sizeof(SVGA3dRenderTargetViewId);
         }
      }
      break;

   case SVGA_3D_CMD_SURFACE_DEFINE:
      _debug_printf("\tSVGA_3D_CMD_SURFACE_DEFINE\n");
      {
         const SVGA3dCmdDefineSurface *cmd = (const SVGA3dCmdDefineSurface *)body;
         dump_SVGA3dCmdDefineSurface(cmd);
         body = (const uint8_t *)&cmd[1];
         while(body + sizeof(SVGA3dSize) <= next) {
            dump_SVGA3dSize((const SVGA3dSize *)body);
            body += sizeof(SVGA3dSize);
         }
      }
      break;
   case SVGA_3D_CMD_SURFACE_DESTROY:
      _debug_printf("\tSVGA_3D_CMD_SURFACE_DESTROY\n");
      {
         const SVGA3dCmdDestroySurface *cmd = (const SVGA3dCmdDestroySurface *)body;
         dump_SVGA3dCmdDestroySurface(cmd);
         body = (const uint8_t *)&cmd[1];
      }
      break;
   case SVGA_3D_CMD_SURFACE_COPY:
      _debug_printf("\tSVGA_3D_CMD_SURFACE_COPY\n");
      {
         const SVGA3dCmdSurfaceCopy *cmd = (const SVGA3dCmdSurfaceCopy *)body;
         dump_SVGA3dCmdSurfaceCopy(cmd);
         body = (const uint8_t *)&cmd[1];
         while(body + sizeof(SVGA3dCopyBox) <= next) {
            dump_SVGA3dCopyBox((const SVGA3dCopyBox *)body);
            body += sizeof(SVGA3dCopyBox);
         }
      }
      break;
   case SVGA_3D_CMD_SURFACE_STRETCHBLT:
      _debug_printf("\tSVGA_3D_CMD_SURFACE_STRETCHBLT\n");
      {
         const SVGA3dCmdSurfaceStretchBlt *cmd = (const SVGA3dCmdSurfaceStretchBlt *)body;
         dump_SVGA3dCmdSurfaceStretchBlt(cmd);
         body = (const uint8_t *)&cmd[1];
      }
      break;
   case SVGA_3D_CMD_SURFACE_DMA:
      _debug_printf("\tSVGA_3D_CMD_SURFACE_DMA\n");
      {
         const SVGA3dCmdSurfaceDMA *cmd = (const SVGA3dCmdSurfaceDMA *)body;
         dump_SVGA3dCmdSurfaceDMA(cmd);
         body = (const uint8_t *)&cmd[1];
         while(body + sizeof(SVGA3dCopyBox) <= next) {
            dump_SVGA3dCopyBox((const SVGA3dCopyBox *)body);
            body += sizeof(SVGA3dCopyBox);
         }
         while(body + sizeof(SVGA3dCmdSurfaceDMASuffix) <= next) {
            dump_SVGA3dCmdSurfaceDMASuffix((const SVGA3dCmdSurfaceDMASuffix *)body);
            body += sizeof(SVGA3dCmdSurfaceDMASuffix);
         }
      }
      break;
   case SVGA_3D_CMD_CONTEXT_DEFINE:
      _debug_printf("\tSVGA_3D_CMD_CONTEXT_DEFINE\n");
      {
         const SVGA3dCmdDefineContext *cmd = (const SVGA3dCmdDefineContext *)body;
         dump_SVGA3dCmdDefineContext(cmd);
         body = (const uint8_t *)&cmd[1];
      }
      break;
   case SVGA_3D_CMD_CONTEXT_DESTROY:
      _debug_printf("\tSVGA_3D_CMD_CONTEXT_DESTROY\n");
      {
         const SVGA3dCmdDestroyContext *cmd = (const SVGA3dCmdDestroyContext *)body;
         dump_SVGA3dCmdDestroyContext(cmd);
         body = (const uint8_t *)&cmd[1];
      }
      break;
   case SVGA_3D_CMD_SETTRANSFORM:
      _debug_printf("\tSVGA_3D_CMD_SETTRANSFORM\n");
      {
         const SVGA3dCmdSetTransform *cmd = (const SVGA3dCmdSetTransform *)body;
         dump_SVGA3dCmdSetTransform(cmd);
         body = (const uint8_t *)&cmd[1];
      }
      break;
   case SVGA_3D_CMD_SETZRANGE:
      _debug_printf("\tSVGA_3D_CMD_SETZRANGE\n");
      {
         const SVGA3dCmdSetZRange *cmd = (const SVGA3dCmdSetZRange *)body;
         dump_SVGA3dCmdSetZRange(cmd);
         body = (const uint8_t *)&cmd[1];
      }
      break;
   case SVGA_3D_CMD_SETRENDERSTATE:
      _debug_printf("\tSVGA_3D_CMD_SETRENDERSTATE\n");
      {
         const SVGA3dCmdSetRenderState *cmd = (const SVGA3dCmdSetRenderState *)body;
         dump_SVGA3dCmdSetRenderState(cmd);
         body = (const uint8_t *)&cmd[1];
         while(body + sizeof(SVGA3dRenderState) <= next) {
            dump_SVGA3dRenderState((const SVGA3dRenderState *)body);
            body += sizeof(SVGA3dRenderState);
         }
      }
      break;
   case SVGA_3D_CMD_SETRENDERTARGET:
      _debug_printf("\tSVGA_3D_CMD_SETRENDERTARGET\n");
      {
         const SVGA3dCmdSetRenderTarget *cmd = (const SVGA3dCmdSetRenderTarget *)body;
         dump_SVGA3dCmdSetRenderTarget(cmd);
         body = (const uint8_t *)&cmd[1];
      }
      break;
   case SVGA_3D_CMD_SETTEXTURESTATE:
      _debug_printf("\tSVGA_3D_CMD_SETTEXTURESTATE\n");
      {
         const SVGA3dCmdSetTextureState *cmd = (const SVGA3dCmdSetTextureState *)body;
         dump_SVGA3dCmdSetTextureState(cmd);
         body = (const uint8_t *)&cmd[1];
         while(body + sizeof(SVGA3dTextureState) <= next) {
            dump_SVGA3dTextureState((const SVGA3dTextureState *)body);
            body += sizeof(SVGA3dTextureState);
         }
      }
      break;
   case SVGA_3D_CMD_SETMATERIAL:
      _debug_printf("\tSVGA_3D_CMD_SETMATERIAL\n");
      {
         const SVGA3dCmdSetMaterial *cmd = (const SVGA3dCmdSetMaterial *)body;
         dump_SVGA3dCmdSetMaterial(cmd);
         body = (const uint8_t *)&cmd[1];
      }
      break;
   case SVGA_3D_CMD_SETLIGHTDATA:
      _debug_printf("\tSVGA_3D_CMD_SETLIGHTDATA\n");
      {
         const SVGA3dCmdSetLightData *cmd = (const SVGA3dCmdSetLightData *)body;
         dump_SVGA3dCmdSetLightData(cmd);
         body = (const uint8_t *)&cmd[1];
      }
      break;
   case SVGA_3D_CMD_SETLIGHTENABLED:
      _debug_printf("\tSVGA_3D_CMD_SETLIGHTENABLED\n");
      {
         const SVGA3dCmdSetLightEnabled *cmd = (const SVGA3dCmdSetLightEnabled *)body;
         dump_SVGA3dCmdSetLightEnabled(cmd);
         body = (const uint8_t *)&cmd[1];
      }
      break;
   case SVGA_3D_CMD_SETVIEWPORT:
      _debug_printf("\tSVGA_3D_CMD_SETVIEWPORT\n");
      {
         const SVGA3dCmdSetViewport *cmd = (const SVGA3dCmdSetViewport *)body;
         dump_SVGA3dCmdSetViewport(cmd);
         body = (const uint8_t *)&cmd[1];
      }
      break;
   case SVGA_3D_CMD_SETCLIPPLANE:
      _debug_printf("\tSVGA_3D_CMD_SETCLIPPLANE\n");
      {
         const SVGA3dCmdSetClipPlane *cmd = (const SVGA3dCmdSetClipPlane *)body;
         dump_SVGA3dCmdSetClipPlane(cmd);
         body = (const uint8_t *)&cmd[1];
      }
      break;
   case SVGA_3D_CMD_CLEAR:
      _debug_printf("\tSVGA_3D_CMD_CLEAR\n");
      {
         const SVGA3dCmdClear *cmd = (const SVGA3dCmdClear *)body;
         dump_SVGA3dCmdClear(cmd);
         body = (const uint8_t *)&cmd[1];
         while(body + sizeof(SVGA3dRect) <= next) {
            dump_SVGA3dRect((const SVGA3dRect *)body);
            body += sizeof(SVGA3dRect);
         }
      }
      break;
   case SVGA_3D_CMD_PRESENT:
      _debug_printf("\tSVGA_3D_CMD_PRESENT\n");
      {
         const SVGA3dCmdPresent *cmd = (const SVGA3dCmdPresent *)body;
         dump_SVGA3dCmdPresent(cmd);
         body = (const uint8_t *)&cmd[1];
         while(body + sizeof(SVGA3dCopyRect) <= next) {
            dump_SVGA3dCopyRect((const SVGA3dCopyRect *)body);
            body += sizeof(SVGA3dCopyRect);
         }
      }
      break;
   case SVGA_3D_CMD_SHADER_DEFINE:
      _debug_printf("\tSVGA_3D_CMD_SHADER_DEFINE\n");
      {
         const SVGA3dCmdDefineShader *cmd = (const SVGA3dCmdDefineShader *)body;
         dump_SVGA3dCmdDefineShader(cmd);
         body = (const uint8_t *)&cmd[1];
         svga_shader_dump((const uint32_t *)body, 
                      (unsigned)(next - body)/sizeof(uint32_t),
                      false );
         body = next;
      }
      break;
   case SVGA_3D_CMD_SHADER_DESTROY:
      _debug_printf("\tSVGA_3D_CMD_SHADER_DESTROY\n");
      {
         const SVGA3dCmdDestroyShader *cmd = (const SVGA3dCmdDestroyShader *)body;
         dump_SVGA3dCmdDestroyShader(cmd);
         body = (const uint8_t *)&cmd[1];
      }
      break;
   case SVGA_3D_CMD_SET_SHADER:
      _debug_printf("\tSVGA_3D_CMD_SET_SHADER\n");
      {
         const SVGA3dCmdSetShader *cmd = (const SVGA3dCmdSetShader *)body;
         dump_SVGA3dCmdSetShader(cmd);
         body = (const uint8_t *)&cmd[1];
      }
      break;
   case SVGA_3D_CMD_SET_SHADER_CONST:
      _debug_printf("\tSVGA_3D_CMD_SET_SHADER_CONST\n");
      {
         const SVGA3dCmdSetShaderConst *cmd = (const SVGA3dCmdSetShaderConst *)body;
         uint32 numConsts = 1 + (size - sizeof *cmd) / (4 * sizeof(uint32));
         dump_SVGA3dCmdSetShaderConst(cmd, numConsts);
         body = next;
      }
      break;
   case SVGA_3D_CMD_DRAW_PRIMITIVES:
      _debug_printf("\tSVGA_3D_CMD_DRAW_PRIMITIVES\n");
      {
         const SVGA3dCmdDrawPrimitives *cmd = (const SVGA3dCmdDrawPrimitives *)body;
         unsigned i, j;
         dump_SVGA3dCmdDrawPrimitives(cmd);
         body = (const uint8_t *)&cmd[1];
         for(i = 0; i < cmd->numVertexDecls; ++i) {
            dump_SVGA3dVertexDecl((const SVGA3dVertexDecl *)body);
            body += sizeof(SVGA3dVertexDecl);
         }
         for(j = 0; j < cmd->numRanges; ++j) {
            dump_SVGA3dPrimitiveRange((const SVGA3dPrimitiveRange *)body);
            body += sizeof(SVGA3dPrimitiveRange);
         }
         while(body + sizeof(SVGA3dVertexDivisor) <= next) {
            dump_SVGA3dVertexDivisor((const SVGA3dVertexDivisor *)body);
            body += sizeof(SVGA3dVertexDivisor);
         }
      }
      break;
   case SVGA_3D_CMD_SETSCISSORRECT:
      _debug_printf("\tSVGA_3D_CMD_SETSCISSORRECT\n");
      {
         const SVGA3dCmdSetScissorRect *cmd = (const SVGA3dCmdSetScissorRect *)body;
         dump_SVGA3dCmdSetScissorRect(cmd);
         body = (const uint8_t *)&cmd[1];
      }
      break;
   case SVGA_3D_CMD_BEGIN_QUERY:
      _debug_printf("\tSVGA_3D_CMD_BEGIN_QUERY\n");
      {
         const SVGA3dCmdBeginQuery *cmd = (const SVGA3dCmdBeginQuery *)body;
         dump_SVGA3dCmdBeginQuery(cmd);
         body = (const uint8_t *)&cmd[1];
      }
      break;
   case SVGA_3D_CMD_END_QUERY:
      _debug_printf("\tSVGA_3D_CMD_END_QUERY\n");
      {
         const SVGA3dCmdEndQuery *cmd = (const SVGA3dCmdEndQuery *)body;
         dump_SVGA3dCmdEndQuery(cmd);
         body = (const uint8_t *)&cmd[1];
      }
      break;
   case SVGA_3D_CMD_WAIT_FOR_QUERY:
      _debug_printf("\tSVGA_3D_CMD_WAIT_FOR_QUERY\n");
      {
         const SVGA3dCmdWaitForQuery *cmd = (const SVGA3dCmdWaitForQuery *)body;
         dump_SVGA3dCmdWaitForQuery(cmd);
         body = (const uint8_t *)&cmd[1];
      }
      break;
   case SVGA_3D_CMD_BLIT_SURFACE_TO_SCREEN:
      _debug_printf("\tSVGA_3D_CMD_BLIT_SURFACE_TO_SCREEN\n");
      {
         const SVGA3dCmdBlitSurfaceToScreen *cmd = (const SVGA3dCmdBlitSurfaceToScreen *)body;
         dump_SVGA3dCmdBlitSurfaceToScreen(cmd);
         body = (const uint8_t *)&cmd[1];
         while(body + sizeof(SVGASignedRect) <= next) {
            dump_SVGASignedRect((const SVGASignedRect *)body);
            body += sizeof(SVGASignedRect);
         }
      }
      break;
   case SVGA_3D_CMD_DEFINE_GB_CONTEXT:
      _debug_printf("\tSVGA_3D_CMD_DEFINE_GB_CONTEXT\n");
      {
         const SVGA3dCmdDefineGBContext *cmd = (const SVGA3dCmdDefineGBContext *) body;
         dump_SVGA3dCmdDefineGBContext(cmd);
         body = (const uint8_t *)&cmd[1];
      }
      break;
   case SVGA_3D_CMD_BIND_GB_CONTEXT:
      _debug_printf("\tSVGA_3D_CMD_BIND_GB_CONTEXT\n");
      {
         const SVGA3dCmdBindGBContext *cmd = (const SVGA3dCmdBindGBContext *) body;
         dump_SVGA3dCmdBindGBContext(cmd);
         body = (const uint8_t *)&cmd[1];
      }
      break;
   case SVGA_3D_CMD_DESTROY_GB_CONTEXT:
      _debug_printf("\tSVGA_3D_CMD_DESTROY_GB_CONTEXT\n");
      {
         const SVGA3dCmdDestroyGBContext *cmd = (const SVGA3dCmdDestroyGBContext *) body;
         dump_SVGA3dCmdDestroyGBContext(cmd);
         body = (const uint8_t *)&cmd[1];
      }
      break;
   case SVGA_3D_CMD_DEFINE_GB_SHADER:
      _debug_printf("\tSVGA_3D_CMD_DEFINE_GB_SHADER\n");
      {
         const SVGA3dCmdDefineGBShader *cmd = (const SVGA3dCmdDefineGBShader *) body;
         dump_SVGA3dCmdDefineGBShader(cmd);
         body = (const uint8_t *)&cmd[1];
      }
      break;
   case SVGA_3D_CMD_BIND_GB_SHADER:
      _debug_printf("\tSVGA_3D_CMD_BIND_GB_SHADER\n");
      {
         const SVGA3dCmdBindGBShader *cmd = (const SVGA3dCmdBindGBShader *) body;
         dump_SVGA3dCmdBindGBShader(cmd);
         body = (const uint8_t *)&cmd[1];
      }
      break;
   case SVGA_3D_CMD_DESTROY_GB_SHADER:
      _debug_printf("\tSVGA_3D_CMD_DESTROY_GB_SHADER\n");
      {
         const SVGA3dCmdDestroyGBShader *cmd = (const SVGA3dCmdDestroyGBShader *) body;
         dump_SVGA3dCmdDestroyGBShader(cmd);
         body = (const uint8_t *)&cmd[1];
      }
      break;
   case SVGA_3D_CMD_BIND_GB_SURFACE:
      _debug_printf("\tSVGA_3D_CMD_BIND_GB_SURFACE\n");
      {
         const SVGA3dCmdBindGBSurface *cmd = (const SVGA3dCmdBindGBSurface *) body;
         dump_SVGA3dCmdBindGBSurface(cmd);
         body = (const uint8_t *)&cmd[1];
      }
      break;
   case SVGA_3D_CMD_UPDATE_GB_SURFACE:
      _debug_printf("\tSVGA_3D_CMD_UPDATE_GB_SURFACE\n");
      {
         const SVGA3dCmdUpdateGBSurface *cmd = (const SVGA3dCmdUpdateGBSurface *) body;
         dump_SVGA3dCmdUpdateGBSurface(cmd);
         body = (const uint8_t *)&cmd[1];
      }
      break;
   case SVGA_3D_CMD_READBACK_GB_IMAGE:
      _debug_printf("\tSVGA_3D_CMD_READBACK_GB_IMAGE:\n");
      {
         const SVGA3dCmdReadbackGBImage *cmd = (SVGA3dCmdReadbackGBImage *) body;
         dump_SVGA3dCmdReadbackGBImage(cmd);
         body = (const uint8_t *)&cmd[1];
      }
      break;
   case SVGA_3D_CMD_UPDATE_GB_IMAGE:
      _debug_printf("\tSVGA_3D_CMD_UPDATE_GB_IMAGE\n");
      {
         const SVGA3dCmdUpdateGBImage *cmd = (const SVGA3dCmdUpdateGBImage *) body;
         dump_SVGA3dCmdUpdateGBImage(cmd);
         body = (const uint8_t *)&cmd[1];
      }
      break;
   case SVGA_3D_CMD_INVALIDATE_GB_IMAGE:
      _debug_printf("\tSVGA_3D_CMD_INVALIDATE_GB_IMAGE\n");
      {
         const SVGA3dCmdInvalidateGBImage *cmd = (const SVGA3dCmdInvalidateGBImage *) body;
         dump_SVGA3dCmdInvalidateGBImage(cmd);
         body = (const uint8_t *)&cmd[1];
      }
      break;
   case SVGA_3D_CMD_INVALIDATE_GB_IMAGE_PARTIAL:
      _debug_printf("\tSVGA_3D_CMD_INVALIDATE_GB_IMAGE_PARTIAL\n");
      {
         const SVGA3dCmdInvalidateGBImagePartial *cmd = (const SVGA3dCmdInvalidateGBImagePartial *) body;
         dump_SVGA3dCmdInvalidateGBImagePartial(cmd);
         body = (const uint8_t *)&cmd[1];
      }
      break;
   case SVGA_3D_CMD_SET_GB_SHADERCONSTS_INLINE:
      _debug_printf("\tSVGA_3D_CMD_SET_GB_SHADERCONSTS_INLINE\n");
      {
         /* XXX Note: re-using the SVGA3dCmdSetShaderConst code here */
         const SVGA3dCmdSetGBShaderConstInline *cmd = (const SVGA3dCmdSetGBShaderConstInline *)body;
         uint32 numConsts = (size - sizeof *cmd) / (4 * sizeof(uint32));
         dump_SVGA3dCmdSetGBShaderConstInline(cmd, numConsts);
         body = next;
      }
      break;
   case SVGA_3D_CMD_INVALIDATE_GB_SURFACE:
      _debug_printf("\tSVGA_3D_CMD_INVALIDATE_GB_SURFACE\n");
      {
         const SVGA3dCmdInvalidateGBSurface *cmd = (const SVGA3dCmdInvalidateGBSurface *)body;
         dump_SVGA3dCmdInvalidateGBSurface(cmd);
         body = (const uint8_t *)&cmd[1];
      }
      break;
   case SVGA_3D_CMD_INTRA_SURFACE_COPY:
      _debug_printf("\tSVGA_3D_CMD_INTRA_SURFACE_COPY\n");
      {
         const SVGA3dCmdIntraSurfaceCopy *cmd = (const SVGA3dCmdIntraSurfaceCopy *)body;
         dump_SVGA3dCmdIntraSurfaceCopy(cmd);
         body = (const uint8_t *)&cmd[1];
      }
      break;
   default:
      _debug_printf("\t0x%08x\n", cmd_id);
      break;
   }

   while(body + sizeof(uint32_t) <= next) {
      _debug_printf("\t\t0x%08x\n", *(const uint32_t *)body);
      body += sizeof(uint32_t);
   }
   while(body + sizeof(uint32_t) <= next)
      _debug_printf("\t\t0x%02x\n", *body++);
}


void            
svga_dump_commands(const void *commands, uint32_t size)
{
   const uint8_t *next = commands;
   const uint8_t *last = next + size;
   
   assert(size % sizeof(uint32_t) == 0);
   
   while(next < last) {
      const uint32_t cmd_id = *(const uint32_t *)next;

      if(SVGA_3D_CMD_BASE <= cmd_id && cmd_id < SVGA_3D_CMD_MAX) {
         const SVGA3dCmdHeader *header = (const SVGA3dCmdHeader *)next;
         const uint8_t *body = (const uint8_t *)&header[1];

         next = body + header->size;
         if(next > last)
            break;

         svga_dump_command(cmd_id, body, header->size);
      }
      else if(cmd_id == SVGA_CMD_FENCE) {
         _debug_printf("\tSVGA_CMD_FENCE\n");
         _debug_printf("\t\t0x%08x\n", ((const uint32_t *)next)[1]);
         next += 2*sizeof(uint32_t);
      }
      else {
         _debug_printf("\t0x%08x\n", cmd_id);
         next += sizeof(uint32_t);
      }
   }
}

