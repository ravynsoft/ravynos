#include this file into another for subclass testing

use strict;
use warnings;

our ($CLASS, $LIB);

##############################################################################
# for testing inheritance of _swap

package Math::Foo;

use Math::BigInt lib => $main::LIB;
our @ISA = qw/Math::BigInt/;

use overload
  # customized overload for sub, since original does not use swap there
  '-'     =>      sub { my @a = ref($_[0])->_swap(@_);
                        $a[0]->bsub($a[1]);
                      };

sub _swap {
    # a fake _swap, which reverses the params
    my $self = shift;           # for override in subclass
    if ($_[2]) {
        my $c = ref($_[0]) || 'Math::Foo';
        return( $_[0]->copy(), $_[1] );
    } else {
        return( Math::Foo->new($_[1]), $_[0] );
    }
}

##############################################################################
package main;

is($CLASS->config('lib'), $LIB, "$CLASS->config('lib')");

my ($x, $y, $z, @args, $try, $got, $want);
my ($f, $round_mode, $expected_class);

while (<DATA>) {
    s/#.*$//;                   # remove comments
    s/\s+$//;                   # remove trailing whitespace
    next unless length;         # skip empty lines

    my ($m, $e);

    if (s/^&//) {
        $f = $_;
        next;
    }

    if (/^\$/) {
        $round_mode = $_;
        $round_mode =~ s/^\$/$CLASS\->/;
        next;
    }

    @args = split(/:/, $_, 99);
    $want = pop(@args);
    $expected_class = $CLASS;

    if ($want =~ /(.*?)=(.*)/) {
        $expected_class = $2;
        $want = $1;
    }

    $try = qq|\$x = $CLASS->new("$args[0]");|;
    if ($f eq "bnorm") {
        $try = qq|\$x = $CLASS->bnorm("$args[0]");|;
    } elsif ($f =~ /^is_(zero|one|odd|even|(non_)?(negative|positive)|nan|int)$/) {
        $try .= " \$x->$f() || 0;";
    } elsif ($f eq "is_inf") {
        $try .= qq| \$x->is_inf("$args[1]");|;
    } elsif ($f eq "binf") {
        $try .= qq| \$x->binf("$args[1]");|;
    } elsif ($f eq "bone") {
        $try .= qq| \$x->bone("$args[1]");|;
    # some unary ops
    } elsif ($f =~ /^b(nan|floor|ceil|int|sstr|neg|abs|sgn|inc|dec|not|sqrt|exp)$/) {
        $try .= " \$x->$f();";
    } elsif ($f =~ /^b[dt]?fac$/) {
        $try .= " \$x->$f();";
    } elsif ($f =~ /^(numify|length|stringify)$/) {
        $try .= " \$x->$f();";
    } elsif ($f =~ /^(to|as)_(hex|oct|bin)$/) {
        $try .= " \$x->$f();";
    # overloaded functions
    } elsif ($f =~ /^(log|exp|sin|cos|atan2|int|neg|abs|sqrt)$/) {
        $try .= " \$x = $f(\$x);";
    } elsif ($f eq "parts") {
        $try .= ' ($m, $e) = $x->parts();';
        # ->bstr() to see if an object is returned
        $try .= ' $m = $m->bstr(); $m = "NaN" if !defined $m;';
        $try .= ' $e = $e->bstr(); $e = "NaN" if !defined $e;';
        $try .= ' "$m,$e";';
    } elsif ($f eq "exponent") {
        # ->bstr() to see if an object is returned
        $try .= ' $x = $x->exponent()->bstr();';
    } elsif ($f eq "mantissa") {
        # ->bstr() to see if an object is returned
        $try .= ' $x = $x->mantissa()->bstr();';
    } elsif ($f eq "bpi") {
        $try .= " $CLASS\->bpi(\$x);";
    } else {
        # binary operators
        $try .= qq| \$y = $CLASS->new("$args[1]");|;
        if ($f eq "bcmp") {
            $try .= ' $x->bcmp($y);';
        } elsif ($f eq "bround") {
            $try .= " $round_mode; \$x->bround(\$y);";
        } elsif ($f eq "bacmp") {
            $try .= ' $x->bacmp($y);';
        } elsif ($f eq "badd") {
            $try .= ' $x->badd($y);';
        } elsif ($f eq "bsub") {
            $try .= ' $x->bsub($y);';
        } elsif ($f eq "bmul") {
            $try .= ' $x->bmul($y);';
        } elsif ($f eq "bdiv") {
            $try .= ' $x->bdiv($y);';
        } elsif ($f eq "bdiv-list") {
            $try .= ' join(",", $x->bdiv($y));';
        } elsif ($f eq "btdiv") {
            $try .= ' $x->btdiv($y);';
        } elsif ($f eq "btdiv-list") {
            $try .= ' join (",", $x->btdiv($y));';
        # overload via x=
        } elsif ($f =~ /^.=$/) {
            $try .= " \$x $f \$y;";
            # overload via x
        } elsif ($f =~ /^.$/) {
            $try .= " \$x $f \$y;";
        } elsif ($f eq "bmod") {
            $try .= ' $x % $y;';
        } elsif ($f eq "bgcd") {
            if (defined $args[2]) {
                $try .= qq| \$z = $CLASS->new("$args[2]");|;
            }
            $try .= " $CLASS\::bgcd(\$x, \$y";
            $try .= ", \$z" if defined $args[2];
            $try .= ");";
        } elsif ($f eq "blcm") {
            if (defined $args[2]) {
                $try .= qq| \$z = $CLASS->new("$args[2]");|;
            }
            $try .= " $CLASS\::blcm(\$x, \$y";
            $try .= ", \$z" if defined $args[2];
            $try .= ");";
        } elsif ($f eq "blsft") {
            if (defined $args[2]) {
                $try .= " \$x->blsft(\$y, $args[2]);";
            } else {
                $try .= " \$x << \$y;";
            }
        } elsif ($f eq "brsft") {
            if (defined $args[2]) {
                $try .= " \$x->brsft(\$y, $args[2]);";
            } else {
                $try .= " \$x >> \$y;";
            }
        } elsif ($f eq "bnok") {
            $try .= " \$x->bnok(\$y);";
        } elsif ($f eq "bmfac") {
            $try .= " \$x->bmfac(\$y);";
        } elsif ($f eq "broot") {
            $try .= " \$x->broot(\$y);";
        } elsif ($f eq "blog") {
            $try .= " \$x->blog(\$y);";
        } elsif ($f eq "band") {
            $try .= " \$x & \$y;";
        } elsif ($f eq "bior") {
            $try .= " \$x | \$y;";
        } elsif ($f eq "bxor") {
            $try .= " \$x ^ \$y;";
        } elsif ($f eq "bpow") {
            $try .= " \$x ** \$y;";
        } elsif ( $f eq "bmodinv") {
            $try .= " \$x->bmodinv(\$y);";
        } elsif ($f eq "digit") {
            $try .= " \$x->digit(\$y);";
        } elsif ($f eq "batan2") {
            $try .= " \$x->batan2(\$y);";
        } else {
            # Functions with three arguments
            $try .= qq| \$z = $CLASS->new("$args[2]");|;

            if ( $f eq "bmodpow") {
                $try .= " \$x->bmodpow(\$y, \$z);";
            } elsif ($f eq "bmuladd") {
                $try .= " \$x->bmuladd(\$y, \$z);";
            } else {
                warn "Unknown op '$f'";
            }
        }
    }                           # end else all other ops

    $got = eval $try;
    print "# Error: $@\n" if $@;

    # convert hex/binary targets to decimal
    if ($want =~ /^(0x0x|0b0b)/) {
        $want =~ s/^0[xb]//;
        $want = Math::BigInt->new($want)->bstr();
    }
    if ($want eq "") {
        is($got, undef, $try);
    } else {
        # print "try: $try ans: $got $want\n";
        is($got, $want, $try);
        is(ref($got), $expected_class,
           qq|output is a "$expected_class" object|)
          if $expected_class ne $CLASS;
    }
    # check internal state of number objects
    is_valid($got, $f) if ref $got;
}                               # end while data tests
close DATA;

# test whether self-multiplication works correctly (result is 2**64)
$try = qq|\$x = $CLASS->new("4294967296");|;
$try .= ' $a = $x->bmul($x);';
$got = eval $try;
is($got, $CLASS->new(2) ** 64, $try);

# test self-pow
$try = qq|\$x = $CLASS->new(10);|;
$try .= ' $a = $x->bpow($x);';
$got = eval $try;
is($got, $CLASS->new(10) ** 10, $try);

###############################################################################
# test whether op destroys args or not (should better not)

$x = $CLASS->new(3);
$y = $CLASS->new(4);
$z = $x & $y;
is($x, 3, '$z = $x & $y; $x');
is($y, 4, '$z = $x & $y; $y');
is($z, 0, '$z = $x & $y; $z');

$z = $x | $y;
is($x, 3, '$z = $x | $y; $x');
is($y, 4, '$z = $x | $y; $y');
is($z, 7, '$z = $x | $y; $z');

$x = $CLASS->new(1);
$y = $CLASS->new(2);
$z = $x | $y;
is($x, 1, '$z = $x | $y; $x');
is($y, 2, '$z = $x | $y; $y');
is($z, 3, '$z = $x | $y; $z');

$x = $CLASS->new(5);
$y = $CLASS->new(4);
$z = $x ^ $y;
is($x, 5, '$z = $x ^ $y; $x');
is($y, 4, '$z = $x ^ $y; $y');
is($z, 1, '$z = $x ^ $y; $z');

$x = $CLASS->new(-5);
$y = -$x;
is($x, -5, '$y = -$x; $x');

$x = $CLASS->new(-5);
$y = abs($x);
is($x, -5, '$y = abs($x); $x');

$x = $CLASS->new(8);
$y = $CLASS->new(-1);
$z = $CLASS->new(5033);
my $u = $x->copy()->bmodpow($y, $z);
is($u, 4404, '$x->copy()->bmodpow($y, $z); $u');
is($y, -1,   '$x->copy()->bmodpow($y, $z); $y');
is($z, 5033, '$x->copy()->bmodpow($y, $z); $z');

$x = $CLASS->new(-5);
$y = -$x;
is($x, -5, '$y = -$x; $x');
is($y, 5,  '$y = -$x; $y');

$x = $CLASS->new(-5);
$y = $x->copy()->bneg();
is($x, -5, '$y = $x->copy()->bneg(); $x');
is($y, 5,  '$y = $x->copy()->bneg(); $y');

$x = $CLASS->new(-5);
$y = $CLASS->new(3);
$x->bmul($y);
is($x, -15, '$x->bmul($y); $x');
is($y, 3,   '$x->bmul($y); $y');

$x = $CLASS->new(-5);
$y = $CLASS->new(3);
$x->badd($y);
is($x, -2, '$x->badd($y); $x');
is($y, 3,  '$x->badd($y); $y');

$x = $CLASS->new(-5);
$y = $CLASS->new(3);
$x->bsub($y);
is($x, -8, '$x->bsub($y); $x');
is($y, 3,  '$x->bsub($y); $y');

$x = $CLASS->new(-15);
$y = $CLASS->new(3);
$x->bdiv($y);
is($x, -5, '$x->bdiv($y); $x');
is($y, 3,  '$x->bdiv($y); $y');

$x = $CLASS->new(-5);
$y = $CLASS->new(3);
$x->bmod($y);
is($x, 1, '$x->bmod($y); $x');
is($y, 3, '$x->bmod($y); $y');

$x = $CLASS->new(5);
$y = $CLASS->new(3);
$x->bmul($y);
is($x, 15, '$x->bmul($y); $x');
is($y, 3,  '$x->bmul($y); $y');

$x = $CLASS->new(5);
$y = $CLASS->new(3);
$x->badd($y);
is($x, 8, '$x->badd($y); $x');
is($y, 3, '$x->badd($y); $y');

$x = $CLASS->new(5);
$y = $CLASS->new(3);
$x->bsub($y);
is($x, 2, '$x->bsub($y); $x');
is($y, 3, '$x->bsub($y); $y');

$x = $CLASS->new(15);
$y = $CLASS->new(3);
$x->bdiv($y);
is($x, 5, '$x->bdiv($y); $x');
is($y, 3, '$x->bdiv($y); $y');

$x = $CLASS->new(5);
$y = $CLASS->new(3);
$x->bmod($y);
is($x, 2, '$x->bmod($y); $x');
is($y, 3, '$x->bmod($y); $y');

$x = $CLASS->new(5);
$y = $CLASS->new(-3);
$x->bmul($y);
is($x, -15, '$x->bmul($y); $x');
is($y, -3,  '$x->bmul($y); $y');

