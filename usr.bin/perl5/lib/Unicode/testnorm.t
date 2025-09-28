use strict;
use warnings;

BEGIN {
    chdir 't' if -d 't';
}

# This is a wrapper for a generated file.  Assumes being run from 't'
# directory.

if (! $ENV{PERL_DEBUG_FULL_TEST}) {
    print "1..0 # skipped: Lengthy Tests Disabled; to enable set environment",
          "variable \$ENV{PERL_DEBUG_FULL_TEST} to a true value\n";
    0
}
else {
    my $file = '../lib/unicore/TestNorm.pl';
    if (-e $file) {
        do $file;
    }
    else {
        print "1..0 # Skip $file not built (perhaps build options don't"
            . " build it)\n";
        0
    }
}
