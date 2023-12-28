#name: bad MVE VDDUP and VDWDUP instructions
#as: -march=armv8.1-m.main+mve
#error_output: mve-vddup-bad.l

.*: +file format .*arm.*

