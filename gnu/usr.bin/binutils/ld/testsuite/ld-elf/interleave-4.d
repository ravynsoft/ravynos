#name: --interleave test byte 4
#source: interleave.s
#ld: -Tinterleave.ld
#objcopy_linked_file: --interleave=8 --interleave-width=1 --byte=4
#objdump: -s

.*:     file format .*

Contents of section \.a0:
 0+0 04                                   .*
Contents of section \.a1:
 0+c 10                                   .*
