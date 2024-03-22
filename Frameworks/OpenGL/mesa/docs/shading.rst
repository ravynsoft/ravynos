Shading Language
================

This page describes the features and status of Mesa's support for the
`OpenGL Shading Language <https://www.khronos.org/opengl/wiki/OpenGL_Shading_Language>`__.

.. _envvars:

Environment Variables
---------------------

The **MESA_GLSL** environment variable can be set to a comma-separated
list of keywords to control some aspects of the GLSL compiler and shader
execution. These are generally used for debugging.

-  **dump** - print GLSL shader code, IR, and NIR to stdout at link time
-  **source** - print GLSL shader code to stdout at link time
-  **log** - log all GLSL shaders to files. The filenames will be
   "shader_X.vert" or "shader_X.frag" where X the shader ID.
-  **cache_info** - print debug information about shader cache
-  **cache_fb** - force cached shaders to be ignored and do a full
   recompile via the fallback path
-  **uniform** - print message to stdout when glUniform is called
-  **nopvert** - force vertex shaders to be a simple shader that just
   transforms the vertex position with ftransform() and passes through
   the color and texcoord[0] attributes.
-  **nopfrag** - force fragment shader to be a simple shader that passes
   through the color attribute.
-  **useprog** - log glUseProgram calls to stderr
-  **errors** - GLSL compilation and link errors will be reported to
   stderr.

Example: export MESA_GLSL=dump,nopt

.. _replacement:

Experimenting with Shader Replacements
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Shaders can be dumped and replaced on runtime for debugging purposes.
This is controlled via following environment variables:

-  **MESA_SHADER_DUMP_PATH** - path where shader sources are dumped
-  **MESA_SHADER_READ_PATH** - path where replacement shaders are read

Note, path set must exist before running for dumping or replacing to
work. When both are set, these paths should be different so the dumped
shaders do not clobber the replacement shaders. Also, the filenames of
the replacement shaders should match the filenames of the corresponding
dumped shaders.

.. _capture:

Capturing Shaders
~~~~~~~~~~~~~~~~~

Setting **MESA_SHADER_CAPTURE_PATH** to a directory will cause the
compiler to write ``.shader_test`` files for use with
`shader-db <https://gitlab.freedesktop.org/mesa/shader-db>`__, a tool
which compiler developers can use to gather statistics about shaders
(instructions, cycles, memory accesses, and so on).

Notably, this captures linked GLSL shaders - with all stages together -
as well as ARB programs.

GLSL Version
------------

The GLSL compiler currently supports version 3.30 of the shading
language.

Several GLSL extensions are also supported:

-  :ext:`GL_ARB_draw_buffers`
-  :ext:`GL_ARB_fragment_coord_conventions`
-  :ext:`GL_ARB_shader_bit_encoding`

Unsupported Features
--------------------

XXX update this section

The following features of the shading language are not yet fully
supported in Mesa:

-  Linking of multiple shaders does not always work. Currently, linking
   is implemented through shader concatenation and re-compiling. This
   doesn't always work because of some #pragma and preprocessor issues.
-  The gl_Color and gl_SecondaryColor varying vars are interpolated
   without perspective correction

All other major features of the shading language should function.

Implementation Notes
--------------------

-  Shading language programs are compiled into low-level programs very
   similar to those of :ext:`GL_ARB_vertex_program` /
   :ext:`GL_ARB_fragment_program`.
-  All vector types (vec2, vec3, vec4, bvec2, etc) currently occupy full
   float[4] registers.
-  Float constants and variables are packed so that up to four floats
   can occupy one program parameter/register.
-  All function calls are inlined.
-  Shaders which use too many registers will not compile.
-  The quality of generated code is pretty good, register usage is fair.
-  Shader error detection and reporting of errors (InfoLog) is not very
   good yet.
-  The ftransform() function doesn't necessarily match the results of
   fixed-function transformation.

These issues will be addressed/resolved in the future.

Programming Hints
-----------------

-  Use the built-in library functions whenever possible. For example,
   instead of writing this:

   .. code-block:: glsl

      float x = 1.0 / sqrt(y);

   Write this:

   .. code-block:: glsl

      float x = inversesqrt(y);

Stand-alone GLSL Compiler
-------------------------

The stand-alone GLSL compiler program can be used to compile GLSL
shaders into GLSL IR code.

This tool is useful for:

-  Inspecting GLSL frontend behavior to gain insight into compilation
-  Debugging the GLSL compiler itself

After building Mesa with the ``-Dtools=glsl`` meson option, the compiler will be
installed as the binary ``glsl_compiler``.

Here's an example of using the compiler to compile a vertex shader and
emit :ext:`GL_ARB_vertex_program`-style instructions:

.. code-block:: console

       src/compiler/glsl/glsl_compiler --version XXX --dump-ast myshader.vert

Options include

-  **--dump-ast** - dump source syntax tree
-  **--dump-hir** - dump high-level IR code
-  **--dump-lir** - dump low-level IR code
-  **--dump-builder** - dump C++ ir_builder code to generate the shader's GLSL IR
-  **--link** - link shaders
-  **--just-log** - display only shader / linker info if exist, without
   any header or separator
-  **--version** - [Mandatory] define the GLSL version to use

Compiler Implementation
-----------------------

The source code for Mesa's shading language compiler is in the
``src/compiler/glsl/`` directory.

XXX provide some info about the compiler....

The final vertex and fragment programs may be interpreted in software
(see prog_execute.c) or translated into a specific hardware architecture
(see drivers/dri/i915/i915_fragprog.c for example).

Compiler Validation
-------------------

Developers working on the GLSL compiler should test frequently to avoid
regressions.

The `Piglit <https://piglit.freedesktop.org/>`__ project has many GLSL
tests.

The Mesa demos repository also has some good GLSL tests.
