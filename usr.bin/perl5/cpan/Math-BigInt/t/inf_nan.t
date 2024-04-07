# -*- mode: perl; -*-

# test inf/NaN handling all in one place

use strict;
use warnings;
use lib 't';

use Test::More tests => 1044;

use Math::BigInt;
use Math::BigFloat;
use Math::BigInt::Subclass;
use Math::BigFloat::Subclass;

my @biclasses = qw/ Math::BigInt   Math::BigInt::Subclass   /;
my @bfclasses = qw/ Math::BigFloat Math::BigFloat::Subclass /;

my (@args, $x, $y, $z, $test);

# +

foreach (qw/

    -inf:-inf:-inf
    -1:-inf:-inf
    -0:-inf:-inf
    0:-inf:-inf
    1:-inf:-inf
    inf:-inf:NaN
    NaN:-inf:NaN

    -inf:-1:-inf
    -1:-1:-2
    -0:-1:-1
    0:-1:-1
    1:-1:0
    inf:-1:inf
    NaN:-1:NaN

    -inf:0:-inf
    -1:0:-1
    -0:0:0
    0:0:0
    1:0:1
    inf:0:inf
    NaN:0:NaN

    -inf:1:-inf
    -1:1:0
    -0:1:1
    0:1:1
    1:1:2
    inf:1:inf
    NaN:1:NaN

    -inf:inf:NaN
    -1:inf:inf
    -0:inf:inf
    0:inf:inf
    1:inf:inf
    inf:inf:inf
    NaN:inf:NaN

    -inf:NaN:NaN
    -1:NaN:NaN
    -0:NaN:NaN
    0:NaN:NaN
    1:NaN:NaN
    inf:NaN:NaN
    NaN:NaN:NaN

  /)
{
    @args = split /:/, $_;
    for my $class (@biclasses, @bfclasses) {
        $args[2] = '0' if $args[2] eq '-0';     # Math::Big* has no -0
        $x = $class->new($args[0]);
        $y = $class->new($args[1]);
        $z = $x->badd($y);

        $test = qq|\$x = $class->new("$args[0]"); |
              . qq|\$y = $class->new("$args[1]"); |
              . qq|\$z = \$x->badd(\$y);|;

        subtest $test => sub {
            plan tests => 6;

            is(ref($x), $class, "\$x is a $class");
            is(ref($y), $class, "\$y is still a $class");
            is(ref($z), $class, "\$z is a $class");
            is($x->bstr(), $args[2], 'value of $x');
            is($y->bstr(), $args[1], 'value of $y');
            is($z->bstr(), $args[2], 'value of $z');
        };
    }
}

# -

foreach (qw/

    -inf:-inf:NaN
    -1:-inf:inf
    -0:-inf:inf
    0:-inf:inf
    1:-inf:inf
    inf:-inf:inf
    NaN:-inf:NaN

    -inf:-1:-inf
    -1:-1:0
    -0:-1:1
    0:-1:1
    1:-1:2
    inf:-1:inf
    NaN:-1:NaN

    -inf:0:-inf
    -1:0:-1
    -0:0:-0
    0:0:0
    1:0:1
    inf:0:inf
    NaN:0:NaN

    -inf:1:-inf
    -1:1:-2
    -0:1:-1
    0:1:-1
    1:1:0
    inf:1:inf
    NaN:1:NaN

    -inf:inf:-inf
    -1:inf:-inf
    -0:inf:-inf
    0:inf:-inf
    1:inf:-inf
    inf:inf:NaN
    NaN:inf:NaN

    -inf:NaN:NaN
    -1:NaN:NaN
    -0:NaN:NaN
    0:NaN:NaN
    1:NaN:NaN
    inf:NaN:NaN
    NaN:NaN:NaN

  /)
{
    @args = split /:/, $_;
    for my $class (@biclasses, @bfclasses) {
        $args[2] = '0' if $args[2] eq '-0';     # Math::Big* has no -0
        $x = $class->new($args[0]);
        $y = $class->new($args[1]);
        $z = $x->bsub($y);

        $test = qq|\$x = $class->new("$args[0]"); |
              . qq|\$y = $class->new("$args[1]"); |
              . qq|\$z = \$x->bsub(\$y);|;

        subtest $test => sub {
            plan tests => 6;

            is(ref($x), $class, "\$x is a $class");
            is(ref($y), $class, "\$y is still a $class");
            is(ref($z), $class, "\$z is a $class");
            is($x->bstr(), $args[2], 'value of $x');
            is($y->bstr(), $args[1], 'value of $y');
            is($z->bstr(), $args[2], 'value of $z');
        };
    }
}

