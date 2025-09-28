#as:
#objdump: -dw
#name: x86-64 AVX IFMA insns
#source: x86-64-avx-ifma.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 d2 dd 08 b5 d4[ 	]*vpmadd52huq %xmm12,%xmm4,%xmm2
[ 	]*[a-f0-9]+:[ 	]*62 d2 dd 08 b5 d4[ 	]*vpmadd52huq %xmm12,%xmm4,%xmm2
[ 	]*[a-f0-9]+:[ 	]*c4 c2 d9 b5 d4[ 	]*\{vex\} vpmadd52huq %xmm12,%xmm4,%xmm2
[ 	]*[a-f0-9]+:[ 	]*c4 e2 d9 b5 11[ 	]*\{vex\} vpmadd52huq \(%rcx\),%xmm4,%xmm2
[ 	]*[a-f0-9]+:[ 	]*62 b2 dd 08 b5 d6[ 	]*vpmadd52huq %xmm22,%xmm4,%xmm2
[ 	]*[a-f0-9]+:[ 	]*62 d2 dd 28 b5 d4[ 	]*vpmadd52huq %ymm12,%ymm4,%ymm2
[ 	]*[a-f0-9]+:[ 	]*62 d2 dd 28 b5 d4[ 	]*vpmadd52huq %ymm12,%ymm4,%ymm2
[ 	]*[a-f0-9]+:[ 	]*c4 c2 dd b5 d4[ 	]*\{vex\} vpmadd52huq %ymm12,%ymm4,%ymm2
[ 	]*[a-f0-9]+:[ 	]*c4 e2 dd b5 11[ 	]*\{vex\} vpmadd52huq \(%rcx\),%ymm4,%ymm2
[ 	]*[a-f0-9]+:[ 	]*62 b2 dd 28 b5 d6[ 	]*vpmadd52huq %ymm22,%ymm4,%ymm2
[ 	]*[a-f0-9]+:[ 	]*62 d2 dd 08 b4 d4[ 	]*vpmadd52luq %xmm12,%xmm4,%xmm2
[ 	]*[a-f0-9]+:[ 	]*62 d2 dd 08 b4 d4[ 	]*vpmadd52luq %xmm12,%xmm4,%xmm2
[ 	]*[a-f0-9]+:[ 	]*c4 c2 d9 b4 d4[ 	]*\{vex\} vpmadd52luq %xmm12,%xmm4,%xmm2
[ 	]*[a-f0-9]+:[ 	]*c4 e2 d9 b4 11[ 	]*\{vex\} vpmadd52luq \(%rcx\),%xmm4,%xmm2
[ 	]*[a-f0-9]+:[ 	]*62 b2 dd 08 b4 d6[ 	]*vpmadd52luq %xmm22,%xmm4,%xmm2
[ 	]*[a-f0-9]+:[ 	]*62 d2 dd 28 b4 d4[ 	]*vpmadd52luq %ymm12,%ymm4,%ymm2
[ 	]*[a-f0-9]+:[ 	]*62 d2 dd 28 b4 d4[ 	]*vpmadd52luq %ymm12,%ymm4,%ymm2
[ 	]*[a-f0-9]+:[ 	]*c4 c2 dd b4 d4[ 	]*\{vex\} vpmadd52luq %ymm12,%ymm4,%ymm2
[ 	]*[a-f0-9]+:[ 	]*c4 e2 dd b4 11[ 	]*\{vex\} vpmadd52luq \(%rcx\),%ymm4,%ymm2
[ 	]*[a-f0-9]+:[ 	]*62 b2 dd 28 b4 d6[ 	]*vpmadd52luq %ymm22,%ymm4,%ymm2
[ 	]*[a-f0-9]+:[ 	]*62 d2 dd 08 b5 d4[ 	]*vpmadd52huq %xmm12,%xmm4,%xmm2
[ 	]*[a-f0-9]+:[ 	]*62 d2 dd 28 b5 d4[ 	]*vpmadd52huq %ymm12,%ymm4,%ymm2
#pass
