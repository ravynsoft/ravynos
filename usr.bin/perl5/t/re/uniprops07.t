use strict;
use warnings;
no warnings 'once';

if ($^O eq 'dec_osf') {
    print "1..0 # $^O cannot handle this test\n";
    exit(0);
}

# TODO: it would be good to have watchdog(5 * 60) in here
# for slow machines, but unfortunately we cannot trivially
# use test.pl because the TestProp.pl avoids using that.

# This is a wrapper for a generated file.  Assumes being run from 't'
# directory.

# It is skipped by default under PERL_DEBUG_READONLY_COW, but you can run
# it directly via:  cd t; ./perl -I../lib ../lib/unicore/TestProp.pl

require Config;
if ($Config::Config{ccflags} =~ /(?:^|\s)-DPERL_DEBUG_READONLY_COW\b/) {
    print "1..0 # Skip PERL_DEBUG_READONLY_COW\n";
    exit;
}

$::TESTCHUNK=7;
do '../lib/unicore/TestProp.pl';

# Since TestProp.pl explicitly exits, we will only get here if it
# could not load.
if (defined &DynaLoader::boot_DynaLoader # not miniperl
 || eval 'require "unicore/UCD.pl"'    # or tables are built
) {
    die "Could not run lib/unicore/TestProp.pl: ", $@||$!;
}
else {
    print "1..0 # Skip Unicode tables not built yet\n";
}

0
