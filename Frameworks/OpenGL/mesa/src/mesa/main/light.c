/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2008  Brian Paul   All Rights Reserved.
 * Copyright (C) 2009  VMware, Inc.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */


#include "util/glheader.h"
#include "context.h"
#include "enums.h"
#include "light.h"
#include "macros.h"
#include "mtypes.h"
#include "math/m_matrix.h"
#include "util/bitscan.h"
#include "api_exec_decl.h"

#include <math.h>

void GLAPIENTRY
_mesa_ShadeModel( GLenum mode )
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glShadeModel %s\n", _mesa_enum_to_string(mode));

   if (ctx->Light.ShadeModel == mode)
      return;

   if (mode != GL_FLAT && mode != GL_SMOOTH) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glShadeModel");
      return;
   }

   FLUSH_VERTICES(ctx, _NEW_LIGHT_STATE, GL_LIGHTING_BIT);
   ctx->Light.ShadeModel = mode;
}


/**
 * Set the provoking vertex (the vertex which specifies the prim's
 * color when flat shading) to either the first or last vertex of the
 * triangle or line.
 */
void GLAPIENTRY
_mesa_ProvokingVertex(GLenum mode)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE&VERBOSE_API)
      _mesa_debug(ctx, "glProvokingVertexEXT 0x%x\n", mode);

   if (ctx->Light.ProvokingVertex == mode)
      return;

   switch (mode) {
   case GL_FIRST_VERTEX_CONVENTION_EXT:
   case GL_LAST_VERTEX_CONVENTION_EXT:
      break;
   default:
      _mesa_error(ctx, GL_INVALID_ENUM, "glProvokingVertexEXT(0x%x)", mode);
      return;
   }

   FLUSH_VERTICES(ctx, _NEW_LIGHT_STATE, GL_LIGHTING_BIT);
   ctx->Light.ProvokingVertex = mode;
}


/**
 * Helper function called by _mesa_Lightfv and _mesa_PopAttrib to set
 * per-light state.
 * For GL_POSITION and GL_SPOT_DIRECTION the params position/direction
 * will have already been transformed by the modelview matrix!
 * Also, all error checking should have already been done.
 */
static void
do_light(struct gl_context *ctx, GLuint lnum, GLenum pname, const GLfloat *params)
{
   struct gl_light *light;

   assert(lnum < MAX_LIGHTS);
   light = &ctx->Light.Light[lnum];

   struct gl_light_uniforms *lu = &ctx->Light.LightSource[lnum];

   switch (pname) {
   case GL_AMBIENT:
      if (TEST_EQ_4V(lu->Ambient, params))
	 return;
      FLUSH_VERTICES(ctx, _NEW_LIGHT_CONSTANTS, GL_LIGHTING_BIT);
      COPY_4V( lu->Ambient, params );
      break;
   case GL_DIFFUSE:
      if (TEST_EQ_4V(lu->Diffuse, params))
	 return;
      FLUSH_VERTICES(ctx, _NEW_LIGHT_CONSTANTS, GL_LIGHTING_BIT);
      COPY_4V( lu->Diffuse, params );
      break;
   case GL_SPECULAR:
      if (TEST_EQ_4V(lu->Specular, params))
	 return;
      FLUSH_VERTICES(ctx, _NEW_LIGHT_CONSTANTS, GL_LIGHTING_BIT);
      COPY_4V( lu->Specular, params );
      break;
   case GL_POSITION: {
      /* NOTE: position has already been transformed by ModelView! */
      if (TEST_EQ_4V(lu->EyePosition, params))
	 return;
      FLUSH_VERTICES(ctx, _NEW_LIGHT_CONSTANTS, GL_LIGHTING_BIT);

      bool old_positional = lu->EyePosition[3] != 0.0f;
      bool positional = params[3] != 0.0f;
      COPY_4V(lu->EyePosition, params);

      if (positional != old_positional) {
         if (positional)
            light->_Flags |= LIGHT_POSITIONAL;
         else
            light->_Flags &= ~LIGHT_POSITIONAL;

         /* Used by fixed-func vertex program. */
         ctx->NewState |= _NEW_FF_VERT_PROGRAM;
      }

      static const GLfloat eye_z[] = {0, 0, 1};
      GLfloat p[3];
      /* Compute infinite half angle vector:
       *   halfVector = normalize(normalize(lightPos) + (0, 0, 1))
       * light.EyePosition.w should be 0 for infinite lights.
       */
      COPY_3V(p, params);
      NORMALIZE_3FV(p);
      ADD_3V(p, p, eye_z);
      NORMALIZE_3FV(p);
      COPY_3V(lu->_HalfVector, p);
      lu->_HalfVector[3] = 1.0;
      break;
   }
   case GL_SPOT_DIRECTION:
      /* NOTE: Direction already transformed by inverse ModelView! */
      if (TEST_EQ_3V(lu->SpotDirection, params))
	 return;
      FLUSH_VERTICES(ctx, _NEW_LIGHT_CONSTANTS, GL_LIGHTING_BIT);
      COPY_3V(lu->SpotDirection, params);
      break;
   case GL_SPOT_EXPONENT:
      assert(params[0] >= 0.0F);
      assert(params[0] <= ctx->Const.MaxSpotExponent);
      if (lu->SpotExponent == params[0])
	 return;
      FLUSH_VERTICES(ctx, _NEW_LIGHT_CONSTANTS, GL_LIGHTING_BIT);
      lu->SpotExponent = params[0];
      break;
   case GL_SPOT_CUTOFF: {
      assert(params[0] == 180.0F || (params[0] >= 0.0F && params[0] <= 90.0F));
      if (lu->SpotCutoff == params[0])
         return;
      FLUSH_VERTICES(ctx, _NEW_LIGHT_CONSTANTS, GL_LIGHTING_BIT);

      bool old_is_180 = lu->SpotCutoff == 180.0f;
      bool is_180 = params[0] == 180.0f;
      lu->SpotCutoff = params[0];
      lu->_CosCutoff = (cosf(lu->SpotCutoff * M_PI / 180.0));
      if (lu->_CosCutoff < 0)
         lu->_CosCutoff = 0;

      if (is_180 != old_is_180) {
         if (!is_180)
            light->_Flags |= LIGHT_SPOT;
         else
            light->_Flags &= ~LIGHT_SPOT;

         /* Used by fixed-func vertex program. */
         ctx->NewState |= _NEW_FF_VERT_PROGRAM;
      }
      break;
   }
   case GL_CONSTANT_ATTENUATION: {
      assert(params[0] >= 0.0F);
      if (lu->ConstantAttenuation == params[0])
	 return;
      FLUSH_VERTICES(ctx, _NEW_LIGHT_CONSTANTS, GL_LIGHTING_BIT);

      bool old_is_one = lu->ConstantAttenuation == 1.0f;
      bool is_one = params[0] == 1.0f;
      lu->ConstantAttenuation = params[0];

      if (old_is_one != is_one) {
         /* Used by fixed-func vertex program. */
         ctx->NewState |= _NEW_FF_VERT_PROGRAM;
      }
      break;
   }
   case GL_LINEAR_ATTENUATION: {
      assert(params[0] >= 0.0F);
      if (lu->LinearAttenuation == params[0])
	 return;
      FLUSH_VERTICES(ctx, _NEW_LIGHT_CONSTANTS, GL_LIGHTING_BIT);

      bool old_is_zero = lu->LinearAttenuation == 0.0f;
      bool is_zero = params[0] == 0.0f;
      lu->LinearAttenuation = params[0];

      if (old_is_zero != is_zero) {
         /* Used by fixed-func vertex program. */
         ctx->NewState |= _NEW_FF_VERT_PROGRAM;
      }
      break;
   }
   case GL_QUADRATIC_ATTENUATION: {
      assert(params[0] >= 0.0F);
      if (lu->QuadraticAttenuation == params[0])
	 return;
      FLUSH_VERTICES(ctx, _NEW_LIGHT_CONSTANTS, GL_LIGHTING_BIT);

      bool old_is_zero = lu->QuadraticAttenuation == 0.0f;
      bool is_zero = params[0] == 0.0f;
      lu->QuadraticAttenuation = params[0];

      if (old_is_zero != is_zero) {
         /* Used by fixed-func vertex program. */
         ctx->NewState |= _NEW_FF_VERT_PROGRAM;
      }
      break;
   }
   default:
      unreachable("Unexpected pname in do_light()");
   }
}


