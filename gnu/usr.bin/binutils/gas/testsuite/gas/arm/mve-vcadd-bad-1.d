#name: bad MVE VCADD instructions
#as: -march=armv8.1-m.main+mve
#error_output: mve-vcadd-bad-1.l

.*: +file format .*arm.*
