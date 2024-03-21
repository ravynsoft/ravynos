#source: attr-gnu-4-1.s
#source: attr-gnu-4-4.s -W
#ld: -r
#error: \A[^\n]*: [^\n]* linking -mfp64 module with previous -mfp32 modules\n
#error:   [^\n]*: warning: .* uses -mdouble-float \(set by .*\), .* uses -mips32r2 -mfp64 \(12 callee-saved\)\n
#error:   [^\n]*: failed to merge target specific data of file [^\n]*\.o\Z
