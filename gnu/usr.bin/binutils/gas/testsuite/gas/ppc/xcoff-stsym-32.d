#as: -a32
#source: xcoff-stsym.s
#objdump: -t
#name: XCOFF C_STSYM test (32-bit)

.*

SYMBOL TABLE:
.*
.*
.*
.*
\[  4\]\(sec  1\).*\(scl 143\) \(nx 0\) 0x0000000a .bs
\[  5\]\(sec -2\).*\(scl 133\) \(nx 0\) 0x00000000 x:V6
\[  6\]\(sec  1\).*\(scl 144\) \(nx 0\) 0x00000000 .es
\[  7\]\(sec  1\).*\(scl 143\) \(nx 0\) 0x0000000a .bs
\[  8\]\(sec -2\).*\(scl 133\) \(nx 0\) 0x00000004 y:V6
\[  9\]\(sec  1\).*\(scl 144\) \(nx 0\) 0x00000000 .es
\[ 10\].* _main\.rw_
.*


