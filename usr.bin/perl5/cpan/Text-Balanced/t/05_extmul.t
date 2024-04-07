use 5.008001;

use strict;
use warnings;
use Test::More;
use Text::Balanced qw ( :ALL );

our $DEBUG;
sub debug { print "\t>>>",@_ if $DEBUG }

sub expect
{
    my ($l1, $l2) = @_;
    is_deeply $l1, $l2 or do {
        diag 'got:', explain $l1;
        diag 'expected:', explain $l2;
    };
}

sub divide
{
    my ($text, @index) = @_;
    my @bits = ();
    unshift @index, 0;
    push @index, length($text);
    for ( my $i= 0; $i < $#index; $i++)
    {
        push @bits, substr($text, $index[$i], $index[$i+1]-$index[$i]);
    }
    pop @bits;
    return @bits;

}

my $stdtext1 = q{$var = do {"val" && $val;};};

my $text = $stdtext1;
expect [ extract_multiple($text,undef,1) ],
       [ divide $stdtext1 => 4 ];

expect [ pos $text], [ 4 ];
expect [ $text ], [ $stdtext1 ];

$text = $stdtext1;
expect [ scalar extract_multiple($text,undef,1) ],
       [ divide $stdtext1 => 4 ];

expect [ pos $text], [ 0 ];
expect [ $text ], [ substr($stdtext1,4) ];


$text = $stdtext1;
expect [ extract_multiple($text,undef,2) ],
       [ divide($stdtext1 => 4, 10) ];

expect [ pos $text], [ 10 ];
expect [ $text ], [ $stdtext1 ];

$text = $stdtext1;
expect [ eval{local$^W;scalar extract_multiple($text,undef,2)} ],
       [ substr($stdtext1,0,4) ];

expect [ pos $text], [ 0 ];
expect [ $text ], [ substr($stdtext1,4) ];


$text = $stdtext1;
expect [ extract_multiple($text,undef,3) ],
       [ divide($stdtext1 => 4, 10, 26) ];

expect [ pos $text], [ 26 ];
expect [ $text ], [ $stdtext1 ];

$text = $stdtext1;
expect [ eval{local$^W;scalar extract_multiple($text,undef,3)} ],
       [ substr($stdtext1,0,4) ];

expect [ pos $text], [ 0 ];
expect [ $text ], [ substr($stdtext1,4) ];


$text = $stdtext1;
expect [ extract_multiple($text,undef,4) ],
       [ divide($stdtext1 => 4, 10, 26, 27) ];

expect [ pos $text], [ 27 ];
expect [ $text ], [ $stdtext1 ];

$text = $stdtext1;
expect [ eval{local$^W;scalar extract_multiple($text,undef,4)} ],
       [ substr($stdtext1,0,4) ];

expect [ pos $text], [ 0 ];
expect [ $text ], [ substr($stdtext1,4) ];


$text = $stdtext1;
expect [ extract_multiple($text,undef,5) ],
       [ divide($stdtext1 => 4, 10, 26, 27) ];

expect [ pos $text], [ 27 ];
expect [ $text ], [ $stdtext1 ];


$text = $stdtext1;
expect [ eval{local$^W;scalar extract_multiple($text,undef,5)} ],
       [ substr($stdtext1,0,4) ];

expect [ pos $text], [ 0 ];
expect [ $text ], [ substr($stdtext1,4) ];



my $stdtext2 = q{$var = "val" && (1,2,3);};

$text = $stdtext2;
expect [ extract_multiple($text) ],
       [ divide($stdtext2 => 4, 7, 12, 24) ];

expect [ pos $text], [ 24 ];
expect [ $text ], [ $stdtext2 ];

$text = $stdtext2;
expect [ scalar extract_multiple($text) ],
       [ substr($stdtext2,0,4) ];

expect [ pos $text], [ 0 ];
expect [ $text ], [ substr($stdtext2,4) ];


$text = $stdtext2;
expect [ extract_multiple($text,[\&extract_bracketed]) ],
       [ substr($stdtext2,0,16), substr($stdtext2,16,7), substr($stdtext2,23) ];

expect [ pos $text], [ 24 ];
expect [ $text ], [ $stdtext2 ];

$text = $stdtext2;
expect [ scalar extract_multiple($text,[\&extract_bracketed]) ],
       [ substr($stdtext2,0,16) ];

expect [ pos $text], [ 0 ];
expect [ $text ], [ substr($stdtext2,15) ];


$text = $stdtext2;
expect [ extract_multiple($text,[\&extract_variable]) ],
       [ substr($stdtext2,0,4), substr($stdtext2,4) ];

expect [ pos $text], [ length($text) ];
expect [ $text ], [ $stdtext2 ];

$text = $stdtext2;
expect [ scalar extract_multiple($text,[\&extract_variable]) ],
       [ substr($stdtext2,0,4) ];

expect [ pos $text], [ 0 ];
expect [ $text ], [ substr($stdtext2,4) ];


$text = $stdtext2;
expect [ extract_multiple($text,[\&extract_quotelike]) ],
       [ substr($stdtext2,0,7), substr($stdtext2,7,5), substr($stdtext2,12) ];

expect [ pos $text], [ length($text) ];
expect [ $text ], [ $stdtext2 ];

$text = $stdtext2;
expect [ scalar extract_multiple($text,[\&extract_quotelike]) ],
       [ substr($stdtext2,0,7) ];

expect [ pos $text], [ 0 ];
expect [ $text ], [ substr($stdtext2,6) ];


$text = $stdtext2;
expect [ extract_multiple($text,[\&extract_quotelike],2,1) ],
       [ substr($stdtext2,7,5) ];

expect [ pos $text], [ 23 ];
expect [ $text ], [ $stdtext2 ];

$text = $stdtext2;
expect [ eval{local$^W;scalar extract_multiple($text,[\&extract_quotelike],2,1)} ],
       [ substr($stdtext2,7,5) ];

expect [ pos $text], [ 6 ];
expect [ $text ], [ substr($stdtext2,0,6). substr($stdtext2,12) ];


$text = $stdtext2;
expect [ extract_multiple($text,[\&extract_quotelike],1,1) ],
       [ substr($stdtext2,7,5) ];

expect [ pos $text], [ 12 ];
expect [ $text ], [ $stdtext2 ];

$text = $stdtext2;
expect [ scalar extract_multiple($text,[\&extract_quotelike],1,1) ],
       [ substr($stdtext2,7,5) ];

expect [ pos $text], [ 6 ];
expect [ $text ], [ substr($stdtext2,0,6). substr($stdtext2,12) ];

my $stdtext3 = "a,b,c";

$_ = $stdtext3;
expect [ extract_multiple(undef, [ sub { /\G[a-z]/gc && $& } ]) ],
       [ divide($stdtext3 => 1,2,3,4,5) ];

expect [ pos ], [ 5 ];
expect [ $_ ], [ $stdtext3 ];

$_ = $stdtext3;
expect [ scalar extract_multiple(undef, [ sub { /\G[a-z]/gc && $& } ]) ],
       [ divide($stdtext3 => 1) ];

expect [ pos ], [ 0 ];
expect [ $_ ], [ substr($stdtext3,1) ];

$_ = $stdtext3;
expect [ extract_multiple(undef, [ qr/\G[a-z]/ ]) ],
       [ divide($stdtext3 => 1,2,3,4,5) ];

expect [ pos ], [ 5 ];
expect [ $_ ], [ $stdtext3 ];

$_ = $stdtext3;
expect [ scalar extract_multiple(undef, [ qr/\G[a-z]/ ]) ],
       [ divide($stdtext3 => 1) ];

expect [ pos ], [ 0 ];
expect [ $_ ], [ substr($stdtext3,1) ];

$_ = $stdtext3;
expect [ extract_multiple(undef, [ q/([a-z]),?/ ]) ],
       [ qw(a b c) ];

expect [ pos ], [ 5 ];
expect [ $_ ], [ $stdtext3 ];

$_ = $stdtext3;
expect [ scalar extract_multiple(undef, [ q/([a-z]),?/ ]) ],
       [ divide($stdtext3 => 1) ];

expect [ pos ], [ 0 ];
expect [ $_ ], [ substr($stdtext3,2) ];

# Fails in Text-Balanced-1.95 with result ['1 ', '""', '1234']
$_ = q{ ""1234};
expect [ extract_multiple(undef, [\&extract_quotelike]) ],
       [ ' ', '""', '1234' ];

my $not_here_doc = "sub f {\n my \$pa <<= 2;\n}\n\n"; # wrong in 2.04
expect [ extract_multiple($not_here_doc, [
  { DONT_MATCH => \&extract_quotelike }
]) ],
       [ "sub f {\n my \$pa <<= 2;\n}\n\n" ];

my $y_falsematch = <<'EOF'; # wrong in 2.04
my $p = {y => 1};
{ $pa=ones(3,3,3); my $f = do { my $i=1; my $v=$$p{y}-$i; $pb = $pa(,$i,) }; }
EOF
expect [ extract_multiple($y_falsematch, [
  \&extract_variable,
  { DONT_MATCH => \&extract_quotelike }
]) ],
  [ 'my ', '$p', " = {y => 1};\n{ ", '$pa', '=ones(3,3,3); my ', '$f',
    ' = do { my ', '$i', '=1; my ', '$v', qw(= $$p{y} - $i), '; ', '$pb',
    ' = ', '$pa', '(,', '$i', ",) }; }\n",
  ];

my $slashmatch = <<'EOF'; # wrong in 2.04
my $var = 10 / 3; if ($var !~ /\./) { decimal() ;}
EOF
my @expect_slash = ('my ', '$var', ' = 10 / 3; if (', '$var', " !~ ",
  '/\\./', ") { decimal() ;}\n"
);
expect [ extract_multiple($slashmatch, [
  \&extract_variable,
  \&extract_quotelike,
]) ],
  \@expect_slash;

$slashmatch = <<'EOF'; # wrong in 2.04
my $var = 10 / 3; if ($var =~ /\./) { decimal() ;}
EOF
$expect_slash[4] = " =~ ";
expect [ extract_multiple($slashmatch, [
  \&extract_variable,
  \&extract_quotelike,
]) ],
  \@expect_slash;

$slashmatch = <<'EOF'; # wrong in 2.04
my $var = 10 / 3; if ($var =~
  # a comment
  /\./) { decimal() ;}
EOF
my $comment = qr/(?<![\$\@%])#.*/;
my $id = qr/\b(?!([ysm]|q[rqxw]?|tr)\b)\w+/;
expect [ extract_multiple($slashmatch, [
  $comment,
  \&extract_variable,
  $id,
  \&extract_quotelike,
]) ],
  [ 'my', ' ', '$var', ' = ', '10', ' / ', '3', '; ', 'if', ' (', '$var',
    " =~\n  ", '# a comment', "\n  ", '/\\./', ') { ', 'decimal', "() ;}\n"
  ];

$slashmatch = <<'EOF'; # wrong in 2.04_01
my $r=(1-$PCi)/1+czip(1, -1)/czip(1, 1);
EOF
expect [ extract_multiple($slashmatch, [
  \&extract_variable, $id, \&extract_quotelike,
]) ],
  [
  'my', ' ', '$r', '=(', '1', '-', '$PCi', ')/', '1', '+',
  'czip', '(', '1', ', -', '1', ')/',
  'czip', '(', '1', ', ', '1', ");\n"
  ];

$slashmatch = <<'EOF'; # wrong in 2.04_01
$ndim--; $min = $mdim <= $ndim ? 1 : 0; $min = $mdim < $ndim ? 1 : 0;
EOF
expect [ extract_multiple($slashmatch, [
  \&extract_variable, $id, \&extract_quotelike,
]) ],
  [
  '$ndim', '--; ',
  '$min', ' = ', '$mdim', ' <= ', '$ndim', ' ? ', '1', ' : ', '0', '; ',
  '$min', ' = ', '$mdim', ' < ', '$ndim', ' ? ', '1', ' : ', '0', ";\n"
  ];

$slashmatch = <<'EOF'; # wrong in 2.04_01
$x->t->(($a))->sever;
wantarray ? 1 : 0; $min = $var ? 0;
EOF
expect [ extract_multiple($slashmatch, [
  \&extract_variable, $id, \&extract_quotelike,
]) ],
  [
  '$x->t->(($a))->sever', ";\n",
  'wantarray', ' ? ', '1', ' : ', '0', '; ',
  '$min', ' = ', '$var', ' ? ', '0', ";\n",
  ];

$slashmatch = <<'EOF'; # wrong in 2.04_01
$var //= 'default'; $x = 1 / 2;
EOF
expect [ extract_multiple($slashmatch, [
  \&extract_variable, \&extract_quotelike,
]) ],
  [
  '$var', ' //= ', '\'default\'', '; ', '$x', " = 1 / 2;\n"
  ];

$slashmatch = <<'EOF'; # wrong in 2.04_01
$m; return wantarray ? ($m, $i) : $var ? $m : 0;
EOF
expect [ extract_multiple($slashmatch, [
  \&extract_variable, \&extract_quotelike,
]) ],
  [
  '$m',
  '; return wantarray ? (', '$m', ', ', '$i', ') : ', '$var', ' ? ', '$m',
  " : 0;\n"
  ];

$slashmatch = <<'EOF'; # wrong in 2.05
$_ = 1 unless defined $_ and /\d\b/;
EOF
expect [ extract_multiple($slashmatch, [
  \&extract_variable, \&extract_quotelike,
]) ],
  [ '$_', ' = 1 unless defined ', '$_', ' and ', '/\\d\\b/', ";\n" ];

done_testing;
