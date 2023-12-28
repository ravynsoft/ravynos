# name: Armv8.1-M Mainline vcvt instruction in it block (with MVE)
# as: -march=armv8.1-m.main+mve.fp+fp.dp
#warning: [^:]*: Assembler messages:
#warning: [^:]*:10: Warning: scalar fp16 instruction cannot be conditional, the behaviour is UNPREDICTABLE
#warning: [^:]*:11: Warning: scalar fp16 instruction cannot be conditional, the behaviour is UNPREDICTABLE
#warning: [^:]*:19: Warning: scalar fp16 instruction cannot be conditional, the behaviour is UNPREDICTABLE
#warning: [^:]*:20: Warning: scalar fp16 instruction cannot be conditional, the behaviour is UNPREDICTABLE
# objdump: -dr --prefix-addresses --show-raw-insn -marmv8.1-m.main

.*: +file format .*arm.*

Disassembly of section .text:
^[^>]*> bf1c[ 	]+itt[ 	]+ne
^[^>]*> eefd 6bc8[ 	]+vcvtne.s32.f64[ 	]+s13, d8
^[^>]*> eefc 6bc8[ 	]+vcvtne.u32.f64[ 	]+s13, d8
^[^>]*> bf1c[ 	]+itt[ 	]+ne
^[^>]*> eefd 6ac4[ 	]+vcvtne.s32.f32[ 	]+s13, s8
^[^>]*> eefc 6ac4[ 	]+vcvtne.u32.f32[ 	]+s13, s8
^[^>]*> bf1c[ 	]+itt[ 	]+ne
^[^>]*> eefd 69c4[ 	]+vcvtne.s32.f16[ 	]+s13, s8.*
^[^>]*> eefc 69c4[ 	]+vcvtne.u32.f16[ 	]+s13, s8.*
^[^>]*> bf1c[ 	]+itt[ 	]+ne
^[^>]*> eeb8 dbc4[ 	]+vcvtne.f64.s32[ 	]+d13, s8
^[^>]*> eeb8 db44[ 	]+vcvtne.f64.u32[ 	]+d13, s8
^[^>]*> bf1c[ 	]+itt[ 	]+ne
^[^>]*> eef8 6ac4[ 	]+vcvtne.f32.s32[ 	]+s13, s8
^[^>]*> eef8 6a44[ 	]+vcvtne.f32.u32[ 	]+s13, s8
^[^>]*> bf1c[ 	]+itt[ 	]+ne
^[^>]*> eef8 69c4[ 	]+vcvtne.f16.s32[ 	]+s13, s8.*
^[^>]*> eef8 6944[ 	]+vcvtne.f16.u32[ 	]+s13, s8.*
#pass
