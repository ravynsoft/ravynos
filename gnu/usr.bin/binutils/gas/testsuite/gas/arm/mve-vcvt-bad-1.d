#name: bad MVE VCVT (between floating-point and fixed-point) instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vcvt-bad-1.l

.*: +file format .*arm.*

