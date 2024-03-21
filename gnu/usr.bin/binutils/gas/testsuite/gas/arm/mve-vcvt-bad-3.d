#name: bad MVE VCVT (between single and half precision floating-point) instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vcvt-bad-3.l

.*: +file format .*arm.*
