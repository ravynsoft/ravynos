# This is an awk script which finds out what the possibilities for
# the signal names are, and dumps them out so that cpp can turn them
# into numbers.  Since we don't need to decide here what the
# real signals are, we can afford to be generous about definitions,
# in case the definitions are in terms of other definitions.
# However, we need to avoid definitions with parentheses, which will
# mess up the syntax.
BEGIN { printf "#include <signal.h>\n\n" }

/^[\t ]*#[\t ]*define[\t _]*SIG[A-Z][A-Z0-9]*[\t ][\t ]*[^(\t ]/ { 
    sigindex = index($0, "SIG")
    sigtail = substr($0, sigindex, 80)
    split(sigtail, tmp)
    signam = substr(tmp[1], 4, 20)
    if (substr($0, sigindex-1, 1) == "_")
        printf("XXNAMES XXSIG%s _SIG%s\n", signam, signam)
    else
        printf("XXNAMES XXSIG%s SIG%s\n", signam, signam)
}
