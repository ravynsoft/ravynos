#name: Bad MVE VMLADAV instructions
#as: -march=armv8.1-m.main+mve
#error_output: mve-vmladav-bad.l

.*: +file format .*arm.*
