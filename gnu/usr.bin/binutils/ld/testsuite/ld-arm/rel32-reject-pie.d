#source: rel32-reject-pie.s
#target: [check_shared_lib_support]
#ld: -pie
#error: .*relocation R_ARM_REL32.*can not.*PIE executable.*
