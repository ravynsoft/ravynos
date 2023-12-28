#source: pr20244-1.s
#as: --32
#ld: -pie -m elf_i386
#error: direct GOT relocation R_386_GOT32 against `bar' without base register can not be used when making a shared object