void GLAPIENTRY
_mesa_Lightf( GLenum light, GLenum pname, GLfloat param )
{
   GLfloat fparam[4];
   fparam[0] = param;
   fparam[1] = fparam[2] = fparam[3] = 0.0F;
   _mesa_Lightfv( light, pname, fparam );
}


void GLAPIENTRY
_mesa_Lightfv( GLenum light, GLenum pname, const GLfloat *params )
{
   GET_CURRENT_CONTEXT(ctx);
   GLint i = (GLint) (light - GL_LIGHT0);
   GLfloat temp[4];

   if (i < 0 || i >= (GLint) ctx->Const.MaxLights) {
      _mesa_error( ctx, GL_INVALID_ENUM, "glLight(light=0x%x)", light );
      return;
   }

   /* do particular error checks, transformations */
   switch (pname) {
   case GL_AMBIENT:
   case GL_DIFFUSE:
   case GL_SPECULAR:
      /* nothing */
      break;
   case GL_POSITION:
      /* transform position by ModelView matrix */
      TRANSFORM_POINT(temp, ctx->ModelviewMatrixStack.Top->m, params);
      params = temp;
      break;
   case GL_SPOT_DIRECTION:
      /* transform direction by inverse modelview */
      if (_math_matrix_is_dirty(ctx->ModelviewMatrixStack.Top)) {
	 _math_matrix_analyse(ctx->ModelviewMatrixStack.Top);
      }
      TRANSFORM_DIRECTION(temp, params, ctx->ModelviewMatrixStack.Top->m);
      params = temp;
      break;
   case GL_SPOT_EXPONENT:
      if (params[0] < 0.0F || params[0] > ctx->Const.MaxSpotExponent) {
	 _mesa_error(ctx, GL_INVALID_VALUE, "glLight");
	 return;
      }
      break;
   case GL_SPOT_CUTOFF:
      if ((params[0] < 0.0F || params[0] > 90.0F) && params[0] != 180.0F) {
	 _mesa_error(ctx, GL_INVALID_VALUE, "glLight");
	 return;
      }
      break;
   case GL_CONSTANT_ATTENUATION:
   case GL_LINEAR_ATTENUATION:
   case GL_QUADRATIC_ATTENUATION:
      if (params[0] < 0.0F) {
	 _mesa_error(ctx, GL_INVALID_VALUE, "glLight");
	 return;
      }
      break;
   default:
      _mesa_error(ctx, GL_INVALID_ENUM, "glLight(pname=0x%x)", pname);
      return;
   }

   do_light(ctx, i, pname, params);
}


void GLAPIENTRY
_mesa_Lighti( GLenum light, GLenum pname, GLint param )
{
   GLint iparam[4];
   iparam[0] = param;
   iparam[1] = iparam[2] = iparam[3] = 0;
   _mesa_Lightiv( light, pname, iparam );
}


void GLAPIENTRY
_mesa_Lightiv( GLenum light, GLenum pname, const GLint *params )
{
   GLfloat fparam[4];

   switch (pname) {
      case GL_AMBIENT:
      case GL_DIFFUSE:
      case GL_SPECULAR:
         fparam[0] = INT_TO_FLOAT( params[0] );
         fparam[1] = INT_TO_FLOAT( params[1] );
         fparam[2] = INT_TO_FLOAT( params[2] );
         fparam[3] = INT_TO_FLOAT( params[3] );
         break;
      case GL_POSITION:
         fparam[0] = (GLfloat) params[0];
         fparam[1] = (GLfloat) params[1];
         fparam[2] = (GLfloat) params[2];
         fparam[3] = (GLfloat) params[3];
         break;
      case GL_SPOT_DIRECTION:
         fparam[0] = (GLfloat) params[0];
         fparam[1] = (GLfloat) params[1];
         fparam[2] = (GLfloat) params[2];
         break;
      case GL_SPOT_EXPONENT:
      case GL_SPOT_CUTOFF:
      case GL_CONSTANT_ATTENUATION:
      case GL_LINEAR_ATTENUATION:
      case GL_QUADRATIC_ATTENUATION:
         fparam[0] = (GLfloat) params[0];
         break;
      default:
         /* error will be caught later in gl_Lightfv */
         ;
   }

   _mesa_Lightfv( light, pname, fparam );
}



