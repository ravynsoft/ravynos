#name: bad MVE VHCADD instructions
#as: -march=armv8.1-m.main+mve
#error_output: mve-vhcadd-bad.l

.*: +file format .*arm.*
