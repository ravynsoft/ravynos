#source: pr26936a.s
#source: pr26936b.s
#source: pr26936c.s
#as: --gen-debug
#ld: -z noseparate-code -Ttext-segment 0x10000 -z max-page-size=0x1000 -z common-page-size=0x1000
#readelf: -wL -W
#target: [check_shared_lib_support]
# Assembly source file for the HPPA assembler is renamed and modifed by
# sed. loongarch and mn10300 and riscv put different numbers of local symbols in
# linkonce section and comdat sections.  xtensa has more than one member
# in comdat groups.
#xfail: am33_2.0-*-* hppa*-*-hpux* loongarch*-*-* mn10300-*-* riscv*-*-* xtensa*-*-*

#...
CU: .*/pr26936c.s:
File name +Line number +Starting address +View +Stmt
pr26936c.s +6 +0x10[0-9a-f][0-9a-f][0-9a-f] +x
#pass
