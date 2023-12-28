#source: pr27491-1.s
#as: --64
#ld: --gc-sections -melf_x86_64 -z start-stop-gc -shared
#nm: -n

#failif
#...
[a-f0-9]+ R __start_xx
#...
[a-f0-9]+ R __stop_xx
#...
