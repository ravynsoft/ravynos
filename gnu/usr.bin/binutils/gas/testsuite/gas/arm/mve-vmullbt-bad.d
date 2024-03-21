#name: bad MVE VMULL instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vmullbt-bad.l

.*: +file format .*arm.*
