#source: ifunc-textrel-2.s
#as: --64 -defsym __x86_64__=1
#ld: -m elf_x86_64 -shared -z notext
#readelf: -r --wide

#failif
[0-9a-f]+ +[0-9a-f]+ +R_X86_64_IRELATIVE +[0-9a-f]+
#..
