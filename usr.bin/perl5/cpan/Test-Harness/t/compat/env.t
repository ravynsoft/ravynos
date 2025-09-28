#!/usr/bin/perl -w

# Test that env vars are honoured.

use strict;
use warnings;
use lib 't/lib';

use Test::More (
    $^O eq 'VMS'
    ? ( skip_all => 'VMS' )
    : ( tests => 1 )
);

use Test::Harness;

# HARNESS_PERL_SWITCHES

my $test_template = <<'END';
#!/usr/bin/perl

use Test::More tests => 1;

is $ENV{HARNESS_PERL_SWITCHES}, '-w';
END

open TEST, ">env_check_t.tmp";
print TEST $test_template;
close TEST;

END { unlink 'env_check_t.tmp'; }

{
    local $ENV{HARNESS_PERL_SWITCHES} = '-w';
    my ( $tot, $failed )
      = Test::Harness::execute_tests( tests => ['env_check_t.tmp'] );
    is $tot->{bad}, 0;
}

1;