$x = $CLASS->new(5);
$y = $CLASS->new(-3);
$x->badd($y);
is($x, 2,  '$x->badd($y); $x');
is($y, -3, '$x->badd($y); $y');

$x = $CLASS->new(5);
$y = $CLASS->new(-3);
$x->bsub($y);
is($x, 8,  '$x->bsub($y); $x');
is($y, -3, '$x->bsub($y); $y');

$x = $CLASS->new(15);
$y = $CLASS->new(-3);
$x->bdiv($y);
is($x, -5, '$x->bdiv($y); $x');
is($y, -3, '$x->bdiv($y); $y');

$x = $CLASS->new(5);
$y = $CLASS->new(-3);
$x->bmod($y);
is($x, -1, '$x->bmod($y); $x');
is($y, -3, '$x->bmod($y); $y');

###############################################################################
# check whether overloading cmp works
$try = '$x = $CLASS->new(0);';
$try .= ' $y = 10;';
$try .= ' $x ne $y;';
$want = eval $try;
ok($want, "overloading cmp works");

# We can't test for working cmpt with other objects here, we would need a dummy
# object with stringify overload for this. See Math::String tests as example.

###############################################################################
# check reversed order of arguments

$try = "\$x = $CLASS->new(10); \$x = 2 ** \$x; \$x == 1024;";
$want = eval $try;
ok($want, $try);

$try = "\$x = $CLASS->new(10); \$x = 2 * \$x; \$x == 20;";
$want = eval $try;
ok($want, $try);

$try = "\$x = $CLASS->new(10); \$x = 2 + \$x; \$x == 12;";
$want = eval $try;
ok($want, $try);

$try = "\$x = $CLASS\->new(10); \$x = 2 - \$x; \$x == -8;";
$want = eval $try;
ok($want, $try);

$try = "\$x = $CLASS\->new(10); \$x = 20 / \$x; \$x == 2;";
$want = eval $try;
ok($want, $try);

$try = "\$x = $CLASS\->new(3); \$x = 20 % \$x; \$x == 2;";
$want = eval $try;
ok($want, $try);

$try = "\$x = $CLASS\->new(7); \$x = 20 & \$x; \$x == 4;";
$want = eval $try;
ok($want, $try);

$try = "\$x = $CLASS\->new(7); \$x = 0x20 | \$x; \$x == 0x27;";
$want = eval $try;
ok($want, $try);

$try = "\$x = $CLASS\->new(7); \$x = 0x20 ^ \$x; \$x == 0x27;";
$want = eval $try;
ok($want, $try);

###############################################################################
# check badd(4, 5) form

$try = "\$x = $CLASS\->badd(4, 5); \$x == 9;";
$want = eval $try;
ok($want, $try);

###############################################################################
# check undefs: NOT DONE YET

###############################################################################
# bool

$x = $CLASS->new(1);
if ($x) {
    pass("\$x = $CLASS->new(1); \$x is true");
} else {
    fail("\$x = $CLASS->new(1); \$x is true");
}

$x = $CLASS->new(0);
if (!$x) {
    pass("\$x = $CLASS->new(0); !\$x is false");
} else {
    fail("\$x = $CLASS->new(0); !\$x is false");
}

###############################################################################
# objectify()

@args = Math::BigInt::objectify(2, 4, 5);
is(scalar(@args), 3,                 "objectify(2, 4, 5) gives $CLASS, 4, 5");
like($args[0],    qr/^Math::BigInt/, "first arg matches /^Math::BigInt/");
is($args[1],      4,                 "second arg is 4");
is($args[2],      5,                 "third arg is 5");

@args = Math::BigInt::objectify(0, 4, 5);
is(scalar(@args), 3,                 "objectify(0, 4, 5) gives $CLASS, 4, 5");
like($args[0],    qr/^Math::BigInt/, "first arg matches /^Math::BigInt/");
is($args[1],      4,                 "second arg is 4");
is($args[2],      5,                 "third arg is 5");

@args = Math::BigInt::objectify(2, 4, 5);
is(scalar(@args), 3,                 "objectify(2, 4, 5) gives $CLASS, 4, 5");
like($args[0],    qr/^Math::BigInt/, "first arg matches /^Math::BigInt/");
is($args[1],      4,                 "second arg is 4");
is($args[2],      5,                 "third arg is 5");

@args = Math::BigInt::objectify(2, 4, 5, 6, 7);
is(scalar(@args), 5,
   "objectify(2, 4, 5, 6, 7) gives $CLASS, 4, 5, 6, 7");
like($args[0],    qr/^Math::BigInt/, "first arg matches /^Math::BigInt/");
is($args[1],      4,                 "second arg is 4");
is(ref($args[1]), $args[0],          "second arg is a $args[0] object");
is($args[2],      5,                 "third arg is 5");
is(ref($args[2]), $args[0],          "third arg is a $args[0] object");
is($args[3],      6,                 "fourth arg is 6");
is(ref($args[3]), '',                "fourth arg is a scalar");
is($args[4],      7,                 "fifth arg is 7");
is(ref($args[4]), '',                "fifth arg is a scalar");

@args = Math::BigInt::objectify(2, $CLASS, 4, 5, 6, 7);
is(scalar(@args), 5,
   "objectify(2, $CLASS, 4, 5, 6, 7) gives $CLASS, 4, 5, 6, 7");
is($args[0],      $CLASS,            "first arg is $CLASS");
is($args[1],      4,                 "second arg is 4");
is(ref($args[1]), $args[0],          "second arg is a $args[0] object");
is($args[2],      5,                 "third arg is 5");
is(ref($args[2]), $args[0],          "third arg is a $args[0] object");
is($args[3],      6,                 "fourth arg is 6");
is(ref($args[3]), '',                "fourth arg is a scalar");
is($args[4],      7,                 "fifth arg is 7");
is(ref($args[4]), '',                "fifth arg is a scalar");

###############################################################################
# test whether an opp calls objectify properly or not (or at least does what
# it should do given non-objects, w/ or w/o objectify())

is($CLASS->new(123)->badd(123), 246,
   qq|$CLASS->new(123)->badd(123) = 246|);;
is($CLASS->badd(123, 321), 444,
   qq|$CLASS->badd(123, 321) = 444|);;
is($CLASS->badd(123, $CLASS->new(321)), 444,
   qq|$CLASS->badd(123, $CLASS->new(321)) = 444|);;

is($CLASS->new(123)->bsub(122), 1,
   qq|$CLASS->new(123)->bsub(122) = 1|);;
is($CLASS->bsub(321, 123), 198,
   qq|$CLASS->bsub(321, 123) = 198|);;
is($CLASS->bsub(321, $CLASS->new(123)), 198,
   qq|$CLASS->bsub(321, $CLASS->new(123)) = 198|);;

is($CLASS->new(123)->bmul(123), 15129,
   qq|$CLASS->new(123)->bmul(123) = 15129|);;
is($CLASS->bmul(123, 123), 15129,
   qq|$CLASS->bmul(123, 123) = 15129|);;
is($CLASS->bmul(123, $CLASS->new(123)), 15129,
   qq|$CLASS->bmul(123, $CLASS->new(123)) = 15129|);;

is($CLASS->new(15129)->bdiv(123), 123,
   qq|$CLASS->new(15129)->bdiv(123) = 123|);;
is($CLASS->bdiv(15129, 123), 123,
   qq|$CLASS->bdiv(15129, 123) = 123|);;
is($CLASS->bdiv(15129, $CLASS->new(123)), 123,
   qq|$CLASS->bdiv(15129, $CLASS->new(123)) = 123|);;

is($CLASS->new(15131)->bmod(123), 2,
   qq|$CLASS->new(15131)->bmod(123) = 2|);;
is($CLASS->bmod(15131, 123), 2,
   qq|$CLASS->bmod(15131, 123) = 2|);;
is($CLASS->bmod(15131, $CLASS->new(123)), 2,
   qq|$CLASS->bmod(15131, $CLASS->new(123)) = 2|);;

is($CLASS->new(2)->bpow(16), 65536,
   qq|$CLASS->new(2)->bpow(16) = 65536|);;
is($CLASS->bpow(2, 16), 65536,
   qq|$CLASS->bpow(2, 16) = 65536|);;
is($CLASS->bpow(2, $CLASS->new(16)), 65536,
   qq|$CLASS->bpow(2, $CLASS->new(16)) = 65536|);;

is($CLASS->new(2**15)->brsft(1), 2**14,
   qq|$CLASS->new(2**15)->brsft(1) = 2**14|);;
is($CLASS->brsft(2**15, 1), 2**14,
   qq|$CLASS->brsft(2**15, 1) = 2**14|);;
is($CLASS->brsft(2**15, $CLASS->new(1)), 2**14,
   qq|$CLASS->brsft(2**15, $CLASS->new(1)) = 2**14|);;

is($CLASS->new(2**13)->blsft(1), 2**14,
   qq|$CLASS->new(2**13)->blsft(1) = 2**14|);;
is($CLASS->blsft(2**13, 1), 2**14,
   qq|$CLASS->blsft(2**13, 1) = 2**14|);;
is($CLASS->blsft(2**13, $CLASS->new(1)), 2**14,
   qq|$CLASS->blsft(2**13, $CLASS->new(1)) = 2**14|);;

###############################################################################
# test for floating-point input (other tests in bnorm() below)

$z = 1050000000000000;          # may be int on systems with 64bit?
$x = $CLASS->new($z);
is($x->bsstr(), '105e+13',      # not 1.05e+15
   qq|\$x = $CLASS->new($z); \$x->bsstr() = "105e+13"|);
$z = 1e+129;                    # definitely a float (may fail on UTS)
# don't compare to $z, since some Perl versions stringify $z into something
# like '1.e+129' or something equally ugly
SKIP:{
    my $vax_float = (pack("d", 1) =~ /^[\x80\x10]\x40/);
    skip("vax float range smaller", 1) if $vax_float;
    $x = $CLASS->new($z);
    is($x -> bsstr(), '1e+129',
       qq|\$x = $CLASS->new($z); \$x->bsstr() = "1e+129"|);
}

###############################################################################
# test for whitespace including newlines to be handled correctly

# is($Math::BigInt::strict, 1);         # the default

foreach my $c (qw/1 12 123 1234 12345 123456 1234567
                  12345678 123456789 1234567890/)
{
    my $m = $CLASS->new($c);
    is($CLASS->new("$c"),        $m,    qq|$CLASS->new("$c") = $m|);
    is($CLASS->new(" $c"),       $m,    qq|$CLASS->new(" $c") = $m|);
    is($CLASS->new("$c "),       $m,    qq|$CLASS->new("$c ") = $m|);
    is($CLASS->new(" $c "),      $m,    qq|$CLASS->new(" $c ") = $m|);
    is($CLASS->new("\n$c"),      $m,    qq|$CLASS->new("\\n$c") = $m|);
    is($CLASS->new("$c\n"),      $m,    qq|$CLASS->new("$c\\n") = $m|);
    is($CLASS->new("\n$c\n"),    $m,    qq|$CLASS->new("\\n$c\\n") = $m|);
    is($CLASS->new(" \n$c\n"),   $m,    qq|$CLASS->new(" \\n$c\\n") = $m|);
    is($CLASS->new(" \n$c \n"),  $m,    qq|$CLASS->new(" \\n$c \\n") = $m|);
    is($CLASS->new(" \n$c\n "),  $m,    qq|$CLASS->new(" \\n$c\\n ") = $m|);
    is($CLASS->new(" \n$c\n1"),  'NaN', qq|$CLASS->new(" \\n$c\\n1") = 'NaN'|);
    is($CLASS->new("1 \n$c\n1"), 'NaN', qq|$CLASS->new("1 \\n$c\\n1") = 'NaN'|);
}

###############################################################################
# prime number tests, also test for **= and length()
# found on: http://www.utm.edu/research/primes/notes/by_year.html

# ((2^148)+1)/17
$x = $CLASS->new(2);
$x **= 148;
$x++;
$x = $x / 17;
is($x, "20988936657440586486151264256610222593863921",
   "value of ((2^148)+1)/17");
is($x->length(), length("20988936657440586486151264256610222593863921"),
   "number of digits in ((2^148)+1)/17");

# MM7 = 2^127-1
$x = $CLASS->new(2);
$x **= 127;
$x--;
is($x, "170141183460469231731687303715884105727", "value of 2^127-1");

$x = $CLASS->new('215960156869840440586892398248');
($x, $y) = $x->length();
is($x, 30, "number of digits in 2^127-1");
is($y, 0, "number of digits in fraction part of 2^127-1");

