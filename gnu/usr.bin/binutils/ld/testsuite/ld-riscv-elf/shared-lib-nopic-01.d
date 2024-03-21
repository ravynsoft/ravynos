#source: shared-lib-nopic-01.s
#as:
#ld: -shared
#error: .*relocation R_RISCV_JAL against `foo' which may bind externally can not be used when making a shared object; recompile with -fPIC
