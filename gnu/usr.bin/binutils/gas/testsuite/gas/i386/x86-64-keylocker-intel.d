#as:
#objdump: -dw -Mintel
#name: x86_64 KEYLOCKER insns (Intel disassembly)
#source: x86-64-keylocker.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 dc f2[ 	]*loadiwkey xmm6,xmm2
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 fa d0[ 	]*encodekey128 edx,eax
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 fb d0[ 	]*encodekey256 edx,eax
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 dc 52 7e[ 	]*aesenc128kl xmm2,\[rdx\+0x7e\]
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 de 52 7e[ 	]*aesenc256kl xmm2,\[rdx\+0x7e\]
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 dd 52 7e[ 	]*aesdec128kl xmm2,\[rdx\+0x7e\]
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 df 52 7e[ 	]*aesdec256kl xmm2,\[rdx\+0x7e\]
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 d8 42 7e[ 	]*aesencwide128kl \[rdx\+0x7e\]
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 d8 52 7e[ 	]*aesencwide256kl \[rdx\+0x7e\]
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 d8 4a 7e[ 	]*aesdecwide128kl \[rdx\+0x7e\]
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 d8 5a 7e[ 	]*aesdecwide256kl \[rdx\+0x7e\]
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 dc f2[ 	]*loadiwkey xmm6,xmm2
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 fa d0[ 	]*encodekey128 edx,eax
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 fb d0[ 	]*encodekey256 edx,eax
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 dc 52 7e[ 	]*aesenc128kl xmm2,\[rdx\+0x7e\]
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 de 52 7e[ 	]*aesenc256kl xmm2,\[rdx\+0x7e\]
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 dd 52 7e[ 	]*aesdec128kl xmm2,\[rdx\+0x7e\]
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 df 52 7e[ 	]*aesdec256kl xmm2,\[rdx\+0x7e\]
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 d8 42 7e[ 	]*aesencwide128kl \[rdx\+0x7e\]
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 d8 52 7e[ 	]*aesencwide256kl \[rdx\+0x7e\]
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 d8 4a 7e[ 	]*aesdecwide128kl \[rdx\+0x7e\]
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 d8 5a 7e[ 	]*aesdecwide256kl \[rdx\+0x7e\]
#pass
