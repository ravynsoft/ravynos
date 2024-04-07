#!/usr/bin/perl -w

# Test that HARNESS_SUBCLASS env var is honoured.

use strict;
use warnings;
use lib 't/lib';

use Test::More (
    $^O eq 'VMS'
    ? ( skip_all => 'VMS' )
    : ( tests => 1 )
);

use Test::Harness;

my $test_template = <<'END';
#!/usr/bin/perl

use Test::More tests => 1;

is $ENV{HARNESS_IS_SUBCLASS}, 'TAP::Harness::TestSubclass';
END

my $tempfile = "_check_subclass_t.tmp";
open TEST, ">$tempfile";
print TEST $test_template;
close TEST;

END { unlink $tempfile; }

{
    local $ENV{HARNESS_SUBCLASS} = 'TAP::Harness::TestSubclass';
    my ( $tot, $failed )
      = Test::Harness::execute_tests( tests => [$tempfile] );
    is $tot->{bad}, 0;
}

1;
