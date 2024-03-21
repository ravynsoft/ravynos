#name: Weak test 3 (main, static) (64-bit)
#source: aix-weak-3b.s
#as: -a64 --defsym size=64
#ld: -b64 -e.main -bnoautoimp tmpdir/aix64-weak-3a.so
#error: .*multiple definition of `x1';[^\n]*first defined here
