# source: data.s
# ld: --enable-linker-version -T ld-version.t 
# readelf: -p.comment
# target: [is_elf_format]

String dump of section '.comment':
.*GNU ld \(.*\) 2.*
