#!./perl

use strict;
use warnings;
BEGIN {
    unless (-d 'blib') {
        chdir 't' if -d 't';
    }
    require './test.pl';
    set_up_inc('../lib');
}
use utf8;
use open qw( :utf8 :std );

plan(skip_all => "Your system has no SIGALRM") if !exists $SIG{ALRM};
plan(tests => 8);

=pod

These are like the 010_complex_merge_classless test,
but an infinite loop has been made in the heirarchy,
to test that we can fail cleanly instead of going
into an infinite loop

=cut

# initial setup, everything sane
{
    package ƙ;
    our @ISA = qw/ᶨ ィ/;
    package ᶨ;
    our @ISA = qw/ｆ/;
    package ィ;
    our @ISA = qw/ʰ ｆ/;
    package ʰ;
    our @ISA = qw/ᶢ/;
    package ᶢ;
    our @ISA = qw/ᛞ/;
    package ｆ;
    our @ISA = qw/ǝ/;
    package ǝ;
    our @ISA = qw/ᛞ/;
    package ᛞ;
    our @ISA = qw/Ạ Ｂ ʗ/;
    package ʗ;
    our @ISA = qw//;
    package Ｂ;
    our @ISA = qw//;
    package Ạ;
    our @ISA = qw//;
}

# A series of 8 aberations that would cause infinite loops,
#  each one undoing the work of the previous
my @loopies = (
    sub { @ǝ::ISA = qw/ｆ/ },
    sub { @ǝ::ISA = qw/ᛞ/; @ʗ::ISA = qw/ｆ/ },
    sub { @ʗ::ISA = qw//; @Ạ::ISA = qw/ƙ/ },
    sub { @Ạ::ISA = qw//; @ᶨ::ISA = qw/ｆ ƙ/ },
    sub { @ᶨ::ISA = qw/ｆ/; @ʰ::ISA = qw/ƙ ᶢ/ },
    sub { @ʰ::ISA = qw/ᶢ/; @Ｂ::ISA = qw/Ｂ/ },
    sub { @Ｂ::ISA = qw//; @ƙ::ISA = qw/ƙ ᶨ ィ/ },
    sub { @ƙ::ISA = qw/ᶨ ィ/; @ᛞ::ISA = qw/Ạ ʰ Ｂ ʗ/ },
);

foreach my $loopy (@loopies) {
    eval {
        local $SIG{ALRM} = sub { die "ALRMTimeout" };
        alarm(3);
        $loopy->();
        mro::get_linear_isa('ƙ', 'dfs');
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
