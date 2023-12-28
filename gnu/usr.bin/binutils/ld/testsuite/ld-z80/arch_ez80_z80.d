#name: eZ80 Z80 mode arch test
#source: dummy1.s
#source: dummy2.s
#as: -march=ez80
#ld: -e 0
#objdump: -f

.*:[     ]+file format (coff)|(elf32)\-z80
architecture: ez80-z80, flags 0x[0-9a-fA-F]+:
.*
.*

