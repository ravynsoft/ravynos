#name: R800 arch test
#source: dummy1.s
#source: dummy2.s
#as: -march=r800
#ld: -e 0
#objdump: -f

.*:[     ]+file format (coff)|(elf32)\-z80
architecture: r800, flags 0x[0-9a-fA-F]+:
.*
.*

