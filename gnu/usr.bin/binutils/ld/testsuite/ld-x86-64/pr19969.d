#source: pr19969b.S
#as: --64
#ld: -melf_x86_64 tmpdir/pr19969.so
#error: .*relocation R_X86_64_32 against symbol `foo' can not be used when making a PDE object; recompile with -fPIE
