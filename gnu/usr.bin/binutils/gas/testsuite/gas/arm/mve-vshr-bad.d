#name: bad MVE VSHR and VRSHR instructions
#as: -march=armv8.1-m.main+mve
#error_output: mve-vshr-bad.l

.*: +file format .*arm.*
