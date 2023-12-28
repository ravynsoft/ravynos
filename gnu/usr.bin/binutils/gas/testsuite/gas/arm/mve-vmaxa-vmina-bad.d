#name: bad MVE VMAXA and VMINA instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vmaxa-vmina-bad.l

.*: +file format .*arm.*
