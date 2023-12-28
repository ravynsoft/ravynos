#name: bad MVE VCVT (from floating-point to integer) instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vcvt-bad-4.l

.*: +file format .*arm.*
