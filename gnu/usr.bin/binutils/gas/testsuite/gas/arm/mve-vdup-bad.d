#name: bad MVE VDUP instructions
#as: -march=armv8.1-m.main+mve
#error_output: mve-vdup-bad.l

.*: +file format .*arm.*
