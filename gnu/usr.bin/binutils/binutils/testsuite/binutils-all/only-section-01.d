#PROG: objcopy
#source: data-sections.s
#objcopy: --only-section=.data.aa.* --only-section=!.data.aa.02
#readelf: -WS

#...
  \[ [0-9]+\] .data.aa.01.*
  \[ [0-9]+\] .data.aa.03.*
#...
