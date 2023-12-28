#source: attr-gnu-4-3.s
#source: attr-gnu-4-4.s -W
#ld: -r
#error: \A[^\n]*: [^\n]* linking -mfp64 module with previous -mfp32 modules\n
#error:   [^\n]*: warning: .* uses -msoft-float \(set by .*\), .* uses -mhard-float\n
#error:   [^\n]*: failed to merge target specific data of file [^\n]*\.o\Z
