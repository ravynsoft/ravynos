#name: Bad MVE VMLAV instructions
#as: -march=armv8.1-m.main+mve
#error_output: mve-vmlav-bad.l

.*: +file format .*arm.*