void GLAPIENTRY
_mesa_GetLightfv( GLenum light, GLenum pname, GLfloat *params )
{
   GET_CURRENT_CONTEXT(ctx);
   GLint l = (GLint) (light - GL_LIGHT0);

   if (l < 0 || l >= (GLint) ctx->Const.MaxLights) {
      _mesa_error( ctx, GL_INVALID_ENUM, "glGetLightfv" );
      return;
   }

   switch (pname) {
      case GL_AMBIENT:
         COPY_4V( params, ctx->Light.LightSource[l].Ambient );
         break;
      case GL_DIFFUSE:
         COPY_4V( params, ctx->Light.LightSource[l].Diffuse );
         break;
      case GL_SPECULAR:
         COPY_4V( params, ctx->Light.LightSource[l].Specular );
         break;
      case GL_POSITION:
         COPY_4V( params, ctx->Light.LightSource[l].EyePosition );
         break;
      case GL_SPOT_DIRECTION:
         COPY_3V( params, ctx->Light.LightSource[l].SpotDirection );
         break;
      case GL_SPOT_EXPONENT:
         params[0] = ctx->Light.LightSource[l].SpotExponent;
         break;
      case GL_SPOT_CUTOFF:
         params[0] = ctx->Light.LightSource[l].SpotCutoff;
         break;
      case GL_CONSTANT_ATTENUATION:
         params[0] = ctx->Light.LightSource[l].ConstantAttenuation;
         break;
      case GL_LINEAR_ATTENUATION:
         params[0] = ctx->Light.LightSource[l].LinearAttenuation;
         break;
      case GL_QUADRATIC_ATTENUATION:
         params[0] = ctx->Light.LightSource[l].QuadraticAttenuation;
         break;
      default:
         _mesa_error( ctx, GL_INVALID_ENUM, "glGetLightfv" );
         break;
   }
}


void GLAPIENTRY
_mesa_GetLightiv( GLenum light, GLenum pname, GLint *params )
{
   GET_CURRENT_CONTEXT(ctx);
   GLint l = (GLint) (light - GL_LIGHT0);

   if (l < 0 || l >= (GLint) ctx->Const.MaxLights) {
      _mesa_error( ctx, GL_INVALID_ENUM, "glGetLightiv" );
      return;
   }

   switch (pname) {
      case GL_AMBIENT:
         params[0] = FLOAT_TO_INT(ctx->Light.LightSource[l].Ambient[0]);
         params[1] = FLOAT_TO_INT(ctx->Light.LightSource[l].Ambient[1]);
         params[2] = FLOAT_TO_INT(ctx->Light.LightSource[l].Ambient[2]);
         params[3] = FLOAT_TO_INT(ctx->Light.LightSource[l].Ambient[3]);
         break;
      case GL_DIFFUSE:
         params[0] = FLOAT_TO_INT(ctx->Light.LightSource[l].Diffuse[0]);
         params[1] = FLOAT_TO_INT(ctx->Light.LightSource[l].Diffuse[1]);
         params[2] = FLOAT_TO_INT(ctx->Light.LightSource[l].Diffuse[2]);
         params[3] = FLOAT_TO_INT(ctx->Light.LightSource[l].Diffuse[3]);
         break;
      case GL_SPECULAR:
         params[0] = FLOAT_TO_INT(ctx->Light.LightSource[l].Specular[0]);
         params[1] = FLOAT_TO_INT(ctx->Light.LightSource[l].Specular[1]);
         params[2] = FLOAT_TO_INT(ctx->Light.LightSource[l].Specular[2]);
         params[3] = FLOAT_TO_INT(ctx->Light.LightSource[l].Specular[3]);
         break;
      case GL_POSITION:
         params[0] = (GLint) ctx->Light.LightSource[l].EyePosition[0];
         params[1] = (GLint) ctx->Light.LightSource[l].EyePosition[1];
         params[2] = (GLint) ctx->Light.LightSource[l].EyePosition[2];
         params[3] = (GLint) ctx->Light.LightSource[l].EyePosition[3];
         break;
      case GL_SPOT_DIRECTION:
         params[0] = (GLint) ctx->Light.LightSource[l].SpotDirection[0];
         params[1] = (GLint) ctx->Light.LightSource[l].SpotDirection[1];
         params[2] = (GLint) ctx->Light.LightSource[l].SpotDirection[2];
         break;
      case GL_SPOT_EXPONENT:
         params[0] = (GLint) ctx->Light.LightSource[l].SpotExponent;
         break;
      case GL_SPOT_CUTOFF:
         params[0] = (GLint) ctx->Light.LightSource[l].SpotCutoff;
         break;
      case GL_CONSTANT_ATTENUATION:
         params[0] = (GLint) ctx->Light.LightSource[l].ConstantAttenuation;
         break;
      case GL_LINEAR_ATTENUATION:
         params[0] = (GLint) ctx->Light.LightSource[l].LinearAttenuation;
         break;
      case GL_QUADRATIC_ATTENUATION:
         params[0] = (GLint) ctx->Light.LightSource[l].QuadraticAttenuation;
         break;
      default:
         _mesa_error( ctx, GL_INVALID_ENUM, "glGetLightiv" );
         break;
   }
}



/**********************************************************************/
/***                        Light Model                             ***/
/**********************************************************************/


