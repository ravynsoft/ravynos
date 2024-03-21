#name: bad MVE VQDMULLT and VQDMULLB instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vqdmull-bad.l

.*: +file format .*arm.*
