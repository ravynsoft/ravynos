#!./perl

#P = start of string  Q = start of substr  R = end of substr  S = end of string

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}
use warnings ;

$a = 'abcdefxyz';
$SIG{__WARN__} = sub {
     if ($_[0] =~ /^substr outside of string/) {
          $w++;
     } elsif ($_[0] =~ /^Attempt to use reference as lvalue in substr/) {
          $w += 2;
     } elsif ($_[0] =~ /^Use of uninitialized value/) {
          $w += 3;
     } else {
          warn $_[0];
     }
};

plan(400);

run_tests() unless caller;

my $krunch = "a";

sub run_tests {

$FATAL_MSG = qr/^substr outside of string/;

is(substr($a,0,3), 'abc');   # P=Q R S
is(substr($a,3,3), 'def');   # P Q R S
is(substr($a,6,999), 'xyz'); # P Q S R
$b = substr($a,999,999) ; # warn # P R Q S
is ($w--, 1);
eval{substr($a,999,999) = "" ; };# P R Q S
like ($@, $FATAL_MSG);
is(substr($a,0,-6), 'abc');  # P=Q R S
is(substr($a,-3,1), 'x');    # P Q R S
sub{$b = shift}->(substr($a,999,999));
is ($w--, 1, 'boundless lvalue substr only warns on fetch');

substr($a,3,3) = 'XYZ';
is($a, 'abcXYZxyz' );
substr($a,0,2) = '';
is($a, 'cXYZxyz' );
substr($a,0,0) = 'ab';
is($a, 'abcXYZxyz' );
substr($a,0,0) = '12345678';
is($a, '12345678abcXYZxyz' );
substr($a,-3,3) = 'def';
is($a, '12345678abcXYZdef');
substr($a,-3,3) = '<';
is($a, '12345678abcXYZ<' );
substr($a,-1,1) = '12345678';
is($a, '12345678abcXYZ12345678' );

$a = 'abcdefxyz';

is(substr($a,6), 'xyz' );        # P Q R=S
is(substr($a,-3), 'xyz' );       # P Q R=S
$b = substr($a,999,999) ; # warning   # P R=S Q
is($w--, 1);
eval{substr($a,999,999) = "" ; } ;    # P R=S Q
like($@, $FATAL_MSG);
is(substr($a,0), 'abcdefxyz');  # P=Q R=S
is(substr($a,9), '');           # P Q=R=S
is(substr($a,-11), 'abcdefxyz'); # Q P R=S
is(substr($a,-9), 'abcdefxyz');  # P=Q R=S

$a = '54321';

$b = substr($a,-7, 1) ; # warn  # Q R P S
is($w--, 1);
eval{substr($a,-7, 1) = "" ; }; # Q R P S
like($@, $FATAL_MSG);
$b = substr($a,-7,-6) ; # warn  # Q R P S
is($w--, 1);
eval{substr($a,-7,-6) = "" ; }; # Q R P S
like($@, $FATAL_MSG);
is(substr($a,-5,-7), '');  # R P=Q S
is(substr($a, 2,-7), '');  # R P Q S
is(substr($a,-3,-7), '');  # R P Q S
is(substr($a, 2,-5), '');  # P=R Q S
is(substr($a,-3,-5), '');  # P=R Q S
is(substr($a, 2,-4), '');  # P R Q S
is(substr($a,-3,-4), '');  # P R Q S
is(substr($a, 5,-6), '');  # R P Q=S
is(substr($a, 5,-5), '');  # P=R Q S
is(substr($a, 5,-3), '');  # P R Q=S
$b = substr($a, 7,-7) ; # warn  # R P S Q
is($w--, 1);
eval{substr($a, 7,-7) = "" ; }; # R P S Q
like($@, $FATAL_MSG);
$b = substr($a, 7,-5) ; # warn  # P=R S Q
is($w--, 1);
eval{substr($a, 7,-5) = "" ; }; # P=R S Q
like($@, $FATAL_MSG);
$b = substr($a, 7,-3) ; # warn  # P Q S Q
is($w--, 1);
eval{substr($a, 7,-3) = "" ; }; # P Q S Q
like($@, $FATAL_MSG);
$b = substr($a, 7, 0) ; # warn  # P S Q=R
is($w--, 1);
eval{substr($a, 7, 0) = "" ; }; # P S Q=R
like($@, $FATAL_MSG);

is(substr($a,-7,2), '');   # Q P=R S
is(substr($a,-7,4), '54'); # Q P R S
is(substr($a,-7,7), '54321');# Q P R=S
is(substr($a,-7,9), '54321');# Q P S R
is(substr($a,-5,0), '');   # P=Q=R S
is(substr($a,-5,3), '543');# P=Q R S
is(substr($a,-5,5), '54321');# P=Q R=S
is(substr($a,-5,7), '54321');# P=Q S R
is(substr($a,-3,0), '');   # P Q=R S
is(substr($a,-3,3), '321');# P Q R=S
is(substr($a,-2,3), '21'); # P Q S R
is(substr($a,0,-5), '');   # P=Q=R S
is(substr($a,2,-3), '');   # P Q=R S
is(substr($a,0,0), '');    # P=Q=R S
is(substr($a,0,5), '54321');# P=Q R=S
is(substr($a,0,7), '54321');# P=Q S R
is(substr($a,2,0), '');    # P Q=R S
is(substr($a,2,3), '321'); # P Q R=S
is(substr($a,5,0), '');    # P Q=R=S
is(substr($a,5,2), '');    # P Q=S R
is(substr($a,-7,-5), '');  # Q P=R S
is(substr($a,-7,-2), '543');# Q P R S
is(substr($a,-5,-5), '');  # P=Q=R S
is(substr($a,-5,-2), '543');# P=Q R S
is(substr($a,-3,-3), '');  # P Q=R S
is(substr($a,-3,-1), '32');# P Q R S

$a = '';

is(substr($a,-2,2), '');   # Q P=R=S
is(substr($a,0,0), '');    # P=Q=R=S
is(substr($a,0,1), '');    # P=Q=S R
is(substr($a,-2,3), '');   # Q P=S R
is(substr($a,-2), '');     # Q P=R=S
is(substr($a,0), '');      # P=Q=R=S


is(substr($a,0,-1), '');   # R P=Q=S
$b = substr($a,-2, 0) ; # warn  # Q=R P=S
is($w--, 1);
eval{substr($a,-2, 0) = "" ; }; # Q=R P=S
like($@, $FATAL_MSG);

$b = substr($a,-2, 1) ; # warn  # Q R P=S
is($w--, 1);
eval{substr($a,-2, 1) = "" ; }; # Q R P=S
like($@, $FATAL_MSG);

$b = substr($a,-2,-1) ; # warn  # Q R P=S
is($w--, 1);
eval{substr($a,-2,-1) = "" ; }; # Q R P=S
like($@, $FATAL_MSG);

$b = substr($a,-2,-2) ; # warn  # Q=R P=S
is($w--, 1);
eval{substr($a,-2,-2) = "" ; }; # Q=R P=S
like($@, $FATAL_MSG);

$b = substr($a, 1,-2) ; # warn  # R P=S Q
is($w--, 1);
eval{substr($a, 1,-2) = "" ; }; # R P=S Q
like($@, $FATAL_MSG);

$b = substr($a, 1, 1) ; # warn  # P=S Q R
is($w--, 1);
eval{substr($a, 1, 1) = "" ; }; # P=S Q R
like($@, $FATAL_MSG);

$b = substr($a, 1, 0) ;# warn   # P=S Q=R
is($w--, 1);
eval{substr($a, 1, 0) = "" ; }; # P=S Q=R
like($@, $FATAL_MSG);

$b = substr($a,1) ; # warning   # P=R=S Q
is($w--, 1);
eval{substr($a,1) = "" ; };     # P=R=S Q
like($@, $FATAL_MSG);

$b = substr($a,-7,-6) ; # warn  # Q R P S
is($w--, 1);
eval{substr($a,-7,-6) = "" ; }; # Q R P S
like($@, $FATAL_MSG);

my $a = 'zxcvbnm';
substr($a,2,0) = '';
is($a, 'zxcvbnm');
substr($a,7,0) = '';
is($a, 'zxcvbnm');
substr($a,5,0) = '';
is($a, 'zxcvbnm');
substr($a,0,2) = 'pq';
is($a, 'pqcvbnm');
substr($a,2,0) = 'r';
is($a, 'pqrcvbnm');
substr($a,8,0) = 'asd';
is($a, 'pqrcvbnmasd');
substr($a,0,2) = 'iop';
is($a, 'ioprcvbnmasd');
substr($a,0,5) = 'fgh';
is($a, 'fghvbnmasd');
substr($a,3,5) = 'jkl';
is($a, 'fghjklsd');
substr($a,3,2) = '1234';
is($a, 'fgh1234lsd');


# with lexicals (and in re-entered scopes)
for (0,1) {
  my $txt;
  unless ($_) {
    $txt = "Foo";
    substr($txt, -1) = "X";
    is($txt, "FoX");
  }
  else {
    substr($txt, 0, 1) = "X";
    is($txt, "X");
  }
}

$w = 0 ;
# coercion of references
{
  my $s = [];
  substr($s, 0, 1) = 'Foo';
  is (substr($s,0,7), "FooRRAY");
  is ($w,2);
  $w = 0;
}

# check no spurious warnings
is($w, 0);

# check new 4 arg replacement syntax
$a = "abcxyz";
$w = 0;
is(substr($a, 0, 3, ""), "abc");
is($a, "xyz");
is(substr($a, 0, 0, "abc"), "");
is($a, "abcxyz");
is(substr($a, 3, -1, ""), "xy");
is($a, "abcz");

is(substr($a, 3, undef, "xy"), "");
is($a, "abcxyz");
is($w, 3);

$w = 0;

is(substr($a, 3, 9999999, ""), "xyz");
is($a, "abc");
eval{substr($a, -99, 0, "") };
like($@, $FATAL_MSG);
eval{substr($a, 99, 3, "") };
like($@, $FATAL_MSG);

substr($a, 0, length($a), "foo");
is ($a, "foo");
is ($w, 0);

# using 4 arg substr as lvalue is a compile time error
eval 'substr($a,0,0,"") = "abc"';
like ($@, qr/Can't modify substr/);
is ($a, "foo");

$a = "abcdefgh";
is(sub { shift }->(substr($a, 0, 4, "xxxx")), 'abcd');
is($a, 'xxxxefgh');

{
    my $y = 10;
    $y = "2" . $y;
    is ($y, 210);
}

# utf8 sanity
{
    my $x = substr("a\x{263a}b",0);
    is(length($x), 3);
    $x = substr($x,1,1);
    is($x, "\x{263a}");
    $x = $x x 2;
    is(length($x), 2);
    substr($x,0,1) = "abcd";
    is($x, "abcd\x{263a}");
    is(length($x), 5);
    $x = reverse $x;
    is(length($x), 5);
    is($x, "\x{263a}dcba");

    my $z = 10;
    $z = "21\x{263a}" . $z;
    is(length($z), 5);
    is($z, "21\x{263a}10");
}

# replacement should work on magical values
require Tie::Scalar;
my %data;
tie $data{'a'}, 'Tie::StdScalar';  # makes $data{'a'} magical
$data{a} = "firstlast";
is(substr($data{'a'}, 0, 5, ""), "first");
is($data{'a'}, "last");

# more utf8

# The following two originally from Ignasi Roca.

$x = "\xF1\xF2\xF3";
substr($x, 0, 1) = "\x{100}"; # Ignasi had \x{FF}
is(length($x), 3);
is($x, "\x{100}\xF2\xF3");
is(substr($x, 0, 1), "\x{100}");
is(substr($x, 1, 1), "\x{F2}");
is(substr($x, 2, 1), "\x{F3}");

$x = "\xF1\xF2\xF3";
substr($x, 0, 1) = "\x{100}\x{FF}"; # Ignasi had \x{FF}
is(length($x), 4);
is($x, "\x{100}\x{FF}\xF2\xF3");
is(substr($x, 0, 1), "\x{100}");
is(substr($x, 1, 1), "\x{FF}");
is(substr($x, 2, 1), "\x{F2}");
is(substr($x, 3, 1), "\x{F3}");

# more utf8 lval exercise

$x = "\xF1\xF2\xF3";
substr($x, 0, 2) = "\x{100}\xFF";
is(length($x), 3);
is($x, "\x{100}\xFF\xF3");
is(substr($x, 0, 1), "\x{100}");
is(substr($x, 1, 1), "\x{FF}");
is(substr($x, 2, 1), "\x{F3}");

$x = "\xF1\xF2\xF3";
substr($x, 1, 1) = "\x{100}\xFF";
is(length($x), 4);
is($x, "\xF1\x{100}\xFF\xF3");
is(substr($x, 0, 1), "\x{F1}");
is(substr($x, 1, 1), "\x{100}");
is(substr($x, 2, 1), "\x{FF}");
is(substr($x, 3, 1), "\x{F3}");

$x = "\xF1\xF2\xF3";
substr($x, 2, 1) = "\x{100}\xFF";
is(length($x), 4);
is($x, "\xF1\xF2\x{100}\xFF");
is(substr($x, 0, 1), "\x{F1}");
is(substr($x, 1, 1), "\x{F2}");
is(substr($x, 2, 1), "\x{100}");
is(substr($x, 3, 1), "\x{FF}");

$x = "\xF1\xF2\xF3";
substr($x, 3, 1) = "\x{100}\xFF";
is(length($x), 5);
is($x, "\xF1\xF2\xF3\x{100}\xFF");
is(substr($x, 0, 1), "\x{F1}");
is(substr($x, 1, 1), "\x{F2}");
is(substr($x, 2, 1), "\x{F3}");
is(substr($x, 3, 1), "\x{100}");
is(substr($x, 4, 1), "\x{FF}");

$x = "\xF1\xF2\xF3";
substr($x, -1, 1) = "\x{100}\xFF";
is(length($x), 4);
is($x, "\xF1\xF2\x{100}\xFF");
is(substr($x, 0, 1), "\x{F1}");
is(substr($x, 1, 1), "\x{F2}");
is(substr($x, 2, 1), "\x{100}");
is(substr($x, 3, 1), "\x{FF}");

$x = "\xF1\xF2\xF3";
substr($x, -1, 0) = "\x{100}\xFF";
is(length($x), 5);
is($x, "\xF1\xF2\x{100}\xFF\xF3");
is(substr($x, 0, 1), "\x{F1}");
is(substr($x, 1, 1), "\x{F2}");
is(substr($x, 2, 1), "\x{100}");
is(substr($x, 3, 1), "\x{FF}");
is(substr($x, 4, 1), "\x{F3}");

$x = "\xF1\xF2\xF3";
substr($x, 0, -1) = "\x{100}\xFF";
is(length($x), 3);
is($x, "\x{100}\xFF\xF3");
is(substr($x, 0, 1), "\x{100}");
is(substr($x, 1, 1), "\x{FF}");
is(substr($x, 2, 1), "\x{F3}");

$x = "\xF1\xF2\xF3";
substr($x, 0, -2) = "\x{100}\xFF";
is(length($x), 4);
is($x, "\x{100}\xFF\xF2\xF3");
is(substr($x, 0, 1), "\x{100}");
is(substr($x, 1, 1), "\x{FF}");
is(substr($x, 2, 1), "\x{F2}");
is(substr($x, 3, 1), "\x{F3}");

$x = "\xF1\xF2\xF3";
substr($x, 0, -3) = "\x{100}\xFF";
is(length($x), 5);
is($x, "\x{100}\xFF\xF1\xF2\xF3");
is(substr($x, 0, 1), "\x{100}");
is(substr($x, 1, 1), "\x{FF}");
is(substr($x, 2, 1), "\x{F1}");
is(substr($x, 3, 1), "\x{F2}");
is(substr($x, 4, 1), "\x{F3}");

$x = "\xF1\xF2\xF3";
substr($x, 1, -1) = "\x{100}\xFF";
is(length($x), 4);
is($x, "\xF1\x{100}\xFF\xF3");
is(substr($x, 0, 1), "\x{F1}");
is(substr($x, 1, 1), "\x{100}");
is(substr($x, 2, 1), "\x{FF}");
is(substr($x, 3, 1), "\x{F3}");

$x = "\xF1\xF2\xF3";
substr($x, -1, -1) = "\x{100}\xFF";
is(length($x), 5);
is($x, "\xF1\xF2\x{100}\xFF\xF3");
is(substr($x, 0, 1), "\x{F1}");
is(substr($x, 1, 1), "\x{F2}");
is(substr($x, 2, 1), "\x{100}");
is(substr($x, 3, 1), "\x{FF}");
is(substr($x, 4, 1), "\x{F3}");

# And tests for already-UTF8 one

$x = "\x{101}\x{F2}\x{F3}";
substr($x, 0, 1) = "\x{100}";
is(length($x), 3);
is($x, "\x{100}\xF2\xF3");
is(substr($x, 0, 1), "\x{100}");
is(substr($x, 1, 1), "\x{F2}");
is(substr($x, 2, 1), "\x{F3}");

$x = "\x{101}\x{F2}\x{F3}";
substr($x, 0, 1) = "\x{100}\x{FF}";
is(length($x), 4);
is($x, "\x{100}\x{FF}\xF2\xF3");
is(substr($x, 0, 1), "\x{100}");
is(substr($x, 1, 1), "\x{FF}");
is(substr($x, 2, 1), "\x{F2}");
is(substr($x, 3, 1), "\x{F3}");

$x = "\x{101}\x{F2}\x{F3}";
substr($x, 0, 2) = "\x{100}\xFF";
is(length($x), 3);
is($x, "\x{100}\xFF\xF3");
is(substr($x, 0, 1), "\x{100}");
is(substr($x, 1, 1), "\x{FF}");
is(substr($x, 2, 1), "\x{F3}");

$x = "\x{101}\x{F2}\x{F3}";
substr($x, 1, 1) = "\x{100}\xFF";
is(length($x), 4);
is($x, "\x{101}\x{100}\xFF\xF3");
is(substr($x, 0, 1), "\x{101}");
is(substr($x, 1, 1), "\x{100}");
is(substr($x, 2, 1), "\x{FF}");
is(substr($x, 3, 1), "\x{F3}");

$x = "\x{101}\x{F2}\x{F3}";
substr($x, 2, 1) = "\x{100}\xFF";
is(length($x), 4);
is($x, "\x{101}\xF2\x{100}\xFF");
is(substr($x, 0, 1), "\x{101}");
is(substr($x, 1, 1), "\x{F2}");
is(substr($x, 2, 1), "\x{100}");
is(substr($x, 3, 1), "\x{FF}");

$x = "\x{101}\x{F2}\x{F3}";
substr($x, 3, 1) = "\x{100}\xFF";
is(length($x), 5);
is($x, "\x{101}\x{F2}\x{F3}\x{100}\xFF");
is(substr($x, 0, 1), "\x{101}");
is(substr($x, 1, 1), "\x{F2}");
is(substr($x, 2, 1), "\x{F3}");
is(substr($x, 3, 1), "\x{100}");
is(substr($x, 4, 1), "\x{FF}");

$x = "\x{101}\x{F2}\x{F3}";
substr($x, -1, 1) = "\x{100}\xFF";
is(length($x), 4);
is($x, "\x{101}\xF2\x{100}\xFF");
is(substr($x, 0, 1), "\x{101}");
is(substr($x, 1, 1), "\x{F2}");
is(substr($x, 2, 1), "\x{100}");
is(substr($x, 3, 1), "\x{FF}");

$x = "\x{101}\x{F2}\x{F3}";
substr($x, -1, 0) = "\x{100}\xFF";
is(length($x), 5);
is($x, "\x{101}\xF2\x{100}\xFF\xF3");
is(substr($x, 0, 1), "\x{101}");
is(substr($x, 1, 1), "\x{F2}");
is(substr($x, 2, 1), "\x{100}");
is(substr($x, 3, 1), "\x{FF}");
is(substr($x, 4, 1), "\x{F3}");

$x = "\x{101}\x{F2}\x{F3}";
substr($x, 0, -1) = "\x{100}\xFF";
is(length($x), 3);
is($x, "\x{100}\xFF\xF3");
is(substr($x, 0, 1), "\x{100}");
is(substr($x, 1, 1), "\x{FF}");
is(substr($x, 2, 1), "\x{F3}");

$x = "\x{101}\x{F2}\x{F3}";
substr($x, 0, -2) = "\x{100}\xFF";
is(length($x), 4);
is($x, "\x{100}\xFF\xF2\xF3");
is(substr($x, 0, 1), "\x{100}");
is(substr($x, 1, 1), "\x{FF}");
is(substr($x, 2, 1), "\x{F2}");
is(substr($x, 3, 1), "\x{F3}");

$x = "\x{101}\x{F2}\x{F3}";
substr($x, 0, -3) = "\x{100}\xFF";
is(length($x), 5);
is($x, "\x{100}\xFF\x{101}\x{F2}\x{F3}");
is(substr($x, 0, 1), "\x{100}");
is(substr($x, 1, 1), "\x{FF}");
is(substr($x, 2, 1), "\x{101}");
is(substr($x, 3, 1), "\x{F2}");
is(substr($x, 4, 1), "\x{F3}");

$x = "\x{101}\x{F2}\x{F3}";
substr($x, 1, -1) = "\x{100}\xFF";
is(length($x), 4);
is($x, "\x{101}\x{100}\xFF\xF3");
is(substr($x, 0, 1), "\x{101}");
is(substr($x, 1, 1), "\x{100}");
is(substr($x, 2, 1), "\x{FF}");
is(substr($x, 3, 1), "\x{F3}");

$x = "\x{101}\x{F2}\x{F3}";
substr($x, -1, -1) = "\x{100}\xFF";
is(length($x), 5);
is($x, "\x{101}\xF2\x{100}\xFF\xF3");
is(substr($x, 0, 1), "\x{101}");
is(substr($x, 1, 1), "\x{F2}");
is(substr($x, 2, 1), "\x{100}");
is(substr($x, 3, 1), "\x{FF}");
is(substr($x, 4, 1), "\x{F3}");

substr($x = "ab", 0, 0, "\x{100}\x{200}");
is($x, "\x{100}\x{200}ab");

substr($x = "\x{100}\x{200}", 0, 0, "ab");
is($x, "ab\x{100}\x{200}");

substr($x = "ab", 1, 0, "\x{100}\x{200}");
is($x, "a\x{100}\x{200}b");

substr($x = "\x{100}\x{200}", 1, 0, "ab");
is($x, "\x{100}ab\x{200}");

substr($x = "ab", 2, 0, "\x{100}\x{200}");
is($x, "ab\x{100}\x{200}");

substr($x = "\x{100}\x{200}", 2, 0, "ab");
is($x, "\x{100}\x{200}ab");

substr($x = "\xFFb", 0, 0, "\x{100}\x{200}");
is($x, "\x{100}\x{200}\xFFb");

substr($x = "\x{100}\x{200}", 0, 0, "\xFFb");
is($x, "\xFFb\x{100}\x{200}");

substr($x = "\xFFb", 1, 0, "\x{100}\x{200}");
is($x, "\xFF\x{100}\x{200}b");

substr($x = "\x{100}\x{200}", 1, 0, "\xFFb");
is($x, "\x{100}\xFFb\x{200}");

substr($x = "\xFFb", 2, 0, "\x{100}\x{200}");
is($x, "\xFFb\x{100}\x{200}");

substr($x = "\x{100}\x{200}", 2, 0, "\xFFb");
is($x, "\x{100}\x{200}\xFFb");

# [perl #20933]
{ 
    my $s = "ab";
    my @r; 
    $r[$_] = \ substr $s, $_, 1 for (0, 1);
    is(join("", map { $$_ } @r), "ab");
}

# [perl #23207]
{
    sub ss {
	substr($_[0],0,1) ^= substr($_[0],1,1) ^=
	substr($_[0],0,1) ^= substr($_[0],1,1);
    }
    my $x = my $y = 'AB'; ss $x; ss $y;
    is($x, $y);
}

# [perl #24605]
{
    my $x = "0123456789\x{500}";
    my $y = substr $x, 4;
    is(substr($x, 7, 1), "7");
}

# multiple assignments to lvalue [perl #24346]   
{
    my $x = "abcdef";
    for (substr($x,1,3)) {
	is($_, 'bcd');
	$_ = 'XX';
	is($_, 'XX');
	is($x, 'aXXef'); 
	$_ = "\xFF";
	is($_, "\xFF"); 
	is($x, "a\xFFef");
	$_ = "\xF1\xF2\xF3\xF4\xF5\xF6";
	is($_, "\xF1\xF2\xF3\xF4\xF5\xF6");
	is($x, "a\xF1\xF2\xF3\xF4\xF5\xF6ef"); 
	$_ = 'YYYY';
	is($_, 'YYYY'); 
	is($x, 'aYYYYef');
    }
    $x = "abcdef";
    for (substr($x,1)) {
	is($_, 'bcdef');
	$_ = 'XX';
	is($_, 'XX');
	is($x, 'aXX');
	$x .= "frompswiggle";
	is $_, "XXfrompswiggle";
    }
    $x = "abcdef";
    for (substr($x,1,-1)) {
	is($_, 'bcde');
	$_ = 'XX';
	is($_, 'XX');
	is($x, 'aXXf');
	$x .= "frompswiggle";
	is $_, "XXffrompswiggl";
    }
    $x = "abcdef";
    for (substr($x,-5,3)) {
	is($_, 'bcd');
	$_ = 'XX';   # now $_ is substr($x, -4, 2)
	is($_, 'XX');
	is($x, 'aXXef');
	$x .= "frompswiggle";
	is $_, "gg";
    }
    $x = "abcdef";
    for (substr($x,-5)) {
	is($_, 'bcdef');
	$_ = 'XX';  # now substr($x, -2)
	is($_, 'XX');
	is($x, 'aXX');
	$x .= "frompswiggle";
	is $_, "le";
    }
    $x = "abcdef";
    for (substr($x,-5,-1)) {
	is($_, 'bcde');
	$_ = 'XX';  # now substr($x, -3, -1)
	is($_, 'XX');
	is($x, 'aXXf');
	$x .= "frompswiggle";
	is $_, "gl";
    }
}

# Also part of perl #24346; scalar(substr...) should not affect lvalueness
{
    my $str = "abcdef";
    sub { $_[0] = 'dea' }->( scalar substr $str, 3, 2 );
    is $str, 'abcdeaf', 'scalar does not affect lvalueness of substr';
}

# [perl #24200] string corruption with lvalue sub

{
    sub bar: lvalue { substr $krunch, 0 }
    bar = "XXX";
    is(bar, 'XXX');
    $krunch = '123456789';
    is(bar, '123456789');
}

# [perl #29149]
{
    my $text  = "0123456789\xED ";
    utf8::upgrade($text);
    my $pos = 5;
    pos($text) = $pos;
    my $a = substr($text, $pos, $pos);
    is(substr($text,$pos,1), $pos);

}

# [perl #34976] incorrect caching of utf8 substr length
{
    my  $a = "abcd\x{100}";
    is(substr($a,1,2), 'bc');
    is(substr($a,1,1), 'b');
}

# [perl #62646] offsets exceeding 32 bits on 64-bit system
SKIP: {
    skip("32-bit system", 24) unless ~0 > 0xffffffff;
    my $a = "abc";
    my $s;
    my $r;

    utf8::downgrade($a);
    for (1..2) {
	$w = 0;
	$r = substr($a, 0xffffffff, 1);
	is($r, undef);
	is($w, 1);

	$w = 0;
	$r = substr($a, 0xffffffff+1, 1);
	is($r, undef);
	is($w, 1);

	$w = 0;
	ok( !eval { $r = substr($s=$a, 0xffffffff, 1, "_"); 1 } );
	is($r, undef);
	is($s, $a);
	is($w, 0);

	$w = 0;
	ok( !eval { $r = substr($s=$a, 0xffffffff+1, 1, "_"); 1 } );
	is($r, undef);
	is($s, $a);
	is($w, 0);

	utf8::upgrade($a);
    }
}

# [perl #77692] UTF8 cache not being reset when TARG is reused
ok eval {
 local ${^UTF8CACHE} = -1;
 for my $i (0..1)
 {
   my $dummy = length(substr("\x{100}",0,$i));
 }
 1
}, 'UTF8 cache is reset when TARG is reused [perl #77692]';

{
    use utf8;
    use open qw( :utf8 :std );
    no warnings 'once';

    my $t = "";
    substr $t, 0, 0, *ワルド;
    is($t, "*main::ワルド", "substr works on UTF-8 globs");

    $t = "The World!";
    substr $t, 0, 9, *ザ::ワルド;
    is($t, "*ザ::ワルド!", "substr works on a UTF-8 glob + stash");
}

{
    my $x = *foo;
    my $y = \substr *foo, 0, 0;
    is ref \$x, 'GLOB', '\substr does not coerce its glob arg just yet';
    $x = \"foo";
    $y = \substr *foo, 0, 0;
    is ref \$x, 'REF', '\substr does not coerce its ref arg just yet';
}

# Test that UTF8-ness of magic var changing does not confuse substr lvalue
# assignment.
# We use overloading for our magic var, but a typeglob would work, too.
package o {
    use overload '""' => sub { ++our $count; $_[0][0] }
}
my $refee = bless ["\x{100}a"], o::;
my $substr = \substr $refee, -2;	# UTF8 flag still off for $$substr.
$$substr = "b";				# UTF8 flag turns on when setsubstr
is $refee, "b",				# magic stringifies $$substr.
     'substr lvalue assignment when stringification turns on UTF8ness';

# Test that changing UTF8-ness does not confuse 4-arg substr.
$refee = bless [], "\x{100}a";
# stringify without returning on UTF8 flag on $refee:
my $string = $refee; $string = "$string";
substr $refee, 0, 0, "\xff";
is $refee, "\xff$string",
  '4-arg substr with target UTF8ness turning on when stringified';
$refee = bless [], "\x{100}";
() = "$refee"; # UTF8 flag now on
bless $refee, "\xff";
$string = $refee; $string = "$string";
substr $refee, 0, 0, "\xff";
is $refee, "\xff$string",
  '4-arg substr with target UTF8ness turning off when stringified';

# Overload count
$refee = bless ["foo"], o::;
$o::count = 0;
substr $refee, 0, 0, "";
is $o::count, 1, '4-arg substr calls overloading once on the target';
$refee = bless ["\x{100}"], o::;
() = "$refee"; # turn UTF8 flag on
$o::count = 0;
() = substr $refee, 0;
is $o::count, 1, 'rvalue substr calls overloading once on utf8 target';
$o::count = 0;
$refee = "";
${\substr $refee, 0} = bless ["\x{100}"], o::;
is $o::count, 1, 'assigning utf8 overload to substr lvalue calls ovld 1ce';

# [perl #7678] core dump with substr reference and localisation
{$b="abcde"; local $k; *k=\substr($b, 2, 1);}

# [perl #128260] assertion failure with \substr %h, \substr @h
{
    my %h = 1..100;
    my @a = 1..100;
    is ${\substr %h, 0}, scalar %h, '\substr %h';
    is ${\substr @a, 0}, scalar @a, '\substr @a';
}

} # sub run_tests - put tests above this line that can run in threads


my $destroyed;
{ package Class; DESTROY { ++$destroyed; } }

$destroyed = 0;
{
    my $x = '';
    substr($x,0,1) = "";
    $x = bless({}, 'Class');
}
is($destroyed, 1, 'Timely scalar destruction with lvalue substr');

{
    my $result_3363;
    sub a_3363 {
        my ($word, $replace) = @_;
        my $ref = \substr($word, 0, 1);
        $$ref = $replace;
        if ($replace eq "b") {
            $result_3363 = $word;
        } else {
            a_3363($word, "b");
        }
    }
    a_3363($_, "v") for "test";

    is($result_3363, "best", "ref-to-substr retains lvalue-ness under recursion [perl #3363]");
}

# failed with ASAN
fresh_perl_is('$0 = "/usr/bin/perl"; substr($0, 0, 0, $0)', '', {}, "(perl #129340) substr() with source in target");


# [perl #130624] - heap-use-after-free, observable under asan
{
    my $x = "\xE9zzzz";
    my $y = "\x{100}";
    my $z = substr $x, 0, 1, $y;
    is $z, "\xE9",        "RT#130624: heap-use-after-free in 4-arg substr (ret)";
    is $x, "\x{100}zzzz", "RT#130624: heap-use-after-free in 4-arg substr (targ)";
}

{
    our @ta;
    $#ta = -1;
    substr($#ta, 0, 2) = 23;
    is $#ta, 23;
    $#ta = -1;
    substr($#ta, 0, 2) =~ s/\A..\z/23/s;
    is $#ta, 23;
    $#ta = -1;
    substr($#ta, 0, 2, 23);
    is $#ta, 23;
    sub ta_tindex :lvalue { $#ta }
    $#ta = -1;
    ta_tindex() = 23;
    is $#ta, 23;
    $#ta = -1;
    substr(ta_tindex(), 0, 2) = 23;
    is $#ta, 23;
    $#ta = -1;
    substr(ta_tindex(), 0, 2) =~ s/\A..\z/23/s;
    is $#ta, 23;
    $#ta = -1;
    substr(ta_tindex(), 0, 2, 23);
    is $#ta, 23;
}

{ # [perl #132527]
    use feature 'refaliasing';
    no warnings 'experimental::refaliasing';
    my %h;
    \$h{foo} = \(my $bar = "baz");
    substr delete $h{foo}, 1, 1, o=>;
    is $bar, boz => 'first arg to 4-arg substr is loose lvalue context';
}

1;
