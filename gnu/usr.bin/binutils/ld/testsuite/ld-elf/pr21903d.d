#source: pr21903.s
#ld: --no-define-common -pie
#target: *-*-linux* *-*-gnu* arm*-*-uclinuxfdpiceabi
#xfail: ![check_pie_support]
#error: --no-define-common may not be used without -shared
