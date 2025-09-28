#source: static.s
#ld: -T discard.ld
#error: `(\.data\.exit|data)' referenced in section `\.text' of tmpdir/static.o: defined in discarded section `\.data\.exit' of tmpdir/static.o
#objdump: -p
#xfail: [is_generic]
#pass
