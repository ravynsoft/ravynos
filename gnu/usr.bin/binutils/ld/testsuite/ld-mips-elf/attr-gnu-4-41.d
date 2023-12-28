#source: attr-gnu-4-4.s -W
#source: attr-gnu-4-1.s
#ld: -r
#error: \A[^\n]*: [^\n]* linking -mfp32 module with previous -mfp64 modules\n
#error:   [^\n]*: warning: .* uses -mips32r2 -mfp64 \(12 callee-saved\) \(set by .*\), .* uses -mdouble-float\n
#error:   [^\n]*: failed to merge target specific data of file [^\n]*\.o\Z
