#as:  -march=score3 -I${srcdir}/${subdir} -EL
#objdump:  -s
#source:  arith_32.s

.*:     file format elf32-littlescore

Contents of section .text:
 0000 0f480f48 0f480f48 0f480f48 0f480f48  .*
 0010 0f480080 113c0080 10401082 10001082  .*
 0020 10440180 10080f49 0f490f49 0f490f49  .*
 0030 0f490f49 0f490f49 0080153c 00801440  .*
 0040 10821400 10821444 01801408 205c1f5c  .*
 0050 e05fdf5f 205c205c 205c205c 205c205c  .*
 0060 205c205c 0384c17f 0384be7f 00844000  .*
 0070 0386c07f 00863e00                    .*
#pass
