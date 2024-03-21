#name: Check require-defined does no error on a defined symbol
#source: require-defined.s
#ld: -e _start --require-defined=bar
#nm: -n

#...
[0-9a-f]+ T bar
#...
