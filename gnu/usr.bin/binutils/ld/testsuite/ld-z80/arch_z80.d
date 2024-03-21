#name: Z80 arch test
#source: dummy1.s -z80
#source: dummy2.s -z80
#ld: -e 0
#objdump: -f

.*:[     ]+file format (coff)|(elf32)\-z80
architecture: z80, flags 0x[0-9a-fA-F]+:
.*
.*

