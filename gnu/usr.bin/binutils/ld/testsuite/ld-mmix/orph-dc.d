#as: -linker-allocated-gregs
#source: start.s
#source: data-1.s
#source: tm-orph1.s
#source: tm-d.s
#source: tm-e.s
#ld: -m mmo -u __etext -u __Sdata -u __Edata -u __Sbss -u __Ebss -u __Eall
#objdump: -rst

# Like orph-d.d but with contents in the section without specified flags.

.*:     file format mmo

SYMBOL TABLE:
0+ +g +\.text Main
0+8 g +.text __etext
0+10 +g +\*ABS\* __TMC_END__
2000000000000008 g +\.data __Ebss
2000000000000008 g +\.data __Edata
0+ +g +\.text _start
2000000000000008 g +\.data __Eall
2000000000000000 g +\.data __Sdata
2000000000000008 g +\.data __Sbss

Contents of section \.text:
 0000 e3fd0001 23fcfe00 .*
Contents of section \.data:
 2000000000000004 00000042 .*
Contents of section \.tm_clone_table:
 0000 000004d2 0000162e 008adf38 00c8860c .*
Contents of section .MMIX.reg_contents:
 07f0 00000000 00000017 .*
