# name: --gc-sections with shared library
# source: dummy.s
# ld: --gc-sections -e main tmpdir/pr11218-2.o tmpdir/pr11218-1.so
# target: *-*-linux* *-*-gnu* arm*-*-uclinuxfdpiceabi
# error: undefined reference to `unresolved_detected_at_runtime_not_at_linktime'
