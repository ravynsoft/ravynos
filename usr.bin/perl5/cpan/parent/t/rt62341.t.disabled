#!perl -w
use strict;
use Benchmark qw/cmpthese/;
use Test::More tests => 1;

{
    package Bench::Base;
    sub foo { 1 };
}

my $c;
my $sub_iter = 100;

cmpthese (-1 => {
    recompute_existing_ISA  => sub {
        $c++;
        for (1..$sub_iter) {
            my $class = "Bench::Par::Sub_${c}";
            no strict 'refs';
            @{ "$class\::ISA"} = (@{ "$class\::ISA"},'Bench::Base');
            die unless $class->foo;
        }
    },
    recompute_new_ISA  => sub {
        $c++;
        for (1..$sub_iter) {
            my $class = "Bench::Par::Sub_${c}::SubSub${_}";
            no strict 'refs';
            @{ "$class\::ISA"} = (@{ "$class\::ISA"},'Bench::Base');
            die unless $class->foo;
        }
    },
    push_existing_ISA  => sub {
        $c++;
        for (1..$sub_iter) {
            my $class = "Bench::Par::Sub_${c}";
            no strict 'refs';
            push @{ "$class\::ISA"}, 'Bench::Base';
            die unless $class->foo;
        }
    },
    push_new_ISA  => sub {
        $c++;
        for (1..$sub_iter) {
            my $class = "Bench::Par::Sub_${c}::SubSub${_}";
            no strict 'refs';
            push @{ "$class\::ISA"}, 'Bench::Base';
            die unless $class->foo;
        }
    },
    push_new_FOO  => sub {
        $c++;
        for (1..$sub_iter) {
            my $class = "Bench::Par::Sub_${c}::SubSub${_}";
            no strict 'refs';
            push @{ "$class\::FOO"}, 'Bench::Base';
            #die unless $class->foo;
        }
    },
    push_existing_FOO => sub {
        $c++;
        for (1..$sub_iter) {
            my $class = "Bench::Par::Sub_${c}";
            no strict 'refs';
            push @{ "$class\::FOO"}, 'Bench::Base';
            #die unless $class->foo;
        }
    },
    recompute_existing_FOO => sub {
        $c++;
        for (1..$sub_iter) {
            my $class = "Bench::Par::Sub_${c}";
            no strict 'refs';
            @{ "$class\::FOO"} = (@{ "$class\::FOO"}, 'Bench::Base');
            #die unless $class->foo;
        }
    },
    
    # Take a reference and manipulate that, in case string references are slow
    refcompute_existing_FOO => sub {
        $c++;
        for (1..$sub_iter) {
            my $class = "Bench::Par::Sub_${c}";
            no strict 'refs';
            my $aref = \@{ "$class\::FOO"};
            @{ $aref } = (@{ $aref }, 'Bench::Base');
            #die unless $class->foo;
        }
    },
    recompute_new_FOO => sub {
        $c++;
        for (1..$sub_iter) {
            my $class = "Bench::Par::Sub_${c}::SubSub${_}";
            no strict 'refs';
            @{ "$class\::FOO"} = (@{ "$class\::FOO"}, 'Bench::Base');
            #die unless $class->foo;
        }
    },
});

pass "Benchmarks run";
