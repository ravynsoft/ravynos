#source: start.s
#ld: tmpdir/symbol3w.o tmpdir/symbol3.a 
#warning: .*: warning: badsym warning$
#readelf: -s
#xfail: [is_generic]
# generic linker targets don't support .gnu.warning sections.

# Check that warnings are generated for the symbols in .gnu.warning
# construct and that the symbol still appears as expected.

#...
 +[0-9]+: +[0-9a-f]+ +4 +OBJECT +GLOBAL +DEFAULT +[1-9] badsym
#pass
