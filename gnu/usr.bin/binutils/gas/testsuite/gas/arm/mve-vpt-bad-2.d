#name: bad MVE FP VPT instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vpt-bad-2.l

.*: +file format .*arm.*
