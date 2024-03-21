#as: -a64
#source: xcoff-visibility-2.s
#objdump: -t
#name: XCOFF Visibility 2 (64 bit)

.*

SYMBOL TABLE:
.*
.*
\[  2\].*\(ty 1000\).*internal
.*
\[  4\].*\(ty 2000\).*hidden
.*
\[  6\].*\(ty 3000\).*protected
.*
\[  8\].*\(ty 1000\).*dual
.*
