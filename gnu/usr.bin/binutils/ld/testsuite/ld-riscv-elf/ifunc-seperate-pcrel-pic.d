#name: Link shared IFUNC resolver with PCREL caller (pic)
#source: ifunc-seperate-caller-pcrel.s
#as:
#ld: -z nocombreloc -shared tmpdir/ifunc-seperate-resolver.so
#error: .*unresolvable R_RISCV_PCREL_HI20 relocation.*
