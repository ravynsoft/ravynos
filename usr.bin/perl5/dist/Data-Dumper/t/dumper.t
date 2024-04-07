#!./perl -w
#
# testsuite for Data::Dumper
#

use strict;
use warnings;

use Data::Dumper;
use Config;
use Test::More;

# Since Perl 5.8.1 because otherwise hash ordering is really random.
$Data::Dumper::Sortkeys = 1;
$Data::Dumper::Pad = "#";

my $XS;

# Force Data::Dumper::Dump to use perl. We test Dumpxs explicitly by calling
# it direct. Out here it lets us knobble the next if to test that the perl
# only tests do work (and count correctly)
$Data::Dumper::Useperl = 1;
if (defined &Data::Dumper::Dumpxs) {
    print "### XS extension loaded, will run XS tests\n";
    $XS = 1;
}
else {
    print "### XS extensions not loaded, will NOT run XS tests\n";
    $XS = 0;
}

our ( @a, $c, $d, $foo, @foo, %foo, @globs, $v, $ping, %ping );
our ( @dogs, %kennel, $mutts );

our ( @numbers, @strings );
our ( @numbers_s, @numbers_i, @numbers_is, @numbers_n, @numbers_ns, @numbers_ni, @numbers_nis );
our ( @strings_s, @strings_i, @strings_is, @strings_n, @strings_ns, @strings_ni, @strings_nis );

# Perl 5.16 was the first version that correctly handled Unicode in typeglob
# names. Tests for how globs are dumped must revise their expectations
# downwards when run on earlier Perls.
sub change_glob_expectation {
    my ($input) = @_;
    if ($] < 5.016) {
        $input =~ s<\\x\{([0-9a-f]+)\}>{
            my $s = chr hex $1;
            utf8::encode($s);
            join '', map sprintf('\\%o', ord), split //, $s;
        }ge;
    }
    return $input;
}

sub convert_to_native {
    my $input = shift;

    my @output;

    # The input should always be one of the following constructs
    while ($input =~ m/ ( \\ [0-7]+ )
                      | ( \\ x \{ [[:xdigit:]]+ } )
                      | ( \\ . )
                      | ( . ) /gx)
    {
        #print STDERR __LINE__, ": ", $&, "\n";
        my $index;
        my $replacement;
        if (defined $4) {       # Literal
            $index = ord $4;
            $replacement = $4;
        }
        elsif (defined $3) {    # backslash escape
            $index = ord eval "\"$3\"";
            $replacement = $3;
        }
        elsif (defined $2) {    # Hex
            $index = utf8::unicode_to_native(ord eval "\"$2\"");

            # But low hex numbers are always in octal.  These are all
            # controls.  The outlier \c? control is also in octal.
            my $format = ($index < ord(" ") || $index == ord("\c?"))
                         ? "\\%o"
                         : "\\x{%x}";
            $replacement = sprintf($format, $index);
        }
        elsif (defined $1) {    # Octal
            $index = utf8::unicode_to_native(ord eval "\"$1\"");
            $replacement = sprintf("\\%o", $index);
        }
        else {
            die "Unexpected match in convert_to_native()";
        }

        if (defined $output[$index]) {
            print STDERR "ordinal $index already has '$output[$index]'; skipping '$replacement'\n";
            next;
        }

        $output[$index] = $replacement;
    }

    return join "", grep { defined } @output;
}

sub TEST {
    my ($string, $desc, $want) = @_;
    Carp::confess("Tests must have a description")
            unless $desc;

    local $Test::Builder::Level = $Test::Builder::Level + 1;
 SKIP: {
        my $have = do {
            no strict;
            eval $string;
        };
        my $error = $@;

        if (defined $error && length $error) {
            is($error, "", "$desc set \$@");
            skip('No point in running eval after an error', 2);
        }

        $have =~ s/([A-Z]+)\(0x[0-9a-f]+\)/$1(0xdeadbeef)/g
            if $want =~ /deadbeef/;
        is($have, $want, $desc);

        {
            no strict;
            eval "$have";
        }

        is($@, "", "$desc - output did not eval")
            or skip('No point in restesting if output failed eval');

        $have = do {
            no strict;
            eval $string;
        };
        $error = $@;

        if (defined $error && length $error) {
            is($error, "", "$desc after eval set \$@");
        }
        else {
            $have =~ s/([A-Z]+)\(0x[0-9a-f]+\)/$1(0xdeadbeef)/g
                if $want =~ /deadbeef/;
            is($have, $want, "$desc after eval");
        }
    }
}

sub SKIP_BOTH {
    my $reason = shift;
 SKIP: {
        skip($reason, $XS ? 6 : 3);
    }
}

# It's more reliable to match (and substitute) on 'Dumpxs' than 'Dump'
# (the latter is a substring of many things), but as historically we've tested
# "pure perl" then "XS" it seems better to have $want_xs as an optional
# parameter.
sub TEST_BOTH {
    my ($testcase, $desc, $want, $want_xs, $skip_xs) = @_;
    $want_xs = $want
        unless defined $want_xs;
    my $desc_pp = $desc;
    my $testcase_pp = $testcase;
    Carp::confess("Testcase must contain ->Dumpxs or DumperX")
            unless $testcase_pp =~ s/->Dumpxs\b/->Dump/g
            || $testcase_pp =~ s/\bDumperX\b/Dumper/g;
    unless ($desc_pp =~ s/Dumpxs/Dump/ || $desc_pp =~ s/\bDumperX\b/Dumper/) {
        $desc .= ', XS';
    }

    local $Test::Builder::Level = $Test::Builder::Level + 1;
    TEST($testcase_pp, $desc_pp, $want);
    return
        unless $XS;
    if ($skip_xs) {
    SKIP: {
            skip($skip_xs, 3);
        }
    }
    else {
        TEST($testcase, $desc, $want_xs);
    }
}


#############

my @c = ('c');
$c = \@c;
$b = {};          # FIXME - use another variable name
$a = [1, $b, $c]; # FIXME - use another variable name
$b->{a} = $a;
$b->{b} = $a->[1];
$b->{c} = $a->[2];

#############
##
my $want = <<'EOT';
#$a = [
#       1,
#       {
#         'a' => $a,
#         'b' => $a->[1],
#         'c' => [
#                  'c'
#                ]
#       },
#       $a->[1]{'c'}
#     ];
#$b = $a->[1];
#$6 = $a->[1]{'c'};
EOT

TEST_BOTH(q(Data::Dumper->Dumpxs([$a,$b,$c], [qw(a b), 6])),
          'basic test with names: Dumpxs()',
          $want);

SCOPE: {
    local $Data::Dumper::Sparseseen = 1;
    TEST_BOTH(q(Data::Dumper->Dumpxs([$a,$b,$c], [qw(a b), 6])),
              'Sparseseen with names: Dumpxs()',
              $want);
}

#############
##
$want = <<'EOT';
#@a = (
#       1,
#       {
#         'a' => [],
#         'b' => {},
#         'c' => [
#                  'c'
#                ]
#       },
#       []
#     );
#$a[1]{'a'} = \@a;
#$a[1]{'b'} = $a[1];
#$a[2] = $a[1]{'c'};
#$b = $a[1];
EOT

$Data::Dumper::Purity = 1;         # fill in the holes for eval
TEST_BOTH(q(Data::Dumper->Dumpxs([$a, $b], [qw(*a b)])),
          'Purity: basic test with dereferenced array: Dumpxs()',
          $want);

SCOPE: {
  local $Data::Dumper::Sparseseen = 1;
  TEST_BOTH(q(Data::Dumper->Dumpxs([$a, $b], [qw(*a b)])),
            'Purity: Sparseseen with dereferenced array: Dumpxs()',
            $want);
}

