#name: bad MVE VIDUP and VIWDUP instructions
#as: -march=armv8.1-m.main+mve
#error_output: mve-vidup-bad.l

.*: +file format .*arm.*
