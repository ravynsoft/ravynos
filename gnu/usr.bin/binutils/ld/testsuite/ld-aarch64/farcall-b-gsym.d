#name: aarch64-farcall-b-gsym
#source: farcall-b-gsym.s
#as:
#ld: -Ttext 0x1000
#error: .*\(.text\+0x0\): relocation truncated to fit: R_AARCH64_JUMP26 against symbol `bar_gsym'.*
