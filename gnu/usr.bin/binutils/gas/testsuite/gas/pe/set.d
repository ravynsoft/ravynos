#nm: --defined-only
#name: set directive in COFF
#
# Ensure that we stick an entry for the left hand side of a set directive
# depending on the name of the left hand side.

#...
.* t _b
#...
.* T _d
#...
