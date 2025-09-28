#!/usr/bin/perl -w
use strict;
use FindBin;
use Test::More tests => 4;
use lib "$FindBin::Bin/lib";
use lethal qw(open);

use constant NO_SUCH_FILE => "this_file_had_better_not_exist";

eval {
    open(my $fh, '<', NO_SUCH_FILE);
};

ok($@, "lethal throws an exception");
isa_ok($@, 'autodie::exception','...which is the correct class');
ok($@->matches('open'),         "...which matches open");
is($@->file,__FILE__,           "...which reports the correct file");
