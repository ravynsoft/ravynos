#source: merge3.s
#ld: -T merge.ld
#objdump: -s
#xfail: [is_generic] hppa64-*-* ip2k-*-*

.*:     file format .*elf.*

Contents of section \.rodata:
 1100 64656667 00000000 30313233 34353637  defg....01234567
 1110 61626364 65666700                    abcdefg.        

Contents of section \.data:
 1200 (10110000|00001110) (00110000|00001100) (08110000|00001108) .*
#pass
