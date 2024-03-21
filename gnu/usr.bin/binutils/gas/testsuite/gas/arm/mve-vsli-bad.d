#name: bad MVE VSLI instructions
#as: -march=armv8.1-m.main+mve
#error_output: mve-vsli-bad.l

.*: +file format .*arm.*
