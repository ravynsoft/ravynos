#source: tls.s
#source: tlslib.s
#as: -a64
#ld:
#objdump: -sj.got
#target: powerpc64*-*-*

.*

Contents of section \.got:
 10010200 (00000000|00820110) (10018200|00000000) (ffffffff|1880ffff) (ffff8018|ffffffff)  .*
.* (ffffffff|5880ffff) (ffff8058|ffffffff)                    .*
