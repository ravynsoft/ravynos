#as:
#objdump: -dw
#name: x86_64 KEYLOCKER insns
#source: x86-64-keylocker.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 dc f2[ 	]*loadiwkey %xmm2,%xmm6
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 fa d0[ 	]*encodekey128 %eax,%edx
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 fb d0[ 	]*encodekey256 %eax,%edx
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 dc 52 7e[ 	]*aesenc128kl 0x7e\(%rdx\),%xmm2
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 de 52 7e[ 	]*aesenc256kl 0x7e\(%rdx\),%xmm2
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 dd 52 7e[ 	]*aesdec128kl 0x7e\(%rdx\),%xmm2
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 df 52 7e[ 	]*aesdec256kl 0x7e\(%rdx\),%xmm2
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 d8 42 7e[ 	]*aesencwide128kl 0x7e\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 d8 52 7e[ 	]*aesencwide256kl 0x7e\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 d8 4a 7e[ 	]*aesdecwide128kl 0x7e\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 d8 5a 7e[ 	]*aesdecwide256kl 0x7e\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 dc f2[ 	]*loadiwkey %xmm2,%xmm6
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 fa d0[ 	]*encodekey128 %eax,%edx
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 fb d0[ 	]*encodekey256 %eax,%edx
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 dc 52 7e[ 	]*aesenc128kl 0x7e\(%rdx\),%xmm2
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 de 52 7e[ 	]*aesenc256kl 0x7e\(%rdx\),%xmm2
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 dd 52 7e[ 	]*aesdec128kl 0x7e\(%rdx\),%xmm2
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 df 52 7e[ 	]*aesdec256kl 0x7e\(%rdx\),%xmm2
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 d8 42 7e[ 	]*aesencwide128kl 0x7e\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 d8 52 7e[ 	]*aesencwide256kl 0x7e\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 d8 4a 7e[ 	]*aesdecwide128kl 0x7e\(%rdx\)
[ 	]*[a-f0-9]+:[ 	]*f3 0f 38 d8 5a 7e[ 	]*aesdecwide256kl 0x7e\(%rdx\)
#pass
