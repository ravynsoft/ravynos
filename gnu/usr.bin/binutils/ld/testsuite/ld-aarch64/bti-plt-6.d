#name: Warn with one missing GNU NOTE BTI input
#source: property-bti-pac2.s
#source: property-bti-pac1.s
#target: [check_shared_lib_support]
#as: -mabi=lp64 -defsym __property_pac__=1
#ld: -shared -z force-bti
#warning: .*property-bti-pac2.*: warning: BTI turned on by -z force-bti.*$
#readelf: -n

# Should warn about the missing input BTI NOTE but should
# still mark output as BTI

Displaying notes found in: .note.gnu.property
[ 	]+Owner[ 	]+Data size[ 	]+Description
  GNU                  0x00000010	NT_GNU_PROPERTY_TYPE_0
      Properties: AArch64 feature: BTI, PAC
