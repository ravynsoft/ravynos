#!./perl -w
# t/indent.t - Test Indent()

use strict;
use warnings;

use Data::Dumper;
use Test::More tests => 9;
use lib qw( ./t/lib );
use Testing qw( _dumptostr );


my $hash = { foo => 42 };

my (%dumpstr);
my $dumper;

$dumper = Data::Dumper->new([$hash]);
$dumpstr{noindent} = _dumptostr($dumper);
# $VAR1 = {
#           'foo' => 42
#         };

$dumper = Data::Dumper->new([$hash]);
$dumper->Indent();
$dumpstr{indent_no_arg} = _dumptostr($dumper);

$dumper = Data::Dumper->new([$hash]);
$dumper->Indent(0);
$dumpstr{indent_0} = _dumptostr($dumper);
# $VAR1 = {'foo' => 42}; # no newline

$dumper = Data::Dumper->new([$hash]);
$dumper->Indent(1);
$dumpstr{indent_1} = _dumptostr($dumper);
# $VAR1 = {
#   'foo' => 42
# };

$dumper = Data::Dumper->new([$hash]);
$dumper->Indent(2);
$dumpstr{indent_2} = _dumptostr($dumper);
# $VAR1 = {
#           'foo' => 42
#         };

is($dumpstr{noindent}, $dumpstr{indent_no_arg},
    "absence of Indent is same as Indent()");
isnt($dumpstr{noindent}, $dumpstr{indent_0},
    "absence of Indent is different from Indent(0)");
isnt($dumpstr{indent_0}, $dumpstr{indent_1},
    "Indent(0) is different from Indent(1)");
cmp_ok(length($dumpstr{indent_0}), '<=', length($dumpstr{indent_1}),
    "Indent(0) is more compact than Indent(1)");
is($dumpstr{noindent}, $dumpstr{indent_2},
    "absence of Indent is same as Indent(2), i.e., 2 is default");
cmp_ok(length($dumpstr{indent_1}), '<=', length($dumpstr{indent_2}),
    "Indent(1) is more compact than Indent(2)");

my $array = [ qw| foo 42 | ];
$dumper = Data::Dumper->new([$array]);
$dumper->Indent(2);
$dumpstr{ar_indent_2} = _dumptostr($dumper);
# $VAR1 = [
#           'foo',
#           '42'
#         ];

$dumper = Data::Dumper->new([$array]);
$dumper->Indent(3);
$dumpstr{ar_indent_3} = _dumptostr($dumper);
# $VAR1 = [
#           #0
#           'foo',
#           #1
#           '42'
#         ];

isnt($dumpstr{ar_indent_2}, $dumpstr{ar_indent_3},
    "On arrays, Indent(2) is different from Indent(3)");
like($dumpstr{ar_indent_3},
    qr/\#0.+'foo'.+\#1.+42/s,
    "Indent(3) annotates array elements with their indices"
);
sub count_newlines { scalar $_[0] =~ tr/\n// }
{
    no if $] < 5.011, warnings => 'deprecated';
    is(count_newlines($dumpstr{ar_indent_2}) + 2,
        count_newlines($dumpstr{ar_indent_3}),
        "Indent(3) runs 2 lines longer than Indent(2)");
}

__END__
is($dumpstr{noindent}, $dumpstr{indent_0},
    "absence of Indent is same as Indent(0)");
isnt($dumpstr{noindent}, $dumpstr{indent_1},
    "absence of Indent is different from Indent(1)");
print STDERR $dumpstr{indent_0};
print STDERR $dumpstr{ar_indent_3};
