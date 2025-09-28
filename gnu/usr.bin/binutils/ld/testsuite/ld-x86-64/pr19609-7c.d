#source: pr19609-7.s
#as: --x32 -mrelax-relocations=yes
#ld: -melf32_x86_64 -Ttext=0x80000000
#error: .*failed to convert GOTPCREL relocation against 'foobar'; relink with --no-relax
