#name: jalr zero-offset symbols
#source: disas-jalr.s
#ld: --no-relax
#objdump: -d

.*:[ 	]+file format .*

Disassembly of section \.text:

.* <_start>:
#...
.*:[ 	]+fffff097[ 	]+auipc[ 	]+ra,0xfffff
.*:[ 	]+000080e7[ 	]+jalr[ 	]+ra # .* <_start>
