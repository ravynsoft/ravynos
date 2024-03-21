#as: --EB
#source: reloc-insn64.s
#objdump: -s
#ld: -Tdata=0xdeadbeef1234 -EB
#name: reloc INSN64 big-endian

.*:     file format .*-bpfbe

#...
Contents of section \.text:
[ 	][0-9a-f]+ 18100000 beef1238 00000000 0000dead .*
#pass
