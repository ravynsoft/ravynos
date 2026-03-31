# Edited version of Src/signames1.awk.
#
# This is an awk script which finds out what the possibilities for
# the error names are, and dumps them out so that cpp can turn them
# into numbers.  Since we don't need to decide here what the
# real signals are, we can afford to be generous about definitions,
# in case the definitions are in terms of other definitions.
# However, we need to avoid definitions with parentheses, which will
# mess up the syntax.
BEGIN { printf "#include <errno.h>\n\n" }

/^[\t ]*#[\t ]*define[\t ]*E[A-Z0-9]*[\t ][\t ]*[^(\t ]/ { 
    eindex = index($0, "E")
    etail = substr($0, eindex, 80)
    split(etail, tmp)
    enam = substr(tmp[1], 2, 20)
    printf("XXNAMES XXE%s E%s\n", enam, enam)
}
