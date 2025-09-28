# name: cskyv2 - elrw
#as: -mcpu=ck801 -melrw
#objdump: -D

.*: +file format .*csky.*
#...
\s*[0-9a-f]*:\s*024e\s*lrw\s*r2,\s*0x1234.*
#...
\s*[0-9a-f]*:\s*c0004020\s*rte
\s*[0-9a-f]*:\s*00001234\s*\.long\s*0x00001234
#...
