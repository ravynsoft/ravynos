#source: pr27491-1.s
#as: --32
#ld: --gc-sections -melf_i386 -z start-stop-gc -shared
#nm: -n

#failif
#...
[a-f0-9]+ R __start_xx
#...
[a-f0-9]+ R __stop_xx
#...
