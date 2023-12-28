#name: bad MVE VQDMLSDH and VQRDMLSDH instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vqdmlsdh-bad.l

.*: +file format .*arm.*
