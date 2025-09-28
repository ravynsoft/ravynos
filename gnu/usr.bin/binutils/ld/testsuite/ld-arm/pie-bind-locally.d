#source: pie-bind-locally-a.s
#source: pie-bind-locally-b.s
#target: [check_shared_lib_support]
#ld: -pie
#readelf: -Wr

Relocation section '\.rel\.dyn' at offset .* contains 2 entries:
[ ]+Offset[ ]+Info[ ]+Type[ ]+.*
[0-9a-f]+[ ]+[0-9a-f]+[ ]+R_ARM_RELATIVE[ ].*
[0-9a-f]+[ ]+[0-9a-f]+[ ]+R_ARM_RELATIVE[ ].*
