#as:
#objdump: -dw
#name: i386 AVX512VNNI insns
#source: avx512vnni.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 48 52 e3[ 	]*vpdpwssd %zmm3,%zmm1,%zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 49 52 e3[ 	]*vpdpwssd %zmm3,%zmm1,%zmm4\{%k1\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 c9 52 e3[ 	]*vpdpwssd %zmm3,%zmm1,%zmm4\{%k1\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 48 52 a4 f4 c0 1d fe ff[ 	]*vpdpwssd -0x1e240\(%esp,%esi,8\),%zmm1,%zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 48 52 62 7f[ 	]*vpdpwssd 0x1fc0\(%edx\),%zmm1,%zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 58 52 62 7f[ 	]*vpdpwssd 0x1fc\(%edx\)\{1to16\},%zmm1,%zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 53 d4[ 	]*vpdpwssds %zmm4,%zmm5,%zmm2
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 4e 53 d4[ 	]*vpdpwssds %zmm4,%zmm5,%zmm2\{%k6\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 ce 53 d4[ 	]*vpdpwssds %zmm4,%zmm5,%zmm2\{%k6\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 53 94 f4 c0 1d fe ff[ 	]*vpdpwssds -0x1e240\(%esp,%esi,8\),%zmm5,%zmm2
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 53 52 7f[ 	]*vpdpwssds 0x1fc0\(%edx\),%zmm5,%zmm2
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 58 53 52 7f[ 	]*vpdpwssds 0x1fc\(%edx\)\{1to16\},%zmm5,%zmm2
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d 48 50 eb[ 	]*vpdpbusd %zmm3,%zmm2,%zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d 49 50 eb[ 	]*vpdpbusd %zmm3,%zmm2,%zmm5\{%k1\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d c9 50 eb[ 	]*vpdpbusd %zmm3,%zmm2,%zmm5\{%k1\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d 48 50 ac f4 c0 1d fe ff[ 	]*vpdpbusd -0x1e240\(%esp,%esi,8\),%zmm2,%zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d 48 50 6a 7f[ 	]*vpdpbusd 0x1fc0\(%edx\),%zmm2,%zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 6d 58 50 6a 7f[ 	]*vpdpbusd 0x1fc\(%edx\)\{1to16\},%zmm2,%zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 48 51 e9[ 	]*vpdpbusds %zmm1,%zmm3,%zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 4a 51 e9[ 	]*vpdpbusds %zmm1,%zmm3,%zmm5\{%k2\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 ca 51 e9[ 	]*vpdpbusds %zmm1,%zmm3,%zmm5\{%k2\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 48 51 ac f4 c0 1d fe ff[ 	]*vpdpbusds -0x1e240\(%esp,%esi,8\),%zmm3,%zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 48 51 6a 7f[ 	]*vpdpbusds 0x1fc0\(%edx\),%zmm3,%zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 65 58 51 6a 7f[ 	]*vpdpbusds 0x1fc\(%edx\)\{1to16\},%zmm3,%zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 48 52 d9[ 	]*vpdpwssd %zmm1,%zmm4,%zmm3
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 4b 52 d9[ 	]*vpdpwssd %zmm1,%zmm4,%zmm3\{%k3\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d cb 52 d9[ 	]*vpdpwssd %zmm1,%zmm4,%zmm3\{%k3\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 48 52 9c f4 c0 1d fe ff[ 	]*vpdpwssd -0x1e240\(%esp,%esi,8\),%zmm4,%zmm3
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 48 52 5a 7f[ 	]*vpdpwssd 0x1fc0\(%edx\),%zmm4,%zmm3
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 58 52 5a 7f[ 	]*vpdpwssd 0x1fc\(%edx\)\{1to16\},%zmm4,%zmm3
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 48 53 da[ 	]*vpdpwssds %zmm2,%zmm1,%zmm3
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 4f 53 da[ 	]*vpdpwssds %zmm2,%zmm1,%zmm3\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 cf 53 da[ 	]*vpdpwssds %zmm2,%zmm1,%zmm3\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 48 53 9c f4 c0 1d fe ff[ 	]*vpdpwssds -0x1e240\(%esp,%esi,8\),%zmm1,%zmm3
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 48 53 5a 7f[ 	]*vpdpwssds 0x1fc0\(%edx\),%zmm1,%zmm3
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 58 53 5a 7f[ 	]*vpdpwssds 0x1fc\(%edx\)\{1to16\},%zmm1,%zmm3
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 48 50 d9[ 	]*vpdpbusd %zmm1,%zmm4,%zmm3
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 4e 50 d9[ 	]*vpdpbusd %zmm1,%zmm4,%zmm3\{%k6\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d ce 50 d9[ 	]*vpdpbusd %zmm1,%zmm4,%zmm3\{%k6\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 48 50 9c f4 c0 1d fe ff[ 	]*vpdpbusd -0x1e240\(%esp,%esi,8\),%zmm4,%zmm3
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 48 50 5a 7f[ 	]*vpdpbusd 0x1fc0\(%edx\),%zmm4,%zmm3
[ 	]*[a-f0-9]+:[ 	]*62 f2 5d 58 50 5a 7f[ 	]*vpdpbusd 0x1fc\(%edx\)\{1to16\},%zmm4,%zmm3
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 48 51 c9[ 	]*vpdpbusds %zmm1,%zmm1,%zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 49 51 c9[ 	]*vpdpbusds %zmm1,%zmm1,%zmm1\{%k1\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 c9 51 c9[ 	]*vpdpbusds %zmm1,%zmm1,%zmm1\{%k1\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 48 51 8c f4 c0 1d fe ff[ 	]*vpdpbusds -0x1e240\(%esp,%esi,8\),%zmm1,%zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 48 51 4a 7f[ 	]*vpdpbusds 0x1fc0\(%edx\),%zmm1,%zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f2 75 58 51 4a 7f[ 	]*vpdpbusds 0x1fc\(%edx\)\{1to16\},%zmm1,%zmm1
#pass
