# name: jbt - csky
#as: -mcpu=ck610 -fpic
#objdump: -D

.*: +file format .*csky.*

Disassembly of section \.text:
#...
\s*[0-9a-f]*:\s*e7ff\s*bt\s*0x0.*
\s*[0-9a-f]*:\s*e80b\s*bf\s*0x1a.*
\s*[0-9a-f]*:\s*2470\s*subi\s*r0,\s*r0,\s*8
\s*[0-9a-f]*:\s*9f00\s*st.w\s*r15,\s*\(r0,\s*0x0\)
\s*[0-9a-f]*:\s*f800\s*bsr\s*0xa.*
\s*[0-9a-f]*:\s*7102\s*lrw\s*r1,\s*0x10810.*
\s*[0-9a-f]*:\s*1cf1\s*addu\s*r1,\s*r1,\s*r15
\s*[0-9a-f]*:\s*8f00\s*ld.w\s*r15,\s*\(r0,\s*0x0\)
\s*[0-9a-f]*:\s*2070\s*addi\s*r0,\s*r0,\s*8
\s*[0-9a-f]*:\s*00c1\s*jmp\s*r1
\s*[0-9a-f]*:\s*0810\s*cprgr\s*r0,\s*cpr1
\s*[0-9a-f]*:\s*0001\s*sync
\s*[0-9a-f]*:\s*0000\s*bkpt
\s*[0-9a-f]*:\s*1200\s*mov\s*r0,\s*r0
#...
