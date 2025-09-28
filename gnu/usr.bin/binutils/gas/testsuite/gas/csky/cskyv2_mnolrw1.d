# name: cskyv2 - nolrw
#as: -mcpu=ck810 -mnolrw -W
#objdump: -D

.*: +file format .*csky.*
#...
00000000 <LRW>:
\s*[0-9a-f]*:\s*ea220000\s*movih\s*r2,\s*0
\s*[0-9a-f]*:\s*ec420100\s*ori\s*r2,\s*r2,\s*256
\s*[0-9a-f]*:\s*ea220000\s*movih\s*r2,\s*0
\s*[0-9a-f]*:\s*ec421000\s*ori\s*r2,\s*r2,\s*4096
\s*[0-9a-f]*:\s*ea220001\s*movih\s*r2,\s*1
\s*[0-9a-f]*:\s*ec420000\s*ori\s*r2,\s*r2,\s*0
\s*[0-9a-f]*:\s*ea221234\s*movih\s*r2,\s*4660
\s*[0-9a-f]*:\s*ec421234\s*ori\s*r2,\s*r2,\s*4660
\s*[0-9a-f]*:\s*ea220000\s*movih\s*r2,\s*0
\s*[0-9a-f]*:\s*ec420000\s*ori\s*r2,\s*r2,\s*0
\s*[0-9a-f]*:\s*ea220000\s*movih\s*r2,\s*0
\s*[0-9a-f]*:\s*ec420000\s*ori\s*r2,\s*r2,\s*0
#...
00000030 <L1>:
\s*[0-9a-f]*:\s*6c8f\s*mov\s*r2,\s*r3
#...
