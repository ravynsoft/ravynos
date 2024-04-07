#!/usr/bin/perl -Tw

use strict;
use Test::More tests => 5;

BEGIN {
    use_ok( 'Locale::Maketext' );
}

{
    package Whunk::L10N;
    our @ISA =  'Locale::Maketext';
}

{
    package Whunk::L10N::en;
    our @ISA = 'Whunk::L10N';
}

my $lh = Whunk::L10N->get_handle('en');
$lh->fail_with('failure_handler_auto');

is($lh->maketext('abcd'), 'abcd', "simple missing keys are handled");
is($lh->maketext('abcd'), 'abcd', "even in repeated calls");
# CPAN RT #25877 - $value Not Set After Second Call to failure_handler_auto()

is($lh->maketext('Hey, [_1]', 'you'), 'Hey, you', "keys with bracket notation ok");

is($lh->maketext('_key'), '_key', "keys which start with _ ok");

