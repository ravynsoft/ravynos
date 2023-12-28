#source: link-hcs12.s -m68hcs12
#source: link-hc12.s -m68hc12
#as: -mshort
#ld: -m m68hc12elf  --script $srcdir/$subdir/far-hc12.ld
#objdump: -p -d --prefix-addresses -r

.*:     file format elf32\-m68hc12

Program Header:
    LOAD off    0x0+1000 vaddr 0x0+c000 paddr 0x0+c000 align 2\*\*12
         filesz 0x0+6 memsz 0x0+6 flags r-x
private flags = 22:\[abi=16\-bit int, 64\-bit double, cpu=HCS12\] \[memory=flat\]

Disassembly of section .text:
0+c000 <_start> jsr	0x0+c005 <main>
0+c003 <_start\+0x3> bra	0x0+c000 <_start>
0+c005 <main> rts


