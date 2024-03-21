#name: bad MVE VQADD and VQSUB instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vqaddsub-bad.l

.*: +file format .*arm.*
