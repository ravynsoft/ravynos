#name: bad MVE VBIC instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vbic-bad.l

.*: +file format .*arm.*
