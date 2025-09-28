#name: GNU Property (single input, combine section)
#source: property-bti-pac1.s
#alltargets: [check_shared_lib_support] *linux*
#as: -march=armv8.5-a -defsym __mult__=0
#ld: -shared
#readelf: -n

Displaying notes found in: .note.gnu.property
[ 	]+Owner[ 	]+Data size[ 	]+Description
  GNU                  0x00000010	NT_GNU_PROPERTY_TYPE_0
      Properties: AArch64 feature: BTI, PAC
