#name: bad MVE VMAX and VMIN instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vmax-vmin-bad.l

.*: +file format .*arm.*
