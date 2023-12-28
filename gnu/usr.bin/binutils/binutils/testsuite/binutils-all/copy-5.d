#PROG: objcopy
#source: bintest.s
#objcopy: -G fred --globalize-symbol jim bintest.o bintest.copy.o
# A few targets cannot assemble the bintest.s source file...
#notarget: pdp11-* *-darwin
#name: Error when using --keep-global-symbol with --globalize-symbol
#error: \A[^\n]*: --globalize-symbol\(s\) is incompatible with -G/--keep-global-symbol\(s\)
