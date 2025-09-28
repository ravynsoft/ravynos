#source: attr-gnu-4-7.s
#source: attr-gnu-4-4.s -W -mips32r2
#ld: -r
#error: \A[^\n]*: [^\n]* linking -mfp64 module with previous -mfp32 modules\n
#error:   [^\n]*: warning: .* uses -mgp32 -mfp64 -mno-odd-spreg \(set by .*\), .* uses -mips32r2 -mfp64 \(12 callee-saved\)\n
#error:   [^\n]*: failed to merge target specific data of file [^\n]*\.o\Z
