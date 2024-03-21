# name: csky - dsp
#as: -mcpu=ck810e -W
#objdump: -D

.*: +file format .*csky.*

Disassembly of section \.text:
#...
\s*[0-9a-f]*:\s*c4818820\s*mulu\s*r1,\s*r4
\s*[0-9a-f]*:\s*c6ec8840\s*mulua\s*r12,\s*r23
\s*[0-9a-f]*:\s*c46f8880\s*mulus\s*r15,\s*r3
\s*[0-9a-f]*:\s*c4418c20\s*muls\s*r1,\s*r2
\s*[0-9a-f]*:\s*c4428c40\s*mulsa\s*r2,\s*r2
\s*[0-9a-f]*:\s*c4638c80\s*mulss\s*r3,\s*r3
\s*[0-9a-f]*:\s*c6689040\s*mulsha\s*r8,\s*r19
\s*[0-9a-f]*:\s*c4319080\s*mulshs\s*r17,\s*r1
\s*[0-9a-f]*:\s*c6ec9440\s*mulswa\s*r12,\s*r23
\s*[0-9a-f]*:\s*c4a39500\s*mulsws\s*r3,\s*r5
#...
