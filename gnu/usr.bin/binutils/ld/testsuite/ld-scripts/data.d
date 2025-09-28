#source: data.s
#ld: -T data.t
#objdump: -s -j .text
#notarget: [is_aout_format]
#xfail: tic4x-*-* tic54x-*-*

.*:     file format .*

Contents of section .text:
 [0-9a-f]* (04)?000000(04)? (0020)?0000(2000)? .*
#pass
