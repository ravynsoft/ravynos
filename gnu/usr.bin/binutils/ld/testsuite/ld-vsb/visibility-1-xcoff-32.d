#source: define.s
#source: undef.s
#as: -a32 --defsym XCOFF_TEST=1
#ld: -b32 -r
#objdump: -t

.*

SYMBOL TABLE:
.*
.*
.*
.*
\[  4\]\(sec  2\).*\(ty 3000\).*protected
.*
\[  6\]\(sec  2\).*\(ty 2000\).*hidden
.*
\[  8\]\(sec  2\).*\(ty 1000\).*internal
.*
.*
.*
