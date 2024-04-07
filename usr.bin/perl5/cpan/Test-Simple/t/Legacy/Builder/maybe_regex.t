#!/usr/bin/perl -w

BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        @INC = ('../lib', 'lib');
    }
    else {
        unshift @INC, 't/lib';
    }
}

use strict;
use Test::More tests => 16;

use Test::Builder;
my $Test = Test::Builder->new;

my $r = $Test->maybe_regex(qr/^FOO$/i);
ok(defined $r, 'qr// detected');
ok(('foo' =~ /$r/), 'qr// good match');
ok(('bar' !~ /$r/), 'qr// bad match');

SKIP: {
    skip "blessed regex checker added in 5.10", 3 if $] < 5.010;
    
    my $obj = bless qr/foo/, 'Wibble';
    my $re = $Test->maybe_regex($obj);
    ok( defined $re, "blessed regex detected" );
    ok( ('foo' =~ /$re/), 'blessed qr/foo/ good match' );
    ok( ('bar' !~ /$re/), 'blessed qr/foo/ bad math' );
}

{
	my $r = $Test->maybe_regex('/^BAR$/i');
	ok(defined $r, '"//" detected');
	ok(('bar' =~ m/$r/), '"//" good match');
	ok(('foo' !~ m/$r/), '"//" bad match');
};

{
	my $r = $Test->maybe_regex('not a regex');
	ok(!defined $r, 'non-regex detected');
};


{
	my $r = $Test->maybe_regex('/0/');
	ok(defined $r, 'non-regex detected');
	ok(('f00' =~ m/$r/), '"//" good match');
	ok(('b4r' !~ m/$r/), '"//" bad match');
};


{
	my $r = $Test->maybe_regex('m,foo,i');
	ok(defined $r, 'm,, detected');
	ok(('fOO' =~ m/$r/), '"//" good match');
	ok(('bar' !~ m/$r/), '"//" bad match');
};
