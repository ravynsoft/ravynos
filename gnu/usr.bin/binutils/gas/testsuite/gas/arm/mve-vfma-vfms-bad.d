#name: bad MVE FP VFMA and VFMS instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vfma-vfms-bad.l

.*: +file format .*arm.*
