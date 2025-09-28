# name: csky - v2-float-part2
#as: -mcpu=ck807f -W
#objdump: -D

.*: +file format .*csky.*

Disassembly of section \.text:
#...
\s*[0-9a-f]*:\s*f4003882\s*flrws\s*fr2,\s*3\.14
\s*[0-9a-f]*:\s*f50b1c02\s*fmovis\s*fr2,\s*1\.5
\s*[0-9a-f]*:\s*f51b1c02\s*fmovis\s*fr2,\s*-1\.5
\s*[0-9a-f]*:\s*f48a1c02\s*fmovis\s*fr2,\s*2\.5
#...
\s*[0-9a-f]*:\s*f4003952\s*flrwd\s*fr2,\s*3\.14
\s*[0-9a-f]*:\s*f48a1e02\s*fmovid\s*fr2,\s*2\.5
\s*[0-9a-f]*:\s*f49a1e02\s*fmovid\s*fr2,\s*-2\.5
\s*[0-9a-f]*:\s*f51b1e02\s*fmovid\s*fr2,\s*-1\.5
\s*[0-9a-f]*:\s*4048f5c3\s*\.long\s*0x4048f5c3
\s*[0-9a-f]*:\s*51eb851f\s*\.long\s*0x51eb851f
\s*[0-9a-f]*:\s*40091eb8\s*\.long\s*0x40091eb8
#...
