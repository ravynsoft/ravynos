#source: pr27128.s
#ld: -shared -version-script pr27128.t
#nm: -n -D --format=sysv --with-symbol-versions
#target: [check_shared_lib_support]
#notarget: [is_underscore_target]
# _Zrm1XS_ doesn't have an extra underscore.
#xfail: hppa64-*-* tic6x-*-*
# hppa64 uses dot-symbols, tic6x DYN lacks dynamic sections for this testcase

#...
VERS_2\.0 +\|0+\| +A +\| +OBJECT\| +\| +\|\*ABS\*
#...
foo@@VERS_2\.0 +\|[0-9a-f]+\| +T +\| +FUNC\|0+10\| +\|\.text
#...
_Zrm1XS_@@VERS_2\.0 +\|[0-9a-f]+\| +T +\| +FUNC\|0+10\| +\|\.text
#pass
