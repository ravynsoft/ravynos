#name: bad MVE VQMOVNT and VQMOVNB instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vqmovn-bad.l

.*: +file format .*arm.*
