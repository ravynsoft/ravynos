# source: data.s
# ld: -T pr18963.t
# nm: -B -n
# notarget: *-*-vms
# Skip for VMS based targets as the linker automatically adds extra libraries that may not be present in a cross build.
# 64-bit Cygwin targets always start their sections at 0x200000000 which is why the regexps include a 2.

#...
0+700 A D
#...
0+700 A E
#...
[02]+800 T A
#...
[02]+900 T B
#...
[02]+a00 D C
#pass
