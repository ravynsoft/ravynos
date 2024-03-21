#as: -linker-allocated-gregs
#source: start.s
#source: tm-orph1.s
#source: tm-awpe.s
#ld: -m mmo -u __etext -u __Sdata -u __Edata -u __Sbss -u __Ebss -u __Eall
#objdump: -rst

# Like orph-d-awp.d, but without .data contents.

.*:     file format mmo

SYMBOL TABLE:
0+ +g +\.text Main
0+8 +g +\.text __etext
20+ +g +\*ABS\* __TMC_END__
20+ +g +\*ABS\* __Ebss
20+ +g +\*ABS\* __Edata
0+ +g +\.text _start
20+ +g +\*ABS\* __Eall
20+ +g +\*ABS\* __Sdata
20+ +g +\*ABS\* __Sbss

Contents of section \.text:
 0000 e3fd0001 23fcfe00 .*
Contents of section \.MMIX\.reg_contents:
 07f0 20000000 00000007 .*
