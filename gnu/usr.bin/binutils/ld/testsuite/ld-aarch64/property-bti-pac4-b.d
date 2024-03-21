#name: PT_GNU_PROPERTY alignment ILP32
#source: property-bti-pac4.s
#as: -mabi=ilp32
#ld: -m [aarch64_choose_ilp32_emul] -z force-bti -e main
#readelf: -l --wide
#target: *linux*
#warning: .*property-bti-pac4.*: warning: BTI turned on by -z force-bti.*

#...
  GNU_PROPERTY .* +0x4
#...