#############
##
$want = <<'EOT';
#%b = (
#       'a' => [
#                1,
#                {},
#                [
#                  'c'
#                ]
#              ],
#       'b' => {},
#       'c' => []
#     );
#$b{'a'}[1] = \%b;
#$b{'b'} = \%b;
#$b{'c'} = $b{'a'}[2];
#$a = $b{'a'};
EOT

TEST_BOTH(q(Data::Dumper->Dumpxs([$b, $a], [qw(*b a)])),
          'basic test with dereferenced hash: Dumpxs()',
          $want);

#############
##
$want = <<'EOT';
#$a = [
#  1,
#  {
#    'a' => [],
#    'b' => {},
#    'c' => []
#  },
#  []
#];
#$a->[1]{'a'} = $a;
#$a->[1]{'b'} = $a->[1];
#$a->[1]{'c'} = \@c;
#$a->[2] = \@c;
#$b = $a->[1];
EOT

$Data::Dumper::Indent = 1;
TEST_BOTH(q{
            $d = Data::Dumper->new([$a,$b], [qw(a b)]);
            $d->Seen({'*c' => $c});
            $d->Dumpxs;
           }, 'Indent: Seen: Dumpxs()',
          $want);

#############
##
$want = <<'EOT';
#$a = [
#       #0
#       1,
#       #1
#       {
#         a => $a,
#         b => $a->[1],
#         c => [
#                #0
#                'c'
#              ]
#       },
#       #2
#       $a->[1]{c}
#     ];
#$b = $a->[1];
EOT

$d->Indent(3);
$d->Purity(0)->Quotekeys(0);
TEST_BOTH(q( $d->Reset; $d->Dumpxs ),
          'Indent(3): Purity(0)->Quotekeys(0): Dumpxs()',
          $want);

#############
##
$want = <<'EOT';
#$VAR1 = [
#  1,
#  {
#    'a' => [],
#    'b' => {},
#    'c' => [
#      'c'
#    ]
#  },
#  []
#];
#$VAR1->[1]{'a'} = $VAR1;
#$VAR1->[1]{'b'} = $VAR1->[1];
#$VAR1->[2] = $VAR1->[1]{'c'};
EOT

TEST_BOTH(q(Data::Dumper::DumperX($a)),
          'DumperX',
          $want);

#############
##
$want = <<'EOT';
#[
#  1,
#  {
#    a => $VAR1,
#    b => $VAR1->[1],
#    c => [
#      'c'
#    ]
#  },
#  $VAR1->[1]{c}
#]
EOT

{
  local $Data::Dumper::Purity = 0;
  local $Data::Dumper::Quotekeys = 0;
  local $Data::Dumper::Terse = 1;
  TEST_BOTH(q(Data::Dumper::DumperX($a)),
            'Purity 0: Quotekeys 0: Terse 1: DumperX',
            $want);
}

#############
##
$want = <<'EOT';
#$VAR1 = {
#  "abc\0'\efg" => "mno\0",
#  "reftest" => \\1
#};
EOT

$foo = { "abc\000\'\efg" => "mno\000",
         "reftest" => \\1,
       };
{
  local $Data::Dumper::Useqq = 1;
  TEST_BOTH(q(Data::Dumper::DumperX($foo)),
            'Useqq: DumperX',
            $want);
}

#############
#############

{
  package main;
  use Data::Dumper;
  $foo = 5;
  @foo = (-10,\*foo);
  %foo = (a=>1,b=>\$foo,c=>\@foo);
  $foo{d} = \%foo;
  $foo[2] = \%foo;

#############
##
  my $want = <<'EOT';
#$foo = \*::foo;
#*::foo = \5;
#*::foo = [
#           #0
#           -10,
#           #1
#           do{my $o},
#           #2
#           {
#             'a' => 1,
#             'b' => do{my $o},
#             'c' => [],
#             'd' => {}
#           }
#         ];
#*::foo{ARRAY}->[1] = $foo;
#*::foo{ARRAY}->[2]{'b'} = *::foo{SCALAR};
#*::foo{ARRAY}->[2]{'c'} = *::foo{ARRAY};
#*::foo{ARRAY}->[2]{'d'} = *::foo{ARRAY}->[2];
#*::foo = *::foo{ARRAY}->[2];
#@bar = @{*::foo{ARRAY}};
#%baz = %{*::foo{ARRAY}->[2]};
EOT

  $Data::Dumper::Purity = 1;
  $Data::Dumper::Indent = 3;
  TEST_BOTH(q(Data::Dumper->Dumpxs([\\*foo, \\@foo, \\%foo], ['*foo', '*bar', '*baz'])),
            'Purity 1: Indent 3: Dumpxs()',
            $want);

#############
##
  $want = <<'EOT';
#$foo = \*::foo;
#*::foo = \5;
#*::foo = [
#  -10,
#  do{my $o},
#  {
#    'a' => 1,
#    'b' => do{my $o},
#    'c' => [],
#    'd' => {}
#  }
#];
#*::foo{ARRAY}->[1] = $foo;
#*::foo{ARRAY}->[2]{'b'} = *::foo{SCALAR};
#*::foo{ARRAY}->[2]{'c'} = *::foo{ARRAY};
#*::foo{ARRAY}->[2]{'d'} = *::foo{ARRAY}->[2];
#*::foo = *::foo{ARRAY}->[2];
#$bar = *::foo{ARRAY};
#$baz = *::foo{ARRAY}->[2];
EOT

  $Data::Dumper::Indent = 1;
  TEST_BOTH(q(Data::Dumper->Dumpxs([\\*foo, \\@foo, \\%foo], ['foo', 'bar', 'baz'])),
            'Purity 1: Indent 1: Dumpxs()',
            $want);

#############
##
  $want = <<'EOT';
#@bar = (
#  -10,
#  \*::foo,
#  {}
#);
#*::foo = \5;
#*::foo = \@bar;
#*::foo = {
#  'a' => 1,
#  'b' => do{my $o},
#  'c' => [],
#  'd' => {}
#};
#*::foo{HASH}->{'b'} = *::foo{SCALAR};
#*::foo{HASH}->{'c'} = \@bar;
#*::foo{HASH}->{'d'} = *::foo{HASH};
#$bar[2] = *::foo{HASH};
#%baz = %{*::foo{HASH}};
#$foo = $bar[1];
EOT

  TEST_BOTH(q(Data::Dumper->Dumpxs([\\@foo, \\%foo, \\*foo], ['*bar', '*baz', '*foo'])),
            'array|hash|glob dereferenced: Dumpxs()',
            $want);

#############
##
  $want = <<'EOT';
#$bar = [
#  -10,
#  \*::foo,
#  {}
#];
#*::foo = \5;
#*::foo = $bar;
#*::foo = {
#  'a' => 1,
#  'b' => do{my $o},
#  'c' => [],
#  'd' => {}
#};
#*::foo{HASH}->{'b'} = *::foo{SCALAR};
#*::foo{HASH}->{'c'} = $bar;
#*::foo{HASH}->{'d'} = *::foo{HASH};
#$bar->[2] = *::foo{HASH};
#$baz = *::foo{HASH};
#$foo = $bar->[1];
EOT

  TEST_BOTH(q(Data::Dumper->Dumpxs([\\@foo, \\%foo, \\*foo], ['bar', 'baz', 'foo'])),
            'array|hash|glob: not dereferenced: Dumpxs()',
            $want);

#############
##
  $want = <<'EOT';
#$foo = \*::foo;
#@bar = (
#  -10,
#  $foo,
#  {
#    a => 1,
#    b => \5,
#    c => \@bar,
#    d => $bar[2]
#  }
#);
#%baz = %{$bar[2]};
EOT

  $Data::Dumper::Purity = 0;
  $Data::Dumper::Quotekeys = 0;
  TEST_BOTH(q(Data::Dumper->Dumpxs([\\*foo, \\@foo, \\%foo], ['*foo', '*bar', '*baz'])),
            'Purity 0: Quotekeys 0: dereferenced: Dumpxs',
            $want);

#############
##
  $want = <<'EOT';
#$foo = \*::foo;
#$bar = [
#  -10,
#  $foo,
#  {
#    a => 1,
#    b => \5,
#    c => $bar,
#    d => $bar->[2]
#  }
#];
#$baz = $bar->[2];
EOT

  TEST_BOTH(q(Data::Dumper->Dumpxs([\\*foo, \\@foo, \\%foo], ['foo', 'bar', 'baz'])),
            'Purity 0: Quotekeys 0: not dereferenced: Dumpxs()',
            $want);
}

