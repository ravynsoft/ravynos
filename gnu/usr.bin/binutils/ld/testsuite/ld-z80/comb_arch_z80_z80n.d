#name: Z80/Z80N arch combination test
#source: dummy1.s -march=z80
#source: dummy2.s -march=z80n
#ld: -e 0
#objdump: -f

.*:[     ]+file format (coff|elf32)\-z80
architecture: z80n, flags 0x[0-9a-fA-F]+:
.*
.*

