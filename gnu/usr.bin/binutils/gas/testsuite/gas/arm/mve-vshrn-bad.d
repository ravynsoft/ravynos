#name: bad MVE VSHRN and VRSHRN instructions
#as: -march=armv8.1-m.main+mve
#error_output: mve-vshrn-bad.l

.*: +file format .*arm.*
