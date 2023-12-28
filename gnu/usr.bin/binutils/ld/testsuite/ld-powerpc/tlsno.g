#source: tls.s
#source: tlslib.s
#as: -a64
#ld: --no-tls-optimize
#objdump: -sj.got
#target: powerpc64*-*-*

.*

Contents of section \.got:
.* (0+ 10018200 0+ 0+01|00820110 0+ 010+ 0+)  .*
.* (f+ f+8000 f+ f+8018|0080f+ f+ 1880f+ f+)  .*
.* (0+ 0+01 f+ f+8078|010+ 0+ 7880f+ f+)  .*
.* (f+ f+8058 0+ 0+01|5880f+ f+ 010+ 0+)  .*
.* (f+ f+8040 f+ f+9060|4080f+ f+ 6090f+ f+)  .*
.* (0+ 0+01|010+ 0+) 0+ 0+ .*
