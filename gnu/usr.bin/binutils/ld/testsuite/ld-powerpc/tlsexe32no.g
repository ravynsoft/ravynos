#source: tls32.s
#as: -a32
#ld: --no-tls-optimize tmpdir/libtlslib32.so
#objdump: -sj.got
#target: powerpc*-*-*

.*

Contents of section \.got:
.* (0+01 f+8000 0+ 0+|010+ 0080f+ 0+ 0+)  .*
.* (0+ 0+ 0+01 f+801c|0+ 0+ 010+ 1c80f+)  .*
.* (f+902c 0+01 0+ 018102dc|2c90f+ 010+ 0+ dc028101)  .*
.* 0+ 0+  .*
