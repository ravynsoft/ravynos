#name: bad MVE VSBC instructions
#as: -march=armv8.1-m.main+mve
#error_output: mve-vsbc-bad.l

.*: +file format .*arm.*
