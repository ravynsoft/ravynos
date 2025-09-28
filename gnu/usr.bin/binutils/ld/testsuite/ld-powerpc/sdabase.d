#source: sdabase.s
#as: -a32
#ld: -melf32ppc -T sdabase.t
#objdump: -s
#target: powerpc*-*-*

.*:     file format .*

Contents of section \.sdata:
 0400 (00008400|00840000) (00000400|00040000) .*
