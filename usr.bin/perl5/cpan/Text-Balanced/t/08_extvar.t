use 5.008001;

use strict;
use warnings;
use Test::More;
use Text::Balanced qw ( extract_variable );

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
    debug "\t list got: [" . join("|",map {defined $_ ? $_ : '<undef>'} @res) . "]\n";
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

my @res = extract_variable('${a}');
is $res[0], '${a}' or diag "error was: $@";

done_testing;

__DATA__

# USING: extract_variable($str);
# THESE SHOULD FAIL
$a->;
$a (1..3) { print $a };

# USING: extract_variable($str);
$::obj;
$obj->nextval;
*var;
*$var;
*{var};
*{$var};
*var{cat};
\&var;
\&mod::var;
\&mod'var;
$a;
$_;
$a[1];
$_[1];
$a{cat};
$_{cat};
$a->[1];
$a->{"cat"}[1];
@$listref;
@{$listref};
$obj->nextval;
$obj->_nextval;
$obj->next_val_;
@{$obj->nextval};
@{$obj->nextval($cat,$dog)->{new}};
@{$obj->nextval($cat?$dog:$fish)->{new}};
@{$obj->nextval(cat()?$dog:$fish)->{new}};
$ a {'cat'};
$a::b::c{d}->{$e->()};
$a'b'c'd{e}->{$e->()};
$a'b::c'd{e}->{$e->()};
$#_;
$#array;
$#{array};
$var[$#var];
$1;
$11;
$&;
$`;
$';
$+;
$*;
$.;
$/;
$|;
$,;
$";
$;;
$#;
$%;
$=;
$-;
$~;
$^;
$:;
$^L;
$^A;
$?;
$!;
$^E;
$@;
$$;
$<;
$>;
$(;
$);
$[;
$];
$^C;
$^D;
$^F;
$^H;
$^I;
$^M;
$^O;
$^P;
$^R;
$^S;
$^T;
$^V;
$^W;
${^WARNING_BITS};
${^WIDE_SYSTEM_CALLS};
$^X;

# THESE SHOULD FAIL
$a->;
@{$;
$ a :: b :: c
$ a ' b ' c

# USING: extract_variable($str,'=*');
========$a;
