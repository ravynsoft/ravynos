#as: --EL
#source: reloc-insn64.s
#objdump: -s
#ld: -Tdata=0xdeadbeef1234 -EL
#name: reloc INSN64 little-endian

.*:     file format .*-bpfle

#...
Contents of section \.text:
[ 	][0-9a-f]+ 18010000 3812efbe 00000000 adde0000 .*
#pass
