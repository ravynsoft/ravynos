#name: Check require-defined can require a symbol from an object
#source: require-defined.s
#ld: -e _start --gc-sections --require-defined=bar
#nm: -n

#...
[0-9a-f]+ T bar
#...
