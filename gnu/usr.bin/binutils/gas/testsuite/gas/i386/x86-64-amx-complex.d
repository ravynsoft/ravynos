#as:
#objdump: -dw
#name: x86_64 AMX-COMPLEX insns
#source: x86-64-amx-complex.s

.*: +file format .*

Disassembly of section \.text:

0+ <_start>:
\s*[a-f0-9]+:\s*c4 e2 59 6c f5\s+tcmmimfp16ps %tmm4,%tmm5,%tmm6
\s*[a-f0-9]+:\s*c4 e2 71 6c da\s+tcmmimfp16ps %tmm1,%tmm2,%tmm3
\s*[a-f0-9]+:\s*c4 e2 58 6c f5\s+tcmmrlfp16ps %tmm4,%tmm5,%tmm6
\s*[a-f0-9]+:\s*c4 e2 70 6c da\s+tcmmrlfp16ps %tmm1,%tmm2,%tmm3
#pass
