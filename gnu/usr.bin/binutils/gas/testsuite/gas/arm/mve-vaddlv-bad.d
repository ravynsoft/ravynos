#name: Bad MVE VADDLV instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vaddlv-bad.l

.*: +file format .*arm.*
