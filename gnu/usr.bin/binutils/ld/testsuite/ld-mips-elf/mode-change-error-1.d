#name: Mode Change Error 1
#source: mode-change-error-1a.s
#source: mode-change-error-1b.s
#ld: -e 0x8000000
#error: \A[^\n]*: in function `main':\n
#error:   \(\.text\+0x0\): unsupported jump between ISA modes; consider recompiling with interlinking enabled\n
#error:   \(\.text\+0x8\): unsupported jump between ISA modes; consider recompiling with interlinking enabled\Z
