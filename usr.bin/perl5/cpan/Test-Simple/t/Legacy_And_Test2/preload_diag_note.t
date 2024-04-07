use strict;
use warnings;

if ($] lt "5.008") {
    print "1..0 # SKIP Test cannot run on perls below 5.8.0\n";
    exit 0;
}

BEGIN {
    require Test2::API;
    Test2::API::test2_start_preload();
}

use Test::More;

my ($stdout, $stderr) = ('', '');
{
    local *STDOUT;
    open(STDOUT, '>', \$stdout) or die "Could not open temp STDOUT";

    local *STDERR;
    open(STDERR, '>', \$stderr) or die "Could not open temp STDOUT";

    diag("test\n", "diag\nfoo");
    note("test\n", "note\nbar");
}

Test2::API::test2_stop_preload();

is($stdout, <<EOT, "Got stdout");
# test
# note
# bar
EOT

is($stderr, <<EOT, "Got stderr");
# test
# diag
# foo
EOT

done_testing;
