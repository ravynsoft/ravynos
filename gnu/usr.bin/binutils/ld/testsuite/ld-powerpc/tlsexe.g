#source: tls.s
#as: -a64
#ld: tmpdir/libtlslib.so
#objdump: -sj.got
#target: powerpc64*-*-*

.*

Contents of section \.got:
 10010600 (00000000|00860110) (10018600|00000000) (ffffffff|1880ffff) (ffff8018|ffffffff)  .*
.* 00000000 00000000 00000000 00000000  .*
.* 00000000 00000000 (ffffffff|5080ffff) (ffff8050|ffffffff)  .*
