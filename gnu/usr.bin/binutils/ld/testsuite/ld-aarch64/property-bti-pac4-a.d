#name: PT_GNU_PROPERTY alignment
#source: property-bti-pac4.s
#as: -mabi=lp64
#ld: -z force-bti -e main
#readelf: -l --wide
#target: *linux*
#warning: .*property-bti-pac4.*: warning: BTI turned on by -z force-bti.*

#...
  GNU_PROPERTY .* +0x8
#...
