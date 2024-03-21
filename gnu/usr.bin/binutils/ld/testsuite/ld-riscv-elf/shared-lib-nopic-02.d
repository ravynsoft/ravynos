#source: shared-lib-nopic-02.s
#as:
#ld: -shared
#error: .*relocation R_RISCV_JAL against `foo_default' which may bind externally can not be used when making a shared object; recompile with -fPIC
