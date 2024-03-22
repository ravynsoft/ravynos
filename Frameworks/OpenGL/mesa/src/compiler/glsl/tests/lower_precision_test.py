# encoding=utf-8
# Copyright © 2019 Google

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import sys
import subprocess
import tempfile
import re
from collections import namedtuple


Test = namedtuple("Test", "name source match_re")

# NOTE: This test is deprecated, please add any new tests to test_gl_lower_mediump.cpp.

TESTS = [
    Test("f32 array-of-array with const index",
         """
         #version 310 es
         precision mediump float;

         uniform float in_aoa[2][2];

         layout(location = 0) out float out_color;

         void main()
         {
                 out_color = in_aoa[0][0] / in_aoa[1][1];
         }
         """,
         r'\(expression +float16_t +/'),
    Test("i32 array-of-array with const index",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform int in_aoa[2][2];

         layout(location = 0) out highp int out_color;

         void main()
         {
                 out_color = in_aoa[0][0] / in_aoa[1][1];
         }
         """,
         r'\(expression +int16_t +/'),
    Test("u32 array-of-array with const index",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform uint in_aoa[2][2];

         layout(location = 0) out highp uint out_color;

         void main()
         {
                 out_color = in_aoa[0][0] / in_aoa[1][1];
         }
         """,
         r'\(expression +uint16_t +/'),
    Test("f32 array-of-array with uniform index",
         """
         #version 310 es
         precision mediump float;

         uniform float in_aoa[2][2];
         uniform int i0, i1;

         layout(location = 0) out float out_color;

         void main()
         {
                 out_color = in_aoa[i0][i0] / in_aoa[i1][i1];
         }
         """,
         r'\(expression +float16_t +/'),
    Test("i32 array-of-array with uniform index",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform int in_aoa[2][2];
         uniform int i0, i1;

         layout(location = 0) out highp int out_color;

         void main()
         {
                 out_color = in_aoa[i0][i0] / in_aoa[i1][i1];
         }
         """,
         r'\(expression +int16_t +/'),
    Test("u32 array-of-array with uniform index",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform uint in_aoa[2][2];
         uniform int i0, i1;

         layout(location = 0) out highp uint out_color;

         void main()
         {
                 out_color = in_aoa[i0][i0] / in_aoa[i1][i1];
         }
         """,
         r'\(expression +uint16_t +/'),
    Test("f32 function",
         """
         precision mediump float;

         uniform float a, b;

         mediump float
         get_a()
         {
                 return a;
         }

         float
         get_b()
         {
                 return b;
         }

         void main()
         {
                 gl_FragColor = vec4(get_a() / get_b());
         }
         """,
         r'\(expression +float16_t +/'),
    Test("i32 function",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform int a, b;

         mediump int
         get_a()
         {
                 return a;
         }

         int
         get_b()
         {
                 return b;
         }

         out highp int color;

         void main()
         {
                 color = get_a() / get_b();
         }
         """,
         r'\(expression +int16_t +/'),
    Test("u32 function",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform uint a, b;

         mediump uint
         get_a()
         {
                 return a;
         }

         uint
         get_b()
         {
                 return b;
         }

         out highp uint color;

         void main()
         {
                 color = get_a() / get_b();
         }
         """,
         r'\(expression +uint16_t +/'),
    Test("f32 function mediump args",
         """
         precision mediump float;

         uniform float a, b;

         mediump float
         do_div(float x, float y)
         {
                 return x / y;
         }

         void main()
         {
                 gl_FragColor = vec4(do_div(a, b));
         }
         """,
         r'\(expression +float16_t +/'),
    Test("i32 function mediump args",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform int a, b;

         mediump int
         do_div(int x, int y)
         {
                 return x / y;
         }

         out highp int color;

         void main()
         {
                 color = do_div(a, b);
         }
         """,
         r'\(expression +int16_t +/'),
    Test("u32 function mediump args",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform uint a, b;

         mediump uint
         do_div(uint x, uint y)
         {
                 return x / y;
         }

         out highp uint color;

         void main()
         {
                 color = do_div(a, b);
         }
         """,
         r'\(expression +uint16_t +/'),
    Test("f32 function highp args",
         """
         precision mediump float;

         uniform float a, b;

         mediump float
         do_div(highp float x, highp float y)
         {
                 return x / y;
         }

         void main()
         {
                 gl_FragColor = vec4(do_div(a, b));
         }
         """,
         r'\(expression +float +/'),
    Test("i32 function highp args",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform int a, b;

         mediump int
         do_div(highp int x, highp int y)
         {
                 return x / y;
         }

         out highp int color;

         void main()
         {
                  color = do_div(a, b);
         }
         """,
         r'\(expression +int +/'),
    Test("u32 function highp args",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform uint a, b;

         mediump uint
         do_div(highp uint x, highp uint y)
         {
                 return x / y;
         }

         out highp uint color;

         void main()
         {
                  color = do_div(a, b);
         }
         """,
         r'\(expression +uint +/'),

    Test("f32 if",
         """
         precision mediump float;

         uniform float a, b;

         void
         main()
         {
                 if (a / b < 0.31)
                         gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
                 else
                         gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
         }
         """,
         r'\(expression +float16_t +/'),
    Test("i32 if",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform int a, b;

         out vec4 color;

         void
         main()
         {
                 if (a / b < 10)
                         color = vec4(0.0, 1.0, 0.0, 1.0);
                 else
                         color = vec4(1.0, 0.0, 0.0, 1.0);
         }
         """,
         r'\(expression +int16_t +/'),
    Test("u32 if",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform uint a, b;

         out vec4 color;

         void
         main()
         {
                 if (a / b < 10u)
                         color = vec4(0.0, 1.0, 0.0, 1.0);
                 else
                         color = vec4(1.0, 0.0, 0.0, 1.0);
         }
         """,
         r'\(expression +uint16_t +/'),
    Test("matrix",
         """
         precision mediump float;

         uniform vec2 a;
         uniform mat2 b;

         void main()
         {
             gl_FragColor = vec4(b * a, 0.0, 0.0);
         }
         """,
         r'\(expression +f16vec2 \* \(var_ref b\) \(var_ref a\)'),
    Test("f32 simple struct deref",
         """
         precision mediump float;

         struct simple {
                 float a, b;
         };

         uniform simple in_simple;

         void main()
         {
                 gl_FragColor = vec4(in_simple.a / in_simple.b);
         }
         """,
         r'\(expression +float16_t +/'),
    Test("i32 simple struct deref",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         struct simple {
                 int a, b;
         };

         uniform simple in_simple;

         out highp int color;

         void main()
         {
                 color = in_simple.a / in_simple.b;
         }
         """,
         r'\(expression +int16_t +/'),
    Test("u32 simple struct deref",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         struct simple {
                 uint a, b;
         };

         uniform simple in_simple;

         out highp uint color;

         void main()
         {
                 color = in_simple.a / in_simple.b;
         }
         """,
         r'\(expression +uint16_t +/'),
    Test("f32 embedded struct deref",
         """
         precision mediump float;

         struct simple {
                 float a, b;
         };

         struct embedded {
                 simple a, b;
         };

         uniform embedded in_embedded;

         void main()
         {
                 gl_FragColor = vec4(in_embedded.a.a / in_embedded.b.b);
         }
         """,
         r'\(expression +float16_t +/'),
    Test("i32 embedded struct deref",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         struct simple {
                 int a, b;
         };

         struct embedded {
                 simple a, b;
         };

         uniform embedded in_embedded;

         out highp int color;

         void main()
         {
                 color = in_embedded.a.a / in_embedded.b.b;
         }
         """,
         r'\(expression +int16_t +/'),
    Test("u32 embedded struct deref",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         struct simple {
                 uint a, b;
         };

         struct embedded {
                 simple a, b;
         };

         uniform embedded in_embedded;

         out highp uint color;

         void main()
         {
                 color = in_embedded.a.a / in_embedded.b.b;
         }
         """,
         r'\(expression +uint16_t +/'),
    Test("f32 arrayed struct deref",
         """
         precision mediump float;

         struct simple {
                 float a, b;
         };

         struct arrayed {
                 simple a[2];
         };

         uniform arrayed in_arrayed;

         void main()
         {
                 gl_FragColor = vec4(in_arrayed.a[0].a / in_arrayed.a[1].b);
         }
         """,
         r'\(expression +float16_t +/'),
    Test("i32 arrayed struct deref",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         struct simple {
                 int a, b;
         };

         struct arrayed {
                 simple a[2];
         };

         uniform arrayed in_arrayed;

         out highp int color;

         void main()
         {
                 color = in_arrayed.a[0].a / in_arrayed.a[1].b;
         }
         """,
         r'\(expression +int16_t +/'),
    Test("u32 arrayed struct deref",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         struct simple {
                 uint a, b;
         };

         struct arrayed {
                 simple a[2];
         };

         uniform arrayed in_arrayed;

         out highp uint color;

         void main()
         {
                 color = in_arrayed.a[0].a / in_arrayed.a[1].b;
         }
         """,
         r'\(expression +uint16_t +/'),
    Test("f32 mixed precision not lowered",
         """
         uniform mediump float a;
         uniform highp float b;

         void main()
         {
                 gl_FragColor = vec4(a / b);
         }
         """,
         r'\(expression +float +/'),
    Test("i32 mixed precision not lowered",
         """
         #version 310 es
         uniform mediump int a;
         uniform highp int b;

         out mediump int color;

         void main()
         {
                 color = a / b;
         }
         """,
         r'\(expression +int +/'),
    Test("u32 mixed precision not lowered",
         """
         #version 310 es
         uniform mediump uint a;
         uniform highp uint b;

         out mediump uint color;

         void main()
         {
                 color = a / b;
         }
         """,
         r'\(expression +uint +/'),
    Test("f32 sampler array",
         """
         #version 320 es
         precision mediump float;
         precision mediump int;

         uniform sampler2D tex[2];
         // highp shouldn't affect the return value of texture2D
         uniform highp vec2 coord;
         uniform float divisor;
         uniform int index;

         out highp vec4 color;

         void main()
         {
                 color = texture2D(tex[index], coord) / divisor;
         }
         """,
         r'\(expression +f16vec4 +/.*\(tex +f16vec4 +'),
    Test("f32 texture sample",
         """
         precision mediump float;

         uniform sampler2D tex;
         // highp shouldn't affect the return value of texture2D
         uniform highp vec2 coord;
         uniform float divisor;

         void main()
         {
                 gl_FragColor = texture2D(tex, coord) / divisor;
         }
         """,
         r'\(expression +f16vec4 +/.*\(tex +f16vec4 +'),
    Test("i32 texture sample",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform mediump isampler2D tex;
         // highp shouldn't affect the return value of texture
         uniform highp vec2 coord;
         uniform int divisor;

         out highp ivec4 color;

         void main()
         {
                 color = texture(tex, coord) / divisor;
         }
         """,
         r'\(expression +i16vec4 +/.*\(tex +i16vec4 +'),
    Test("u32 texture sample",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform mediump usampler2D tex;
         // highp shouldn't affect the return value of texture
         uniform highp vec2 coord;
         uniform uint divisor;

         out highp uvec4 color;

         void main()
         {
                 color = texture(tex, coord) / divisor;
         }
         """,
         r'\(expression +u16vec4 +/.*\(tex +u16vec4 +'),
    Test("f32 image array",
         """
         #version 320 es
         precision mediump float;

         layout(rgba16f) readonly uniform mediump image2D img[2];
         // highp shouldn't affect the return value of imageLoad
         uniform highp ivec2 coord;
         uniform float divisor;

         out highp vec4 color;

         void main()
         {
                 color = imageLoad(img[1], coord) / divisor;
         }
         """,
         r'\(expression +f16vec4 +/'),
    Test("f32 image load",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         layout(rgba16f) readonly uniform mediump image2D img;
         // highp shouldn't affect the return value of imageLoad
         uniform highp ivec2 coord;
         uniform float divisor;

         out highp vec4 color;

         void main()
         {
                 color = imageLoad(img, coord) / divisor;
         }
         """,
         r'\(expression +f16vec4 +/'),
    Test("i32 image load",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         layout(rgba16i) readonly uniform mediump iimage2D img;
         // highp shouldn't affect the return value of imageLoad
         uniform highp ivec2 coord;
         uniform int divisor;

         out highp ivec4 color;

         void main()
         {
                 color = imageLoad(img, coord) / divisor;
         }
         """,
         r'\(expression +i16vec4 +/'),
    Test("u32 image load",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         layout(rgba16ui) readonly uniform mediump uimage2D img;
         // highp shouldn't affect the return value of imageLoad
         uniform highp ivec2 coord;
         uniform uint divisor;

         out highp uvec4 color;

         void main()
         {
                 color = imageLoad(img, coord) / divisor;
         }
         """,
         r'\(expression +u16vec4 +/'),
    Test("f32 expression in lvalue",
         """
         uniform mediump float a, b;

         void main()
         {
                 gl_FragColor = vec4(1.0);
                 gl_FragColor[int(a / b)] = 0.5;
         }
         """,
         r'\(expression +float16_t +/'),
    Test("i32 expression in lvalue",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform mediump int a, b;

         out vec4 color;

         void main()
         {
                 color = vec4(1.0);
                 color[a / b] = 0.5;
         }
         """,
         r'\(expression +int16_t +/'),
    Test("f32 builtin with const arg",
         """
         uniform mediump float a;

         void main()
         {
                 gl_FragColor = vec4(min(a, 3.0));
         }
         """,
         r'\(expression +float16_t min'),
    Test("i32 builtin with const arg",
         """
         #version 310 es
         uniform mediump int a;

         out highp int color;

         void main()
         {
                 color = min(a, 3);
         }
         """,
         r'\(expression +int16_t min'),
    Test("u32 builtin with const arg",
         """
         #version 310 es
         uniform mediump uint a;

         out highp uint color;

         void main()
         {
                 color = min(a, 3u);
         }
         """,
         r'\(expression +uint16_t min'),
    Test("dFdx",
         """
         #version 300 es
         precision mediump float;

         in vec4 var;
         out vec4 color;

         void main()
         {
                 color = dFdx(var);
         }
         """,
         r'\(expression +f16vec4 +dFdx +\(expression +f16vec4'),
    Test("dFdy",
         """
         #version 300 es
         precision mediump float;

         in vec4 var;
         out vec4 color;

         void main()
         {
                 color = dFdy(var);
         }
         """,
         r'\(expression +f16vec4 +dFdy +\(expression +f16vec4'),
    Test("textureSize",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform mediump sampler2D tex;
         out ivec2 color;

         void main()
         {
                 color = textureSize(tex, 0) * ivec2(2);
         }
         """,
         r'expression ivec2 \* \(txs ivec2 \(var_ref tex'),
    Test("floatBitsToInt",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform float val;
         out int color;

         void main()
         {
                 color = floatBitsToInt(val + 1.0) + 1;
         }
         """,
         r'expression int bitcast_f2i \(expression float'),
    Test("floatBitsToUint",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform float val;
         out uint color;

         void main()
         {
                 color = floatBitsToUint(val + 1.0) + 1u;
         }
         """,
         r'expression uint bitcast_f2u \(expression float'),
    Test("intBitsToFloat",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform int val;
         out float color;

         void main()
         {
                 color = intBitsToFloat(val + 1) + 1.0;
         }
         """,
         r'expression float bitcast_i2f \(expression int'),
    Test("uintBitsToFloat",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform uint val;
         out float color;

         void main()
         {
                 color = uintBitsToFloat(val + 1u) + 1.0;
         }
         """,
         r'expression float bitcast_u2f \(expression uint'),
    Test("bitfieldReverse",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform int val;
         out int color;

         void main()
         {
                 color = bitfieldReverse(val + 1) + 1;
         }
         """,
         r'expression int bitfield_reverse \(expression int'),
    Test("frexp",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform float val;
         out float color;
         out int color2;

         void main()
         {
                 int y;
                 float x = frexp(val + 1.0, y);
                 color = x + 1.0;
                 color2 = y + 1;
         }
         """,
         r'expression int16_t i2imp \(expression int frexp_exp \(expression float f162f'),
    Test("ldexp",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform float val;
         uniform int exp;
         out float color;

         void main()
         {
                 color = ldexp(val + 1.0, exp + 1) + 1.0;
         }
         """,
         r'expression float ldexp \(expression float'),
    Test("uaddCarry",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform uint x, y;
         out uint color;

         void main()
         {
                 lowp uint carry;
                 color = uaddCarry(x * 2u, y * 2u, carry) * 2u;
                 color *= carry;
         }
         """,
         r'expression uint \+ \(expression uint u2u \(expression uint16_t \* \(expression uint16_t u2ump \(var_ref x\) \) \(constant uint16_t \(2\)\) \) \) \(expression uint u2u \(expression uint16_t \* \(expression uint16_t u2ump \(var_ref y'),
    Test("usubBorrow",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform uint x, y;
         out uint color;

         void main()
         {
                 lowp uint borrow;
                 color = usubBorrow(x * 2u, y * 2u, borrow) * 2u;
                 color *= borrow;
         }
         """,
         r'expression uint \- \(expression uint u2u \(expression uint16_t \* \(expression uint16_t u2ump \(var_ref x\) \) \(constant uint16_t \(2\)\) \) \) \(expression uint u2u \(expression uint16_t \* \(expression uint16_t u2ump \(var_ref y'),
    Test("imulExtended",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform int x, y;
         out int color;

         void main()
         {
                 int msb, lsb;
                 imulExtended(x + 2, y + 2, msb, lsb);
                 color = msb + lsb;
         }
         """,
         r'expression int64_t \* \(expression int'),
    Test("umulExtended",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform uint x, y;
         out uint color;

         void main()
         {
                 uint msb, lsb;
                 umulExtended(x + 2u, y + 2u, msb, lsb);
                 color = msb + lsb;
         }
         """,
         r'expression uint64_t \* \(expression uint'),
    Test("unpackUnorm2x16",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform uint val;
         out vec2 color;

         void main()
         {
                 color = unpackUnorm2x16(val + 1u) + vec2(1.0);
         }
         """,
         r'expression vec2 unpackUnorm2x16 \(expression uint'),
    Test("unpackSnorm2x16",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform uint val;
         out vec2 color;

         void main()
         {
                 color = unpackSnorm2x16(val + 1u) + vec2(1.0);
         }
         """,
         r'expression vec2 unpackSnorm2x16 \(expression uint'),
    Test("packUnorm2x16",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform vec2 val;
         out uint color;

         void main()
         {
                 color = packUnorm2x16(val + vec2(1.0)) + 1u;
         }
         """,
         r'expression uint packUnorm2x16 \(expression vec2'),
    Test("packSnorm2x16",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform vec2 val;
         out uint color;

         void main()
         {
                 color = packSnorm2x16(val + vec2(1.0)) + 1u;
         }
         """,
         r'expression uint packSnorm2x16 \(expression vec2'),
    Test("packHalf2x16",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform vec2 val;
         out uint color;

         void main()
         {
                 color = packHalf2x16(val + vec2(1.0)) + 1u;
         }
         """,
         r'expression uint packHalf2x16 \(expression vec2'),
    Test("packUnorm4x8",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform vec4 val;
         out uint color;

         void main()
         {
                 color = packUnorm4x8(val + vec4(1.0)) + 1u;
         }
         """,
         r'expression uint packUnorm4x8 \(expression vec4'),
    Test("packSnorm4x8",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform vec4 val;
         out uint color;

         void main()
         {
                 color = packSnorm4x8(val + vec4(1.0)) + 1u;
         }
         """,
         r'expression uint packSnorm4x8 \(expression vec4'),
    Test("interpolateAtCentroid",
         """
         #version 320 es
         precision mediump float;
         precision mediump int;

         in float val;
         out float color;

         void main()
         {
                 color = interpolateAtCentroid(val) + 1.0;
         }
         """,
         r'expression float16_t interpolate_at_centroid \(expression float16_t'),
    Test("interpolateAtOffset",
         """
         #version 320 es
         precision mediump float;
         precision mediump int;

         uniform highp vec2 offset;
         in float val;
         out float color;

         void main()
         {
                 color = interpolateAtOffset(val, offset) + 1.0;
         }
         """,
         r'expression float16_t interpolate_at_offset \(expression float16_t'),
    Test("interpolateAtSample",
         """
         #version 320 es
         precision mediump float;
         precision mediump int;

         uniform highp int sample_index;
         in float val;
         out float color;

         void main()
         {
                 color = interpolateAtSample(val, sample_index) + 1.0;
         }
         """,
         r'expression float16_t interpolate_at_sample \(expression float16_t'),
    Test("bitfieldExtract",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform highp int offset, bits;
         uniform int val;
         out int color;

         void main()
         {
                 color = bitfieldExtract(val, offset, bits) + 1;
         }
         """,
         r'expression int16_t bitfield_extract \(expression int16_t'),
    Test("bitfieldInsert",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform highp int offset, bits;
         uniform int val, val2;
         out int color;

         void main()
         {
                 color = bitfieldInsert(val, val2, offset, bits) + 1;
         }
         """,
         r'expression int16_t bitfield_insert \(expression int16_t'),
    Test("bitCount",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform highp int val;
         out int color;

         void main()
         {
                 color = bitCount(val) + 1;
         }
         """,
         r'expression int16_t \+ \(expression int16_t i2imp \(expression int bit_count \(var_ref val'),
    Test("findLSB",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform highp int val;
         out int color;

         void main()
         {
                 color = findLSB(val) + 1;
         }
         """,
         r'expression int16_t \+ \(expression int16_t i2imp \(expression int find_lsb \(var_ref val'),
    Test("findMSB",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform highp int val;
         out int color;

         void main()
         {
                 color = findMSB(val) + 1;
         }
         """,
         r'expression int16_t \+ \(expression int16_t i2imp \(expression int find_msb \(var_ref val'),
    Test("unpackHalf2x16",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform highp uint val;
         out vec2 color;

         void main()
         {
                 color = unpackHalf2x16(val) + vec2(1.0);
         }
         """,
         r'expression f16vec2 \+ \(expression f16vec2 f2fmp \(expression vec2 unpackHalf2x16 \(var_ref val'),
    Test("unpackUnorm4x8",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform highp uint val;
         out vec4 color;

         void main()
         {
                 color = unpackUnorm4x8(val) + vec4(1.0);
         }
         """,
         r'expression f16vec4 \+ \(expression f16vec4 f2fmp \(expression vec4 unpackUnorm4x8 \(var_ref val'),
    Test("unpackSnorm4x8",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform highp uint val;
         out vec4 color;

         void main()
         {
                 color = unpackSnorm4x8(val) + vec4(1.0);
         }
         """,
         r'expression f16vec4 \+ \(expression f16vec4 f2fmp \(expression vec4 unpackSnorm4x8 \(var_ref val'),
    Test("f32 csel",
         """
         #version 300 es
         precision mediump float;

         in vec4 var;
         out vec4 color;

         void main()
         {
                 color = (var.x > var.y) ? var : vec4(10.0);
         }
         """,
         r'\(expression f16vec4 f2fmp \(constant vec4 \(10'),
    Test("i32 csel",
         """
         #version 310 es
         precision mediump int;

         in flat ivec4 var;
         out ivec4 color;

         void main()
         {
                 color = (var.x > var.y) ? var : ivec4(10);
         }
         """,
         r'\(expression i16vec4 i2imp \(constant ivec4 \(10'),
    Test("u32 csel",
         """
         #version 310 es
         precision mediump int;

         in flat uvec4 var;
         out uvec4 color;

         void main()
         {
                 color = (var.x > var.y) ? var : uvec4(10);
         }
         """,
         r'\(expression u16vec4 u2ump \(constant uvec4 \(10'),
    Test("f32 loop counter",
         """
         #version 300 es
         precision mediump float;

         uniform float n, incr;
         out float color;

         void main()
         {
                 color = 0.0;
                 for (float x = 0.0; x < n; x += incr)
                    color += x;
         }
         """,
         r'\(assign  \(x\) \(var_ref x\)  \(expression float16_t \+ \(var_ref x\) \(var_ref incr'),
    Test("i32 loop counter",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform int n, incr;
         out int color;

         void main()
         {
                 color = 0;
                 for (int x = 0; x < n; x += incr)
                    color += x;
         }
         """,
         r'\(assign  \(x\) \(var_ref x\)  \(expression int16_t \+ \(var_ref x\) \(expression int16_t i2imp \(var_ref incr'),
    Test("u32 loop counter",
         """
         #version 310 es
         precision mediump float;
         precision mediump int;

         uniform uint n, incr;
         out uint color;

         void main()
         {
                 color = 0u;
                 for (uint x = 0u; x < n; x += incr)
                    color += x;
         }
         """,
         r'\(assign  \(x\) \(var_ref x\)  \(expression uint16_t \+ \(var_ref x\) \(expression uint16_t u2ump \(var_ref incr'),
    Test("f32 temp array",
         """
         #version 300 es
         precision mediump float;

         uniform float x,y;
         out float color;

         void main()
         {
                 float a[2] = float[2](x, y);
                 if (x > 0.0)
                     a[1] = 3.0;
                 color = a[0] + a[1];
         }
         """,
         r'\(expression float16_t f2fmp \(constant float \(3'),
    Test("i32 temp array",
         """
         #version 310 es
         precision mediump int;

         uniform int x,y;
         out int color;

         void main()
         {
                 int a[2] = int[2](x, y);
                 if (x > 0)
                     a[1] = 3;
                 color = a[0] + a[1];
         }
         """,
         r'\(expression int16_t i2imp \(constant int \(3'),
    Test("u32 temp array",
         """
         #version 310 es
         precision mediump int;

         uniform uint x,y;
         out uint color;

         void main()
         {
                 uint a[2] = uint[2](x, y);
                 if (x > 0u)
                     a[1] = 3u;
                 color = a[0] + a[1];
         }
         """,
         r'\(expression uint16_t u2ump \(constant uint \(3'),
    Test("f32 temp array of array",
         """
         #version 310 es
         precision mediump float;

         uniform float x,y;
         out float color;

         void main()
         {
                 float a[2][2] = float[2][2](float[2](x, y), float[2](x, y));
                 if (x > 0.0)
                     a[1][1] = 3.0;
                 color = a[0][0] + a[1][1];
         }
         """,
         r'\(expression float16_t f2fmp \(constant float \(3'),
    Test("i32 temp array of array",
         """
         #version 310 es
         precision mediump int;

         uniform int x,y;
         out int color;

         void main()
         {
                 int a[2][2] = int[2][2](int[2](x, y), int[2](x, y));
                 if (x > 0)
                     a[1][1] = 3;
                 color = a[0][0] + a[1][1];
         }
         """,
         r'\(expression int16_t i2imp \(constant int \(3'),
    Test("u32 temp array of array",
         """
         #version 310 es
         precision mediump int;

         uniform uint x,y;
         out uint color;

         void main()
         {
                 uint a[2][2] = uint[2][2](uint[2](x, y), uint[2](x, y));
                 if (x > 0u)
                     a[1][1] = 3u;
                 color = a[0][0] + a[1][1];
         }
         """,
         r'\(expression uint16_t u2ump \(constant uint \(3'),
    Test("f32 temp array of array assigned from highp",
         """
         #version 310 es
         precision mediump float;

         uniform float x,y;
         out float color;

         void main()
         {
                 highp float b[2][2] = float[2][2](float[2](x, y), float[2](x, y));
                 float a[2][2];
                 a = b;
                 if (x > 0.0)
                     a[1][1] = 3.0;
                 color = a[0][0] + a[1][1];
         }
         """,
         r'\(expression float16_t f2fmp \(constant float \(3'),
    Test("i32 temp array of array assigned from highp",
         """
         #version 310 es
         precision mediump int;

         uniform int x,y;
         out int color;

         void main()
         {
                 highp int b[2][2] = int[2][2](int[2](x, y), int[2](x, y));
                 int a[2][2];
                 a = b;
                 if (x > 0)
                     a[1][1] = 3;
                 color = a[0][0] + a[1][1];
         }
         """,
         r'\(expression int16_t i2imp \(constant int \(3'),
    Test("u32 temp array of array assigned from highp",
         """
         #version 310 es
         precision mediump int;

         uniform uint x,y;
         out uint color;

         void main()
         {
                 highp uint b[2][2] = uint[2][2](uint[2](x, y), uint[2](x, y));
                 uint a[2][2];
                 a = b;
                 if (x > 0u)
                     a[1][1] = 3u;
                 color = a[0][0] + a[1][1];
         }
         """,
         r'\(expression uint16_t u2ump \(constant uint \(3'),
    Test("f32 temp array of array assigned to highp",
         """
         #version 310 es
         precision mediump float;

         uniform float x,y;
         out float color;

         void main()
         {
                 float a[2][2] = float[2][2](float[2](x, y), float[2](x, y));
                 highp float b[2][2];
                 b = a;
                 a = b;
                 if (x > 0.0)
                     a[1][1] = 3.0;
                 color = a[0][0] + a[1][1];
         }
         """,
         r'\(expression float16_t f2fmp \(constant float \(3'),
    Test("i32 temp array of array assigned to highp",
         """
         #version 310 es
         precision mediump int;

         uniform int x,y;
         out int color;

         void main()
         {
                 int a[2][2] = int[2][2](int[2](x, y), int[2](x, y));
                 highp int b[2][2];
                 b = a;
                 a = b;
                 if (x > 0)
                     a[1][1] = 3;
                 color = a[0][0] + a[1][1];
         }
         """,
         r'\(expression int16_t i2imp \(constant int \(3'),
    Test("u32 temp array of array assigned to highp",
         """
         #version 310 es
         precision mediump int;

         uniform uint x,y;
         out uint color;

         void main()
         {
                 uint a[2][2] = uint[2][2](uint[2](x, y), uint[2](x, y));
                 highp uint b[2][2];
                 b = a;
                 a = b;
                 if (x > 0u)
                     a[1][1] = 3u;
                 color = a[0][0] + a[1][1];
         }
         """,
         r'\(expression uint16_t u2ump \(constant uint \(3'),
    Test("f32 temp array of array returned by function",
         """
         #version 310 es
         precision mediump float;

         uniform float x,y;
         out float color;

         float[2][2] f(void)
         {
            return float[2][2](float[2](x, y), float[2](x, y));
         }

         void main()
         {
                 float a[2][2] = f();
                 if (x > 0.0)
                     a[1][1] = 3.0;
                 color = a[0][0] + a[1][1];
         }
         """,
         r'\(expression float16_t f2fmp \(constant float \(3'),
    Test("i32 temp array of array returned by function",
         """
         #version 310 es
         precision mediump int;

         uniform int x,y;
         out int color;

         int[2][2] f(void)
         {
            return int[2][2](int[2](x, y), int[2](x, y));
         }

         void main()
         {
                 int a[2][2] = f();
                 if (x > 0)
                     a[1][1] = 3;
                 color = a[0][0] + a[1][1];
         }
         """,
         r'\(expression int16_t i2imp \(constant int \(3'),
    Test("u32 temp array of array returned by function",
         """
         #version 310 es
         precision mediump int;

         uniform uint x,y;
         out uint color;

         uint[2][2] f(void)
         {
            return uint[2][2](uint[2](x, y), uint[2](x, y));
         }

         void main()
         {
                 uint a[2][2] = f();
                 if (x > 0u)
                     a[1][1] = 3u;
                 color = a[0][0] + a[1][1];
         }
         """,
         r'\(expression uint16_t u2ump \(constant uint \(3'),
    Test("f32 temp array of array as function out",
         """
         #version 310 es
         precision mediump float;

         uniform float x,y;
         out float color;

         void f(out float[2][2] v)
         {
            v = float[2][2](float[2](x, y), float[2](x, y));
         }

         void main()
         {
                 float a[2][2];
                 f(a);
                 if (x > 0.0)
                     a[1][1] = 3.0;
                 color = a[0][0] + a[1][1];
         }
         """,
         r'\(expression float16_t f2fmp \(constant float \(3'),
    Test("i32 temp array of array as function out",
         """
         #version 310 es
         precision mediump int;

         uniform int x,y;
         out int color;

         void f(out int[2][2] v)
         {
            v = int[2][2](int[2](x, y), int[2](x, y));
         }

         void main()
         {
                 int a[2][2];
                 f(a);
                 if (x > 0)
                     a[1][1] = 3;
                 color = a[0][0] + a[1][1];
         }
         """,
         r'\(expression int16_t i2imp \(constant int \(3'),
    Test("u32 temp array of array as function out",
         """
         #version 310 es
         precision mediump int;

         uniform uint x,y;
         out uint color;

         void f(out uint[2][2] v)
         {
            v = uint[2][2](uint[2](x, y), uint[2](x, y));
         }

         void main()
         {
                 uint a[2][2];
                 f(a);
                 if (x > 0u)
                     a[1][1] = 3u;
                 color = a[0][0] + a[1][1];
         }
         """,
         r'\(expression uint16_t u2ump \(constant uint \(3'),
    Test("f32 temp array of array as function in",
         """
         #version 310 es
         precision mediump float;

         uniform float x,y;
         out float color;

         float[2][2] f(in float[2][2] v)
         {
            float t[2][2] = v;
            return t;
         }

         void main()
         {
                 float a[2][2];
                 a = f(a);
                 if (x > 0.0)
                     a[1][1] = 3.0;
                 color = a[0][0] + a[1][1];
         }
         """,
         r'\(expression float16_t f2fmp \(constant float \(3'),
    Test("i32 temp array of array as function in",
         """
         #version 310 es
         precision mediump int;

         uniform int x,y;
         out int color;

         int[2][2] f(in int[2][2] v)
         {
            int t[2][2] = v;
            return t;
         }

         void main()
         {
                 int a[2][2];
                 a = f(a);
                 if (x > 0)
                     a[1][1] = 3;
                 color = a[0][0] + a[1][1];
         }
         """,
         r'\(expression int16_t i2imp \(constant int \(3'),
    Test("u32 temp array of array as function in",
         """
         #version 310 es
         precision mediump int;

         uniform uint x,y;
         out uint color;

         uint[2][2] f(in uint[2][2] v)
         {
            uint t[2][2] = v;
            return t;
         }

         void main()
         {
                 uint a[2][2];
                 a = f(a);
                 if (x > 0u)
                     a[1][1] = 3u;
                 color = a[0][0] + a[1][1];
         }
         """,
         r'\(expression uint16_t u2ump \(constant uint \(3'),
    Test("f32 temp array of array as function inout",
         """
         #version 310 es
         precision mediump float;

         uniform float x,y;
         out float color;

         void f(inout float[2][2] v)
         {
            float t[2][2] = v;
            v = t;
         }

         void main()
         {
                 float a[2][2];
                 f(a);
                 if (x > 0.0)
                     a[1][1] = 3.0;
                 color = a[0][0] + a[1][1];
         }
         """,
         r'\(expression float16_t f2fmp \(constant float \(3'),
    Test("i32 temp array of array as function inout",
         """
         #version 310 es
         precision mediump int;

         uniform int x,y;
         out int color;

         void f(inout int[2][2] v)
         {
            int t[2][2] = v;
            v = t;
         }

         void main()
         {
                 int a[2][2];
                 f(a);
                 if (x > 0)
                     a[1][1] = 3;
                 color = a[0][0] + a[1][1];
         }
         """,
         r'\(expression int16_t i2imp \(constant int \(3'),
    Test("u32 temp array of array as function inout",
         """
         #version 310 es
         precision mediump int;

         uniform uint x,y;
         out uint color;

         void f(inout uint[2][2] v)
         {
            uint t[2][2] = v;
            v = t;
         }

         void main()
         {
                 uint a[2][2];
                 f(a);
                 if (x > 0u)
                     a[1][1] = 3u;
                 color = a[0][0] + a[1][1];
         }
         """,
         r'\(expression uint16_t u2ump \(constant uint \(3'),
    Test("f32 temp struct (not lowered in the presence of control flow - TODO)",
         """
         #version 300 es
         precision mediump float;

         uniform float x,y;
         out float color;

         void main()
         {
                 struct { float x,y; } s;
                 s.x = x;
                 s.y = y;
                 if (x > 0.0)
                     s.y = 3.0;
                 color = s.x + s.y;
         }
         """,
         r'\(constant float \(3'), # should be float16_t
    Test("i32 temp struct (not lowered in the presence of control flow - TODO)",
         """
         #version 300 es
         precision mediump int;

         uniform int x,y;
         out int color;

         void main()
         {
                 struct { int x,y; } s;
                 s.x = x;
                 s.y = y;
                 if (x > 0)
                     s.y = 3;
                 color = s.x + s.y;
         }
         """,
         r'\(constant int \(3'), # should be int16_t
    Test("u32 temp struct (not lowered in the presence of control flow - TODO)",
         """
         #version 300 es
         precision mediump int;

         uniform uint x,y;
         out uint color;

         void main()
         {
                 struct { uint x,y; } s;
                 s.x = x;
                 s.y = y;
                 if (x > 0u)
                     s.y = 3u;
                 color = s.x + s.y;
         }
         """,
         r'\(constant uint \(3'), # should be uint16_t

    Test("vec4 constructor from float",
         """
         uniform highp float a;
         uniform mediump float b;

         void main()
         {
                 gl_FragColor = vec4(a) * b;
         }
         """,
         r'\(expression vec4 \* \(swiz xxxx \(var_ref a\) \)\(expression float f162f \(var_ref b\) \) \)'),

    Test("respect copies",
         """
         uniform mediump float a, b;

         void main()
         {
            highp float x = a;
            gl_FragColor.x = x * b;
         }
         """,
         r'expression float \* \(expression float f162f \(var_ref a\) \) \(expression float f162f \(var_ref b\) \) '), # should be uint16_t

    Test("conversion constructor precision",
         """
         #version 300 es
         uniform mediump uint a;
         out highp float result;

         void main()
         {
            /* Constructors don't have a precision qualifier themselves, but
             * constructors are an operation, and so they do the usual "get
             * precision from my operands, or default to the precision of the
             * lvalue" rule.  So, the u2f is done at mediump due to a's precision.
             */
            result = float(a);
         }
         """,
         r'expression float16_t u2f \(expression uint16_t u2ump \(var_ref a\) \)'), # should be uint16_t

]


def compile_shader(standalone_compiler, source):
    with tempfile.NamedTemporaryFile(mode='wt', suffix='.frag') as source_file:
        print(source, file=source_file)
        source_file.flush()
        return subprocess.check_output([standalone_compiler,
                                        '--version', '300',
                                        '--lower-precision',
                                        '--dump-lir',
                                        source_file.name],
                                       universal_newlines=True)


def run_test(standalone_compiler, test):
    ir = compile_shader(standalone_compiler, test.source)

    if re.search(test.match_re, ir) is None:
        print(ir)
        return False

    return True


def main():
    standalone_compiler = sys.argv[1]
    passed = 0

    for test in TESTS:
        print('Testing {} ... '.format(test.name), end='')

        result = run_test(standalone_compiler, test)

        if result:
            print('PASS')
            passed += 1
        else:
            print('FAIL')

    print('{}/{} tests returned correct results'.format(passed, len(TESTS)))
    sys.exit(0 if passed == len(TESTS) else 1)


if __name__ == '__main__':
    main()