void GLAPIENTRY
_mesa_LightModelfv( GLenum pname, const GLfloat *params )
{
   GLenum newenum;
   GLboolean newbool;
   GET_CURRENT_CONTEXT(ctx);

   switch (pname) {
      case GL_LIGHT_MODEL_AMBIENT:
         if (TEST_EQ_4V( ctx->Light.Model.Ambient, params ))
	    return;
	 FLUSH_VERTICES(ctx, _NEW_LIGHT_CONSTANTS, GL_LIGHTING_BIT);
         COPY_4V( ctx->Light.Model.Ambient, params );
         break;
      case GL_LIGHT_MODEL_LOCAL_VIEWER:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_pname;
         newbool = (params[0] != 0.0F);
	 if (ctx->Light.Model.LocalViewer == newbool)
	    return;
	 FLUSH_VERTICES(ctx, _NEW_LIGHT_CONSTANTS | _NEW_FF_VERT_PROGRAM,
                        GL_LIGHTING_BIT);
	 ctx->Light.Model.LocalViewer = newbool;
         break;
      case GL_LIGHT_MODEL_TWO_SIDE:
         newbool = (params[0] != 0.0F);
	 if (ctx->Light.Model.TwoSide == newbool)
	    return;
	 FLUSH_VERTICES(ctx, _NEW_LIGHT_CONSTANTS | _NEW_FF_VERT_PROGRAM |
                        _NEW_LIGHT_STATE, GL_LIGHTING_BIT);
	 ctx->Light.Model.TwoSide = newbool;
         break;
      case GL_LIGHT_MODEL_COLOR_CONTROL:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_pname;
         if (params[0] == (GLfloat) GL_SINGLE_COLOR)
	    newenum = GL_SINGLE_COLOR;
         else if (params[0] == (GLfloat) GL_SEPARATE_SPECULAR_COLOR)
	    newenum = GL_SEPARATE_SPECULAR_COLOR;
	 else {
            _mesa_error( ctx, GL_INVALID_ENUM, "glLightModel(param=0x0%x)",
                         (GLint) params[0] );
	    return;
         }
	 if (ctx->Light.Model.ColorControl == newenum)
	    return;
	 FLUSH_VERTICES(ctx, _NEW_LIGHT_CONSTANTS | _NEW_FF_VERT_PROGRAM |
                        _NEW_FF_FRAG_PROGRAM, GL_LIGHTING_BIT);
	 ctx->Light.Model.ColorControl = newenum;
         break;
      default:
         goto invalid_pname;
   }

   return;

invalid_pname:
   _mesa_error( ctx, GL_INVALID_ENUM, "glLightModel(pname=0x%x)", pname );
   return;
}


void GLAPIENTRY
_mesa_LightModeliv( GLenum pname, const GLint *params )
{
   GLfloat fparam[4];

   switch (pname) {
      case GL_LIGHT_MODEL_AMBIENT:
         fparam[0] = INT_TO_FLOAT( params[0] );
         fparam[1] = INT_TO_FLOAT( params[1] );
         fparam[2] = INT_TO_FLOAT( params[2] );
         fparam[3] = INT_TO_FLOAT( params[3] );
         break;
      case GL_LIGHT_MODEL_LOCAL_VIEWER:
      case GL_LIGHT_MODEL_TWO_SIDE:
      case GL_LIGHT_MODEL_COLOR_CONTROL:
         fparam[0] = (GLfloat) params[0];
         break;
      default:
         /* Error will be caught later in gl_LightModelfv */
         ASSIGN_4V(fparam, 0.0F, 0.0F, 0.0F, 0.0F);
   }
   _mesa_LightModelfv( pname, fparam );
}


void GLAPIENTRY
_mesa_LightModeli( GLenum pname, GLint param )
{
   GLint iparam[4];
   iparam[0] = param;
   iparam[1] = iparam[2] = iparam[3] = 0;
   _mesa_LightModeliv( pname, iparam );
}


void GLAPIENTRY
_mesa_LightModelf( GLenum pname, GLfloat param )
{
   GLfloat fparam[4];
   fparam[0] = param;
   fparam[1] = fparam[2] = fparam[3] = 0.0F;
   _mesa_LightModelfv( pname, fparam );
}



/********** MATERIAL **********/


/*
 * Given a face and pname value (ala glColorMaterial), compute a bitmask
 * of the targeted material values.
 */
GLuint
_mesa_material_bitmask( struct gl_context *ctx, GLenum face, GLenum pname,
                        GLuint legal, const char *where )
{
   GLuint bitmask = 0;

   /* Make a bitmask indicating what material attribute(s) we're updating */
   switch (pname) {
      case GL_EMISSION:
         bitmask |= MAT_BIT_FRONT_EMISSION | MAT_BIT_BACK_EMISSION;
         break;
      case GL_AMBIENT:
         bitmask |= MAT_BIT_FRONT_AMBIENT | MAT_BIT_BACK_AMBIENT;
         break;
      case GL_DIFFUSE:
         bitmask |= MAT_BIT_FRONT_DIFFUSE | MAT_BIT_BACK_DIFFUSE;
         break;
      case GL_SPECULAR:
         bitmask |= MAT_BIT_FRONT_SPECULAR | MAT_BIT_BACK_SPECULAR;
         break;
      case GL_SHININESS:
         bitmask |= MAT_BIT_FRONT_SHININESS | MAT_BIT_BACK_SHININESS;
         break;
      case GL_AMBIENT_AND_DIFFUSE:
         bitmask |= MAT_BIT_FRONT_AMBIENT | MAT_BIT_BACK_AMBIENT;
         bitmask |= MAT_BIT_FRONT_DIFFUSE | MAT_BIT_BACK_DIFFUSE;
         break;
      case GL_COLOR_INDEXES:
         bitmask |= MAT_BIT_FRONT_INDEXES  | MAT_BIT_BACK_INDEXES;
         break;
      default:
         _mesa_error( ctx, GL_INVALID_ENUM, "%s", where );
         return 0;
   }

   if (face==GL_FRONT) {
      bitmask &= FRONT_MATERIAL_BITS;
   }
   else if (face==GL_BACK) {
      bitmask &= BACK_MATERIAL_BITS;
   }
   else if (face != GL_FRONT_AND_BACK) {
      _mesa_error( ctx, GL_INVALID_ENUM, "%s", where );
      return 0;
   }

   if (bitmask & ~legal) {
      _mesa_error( ctx, GL_INVALID_ENUM, "%s", where );
      return 0;
   }

   return bitmask;
}



/* Update derived values following a change in ctx->Light.Material
 */
