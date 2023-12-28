#name: bad MVE VABAV instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vabav-bad.l

.*: +file format .*arm.*

