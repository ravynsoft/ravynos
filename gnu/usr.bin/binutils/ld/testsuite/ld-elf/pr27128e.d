#source: pr27128.s
#ld: -shared -version-script pr27128.t
#nm: -n --demangle -D --format=posix --with-symbol-versions
#target: [check_shared_lib_support]
#notarget: [is_underscore_target]
# _Zrm1XS_ doesn't have an extra underscore.
#xfail: hppa64-*-* tic6x-*-*
# hppa64 uses dot-symbols, tic6x DYN lacks dynamic sections for this testcase

#...
VERS_2\.0 A 0+ 
#...
foo@@VERS_2\.0 T [0-9a-f]+ 10
#...
operator%\(X, X\)@@VERS_2\.0 T [0-9a-f]+ 10
#pass
