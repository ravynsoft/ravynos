#name: Weak test 3 (main, static) (32-bit)
#source: aix-weak-3b.s
#as: -a32 --defsym size=32
#ld: -b32 -e.main -bnoautoimp tmpdir/aix-weak-3a.so
#error: .*multiple definition of `x1';[^\n]*first defined here
