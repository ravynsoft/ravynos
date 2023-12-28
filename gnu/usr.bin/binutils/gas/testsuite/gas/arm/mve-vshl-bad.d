#name: bad MVE VSHL instructions
#as: -march=armv8.1-m.main+mve
#error_output: mve-vshl-bad.l

.*: +file format .*arm.*
