#!./perl

BEGIN {
    unshift @INC, 't';
    require Config;
    if (($Config::Config{'extensions'} !~ /\bB\b/) ){
        print "1..0 # Skip -- Perl configured without B module\n";
        exit 0;
    }
}

use warnings;
use strict;
use Test::More;

BEGIN { use_ok( 'B' ); }

# Somewhat minimal tests.

my %seen;

sub B::OP::pie {
    my $self = shift;
    return ++$seen{$self->name};
}

my %debug;
sub B::OP::walkoptree_debug {
    my $self = shift;
    return ++$debug{$self->name};
}

my $victim = sub {
    # This gives us a substcont, which gets to the second recursive call
    # point (in the if statement in the XS code)
    $_[0] =~ s/(a)/ $1/;
    # PMOP_pmreplroot(cPMOPo) is NULL for this
    $_[0] =~ s/(b)//;
    # This gives an OP_SPLIT
    split /c/;
};

is (B::walkoptree_debug, 0, 'walkoptree_debug() is 0');
B::walkoptree(B::svref_2object($victim)->ROOT, "pie");
foreach (qw(substcont split leavesub)) {
    is ($seen{$_}, 1, "Our victim had a $_ OP");
}
is_deeply ([keys %debug], [], 'walkoptree_debug was not called');

B::walkoptree_debug(2);
is (B::walkoptree_debug(), 1, 'walkoptree_debug() is 1');
B::walkoptree_debug(0);
is (B::walkoptree_debug(), 0, 'walkoptree_debug() is 0');
B::walkoptree_debug(1);
is (B::walkoptree_debug(), 1, 'walkoptree_debug() is 1 again');
%seen = ();

B::walkoptree(B::svref_2object($victim)->ROOT, "pie");
foreach (qw(substcont split leavesub)) {
    is ($seen{$_}, 1, "Our victim had a $_ OP");
}
is_deeply (\%debug, \%seen, 'walkoptree_debug was called correctly');

my %seen2;

# Now try to exercise the code in walkoptree that decides that it can't re-use
# the object and reference.
sub B::OP::fiddle {
    my $name = $_[0]->name;
    ++$seen2{$name};
    if ($name =~ /^s/) {
	# Take another reference to the reference
	push @::junk, \$_[0];
    } elsif ($name =~ /^p/) {
	# Take another reference to the object
	push @::junk, \${$_[0]};
    } elsif ($name =~ /^l/) {
	undef $_[0];
    } elsif ($name =~ /g/) {
	${$_[0]} = "Muhahahahaha!";
    } elsif ($name =~ /^c/) {
	bless \$_[0];
    }
}

B::walkoptree(B::svref_2object($victim)->ROOT, "fiddle");
is_deeply (\%seen2, \%seen, 'everything still seen');

done_testing();
