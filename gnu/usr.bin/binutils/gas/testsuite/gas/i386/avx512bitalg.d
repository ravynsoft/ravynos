#as:
#objdump: -dw
#name: i386 AVX512BITALG insns
#source: avx512bitalg.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8f ec[ 	]*vpshufbitqmb %zmm4,%zmm5,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 4f 8f ec[ 	]*vpshufbitqmb %zmm4,%zmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8f ac f4 c0 1d fe ff[ 	]*vpshufbitqmb -0x1e240\(%esp,%esi,8\),%zmm5,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8f 6a 7f[ 	]*vpshufbitqmb 0x1fc0\(%edx\),%zmm5,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 54 f5[ 	]*vpopcntb %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 54 f5[ 	]*vpopcntb %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 54 f5[ 	]*vpopcntb %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 54 b4 f4 c0 1d fe ff[ 	]*vpopcntb -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 54 72 7f[ 	]*vpopcntb 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 54 f5[ 	]*vpopcntw %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 54 f5[ 	]*vpopcntw %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 54 f5[ 	]*vpopcntw %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 54 b4 f4 c0 1d fe ff[ 	]*vpopcntw -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 54 72 7f[ 	]*vpopcntw 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 f5[ 	]*vpopcntd %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 55 f5[ 	]*vpopcntd %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 55 f5[ 	]*vpopcntd %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 b4 f4 c0 1d fe ff[ 	]*vpopcntd -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 72 7f[ 	]*vpopcntd 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 58 55 72 7f[ 	]*vpopcntd 0x1fc\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 f5[ 	]*vpopcntq %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 55 f5[ 	]*vpopcntq %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 55 f5[ 	]*vpopcntq %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 b4 f4 c0 1d fe ff[ 	]*vpopcntq -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 72 7f[ 	]*vpopcntq 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 58 55 72 7f[ 	]*vpopcntq 0x3f8\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8f ec[ 	]*vpshufbitqmb %zmm4,%zmm5,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 4f 8f ec[ 	]*vpshufbitqmb %zmm4,%zmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8f ac f4 c0 1d fe ff[ 	]*vpshufbitqmb -0x1e240\(%esp,%esi,8\),%zmm5,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 8f 6a 7f[ 	]*vpshufbitqmb 0x1fc0\(%edx\),%zmm5,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 54 f5[ 	]*vpopcntb %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 54 f5[ 	]*vpopcntb %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 54 f5[ 	]*vpopcntb %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 54 b4 f4 c0 1d fe ff[ 	]*vpopcntb -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 54 72 7f[ 	]*vpopcntb 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 54 f5[ 	]*vpopcntw %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 54 f5[ 	]*vpopcntw %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 54 f5[ 	]*vpopcntw %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 54 b4 f4 c0 1d fe ff[ 	]*vpopcntw -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 54 72 7f[ 	]*vpopcntw 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 f5[ 	]*vpopcntd %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 55 f5[ 	]*vpopcntd %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 55 f5[ 	]*vpopcntd %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 b4 f4 c0 1d fe ff[ 	]*vpopcntd -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 55 72 7f[ 	]*vpopcntd 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 58 55 72 7f[ 	]*vpopcntd 0x1fc\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 f5[ 	]*vpopcntq %zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 55 f5[ 	]*vpopcntq %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 55 f5[ 	]*vpopcntq %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 b4 f4 c0 1d fe ff[ 	]*vpopcntq -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 55 72 7f[ 	]*vpopcntq 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 58 55 72 7f[ 	]*vpopcntq 0x3f8\(%edx\)\{1to8\},%zmm6
#pass
