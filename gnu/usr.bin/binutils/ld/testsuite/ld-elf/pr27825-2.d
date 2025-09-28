#source: pr27825-2a.s
#source: pr27825-2b.s
#source: pr27825-2c.s
#ld: -e _start --emit-relocs -z unique-symbol
#nm: --defined-only
#xfail: [uses_genelf]
# These targets don't support -z.

#...
[0-9a-f]+ t bar.0
#...
[0-9a-f]+ t bar.1
#...
[0-9a-f]+ t bar.1.0
#...
[0-9a-f]+ t bar.2.0
#pass
