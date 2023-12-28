#PROG: strip
#source: empty.s
#strip: -g
#readelf: -S --wide
#name: strip -g empty file
# The RL78 linker scripts always PROVIDE a __rl78_abs__ symbol so the stripped symbol table is never empty.
#notarget: rl78-*-*

#...
  \[ 0\] +NULL +0+ .*
#...
  \[ .\] \.shstrtab +STRTAB +0+ .*
Key to Flags:
#pass
