# name: cskyv2 - lrw
#as: -mcpu=ck810 -W
#objdump: -D

.*: +file format .*csky.*
#...
00000000 <LRW>:
\s*[0-9a-f]*:\s*ea020100\s*movi\s*r2,\s*256
\s*[0-9a-f]*:\s*ea021000\s*movi\s*r2,\s*4096
\s*[0-9a-f]*:\s*ea220001\s*movih\s*r2,\s*1
\s*[0-9a-f]*:\s*1042\s*lrw\s*r2,\s*0x12341234.*
\s*[0-9a-f]*:\s*1043\s*lrw\s*r2,\s*0x0.*
\s*[0-9a-f]*:\s*1041\s*lrw\s*r2,\s*0x12341234.*
#...
00000012 <L1>:
\s*[0-9a-f]*:\s*6c8f\s*mov\s*r2,\s*r3
\s*[0-9a-f]*:\s*12341234\s*\.long\s*0x12341234
\s*[0-9a-f]*:\s*00000000\s*\.long\s*0x00000000
#...
