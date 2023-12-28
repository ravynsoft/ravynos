#as:
#objdump: -dw
#name: i386 AVX IFMA insns
#source: avx-ifma.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 dd 08 b5 d2[ 	]*vpmadd52huq %xmm2,%xmm4,%xmm2
[ 	]*[a-f0-9]+:[ 	]*62 f2 dd 08 b5 d2[ 	]*vpmadd52huq %xmm2,%xmm4,%xmm2
[ 	]*[a-f0-9]+:[ 	]*c4 e2 d9 b5 d2[ 	]*\{vex\} vpmadd52huq %xmm2,%xmm4,%xmm2
[ 	]*[a-f0-9]+:[ 	]*c4 e2 d9 b5 11[ 	]*\{vex\} vpmadd52huq \(%ecx\),%xmm4,%xmm2
[ 	]*[a-f0-9]+:[ 	]*62 f2 dd 28 b5 d2[ 	]*vpmadd52huq %ymm2,%ymm4,%ymm2
[ 	]*[a-f0-9]+:[ 	]*62 f2 dd 28 b5 d2[ 	]*vpmadd52huq %ymm2,%ymm4,%ymm2
[ 	]*[a-f0-9]+:[ 	]*c4 e2 dd b5 d2[ 	]*\{vex\} vpmadd52huq %ymm2,%ymm4,%ymm2
[ 	]*[a-f0-9]+:[ 	]*c4 e2 dd b5 11[ 	]*\{vex\} vpmadd52huq \(%ecx\),%ymm4,%ymm2
[ 	]*[a-f0-9]+:[ 	]*62 f2 dd 08 b4 d2[ 	]*vpmadd52luq %xmm2,%xmm4,%xmm2
[ 	]*[a-f0-9]+:[ 	]*62 f2 dd 08 b4 d2[ 	]*vpmadd52luq %xmm2,%xmm4,%xmm2
[ 	]*[a-f0-9]+:[ 	]*c4 e2 d9 b4 d2[ 	]*\{vex\} vpmadd52luq %xmm2,%xmm4,%xmm2
[ 	]*[a-f0-9]+:[ 	]*c4 e2 d9 b4 11[ 	]*\{vex\} vpmadd52luq \(%ecx\),%xmm4,%xmm2
[ 	]*[a-f0-9]+:[ 	]*62 f2 dd 28 b4 d2[ 	]*vpmadd52luq %ymm2,%ymm4,%ymm2
[ 	]*[a-f0-9]+:[ 	]*62 f2 dd 28 b4 d2[ 	]*vpmadd52luq %ymm2,%ymm4,%ymm2
[ 	]*[a-f0-9]+:[ 	]*c4 e2 dd b4 d2[ 	]*\{vex\} vpmadd52luq %ymm2,%ymm4,%ymm2
[ 	]*[a-f0-9]+:[ 	]*c4 e2 dd b4 11[ 	]*\{vex\} vpmadd52luq \(%ecx\),%ymm4,%ymm2
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 b5 c0[ 	]*vpmadd52huq %zmm0,%zmm0,%zmm0
[ 	]*[a-f0-9]+:[ 	]*c4 e2 fd b5 c0[ 	]*\{vex\} vpmadd52huq %ymm0,%ymm0,%ymm0
[ 	]*[a-f0-9]+:[ 	]*c4 e2 f9 b5 c0[ 	]*\{vex\} vpmadd52huq %xmm0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:[ 	]*c4 e2 fd b5 c0[ 	]*\{vex\} vpmadd52huq %ymm0,%ymm0,%ymm0
[ 	]*[a-f0-9]+:[ 	]*c4 e2 f9 b5 c0[ 	]*\{vex\} vpmadd52huq %xmm0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:[ 	]*c4 e2 fd b5 c0[ 	]*\{vex\} vpmadd52huq %ymm0,%ymm0,%ymm0
[ 	]*[a-f0-9]+:[ 	]*c4 e2 f9 b5 c0[ 	]*\{vex\} vpmadd52huq %xmm0,%xmm0,%xmm0
[ 	]*[a-f0-9]+:[ 	]*62 f2 dd 08 b5 d2[ 	]*vpmadd52huq %xmm2,%xmm4,%xmm2
[ 	]*[a-f0-9]+:[ 	]*62 f2 dd 28 b5 d2[ 	]*vpmadd52huq %ymm2,%ymm4,%ymm2
#pass
