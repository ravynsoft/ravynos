#source: tls32.s
#source: tlslib32.s
#as: -a32
#ld: --no-tls-optimize
#objdump: -sj.got
#target: powerpc*-*-*

.*

Contents of section \.got:
 1810144 (0+01 f+8000 0+01 f+803c|010+ 0080f+ 010+ 3c80f+)  .*
 1810154 (0+01 f+8020 f+9030 0+01|010+ 2080f+ 3090f+ 010+)  .*
 1810164 0+ 0+ 0+ 0+0  .*
