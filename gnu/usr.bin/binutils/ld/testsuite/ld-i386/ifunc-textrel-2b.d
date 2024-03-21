#source: ../ld-x86-64/ifunc-textrel-2.s
#as: --32
#ld: -m elf_i386 -shared -z notext
#readelf: -r --wide

#failif
[0-9a-f]+ +[0-9a-f]+ +R_386_IRELATIVE +
#..