#############
#############

{
  package main;
  @dogs = ( 'Fido', 'Wags' );
  %kennel = (
            First => \$dogs[0],
            Second =>  \$dogs[1],
           );
  $dogs[2] = \%kennel;
  $mutts = \%kennel;
  $mutts = $mutts;         # avoid warning

#############
##
  my $want = <<'EOT';
#%kennels = (
#  First => \'Fido',
#  Second => \'Wags'
#);
#@dogs = (
#  ${$kennels{First}},
#  ${$kennels{Second}},
#  \%kennels
#);
#%mutts = %kennels;
EOT

  TEST_BOTH(q{
              $d = Data::Dumper->new([\\%kennel, \\@dogs, $mutts],
                                     [qw(*kennels *dogs *mutts)] );
              $d->Dumpxs;
	    }, 'constructor: hash|array|scalar: Dumpxs()',
            $want);

#############
##
  $want = <<'EOT';
#%kennels = %kennels;
#@dogs = @dogs;
#%mutts = %kennels;
EOT

  TEST_BOTH(q($d->Dumpxs),
            'object call: Dumpxs',
            $want);

#############
##
  $want = <<'EOT';
#%kennels = (
#  First => \'Fido',
#  Second => \'Wags'
#);
#@dogs = (
#  ${$kennels{First}},
#  ${$kennels{Second}},
#  \%kennels
#);
#%mutts = %kennels;
EOT

  TEST_BOTH(q($d->Reset; $d->Dumpxs),
            'Reset and Dumpxs separate calls',
            $want);

#############
##
  $want = <<'EOT';
#@dogs = (
#  'Fido',
#  'Wags',
#  {
#    First => \$dogs[0],
#    Second => \$dogs[1]
#  }
#);
#%kennels = %{$dogs[2]};
#%mutts = %{$dogs[2]};
EOT

  TEST_BOTH(q{
              $d = Data::Dumper->new([\\@dogs, \\%kennel, $mutts],
                                     [qw(*dogs *kennels *mutts)] );
              $d->Dumpxs;
	    }, 'constructor: array|hash|scalar: Dumpxs()',
            $want);

#############
##
  TEST_BOTH(q($d->Reset->Dumpxs),
            'Reset Dumpxs chained',
            $want);

#############
##
  $want = <<'EOT';
#@dogs = (
#  'Fido',
#  'Wags',
#  {
#    First => \'Fido',
#    Second => \'Wags'
#  }
#);
#%kennels = (
#  First => \'Fido',
#  Second => \'Wags'
#);
EOT

  TEST_BOTH(q{
              $d = Data::Dumper->new( [\@dogs, \%kennel], [qw(*dogs *kennels)] );
              $d->Deepcopy(1)->Dumpxs;
             }, 'Deepcopy(1): Dumpxs',
            $want);
}

{

sub z { print "foo\n" }
$c = [ \&z ];

#############
##
  my $want = <<'EOT';
#$a = $b;
#$c = [
#  $b
#];
EOT

   TEST_BOTH(q(Data::Dumper->new([\&z,$c],['a','c'])->Seen({'b' => \&z})->Dumpxs;),
             'Seen: scalar: Dumpxs',
             $want);

#############
##
  $want = <<'EOT';
#$a = \&b;
#$c = [
#  \&b
#];
EOT

  TEST_BOTH(q(Data::Dumper->new([\&z,$c],['a','c'])->Seen({'*b' => \&z})->Dumpxs;),
            'Seen: glob: Dumpxs',
            $want);

#############
##
  $want = <<'EOT';
#*a = \&b;
#@c = (
#  \&b
#);
EOT

  TEST_BOTH(q(Data::Dumper->new([\&z,$c],['*a','*c'])->Seen({'*b' => \&z})->Dumpxs;),
            'Seen: glob: derference: Dumpxs',
            $want);
}

{
  $a = [];
  $a->[1] = \$a->[0];

#############
##
  my $want = <<'EOT';
#@a = (
#  undef,
#  do{my $o}
#);
#$a[1] = \$a[0];
EOT

  TEST_BOTH(q(Data::Dumper->new([$a],['*a'])->Purity(1)->Dumpxs;),
            'Purity(1): dereference: Dumpxs',
            $want);
}

{
  $a = \\\\\'foo';
  $b = $$$a;

#############
##
  my $want = <<'EOT';
#$a = \\\\\'foo';
#$b = ${${$a}};
EOT

  TEST_BOTH(q(Data::Dumper->new([$a,$b],['a','b'])->Purity(1)->Dumpxs;),
            'Purity(1): not dereferenced: Dumpxs',
            $want);
}

{
  $a = [{ a => \$b }, { b => undef }];
  $b = [{ c => \$b }, { d => \$a }];

#############
##
  my $want = <<'EOT';
#$a = [
#  {
#    a => \[
#        {
#          c => do{my $o}
#        },
#        {
#          d => \[]
#        }
#      ]
#  },
#  {
#    b => undef
#  }
#];
#${$a->[0]{a}}->[0]->{c} = $a->[0]{a};
#${${$a->[0]{a}}->[1]->{d}} = $a;
#$b = ${$a->[0]{a}};
EOT

  TEST_BOTH(q(Data::Dumper->new([$a,$b],['a','b'])->Purity(1)->Dumpxs;),
            'Purity(1); Dumpxs again',
            $want);
}

{
  $a = [[[[\\\\\'foo']]]];
  $b = $a->[0][0];
  $c = $${$b->[0][0]};

#############
##
  my $want = <<'EOT';
#$a = [
#  [
#    [
#      [
#        \\\\\'foo'
#      ]
#    ]
#  ]
#];
#$b = $a->[0][0];
#$c = ${${$a->[0][0][0][0]}};
EOT

  TEST_BOTH(q(Data::Dumper->new([$a,$b,$c],['a','b','c'])->Purity(1)->Dumpxs;),
            'Purity(1): Dumpxs: 3 elements',
            $want);
}

{
    my $f = "pearl";
    my $e = [        $f ];
    $d = { 'e' => $e };
    $c = [        $d ];
    $b = { 'c' => $c }; # FIXME use different variable name
    $a = { 'b' => $b }; # FIXME use different variable name

#############
##
    my $want = <<'EOT';
#$a = {
#  b => {
#    c => [
#      {
#        e => 'ARRAY(0xdeadbeef)'
#      }
#    ]
#  }
#};
#$b = $a->{b};
#$c = $a->{b}{c};
EOT

    TEST_BOTH(q(Data::Dumper->new([$a,$b,$c],['a','b','c'])->Maxdepth(4)->Dumpxs;),
              'Maxdepth(4): Dumpxs()',
              $want);

#############
##
    $want = <<'EOT';
#$a = {
#  b => 'HASH(0xdeadbeef)'
#};
#$b = $a->{b};
#$c = [
#  'HASH(0xdeadbeef)'
#];
EOT

    TEST_BOTH(q(Data::Dumper->new([$a,$b,$c],['a','b','c'])->Maxdepth(1)->Dumpxs;),
              'Maxdepth(1): Dumpxs()',
              $want);
}

