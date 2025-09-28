#as: -a64
#source: xcoff-stsym.s
#objdump: -t
#name: XCOFF C_STSYM test (64-bit)

.*

SYMBOL TABLE:
.*
.*
.*
.*
\[  4\]\(sec  1\).*\(scl 143\) \(nx 0\) 0x000000000000000a .bs
\[  5\]\(sec -2\).*\(scl 133\) \(nx 0\) 0x0000000000000000 x:V6
\[  6\]\(sec  1\).*\(scl 144\) \(nx 0\) 0x0000000000000000 .es
\[  7\]\(sec  1\).*\(scl 143\) \(nx 0\) 0x000000000000000a .bs
\[  8\]\(sec -2\).*\(scl 133\) \(nx 0\) 0x0000000000000004 y:V6
\[  9\]\(sec  1\).*\(scl 144\) \(nx 0\) 0x0000000000000000 .es
\[ 10\].* _main\.rw_
.*


