use 5.008001;

use strict;
use warnings;
use Test::More;
use Text::Balanced qw ( extract_codeblock );

our $DEBUG;
sub debug { print "\t>>>",@_ if $DEBUG }

## no critic (BuiltinFunctions::ProhibitStringyEval)

my $cmd = "print";
my $neg = 0;
my $str;
while (defined($str = <DATA>))
{
    chomp $str;
    if ($str =~ s/\A# USING://) { $neg = 0; $cmd = $str; next; }
    elsif ($str =~ /\A# TH[EI]SE? SHOULD FAIL/) { $neg = 1; next; }
    elsif (!$str || $str =~ /\A#/) { $neg = 0; next }
    my $orig_str = $str;
    $str =~ s/\\n/\n/g;
    debug "\tUsing: $cmd\n";
    debug "\t   on: [$str]\n";

    my @res;
    my $var = eval "\@res = $cmd";
    is $@, '', 'no error';
    debug "\t list got: [" . join("|", map {defined $_ ? $_ : '<undef>'} @res) . "]\n";
    debug "\t list left: [$str]\n";
    ($neg ? \&isnt : \&is)->(substr($str,pos($str)||0,1), ';', "$orig_str matched list");

    pos $str = 0;
    $var = eval $cmd;
    is $@, '', 'no error';
    $var = "<undef>" unless defined $var;
    debug "\t scalar got: [$var]\n";
    debug "\t scalar left: [$str]\n";
    ($neg ? \&unlike : \&like)->( $str, qr/\A;/, "$orig_str matched scalar");
}

my $grammar = <<'EOF';
given 2 { when __ < 1 { ok(0) } else { ok(1) } }
EOF
pos $grammar = 8;
my ($out) = Text::Balanced::_match_codeblock(\$grammar,qr/\s*/,qr/\{/,qr/\}/,qr/\{/,qr/\}/,undef);
ok $out, 'Switch error from calling _match_codeblock';

$grammar = <<'EOF';
comment:  m/a/
enum_list: (/b/)
EOF
pos $grammar = 10;
($out) = Text::Balanced::extract_quotelike($grammar);
is $out, 'm/a/', 'PRD error (setup for real error)';
pos $grammar = 26;
($out) = extract_codeblock($grammar,'{([',undef,'(',1);
is $out, '(/b/)', 'PRD error';

done_testing;

__DATA__

# USING: extract_codeblock($str,'(){}',undef,'()');
(Foo(')'));

# USING: extract_codeblock($str);
{ $data[4] =~ /['"]/; };
{1<<2};
{1<<2};\n
{1<<2};\n\n
{ $a = /\}/; };
{ sub { $_[0] /= $_[1] } };  # / here
{ 1; };
{ $a = 1; };

# USING: extract_codeblock($str,'<>');
< %x = ( try => "this") >;
< %x = () >;
< %x = ( $try->{this}, "too") >;
< %'x = ( $try->{this}, "too") >;
< %'x'y = ( $try->{this}, "too") >;
< %::x::y = ( $try->{this}, "too") >;

# THIS SHOULD FAIL
< %x = do { $try > 10 } >;

# USING: extract_codeblock($str, '()');
(($x || 2)); split /z/, $y
(($x // 2)); split /z/, $y

# USING: extract_codeblock($str,undef,'=*');
========{$a=1};

# USING: extract_codeblock($str,'{}<>');
< %x = do { $try > 10 } >;

# USING: extract_codeblock($str,'{}',undef,'<>');
< %x = do { $try > 10 } >;

# USING: extract_codeblock($str,'{}');
{ $a = $b; # what's this doing here? \n };'
{ $a = $b; \n $a =~ /$b/; \n @a = map /\s/ @b };

# THIS SHOULD FAIL
{ $a = $b; # what's this doing here? };'
{ $a = $b; # what's this doing here? ;'
