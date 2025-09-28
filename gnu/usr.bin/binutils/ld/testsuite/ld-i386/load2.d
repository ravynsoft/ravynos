#as: --32 -mrelax-relocations=yes
#ld: -melf_i386 -shared
#error: direct GOT relocation R_386_GOT32X against `foo' without base register can not be used when making a shared object
