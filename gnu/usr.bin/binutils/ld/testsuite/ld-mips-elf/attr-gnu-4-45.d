#source: attr-gnu-4-4.s -W -mips32r2
#source: attr-gnu-4-5.s
#ld: -r
#error: \A[^\n]*: [^\n]* linking -mfp32 module with previous -mfp64 modules\n
#error:   [^\n]*: warning: .* uses -mips32r2 -mfp64 \(12 callee-saved\) \(set by .*\), .* uses -mfpxx\n
#error:   [^\n]*: failed to merge target specific data of file [^\n]*\.o\Z
