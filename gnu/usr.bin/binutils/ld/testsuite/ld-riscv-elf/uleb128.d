#source: uleb128.s
#as: -march=rv32ic
#ld: -melf32lriscv
#objdump: -d

.*:[ 	]+file format .*

Disassembly of section .text:

.* <_start>:
.*jal.*<bar>
.*jal.*<bar>
.*jal.*<bar>
.*jal.*<bar>
.*jal.*<bar>
.*jal.*<bar>
.*:[ 	]+0e0c.*
#pass
