#name: bad MVE FP VFMAS instructions 
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vfmas-bad.l

.*: +file format .*arm.*
