#source: tlslib.s
#source: tlstoc.s
#as: -a64
#ld: --no-tls-optimize
#objdump: -sj.got
#target: powerpc64*-*-*

.*

Contents of section \.got:
.* (0+ 0+01 f+ f+8040|010+ 0+ 4080f+ f+)  .*
.* (0+ 0+01|010+ 0+) 0+ 0+  .*
.* (0+ 0+01 f+ f+8048|010+ 0+ 4880f+ f+)  .*
.* (0+ 0+01|010+ 0+) 0+ 0+  .*
.* (f+ f+8060 f+ f+9068|6080f+ f+ 6890f+ f+)  .*
