#!./perl

BEGIN {
    require './test.pl';
    unless (-d 'blib') {
        chdir 't' if -d 't';
    }
    set_up_inc('../lib');
}

use strict;
use warnings;

plan(skip_all => "Your system has no SIGALRM") if !exists $SIG{ALRM};
plan(tests => 8);

require mro;

=pod

These are like the 010_complex_merge_classless test,
but an infinite loop has been made in the heirarchy,
to test that we can fail cleanly instead of going
into an infinite loop

=cut

# initial setup, everything sane
{
    package K;
    use mro 'c3';
    our @ISA = qw/J I/;
    package J;
    use mro 'c3';
    our @ISA = qw/F/;
    package I;
    use mro 'c3';
    our @ISA = qw/H F/;
    package H;
    use mro 'c3';
    our @ISA = qw/G/;
    package G;
    use mro 'c3';
    our @ISA = qw/D/;
    package F;
    use mro 'c3';
    our @ISA = qw/E/;
    package E;
    use mro 'c3';
    our @ISA = qw/D/;
    package D;
    use mro 'c3';
    our @ISA = qw/A B C/;
    package C;
    use mro 'c3';
    our @ISA = qw//;
    package B;
    use mro 'c3';
    our @ISA = qw//;
    package A;
    use mro 'c3';
    our @ISA = qw//;
}

# A series of 8 aberations that would cause infinite loops,
#  each one undoing the work of the previous
my @loopies = (
    sub { @E::ISA = qw/F/ },
    sub { @E::ISA = qw/D/; @C::ISA = qw/F/ },
    sub { @C::ISA = qw//; @A::ISA = qw/K/ },
    sub { @A::ISA = qw//; @J::ISA = qw/F K/ },
    sub { @J::ISA = qw/F/; @H::ISA = qw/K G/ },
    sub { @H::ISA = qw/G/; @B::ISA = qw/B/ },
    sub { @B::ISA = qw//; @K::ISA = qw/K J I/ },
    sub { @K::ISA = qw/J I/; @D::ISA = qw/A H B C/ },
);

foreach my $loopy (@loopies) {
    eval {
        local $SIG{ALRM} = sub { die "ALRMTimeout" };
        alarm(3);
        $loopy->();
        mro::get_linear_isa('K', 'c3');
    };

    if(my $err = $@) {
        if($err =~ /ALRMTimeout/) {
            ok(0, "Loop terminated by SIGALRM");
        }
        elsif($err =~ /Recursive inheritance detected/) {
            ok(1, "Graceful exception thrown");
        }
        else {
            ok(0, "Unrecognized exception: $err");
        }
    }
    else {
        ok(0, "Infinite loop apparently succeeded???");
    }
}
