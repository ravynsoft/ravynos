#!/usr/bin/perl -wT

use strict;
use warnings;
use lib 't/lib';

use Test::More tests => 5;

use TAP::Parser;

my $tap = <<'END_TAP';
1..2
ok 1 - input file opened
... this is junk
    Bail out!  We ran out of foobar.
END_TAP
my $parser = TAP::Parser->new( { tap => $tap } );
isa_ok $parser, 'TAP::Parser',
  '... we should be able to parse bailed out tests';

my @results;
while ( my $result = $parser->next ) {
    push @results => $result;
}
my $bailout = pop @results;
ok $bailout->is_bailout, 'We should be able to parse a nested bailout';
is $bailout->as_string,  'We ran out of foobar.',
  '... and as_string() should return the explanation';
is $bailout->raw, '    Bail out!  We ran out of foobar.',
  '... and raw() should return the explanation';
is $bailout->explanation, 'We ran out of foobar.',
  '... and it should have the correct explanation';