{
    $a = \$a;
    $b = [$a];

#############
##
    my $want = <<'EOT';
#$b = [
#  \$b->[0]
#];
EOT

    TEST_BOTH(q(Data::Dumper->new([$b],['b'])->Purity(0)->Dumpxs;),
               'Purity(0): Dumpxs()',
               $want);

#############
##
    $want = <<'EOT';
#$b = [
#  \do{my $o}
#];
#${$b->[0]} = $b->[0];
EOT

    TEST_BOTH(q(Data::Dumper->new([$b],['b'])->Purity(1)->Dumpxs;),
              'Purity(1): Dumpxs',
              $want);
}

{
  $a = "\x{09c10}";
#############
## XS code was adding an extra \0
  my $want = <<'EOT';
#$a = "\x{9c10}";
EOT

  TEST_BOTH(q(Data::Dumper->Dumpxs([$a], ['a'])),
            "\\x{9c10}",
            $want);
}

{
  my $i = 0;
  $a = { map { ("$_$_$_", ++$i) } 'I'..'Q' }; # FIXME use different variable name

#############
##
  my $want = <<'EOT';
#$VAR1 = {
#  III => 1,
#  JJJ => 2,
#  KKK => 3,
#  LLL => 4,
#  MMM => 5,
#  NNN => 6,
#  OOO => 7,
#  PPP => 8,
#  QQQ => 9
#};
EOT

  TEST_BOTH(q(Data::Dumper->new([$a])->Dumpxs;),
            'basic test without names: Dumpxs()',
            $want);
}

{
  my $i = 5;
  $c = { map { (++$i, "$_$_$_") } 'I'..'Q' };
  local $Data::Dumper::Sortkeys = \&sort199;
  sub sort199 {
    my $hash = shift;
    return [ sort { $b <=> $a } keys %$hash ];
  }

#############
##
  my $want = <<'EOT';
#$VAR1 = {
#  14 => 'QQQ',
#  13 => 'PPP',
#  12 => 'OOO',
#  11 => 'NNN',
#  10 => 'MMM',
#  9 => 'LLL',
#  8 => 'KKK',
#  7 => 'JJJ',
#  6 => 'III'
#};
EOT

  TEST_BOTH(q(Data::Dumper->new([$c])->Dumpxs;),
            "sortkeys sub",
            $want);
}

{
  my $i = 5;
  $c = { map { (++$i, "$_$_$_") } 'I'..'Q' };
  $d = { reverse %$c };
  local $Data::Dumper::Sortkeys = \&sort205;
  sub sort205 {
    my $hash = shift;
    return [
      $hash eq $c ? (sort { $a <=> $b } keys %$hash)
		  : (reverse sort keys %$hash)
    ];
  }

#############
##
  my $want = <<'EOT';
#$VAR1 = [
#  {
#    6 => 'III',
#    7 => 'JJJ',
#    8 => 'KKK',
#    9 => 'LLL',
#    10 => 'MMM',
#    11 => 'NNN',
#    12 => 'OOO',
#    13 => 'PPP',
#    14 => 'QQQ'
#  },
#  {
#    QQQ => 14,
#    PPP => 13,
#    OOO => 12,
#    NNN => 11,
#    MMM => 10,
#    LLL => 9,
#    KKK => 8,
#    JJJ => 7,
#    III => 6
#  }
#];
EOT

  # the XS code does number values as strings
  my $want_xs = $want;
  $want_xs =~ s/ (\d+)(,?)$/ '$1'$2/gm;
  TEST_BOTH(q(Data::Dumper->new([[$c, $d]])->Dumpxs;),
            "more sortkeys sub",
            $want, $want_xs);
}

{
  local $Data::Dumper::Deparse = 1;
  local $Data::Dumper::Indent = 2;

#############
##
  my $want = <<'EOT';
#$VAR1 = {
#          foo => sub {
#                     use warnings;
#                     print 'foo';
#                 }
#        };
EOT

  if(" $Config{'extensions'} " !~ m[ B ]) {
    SKIP_BOTH("Perl configured without B module");
  } else {
    TEST_BOTH(q(Data::Dumper->new([{ foo => sub { print "foo"; } }])->Dumpxs),
              'Deparse 1: Indent 2; Dumpxs()',
              $want);
  }
}

#############
##

# This is messy.
# The controls (bare numbers) are stored either as integers or floating point.
# [depending on whether the tokeniser sees things like ".".]
# The peephole optimiser only runs for constant folding, not single constants,
# so I already have some NVs, some IVs
# The string versions are not. They are all PV

# This is arguably all far too chummy with the implementation, but I really
# want to ensure that we don't go wrong when flags on scalars get as side
# effects of reading them.

# These tests are actually testing the precise output of the current
# implementation, so will most likely fail if the implementation changes,
# even if the new implementation produces different but correct results.
# It would be nice to test for wrong answers, but I can't see how to do that,
# so instead I'm checking for unexpected answers. (ie -2 becoming "-2" is not
# wrong, but I can't see an easy, reliable way to code that knowledge)

