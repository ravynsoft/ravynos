#as: -linker-allocated-gregs
#source: start.s
#source: data-1.s
#source: tm-orph1.s
#source: tm-d-ap.s
#source: tm-ape.s
#ld: -m mmo -u __etext -u __Sdata -u __Edata -u __Sbss -u __Ebss -u __Eall
#objdump: -rst

# Like orph-d-a.d but with contents in that section (and with
# @progbits, which doesn't reflect in flags), making sure it's really
# treated as .text contents.

.*:     file format mmo

SYMBOL TABLE:
0+ +g +\.text Main
0+18 +g +\*ABS\* __etext
0+18 +g +\*ABS\* __TMC_END__
20+8 +g +\.data __Ebss
20+8 +g +\.data __Edata
0+ +g +\.text _start
20+8 +g +\.data __Eall
20+ +g +\.data __Sdata
20+8 +g +\.data __Sbss

Contents of section \.text:
 0000 e3fd0001 23fcfe00  .*
Contents of section \.tm_clone_table:
 0008 000004d2 0000162e 008adf38 00c8860c  .*
Contents of section \.data:
 2000000000000004 00000042 .*
Contents of section \.MMIX\.reg_contents:
 07f0 00000000 0000001f .*
