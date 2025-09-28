#source: tlspic1.s
#source: tlspic2.s
#as:
#ld: -shared -melf32xtensa
#objdump: -sj.tdata
#target: xtensa*-*-linux*

.*: +file format elf32-xtensa-.e

Contents of section .tdata:
 *[0-9a-f]+ 0*110* 0*120* 0*130* 0*140*  .*
 *[0-9a-f]+ 0*150* 0*160* 0*170* 0*180*  .*
 *[0-9a-f]+ 0*410* 0*420* 0*430* 0*440*  .*
 *[0-9a-f]+ 0*450* 0*460* 0*470* 0*480*  .*
 *[0-9a-f]+ 0*9d0* 0*9e0* 0*9f0* 0*a00*  .*
 *[0-9a-f]+ 0*a10* 0*a20* 0*a30* 0*a40*  .*
