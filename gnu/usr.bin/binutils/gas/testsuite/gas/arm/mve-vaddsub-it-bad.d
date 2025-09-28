#name: Bad MVE vadd/vsub instructions in IT blocks
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vaddsub-it-bad.l

.*: +file format .*arm.*