$x = $CLASS->new('1_000_000_000_000');
($x, $y) = $x->length();
is($x, 13, "number of digits in 1_000_000_000_000");
is($y, 0, "number of digits in fraction part of 1_000_000_000_000");

# test <<=, >>=
$x = $CLASS->new('2');
$y = $CLASS->new('18');
is($x <<= $y, 2 << 18, "2 <<= 18 with $CLASS objects");
is($x,        2 << 18, "2 <<= 18 with $CLASS objects");
is($x >>= $y, 2,       "2 >>= 18 with $CLASS objects");
is($x,        2,       "2 >>= 18 with $CLASS objects");

# I am afraid the following is not yet possible due to slowness
# Also, testing for 2 meg output is a bit hard ;)
#$x = $CLASS->new(2);
#$x **= 6972593;
#$x--;

# 593573509*2^332162+1 has exactly 1,000,000 digits
# takes about 24 mins on 300 Mhz, so cannot be done yet ;)
#$x = $CLASS->new(2);
#$x **= 332162;
#$x *= "593573509";
#$x++;
#is($x->length(), 1_000_000);

###############################################################################
# inheritance and overriding of _swap

$x = Math::Foo->new(5);
$x = $x - 8;            # 8 - 5 instead of 5-8
is($x, 3, '$x = Math::Foo->new(5); $x = $x - 8; $x = 3');
is(ref($x), 'Math::Foo', '$x is an object of class "Math::Foo"');

$x = Math::Foo->new(5);
$x = 8 - $x;            # 5 - 8 instead of 8 - 5
is($x, -3, '$x = Math::Foo->new(5); $x = 8 - $x; $x = -3');
is(ref($x), 'Math::Foo', '$x is an object of class "Math::Foo"');

###############################################################################
# Test whether +inf eq inf
#
# This tried to test whether Math::BigInt inf equals Perl inf. Unfortunately,
# Perl hasn't (before 5.7.3 at least) a consistent way to say inf, and some
# things like 1e100000 crash on some platforms. So simple test for the string
# 'inf'.

$x = $CLASS->new('+inf');
is($x, 'inf', qq|$CLASS->new("+inf") = "inf"|);

###############################################################################
# numify() and 64 bit integer support

require Config;
SKIP: {
    skip("no 64 bit integer support", 4)
      if ! $Config::Config{use64bitint} || ! $Config::Config{use64bitall}
          || "$]" < 5.007001;

    # The following should not give "1.84467440737096e+19".

    $x = $CLASS -> new(2) -> bpow(64) -> bdec();
    is($x -> bstr(),   "18446744073709551615", "bigint 2**64-1 as string");
    is($x -> numify(), "18446744073709551615", "bigint 2**64-1 as number");

    # The following should not give "-9.22337203685478e+18".

    $x = $CLASS -> new(2) -> bpow(63) -> bneg();
    is($x -> bstr(),   "-9223372036854775808", "bigint -2**63 as string");
    is($x -> numify(), "-9223372036854775808", "bigint -2**63 as number");
};

###############################################################################
###############################################################################
# the following tests only make sense with Math::BigInt::Calc or BareCalc or
# FastCalc

SKIP: {
    # skip GMP, Pari et al.
    skip("skipping tests not intended for the backend $LIB", 50)
      unless $LIB =~ /^Math::BigInt::(Bare|Fast)?Calc$/;

    ###########################################################################
    # check proper length of internal arrays

    my $bl = $LIB->_base_len();
    my $BASE = '9' x $bl;
    my $MAX = $BASE;
    $BASE++;

    # f.i. 9999
    $x = $CLASS->new($MAX);
    is_valid($x);

    # 10000
    $x += 1;
    is($x, $BASE, "\$x == $BASE");
    is_valid($x);

    # 9999 again
    $x -= 1;
    is($x, $MAX, "\$x == $MAX");
    is_valid($x);

    ###########################################################################
    # check numify

    $x = $CLASS->new($BASE-1);
    is($x->numify(), $BASE-1, q|$x->numify() = $BASE-1|);

    $x = $CLASS->new(-($BASE-1));
    is($x->numify(), -($BASE-1), q|$x->numify() = -($BASE-1)|);

    # +0 is to protect from 1e15 vs 100000000 (stupid to_string aaarglburbll...)
    $x = $CLASS->new($BASE);
    is($x->numify()+0, $BASE+0, q|$x->numify()+0 = $BASE+0|);

    $x = $CLASS->new(-$BASE);
    is($x->numify(), -$BASE, q|$x->numify() = -$BASE|);

    $x = $CLASS->new(-($BASE*$BASE*1+$BASE*1+1));
    is($x->numify(), -($BASE*$BASE*1+$BASE*1+1),
       q|$x->numify() = -($BASE*$BASE*1+$BASE*1+1))|);

    ###########################################################################
    # test bug in _digits with length($c[-1]) where $c[-1] was "00001" instead
    # of 1

    $x = $CLASS->new($BASE - 2);
    $x++;
    $x++;
    $x++;
    $x++;
    ok($x > $BASE, '$x > $BASE');

    $x = $CLASS->new($BASE + 3);
    $x++;
    ok($x > $BASE, '$x > $BASE');

    # test for +0 instead of int():
    $x = $CLASS->new($MAX);
    is($x->length(), length($MAX), q|$x->length() = length($MAX)|);

    ###########################################################################
    # test bug that $CLASS->digit($string) did not work

    is($CLASS->digit(123, 2), 1, qq|$CLASS->digit(123, 2) = 1|);

    ###########################################################################
    # bug in sub where number with at least 6 trailing zeros after any op failed

    $x = $CLASS->new(123456);
    $z = $CLASS->new(10000);
    $z *= 10;
    $x -= $z;
    is($z, 100000, "testing bug in sub");
    is($x, 23456, "testing bug in sub");

    ###########################################################################
    # bug in shortcut in mul()

    # construct a number with a zero-hole of BASE_LEN_SMALL
    {
        my @bl = $LIB->_base_len();
        my $bl = $bl[5];

        # Compute the value.
        $x = ('1' x $bl) . ('0' x $bl) . ('1' x $bl) . ('0' x $bl);
        $y = '1' x (2 * $bl);
        $x = $CLASS->new($x)->bmul($y);

        # Build the expected output.
        $y = '';
        if ($bl >= 2) {
            $y .= '123456790' x int(($bl - 2) / 9);
            $y .= substr '123456790', 0, ($bl - 2) % 9;
            $y .= ($bl - 1) % 9;
        }
        $y .= ((($bl - 1) % 9) + 1) x ($bl * 3);
        if ($bl >= 2) {
            $y .= substr '098765432', -(($bl - 1) % 9);
            $y .= '098765432' x int(($bl - 2) / 9);
        }
        $y .= '1';
        $y .= '0' x $bl;

        is($x, $y, "testing number with a zero-hole of BASE_LEN_SMALL");

        #########################################################################
        # see if mul shortcut for small numbers works

        $x = '9' x $bl;
        $x = $CLASS->new($x);
        # 999 * 999 => 998 . 001
        # 9999 * 9999 => 9998 . 0001
        $y = '9' x ($bl - 1) . '8' . '0' x ($bl - 1) . '1';
        is($x * $x, $y,
           "see if mul shortcut for small numbers works ($x * $x = $y)");
    }

    ###########################################################################
    # bug with rest "-0" in div, causing further div()s to fail

    $x = $CLASS->new('-322056000');
    ($x, $y) = $x->bdiv('-12882240');

    is($y, '0', '-322056000 / -12882240 has remainder 0');
    is_valid($y);               # $y not '-0'

    ###########################################################################
    # bug in $x->bmod($y)

    # if $x < 0 and $y > 0
    $x = $CLASS->new('-629');
    is($x->bmod(5033), 4404, q|$x->bmod(5033) = 4404|);

    ###########################################################################
    # bone/binf etc as plain calls (Lite failed them)

    is($CLASS->bzero(),      0,      qq|$CLASS->bzero() = 0|);
    is($CLASS->bone(),       1,      qq|$CLASS->bone() = 1|);
    is($CLASS->bone("+"),    1,      qq|$CLASS->bone("+") = 1|);
    is($CLASS->bone("-"),    -1,     qq|$CLASS->bone("-") = -1|);
    is($CLASS->bnan(),       "NaN",  qq|$CLASS->bnan() = "NaN"|);
    is($CLASS->binf(),       "inf",  qq|$CLASS->binf() = "inf"|);
    is($CLASS->binf("+"),    "inf",  qq|$CLASS->binf("+") = "inf"|);
    is($CLASS->binf("-"),    "-inf", qq|$CLASS->binf("-") = "-inf"|);
    is($CLASS->binf("-inf"), "-inf", qq|$CLASS->binf("-inf") = "-inf"|);

    ###########################################################################
    # is_one("-")

    is($CLASS->new(1)->is_one("-"),  0, qq|$CLASS->new(1)->is_one("-") = 0|);
    is($CLASS->new(-1)->is_one("-"), 1, qq|$CLASS->new(-1)->is_one("-") = 1|);
    is($CLASS->new(1)->is_one(),     1, qq|$CLASS->new(1)->is_one() = 1|);
    is($CLASS->new(-1)->is_one(),    0, qq|$CLASS->new(-1)->is_one() = 0|);

    ###########################################################################
    # [perl #30609] bug with $x -= $x not being 0, but 2*$x

    $x = $CLASS->new(3);
    $x -= $x;
    is($x, 0, qq|\$x = $CLASS->new(3); \$x -= \$x; = 0|);

    $x = $CLASS->new(-3);
    $x -= $x;
    is($x, 0, qq|\$x = $CLASS->new(-3); \$x -= \$x; = 0|);

    $x = $CLASS->new("NaN");
    $x -= $x;
    is($x->is_nan(), 1,
       qq|\$x = $CLASS->new("NaN"); \$x -= \$x; \$x->is_nan() = 1|);

    $x = $CLASS->new("inf");
    $x -= $x;
    is($x->is_nan(), 1,
       qq|\$x = $CLASS->new("inf"); \$x -= \$x; \$x->is_nan() = 1|);

    $x = $CLASS->new("-inf");
    $x -= $x;
    is($x->is_nan(), 1,
       qq|\$x = $CLASS->new("-inf"); \$x -= \$x; \$x->is_nan() = 1|);

    $x = $CLASS->new("NaN");
    $x += $x;
    is($x->is_nan(), 1,
       qq|\$x = $CLASS->new("NaN"); \$x += \$x; \$x->is_nan() = 1|);

    $x = $CLASS->new("inf");
    $x += $x;
    is($x->is_inf(), 1,
       qq|\$x = $CLASS->new("inf"); \$x += \$x; \$x->is_inf() = 1|);

    $x = $CLASS->new("-inf");
    $x += $x;
    is($x->is_inf("-"), 1,
       qq|\$x = $CLASS->new("-inf"); \$x += \$x; \$x->is_inf("-") = 1|);

    $x = $CLASS->new(3);
    $x += $x;
    is($x, 6, qq|\$x = $CLASS->new(3); \$x += \$x; \$x = 6|);

    $x = $CLASS->new(-3);
    $x += $x;
    is($x, -6, qq|\$x = $CLASS->new(-3); \$x += \$x; \$x = -6|);

    $x = $CLASS->new(3);
    $x *= $x;
    is($x, 9, qq|\$x = $CLASS->new(3); \$x *= \$x; \$x = 9|);

    $x = $CLASS->new(-3);
    $x *= $x;
    is($x, 9, qq|\$x = $CLASS->new(-3); \$x *= \$x; \$x = 9|);

    $x = $CLASS->new(3);
    $x /= $x;
    is($x, 1, qq|\$x = $CLASS->new(3); \$x /= \$x; \$x = 1|);

    $x = $CLASS->new(-3);
    $x /= $x;
    is($x, 1, qq|\$x = $CLASS->new(-3); \$x /= \$x; \$x = 1|);

    $x = $CLASS->new(3);
    $x %= $x;
    is($x, 0, qq|\$x = $CLASS->new(3); \$x %= \$x; \$x = 0|);

    $x = $CLASS->new(-3);
    $x %= $x;
    is($x, 0, qq|\$x = $CLASS->new(-3); \$x %= \$x; \$x = 0|);
}

###############################################################################
# all tests done

1;

###############################################################################
# sub to check validity of a Math::BigInt internally, to ensure that no op
# leaves a number object in an invalid state (f.i. "-0")

