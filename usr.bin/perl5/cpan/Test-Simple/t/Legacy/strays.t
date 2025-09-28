#!/usr/bin/perl -w 

# Check that stray newlines in test output are properly handed.

BEGIN {
    print "1..0 # Skip not completed\n";
    exit 0;
}

BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        @INC = ('../lib', 'lib');
    }
    else {
        unshift @INC, 't/lib';
    }
}
chdir 't';

use Test::Builder::NoOutput;
my $tb = Test::Builder::NoOutput->create;

$tb->ok(1, "name\n");
$tb->ok(0, "foo\nbar\nbaz");
$tb->skip("\nmoofer");
$tb->todo_skip("foo\n\n");
