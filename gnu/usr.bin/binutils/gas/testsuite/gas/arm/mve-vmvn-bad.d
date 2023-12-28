#name: bad MVE VMVN instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vmvn-bad.l

.*: +file format .*arm.*
