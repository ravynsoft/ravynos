#name: GNU Property (combine multiple with PAC)
#source: property-bti-pac1.s
#source: property-bti-pac2.s
#as: -mabi=lp64 -defsym __property_pac__=1
#ld: -e _start
#readelf: -n
#target: *linux*

Displaying notes found in: .note.gnu.property
[ 	]+Owner[ 	]+Data size[ 	]+Description
  GNU                  0x00000010	NT_GNU_PROPERTY_TYPE_0
      Properties: AArch64 feature: PAC