void
_mesa_update_material( struct gl_context *ctx, GLuint bitmask )
{
   GLfloat (*mat)[4] = ctx->Light.Material.Attrib;

   if (MESA_VERBOSE & VERBOSE_MATERIAL)
      _mesa_debug(ctx, "_mesa_update_material, mask 0x%x\n", bitmask);

   if (!bitmask)
      return;

   /* update material ambience */
   if (bitmask & MAT_BIT_FRONT_AMBIENT) {
      GLbitfield mask = ctx->Light._EnabledLights;
      while (mask) {
         const int i = u_bit_scan(&mask);
         struct gl_light *light = &ctx->Light.Light[i];
         struct gl_light_uniforms *lu = &ctx->Light.LightSource[i];
         SCALE_3V( light->_MatAmbient[0], lu->Ambient,
		   mat[MAT_ATTRIB_FRONT_AMBIENT]);
      }
   }

   if (bitmask & MAT_BIT_BACK_AMBIENT) {
      GLbitfield mask = ctx->Light._EnabledLights;
      while (mask) {
         const int i = u_bit_scan(&mask);
         struct gl_light *light = &ctx->Light.Light[i];
         struct gl_light_uniforms *lu = &ctx->Light.LightSource[i];
         SCALE_3V( light->_MatAmbient[1], lu->Ambient,
		   mat[MAT_ATTRIB_BACK_AMBIENT]);
      }
   }

   /* update BaseColor = emission + scene's ambience * material's ambience */
   if (bitmask & (MAT_BIT_FRONT_EMISSION | MAT_BIT_FRONT_AMBIENT)) {
      COPY_3V( ctx->Light._BaseColor[0], mat[MAT_ATTRIB_FRONT_EMISSION] );
      ACC_SCALE_3V( ctx->Light._BaseColor[0], mat[MAT_ATTRIB_FRONT_AMBIENT],
		    ctx->Light.Model.Ambient );
   }

   if (bitmask & (MAT_BIT_BACK_EMISSION | MAT_BIT_BACK_AMBIENT)) {
      COPY_3V( ctx->Light._BaseColor[1], mat[MAT_ATTRIB_BACK_EMISSION] );
      ACC_SCALE_3V( ctx->Light._BaseColor[1], mat[MAT_ATTRIB_BACK_AMBIENT],
		    ctx->Light.Model.Ambient );
   }

   /* update material diffuse values */
   if (bitmask & MAT_BIT_FRONT_DIFFUSE) {
      GLbitfield mask = ctx->Light._EnabledLights;
      while (mask) {
         const int i = u_bit_scan(&mask);
         struct gl_light *light = &ctx->Light.Light[i];
         struct gl_light_uniforms *lu = &ctx->Light.LightSource[i];
	 SCALE_3V( light->_MatDiffuse[0], lu->Diffuse,
		   mat[MAT_ATTRIB_FRONT_DIFFUSE] );
      }
   }

   if (bitmask & MAT_BIT_BACK_DIFFUSE) {
      GLbitfield mask = ctx->Light._EnabledLights;
      while (mask) {
         const int i = u_bit_scan(&mask);
         struct gl_light *light = &ctx->Light.Light[i];
         struct gl_light_uniforms *lu = &ctx->Light.LightSource[i];
	 SCALE_3V( light->_MatDiffuse[1], lu->Diffuse,
		   mat[MAT_ATTRIB_BACK_DIFFUSE] );
      }
   }

   /* update material specular values */
   if (bitmask & MAT_BIT_FRONT_SPECULAR) {
      GLbitfield mask = ctx->Light._EnabledLights;
      while (mask) {
         const int i = u_bit_scan(&mask);
         struct gl_light *light = &ctx->Light.Light[i];
         struct gl_light_uniforms *lu = &ctx->Light.LightSource[i];
	 SCALE_3V( light->_MatSpecular[0], lu->Specular,
		   mat[MAT_ATTRIB_FRONT_SPECULAR]);
      }
   }

   if (bitmask & MAT_BIT_BACK_SPECULAR) {
      GLbitfield mask = ctx->Light._EnabledLights;
      while (mask) {
         const int i = u_bit_scan(&mask);
         struct gl_light *light = &ctx->Light.Light[i];
         struct gl_light_uniforms *lu = &ctx->Light.LightSource[i];
	 SCALE_3V( light->_MatSpecular[1], lu->Specular,
		   mat[MAT_ATTRIB_BACK_SPECULAR]);
      }
   }
}


/*
 * Update the current materials from the given rgba color
 * according to the bitmask in _ColorMaterialBitmask, which is
 * set by glColorMaterial().
 */
void
_mesa_update_color_material( struct gl_context *ctx, const GLfloat color[4] )
{
   GLbitfield bitmask = ctx->Light._ColorMaterialBitmask;
   struct gl_material *mat = &ctx->Light.Material;

   while (bitmask) {
      const int i = u_bit_scan(&bitmask);

      if (memcmp(mat->Attrib[i], color, sizeof(mat->Attrib[i]))) {
         COPY_4FV(mat->Attrib[i], color);
         ctx->NewState |= _NEW_MATERIAL;
      }
   }
}


void GLAPIENTRY
_mesa_ColorMaterial( GLenum face, GLenum mode )
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint bitmask;
   GLuint legal = (MAT_BIT_FRONT_EMISSION | MAT_BIT_BACK_EMISSION |
		   MAT_BIT_FRONT_SPECULAR | MAT_BIT_BACK_SPECULAR |
		   MAT_BIT_FRONT_DIFFUSE  | MAT_BIT_BACK_DIFFUSE  |
		   MAT_BIT_FRONT_AMBIENT  | MAT_BIT_BACK_AMBIENT);

   if (MESA_VERBOSE&VERBOSE_API)
      _mesa_debug(ctx, "glColorMaterial %s %s\n",
                  _mesa_enum_to_string(face),
                  _mesa_enum_to_string(mode));

   bitmask = _mesa_material_bitmask(ctx, face, mode, legal, "glColorMaterial");
   if (bitmask == 0)
      return; /* error was recorded */

   if (ctx->Light._ColorMaterialBitmask == bitmask &&
       ctx->Light.ColorMaterialFace == face &&
       ctx->Light.ColorMaterialMode == mode)
      return;

   FLUSH_VERTICES(ctx, 0, GL_LIGHTING_BIT);
   ctx->Light._ColorMaterialBitmask = bitmask;
   ctx->Light.ColorMaterialFace = face;
   ctx->Light.ColorMaterialMode = mode;

   if (ctx->Light.ColorMaterialEnabled) {
      /* Used by fixed-func vertex program. */
      FLUSH_CURRENT(ctx, _NEW_FF_VERT_PROGRAM);
      _mesa_update_color_material(ctx,ctx->Current.Attrib[VERT_ATTRIB_COLOR0]);
   }
}


