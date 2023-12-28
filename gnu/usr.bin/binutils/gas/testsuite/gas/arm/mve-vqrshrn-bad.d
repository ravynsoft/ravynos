#name: bad MVE VQRSHRNT, VQRSHRNB, VQRHSRUNT and MVQRSHRUNB instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vqrshrn-bad.l

.*: +file format .*arm.*
