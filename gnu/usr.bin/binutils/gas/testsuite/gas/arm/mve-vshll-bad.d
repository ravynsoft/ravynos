#name: bad MVE VSHLLT and VSHLLB instructions
#as: -march=armv8.1-m.main+mve
#error_output: mve-vshll-bad.l

.*: +file format .*arm.*
