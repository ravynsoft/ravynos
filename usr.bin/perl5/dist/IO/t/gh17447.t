#!/usr/bin/perl

# Regression test for https://github.com/Perl/perl5/issues/17447

use strict;
use warnings;

use Test::More tests => 2;

use IO::Select;
use IO::Handle;

pipe( my $rd, my $wr ) or die "Cannot pipe() - $!";
binmode $rd;
binmode $wr;
$wr->syswrite("data\n");

my $select = IO::Select->new();
$select->add($rd);

is( scalar $select->handles, 1, '$select has 1 handle' );

# close first, then remove afterwards
$rd->close;
$select->remove($rd);

is( scalar $select->handles, 0, '$select has 0 handles' );

exit;
