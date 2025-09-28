#name: bad MVE VMULH and VRMULH instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vmulh-bad.l

.*: +file format .*arm.*
