#source: pr24905.s
#as: --x32
#ld: -shared -melf32_x86_64 $srcdir/$subdir/pr24905.t
#nm: -n

#...
[0-9a-f]* t EXTERNAL_SYM
#pass
