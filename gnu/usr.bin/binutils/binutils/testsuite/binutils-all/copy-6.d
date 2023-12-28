#PROG: objcopy
#source: bintest.s
#objcopy: -O ihex
# A few targets cannot assemble the bintest.s source file...
#notarget: pdp11-* *-darwin
#name: ihex objcopy test
#objdump: -h
#...