void GLAPIENTRY
_mesa_GetMaterialfv( GLenum face, GLenum pname, GLfloat *params )
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint f;
   GLfloat (*mat)[4] = ctx->Light.Material.Attrib;

   FLUSH_VERTICES(ctx, 0, 0); /* update materials */
   FLUSH_CURRENT(ctx, 0); /* update ctx->Light.Material from vertex buffer */

   if (face==GL_FRONT) {
      f = 0;
   }
   else if (face==GL_BACK) {
      f = 1;
   }
   else {
      _mesa_error( ctx, GL_INVALID_ENUM, "glGetMaterialfv(face)" );
      return;
   }

   switch (pname) {
      case GL_AMBIENT:
         COPY_4FV( params, mat[MAT_ATTRIB_AMBIENT(f)] );
         break;
      case GL_DIFFUSE:
         COPY_4FV( params, mat[MAT_ATTRIB_DIFFUSE(f)] );
	 break;
      case GL_SPECULAR:
         COPY_4FV( params, mat[MAT_ATTRIB_SPECULAR(f)] );
	 break;
      case GL_EMISSION:
	 COPY_4FV( params, mat[MAT_ATTRIB_EMISSION(f)] );
	 break;
      case GL_SHININESS:
	 *params = mat[MAT_ATTRIB_SHININESS(f)][0];
	 break;
      case GL_COLOR_INDEXES:
         if (ctx->API != API_OPENGL_COMPAT) {
            _mesa_error( ctx, GL_INVALID_ENUM, "glGetMaterialfv(pname)" );
            return;
         }
	 params[0] = mat[MAT_ATTRIB_INDEXES(f)][0];
	 params[1] = mat[MAT_ATTRIB_INDEXES(f)][1];
	 params[2] = mat[MAT_ATTRIB_INDEXES(f)][2];
	 break;
      default:
         _mesa_error( ctx, GL_INVALID_ENUM, "glGetMaterialfv(pname)" );
   }
}


void GLAPIENTRY
_mesa_GetMaterialiv( GLenum face, GLenum pname, GLint *params )
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint f;
   GLfloat (*mat)[4] = ctx->Light.Material.Attrib;

   assert(_mesa_is_desktop_gl_compat(ctx));

   FLUSH_VERTICES(ctx, 0, 0); /* update materials */
   FLUSH_CURRENT(ctx, 0); /* update ctx->Light.Material from vertex buffer */

   if (face==GL_FRONT) {
      f = 0;
   }
   else if (face==GL_BACK) {
      f = 1;
   }
   else {
      _mesa_error( ctx, GL_INVALID_ENUM, "glGetMaterialiv(face)" );
      return;
   }
   switch (pname) {
      case GL_AMBIENT:
         params[0] = FLOAT_TO_INT( mat[MAT_ATTRIB_AMBIENT(f)][0] );
         params[1] = FLOAT_TO_INT( mat[MAT_ATTRIB_AMBIENT(f)][1] );
         params[2] = FLOAT_TO_INT( mat[MAT_ATTRIB_AMBIENT(f)][2] );
         params[3] = FLOAT_TO_INT( mat[MAT_ATTRIB_AMBIENT(f)][3] );
         break;
      case GL_DIFFUSE:
         params[0] = FLOAT_TO_INT( mat[MAT_ATTRIB_DIFFUSE(f)][0] );
         params[1] = FLOAT_TO_INT( mat[MAT_ATTRIB_DIFFUSE(f)][1] );
         params[2] = FLOAT_TO_INT( mat[MAT_ATTRIB_DIFFUSE(f)][2] );
         params[3] = FLOAT_TO_INT( mat[MAT_ATTRIB_DIFFUSE(f)][3] );
	 break;
      case GL_SPECULAR:
         params[0] = FLOAT_TO_INT( mat[MAT_ATTRIB_SPECULAR(f)][0] );
         params[1] = FLOAT_TO_INT( mat[MAT_ATTRIB_SPECULAR(f)][1] );
         params[2] = FLOAT_TO_INT( mat[MAT_ATTRIB_SPECULAR(f)][2] );
         params[3] = FLOAT_TO_INT( mat[MAT_ATTRIB_SPECULAR(f)][3] );
	 break;
      case GL_EMISSION:
         params[0] = FLOAT_TO_INT( mat[MAT_ATTRIB_EMISSION(f)][0] );
         params[1] = FLOAT_TO_INT( mat[MAT_ATTRIB_EMISSION(f)][1] );
         params[2] = FLOAT_TO_INT( mat[MAT_ATTRIB_EMISSION(f)][2] );
         params[3] = FLOAT_TO_INT( mat[MAT_ATTRIB_EMISSION(f)][3] );
	 break;
      case GL_SHININESS:
         *params = lroundf( mat[MAT_ATTRIB_SHININESS(f)][0] );
	 break;
      case GL_COLOR_INDEXES:
	 params[0] = lroundf( mat[MAT_ATTRIB_INDEXES(f)][0] );
	 params[1] = lroundf( mat[MAT_ATTRIB_INDEXES(f)][1] );
	 params[2] = lroundf( mat[MAT_ATTRIB_INDEXES(f)][2] );
	 break;
      default:
         _mesa_error( ctx, GL_INVALID_ENUM, "glGetMaterialfv(pname)" );
   }
}



/**
 * Examine current lighting parameters to determine if the optimized lighting
 * function can be used.
 * Also, precompute some lighting values such as the products of light
 * source and material ambient, diffuse and specular coefficients.
 */
GLbitfield
_mesa_update_lighting( struct gl_context *ctx )
{
   GLbitfield flags = 0;
   bool old_need_eye_coords = ctx->Light._NeedEyeCoords;
   ctx->Light._NeedEyeCoords = GL_FALSE;

   if (!ctx->Light.Enabled) {
      return old_need_eye_coords != ctx->Light._NeedEyeCoords ?
                                       _NEW_TNL_SPACES : 0;
   }

   GLbitfield mask = ctx->Light._EnabledLights;
   while (mask) {
      const int i = u_bit_scan(&mask);
      struct gl_light *light = &ctx->Light.Light[i];
      flags |= light->_Flags;
   }

   ctx->Light._NeedVertices =
      ((flags & (LIGHT_POSITIONAL|LIGHT_SPOT)) ||
       ctx->Light.Model.ColorControl == GL_SEPARATE_SPECULAR_COLOR ||
       ctx->Light.Model.LocalViewer);

   ctx->Light._NeedEyeCoords = ((flags & LIGHT_POSITIONAL) ||
				ctx->Light.Model.LocalViewer);

   /* XXX: This test is overkill & needs to be fixed both for software and
    * hardware t&l drivers.  The above should be sufficient & should
    * be tested to verify this.
    */
   if (ctx->Light._NeedVertices)
      ctx->Light._NeedEyeCoords = GL_TRUE;

   return old_need_eye_coords != ctx->Light._NeedEyeCoords ?
                                    _NEW_TNL_SPACES : 0;
}

