#source: start.s
#source: symbol2ref.s
#source: symbol2w.s
#ld: -T group.ld
#warning: ^[^\n]*\.[obj]+: warning: function 'Foo' used$
#readelf: -s
# if not using elf.em, you don't get fancy section handling
#xfail: [uses_genelf]

# Check that warnings are generated for the symbols in .gnu.warning
# construct and that the symbol still appears as expected.

#...
 +[0-9]+: +[0-9a-f]+ +20 +OBJECT +GLOBAL +DEFAULT +[1-9] Foo
#pass
