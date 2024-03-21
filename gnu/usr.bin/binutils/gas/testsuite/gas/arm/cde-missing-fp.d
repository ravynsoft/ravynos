#name: Custom Datapath Extension FP missing (CDE)
#source: cde.s
#as: -mno-warn-deprecated -march=armv8-m.main+cdecp0+cdecp7 -I$srcdir/$subdir
#as: -mno-warn-deprecated -march=armv8-m.main+cdecp0+cdecp1+cdecp2+cdecp3+cdecp4+cdecp5+cdecp6+cdecp7 -I$srcdir/$subdir
#error_output: cde-missing-fp.l
