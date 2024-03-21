#source: align-small-region.s
#as: -march=rv32i
#ld: -melf32lriscv --relax -Talign-small-region.ld --defsym=_start=0x100
#objdump: -d

.*:[ 	]+file format .*

Disassembly of section \.entry:

00000000 <_reset>:
.*:[ 	]+[0-9a-f]+[ 	]+j[ 	]+100[ 	]+<_start>
#pass
