#name: bad MVE VSRI instructions
#as: -march=armv8.1-m.main+mve
#error_output: mve-vsri-bad.l

.*: +file format .*arm.*
