#as: -linker-allocated-gregs
#source: start.s
#source: data-1.s
#source: tm-orph1.s
#source: tm-d-awp.s
#source: tm-awpe.s
#ld: -m mmo -u __etext -u __Sdata -u __Edata -u __Sbss -u __Ebss -u __Eall
#objdump: -rst

# Like orph-d-awp.d, but with contents in the extra section.

.*:     file format mmo

SYMBOL TABLE:
0+ +g +\.text Main
0+8 g +\.text __etext
2000000000000018 +g +\*ABS\* __TMC_END__
2000000000000018 g +\*ABS\* __Ebss
2000000000000018 g +\*ABS\* __Edata
0+ g +\.text _start
2000000000000018 g +\*ABS\* __Eall
20+ g +.data __Sdata
2000000000000018 g +\*ABS\* __Sbss

Contents of section \.text:
 0000 e3fd0001 23fcfe00  .*
Contents of section \.data:
 2000000000000004 00000042  .*
Contents of section \.tm_clone_table:
 2000000000000008 000004d2 0000162e 008adf38 00c8860c  .*
Contents of section \.MMIX\.reg_contents:
 07f0 20000000 0000001f .*
