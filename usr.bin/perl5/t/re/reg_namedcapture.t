#!./perl

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    unless (defined &DynaLoader::boot_DynaLoader) {
      print "1..0 # Skip: no dynamic loading on miniperl, no Tie::Hash::NamedCapture\n";
      exit 0;
    }
}

# WARNING: Do not directly use any modules as part of this test code.
# We could get action at a distance that would invalidate the tests.

print "1..2\n";

# This tests whether glob assignment fails to load the tie.
*X = *-;
'X'=~/(?<X>X)/;
print eval '*X{HASH}{X} || 1' ? "" :"not ","ok ",++$test,"\n";

# And since it's a similar case we check %! as well. Note that
# this can't be done until ../lib/Errno.pm is in place, as the
# glob hits $!, which needs that module.
*Y = *!;
print 0<keys(%Y) ? "" :"not ","ok ",++$test,"\n";
