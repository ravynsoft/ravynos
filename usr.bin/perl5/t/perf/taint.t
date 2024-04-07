#!./perl -T
#
# All the tests in this file are ones that run exceptionally slowly
# (each test taking seconds or even minutes) in the absence of particular
# optimisations. Thus it is a sort of canary for optimisations being
# broken.
#
# Although it includes a watchdog timeout, this is set to a generous limit
# to allow for running on slow systems; therefore a broken optimisation
# might be indicated merely by this test file taking unusually long to
# run, rather than actually timing out.
#
# This is similar to t/perf/speed.t but tests performance regressions specific
# to taint.
#

BEGIN {
    chdir 't' if -d 't';
    @INC = ('../lib');
    require Config; import Config;
    require './test.pl';
    skip_all_if_miniperl("No Scalar::Util under miniperl");
    if (exists($Config{taint_support}) && !$Config{taint_support}) {
        skip_all("built without taint support");
    }
}

use strict;
use warnings;
use Scalar::Util qw(tainted);

$| = 1;

plan tests => 4;

watchdog(60);

my $taint = substr($ENV{PATH}, 0, 0); # and empty tainted string

{
    my $in = $taint . ( "ab" x 200_000 );
    utf8::upgrade($in);
    ok(tainted($in), "performance issue only when tainted");
    while ($in =~ /\Ga+b/g) { }
    pass("\\G on tainted string");
}

# RT #130584
# tainted string caused the utf8 pos cache to be cleared each time

{
    my $repeat = 30_000;
    my $in = $taint . ("abcdefghijklmnopqrstuvwxyz" x $repeat);
    utf8::upgrade($in);
    ok(tainted($in), "performance issue only when tainted");
    local ${^UTF8CACHE} = 1;  # defeat debugging
    for my $i (1..$repeat) {
        $in =~ /abcdefghijklmnopqrstuvwxyz/g or die;
        my $p = pos($in); # this was slow
    }
    pass("RT #130584 pos on tainted utf8 string");
}

1;
