#source: tlstoc.s
#as: -a64
#ld: --no-tls-optimize tmpdir/libtlslib.so
#objdump: -sj.got
#target: powerpc64*-*-*

.*

Contents of section \.got:
.* (0+ 10018600 0+ 0+|00860110 0+ 0+ 0+)  .*
.* 0+ 0+ 0+ 0+  .*
.* 0+ 0+ (0+ 0+01|010+ 0+)  .*
.* (f+ f+8038 0+ 0+01|3880f+ f+ 010+ 0+)  .*
.* 0+ 0+ (f+ f+8050|5080f+ f+)  .*
.* (f+ f+9058|5890f+ f+)  .*
