# name: ARM V6t2 Alignment
# as: -march=armv6kt2
# objdump: -dr --prefix-addresses --show-raw-insn
#skip: *-*-pe *-*-wince

.*: +file format .*arm.*

Disassembly of section .text:
0+000 <[^>]*> bf00      	nop
0+002 <[^>]*> 4611      	mov	r1, r2
0+004 <[^>]*> f3af 8000 	nop.w
0+008 <[^>]*> f3af 8000 	nop.w
0+00c <[^>]*> f3af 8000 	nop.w
0+010 <[^>]*> 4611      	mov	r1, r2
0+012 <[^>]*> bf00      	nop
0+014 <[^>]*> f3af 8000 	nop.w
0+018 <[^>]*> e320f000 	nop	\{0\}
0+01c <[^>]*> e1a01002 	mov	r1, r2
0+020 <[^>]*> e1a01002 	mov	r1, r2
0+024 <[^>]*> e320f000 	nop	\{0\}
0+028 <[^>]*> e320f000 	nop	\{0\}
0+02c <[^>]*> e320f000 	nop	\{0\}
0+030 <[^>]*> e320f000 	nop	\{0\}
0+034 <[^>]*> e320f000 	nop	\{0\}
0+038 <[^>]*> e320f000 	nop	\{0\}
0+03c <[^>]*> e320f000 	nop	\{0\}