void
_mesa_update_light_materials(struct gl_context *ctx)
{
   /* Precompute some shading values.  Although we reference
    * Light.Material here, we can get away without flushing
    * FLUSH_UPDATE_CURRENT, as when any outstanding material changes
    * are flushed, they will update the derived state at that time.
    */
   if (ctx->Light.Model.TwoSide)
      _mesa_update_material(ctx,
			    MAT_BIT_FRONT_EMISSION |
			    MAT_BIT_FRONT_AMBIENT |
			    MAT_BIT_FRONT_DIFFUSE |
			    MAT_BIT_FRONT_SPECULAR |
			    MAT_BIT_BACK_EMISSION |
			    MAT_BIT_BACK_AMBIENT |
			    MAT_BIT_BACK_DIFFUSE |
			    MAT_BIT_BACK_SPECULAR);
   else
      _mesa_update_material(ctx,
			    MAT_BIT_FRONT_EMISSION |
			    MAT_BIT_FRONT_AMBIENT |
			    MAT_BIT_FRONT_DIFFUSE |
			    MAT_BIT_FRONT_SPECULAR);
}


/**
 * Update state derived from light position, spot direction.
 * Called upon:
 *   _NEW_MODELVIEW
 *   _NEW_LIGHT_CONSTANTS
 *   _TNL_NEW_NEED_EYE_COORDS
 *
 * Update on (_NEW_MODELVIEW | _NEW_LIGHT_CONSTANTS) when lighting is enabled.
 * Also update on lighting space changes.
 */
static void
compute_light_positions( struct gl_context *ctx )
{
   static const GLfloat eye_z[3] = { 0, 0, 1 };

   if (!ctx->Light.Enabled)
      return;

   if (ctx->_NeedEyeCoords) {
      COPY_3V( ctx->_EyeZDir, eye_z );
   }
   else {
      TRANSFORM_NORMAL( ctx->_EyeZDir, eye_z, ctx->ModelviewMatrixStack.Top->m );
   }

   GLbitfield mask = ctx->Light._EnabledLights;
   while (mask) {
      const int i = u_bit_scan(&mask);
      struct gl_light *light = &ctx->Light.Light[i];
      struct gl_light_uniforms *lu = &ctx->Light.LightSource[i];

      if (ctx->_NeedEyeCoords) {
         /* _Position is in eye coordinate space */
	 COPY_4FV( light->_Position, lu->EyePosition );
      }
      else {
         /* _Position is in object coordinate space */
	 TRANSFORM_POINT( light->_Position, ctx->ModelviewMatrixStack.Top->inv,
			  lu->EyePosition );
      }

      if (!(light->_Flags & LIGHT_POSITIONAL)) {
	 /* VP (VP) = Normalize( Position ) */
	 COPY_3V( light->_VP_inf_norm, light->_Position );
	 NORMALIZE_3FV( light->_VP_inf_norm );

	 if (!ctx->Light.Model.LocalViewer) {
	    /* _h_inf_norm = Normalize( V_to_P + <0,0,1> ) */
	    ADD_3V( light->_h_inf_norm, light->_VP_inf_norm, ctx->_EyeZDir);
	    NORMALIZE_3FV( light->_h_inf_norm );
	 }
	 light->_VP_inf_spot_attenuation = 1.0;
      }
      else {
         /* positional light w/ homogeneous coordinate, divide by W */
         GLfloat wInv = 1.0F / light->_Position[3];
         light->_Position[0] *= wInv;
         light->_Position[1] *= wInv;
         light->_Position[2] *= wInv;
      }

      if (light->_Flags & LIGHT_SPOT) {
         /* Note: we normalize the spot direction now */

	 if (ctx->_NeedEyeCoords) {
	    COPY_3V( light->_NormSpotDirection, lu->SpotDirection );
            NORMALIZE_3FV( light->_NormSpotDirection );
	 }
         else {
            GLfloat spotDir[3];
            COPY_3V(spotDir, lu->SpotDirection);
            NORMALIZE_3FV(spotDir);
	    TRANSFORM_NORMAL( light->_NormSpotDirection,
			      spotDir,
			      ctx->ModelviewMatrixStack.Top->m);
	 }

	 NORMALIZE_3FV( light->_NormSpotDirection );

	 if (!(light->_Flags & LIGHT_POSITIONAL)) {
	    GLfloat PV_dot_dir = - DOT3(light->_VP_inf_norm,
					light->_NormSpotDirection);

	    if (PV_dot_dir > lu->_CosCutoff) {
	       light->_VP_inf_spot_attenuation =
                  powf(PV_dot_dir, lu->SpotExponent);
	    }
	    else {
	       light->_VP_inf_spot_attenuation = 0;
            }
	 }
      }
   }
}



static void
update_modelview_scale( struct gl_context *ctx )
{
   ctx->_ModelViewInvScale = 1.0F;
   ctx->_ModelViewInvScaleEyespace = 1.0F;
   if (!_math_matrix_is_length_preserving(ctx->ModelviewMatrixStack.Top)) {
      const GLfloat *m = ctx->ModelviewMatrixStack.Top->inv;
      GLfloat f = m[2] * m[2] + m[6] * m[6] + m[10] * m[10];
      if (f < 1e-12f) f = 1.0f;
      if (ctx->_NeedEyeCoords)
	 ctx->_ModelViewInvScale = 1.0f / sqrtf(f);
      else
	 ctx->_ModelViewInvScale = sqrtf(f);
      ctx->_ModelViewInvScaleEyespace = 1.0f / sqrtf(f);
   }
}


/**
 * Bring up to date any state that relies on _NeedEyeCoords.
 *
 * Return true if ctx->_NeedEyeCoords has been changed.
 */
bool
_mesa_update_tnl_spaces( struct gl_context *ctx, GLuint new_state )
{
   const GLuint oldneedeyecoords = ctx->_NeedEyeCoords;

   (void) new_state;
   ctx->_NeedEyeCoords = GL_FALSE;

   if ((ctx->Texture._GenFlags & TEXGEN_NEED_EYE_COORD) ||
       ctx->Point._Attenuated ||
       ctx->Light._NeedEyeCoords)
      ctx->_NeedEyeCoords = GL_TRUE;

   if (ctx->Light.Enabled &&
       !_math_matrix_is_length_preserving(ctx->ModelviewMatrixStack.Top))
      ctx->_NeedEyeCoords = GL_TRUE;

   /* Check if the truth-value interpretations of the bitfields have
    * changed:
    */
   if (oldneedeyecoords != ctx->_NeedEyeCoords) {
      /* Recalculate all state that depends on _NeedEyeCoords.
       */
      update_modelview_scale(ctx);
      compute_light_positions( ctx );

      return true;
   }
   else {
      GLuint new_state2 = ctx->NewState;

      /* Recalculate that same state only if it has been invalidated
       * by other statechanges.
       */
      if (new_state2 & _NEW_MODELVIEW)
	 update_modelview_scale(ctx);

      if (new_state2 & (_NEW_LIGHT_CONSTANTS | _NEW_MODELVIEW))
	 compute_light_positions( ctx );

      return false;
   }
}


