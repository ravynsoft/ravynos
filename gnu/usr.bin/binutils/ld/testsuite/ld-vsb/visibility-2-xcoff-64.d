#source: undef.s
#as: -a64 --defsym XCOFF_TEST=1
#ld: -b64 -r
#objdump: -t

.*

SYMBOL TABLE:
.*
.*
\[  2\]\(sec  0\).*\(ty 3000\).*protected
.*
\[  4\]\(sec  0\).*\(ty 2000\).*hidden
.*
\[  6\]\(sec  0\).*\(ty 1000\).*internal
.*
