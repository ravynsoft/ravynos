#source: pcrel-reloc.s
#source: pcrel-reloc-abs.s
#as: -march=rv64i -mabi=lp64
#ld: -melf64lriscv --no-pie --no-relax
#objdump: -d

.*:[ 	]+file format .*

Disassembly of section \.text:

[0-9a-f]+ <_start>:
.*auipc.*
.*lw.*# [0-9a-f]* <sym>
#pass
