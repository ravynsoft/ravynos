#!/usr/bin/perl

# This is a test suite to cover all the nasty and horrible data
# structures that cause bizarre corner cases.

#  Everyone's invited! :-D

sub BEGIN {
    unshift @INC, 't';
    unshift @INC, 't/compat' if $] < 5.006002;
    require Config; import Config;
    if ($ENV{PERL_CORE} and $Config{'extensions'} !~ /\bStorable\b/) {
        print "1..0 # Skip: Storable was not built\n";
        exit 0;
    }
}

use strict;
BEGIN {
    if (!eval q{
        use Test::More;
        use B::Deparse 0.61;
        use 5.006;
        1;
    }) {
        print "1..0 # skip: tests only work with B::Deparse 0.61 and at least perl 5.6.0\n";
        exit;
    }
    require File::Spec;
    if ($File::Spec::VERSION < 0.8) {
        print "1..0 # Skip: newer File::Spec needed\n";
        exit 0;
    }
}

use Storable qw(freeze thaw);

$Storable::flags = Storable::FLAGS_COMPAT;

#$Storable::DEBUGME = 1;
BEGIN {
    plan tests => 34;
}

{
    package Banana;
    use overload   
	'<=>' => \&compare,
	    '==' => \&equal,
		'""' => \&real,
		fallback => 1;
    sub compare { return int(rand(3))-1 };
    sub equal { return 1 if rand(1) > 0.5 }
    sub real { return "keep it so" }
}

my (@a);

for my $dbun (1, 0) {  # dbun - don't be utterly nasty - being utterly
                       # nasty means having a reference to the object
                       # directly within itself. otherwise it's in the
                       # second array.
    my $nasty = [
		 ($a[0] = bless [ ], "Banana"),
		 ($a[1] = [ ]),
		];

    $a[$dbun]->[0] = $a[0];

    is(ref($nasty), "ARRAY", "Sanity found (now to play with it :->)");

    $Storable::Deparse = $Storable::Deparse = 1;
    $Storable::Eval = $Storable::Eval = 1;

    headit("circular overload 1 - freeze");
    my $icicle = freeze $nasty;
    #print $icicle;   # cat -ve recommended :)
    headit("circular overload 1 - thaw");
    my $oh_dear = thaw $icicle;
    is(ref($oh_dear), "ARRAY", "dclone - circular overload");
    is($oh_dear->[0], "keep it so", "amagic ok 1");
    is($oh_dear->[$dbun]->[0], "keep it so", "amagic ok 2");

    headit("closure dclone - freeze");
    $icicle = freeze sub { "two" };
    #print $icicle;
    headit("closure dclone - thaw");
    my $sub2 = thaw $icicle;
    is($sub2->(), "two", "closures getting dcloned OK");

    headit("circular overload, after closure - freeze");
    #use Data::Dumper;
    #print Dumper $nasty;
    $icicle = freeze $nasty;
    #print $icicle;
    headit("circular overload, after closure - thaw");
    $oh_dear = thaw $icicle;
    is(ref($oh_dear), "ARRAY", "dclone - after a closure dclone");
    is($oh_dear->[0], "keep it so", "amagic ok 1");
    is($oh_dear->[$dbun]->[0], "keep it so", "amagic ok 2");

    push @{$nasty}, sub { print "Goodbye, cruel world.\n" };
    headit("closure freeze AFTER circular overload");
    #print Dumper $nasty;
    $icicle = freeze $nasty;
    #print $icicle;
    headit("circular thaw AFTER circular overload");
    $oh_dear = thaw $icicle;
    is(ref($oh_dear), "ARRAY", "dclone - before a closure dclone");
    is($oh_dear->[0], "keep it so", "amagic ok 1");
    is($oh_dear->[$dbun]->[0], "keep it so", "amagic ok 2");

    @{$nasty} = @{$nasty}[0, 2, 1];
    headit("closure freeze BETWEEN circular overload");
    #print Dumper $nasty;
    $icicle = freeze $nasty;
    #print $icicle;
    headit("circular thaw BETWEEN circular overload");
    $oh_dear = thaw $icicle;
    is(ref($oh_dear), "ARRAY", "dclone - between a closure dclone");
    is($oh_dear->[0], "keep it so", "amagic ok 1");
    is($oh_dear->[$dbun?2:0]->[0], "keep it so", "amagic ok 2");

    @{$nasty} = @{$nasty}[1, 0, 2];
    headit("closure freeze BEFORE circular overload");
    #print Dumper $nasty;
    $icicle = freeze $nasty;
    #print $icicle;
    headit("circular thaw BEFORE circular overload");
    $oh_dear = thaw $icicle;
    is(ref($oh_dear), "ARRAY", "dclone - after a closure dclone");
    is($oh_dear->[1], "keep it so", "amagic ok 1");
    is($oh_dear->[$dbun+1]->[0], "keep it so", "amagic ok 2");
}

sub headit {

    return;  # comment out to get headings - useful for scanning
             # output with $Storable::DEBUGME = 1

    my $title = shift;

    my $size_left = (66 - length($title)) >> 1;
    my $size_right = (67 - length($title)) >> 1;

    print "# ".("-" x $size_left). " $title "
	.("-" x $size_right)."\n";
}

