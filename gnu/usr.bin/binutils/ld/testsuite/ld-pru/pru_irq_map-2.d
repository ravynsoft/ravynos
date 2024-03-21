#name: pru_irq_map special section for host 2
#source: pru_irq_map.s
#ld: --defsym=__HEAP_SIZE=0 --defsym=__STACK_SIZE=0
#readelf: -S --wide

# Ensure that .pru_irq_map section is present.

#...
 +\[[ 0-9]+\] +.pru_irq_map +PROGBITS +0+ +[0-9a-f]+ +0+5 +00 +0 +0 +1
#...