# *

foreach (qw/

    -inf:-inf:inf
    -1:-inf:inf
    -0:-inf:NaN
    0:-inf:NaN
    1:-inf:-inf
    inf:-inf:-inf
    NaN:-inf:NaN

    -inf:-1:inf
    -1:-1:1
    -0:-1:0
    0:-1:-0
    1:-1:-1
    inf:-1:-inf
    NaN:-1:NaN

    -inf:0:NaN
    -1:0:-0
    -0:0:-0
    0:0:0
    1:0:0
    inf:0:NaN
    NaN:0:NaN

    -inf:1:-inf
    -1:1:-1
    -0:1:-0
    0:1:0
    1:1:1
    inf:1:inf
    NaN:1:NaN

    -inf:inf:-inf
    -1:inf:-inf
    -0:inf:NaN
    0:inf:NaN
    1:inf:inf
    inf:inf:inf
    NaN:inf:NaN

    -inf:NaN:NaN
    -1:NaN:NaN
    -0:NaN:NaN
    0:NaN:NaN
    1:NaN:NaN
    inf:NaN:NaN
    NaN:NaN:NaN

    /)
{
    @args = split /:/, $_;
    for my $class (@biclasses, @bfclasses) {
        $args[2] = '0' if $args[2] eq '-0';     # Math::Big* has no -0
        $x = $class->new($args[0]);
        $y = $class->new($args[1]);
        $z = $x->bmul($y);

        $test = qq|\$x = $class->new("$args[0]"); |
              . qq|\$y = $class->new("$args[1]"); |
              . qq|\$z = \$x->bmul(\$y);|;

        subtest $test => sub {
            plan tests => 6;

            is(ref($x), $class, "\$x is a $class");
            is(ref($y), $class, "\$y is still a $class");
            is(ref($z), $class, "\$z is a $class");
            is($x->bstr(), $args[2], 'value of $x');
            is($y->bstr(), $args[1], 'value of $y');
            is($z->bstr(), $args[2], 'value of $z');
        };
    }
}

# /

foreach (qw/

    -inf:-inf:NaN
    -1:-inf:0
    -0:-inf:0
    0:-inf:-0
    1:-inf:-1
    inf:-inf:NaN
    NaN:-inf:NaN

    -inf:-1:inf
    -1:-1:1
    -0:-1:0
    0:-1:-0
    1:-1:-1
    inf:-1:-inf
    NaN:-1:NaN

    -inf:0:-inf
    -1:0:-inf
    -0:0:NaN
    0:0:NaN
    1:0:inf
    inf:0:inf
    NaN:0:NaN

    -inf:1:-inf
    -1:1:-1
    -0:1:-0
    0:1:0
    1:1:1
    inf:1:inf
    NaN:1:NaN

    -inf:inf:NaN
    -1:inf:-1
    -0:inf:-0
    0:inf:0
    1:inf:0
    inf:inf:NaN
    NaN:inf:NaN

    -inf:NaN:NaN
    -1:NaN:NaN
    -0:NaN:NaN
    0:NaN:NaN
    1:NaN:NaN
    inf:NaN:NaN
    NaN:NaN:NaN

    /)
{
    @args = split /:/, $_;
    for my $class (@biclasses, @bfclasses) {
        $args[2] = '0' if $args[2] eq '-0';     # Math::Big* has no -0

        my ($q, $r);

        # bdiv in scalar context

        $x = $class->new($args[0]);
        $y = $class->new($args[1]);

        unless ($class =~ /^Math::BigFloat/) {
            $q = $x->bdiv($y);

            $test = qq|\$x = $class->new("$args[0]"); |
                  . qq|\$y = $class->new("$args[1]"); |
                  . qq|\$q = \$x->bdiv(\$y);|;

            subtest $test => sub {
                plan tests => 6;

                is(ref($x), $class, "\$x is a $class");
                is(ref($y), $class, "\$y is still a $class");
                is(ref($q), $class, "\$q is a $class");
                is($x->bstr(), $args[2], 'value of $x');
                is($y->bstr(), $args[1], 'value of $y');
                is($q->bstr(), $args[2], 'value of $q');
            };
        }

        # bmod and bdiv in list context

        $x = $class->new($args[0]);
        $y = $class->new($args[1]);

        ($q, $r) = $x->bdiv($y);

        # bdiv in list context

        $test = qq|\$x = $class->new("$args[0]"); |
              . qq|\$y = $class->new("$args[1]"); |
              . qq|(\$q, \$r) = \$x->bdiv(\$y);|;

        subtest $test => sub {
            plan tests => 7;

            is(ref($x), $class, "\$x is a $class");
            is(ref($y), $class, "\$y is still a $class");
            is(ref($q), $class, "\$q is a $class");
            is(ref($r), $class, "\$r is a $class");
            is($x->bstr(), $args[2], 'value of $x');
            is($y->bstr(), $args[1], 'value of $y');
            is($q->bstr(), $args[2], 'value of $q');
        };

        # bmod

        $x = $class->new($args[0]);
        $y = $class->new($args[1]);

        my $m = $x->bmod($y);

        $test = qq|\$x = $class->new("$args[0]"); |
              . qq|\$y = $class->new("$args[1]"); |
              . qq|\$m = \$x->bmod(\$y);|;

        subtest $test => sub {
            plan tests => 6;

            is(ref($x), $class, "\$x is a $class");
            is(ref($y), $class, "\$y is still a $class");
            is(ref($m), $class, "\$m is a $class");
            is($x->bstr(), $r->bstr(), 'value of $x');
            is($y->bstr(), $args[1], 'value of $y');
            is($m->bstr(), $r->bstr(), 'value of $m');
        };
    }
}

