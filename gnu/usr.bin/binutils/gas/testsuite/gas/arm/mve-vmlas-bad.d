#name: Bad MVE VMLAS instructions
#as: -march=armv8.1-m.main+mve
#error_output: mve-vmlas-bad.l

.*: +file format .*arm.*
