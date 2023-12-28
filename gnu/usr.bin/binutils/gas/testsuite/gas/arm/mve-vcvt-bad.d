#name: bad MVE VCVT instructions 
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vcvt-bad.l

.*: +file format .*arm.*