{
    # Numbers (seen by the tokeniser as numbers, stored as numbers.
    @numbers = (
        0, +1, -2, 3.0, +4.0, -5.0, 6.5, +7.5, -8.5,
        9,  +10,  -11,  12.0,  +13.0,  -14.0,  15.5,  +16.25,  -17.75,
    );
    # Strings
  @strings = (
      "0", "+1", "-2", "3.0", "+4.0", "-5.0", "6.5", "+7.5", "-8.5", " 9",
      " +10", " -11", " 12.0", " +13.0", " -14.0", " 15.5", " +16.25", " -17.75",
  );

    # The perl code always does things the same way for numbers.
    my $WANT_PL_N = <<'EOT';
#$VAR1 = 0;
#$VAR2 = 1;
#$VAR3 = -2;
#$VAR4 = 3;
#$VAR5 = 4;
#$VAR6 = -5;
#$VAR7 = '6.5';
#$VAR8 = '7.5';
#$VAR9 = '-8.5';
#$VAR10 = 9;
#$VAR11 = 10;
#$VAR12 = -11;
#$VAR13 = 12;
#$VAR14 = 13;
#$VAR15 = -14;
#$VAR16 = '15.5';
#$VAR17 = '16.25';
#$VAR18 = '-17.75';
EOT
    # The perl code knows that 0 and -2 stringify exactly back to the strings,
    # so it dumps them as numbers, not strings.
    my $WANT_PL_S = <<'EOT';
#$VAR1 = 0;
#$VAR2 = '+1';
#$VAR3 = -2;
#$VAR4 = '3.0';
#$VAR5 = '+4.0';
#$VAR6 = '-5.0';
#$VAR7 = '6.5';
#$VAR8 = '+7.5';
#$VAR9 = '-8.5';
#$VAR10 = ' 9';
#$VAR11 = ' +10';
#$VAR12 = ' -11';
#$VAR13 = ' 12.0';
#$VAR14 = ' +13.0';
#$VAR15 = ' -14.0';
#$VAR16 = ' 15.5';
#$VAR17 = ' +16.25';
#$VAR18 = ' -17.75';
EOT

    # The XS code differs.
    # These are the numbers as seen by the tokeniser. Constants aren't folded
    # (which makes IVs where possible) so values the tokeniser thought were
    # floating point are stored as NVs. The XS code outputs these as strings,
    # but as it has converted them from NVs, leading + signs will not be there.
    my $WANT_XS_N = <<'EOT';
#$VAR1 = 0;
#$VAR2 = 1;
#$VAR3 = -2;
#$VAR4 = '3';
#$VAR5 = '4';
#$VAR6 = '-5';
#$VAR7 = '6.5';
#$VAR8 = '7.5';
#$VAR9 = '-8.5';
#$VAR10 = 9;
#$VAR11 = 10;
#$VAR12 = -11;
#$VAR13 = '12';
#$VAR14 = '13';
#$VAR15 = '-14';
#$VAR16 = '15.5';
#$VAR17 = '16.25';
#$VAR18 = '-17.75';
EOT

    # These are the strings as seen by the tokeniser. The XS code will output
    # these for all cases except where the scalar has been used in integer context
    my $WANT_XS_S = <<'EOT';
#$VAR1 = '0';
#$VAR2 = '+1';
#$VAR3 = '-2';
#$VAR4 = '3.0';
#$VAR5 = '+4.0';
#$VAR6 = '-5.0';
#$VAR7 = '6.5';
#$VAR8 = '+7.5';
#$VAR9 = '-8.5';
#$VAR10 = ' 9';
#$VAR11 = ' +10';
#$VAR12 = ' -11';
#$VAR13 = ' 12.0';
#$VAR14 = ' +13.0';
#$VAR15 = ' -14.0';
#$VAR16 = ' 15.5';
#$VAR17 = ' +16.25';
#$VAR18 = ' -17.75';
EOT

    # These are the numbers as IV-ized by &
    # These will differ from WANT_XS_N because now IV flags will be set on all
    # values that were actually integer, and the XS code will then output these
    # as numbers not strings.
    my $WANT_XS_I = <<'EOT';
#$VAR1 = 0;
#$VAR2 = 1;
#$VAR3 = -2;
#$VAR4 = 3;
#$VAR5 = 4;
#$VAR6 = -5;
#$VAR7 = '6.5';
#$VAR8 = '7.5';
#$VAR9 = '-8.5';
#$VAR10 = 9;
#$VAR11 = 10;
#$VAR12 = -11;
#$VAR13 = 12;
#$VAR14 = 13;
#$VAR15 = -14;
#$VAR16 = '15.5';
#$VAR17 = '16.25';
#$VAR18 = '-17.75';
EOT

    # Some of these tests will be redundant.
    @numbers_s = @numbers_i = @numbers_is = @numbers_n = @numbers_ns
        = @numbers_ni = @numbers_nis = @numbers;
    @strings_s = @strings_i = @strings_is = @strings_n = @strings_ns
        = @strings_ni = @strings_nis = @strings;
    # Use them in an integer context
    foreach (@numbers_i, @numbers_ni, @numbers_nis, @numbers_is,
             @strings_i, @strings_ni, @strings_nis, @strings_is) {
        my $b = sprintf "%d", $_;
    }
    # Use them in a floating point context
    foreach (@numbers_n, @numbers_ni, @numbers_nis, @numbers_ns,
             @strings_n, @strings_ni, @strings_nis, @strings_ns) {
        my $b = sprintf "%e", $_;
    }
    # Use them in a string context
    foreach (@numbers_s, @numbers_is, @numbers_nis, @numbers_ns,
             @strings_s, @strings_is, @strings_nis, @strings_ns) {
        my $b = sprintf "%s", $_;
    }

    # use Devel::Peek; Dump ($_) foreach @vanilla_c;

    my $nv_preserves_uv_4bits = defined $Config{d_nv_preserves_uv}
        || (exists($Config{nv_preserves_uv_bits}) && $Config{nv_preserves_uv_bits} >= 4);

    TEST_BOTH(q(Data::Dumper->new(\@numbers)->Dumpxs),
              'Numbers',
              $WANT_PL_N, $WANT_XS_N);
    TEST_BOTH(q(Data::Dumper->new(\@numbers_s)->Dumpxs),
              'Numbers PV',
              $WANT_PL_N, $WANT_XS_N);
    TEST_BOTH(q(Data::Dumper->new(\@numbers_i)->Dumpxs),
              'Numbers IV',
              $WANT_PL_N, $WANT_XS_I,
              $nv_preserves_uv_4bits ? "" : "NV does not preserve 4bits");
    TEST_BOTH(q(Data::Dumper->new(\@numbers_is)->Dumpxs),
              'Numbers IV,PV',
              $WANT_PL_N, $WANT_XS_I,
              $nv_preserves_uv_4bits ? "" : "NV does not preserve 4bits");
    TEST_BOTH(q(Data::Dumper->new(\@numbers_n)->Dumpxs),
              'XS Numbers NV',
              $WANT_PL_N, $WANT_XS_N);
    TEST_BOTH(q(Data::Dumper->new(\@numbers_ns)->Dumpxs),
              'XS Numbers NV,PV',
              $WANT_PL_N, $WANT_XS_N);
    TEST_BOTH(q(Data::Dumper->new(\@numbers_ni)->Dumpxs),
              'Numbers NV,IV',
              $WANT_PL_N, $WANT_XS_I,
              $nv_preserves_uv_4bits ? "" : "NV does not preserve 4bits");
    TEST_BOTH(q(Data::Dumper->new(\@numbers_nis)->Dumpxs),
              'Numbers NV,IV,PV',
              $WANT_PL_N, $WANT_XS_I,
              $nv_preserves_uv_4bits ? "" : "NV does not preserve 4bits");

    TEST_BOTH(q(Data::Dumper->new(\@strings)->Dumpxs),
              'Strings',
              $WANT_PL_S, $WANT_XS_S);
    TEST_BOTH(q(Data::Dumper->new(\@strings_s)->Dumpxs),
              'Strings PV',
              $WANT_PL_S, $WANT_XS_S);
    # This one used to really mess up. New code actually emulates the .pm code
    TEST_BOTH(q(Data::Dumper->new(\@strings_i)->Dumpxs),
              'Strings IV',
              $WANT_PL_S);
    TEST_BOTH(q(Data::Dumper->new(\@strings_is)->Dumpxs),
              'Strings IV,PV',
              $WANT_PL_S);
    TEST_BOTH(q(Data::Dumper->new(\@strings_n)->Dumpxs),
              'Strings NV',
              $WANT_PL_S, $WANT_XS_S,
              $nv_preserves_uv_4bits ? "" : "NV does not preserve 4bits");
    TEST_BOTH(q(Data::Dumper->new(\@strings_ns)->Dumpxs),
              'Strings NV,PV',
              $WANT_PL_S, $WANT_XS_S,
              $nv_preserves_uv_4bits ? "" : "NV does not preserve 4bits");
    # This one used to really mess up. New code actually emulates the .pm code
    TEST_BOTH(q(Data::Dumper->new(\@strings_ni)->Dumpxs),
              'Strings NV,IV',
              $WANT_PL_S);
    TEST_BOTH(q(Data::Dumper->new(\@strings_nis)->Dumpxs),
              'Strings NV,IV,PV',
              $WANT_PL_S);
}

{
  $a = "1\n";
#############
## Perl code was using /...$/ and hence missing the \n.
  my $want = <<'EOT';
my $VAR1 = '42
';
EOT

  # Can't pad with # as the output has an embedded newline.
  local $Data::Dumper::Pad = "my ";
  TEST_BOTH(q(Data::Dumper->Dumpxs(["42\n"])),
            "number with trailing newline",
            $want);
}

{
  @a = (
        999999999,
        1000000000,
        9999999999,
        10000000000,
        -999999999,
        -1000000000,
        -9999999999,
        -10000000000,
        4294967295,
        4294967296,
        -2147483648,
        -2147483649,
        );
#############
## Perl code flips over at 10 digits.
  my $want = <<'EOT';
#$VAR1 = 999999999;
#$VAR2 = '1000000000';
#$VAR3 = '9999999999';
#$VAR4 = '10000000000';
#$VAR5 = -999999999;
#$VAR6 = '-1000000000';
#$VAR7 = '-9999999999';
#$VAR8 = '-10000000000';
#$VAR9 = '4294967295';
#$VAR10 = '4294967296';
#$VAR11 = '-2147483648';
#$VAR12 = '-2147483649';
EOT

## XS code flips over at 11 characters ("-" is a char) or larger than int.
  my $want_xs = ~0 == 0xFFFFFFFF ? << 'EOT32' : << 'EOT64';
#$VAR1 = 999999999;
#$VAR2 = 1000000000;
#$VAR3 = '9999999999';
#$VAR4 = '10000000000';
#$VAR5 = -999999999;
#$VAR6 = '-1000000000';
#$VAR7 = '-9999999999';
#$VAR8 = '-10000000000';
#$VAR9 = 4294967295;
#$VAR10 = '4294967296';
#$VAR11 = '-2147483648';
#$VAR12 = '-2147483649';
EOT32
#$VAR1 = 999999999;
#$VAR2 = 1000000000;
#$VAR3 = 9999999999;
#$VAR4 = '10000000000';
#$VAR5 = -999999999;
#$VAR6 = '-1000000000';
#$VAR7 = '-9999999999';
#$VAR8 = '-10000000000';
#$VAR9 = 4294967295;
#$VAR10 = 4294967296;
#$VAR11 = '-2147483648';
#$VAR12 = '-2147483649';
EOT64

  TEST_BOTH(q(Data::Dumper->Dumpxs(\@a)),
            "long integers",
            $want, $want_xs);
}

