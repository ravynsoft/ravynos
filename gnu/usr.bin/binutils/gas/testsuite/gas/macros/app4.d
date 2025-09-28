#as: -I$srcdir/$subdir
#nm: -n
#name: included file with .if 0 wrapped in APP/NO_APP, no final NO_APP, macro in main file
#xfail: tic30-*-*
#...
0+ T label_d
#...
0+[1-f] T label_a
#...
0+[2-f] T label_b
#pass
