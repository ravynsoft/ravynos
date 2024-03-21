#as: --x32
#ld: -m elf32_x86_64 -shared
#readelf: -r --wide

#...
[0-9a-f]+ +[0-9a-f]+ +R_X86_64_RELATIVE +[0-9a-f]+
[0-9a-f]+ +[0-9a-f]+ +R_X86_64_GLOB_DAT +[0-9a-f]+ +foo \+ 0
[0-9a-f]+ +[0-9a-f]+ +R_X86_64_32 +[0-9a-f]+ +foo \+ 0
