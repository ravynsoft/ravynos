#source: pr20244-2.s
#as: --32
#ld: -pie -m elf_i386
#error: direct GOT relocation R_386_GOT32X against `foo' without base register can not be used when making a shared object
