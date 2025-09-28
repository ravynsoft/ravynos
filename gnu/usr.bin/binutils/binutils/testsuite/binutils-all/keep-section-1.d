#PROG: objcopy
#source: data-sections.s
#objcopy: --remove-section=.data.aa.* --keep-section=.data.aa.02
#readelf: -WS

#...
  \[ [0-9]+\] \.data\.aa\.02.*
#...
