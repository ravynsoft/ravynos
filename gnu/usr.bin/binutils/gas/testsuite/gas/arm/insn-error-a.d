# Check that changing arm -> thumb state immediately 
# after an invalid instruction does not cause an internal error.
#name: invalid instruction recovery test - ARM version
#objdump: -d --prefix-addresses --show-raw-insn
#skip: *-*-pe *-*-wince
#error_output: insn-error-a.l
