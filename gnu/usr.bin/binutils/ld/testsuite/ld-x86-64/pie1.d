#name: PIE with undefined symbol
#as: --64
#ld: -pie -melf_x86_64 --noinhibit-exec -z notext
#readelf: -s --wide
#warning: \A[^\n]*\.o[^\n]*in function `_start':\n[^\n]*: undefined reference to `foo'\Z

#failif
#...
 +[0-9]+: +[0-9a-f]+ +0 +NOTYPE +GLOBAL +DEFAULT +UND foo
#...
