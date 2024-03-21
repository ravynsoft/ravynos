#name: bad MVE VST2/4 VLD2/4 instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vstld-bad.l

.*: +file format .*arm.*
