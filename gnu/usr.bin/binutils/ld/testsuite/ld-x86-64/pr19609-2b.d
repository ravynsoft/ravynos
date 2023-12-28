#source: pr19609-2.s
#as: --x32 -mrelax-relocations=yes
#ld: -melf32_x86_64 -Ttext=0x70000000 -Tdata=0xa0000000
#error: .*failed to convert GOTPCREL relocation against 'foo'; relink with --no-relax.*
