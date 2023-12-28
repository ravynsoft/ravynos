#name: bad MVE VCVT (between floating-point and integer) instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vcvt-bad-2.l

.*: +file format .*arm.*
