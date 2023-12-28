#source: nan-2008.s
#source: nan-legacy.s
#ld: -r
#error: \A[^\n]*: [^\n]* linking -mnan=legacy module with previous -mnan=2008 modules\n
#error:   [^\n]*: failed to merge target specific data of file [^\n]*\.o\Z
