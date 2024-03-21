#source: pr17550-2.s
#source: pr17550-3.s
#ld: -r
#error: .*: defined in discarded section `\.data\[foo_group\]'
#xfail: alpha-*-* [is_generic]
# Disabled on alpha because alpha has a different .set directive.
# Generic linker targets don't support comdat group sections.
