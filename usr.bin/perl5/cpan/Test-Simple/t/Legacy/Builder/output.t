#!perl -w

use strict;

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

use Test::Builder;

# The real Test::Builder
my $Test = Test::Builder->new;
$Test->plan( tests => 6 );


# The one we're going to test.
my $tb = Test::Builder->create();

my $tmpfile = 'foo.tmp';
END { 1 while unlink($tmpfile) }

# Test output to a file
{
    my $out = $tb->output($tmpfile);
    $Test->ok( defined $out );

    print $out "hi!\n";
    close *$out;

    undef $out;
    open(IN, $tmpfile) or die $!;
    chomp(my $line = <IN>);
    close IN;

    $Test->is_eq($line, 'hi!');
}


# Test output to a filehandle
{
    open(FOO, ">>$tmpfile") or die $!;
    my $out = $tb->output(\*FOO);
    my $old = select *$out;
    print "Hello!\n";
    close *$out;
    undef $out;
    select $old;
    open(IN, $tmpfile) or die $!;
    my @lines = <IN>;
    close IN;

    $Test->like($lines[1], qr/Hello!/);
}


# Test output to a scalar ref
{
    my $scalar = '';
    my $out = $tb->output(\$scalar);

    print $out "Hey hey hey!\n";
    $Test->is_eq($scalar, "Hey hey hey!\n");
}


# Test we can output to the same scalar ref
{
    my $scalar = '';
    my $out = $tb->output(\$scalar);
    my $err = $tb->failure_output(\$scalar);

    print $out "To output ";
    print $err "and beyond!";

    $Test->is_eq($scalar, "To output and beyond!", "One scalar, two filehandles");
}


# Ensure stray newline in name escaping works.
{
    my $fakeout = '';
    my $out = $tb->output(\$fakeout);
    $tb->exported_to(__PACKAGE__);
    $tb->no_ending(1);
    $tb->plan(tests => 5);

    $tb->ok(1, "ok");
    $tb->ok(1, "ok\n");
    $tb->ok(1, "ok, like\nok");
    $tb->skip("wibble\nmoof");
    $tb->todo_skip("todo\nskip\n");

    $Test->is_eq( $fakeout, <<OUTPUT ) || print STDERR $fakeout;
1..5
ok 1 - ok
ok 2 - ok
# 
ok 3 - ok, like
# ok
ok 4 # skip wibble
# moof
not ok 5 # TODO & SKIP todo
# skip
# 
OUTPUT
}
