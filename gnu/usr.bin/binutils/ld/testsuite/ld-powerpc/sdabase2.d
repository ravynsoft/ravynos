#source: sdabase.s
#as: -a32
#ld: -melf32ppc -T sdabase2.t
#objdump: -s
#target: powerpc*-*-*

.*:     file format .*

Contents of section \.sdata:
 0400 (00000400|00040000) (00000400|00040000) .*
