#objdump : -s -j .data -j "\$DATA\$"
#name : .sleb128 tests (4)
# RISC-V doesn't support .sleb operands that are the difference of two symbols
# because symbol values are not known until after linker relaxation has been
# performed.
#xfail: riscv*-*-*

.*: .*

Contents of section (\.data|\$DATA\$):
 .* 83808080 082a.*
