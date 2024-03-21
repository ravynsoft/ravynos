# name: 26-bit teq/cmn/tst/cmp instructions
# objdump: -dr --prefix-addresses --show-raw-insn -marmv4
# skip: *-*-pe *-*-wince

.*: +file format .*arm.*


Disassembly of section .text:
0+000 <[^>]*> e330f00a ?	teqp	r0, #10
0+004 <[^>]*> e132f004 ?	teqp	r2, r4
0+008 <[^>]*> e135f287 ?	teqp	r5, r7, lsl #5
0+00c <[^>]*> e131f113 ?	teqp	r1, r3, lsl r1
0+010 <[^>]*> e370f00a ?	cmnp	r0, #10
0+014 <[^>]*> e172f004 ?	cmnp	r2, r4
0+018 <[^>]*> e175f287 ?	cmnp	r5, r7, lsl #5
0+01c <[^>]*> e171f113 ?	cmnp	r1, r3, lsl r1
0+020 <[^>]*> e350f00a ?	cmpp	r0, #10
0+024 <[^>]*> e152f004 ?	cmpp	r2, r4
0+028 <[^>]*> e155f287 ?	cmpp	r5, r7, lsl #5
0+02c <[^>]*> e151f113 ?	cmpp	r1, r3, lsl r1
0+030 <[^>]*> e310f00a ?	tstp	r0, #10
0+034 <[^>]*> e112f004 ?	tstp	r2, r4
0+038 <[^>]*> e115f287 ?	tstp	r5, r7, lsl #5
0+03c <[^>]*> e111f113 ?	tstp	r1, r3, lsl r1
