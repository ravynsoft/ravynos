#name: --gc-sections removing __stop_
#ld: --gc-sections -e _start
#nm: -n

#failif
#...
[0-9a-f]+ D +_?__stop__foo
#...
