# name: Mixed 16 and 32-bit Thumb conditional instructions
# as: -march=armv6kt2 -mimplicit-it=always
# source: thumb2_it.s
# objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arm.*

Disassembly of section .text:
0+000 <[^>]+> bf05      	ittet	eq
0+002 <[^>]+> 1880      	addeq	r0, r0, r2
0+004 <[^>]+> 4440      	addeq	r0, r8
0+006 <[^>]+> 1888      	addne	r0, r1, r2
0+008 <[^>]+> eb11 0002 	addseq.w	r0, r1, r2
0+00c <[^>]+> 4410      	add	r0, r2
0+00e <[^>]+> 4440      	add	r0, r8
0+010 <[^>]+> 1880      	adds	r0, r0, r2
0+012 <[^>]+> eb10 0008 	adds.w	r0, r0, r8
0+016 <[^>]+> 1888      	adds	r0, r1, r2
0+018 <[^>]+> bf0a      	itet	eq
0+01a <[^>]+> 4310      	orreq	r0, r2
0+01c <[^>]+> ea40 0008 	orrne.w	r0, r0, r8
0+020 <[^>]+> ea50 0002 	orrseq.w	r0, r0, r2
0+024 <[^>]+> ea40 0002 	orr.w	r0, r0, r2
0+028 <[^>]+> ea40 0008 	orr.w	r0, r0, r8
0+02c <[^>]+> 4310      	orrs	r0, r2
0+02e <[^>]+> bf01      	itttt	eq
0+030 <[^>]+> 4090      	lsleq	r0, r2
0+032 <[^>]+> fa00 f008 	lsleq.w	r0, r0, r8
0+036 <[^>]+> fa01 f002 	lsleq.w	r0, r1, r2
0+03a <[^>]+> fa10 f002 	lslseq.w	r0, r0, r2
0+03e <[^>]+> bf02      	ittt	eq
0+040 <[^>]+> 0048      	lsleq	r0, r1, #1
0+042 <[^>]+> ea4f 0048 	moveq.w	r0, r8, lsl #1
0+046 <[^>]+> ea5f 0040 	movseq.w	r0, r0, lsl #1
0+04a <[^>]+> fa00 f002 	lsl.w	r0, r0, r2
0+04e <[^>]+> 4090      	lsls	r0, r2
0+050 <[^>]+> ea4f 0041 	mov.w	r0, r1, lsl #1
0+054 <[^>]+> 0048      	lsls	r0, r1, #1
0+056 <[^>]+> bf01      	itttt	eq
0+058 <[^>]+> 4288      	cmpeq	r0, r1
0+05a <[^>]+> 4540      	cmpeq	r0, r8
0+05c <[^>]+> 4608      	moveq	r0, r1
0+05e <[^>]+> ea5f 0001 	movseq.w	r0, r1
0+062 <[^>]+> bf08      	it	eq
0+064 <[^>]+> 4640      	moveq	r0, r8
0+066 <[^>]+> 4608      	mov	r0, r1
0+068 <[^>]+> 0008      	movs	r0, r1
0+06a <[^>]+> ea5f 0008 	movs.w	r0, r8
0+06e <[^>]+> bf01      	itttt	eq
0+070 <[^>]+> 43c8      	mvneq	r0, r1
0+072 <[^>]+> ea6f 0008 	mvneq.w	r0, r8
0+076 <[^>]+> ea7f 0001 	mvnseq.w	r0, r1
0+07a <[^>]+> 42c8      	cmneq	r0, r1
0+07c <[^>]+> ea6f 0001 	mvn.w	r0, r1
0+080 <[^>]+> 43c8      	mvns	r0, r1
0+082 <[^>]+> bf02      	ittt	eq
0+084 <[^>]+> 4248      	negeq	r0, r1
0+086 <[^>]+> f1c8 0000 	rsbeq	r0, r8, #0
0+08a <[^>]+> f1d1 0000 	rsbseq	r0, r1, #0
0+08e <[^>]+> f1c1 0000 	rsb	r0, r1, #0
0+092 <[^>]+> 4248      	negs	r0, r1
0+094 <[^>]+> bfc8      	it	gt
0+096 <[^>]+> 4220      	tstgt	r0, r4