{
    $b = "Bad. XS didn't escape dollar sign";
#############
    # B6 is chosen because it is UTF-8 variant on ASCII and all 3 EBCDIC
    # platforms that Perl currently purports to work on.  It also is the only
    # such code point that has the same meaning on all 4, the paragraph sign.
    my $want = <<"EOT"; # Careful. This is '' string written inside "" here doc
#\$VAR1 = '\$b\"\@\\\\\xB6';
EOT

    $a = "\$b\"\@\\\xB6\x{100}";
    chop $a;
    my $want_xs = <<'EOT'; # While this is "" string written inside "" here doc
#$VAR1 = "\$b\"\@\\\x{b6}";
EOT
    TEST_BOTH(q(Data::Dumper->Dumpxs([$a])),
              "XS utf8 flag with \" and \$",
              $want, $want_xs);

  # XS used to produce "$b\"' which is 4 chars, not 3. [ie wrongly qq(\$b\\\")]
#############
  $want = <<'EOT';
#$VAR1 = '$b"';
EOT

  $a = "\$b\"\x{100}";
  chop $a;
  TEST_BOTH(q(Data::Dumper->Dumpxs([$a])),
            "XS utf8 flag with \" and \$",
            $want);


  # XS used to produce 'D'oh!' which is well, D'oh!
  # Andreas found this one, which in turn discovered the previous two.
#############
  $want = <<'EOT';
#$VAR1 = 'D\'oh!';
EOT

  $a = "D'oh!\x{100}";
  chop $a;
  TEST_BOTH(q(Data::Dumper->Dumpxs([$a])),
            "XS utf8 flag with '",
            $want);
}

# Jarkko found that -Mutf8 caused some tests to fail.  Turns out that there
# was an otherwise untested code path in the XS for utf8 hash keys with purity
# 1

{
  my $want = <<'EOT';
#$ping = \*::ping;
#*::ping = \5;
#*::ping = {
#  "\x{decaf}\x{decaf}\x{decaf}\x{decaf}" => do{my $o}
#};
#*::ping{HASH}->{"\x{decaf}\x{decaf}\x{decaf}\x{decaf}"} = *::ping{SCALAR};
#%pong = %{*::ping{HASH}};
EOT
  local $Data::Dumper::Purity = 1;
  local $Data::Dumper::Sortkeys;
  $ping = 5;
  %ping = (chr (0xDECAF) x 4  =>\$ping);
  for $Data::Dumper::Sortkeys (0, 1) {
    TEST_BOTH(q(Data::Dumper->Dumpxs([\\*ping, \\%ping], ['*ping', '*pong'])),
              "utf8: Purity 1: Sortkeys: Dumpxs()",
              $want);
  }
}

# XS for quotekeys==0 was not being defensive enough against utf8 flagged
# scalars

{
  my $want = <<'EOT';
#$VAR1 = {
#  perl => 'rocks'
#};
EOT
  local $Data::Dumper::Quotekeys = 0;
  my $k = 'perl' . chr 256;
  chop $k;
  %foo = ($k => 'rocks');

  TEST_BOTH(q(Data::Dumper->Dumpxs([\\%foo])),
            "quotekeys == 0 for utf8 flagged ASCII",
            $want);
}
#############
{
  my $want = <<'EOT';
#$VAR1 = [
#  undef,
#  undef,
#  1
#];
EOT
    @foo = ();
    $foo[2] = 1;
    TEST_BOTH(q(Data::Dumper->Dumpxs([\@foo])),
              'Richard Clamp, Message-Id: <20030104005247.GA27685@mirth.demon.co.uk>: Dumpxs()',
              $want);
}

#############
# Make sure $obj->Dumpxs returns the right thing in list context. This was
# broken by the initial attempt to fix [perl #74170].
{
    my $want = <<'EOT';
#$VAR1 = [];
EOT
    TEST_BOTH(q(join " ", new Data::Dumper [[]],[] =>->Dumpxs),
              '$obj->Dumpxs in list context',
              $want);
}

#############
{
  my $want = '\0\1\2\3\4\5\6\a\b\t\n\13\f\r\16\17\20\21\22\23\24\25\26\27\30\31\32\e\34\35\36\37 !\"#\$%&\'()*+,-./0123456789:;<=>?\@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\177\200\201\202\203\204\205\206\207\210\211\212\213\214\215\216\217\220\221\222\223\224\225\226\227\230\231\232\233\234\235\236\237\240\241\242\243\244\245\246\247\250\251\252\253\254\255\256\257\260\261\262\263\264\265\266\267\270\271\272\273\274\275\276\277\300\301\302\303\304\305\306\307\310\311\312\313\314\315\316\317\320\321\322\323\324\325\326\327\330\331\332\333\334\335\336\337\340\341\342\343\344\345\346\347\350\351\352\353\354\355\356\357\360\361\362\363\364\365\366\367\370\371\372\373\374\375\376\377';
  $want = convert_to_native($want);
  $want = <<"EOT";
#\$VAR1 = [
#  "$want"
#];
EOT

  $foo = [ join "", map chr, 0..255 ];
  local $Data::Dumper::Useqq = 1;
  TEST_BOTH(q(Data::Dumper::DumperX($foo)),
            'All latin1 characters: DumperX',
            $want);
}

#############
{
  my $want = '\0\1\2\3\4\5\6\a\b\t\n\13\f\r\16\17\20\21\22\23\24\25\26\27\30\31\32\e\34\35\36\37 !\"#\$%&\'()*+,-./0123456789:;<=>?\@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\177\x{80}\x{81}\x{82}\x{83}\x{84}\x{85}\x{86}\x{87}\x{88}\x{89}\x{8a}\x{8b}\x{8c}\x{8d}\x{8e}\x{8f}\x{90}\x{91}\x{92}\x{93}\x{94}\x{95}\x{96}\x{97}\x{98}\x{99}\x{9a}\x{9b}\x{9c}\x{9d}\x{9e}\x{9f}\x{a0}\x{a1}\x{a2}\x{a3}\x{a4}\x{a5}\x{a6}\x{a7}\x{a8}\x{a9}\x{aa}\x{ab}\x{ac}\x{ad}\x{ae}\x{af}\x{b0}\x{b1}\x{b2}\x{b3}\x{b4}\x{b5}\x{b6}\x{b7}\x{b8}\x{b9}\x{ba}\x{bb}\x{bc}\x{bd}\x{be}\x{bf}\x{c0}\x{c1}\x{c2}\x{c3}\x{c4}\x{c5}\x{c6}\x{c7}\x{c8}\x{c9}\x{ca}\x{cb}\x{cc}\x{cd}\x{ce}\x{cf}\x{d0}\x{d1}\x{d2}\x{d3}\x{d4}\x{d5}\x{d6}\x{d7}\x{d8}\x{d9}\x{da}\x{db}\x{dc}\x{dd}\x{de}\x{df}\x{e0}\x{e1}\x{e2}\x{e3}\x{e4}\x{e5}\x{e6}\x{e7}\x{e8}\x{e9}\x{ea}\x{eb}\x{ec}\x{ed}\x{ee}\x{ef}\x{f0}\x{f1}\x{f2}\x{f3}\x{f4}\x{f5}\x{f6}\x{f7}\x{f8}\x{f9}\x{fa}\x{fb}\x{fc}\x{fd}\x{fe}\x{ff}\x{20ac}';
  $want = convert_to_native($want);
  $want = <<"EOT";
#\$VAR1 = [
#  "$want"
#];
EOT

  $foo = [ join "", map chr, 0..255, 0x20ac ];
  local $Data::Dumper::Useqq = 1;
  TEST_BOTH(q(Data::Dumper::DumperX($foo)),
            'All latin1 characters with utf8 flag including a wide character: DumperX',
            $want);
}

