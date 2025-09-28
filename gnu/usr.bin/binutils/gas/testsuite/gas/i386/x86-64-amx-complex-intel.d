#as:
#objdump: -dw -Mintel
#name: x86_64 AMX-COMPLEX insns (Intel disassembly)
#source: x86-64-amx-complex.s

.*: +file format .*

Disassembly of section \.text:

0+ <_start>:
\s*[a-f0-9]+:\s*c4 e2 59 6c f5\s+tcmmimfp16ps tmm6,tmm5,tmm4
\s*[a-f0-9]+:\s*c4 e2 71 6c da\s+tcmmimfp16ps tmm3,tmm2,tmm1
\s*[a-f0-9]+:\s*c4 e2 58 6c f5\s+tcmmrlfp16ps tmm6,tmm5,tmm4
\s*[a-f0-9]+:\s*c4 e2 70 6c da\s+tcmmrlfp16ps tmm3,tmm2,tmm1
\s*[a-f0-9]+:\s*c4 e2 59 6c f5\s+tcmmimfp16ps tmm6,tmm5,tmm4
\s*[a-f0-9]+:\s*c4 e2 71 6c da\s+tcmmimfp16ps tmm3,tmm2,tmm1
\s*[a-f0-9]+:\s*c4 e2 58 6c f5\s+tcmmrlfp16ps tmm6,tmm5,tmm4
\s*[a-f0-9]+:\s*c4 e2 70 6c da\s+tcmmrlfp16ps tmm3,tmm2,tmm1
#pass