# /

foreach (qw/

    -inf:-inf:NaN
    -1:-inf:0
    -0:-inf:0
    0:-inf:-0
    1:-inf:-0
    inf:-inf:NaN
    NaN:-inf:NaN

    -inf:-1:inf
    -1:-1:1
    -0:-1:0
    0:-1:-0
    1:-1:-1
    inf:-1:-inf
    NaN:-1:NaN

    -inf:0:-inf
    -1:0:-inf
    -0:0:NaN
    0:0:NaN
    1:0:inf
    inf:0:inf
    NaN:0:NaN

    -inf:1:-inf
    -1:1:-1
    -0:1:-0
    0:1:0
    1:1:1
    inf:1:inf
    NaN:1:NaN

    -inf:inf:NaN
    -1:inf:-0
    -0:inf:-0
    0:inf:0
    1:inf:0
    inf:inf:NaN
    NaN:inf:NaN

    -inf:NaN:NaN
    -1:NaN:NaN
    -0:NaN:NaN
    0:NaN:NaN
    1:NaN:NaN
    inf:NaN:NaN
    NaN:NaN:NaN

    /)
{
    @args = split /:/, $_;
    for my $class (@bfclasses) {
        $args[2] = '0' if $args[2] eq '-0';     # Math::Big* has no -0
        $x = $class->new($args[0]);
        $y = $class->new($args[1]);
        $z = $x->bdiv($y);

        $test = qq|\$x = $class->new("$args[0]"); |
              . qq|\$y = $class->new("$args[1]"); |
              . qq|\$z = \$x->bdiv(\$y);|;

        subtest $test => sub {
            plan tests => 6;

            is(ref($x), $class, "\$x is a $class");
            is(ref($y), $class, "\$y is still a $class");
            is(ref($z), $class, "\$z is a $class");
            is($x->bstr(), $args[2], 'value of $x');
            is($y->bstr(), $args[1], 'value of $y');
            is($z->bstr(), $args[2], 'value of $z');
        };
    }
}

#############################################################################
# overloaded comparisons

foreach my $c (@biclasses, @bfclasses) {
    $x = $c->bnan();
    $y = $c->bnan();            # test with two different objects, too
    $z = $c->bzero();

    is($x == $y, '', 'NaN == NaN: ""');
    is($x != $y, 1,  'NaN != NaN: 1');

    is($x == $x, '', 'NaN == NaN: ""');
    is($x != $x, 1,  'NaN != NaN: 1');

    is($z != $x, 1,  '0 != NaN: 1');
    is($z == $x, '', '0 == NaN: ""');

    is($z < $x,  '', '0 < NaN: ""');
    is($z <= $x, '', '0 <= NaN: ""');
    is($z >= $x, '', '0 >= NaN: ""');
    #is($z > $x,  '', '0 > NaN: ""');   # Bug! Todo: fix it!
}

# All done.
