#name: bad MVE VABS and VNEG instructions
#as: -march=armv8.1-m.main+mve
#error_output: mve-vabsneg-bad-1.l

.*: +file format .*arm.*

