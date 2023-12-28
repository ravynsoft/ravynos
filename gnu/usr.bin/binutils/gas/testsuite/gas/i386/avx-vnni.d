#objdump: -dw
#name: i386 AVX VNNI insns

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	62 f2 5d 08 50 d2    	vpdpbusd %xmm2,%xmm4,%xmm2
 +[a-f0-9]+:	62 f2 5d 08 50 d2    	vpdpbusd %xmm2,%xmm4,%xmm2
 +[a-f0-9]+:	c4 e2 59 50 d2       	\{vex\} vpdpbusd %xmm2,%xmm4,%xmm2
 +[a-f0-9]+:	c4 e2 59 50 d2       	\{vex\} vpdpbusd %xmm2,%xmm4,%xmm2
 +[a-f0-9]+:	c4 e2 59 50 11       	\{vex\} vpdpbusd \(%ecx\),%xmm4,%xmm2
 +[a-f0-9]+:	c4 e2 59 50 11       	\{vex\} vpdpbusd \(%ecx\),%xmm4,%xmm2
 +[a-f0-9]+:	62 f2 5d 08 52 d2    	vpdpwssd %xmm2,%xmm4,%xmm2
 +[a-f0-9]+:	62 f2 5d 08 52 d2    	vpdpwssd %xmm2,%xmm4,%xmm2
 +[a-f0-9]+:	c4 e2 59 52 d2       	\{vex\} vpdpwssd %xmm2,%xmm4,%xmm2
 +[a-f0-9]+:	c4 e2 59 52 d2       	\{vex\} vpdpwssd %xmm2,%xmm4,%xmm2
 +[a-f0-9]+:	c4 e2 59 52 11       	\{vex\} vpdpwssd \(%ecx\),%xmm4,%xmm2
 +[a-f0-9]+:	c4 e2 59 52 11       	\{vex\} vpdpwssd \(%ecx\),%xmm4,%xmm2
 +[a-f0-9]+:	62 f2 5d 08 51 d2    	vpdpbusds %xmm2,%xmm4,%xmm2
 +[a-f0-9]+:	62 f2 5d 08 51 d2    	vpdpbusds %xmm2,%xmm4,%xmm2
 +[a-f0-9]+:	c4 e2 59 51 d2       	\{vex\} vpdpbusds %xmm2,%xmm4,%xmm2
 +[a-f0-9]+:	c4 e2 59 51 d2       	\{vex\} vpdpbusds %xmm2,%xmm4,%xmm2
 +[a-f0-9]+:	c4 e2 59 51 11       	\{vex\} vpdpbusds \(%ecx\),%xmm4,%xmm2
 +[a-f0-9]+:	c4 e2 59 51 11       	\{vex\} vpdpbusds \(%ecx\),%xmm4,%xmm2
 +[a-f0-9]+:	62 f2 5d 08 53 d2    	vpdpwssds %xmm2,%xmm4,%xmm2
 +[a-f0-9]+:	62 f2 5d 08 53 d2    	vpdpwssds %xmm2,%xmm4,%xmm2
 +[a-f0-9]+:	c4 e2 59 53 d2       	\{vex\} vpdpwssds %xmm2,%xmm4,%xmm2
 +[a-f0-9]+:	c4 e2 59 53 d2       	\{vex\} vpdpwssds %xmm2,%xmm4,%xmm2
 +[a-f0-9]+:	c4 e2 59 53 11       	\{vex\} vpdpwssds \(%ecx\),%xmm4,%xmm2
 +[a-f0-9]+:	c4 e2 59 53 11       	\{vex\} vpdpwssds \(%ecx\),%xmm4,%xmm2
 +[a-f0-9]+:	62 f2 7d 48 50 c0    	vpdpbusd %zmm0,%zmm0,%zmm0
 +[a-f0-9]+:	c4 e2 7d 50 c0       	\{vex\} vpdpbusd %ymm0,%ymm0,%ymm0
 +[a-f0-9]+:	c4 e2 79 50 c0       	\{vex\} vpdpbusd %xmm0,%xmm0,%xmm0
 +[a-f0-9]+:	c4 e2 7d 50 c0       	\{vex\} vpdpbusd %ymm0,%ymm0,%ymm0
 +[a-f0-9]+:	c4 e2 79 50 c0       	\{vex\} vpdpbusd %xmm0,%xmm0,%xmm0
 +[a-f0-9]+:	c4 e2 7d 50 c0       	\{vex\} vpdpbusd %ymm0,%ymm0,%ymm0
 +[a-f0-9]+:	c4 e2 79 50 c0       	\{vex\} vpdpbusd %xmm0,%xmm0,%xmm0
 +[a-f0-9]+:	62 f2 5d 08 50 d2    	vpdpbusd %xmm2,%xmm4,%xmm2
 +[a-f0-9]+:	c4 e2 59 50 91 f0 07 00 00 	\{vex\} vpdpbusd 0x7f0\(%ecx\),%xmm4,%xmm2
#pass
