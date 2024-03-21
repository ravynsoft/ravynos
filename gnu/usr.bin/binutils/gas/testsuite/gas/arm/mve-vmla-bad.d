#name: Bad MVE VMLA instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vmla-bad.l

.*: +file format .*arm.*
