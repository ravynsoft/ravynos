# name: csky - ck860
#as: -mcpu=ck860
#objdump: -D

.*: +file format .*csky.*

Disassembly of section \.text:
#...
\s*[0-9a-f]*:\s*c0008820\s*tlbi.all
\s*[0-9a-f]*:\s*c2008820\s*tlbi.alls
\s*[0-9a-f]*:\s*c0618820\s*tlbi.va\s*r1
\s*[0-9a-f]*:\s*c26f8820\s*tlbi.vas\s*r15
\s*[0-9a-f]*:\s*c0218820\s*tlbi.asid\s*r1
\s*[0-9a-f]*:\s*c22f8820\s*tlbi.asids\s*r15
\s*[0-9a-f]*:\s*c04f8820\s*tlbi.vaa\s*r15
\s*[0-9a-f]*:\s*c24f8820\s*tlbi.vaas\s*r15
\s*[0-9a-f]*:\s*c1df9420\s*dcache.cisw\s*r31
\s*[0-9a-f]*:\s*c1ff9420\s*dcache.civa\s*r31
\s*[0-9a-f]*:\s*c1809420\s*dcache.ciall
\s*[0-9a-f]*:\s*c0df9420\s*dcache.csw\s*r31
\s*[0-9a-f]*:\s*c0ff9420\s*dcache.cva\s*r31
\s*[0-9a-f]*:\s*c2ff9420\s*dcache.cval1\s*r31
\s*[0-9a-f]*:\s*c0809420\s*dcache.call
\s*[0-9a-f]*:\s*c15f9420\s*dcache.isw\s*r31
\s*[0-9a-f]*:\s*c17f9420\s*dcache.iva\s*r31
\s*[0-9a-f]*:\s*c1009420\s*dcache.iall
\s*[0-9a-f]*:\s*c1009020\s*icache.iall
\s*[0-9a-f]*:\s*c3009020\s*icache.ialls
\s*[0-9a-f]*:\s*c17f9020\s*icache.iva\s*r31
\s*[0-9a-f]*:\s*c000842f\s*bar.brwarw
\s*[0-9a-f]*:\s*c200842f\s*bar.brwarws
\s*[0-9a-f]*:\s*c0008425\s*bar.brar
\s*[0-9a-f]*:\s*c2008425\s*bar.brars
\s*[0-9a-f]*:\s*c000842a\s*bar.bwaw
\s*[0-9a-f]*:\s*c200842a\s*bar.bwaws
\s*[0-9a-f]*:\s*c2200420\s*sync.is
\s*[0-9a-f]*:\s*c0200420\s*sync.i
\s*[0-9a-f]*:\s*c2000420\s*sync.s
\s*[0-9a-f]*:\s*c0000420\s*sync
\s*[0-9a-f]*:\s*d8437000\s*ldex.w\s*r2,\s*\(r3,\s*0x0\)
\s*[0-9a-f]*:\s*dc437000\s*stex.w\s*r2,\s*\(r3,\s*0x0\)
#...
