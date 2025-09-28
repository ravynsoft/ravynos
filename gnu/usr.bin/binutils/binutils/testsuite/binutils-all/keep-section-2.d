#PROG: objcopy
#source: data-sections.s
#objcopy: --remove-section=.data.aa.* --keep-section=.data.aa.*
#readelf: -WS

#...
  \[ [0-9]+\] \.data\.aa\.01.*
  \[ [0-9]+\] \.data\.aa\.02.*
  \[ [0-9]+\] \.data\.aa\.03.*
#...
