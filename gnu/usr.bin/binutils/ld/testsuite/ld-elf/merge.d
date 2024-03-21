#source: merge.s
#ld: -T merge.ld
#objdump: -s
#xfail: [is_generic] hppa64-*-* ip2k-*-* nds32*-*-*

.*:     file format .*elf.*

Contents of section .rodata:
 1100 61626300 .*

Contents of section .data:
 1200 (0011)?0000(1100)? (0211)?0000(1102)? (04)?000000(04)? (02)?000000(02)? .*
#pass
