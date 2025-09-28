#source: start1.s
#source: got7.s
#source: tls128.s
#source: tls-gd-2.s
#source: tls-ld-6.s
#source: tls-ie-10.s
#source: tls-hx.s
#source: tls-hx1x2.s
#as: --no-underscore --em=criself -I$srcdir/$subdir --pic
#ld: -m crislinux
#objdump: -d -s -h -t -r -p

# Like tls-e-20.d but with an offset for all TLS GOT entries, and an
# offset for all TLS data we care about.  Offsets different for TLS
# than for GOT.

.*:     file format elf32-cris

Program Header:
    LOAD off    0x0+ vaddr 0x0+80000 paddr 0x0+80000 align 2\*\*13
         filesz 0x0+dc memsz 0x0+dc flags r-x
    LOAD off    0x0+dc vaddr 0x0+820dc paddr 0x0+820dc align 2\*\*13
         filesz 0x0+e4 memsz 0x0+e4 flags rw-
     TLS off    0x0+dc vaddr 0x0+820dc paddr 0x0+820dc align 2\*\*2
         filesz 0x0+8c memsz 0x0+8c flags r--
private flags = 0:

Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 \.text         0+48  0+80094  0+80094  0+94  2\*\*1
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  1 \.tdata        0+8c  0+820dc  0+820dc  0+dc  2\*\*2
                  CONTENTS, ALLOC, LOAD, DATA, THREAD_LOCAL
  2 \.got          0+3c  0+82168  0+82168  0+168  2\*\*2
                  CONTENTS, ALLOC, LOAD, DATA
  3 \.data         0+1c  0+821a4  0+821a4  0+1a4  2\*\*0
                  CONTENTS, ALLOC, LOAD, DATA
SYMBOL TABLE:
0+80094 l    d  \.text	0+ \.text
0+820dc l    d  \.tdata	0+ \.tdata
0+82168 l    d  \.got	0+ \.got
0+821a4 l    d  \.data	0+ \.data
0+ l    df \*ABS\*	0+ .*
0+ l       \.tdata	0+80 tls128
0+ l    df \*ABS\*	0+ .*
0+80 l       \.tdata	0+4 x
0+82168 l     O \.got	0+ _GLOBAL_OFFSET_TABLE_
0+800c4 g     F \.text	0+6 tlsdsofn2
0+821b4 g     O \.data	0+4 got7var5
0+88 g       \.tdata	0+4 \.hidden x2
0+821ac g     O \.data	0+4 got7var3
0+821bc g     O \.data	0+4 got7var7
0+80098 g     F \.text	0+2a got7fn
0+821b8 g     O \.data	0+4 got7var6
0+80094 g       \.text	0+ _start
0+821c0 g       \.data	0+ __bss_start
0+821a4 g     O \.data	0+4 got7var1
0+821b0 g     O \.data	0+4 got7var4
0+800cc g     F \.text	0+6 tlsdsofn
0+84 g       \.tdata	0+4 \.hidden x1
0+821c0 g       \.data	0+ _edata
0+821c0 g       \.data	0+ _end
0+821a8 g     O \.data	0+4 got7var2
0+800d4 g     F \.text	0+6 tlsdsofn10

Contents of section \.text:
#...
Contents of section \.tdata:
 820dc 2f000000 00000000 00000000 00000000  .*
 820ec 00000000 00000000 00000000 00000000  .*
 820fc 00000000 00000000 00000000 00000000  .*
 8210c 00000000 00000000 00000000 00000000  .*
 8211c 00000000 00000000 00000000 00000000  .*
 8212c 00000000 00000000 00000000 00000000  .*
 8213c 00000000 00000000 00000000 00000000  .*
 8214c 00000000 00000000 00000000 00000000  .*
 8215c 28000000 29000000 2a000000           .*
Contents of section \.got:
 82168 00000000 00000000 00000000 01000000  .*
 82178 00000000 f4ffffff 01000000 80000000  .*
 82188 b4210800 ac210800 bc210800 b8210800  .*
 82198 a4210800 b0210800 a8210800           .*
Contents of section \.data:
 821a4 00000000 00000000 00000000 00000000  .*
 821b4 00000000 00000000 00000000           .*

Disassembly of section \.text:

00080094 <_start>:
   80094:	41b2                	moveq 1,\$r11
#...
00080098 <got7fn>:
   80098:	6fae 3000 0000      	move\.d 30 <tls128\+0x30>,\$r10
   8009e:	6fae 3800 0000      	move\.d 38 <tls128\+0x38>,\$r10
   800a4:	6fae 2400 0000      	move\.d 24 <tls128\+0x24>,\$r10
   800aa:	6fae 3400 0000      	move\.d 34 <tls128\+0x34>,\$r10
   800b0:	6fae 2000 0000      	move\.d 20 <tls128\+0x20>,\$r10
   800b6:	6fae 2c00 0000      	move\.d 2c <tls128\+0x2c>,\$r10
   800bc:	6fae 2800 0000      	move\.d 28 <tls128\+0x28>,\$r10
#...

000800c4 <tlsdsofn2>:
   800c4:	6fae 1800 0000      	move\.d 18 <tls128\+0x18>,\$r10
#...

000800cc <tlsdsofn>:
   800cc:	6fae 8000 0000      	move\.d 80 <x>,\$r10
#...

000800d4 <tlsdsofn10>:
   800d4:	6fae 1400 0000      	move\.d 14 <tls128\+0x14>,\$r10
#...