#############
{
  if (!Data::Dumper::SUPPORTS_CORE_BOOLS) {
      SKIP_BOTH("Core booleans not supported on older perls");
      last;
  }
  my $want = <<'EOT';
#$VAR1 = [
#  !!1,
#  !!0
#];
EOT

  $foo = [ !!1, !!0 ];
  TEST_BOTH(q(Data::Dumper::DumperX($foo)),
            'Booleans',
            $want);
}


#############
{
  # If XS cannot load, the pure-Perl version cannot deparse vstrings with
  # underscores properly.
  # Says the original comment. However, the story is more complex than that.
  # 1) If *all* XS cannot load, Data::Dumper fails hard, because it needs
  #    Scalar::Util.
  # 2) However, if Data::Dumper's XS cannot load, then Data::Dumper uses the
  #    "Pure Perl" implementation, which uses C<sprintf "%vd", $val> and the
  #    comment above applies.
  # 3) However, if we "just" set $Data::Dumper::Useperl true, then Dump *calls*
  #    the "Pure Perl" (general) implementation, but that calls a helper in the
  #    XS code (&_vstring) and it *does* deparse these vstrings properly
  # Meaning that for case 3, what we actually *test*, we get "VSTRINGS_CORRECT"
  # The "problem" comes that if one deletes Dumper.so and re-tests, it's case 2
  # and this test will fail, because case 2 output is:
  #
  #$a = \v65.66.67;
  #$b = \v65.66.67;
  #$c = \v65.66.67;
  #$d = \'ABC';
  #
  # This is the test output removed by commit 55d1a9a4aa623c18 in Aug 2012:
  #     Data::Dumper: Fix tests for pure-Perl implementation
  #
  #     Father Chrysostomos fixed vstring handling in both XS and pure-Perl
  #     implementations of Data::Dumper in
  #     de5ef703c7d8db6517e7d56d9c018d3ad03f210e.
  #
  #     He also updated the tests for the default XS implementation, but it seems
  #     that he missed the test changes necessary for the pure-Perl implementation
  #     which now also does the right thing.
  #
  # (But the relevant previous commit is not de5ef703c7d8 but d036e907fea3)
  # Part of the confusion here comes because at commit d036e907fea3 it was *not*
  # possible to remove Dumper.so and have Data::Dumper load - that bug was fixed
  # later (commit 1e9285c2ad54ae39, Dec 2011)
  #
  # Sigh, but even the test output added in d036e907fea3 was not correct
  # at least not consistent, as it had \v65.66.67, but the code at the time
  # generated \65.66.77 (no v). Now fixed.
  my $ABC_native = chr(65) . chr(66) . chr(67);
  my $want = $XS ? <<"VSTRINGS_CORRECT" : <<"NO_vstring_HELPER";
#\$a = \\v65.66.67;
#\$b = \\v65.66.067;
#\$c = \\v65.66.6_7;
#\$d = \\'$ABC_native';
VSTRINGS_CORRECT
#\$a = \\v65.66.67;
#\$b = \\v65.66.67;
#\$c = \\v65.66.67;
#\$d = \\'$ABC_native';
NO_vstring_HELPER

  @::_v = (
    \v65.66.67,
    \(eval 'v65.66.067'),
    \v65.66.6_7,
    \~v190.189.188
  );
  if ($] >= 5.010) {
    TEST_BOTH(q(Data::Dumper->Dumpxs(\@::_v, [qw(a b c d)])),
              'vstrings',
              $want);
  }
  else { # Skip tests before 5.10. vstrings considered funny before
    SKIP_BOTH("vstrings considered funny before 5.10.0");
  }
}

#############
{
  # [perl #107372] blessed overloaded globs
  my $want = <<'EOW';
#$VAR1 = bless( \*::finkle, 'overtest' );
EOW
  {
    package overtest;
    use overload fallback=>1, q\""\=>sub{"oaoaa"};
  }
  TEST_BOTH(q(Data::Dumper->Dumpxs([bless \*finkle, "overtest"])),
            'blessed overloaded globs',
            $want);
}
#############
{
  # [perl #74798] uncovered behaviour
  my $want = <<'EOW';
#$VAR1 = "\0000";
EOW
  local $Data::Dumper::Useqq = 1;
  TEST_BOTH(q(Data::Dumper->Dumpxs(["\x000"])),
            "\\ octal followed by digit",
            $want);

  $want = <<'EOW';
#$VAR1 = "\x{100}\0000";
EOW
  local $Data::Dumper::Useqq = 1;
  TEST_BOTH(q(Data::Dumper->Dumpxs(["\x{100}\x000"])),
            "\\ octal followed by digit unicode",
            $want);

  $want = <<'EOW';
#$VAR1 = "\0\x{660}";
EOW
  TEST_BOTH(q(Data::Dumper->Dumpxs(["\\x00\\x{0660}"])),
            "\\ octal followed by unicode digit",
            $want);

  # [perl #118933 - handling of digits
  $want = <<'EOW';
#$VAR1 = 0;
#$VAR2 = 1;
#$VAR3 = 90;
#$VAR4 = -10;
#$VAR5 = "010";
#$VAR6 = 112345678;
#$VAR7 = "1234567890";
EOW
  TEST_BOTH(q(Data::Dumper->Dumpxs([0, 1, 90, -10, "010", "112345678", "1234567890" ])),
            "numbers and number-like scalars",
            $want);
}
#############
{
  # [github #18614 - handling of Unicode characters in regexes]
  # [github #18764 - ... without breaking subsequent Latin-1]
  if ($] lt '5.010') {
      SKIP_BOTH("Incomplete support for UTF-8 in old perls");
      last;
  }
  my $want = <<"EOW";
#\$VAR1 = [
#  "\\x{41f}",
#  qr/\x{8b80}/,
#  qr/\x{41f}/,
#  qr/\x{b6}/,
#  '\xb6'
#];
EOW
  if ($] lt '5.010001') {
      $want =~ s!qr/!qr/(?-xism:!g;
      $want =~ s!/,!)/,!g;
  }
  elsif ($] gt '5.014') {
      $want =~ s{/(,?)$}{/u$1}mg;
  }
  my $want_xs = $want;
  $want_xs =~ s/'\xb6'/"\\x{b6}"/;
  $want_xs =~ s<([[:^ascii:]])> <sprintf '\\x{%x}', ord $1>ge;
  TEST_BOTH(qq(Data::Dumper->Dumpxs([ [qq/\x{41f}/, qr/\x{8b80}/, qr/\x{41f}/, qr/\x{b6}/, "\xb6"] ])),
            "string with Unicode + regexp with Unicode",
            $want, $want_xs);
}
#############
{
  # [more perl #58608 tests]
  my $bs = "\\\\";
  my $want = <<"EOW";
#\$VAR1 = [
#  qr/ \\/ /,
#  qr/ \\?\\/ /,
#  qr/ $bs\\/ /,
#  qr/ $bs:\\/ /,
#  qr/ \\?$bs:\\/ /,
#  qr/ $bs$bs\\/ /,
#  qr/ $bs$bs:\\/ /,
#  qr/ $bs$bs$bs\\/ /
#];
EOW
  if ($] lt '5.010001') {
      $want =~ s!qr/!qr/(?-xism:!g;
      $want =~ s! /! )/!g;
  }
  TEST_BOTH(qq(Data::Dumper->Dumpxs([ [qr! / !, qr! \\?/ !, qr! $bs/ !, qr! $bs:/ !, qr! \\?$bs:/ !, qr! $bs$bs/ !, qr! $bs$bs:/ !, qr! $bs$bs$bs/ !, ] ])),
            "more perl #58608",
            $want);
}
#############
{
  # [github #18614, github #18764, perl #58608 corner cases]
  if ($] lt '5.010') {
      SKIP_BOTH("Incomplete support for UTF-8 in old perls");
      last;
  }
  my $bs = "\\\\";
  my $want = <<"EOW";
