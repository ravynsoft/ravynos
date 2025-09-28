#name: Custom Datapath Extension (CDE) Warnings
#source: cde-warnings.s
#as: -mno-warn-deprecated -march=armv8.1-m.main+cdecp0+mve -I$srcdir/$subdir
#as: -mno-warn-deprecated -march=armv8.1-m.main+cdecp0+cdecp2+cdecp3+cdecp4+cdecp5+cdecp6+cdecp7+mve -I$srcdir/$subdir
#error_output: cde-warnings.l
