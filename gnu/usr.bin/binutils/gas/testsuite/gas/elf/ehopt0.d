#objdump: -s -j .eh_frame
#name: elf ehopt0 
# The loongarch target do not evaluate .eh_frame fde cfa advance loc at assembly time.
# Because loongarch use add/sub reloc evaluate cfa advance loc, so gas should write 0 to cfa advance loc address.
#xfail: loongarch*-*

.*: +file format .*

Contents of section .eh_frame:
 0+000 (10|00)0000(00|10) 00000000 017a0001 781a0004 .*
 0+010 (01|00)0000(00|01) (12|00)0000(00|12) (18|00)0000(00|18) 00000000 .*
 0+020 (08|00)0000(00|08) 04(08|00)0000 (00|08)44 .*
