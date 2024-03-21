#name: bad MVE VSHLC instructions
#as: -march=armv8.1-m.main+mve
#error_output: mve-vshlc-bad.l

.*: +file format .*arm.*
