#source: tls.s
#as: -a64
#ld: --no-tls-optimize tmpdir/libtlslib.so
#objdump: -sj.got
#target: powerpc64*-*-*

.*

Contents of section \.got:
.* (0+ 10018600 0+ 0+01|00860110 0+ 010+ 0+)  .*
.* (f+ f+8000 f+ f+8018|0080f+ f+ 1880f+ f+)  .*
.* 0+ 0+ 0+ 0+  .*
.* 0+ 0+ 0+ 0+  .*
.* (f+ f+8050 0+ 0+01|5080f+ f+ 010+ 0+)  .*
.* (f+ f+8038 f+ f+9058|3880f+ f+ 5890f+ f+)  .*
.* (0+ 0+01|010+ 0+) 0+0 0+  .*
