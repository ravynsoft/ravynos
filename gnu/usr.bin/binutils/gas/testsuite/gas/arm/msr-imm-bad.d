# name: Cannot use MSR with immediates in thumb mode.
# skip: *-*-pe *-*-wince
# error_output: msr-imm-bad.l
# source: msr-imm.s
# as: -march=armv7-a -mthumb