sub is_valid {
    my ($x, $f) = @_;

    my $e = 0;                  # error?

    # allow the check to pass for all Lite, and all MBI and subclasses
    # ok as reference?
    $e = 'Not a reference to Math::BigInt' if ref($x) !~ /^Math::BigInt/;

    if (ref($x) ne 'Math::BigInt::Lite') {
        # has ok sign?
        $e = qq|Illegal sign $x->{sign}|
          . qq| (expected: "+", "-", "-inf", "+inf" or "NaN"|
            if $e eq '0' && $x->{sign} !~ /^(\+|-|\+inf|-inf|NaN)$/;

        $e = "-0 is invalid!" if $e ne '0' && $x->{sign} eq '-' && $x == 0;
        $e = $LIB->_check($x->{value}) if $e eq '0';
    }

    # test done, see if error did crop up
    if ($e eq '0') {
        pass('is a valid object');
        return;
    }

    fail($e . " after op '$f'");
}

__DATA__

&.=
1234:-345:1234-345

&+=
1:2:3
-1:-2:-3

&-=
1:2:-1
-1:-2:1

&*=
2:3:6
-1:5:-5

&%=
100:3:1
8:9:8
-629:5033:4404

&/=
100:3:33
-8:2:-4

&|=
2:1:3

&&=
5:7:5

&^=
5:7:2

&blog
#
invalid:2:NaN
122:invalid:NaN
invalid:invalid:NaN
#
122:inf:0
inf:122:inf
122:-inf:0
-inf:122:inf
-inf:-inf:NaN
0:4:-inf
-21:4:NaN
21:-21:NaN
#
0:-inf:NaN
0:-1:NaN
0:0:NaN
0:1:NaN
0:inf:NaN
#
1:-inf:0
1:-1:0
1:0:0
1:1:NaN
1:4:0
1:inf:0
#
inf:-inf:NaN
inf:-1:NaN
inf:0:NaN
inf:1:NaN
inf:4:inf
inf:inf:NaN
#
# normal results
1024:2:10
81:3:4
# 3.01.. truncate
82:3:4
# 3.9... truncate
80:3:3
4096:2:12
15625:5:6
15626:5:6
15624:5:5
1000:10:3
10000:10:4
100000:10:5
1000000:10:6
10000000:10:7
100000000:10:8
8916100448256:12:12
8916100448257:12:12
8916100448255:12:11
2251799813685248:8:17
72057594037927936:2:56
144115188075855872:2:57
288230376151711744:2:58
576460752303423488:2:59
1329227995784915872903807060280344576:2:120
# $x == $base => result 1
3:3:1
# $x < $base => result 0 ($base ** 0 <= $x)
3:4:0
# $x == 1 => result 0
1:5:0

&is_negative
0:0
-1:1
1:0
+inf:0
-inf:1
invalid:0

&is_positive
0:0
-1:0
1:1
+inf:1
-inf:0
invalid:0

&is_non_negative
0:1
-1:0
1:1
+inf:1
-inf:0
NaN:0

&is_non_positive
0:1
-1:1
1:0
+inf:0
-inf:1
NaN:0

&is_int
-inf:0
+inf:0
invalid:0
1:1
0:1
123e12:1

&is_odd
abc:0
0:0
1:1
3:1
-1:1
-3:1
10000001:1
10000002:0
2:0
120:0
121:1

&is_even
abc:0
0:1
1:0
3:0
-1:0
-3:0
10000001:0
10000002:1
2:1
120:1
121:0

&bacmp
+0:-0:0
+0:+1:-1
-1:+1:0
+1:-1:0
-1:+2:-1
+2:-1:1
-123456789:+987654321:-1
+123456789:-987654321:-1
+987654321:+123456789:1
-987654321:+123456789:1
-123:+4567889:-1
# NaNs
invalid:123:
123:invalid:
invalid:invalid:
# infinity
+inf:+inf:0
-inf:-inf:0
+inf:-inf:0
-inf:+inf:0
+inf:123:1
-inf:123:1
+inf:-123:1
-inf:-123:1
123:-inf:-1
-123:inf:-1
-123:-inf:-1
123:inf:-1
# return undef
+inf:NaN:
NaN:inf:
-inf:NaN:
NaN:-inf:

&bnorm
0e999:0
0e-999:0
-0e999:0
-0e-999:0
123:123
123.000:123
123e0:123
123e+0:123
123e-0:123
123.000e0:123
123.000e+0:123
123.000e-0:123
# binary input
0babc:NaN
0b123:NaN
0b0:0
-0b0:0
-0b1:-1
0b0001:1
0b001:1
0b011:3
0b101:5
0b1001:9
0b10001:17
0b100001:33
0b1000001:65
0b10000001:129
0b100000001:257
0b1000000001:513
0b10000000001:1025
0b100000000001:2049
0b1000000000001:4097
0b10000000000001:8193
0b100000000000001:16385
0b1000000000000001:32769
0b10000000000000001:65537
0b100000000000000001:131073
0b1000000000000000001:262145
0b10000000000000000001:524289
0b100000000000000000001:1048577
0b1000000000000000000001:2097153
0b10000000000000000000001:4194305
0b100000000000000000000001:8388609
0b1000000000000000000000001:16777217
0b10000000000000000000000001:33554433
0b100000000000000000000000001:67108865
0b1000000000000000000000000001:134217729
0b10000000000000000000000000001:268435457
0b100000000000000000000000000001:536870913
0b1000000000000000000000000000001:1073741825
0b10000000000000000000000000000001:2147483649
0b100000000000000000000000000000001:4294967297
0b1000000000000000000000000000000001:8589934593
0b10000000000000000000000000000000001:17179869185
0b1_0_1:5
0b0_0_0_1:1
# hex input
-0x0:0
0xabcdefgh:NaN
0x1234:4660
0xabcdef:11259375
-0xABCDEF:-11259375
-0x1234:-4660
0x12345678:305419896
0x1_2_3_4_56_78:305419896
0xa_b_c_d_e_f:11259375
0x9:9
0x11:17
0x21:33
0x41:65
0x81:129
0x101:257
0x201:513
0x401:1025
0x801:2049
0x1001:4097
0x2001:8193
0x4001:16385
0x8001:32769
0x10001:65537
0x20001:131073
0x40001:262145
0x80001:524289
0x100001:1048577
0x200001:2097153
0x400001:4194305
0x800001:8388609
0x1000001:16777217
0x2000001:33554433
0x4000001:67108865
0x8000001:134217729
0x10000001:268435457
0x20000001:536870913
0x40000001:1073741825
0x80000001:2147483649
0x100000001:4294967297
0x200000001:8589934593
0x400000001:17179869185
0x800000001:34359738369
# bug found by Mark Lakata in Calc.pm creating too big one-element numbers
# in _from_hex()
0x2dd59e18a125dbed30a6ab1d93e9c855569f44f75806f0645dc9a2e98b808c3:1295719234436071846486578237372801883390756472611551858964079371952886122691
# inf input
inf:inf
+inf:inf
-inf:-inf
0inf:NaN
# abnormal input
:NaN
abc:NaN
   1 a:NaN
1bcd2:NaN
11111b:NaN
+1z:NaN
-1z:NaN
# only one underscore between two digits
_123:NaN
_123_:NaN
1E_12:NaN
1_E_12:NaN
+_1E12:NaN
+0_1E2:100
+0_0_1E2:100
-0_0_1E2:-100
-0_0_1E+0_0_2:-100
E1:NaN
E23:NaN
1.23E1:NaN
1.23E-1:NaN
# bug with two E's in number being valid
1e2e3:NaN
1e2r:NaN
1e2.0:NaN
# bug with two '.' in number being valid
1.2.2:NaN
1.2.3e1:NaN
-1.2.3:NaN
-1.2.3e-4:NaN
1.2e3.4:NaN
1.2e-3.4:NaN
1.2.3.4:NaN
1.2.t:NaN
1..2:NaN
1..2e1:NaN
1..2e1..1:NaN
12e1..1:NaN
..2:NaN
.-2:NaN
# leading zeros
012:12
0123:123
01234:1234
012345:12345
0123456:123456
01234567:1234567
012345678:12345678
0123456789:123456789
01234567891:1234567891
012345678912:12345678912
0123456789123:123456789123
01234567891234:1234567891234
# some inputs that result in zero
0e0:0
+0e0:0
+0e+0:0
-0e+0:0
0e-0:0
-0e-0:0
+0e-0:0
000:0
00e2:0
00e02:0
000e002:0
000e1230:0
00e-3:0
00e+3:0
00e-03:0
00e+03:0
-000:0
-00e2:0
-00e02:0
-000e002:0
-000e1230:0
-00e-3:0
-00e+3:0
-00e-03:0
-00e+03:0
# normal input
0:0
+0:0
+00:0
+000:0
000000000000000000:0
-0:0
-0000:0
+1:1
+01:1
+001:1
+00000100000:100000
123456789:123456789
-1:-1
-01:-1
-001:-1
-123456789:-123456789
-00000100000:-100000
1_2_3:123
10000000000E-1_0:1
1E2:100
1E1:10
1E0:1
1.23E2:123
100E-1:10
# floating point input
# .2e2:20
1.E3:1000
1.01E2:101
1010E-1:101
-1010E0:-1010
-1010E1:-10100
1234.00:1234
# non-integer numbers
-1010E-2:NaN
-1.01E+1:NaN
-1.01E-1:NaN
1E-999999:NaN
0.5:NaN

&bnan
1:NaN
2:NaN
abc:NaN

&bone
2:+:1
2:-:-1
invalid:-:-1
invalid:+:1
3::1

&binf
1:+:inf
2:-:-inf
3:+inf:inf

&is_nan
123:0
abc:1
NaN:1
-123:0

&is_inf
+inf::1
-inf::1
abc::0
1::0
NaN::0
-1::0
+inf:-:0
+inf:+:1
-inf:-:1
-inf:+:0
-inf:-inf:1
-inf:+inf:0
+inf:-inf:0
+inf:+inf:1
+iNfInItY::1
-InFiNiTy::1

&blsft
abc:abc:NaN
+2:+2:8
+1:+32:4294967296
+1:+48:281474976710656
+8:-2:NaN
# exercise base 10
+12345:4:10:123450000
-1234:0:10:-1234
+1234:0:10:1234
+2:2:10:200
+12:2:10:1200
+1234:-3:10:NaN
1234567890123:12:10:1234567890123000000000000
-3:1:2:-6
-5:1:2:-10
-2:1:2:-4
-102533203:1:2:-205066406

&brsft
abc:abc:NaN
+8:+2:2
+4294967296:+32:1
+281474976710656:+48:1
+2:-2:NaN
# exercise base 10
-1234:0:10:-1234
+1234:0:10:1234
+200:2:10:2
+1234:3:10:1
+1234:2:10:12
+1234:-3:10:NaN
310000:4:10:31
12300000:5:10:123
1230000000000:10:10:123
09876123456789067890:12:10:9876123
1234561234567890123:13:10:123456
820265627:1:2:410132813
# test shifting negative numbers in base 2
-15:1:2:-8
-14:1:2:-7
-13:1:2:-7
-12:1:2:-6
-11:1:2:-6
-10:1:2:-5
-9:1:2:-5
-8:1:2:-4
-7:1:2:-4
-6:1:2:-3
-5:1:2:-3
-4:1:2:-2
-3:1:2:-2
-2:1:2:-1
-1:1:2:-1
-1640531254:2:2:-410132814
-1640531254:1:2:-820265627
-820265627:1:2:-410132814
-205066405:1:2:-102533203

&bsstr
+inf:inf
-inf:-inf
1e+34:1e+34
123.456E3:123456e+0
100:1e+2
bsstrabc:NaN
-5:-5e+0
-100:-1e+2

&numify
5:5
-5:-5
100:100
-100:-100

&bneg
invalid:NaN
+inf:-inf
-inf:inf
abd:NaN
0:0
1:-1
-1:1
+123456789:-123456789
-123456789:123456789

&babs
invalid:NaN
+inf:inf
-inf:inf
0:0
1:1
-1:1
+123456789:123456789
-123456789:123456789

&bsgn
NaN:NaN
+inf:1
-inf:-1
0:0
+123456789:1
-123456789:-1

&bcmp
invalid:invalid:
invalid:0:
0:invalid:
0:0:0
-1:0:-1
0:-1:1
1:0:1
0:1:-1
-1:1:-1
1:-1:1
-1:-1:0
1:1:0
123:123:0
123:12:1
12:123:-1
-123:-123:0
-123:-12:-1
-12:-123:1
123:124:-1
124:123:1
-123:-124:1
-124:-123:-1
100:5:1
-123456789:987654321:-1
+123456789:-987654321:1
-987654321:123456789:-1
-inf:5432112345:-1
+inf:5432112345:1
-inf:-5432112345:-1
+inf:-5432112345:1
+inf:+inf:0
-inf:-inf:0
+inf:-inf:1
-inf:+inf:-1
5:inf:-1
5:inf:-1
-5:-inf:1
-5:-inf:1
# return undef
+inf:NaN:
NaN:inf:
-inf:NaN:
NaN:-inf:

&binc
abc:NaN
+inf:inf
-inf:-inf
+0:1
+1:2
-1:0

&bdec
abc:NaN
+inf:inf
-inf:-inf
+0:-1
+1:0
-1:-2

&badd
abc:abc:NaN
abc:0:NaN
+0:abc:NaN
+inf:-inf:NaN
-inf:+inf:NaN
+inf:+inf:inf
-inf:-inf:-inf
invalid:+inf:NaN
invalid:+inf:NaN
+inf:invalid:NaN
-inf:invalid:NaN
0:0:0
1:0:1
0:1:1
1:1:2
-1:0:-1
0:-1:-1
-1:-1:-2
-1:+1:0
+1:-1:0
+9:+1:10
+99:+1:100
+999:+1:1000
+9999:+1:10000
+99999:+1:100000
+999999:+1:1000000
+9999999:+1:10000000
+99999999:+1:100000000
+999999999:+1:1000000000
+9999999999:+1:10000000000
+99999999999:+1:100000000000
+10:-1:9
+100:-1:99
+1000:-1:999
+10000:-1:9999
+100000:-1:99999
+1000000:-1:999999
+10000000:-1:9999999
+100000000:-1:99999999
+1000000000:-1:999999999
+10000000000:-1:9999999999
+123456789:987654321:1111111110
-123456789:987654321:864197532
-123456789:-987654321:-1111111110
+123456789:-987654321:-864197532
-1:10001:10000
-1:100001:100000
-1:1000001:1000000
-1:10000001:10000000
-1:100000001:100000000
-1:1000000001:1000000000
-1:10000000001:10000000000
-1:100000000001:100000000000
-1:1000000000001:1000000000000
-1:10000000000001:10000000000000
-1:-10001:-10002
-1:-100001:-100002
-1:-1000001:-1000002
-1:-10000001:-10000002
-1:-100000001:-100000002
-1:-1000000001:-1000000002
-1:-10000000001:-10000000002
-1:-100000000001:-100000000002
-1:-1000000000001:-1000000000002
-1:-10000000000001:-10000000000002

&bsub
abc:abc:NaN
abc:+0:NaN
+0:abc:NaN
+inf:-inf:inf
-inf:+inf:-inf
+inf:+inf:NaN
-inf:-inf:NaN
+0:+0:0
+1:+0:1
+0:+1:-1
+1:+1:0
-1:+0:-1
+0:-1:1
-1:-1:0
-1:+1:-2
+1:-1:2
+9:+1:8
+99:+1:98
+999:+1:998
+9999:+1:9998
+99999:+1:99998
+999999:+1:999998
+9999999:+1:9999998
+99999999:+1:99999998
+999999999:+1:999999998
+9999999999:+1:9999999998
+99999999999:+1:99999999998
+10:-1:11
+100:-1:101
+1000:-1:1001
+10000:-1:10001
+100000:-1:100001
+1000000:-1:1000001
+10000000:-1:10000001
+100000000:-1:100000001
+1000000000:-1:1000000001
+10000000000:-1:10000000001
+123456789:+987654321:-864197532
-123456789:+987654321:-1111111110
-123456789:-987654321:864197532
+123456789:-987654321:1111111110
10001:1:10000
100001:1:100000
1000001:1:1000000
10000001:1:10000000
100000001:1:100000000
1000000001:1:1000000000
10000000001:1:10000000000
100000000001:1:100000000000
1000000000001:1:1000000000000
10000000000001:1:10000000000000
10001:-1:10002
100001:-1:100002
1000001:-1:1000002
10000001:-1:10000002
100000001:-1:100000002
1000000001:-1:1000000002
10000000001:-1:10000000002
100000000001:-1:100000000002
1000000000001:-1:1000000000002
10000000000001:-1:10000000000002

&bmuladd
abc:abc:0:NaN
abc:+0:0:NaN
+0:abc:0:NaN
+0:0:abc:NaN
invalid:+inf:0:NaN
invalid:-inf:0:NaN
-inf:invalid:0:NaN
+inf:invalid:0:NaN
+inf:+inf:0:inf
+inf:-inf:0:-inf
-inf:+inf:0:-inf
-inf:-inf:0:inf
+0:+0:0:0
+0:+1:0:0
+1:+0:0:0
+0:-1:0:0
-1:+0:0:0
123456789123456789:0:0:0
0:123456789123456789:0:0
-1:-1:0:1
-1:-1:0:1
-1:+1:0:-1
+1:-1:0:-1
+1:+1:0:1
+2:+3:0:6
-2:+3:0:-6
+2:-3:0:-6
-2:-3:0:6
111:111:0:12321
10101:10101:0:102030201
1001001:1001001:0:1002003002001
100010001:100010001:0:10002000300020001
10000100001:10000100001:0:100002000030000200001
11111111111:9:0:99999999999
22222222222:9:0:199999999998
33333333333:9:0:299999999997
44444444444:9:0:399999999996
55555555555:9:0:499999999995
66666666666:9:0:599999999994
77777777777:9:0:699999999993
88888888888:9:0:799999999992
99999999999:9:0:899999999991
11111111111:9:1:100000000000
22222222222:9:1:199999999999
33333333333:9:1:299999999998
44444444444:9:1:399999999997
55555555555:9:1:499999999996
66666666666:9:1:599999999995
77777777777:9:1:699999999994
88888888888:9:1:799999999993
99999999999:9:1:899999999992
-3:-4:-5:7
3:-4:-5:-17
-3:4:-5:-17
3:4:-5:7
-3:4:5:-7
3:-4:5:-7
9999999999999999999:10000000000000000000:1234567890:99999999999999999990000000001234567890
2:3:12345678901234567890:12345678901234567896

&bmul
abc:abc:NaN
abc:+0:NaN
+0:abc:NaN
invalid:+inf:NaN
invalid:-inf:NaN
-inf:invalid:NaN
+inf:invalid:NaN
+inf:+inf:inf
+inf:-inf:-inf
-inf:+inf:-inf
-inf:-inf:inf
+0:+0:0
+0:+1:0
+1:+0:0
+0:-1:0
-1:+0:0
123456789123456789:0:0
0:123456789123456789:0
-1:-1:1
-1:+1:-1
+1:-1:-1
+1:+1:1
+2:+3:6
-2:+3:-6
+2:-3:-6
-2:-3:6
111:111:12321
10101:10101:102030201
1001001:1001001:1002003002001
100010001:100010001:10002000300020001
10000100001:10000100001:100002000030000200001
11111111111:9:99999999999
22222222222:9:199999999998
33333333333:9:299999999997
44444444444:9:399999999996
55555555555:9:499999999995
66666666666:9:599999999994
77777777777:9:699999999993
88888888888:9:799999999992
99999999999:9:899999999991
+25:+25:625
+12345:+12345:152399025
+99999:+11111:1111088889
9999:10000:99990000
99999:100000:9999900000
999999:1000000:999999000000
9999999:10000000:99999990000000
99999999:100000000:9999999900000000
999999999:1000000000:999999999000000000
9999999999:10000000000:99999999990000000000
99999999999:100000000000:9999999999900000000000
999999999999:1000000000000:999999999999000000000000
9999999999999:10000000000000:99999999999990000000000000
99999999999999:100000000000000:9999999999999900000000000000
999999999999999:1000000000000000:999999999999999000000000000000
9999999999999999:10000000000000000:99999999999999990000000000000000
99999999999999999:100000000000000000:9999999999999999900000000000000000
999999999999999999:1000000000000000000:999999999999999999000000000000000000
9999999999999999999:10000000000000000000:99999999999999999990000000000000000000

&bdiv-list

# Divide by zero and modulo zero.

inf:0:inf,inf
5:0:inf,5
0:0:NaN,0
-5:0:-inf,-5
-inf:0:-inf,-inf

# Numerator (dividend) is +/-inf, and denominator is finite and non-zero.

inf:-inf:NaN,NaN
inf:-5:-inf,NaN
inf:5:inf,NaN
inf:inf:NaN,NaN

-inf:-inf:NaN,NaN
-inf:-5:inf,NaN
-inf:5:-inf,NaN
-inf:inf:NaN,NaN

# Denominator (divisor) is +/-inf. The cases when the numerator is +/-inf
# are covered above.

-5:inf:-1,inf
0:inf:0,0
5:inf:0,5

-5:-inf:0,-5
0:-inf:0,0
5:-inf:-1,-inf

# Numerator is finite, and denominator is finite and non-zero.

-5:-5:1,0
-5:-2:2,-1
-5:-1:5,0
-5:1:-5,0
-5:2:-3,1
-5:5:-1,0
-2:-5:0,-2
-2:-2:1,0
-2:-1:2,0
-2:1:-2,0
-2:2:-1,0
-2:5:-1,3
-1:-5:0,-1
-1:-2:0,-1
-1:-1:1,0
-1:1:-1,0
-1:2:-1,1
-1:5:-1,4
0:-5:0,0
0:-2:0,0
0:-1:0,0
0:1:0,0
0:2:0,0
0:5:0,0
1:-5:-1,-4
1:-2:-1,-1
1:-1:-1,0
1:1:1,0
1:2:0,1
1:5:0,1
2:-5:-1,-3
2:-2:-1,0
2:-1:-2,0
2:1:2,0
2:2:1,0
2:5:0,2
5:-5:-1,0
5:-2:-3,-1
5:-1:-5,0
5:1:5,0
5:2:2,1
5:5:1,0

# test the shortcut in Calc if @$x == @$yorg
1234567812345678:123456712345678:10,688888898
12345671234567:1234561234567:10,58888897
123456123456:12345123456:10,4888896
1234512345:123412345:10,388895
1234567890999999999:1234567890:1000000000,999999999
1234567890000000000:1234567890:1000000000,0
1234567890999999999:9876543210:124999998,9503086419
1234567890000000000:9876543210:124999998,8503086420
96969696969696969696969696969678787878626262626262626262626262:484848484848484848484848486666666666666689898989898989898989:199,484848484848484848484848123012121211954972727272727272727451
# bug in v1.76
1267650600228229401496703205375:1267650600228229401496703205376:0,1267650600228229401496703205375
# exercise shortcut for numbers of the same length in div
999999999999999999999999999999999:999999999999999999999999999999999:1,0
999999999999999999999999999999999:888888888888888888888888888888888:1,111111111111111111111111111111111
999999999999999999999999999999999:777777777777777777777777777777777:1,222222222222222222222222222222222
999999999999999999999999999999999:666666666666666666666666666666666:1,333333333333333333333333333333333
999999999999999999999999999999999:555555555555555555555555555555555:1,444444444444444444444444444444444
999999999999999999999999999999999:444444444444444444444444444444444:2,111111111111111111111111111111111
999999999999999999999999999999999:333333333333333333333333333333333:3,0
999999999999999999999999999999999:222222222222222222222222222222222:4,111111111111111111111111111111111
999999999999999999999999999999999:111111111111111111111111111111111:9,0
9999999_9999999_9999999_9999999:3333333_3333333_3333333_3333333:3,0
9999999_9999999_9999999_9999999:3333333_0000000_0000000_0000000:3,999999999999999999999
9999999_9999999_9999999_9999999:3000000_0000000_0000000_0000000:3,999999999999999999999999999
9999999_9999999_9999999_9999999:2000000_0000000_0000000_0000000:4,1999999999999999999999999999
9999999_9999999_9999999_9999999:1000000_0000000_0000000_0000000:9,999999999999999999999999999
9999999_9999999_9999999_9999999:100000_0000000_0000000_0000000:99,99999999999999999999999999
9999999_9999999_9999999_9999999:10000_0000000_0000000_0000000:999,9999999999999999999999999
9999999_9999999_9999999_9999999:1000_0000000_0000000_0000000:9999,999999999999999999999999
9999999_9999999_9999999_9999999:100_0000000_0000000_0000000:99999,99999999999999999999999
9999999_9999999_9999999_9999999:10_0000000_0000000_0000000:999999,9999999999999999999999
9999999_9999999_9999999_9999999:1_0000000_0000000_0000000:9999999,999999999999999999999

&bdiv

# Divide by zero and modulo zero.

inf:0:inf
5:0:inf
0:0:NaN
-5:0:-inf
-inf:0:-inf

# Numerator (dividend) is +/-inf, and denominator is finite and non-zero.

inf:-inf:NaN
inf:-5:-inf
inf:5:inf
inf:inf:NaN

-inf:-inf:NaN
-inf:-5:inf
-inf:5:-inf
-inf:inf:NaN

# Denominator (divisor) is +/-inf. The cases when the numerator is +/-inf
# are covered above.

-5:inf:-1
0:inf:0
5:inf:0

-5:-inf:0
0:-inf:0
5:-inf:-1

# Numerator is finite, and denominator is finite and non-zero.

5:5:1
-5:-5:1
11:2:5
-11:-2:5
-11:2:-6
11:-2:-6
0:1:0
0:-1:0
1:1:1
-1:-1:1
1:-1:-1
-1:1:-1
1:2:0
2:1:2
1:26:0
1000000000:9:111111111
2000000000:9:222222222
3000000000:9:333333333
4000000000:9:444444444
5000000000:9:555555555
6000000000:9:666666666
7000000000:9:777777777
8000000000:9:888888888
9000000000:9:1000000000
35500000:113:314159
71000000:226:314159
106500000:339:314159
1000000000:3:333333333
+10:+5:2
+100:+4:25
+1000:+8:125
+10000:+16:625
999999999999:9:111111111111
999999999999:99:10101010101
999999999999:999:1001001001
999999999999:9999:100010001
999999999999999:99999:10000100001
+1111088889:99999:11111
-5:-3:1
-5:3:-2
4:3:1
4:-3:-2
1:3:0
1:-3:-1
-2:-3:0
-2:3:-1
8:3:2
-8:3:-3
14:-3:-5
-14:3:-5
-14:-3:4
14:3:4
# bug in Calc with '99999' vs $BASE-1
10000000000000000000000000000000000000000000000000000000000000000000000000000000000:10000000375084540248994272022843165711074:999999962491547381984643365663244474111576
# test the shortcut in Calc if @$x == @$yorg
1234567812345678:123456712345678:10
12345671234567:1234561234567:10
123456123456:12345123456:10
1234512345:123412345:10
1234567890999999999:1234567890:1000000000
1234567890000000000:1234567890:1000000000
1234567890999999999:9876543210:124999998
1234567890000000000:9876543210:124999998
96969696969696969696969696969678787878626262626262626262626262:484848484848484848484848486666666666666689898989898989898989:199
# bug up to v0.35 in Calc (--$q one too many)
84696969696969696956565656566184292929292929292847474747436308080808080808086765396464646464646465:13131313131313131313131313131394949494949494949494949494943535353535353535353535:6449999999999999999
84696969696969696943434343434871161616161616161452525252486813131313131313143230042929292929292930:13131313131313131313131313131394949494949494949494949494943535353535353535353535:6449999999999999998
84696969696969696969696969697497424242424242424242424242385803030303030303030300750000000000000000:13131313131313131313131313131394949494949494949494949494943535353535353535353535:6450000000000000000
84696969696969696930303030303558030303030303030057575757537318181818181818199694689393939393939395:13131313131313131313131313131394949494949494949494949494943535353535353535353535:6449999999999999997
# exercise shortcut for numbers of the same length in div
999999999999999999999999999999999:999999999999999999999999999999999:1
999999999999999999999999999999999:888888888888888888888888888888888:1
999999999999999999999999999999999:777777777777777777777777777777777:1
999999999999999999999999999999999:666666666666666666666666666666666:1
999999999999999999999999999999999:555555555555555555555555555555555:1
999999999999999999999999999999999:444444444444444444444444444444444:2
999999999999999999999999999999999:333333333333333333333333333333333:3
999999999999999999999999999999999:222222222222222222222222222222222:4
999999999999999999999999999999999:111111111111111111111111111111111:9
9999999_9999999_9999999_9999999:3333333_3333333_3333333_3333333:3
9999999_9999999_9999999_9999999:3333333_0000000_0000000_0000000:3
9999999_9999999_9999999_9999999:3000000_0000000_0000000_0000000:3
9999999_9999999_9999999_9999999:2000000_0000000_0000000_0000000:4
9999999_9999999_9999999_9999999:1000000_0000000_0000000_0000000:9
9999999_9999999_9999999_9999999:100000_0000000_0000000_0000000:99
9999999_9999999_9999999_9999999:10000_0000000_0000000_0000000:999
9999999_9999999_9999999_9999999:1000_0000000_0000000_0000000:9999
9999999_9999999_9999999_9999999:100_0000000_0000000_0000000:99999
9999999_9999999_9999999_9999999:10_0000000_0000000_0000000:999999
9999999_9999999_9999999_9999999:1_0000000_0000000_0000000:9999999
# bug with shortcut in Calc 0.44
949418181818187070707070707070707070:181818181853535353535353535353535353:5

&btdiv-list

# Divide by zero and modulo zero.

inf:0:inf,inf
5:0:inf,5
0:0:NaN,0
-5:0:-inf,-5
-inf:0:-inf,-inf

# Numerator (dividend) is +/-inf, and denominator is finite and non-zero.

inf:-inf:NaN,NaN
inf:-5:-inf,NaN
inf:5:inf,NaN
inf:inf:NaN,NaN

-inf:-inf:NaN,NaN
-inf:-5:inf,NaN
-inf:5:-inf,NaN
-inf:inf:NaN,NaN

# Denominator (divisor) is +/-inf. The cases when the numerator is +/-inf
# are covered above.

-5:inf:0,-5
0:inf:0,0
5:inf:0,5

-5:-inf:0,-5
0:-inf:0,0
5:-inf:0,5

# Numerator is finite, and denominator is finite and non-zero.

-5:-5:1,0
-5:-2:2,-1
-5:-1:5,0
-5:1:-5,0
-5:2:-2,-1
-5:5:-1,0
-2:-5:0,-2
-2:-2:1,0
-2:-1:2,0
-2:1:-2,0
-2:2:-1,0
-2:5:0,-2
-1:-5:0,-1
-1:-2:0,-1
-1:-1:1,0
-1:1:-1,0
-1:2:0,-1
-1:5:0,-1
0:-5:0,0
0:-2:0,0
0:-1:0,0
0:1:0,0
0:2:0,0
0:5:0,0
1:-5:0,1
1:-2:0,1
1:-1:-1,0
1:1:1,0
1:2:0,1
1:5:0,1
2:-5:0,2
2:-2:-1,0
2:-1:-2,0
2:1:2,0
2:2:1,0
2:5:0,2
5:-5:-1,0
5:-2:-2,1
5:-1:-5,0
5:1:5,0
5:2:2,1
5:5:1,0

&btdiv

# Divide by zero and modulo zero.

inf:0:inf
5:0:inf
0:0:NaN
-5:0:-inf
-inf:0:-inf

# Numerator (dividend) is +/-inf, and denominator is finite and non-zero.

inf:-inf:NaN
inf:-5:-inf
inf:5:inf
inf:inf:NaN

-inf:-inf:NaN
-inf:-5:inf
-inf:5:-inf
-inf:inf:NaN

# Denominator (divisor) is +/-inf. The cases when the numerator is +/-inf
# are covered above.

-5:inf:0
0:inf:0
5:inf:0

-5:-inf:0
0:-inf:0
5:-inf:0

# Numerator is finite, and denominator is finite and non-zero.

-5:-5:1
-5:-2:2
-5:-1:5
-5:1:-5
-5:2:-2
-5:5:-1
-2:-5:0
-2:-2:1
-2:-1:2
-2:1:-2
-2:2:-1
-2:5:0
-1:-5:0
-1:-2:0
-1:-1:1
-1:1:-1
-1:2:0
-1:5:0
0:-5:0
0:-2:0
0:-1:0
0:1:0
0:2:0
0:5:0
1:-5:0
1:-2:0
1:-1:-1
1:1:1
1:2:0
1:5:0
2:-5:0
2:-2:-1
2:-1:-2
2:1:2
2:2:1
2:5:0
5:-5:-1
5:-2:-2
5:-1:-5
5:1:5
5:2:2
5:5:1

###############################################################################

&bmodinv
# format: number:modulus:result
# bmodinv Data errors
abc:abc:NaN
abc:5:NaN
5:abc:NaN
# bmodinv Expected Results from normal use
1:5:1
3:5:2
3:-5:-3
-2:5:2
8:5033:4404
1234567891:13:6
-1234567891:13:7
324958749843759385732954874325984357439658735983745:2348249874968739:1741662881064902
-2:1:0
-1:1:0
0:1:0
1:1:0
2:1:0
3:1:0
4:1:0
-2:3:1
-1:3:2
0:3:NaN
1:3:1
2:3:2
3:3:NaN
4:3:1
-2:4:NaN
-1:4:3
0:4:NaN
1:4:1
2:4:NaN
3:4:3
4:4:NaN
## bmodinv Error cases / useless use of function
inf:5:NaN
5:inf:NaN
-inf:5:NaN
5:-inf:NaN

&bmodpow
# format: number:exponent:modulus:result
# bmodpow Data errors
abc:abc:abc:NaN
5:abc:abc:NaN
abc:5:abc:NaN
abc:abc:5:NaN
5:5:abc:NaN
5:abc:5:NaN
abc:5:5:NaN
3:5:0:3
# bmodpow Expected results
0:0:2:1
1:0:2:1
0:3:5:0
-2:-2:1:0
-1:-2:1:0
0:-2:1:0
1:-2:1:0
2:-2:1:0
3:-2:1:0
4:-2:1:0
-2:-1:1:0
-1:-1:1:0
0:-1:1:0
1:-1:1:0
2:-1:1:0
3:-1:1:0
4:-1:1:0
-2:0:1:0
-1:0:1:0
0:0:1:0
1:0:1:0
2:0:1:0
3:0:1:0
4:0:1:0
-2:1:1:0
-1:1:1:0
0:1:1:0
1:1:1:0
2:1:1:0
3:1:1:0
4:1:1:0
-2:2:1:0
-1:2:1:0
0:2:1:0
1:2:1:0
2:2:1:0
3:2:1:0
4:2:1:0
-2:3:1:0
-1:3:1:0
0:3:1:0
1:3:1:0
2:3:1:0
3:3:1:0
4:3:1:0
-2:4:1:0
-1:4:1:0
0:4:1:0
1:4:1:0
2:4:1:0
3:4:1:0
4:4:1:0
-2:-2:3:1
-1:-2:3:1
0:-2:3:NaN
1:-2:3:1
2:-2:3:1
3:-2:3:NaN
4:-2:3:1
-2:-1:3:1
-1:-1:3:2
0:-1:3:NaN
1:-1:3:1
2:-1:3:2
3:-1:3:NaN
4:-1:3:1
-2:0:3:1
-1:0:3:1
0:0:3:1
1:0:3:1
2:0:3:1
3:0:3:1
4:0:3:1
-2:1:3:1
-1:1:3:2
0:1:3:0
1:1:3:1
2:1:3:2
3:1:3:0
4:1:3:1
-2:2:3:1
-1:2:3:1
0:2:3:0
1:2:3:1
2:2:3:1
3:2:3:0
4:2:3:1
-2:3:3:1
-1:3:3:2
0:3:3:0
1:3:3:1
2:3:3:2
3:3:3:0
4:3:3:1
-2:4:3:1
-1:4:3:1
0:4:3:0
1:4:3:1
2:4:3:1
3:4:3:0
4:4:3:1
-2:-2:4:NaN
-1:-2:4:1
0:-2:4:NaN
1:-2:4:1
2:-2:4:NaN
3:-2:4:1
4:-2:4:NaN
-2:-1:4:NaN
-1:-1:4:3
0:-1:4:NaN
1:-1:4:1
2:-1:4:NaN
3:-1:4:3
4:-1:4:NaN
-2:0:4:1
-1:0:4:1
0:0:4:1
1:0:4:1
2:0:4:1
3:0:4:1
4:0:4:1
-2:1:4:2
-1:1:4:3
0:1:4:0
1:1:4:1
2:1:4:2
3:1:4:3
4:1:4:0
-2:2:4:0
-1:2:4:1
0:2:4:0
1:2:4:1
2:2:4:0
3:2:4:1
4:2:4:0
-2:3:4:0
-1:3:4:3
0:3:4:0
1:3:4:1
2:3:4:0
3:3:4:3
4:3:4:0
-2:4:4:0
-1:4:4:1
0:4:4:0
1:4:4:1
2:4:4:0
3:4:4:1
4:4:4:0
8:-1:16:NaN
8:-1:5033:4404
8:7:5032:3840
8:8:-5:-4
1e50:1:1:0
98436739867439843769485798542749827593285729587325:43698764986460981048259837659386739857456983759328457:6943857329857295827698367:3104744730915914415259518
# bmodpow Error cases
inf:5:13:NaN
5:inf:13:NaN

&bmod

# Divide by zero and modulo zero.

inf:0:inf
5:0:5
0:0:0
-5:0:-5
-inf:0:-inf

# Numerator (dividend) is +/-inf, and denominator is finite and non-zero.

inf:-inf:NaN
inf:-5:NaN
inf:5:NaN
inf:inf:NaN

-inf:-inf:NaN
-inf:-5:NaN
-inf:5:NaN
-inf:inf:NaN

# Denominator (divisor) is +/-inf. The cases when the numerator is +/-inf
# are covered above.

-5:inf:inf
0:inf:0
5:inf:5

-5:-inf:-5
0:-inf:0
5:-inf:-inf

# Numerator is finite, and denominator is finite and non-zero.

5:5:0
-5:-5:0
0:1:0
0:-1:0
1:1:0
-1:-1:0
1:-1:0
-1:1:0
1:2:1
2:1:0
1000000000:9:1
2000000000:9:2
3000000000:9:3
4000000000:9:4
5000000000:9:5
6000000000:9:6
7000000000:9:7
8000000000:9:8
9000000000:9:0
35500000:113:33
71000000:226:66
106500000:339:99
1000000000:3:1
10:5:0
100:4:0
1000:8:0
10000:16:0
999999999999:9:0
999999999999:99:0
999999999999:999:0
999999999999:9999:0
999999999999999:99999:0
-9:+5:1
+9:-5:-1
-9:-5:-4
-5:3:1
-2:3:1
4:3:1
1:3:1
-5:-3:-2
-2:-3:-2
4:-3:-2
1:-3:-2
4095:4095:0
100041000510123:3:0
152403346:12345:4321
9:5:4
# test shortcuts in Calc
# 1ex % 9 is always == 1, 1ex % 113 is != 1 for x = (4..9), 1ex % 10 = 0
1234:9:1
123456:9:3
12345678:9:0
1234567891:9:1
123456789123:9:6
12345678912345:9:6
1234567891234567:9:1
123456789123456789:9:0
1234:10:4
123456:10:6
12345678:10:8
1234567891:10:1
123456789123:10:3
12345678912345:10:5
1234567891234567:10:7
123456789123456789:10:9
1234:113:104
123456:113:60
12345678:113:89
1234567891:113:64
123456789123:113:95
12345678912345:113:53
1234567891234567:113:56
123456789123456789:113:39
# bug in bmod() not modifying the variable in place
-629:5033:4404
# bug in bmod() in Calc in the _div_use_div() shortcut code path,
# when X == X and X was big
111111111111111111111111111111:111111111111111111111111111111:0
12345678901234567890:12345678901234567890:0

&bgcd
inf:12:NaN
-inf:12:NaN
12:inf:NaN
12:-inf:NaN
inf:inf:NaN
inf:-inf:NaN
-inf:-inf:NaN
abc:abc:NaN
abc:+0:NaN
+0:abc:NaN
+0:+0:0
+0:+1:1
+1:+0:1
+1:+1:1
+2:+3:1
+3:+2:1
-3:+2:1
-3:-2:1
-144:-60:12
144:-60:12
144:60:12
100:625:25
4096:81:1
1034:804:2
27:90:56:1
27:90:54:9

&blcm
abc:abc:NaN
abc:+0:NaN
+0:abc:NaN
+0:+0:0
+1:+0:0
+0:+1:0
+27:+90:270
+1034:+804:415668

&band
abc:abc:NaN
abc:0:NaN
0:abc:NaN
1:2:0
3:2:2
+8:+2:0
+281474976710656:0:0
+281474976710656:1:0
+281474976710656:+281474976710656:281474976710656
281474976710656:-1:281474976710656
-2:-3:-4
-1:-1:-1
-6:-6:-6
-7:-4:-8
-7:4:0
-4:7:4
# negative argument is bitwise shorter than positive [perl #26559]
30:-3:28
123:-1:123
# equal arguments are treated special, so also do some test with unequal ones
0xFFFF:0xFFFF:0x0xFFFF
0xFFFFFF:0xFFFFFF:0x0xFFFFFF
0xFFFFFFFF:0xFFFFFFFF:0x0xFFFFFFFF
0xFFFFFFFFFF:0xFFFFFFFFFF:0x0xFFFFFFFFFF
0xFFFFFFFFFFFF:0xFFFFFFFFFFFF:0x0xFFFFFFFFFFFF
0xF0F0:0xF0F0:0x0xF0F0
0x0F0F:0x0F0F:0x0x0F0F
0xF0F0F0:0xF0F0F0:0x0xF0F0F0
0x0F0F0F:0x0F0F0F:0x0x0F0F0F
0xF0F0F0F0:0xF0F0F0F0:0x0xF0F0F0F0
0x0F0F0F0F:0x0F0F0F0F:0x0x0F0F0F0F
0xF0F0F0F0F0:0xF0F0F0F0F0:0x0xF0F0F0F0F0
0x0F0F0F0F0F:0x0F0F0F0F0F:0x0x0F0F0F0F0F
0xF0F0F0F0F0F0:0xF0F0F0F0F0F0:0x0xF0F0F0F0F0F0
0x0F0F0F0F0F0F:0x0F0F0F0F0F0F:0x0x0F0F0F0F0F0F
0x1F0F0F0F0F0F:0x3F0F0F0F0F0F:0x0x1F0F0F0F0F0F

&bior
abc:abc:NaN
abc:0:NaN
0:abc:NaN
1:2:3
+8:+2:10
+281474976710656:0:281474976710656
+281474976710656:1:281474976710657
+281474976710656:281474976710656:281474976710656
-2:-3:-1
-1:-1:-1
-6:-6:-6
-7:4:-3
-4:7:-1
+281474976710656:-1:-1
30:-3:-1
30:-4:-2
300:-76:-68
-76:300:-68
# equal arguments are treated special, so also do some test with unequal ones
0xFFFF:0xFFFF:0x0xFFFF
0xFFFFFF:0xFFFFFF:0x0xFFFFFF
0xFFFFFFFF:0xFFFFFFFF:0x0xFFFFFFFF
0xFFFFFFFFFF:0xFFFFFFFFFF:0x0xFFFFFFFFFF
0xFFFFFFFFFFFF:0xFFFFFFFFFFFF:0x0xFFFFFFFFFFFF
0:0xFFFF:0x0xFFFF
0:0xFFFFFF:0x0xFFFFFF
0:0xFFFFFFFF:0x0xFFFFFFFF
0:0xFFFFFFFFFF:0x0xFFFFFFFFFF
0:0xFFFFFFFFFFFF:0x0xFFFFFFFFFFFF
0xFFFF:0:0x0xFFFF
0xFFFFFF:0:0x0xFFFFFF
0xFFFFFFFF:0:0x0xFFFFFFFF
0xFFFFFFFFFF:0:0x0xFFFFFFFFFF
0xFFFFFFFFFFFF:0:0x0xFFFFFFFFFFFF
0xF0F0:0xF0F0:0x0xF0F0
0x0F0F:0x0F0F:0x0x0F0F
0xF0F0:0x0F0F:0x0xFFFF
0xF0F0F0:0xF0F0F0:0x0xF0F0F0
0x0F0F0F:0x0F0F0F:0x0x0F0F0F
0x0F0F0F:0xF0F0F0:0x0xFFFFFF
0xF0F0F0F0:0xF0F0F0F0:0x0xF0F0F0F0
0x0F0F0F0F:0x0F0F0F0F:0x0x0F0F0F0F
0x0F0F0F0F:0xF0F0F0F0:0x0xFFFFFFFF
0xF0F0F0F0F0:0xF0F0F0F0F0:0x0xF0F0F0F0F0
0x0F0F0F0F0F:0x0F0F0F0F0F:0x0x0F0F0F0F0F
0x0F0F0F0F0F:0xF0F0F0F0F0:0x0xFFFFFFFFFF
0xF0F0F0F0F0F0:0xF0F0F0F0F0F0:0x0xF0F0F0F0F0F0
0x0F0F0F0F0F0F:0x0F0F0F0F0F0F:0x0x0F0F0F0F0F0F
0x0F0F0F0F0F0F:0xF0F0F0F0F0F0:0x0xFFFFFFFFFFFF
0x1F0F0F0F0F0F:0xF0F0F0F0F0F0:0x0xFFFFFFFFFFFF

&bxor
abc:abc:NaN
abc:0:NaN
0:abc:NaN
1:2:3
+8:+2:10
+281474976710656:0:281474976710656
+281474976710656:1:281474976710657
+281474976710656:281474976710656:0
-2:-3:3
-1:-1:0
-6:-6:0
-7:4:-3
-4:7:-5
4:-7:-3
-4:-7:5
30:-3:-29
30:-4:-30
300:-76:-360
-76:300:-360
# equal arguments are treated special, so also do some test with unequal ones
0xFFFF:0xFFFF:0
0xFFFFFF:0xFFFFFF:0
0xFFFFFFFF:0xFFFFFFFF:0
0xFFFFFFFFFF:0xFFFFFFFFFF:0
0xFFFFFFFFFFFF:0xFFFFFFFFFFFF:0
0:0xFFFF:0x0xFFFF
0:0xFFFFFF:0x0xFFFFFF
0:0xFFFFFFFF:0x0xFFFFFFFF
0:0xFFFFFFFFFF:0x0xFFFFFFFFFF
0:0xFFFFFFFFFFFF:0x0xFFFFFFFFFFFF
0xFFFF:0:0x0xFFFF
0xFFFFFF:0:0x0xFFFFFF
0xFFFFFFFF:0:0x0xFFFFFFFF
0xFFFFFFFFFF:0:0x0xFFFFFFFFFF
0xFFFFFFFFFFFF:0:0x0xFFFFFFFFFFFF
0xF0F0:0xF0F0:0
0x0F0F:0x0F0F:0
0xF0F0:0x0F0F:0x0xFFFF
0xF0F0F0:0xF0F0F0:0
0x0F0F0F:0x0F0F0F:0
0x0F0F0F:0xF0F0F0:0x0xFFFFFF
0xF0F0F0F0:0xF0F0F0F0:0
0x0F0F0F0F:0x0F0F0F0F:0
0x0F0F0F0F:0xF0F0F0F0:0x0xFFFFFFFF
0xF0F0F0F0F0:0xF0F0F0F0F0:0
0x0F0F0F0F0F:0x0F0F0F0F0F:0
0x0F0F0F0F0F:0xF0F0F0F0F0:0x0xFFFFFFFFFF
0xF0F0F0F0F0F0:0xF0F0F0F0F0F0:0
0x0F0F0F0F0F0F:0x0F0F0F0F0F0F:0
0x0F0F0F0F0F0F:0xF0F0F0F0F0F0:0x0xFFFFFFFFFFFF

&bnot
abc:NaN
+0:-1
+8:-9
+281474976710656:-281474976710657
-1:0
-2:1
-12:11

&digit
0:0:0
12:0:2
12:1:1
123:0:3
123:1:2
123:2:1
123:-1:1
123:-2:2
123:-3:3
123456:0:6
123456:1:5
123456:2:4
123456:3:3
123456:4:2
123456:5:1
123456:-1:1
123456:-2:2
123456:-3:3
100000:-3:0
100000:0:0
100000:1:0

&mantissa
abc:NaN
1e4:1
2e0:2
123:123
-1:-1
-2:-2
+inf:inf
-inf:-inf

&exponent
abc:NaN
1e4:4
2e0:0
123:0
-1:0
-2:0
0:0
+inf:inf
-inf:inf

&parts
abc:NaN,NaN
1e4:1,4
2e0:2,0
123:123,0
-1:-1,0
-2:-2,0
0:0,0
+inf:inf,inf
-inf:-inf,inf

&bfac
NaN:NaN
+inf:inf
-inf:NaN
-1:NaN
0:1
1:1
2:2
3:6
4:24
5:120
6:720
7:5040
8:40320
9:362880
10:3628800
11:39916800
12:479001600
20:2432902008176640000
22:1124000727777607680000
69:171122452428141311372468338881272839092270544893520369393648040923257279754140647424000000000000000

&bdfac
NaN:NaN
+inf:inf
-inf:NaN
-2:NaN
-1:1
0:1
1:1
2:2
3:3
4:8
5:15
6:48
7:105
8:384
9:945
10:3840
11:10395
12:46080

&btfac
NaN:NaN
+inf:inf
-inf:NaN
-3:NaN
-2:1
-1:1
0:1
1:1
2:2
3:3
4:4
5:10
6:18
7:28
8:80
9:162
10:280
11:880
12:1944

&bmfac

7:-inf:NaN
7:-1:NaN
7:0:NaN
7:inf:7
7:NaN:NaN

NaN:1:NaN
+inf:1:inf
-inf:1:NaN
-1:1:NaN
0:1:1
1:1:1
2:1:2
3:1:6
4:1:24
5:1:120
6:1:720
7:1:5040
8:1:40320
9:1:362880
10:1:3628800

NaN:2:NaN
+inf:2:inf
-inf:2:NaN
-2:2:NaN
-1:2:1
0:2:1
1:2:1
2:2:2
3:2:3
4:2:8
5:2:15
6:2:48
7:2:105
8:2:384
9:2:945
10:2:3840

NaN:3:NaN
+inf:3:inf
-inf:3:NaN
-3:3:NaN
-2:3:1
-1:3:1
0:3:1
1:3:1
2:3:2
3:3:3
4:3:4
5:3:10
6:3:18
7:3:28
8:3:80
9:3:162
10:3:280

NaN:4:NaN
+inf:4:inf
-inf:4:NaN
-4:4:NaN
-3:4:1
-2:4:1
-1:4:1
0:4:1
1:4:1
2:4:2
3:4:3
4:4:4
5:4:5
6:4:12
7:4:21
8:4:32
9:4:45
10:4:120

NaN:5:NaN
+inf:5:inf
-inf:5:NaN
-5:5:NaN
-4:5:1
-3:5:1
-2:5:1
-1:5:1
0:5:1
1:5:1
2:5:2
3:5:3
4:5:4
5:5:5
6:5:6
7:5:14
8:5:24
9:5:36
10:5:50

&bpow
#
abc:12:NaN
12:abc:NaN
#
#
-inf:-inf:0
-inf:-3:0
-inf:-2:0
-inf:-1:0
-inf:0:NaN
-inf:1:-inf
-inf:2:inf
-inf:3:-inf
-inf:inf:inf    # complex infinity
-inf:NaN:NaN
#
-3:-inf:0
-3:-3:0
-3:-2:0
-3:-1:0
-3:0:1
-3:1:-3
-3:2:9
-3:3:-27
-3:inf:inf      # complex infinity
-3:NaN:NaN
#
-2:-inf:0
-2:-3:0
-2:-2:0
-2:-1:0
-2:0:1
-2:1:-2
-2:2:4
-2:3:-8
-2:inf:inf      # complex infinity
-2:NaN:NaN
#
-1:-inf:NaN
-1:-3:-1
-1:-2:1
-1:-1:-1
-1:0:1
-1:1:-1
-1:2:1
-1:3:-1
-1:inf:NaN
-1:NaN:NaN
#
0:-inf:inf      # complex infinity
0:-3:inf        # complex infinity
0:-2:inf        # complex infinity
0:-1:inf        # complex infinity
0:0:1
0:1:0
0:2:0
0:3:0
0:inf:0
0:NaN:NaN
#
1:-inf:1
1:-3:1
1:-2:1
1:-1:1
1:0:1
1:1:1
1:2:1
1:3:1
1:inf:1
1:NaN:NaN
#
2:-inf:0
2:-3:0
2:-2:0
2:-1:0
2:0:1
2:1:2
2:2:4
2:3:8
2:inf:inf
2:NaN:NaN
#
3:-inf:0
3:-3:0
3:-2:0
3:-1:0
3:0:1
3:1:3
3:2:9
3:3:27
3:inf:inf
3:NaN:NaN
#
inf:-inf:0
inf:-3:0
inf:-2:0
inf:-1:0
inf:0:NaN
inf:1:inf
inf:2:inf
inf:3:inf
inf:inf:inf
inf:NaN:NaN
#
NaN:-inf:NaN
NaN:-3:NaN
NaN:-2:NaN
NaN:-1:NaN
NaN:0:NaN
NaN:1:NaN
NaN:2:NaN
NaN:3:NaN
NaN:inf:NaN
NaN:NaN:NaN
#
+inf:1234500012:inf
-inf:1234500012:inf
-inf:1234500013:-inf
+inf:-12345000123:0
-inf:-12345000123:0
#
10:2:100
10:3:1000
10:4:10000
10:5:100000
10:6:1000000
10:7:10000000
10:8:100000000
10:9:1000000000
10:20:100000000000000000000
123456:2:15241383936
-2:4:16
-2:5:-32
-3:3:-27
-3:4:81
-3:5:-243

&length
100:3
10:2
1:1
0:1
12345:5
10000000000000000:17
-123:3
215960156869840440586892398248:30

&broot
# sqrt()
+0:2:0
+1:2:1
-1:2:NaN
# -$x ** (1/2) => -$y, but not in broot()
-123:2:NaN
+inf:2:inf
-inf:2:NaN
2:2:1
-2:2:NaN
4:2:2
9:2:3
16:2:4
100:2:10
123:2:11
15241:2:123
144:2:12
12:2:3
# invalid ones
1:NaN:NaN
-1:NaN:NaN
0:NaN:NaN
-inf:NaN:NaN
+inf:NaN:NaN
NaN:0:NaN
NaN:2:NaN
NaN:inf:NaN
NaN:inf:NaN
12:-inf:NaN
12:inf:NaN
+0:0:NaN
+1:0:NaN
-1:0:NaN
-2:0:NaN
-123.45:0:NaN
+inf:0:NaN
12:1:12
-12:1:NaN
8:-1:NaN
-8:-1:NaN
# cubic root
8:3:2
-8:3:NaN
# fourths root
16:4:2
81:4:3
# 2 ** 64
18446744073709551616:4:65536
18446744073709551616:8:256
18446744073709551616:16:16
18446744073709551616:32:4
18446744073709551616:64:2
18446744073709551616:128:1
# 213 ** 15
84274086103068221283760416414557757:15:213

# see t/bigroot.t for more tests
&bsqrt
145:12
144:12
143:11
16:4
170:13
169:13
168:12
4:2
3:1
2:1
9:3
12:3
256:16
100000000:10000
4000000000000:2000000
152399026:12345
152399025:12345
152399024:12344
# 2 ** 64 => 2 ** 32
18446744073709551616:4294967296
84274086103068221283760416414557757:290299993288095377
1:1
0:0
-2:NaN
-123:NaN
Nan:NaN
+inf:inf
-inf:NaN

# see t/biglog.t for more tests
&bexp
NaN:NaN
inf:inf
1:2
2:7

&batan2
NaN:1:10:NaN
NaN:NaN:10:NaN
1:NaN:10:NaN
inf:1:14:1
-inf:1:14:-1
0:-inf:14:3
-1:-inf:14:-3
1:-inf:14:3
0:inf:14:0
inf:-inf:14:2
-inf:-inf:14:-2
# +- 0.78....
inf:+inf:14:0
-inf:+inf:14:0
1:5:13:0
1:5:14:0
0:0:10:0
0:1:14:0
0:2:14:0
1:0:14:1
5:0:14:1
-1:0:11:-1
-2:0:77:-1
2:0:77:1
-1:5:14:0
1:5:14:0
-1:8:14:0
1:8:14:0
-1:1:14:0

&bpi
77:3
+0:3
11:3

# see t/bignok.t for more tests
&bnok
+inf:10:inf
NaN:NaN:NaN
NaN:1:NaN
1:NaN:NaN
1:1:1
# k > n
1:2:0
2:3:0
# k < 0
1:-2:0
# 7 over 3 = 35
7:3:35
7:6:7
100:90:17310309456440
100:95:75287520
2:0:1
7:0:1
2:1:2

&bround
$round_mode("trunc")
0:12:0
invalid:12:NaN
+inf:12:inf
-inf:12:-inf
1234:0:1234
1234:2:1200
123456:4:123400
123456:5:123450
123456:6:123456
+10123456789:5:10123000000
-10123456789:5:-10123000000
+10123456789:9:10123456700
-10123456789:9:-10123456700
+101234500:6:101234000
-101234500:6:-101234000
#+101234500:-4:101234000
#-101234500:-4:-101234000
$round_mode("zero")
+20123456789:5:20123000000
-20123456789:5:-20123000000
+20123456789:9:20123456800
-20123456789:9:-20123456800
+201234500:6:201234000
-201234500:6:-201234000
#+201234500:-4:201234000
#-201234500:-4:-201234000
+12345000:4:12340000
-12345000:4:-12340000
$round_mode("+inf")
+30123456789:5:30123000000
-30123456789:5:-30123000000
+30123456789:9:30123456800
-30123456789:9:-30123456800
+301234500:6:301235000
-301234500:6:-301234000
#+301234500:-4:301235000
#-301234500:-4:-301234000
+12345000:4:12350000
-12345000:4:-12340000
$round_mode("-inf")
+40123456789:5:40123000000
-40123456789:5:-40123000000
+40123456789:9:40123456800
-40123456789:9:-40123456800
+401234500:6:401234000
+401234500:6:401234000
#-401234500:-4:-401235000
#-401234500:-4:-401235000
+12345000:4:12340000
-12345000:4:-12350000
$round_mode("odd")
+50123456789:5:50123000000
-50123456789:5:-50123000000
+50123456789:9:50123456800
-50123456789:9:-50123456800
+501234500:6:501235000
-501234500:6:-501235000
#+501234500:-4:501235000
#-501234500:-4:-501235000
+12345000:4:12350000
-12345000:4:-12350000
$round_mode("common")
+60123456789:5:60123000000
+60123199999:5:60123000000
+60123299999:5:60123000000
+60123399999:5:60123000000
+60123499999:5:60123000000
+60123500000:5:60124000000
+60123600000:5:60124000000
+60123700000:5:60124000000
+60123800000:5:60124000000
+60123900000:5:60124000000
-60123456789:5:-60123000000
-60123199999:5:-60123000000
-60123299999:5:-60123000000
-60123399999:5:-60123000000
-60123499999:5:-60123000000
-60123500000:5:-60124000000
-60123600000:5:-60124000000
-60123700000:5:-60124000000
-60123800000:5:-60124000000
-60123900000:5:-60124000000
$round_mode("even")
+60123456789:5:60123000000
-60123456789:5:-60123000000
+60123456789:9:60123456800
-60123456789:9:-60123456800
+601234500:6:601234000
-601234500:6:-601234000
#+601234500:-4:601234000
#-601234500:-4:-601234000
#-601234500:-9:0
#-501234500:-9:0
#-601234500:-8:0
#-501234500:-8:0
+1234567:7:1234567
+1234567:6:1234570
+12345000:4:12340000
-12345000:4:-12340000

&is_zero
0:1
invalid:0
+inf:0
-inf:0
123:0
-1:0
1:0

&is_one
0:0
invalid:0
+inf:0
-inf:0
1:1
2:0
-1:0
-2:0

# floor, ceil, and int are pretty pointless in integer space, but play safe
&bfloor
0:0
invalid:NaN
+inf:inf
-inf:-inf
-1:-1
-2:-2
2:2
3:3
abc:NaN

&bceil
invalid:NaN
+inf:inf
-inf:-inf
0:0
-1:-1
-2:-2
2:2
3:3
abc:NaN

&bint
NaN:NaN
+inf:inf
-inf:-inf
0:0
-1:-1
-2:-2
2:2
3:3

&as_hex
128:0x80
-128:-0x80
0:0x0
-0:0x0
1:0x1
0x123456789123456789:0x123456789123456789
+inf:inf
-inf:-inf
invalid:NaN

&as_bin
128:0b10000000
-128:-0b10000000
0:0b0
-0:0b0
1:0b1
0b1010111101010101010110110110110110101:0b1010111101010101010110110110110110101
0x123456789123456789:0b100100011010001010110011110001001000100100011010001010110011110001001
+inf:inf
-inf:-inf
invalid:NaN

&as_oct
128:0200
-128:-0200
0:00
-0:00
1:01
0b1010111101010101010110110110110110101:01275252666665
0x123456789123456789:044321263611044321263611
+inf:inf
-inf:-inf
invalid:NaN

&to_hex
128:80
-128:-80
0:0
-0:0
1:1
0x123456789123456789:123456789123456789
+inf:inf
-inf:-inf
invalid:NaN

&to_bin
128:10000000
-128:-10000000
0:0
-0:0
1:1
0b1010111101010101010110110110110110101:1010111101010101010110110110110110101
0x123456789123456789:100100011010001010110011110001001000100100011010001010110011110001001
+inf:inf
-inf:-inf
invalid:NaN

&to_oct
128:200
-128:-200
0:0
-0:0
1:1
0b1010111101010101010110110110110110101:1275252666665
0x123456789123456789:44321263611044321263611
+inf:inf
-inf:-inf
invalid:NaN

# overloaded functions
&log
-1:NaN
0:-inf
1:0
2:0
3:1
123456789:18
1234567890987654321:41
-inf:inf
inf:inf
NaN:NaN

&exp

&sin

&cos

&atan2

&int

&neg

&abs

&sqrt
