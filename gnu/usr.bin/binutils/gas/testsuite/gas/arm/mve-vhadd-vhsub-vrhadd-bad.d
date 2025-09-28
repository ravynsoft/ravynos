#name: bad MVE VHADD, VHSUB and VRHADD instructions
#as: -march=armv8.1-m.main+mve
#error_output: mve-vhadd-vhsub-vrhadd-bad.l

.*: +file format .*arm.*
