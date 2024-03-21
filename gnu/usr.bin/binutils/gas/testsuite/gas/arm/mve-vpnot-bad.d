#name: bad MVE VPNOT instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vpnot-bad.l

.*: +file format .*arm.*
