#name: --interleave test byte 0
#source: interleave.s
#ld: -Tinterleave.ld
#objcopy_linked_file: --interleave=8 --interleave-width=1 --byte=0
#objdump: -s

.*:     file format .*

Contents of section \.a0:
 0+0 00                                   .*
Contents of section \.a1:
 0+c 14                                   .*
