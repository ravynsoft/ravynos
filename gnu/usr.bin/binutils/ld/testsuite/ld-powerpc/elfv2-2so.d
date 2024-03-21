#source: elfv2-2a.s
#source: elfv2-2b.s
#as: -a64
#ld: -melf64ppc -shared -e f1
#error: .* R_PPC64_ADDR64_LOCAL reloc unsupported in shared libraries and PIEs.*
