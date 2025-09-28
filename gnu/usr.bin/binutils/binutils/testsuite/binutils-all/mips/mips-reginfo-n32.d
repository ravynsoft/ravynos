#PROG: objcopy
#name: MIPS objcopy .reginfo section size (n32)
#objcopy: --rename-section .foo=.reginfo
#source: mips-reginfo.s
#error: \A[^\n]*: incorrect `\.reginfo' section size; expected 24, got 4\n
#error:   [^\n]*: bad value\Z
