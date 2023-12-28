#name: bad MVE VMAXV, VMAXAV, VMIMV and VMINAV instructions
#as: -march=armv8.1-m.main+mve
#error_output: mve-vmaxv-vminv-bad.l

.*: +file format .*arm.*
