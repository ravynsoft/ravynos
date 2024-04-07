#!/usr/bin/perl -w
use strict;
use Test::More 'no_plan';
use Fcntl;

use autodie qw(sysopen);

use constant NO_SUCH_FILE => "this_file_had_better_not_be_here_at_all";

my $fh;
eval {
	sysopen($fh, $0, O_RDONLY);
};

is($@, "", "sysopen can open files that exist");

like(scalar( <$fh> ), qr/perl/, "Data in file read");

eval {
	sysopen(my $fh2, NO_SUCH_FILE, O_RDONLY);
};

isa_ok($@, 'autodie::exception', 'Opening a bad file fails with sysopen');
