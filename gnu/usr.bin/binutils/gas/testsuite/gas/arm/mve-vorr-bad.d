#name: bad MVE VORR instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vorr-bad.l

.*: +file format .*arm.*
