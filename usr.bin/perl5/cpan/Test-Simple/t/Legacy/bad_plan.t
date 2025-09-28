#!/usr/bin/perl -w

BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        @INC = '../lib';
    }
}

use Test::Builder;
my $Test = Test::Builder->new;
$Test->plan( tests => 2 );
$Test->level(0);

my $tb = Test::Builder->create;

eval { $tb->plan(7); };
$Test->like( $@, qr/^plan\(\) doesn't understand 7/, 'bad plan()' ) ||
    print STDERR "# $@";

eval { $tb->plan(wibble => 7); };
$Test->like( $@, qr/^plan\(\) doesn't understand wibble 7/, 'bad plan()' ) ||
    print STDERR "# $@";
