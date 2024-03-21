#as: --64
#ld: -shared -melf_x86_64
#readelf: -r --wide

#...
[0-9a-f]+ +[0-9a-f]+ +R_X86_64_GLOB_DAT +[0-9a-f]+ +foo \+ 0
