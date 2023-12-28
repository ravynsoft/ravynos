#name: Silently Ignore -md Option
#source: empty.s
#as: -md --fatal-warnings
#DUMPPROG: nm

#...
.*foo.*
#pass
