#source: uleb128.s
#as: -mcpu=msp430x
#ld:
#objdump: -sj.text

.*:[ 	]+file format .*

Contents of section .text:
 [0-9a-f]+ 04000401 04840200.*
#pass