#\$VAR1 = [
#  "\\x{2e18}",
#  qr/ \x{203d}\\/ /,
#  qr/ \\\x{203d}\\/ /,
#  qr/ \\\x{203d}$bs:\\/ /,
#  '\xB6'
#];
EOW
  if ($] lt '5.010001') {
      $want =~ s!qr/!qr/(?-xism:!g;
      $want =~ s!/,!)/,!g;
  }
  elsif ($] gt '5.014') {
      $want =~ s{/(,?)$}{/u$1}mg;
  }
  my $want_xs = $want;
  $want_xs =~ s/'\x{B6}'/"\\x{b6}"/;
  $want_xs =~ s/\x{203D}/\\x{203d}/g;
  TEST_BOTH(qq(Data::Dumper->Dumpxs([ [ '\x{2e18}', qr! \x{203d}/ !, qr! \\\x{203d}/ !, qr! \\\x{203d}$bs:/ !, "\xb6"] ])),
            "github #18614, github #18764, perl #58608 corner cases",
            $want, $want_xs);
}
#############
{
  # [CPAN #84569]
  my $dollar = '${\q($)}';
  my $want = <<"EOW";
#\$VAR1 = [
#  "\\x{2e18}",
#  qr/^\$/,
#  qr/^\$/,
#  qr/${dollar}foo/,
#  qr/\\\$foo/,
#  qr/$dollar \x{B6} /u,
#  qr/$dollar \x{203d} /u,
#  qr/\\\$ \x{203d} /u,
#  qr/\\\\$dollar \x{203d} /u,
#  qr/ \$| \x{203d} /u,
#  qr/ (\$) \x{203d} /u,
#  '\xB6'
#];
EOW
  if ($] lt '5.014') {
      $want =~ s{/u,$}{/,}mg;
  }
  if ($] lt '5.010001') {
      $want =~ s!qr/!qr/(?-xism:!g;
      $want =~ s!/,!)/,!g;
  }
  my $want_xs = $want;
  $want_xs =~ s/'\x{B6}'/"\\x{b6}"/;
  $want_xs =~ s/\x{B6}/\\x{b6}/;
  $want_xs =~ s/\x{203D}/\\x{203d}/g;
  my $have = <<"EOT";
Data::Dumper->Dumpxs([ [
  "\\x{2e18}",
  qr/^\$/,
  qr'^\$',
  qr'\$foo',
  qr/\\\$foo/,
  qr'\$ \x{B6} ',
  qr'\$ \x{203d} ',
  qr/\\\$ \x{203d} /,
  qr'\\\\\$ \x{203d} ',
  qr/ \$| \x{203d} /,
  qr/ (\$) \x{203d} /,
  '\xB6'
] ]);
EOT
  TEST_BOTH($have, "CPAN #84569", $want, $want_xs);
}
#############
{
  # [perl #82948]
  # re::regexp_pattern was moved to universal.c in v5.10.0-252-g192c1e2
  # and apparently backported to maint-5.10
  my $want = $] > 5.010 ? <<'NEW' : <<'OLD';
#$VAR1 = qr/abc/;
#$VAR2 = qr/abc/i;
NEW
#$VAR1 = qr/(?-xism:abc)/;
#$VAR2 = qr/(?i-xsm:abc)/;
OLD
  TEST_BOTH(q(Data::Dumper->Dumpxs([ qr/abc/, qr/abc/i ])), "qr// xs", $want);
}
#############

{
  sub foo {}
  my $want = <<'EOW';
#*a = sub { "DUMMY" };
#$b = \&a;
EOW

  TEST_BOTH(q(Data::Dumper->new([ \&foo, \\&foo ], [ "*a", "b" ])->Dumpxs),
            "name of code in *foo",
            $want);
}
############# [perl #124091]
{
    my $want = <<'EOT';
#$VAR1 = "\n";
EOT
    local $Data::Dumper::Useqq = 1;
    TEST_BOTH(qq(Data::Dumper::DumperX("\n")),
              '\n alone',
              $want);
}
#############
{
    no strict 'refs';
    @globs = map { $_, \$_ } map { *$_ } map { $_, "s::$_" }
        "foo", "\1bar", "L\x{e9}on", "m\x{100}cron", "snow\x{2603}";
}

{
  my $want = change_glob_expectation(<<'EOT');
#$globs = [
#  *::foo,
#  \*::foo,
#  *s::foo,
#  \*s::foo,
#  *{"::\1bar"},
#  \*{"::\1bar"},
#  *{"s::\1bar"},
#  \*{"s::\1bar"},
#  *{"::L\351on"},
#  \*{"::L\351on"},
#  *{"s::L\351on"},
#  \*{"s::L\351on"},
#  *{"::m\x{100}cron"},
#  \*{"::m\x{100}cron"},
#  *{"s::m\x{100}cron"},
#  \*{"s::m\x{100}cron"},
#  *{"::snow\x{2603}"},
#  \*{"::snow\x{2603}"},
#  *{"s::snow\x{2603}"},
#  \*{"s::snow\x{2603}"}
#];
EOT
  local $Data::Dumper::Useqq = 1;
  if (ord("A") == 65) {
    TEST_BOTH(q(Data::Dumper->Dumpxs([\@globs], ["globs"])), 'globs: Dumpxs()',
              $want);
  }
  else {
    SKIP_BOTH("ASCII-dependent test");
  }
}
#############
{
  my $want = change_glob_expectation(<<'EOT');
#$v = {
#  a => \*::ppp,
#  b => \*{'::a/b'},
#  c => \*{"::a\x{2603}b"}
#};
#*::ppp = {
#  a => 1
#};
#*{'::a/b'} = {
#  b => 3
#};
#*{"::a\x{2603}b"} = {
#  c => 5
#};
EOT
  *ppp = { a => 1 };
  {
    no strict 'refs';
    *{"a/b"} = { b => 3 };
    *{"a\x{2603}b"} = { c => 5 };
    $v = { a => \*ppp, b => \*{"a/b"}, c => \*{"a\x{2603}b"} };
  }
  local $Data::Dumper::Purity = 1;
  TEST_BOTH(q(Data::Dumper->Dumpxs([$v], ["v"])),
            'glob purity: Dumpxs()',
            $want);
  $want =~ tr/'/"/;
  local $Data::Dumper::Useqq = 1;
  TEST_BOTH(q(Data::Dumper->Dumpxs([$v], ["v"])),
            'glob purity, useqq: Dumpxs()',
            $want);
}
#############
{
  my $want = <<'EOT';
#$3 = {};
#$bang = [];
EOT
  {
    package fish;

    use overload '""' => sub { return "bang" };

    sub new {
      return bless qr//;
    }
  }
  # 4.5/1.5 generates the *NV* 3.0, which doesn't set SVf_POK true in 5.20.0+
  # overloaded strings never set SVf_POK true
  TEST_BOTH(q(Data::Dumper->Dumpxs([{}, []], [4.5/1.5, fish->new()])),
            'names that are not simple strings: Dumpxs()',
            $want);
}

done_testing();
