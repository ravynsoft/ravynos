#name: bad MVE WLSTP, DLSTP and LETP instructions
#as: -march=armv8.1-m.main+mve
#error_output: mve-tailpredloop-bad.l

.*: +file format .*arm.*