/**********************************************************************/
/*****                      Initialization                        *****/
/**********************************************************************/

/**
 * Initialize the n-th light data structure.
 *
 * \param l pointer to the gl_light structure to be initialized.
 * \param n number of the light.
 * \note The defaults for light 0 are different than the other lights.
 */
static void
init_light( struct gl_light *l, struct gl_light_uniforms *lu, GLuint n )
{
   ASSIGN_4V( lu->Ambient, 0.0, 0.0, 0.0, 1.0 );
   if (n==0) {
      ASSIGN_4V( lu->Diffuse, 1.0, 1.0, 1.0, 1.0 );
      ASSIGN_4V( lu->Specular, 1.0, 1.0, 1.0, 1.0 );
   }
   else {
      ASSIGN_4V( lu->Diffuse, 0.0, 0.0, 0.0, 1.0 );
      ASSIGN_4V( lu->Specular, 0.0, 0.0, 0.0, 1.0 );
   }
   ASSIGN_4V( lu->EyePosition, 0.0, 0.0, 1.0, 0.0 );
   ASSIGN_3V( lu->SpotDirection, 0.0, 0.0, -1.0 );
   lu->SpotExponent = 0.0;
   lu->SpotCutoff = 180.0;
   lu->_CosCutoff = 0.0;		/* KW: -ve values not admitted */
   lu->ConstantAttenuation = 1.0;
   lu->LinearAttenuation = 0.0;
   lu->QuadraticAttenuation = 0.0;
   l->Enabled = GL_FALSE;
}


/**
 * Initialize the light model data structure.
 *
 * \param lm pointer to the gl_lightmodel structure to be initialized.
 */
static void
init_lightmodel( struct gl_lightmodel *lm )
{
   ASSIGN_4V( lm->Ambient, 0.2F, 0.2F, 0.2F, 1.0F );
   lm->LocalViewer = GL_FALSE;
   lm->TwoSide = GL_FALSE;
   lm->ColorControl = GL_SINGLE_COLOR;
}


/**
 * Initialize the material data structure.
 *
 * \param m pointer to the gl_material structure to be initialized.
 */
static void
init_material( struct gl_material *m )
{
   ASSIGN_4V( m->Attrib[MAT_ATTRIB_FRONT_AMBIENT],  0.2F, 0.2F, 0.2F, 1.0F );
   ASSIGN_4V( m->Attrib[MAT_ATTRIB_FRONT_DIFFUSE],  0.8F, 0.8F, 0.8F, 1.0F );
   ASSIGN_4V( m->Attrib[MAT_ATTRIB_FRONT_SPECULAR], 0.0F, 0.0F, 0.0F, 1.0F );
   ASSIGN_4V( m->Attrib[MAT_ATTRIB_FRONT_EMISSION], 0.0F, 0.0F, 0.0F, 1.0F );
   ASSIGN_4V( m->Attrib[MAT_ATTRIB_FRONT_SHININESS], 0.0F, 0.0F, 0.0F, 0.0F );
   ASSIGN_4V( m->Attrib[MAT_ATTRIB_FRONT_INDEXES], 0.0F, 1.0F, 1.0F, 0.0F );

   ASSIGN_4V( m->Attrib[MAT_ATTRIB_BACK_AMBIENT],  0.2F, 0.2F, 0.2F, 1.0F );
   ASSIGN_4V( m->Attrib[MAT_ATTRIB_BACK_DIFFUSE],  0.8F, 0.8F, 0.8F, 1.0F );
   ASSIGN_4V( m->Attrib[MAT_ATTRIB_BACK_SPECULAR], 0.0F, 0.0F, 0.0F, 1.0F );
   ASSIGN_4V( m->Attrib[MAT_ATTRIB_BACK_EMISSION], 0.0F, 0.0F, 0.0F, 1.0F );
   ASSIGN_4V( m->Attrib[MAT_ATTRIB_BACK_SHININESS], 0.0F, 0.0F, 0.0F, 0.0F );
   ASSIGN_4V( m->Attrib[MAT_ATTRIB_BACK_INDEXES], 0.0F, 1.0F, 1.0F, 0.0F );
}


/**
 * Initialize all lighting state for the given context.
 */
void
_mesa_init_lighting( struct gl_context *ctx )
{
   GLuint i;

   /* Lighting group */
   ctx->Light._EnabledLights = 0;
   for (i = 0; i < MAX_LIGHTS; i++) {
      init_light( &ctx->Light.Light[i], &ctx->Light.LightSource[i], i );
   }

   init_lightmodel( &ctx->Light.Model );
   init_material( &ctx->Light.Material );
   ctx->Light.ShadeModel = GL_SMOOTH;
   ctx->Light.ProvokingVertex = GL_LAST_VERTEX_CONVENTION_EXT;
   ctx->Light.Enabled = GL_FALSE;
   ctx->Light.ColorMaterialFace = GL_FRONT_AND_BACK;
   ctx->Light.ColorMaterialMode = GL_AMBIENT_AND_DIFFUSE;
   ctx->Light._ColorMaterialBitmask = _mesa_material_bitmask( ctx,
                                               GL_FRONT_AND_BACK,
                                               GL_AMBIENT_AND_DIFFUSE, ~0,
                                               NULL );

   ctx->Light.ColorMaterialEnabled = GL_FALSE;
   ctx->Light.ClampVertexColor = _mesa_is_desktop_gl_compat(ctx);
   ctx->Light._ClampVertexColor = _mesa_is_desktop_gl_compat(ctx);

   /* Miscellaneous */
   ctx->Light._NeedEyeCoords = GL_FALSE;
   ctx->_NeedEyeCoords = GL_FALSE;
   ctx->_ModelViewInvScale = 1.0;
   ctx->_ModelViewInvScaleEyespace = 1.0;
}
