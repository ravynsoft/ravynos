#name: Check -z force-bti emits BTI feature (exec)
#source: bti-plt-1.s
#target: [check_shared_lib_support]
#as: -mabi=lp64
#ld: -z force-bti -e _start -T bti-plt.ld -L./tmpdir -lbti-plt-so
#readelf: -n

Displaying notes found in: .note.gnu.property
[ 	]+Owner[ 	]+Data size[ 	]+Description
  GNU                  0x00000010	NT_GNU_PROPERTY_TYPE_0
      Properties: AArch64 feature: BTI
