#PROG: objcopy
#source: data-sections.s
#objcopy: --remove-section=.data.aa.* --remove-section=!.data.aa.02
#readelf: -WS

#...
  \[ [0-9]+\] \.data\.aa\.02.*
#...
