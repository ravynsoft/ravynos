#source: start.s
#source: symbol1ref.s
#source: symbol1w.s
#ld: -T group.ld
#warning: ^[^\n]*\): warning: witty one-liner$
#readelf: -s
#xfail: [is_generic]

# Check that warnings are generated for the .gnu.warning.SYMBOL
# construct and that the symbol still appears as expected.

#...
 +[0-9]+: +[0-9a-f]+ +[48] +FUNC +GLOBAL +DEFAULT +[1-9] symbol1
#pass
