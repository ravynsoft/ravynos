#!./perl -w

# What does this test?
# This checks that changes to pod/perlfunc.pod don't accidentally break the
# build by causing ext/Pod-Functions/Functions_pm.PL to abort.
#
# Why do we test this?
# Pod::Functions is generated from pod/perlfunc.pod by
# ext/Pod-Functions/Functions_pm.PL
# If it can't parse pod/perlfunc.pod, it will abort, which will cause the
# build to break. It's really not possible for it to carry on, hence aborting
# is the only option. However, innocent-seeming changes to documentation
# shouldn't break the build, and we expect everyone to run (at least)
# the porting tests, hence this test, to catch such problems before it's too
# late. To avoid duplicating the parsing logic, we make Functions_pm.PL take
# a --tap option, to test that all is well.
#
# It's broken - how do I fix it?
# Likely it's because you changed something in pod/perlfunc.pod
# If you added a new function, it needs to be added to one or more groups in
# "Perl Functions by Category", and to have a one line summary for
# Pod::Functions provided by a =for directive.

BEGIN {
    @INC = ('..', '../lib') if -f '../TestInit.pm';
}

use Config;
use TestInit qw(T A); # T is chdir to the top level, A makes paths absolute

if ( $Config{usecrosscompile} ) {
    print "1..0 # Not all files are available during cross-compilation\n";
    exit 0;
}

if ( ord("A") == 193) {
    print "1..0 # EBCDIC sorts differenly than expected\n";
    exit 0;
}

system "$^X ext/Pod-Functions/Functions_pm.PL --tap pod/perlfunc.pod";
