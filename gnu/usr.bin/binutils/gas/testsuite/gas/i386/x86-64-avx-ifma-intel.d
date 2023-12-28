#as:
#objdump: -dw -Mintel
#name: x86-64 AVX IFMA insns (Intel disassembly)
#source: x86-64-avx-ifma.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 d2 dd 08 b5 d4[ 	]*vpmadd52huq xmm2,xmm4,xmm12
[ 	]*[a-f0-9]+:[ 	]*62 d2 dd 08 b5 d4[ 	]*vpmadd52huq xmm2,xmm4,xmm12
[ 	]*[a-f0-9]+:[ 	]*c4 c2 d9 b5 d4[ 	]*\{vex\} vpmadd52huq xmm2,xmm4,xmm12
[ 	]*[a-f0-9]+:[ 	]*c4 e2 d9 b5 11[ 	]*\{vex\} vpmadd52huq xmm2,xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b2 dd 08 b5 d6[ 	]*vpmadd52huq xmm2,xmm4,xmm22
[ 	]*[a-f0-9]+:[ 	]*62 d2 dd 28 b5 d4[ 	]*vpmadd52huq ymm2,ymm4,ymm12
[ 	]*[a-f0-9]+:[ 	]*62 d2 dd 28 b5 d4[ 	]*vpmadd52huq ymm2,ymm4,ymm12
[ 	]*[a-f0-9]+:[ 	]*c4 c2 dd b5 d4[ 	]*\{vex\} vpmadd52huq ymm2,ymm4,ymm12
[ 	]*[a-f0-9]+:[ 	]*c4 e2 dd b5 11[ 	]*\{vex\} vpmadd52huq ymm2,ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b2 dd 28 b5 d6[ 	]*vpmadd52huq ymm2,ymm4,ymm22
[ 	]*[a-f0-9]+:[ 	]*62 d2 dd 08 b4 d4[ 	]*vpmadd52luq xmm2,xmm4,xmm12
[ 	]*[a-f0-9]+:[ 	]*62 d2 dd 08 b4 d4[ 	]*vpmadd52luq xmm2,xmm4,xmm12
[ 	]*[a-f0-9]+:[ 	]*c4 c2 d9 b4 d4[ 	]*\{vex\} vpmadd52luq xmm2,xmm4,xmm12
[ 	]*[a-f0-9]+:[ 	]*c4 e2 d9 b4 11[ 	]*\{vex\} vpmadd52luq xmm2,xmm4,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b2 dd 08 b4 d6[ 	]*vpmadd52luq xmm2,xmm4,xmm22
[ 	]*[a-f0-9]+:[ 	]*62 d2 dd 28 b4 d4[ 	]*vpmadd52luq ymm2,ymm4,ymm12
[ 	]*[a-f0-9]+:[ 	]*62 d2 dd 28 b4 d4[ 	]*vpmadd52luq ymm2,ymm4,ymm12
[ 	]*[a-f0-9]+:[ 	]*c4 c2 dd b4 d4[ 	]*\{vex\} vpmadd52luq ymm2,ymm4,ymm12
[ 	]*[a-f0-9]+:[ 	]*c4 e2 dd b4 11[ 	]*\{vex\} vpmadd52luq ymm2,ymm4,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b2 dd 28 b4 d6[ 	]*vpmadd52luq ymm2,ymm4,ymm22
[ 	]*[a-f0-9]+:[ 	]*62 d2 dd 08 b5 d4[ 	]*vpmadd52huq xmm2,xmm4,xmm12
[ 	]*[a-f0-9]+:[ 	]*62 d2 dd 28 b5 d4[ 	]*vpmadd52huq ymm2,ymm4,ymm12
#pass
