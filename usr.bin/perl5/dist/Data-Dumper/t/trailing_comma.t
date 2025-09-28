#!./perl -w
# t/trailing_comma.t - Test TrailingComma()

use strict;
use warnings;

use Data::Dumper;
use Test::More;
use lib qw( ./t/lib );
use Testing qw( _dumptostr );

my @cases = ({
    input  => [],
    output => "[]",
    desc   => 'empty array',
}, {
    input  => [17],
    output => "[17]",
    desc   => 'single-element array, no indent',
    conf   => { Indent => 0 },
}, {
    input  => [17],
    output => "[\n  17,\n]",
    desc   => 'single-element array, indent=1',
    conf   => { Indent => 1 },
}, {
    input  => [17],
    output => "[\n          17,\n        ]",
    desc   => 'single-element array, indent=2',
    conf   => { Indent => 2 },
}, {
    input  => [17, 18],
    output => "[17,18]",
    desc   => 'two-element array, no indent',
    conf   => { Indent => 0 },
}, {
    input  => [17, 18],
    output => "[\n  17,\n  18,\n]",
    desc   => 'two-element array, indent=1',
    conf   => { Indent => 1 },
}, {
    input  => [17, 18],
    output => "[\n          17,\n          18,\n        ]",
    desc   => 'two-element array, indent=2',
    conf   => { Indent => 2 },
}, {
    input  => {},
    output => "{}",
    desc   => 'empty hash',
}, {
    input  => {foo => 17},
    output => "{'foo' => 17}",
    desc   => 'single-element hash, no indent',
    conf   => { Indent => 0 },
}, {
    input  => {foo => 17},
    output => "{\n  'foo' => 17,\n}",
    desc   => 'single-element hash, indent=1',
    conf   => { Indent => 1 },
}, {
    input  => {foo => 17},
    output => "{\n          'foo' => 17,\n        }",
    desc   => 'single-element hash, indent=2',
    conf   => { Indent => 2 },
}, {
    input  => {foo => 17, quux => 18},
    output => "{'foo' => 17,'quux' => 18}",
    desc   => 'two-element hash, no indent',
    conf   => { Indent => 0 },
}, {
    input  => {foo => 17, quux => 18},
    output => "{\n  'foo' => 17,\n  'quux' => 18,\n}",
    desc   => 'two-element hash, indent=1',
    conf   => { Indent => 1 },
}, {
    input  => {foo => 17, quux => 18},
    output => "{\n          'foo' => 17,\n          'quux' => 18,\n        }",
    desc   => 'two-element hash, indent=2',
    conf   => { Indent => 2 },
});

my $xs_available = !$Data::Dumper::Useperl;
my $tests_per_case = $xs_available ? 2 : 1;

plan tests => $tests_per_case * @cases;

for my $case (@cases) {
    run_case($case, $xs_available ? 'XS' : 'PP');
    if ($xs_available) {
        local $Data::Dumper::Useperl = 1;
        run_case($case, 'PP');
    }
}

sub run_case {
    my ($case, $mode) = @_;
    my ($input, $output, $desc, $conf) = @$case{qw<input output desc conf>};
    my $obj = Data::Dumper->new([$input]);
    $obj->Trailingcomma(1);     # default to on for these tests
    $obj->Sortkeys(1);
    for my $k (sort keys %{ $conf || {} }) {
        $obj->$k($conf->{$k});
    }
    chomp(my $got = _dumptostr($obj));
    is($got, "\$VAR1 = $output;", "$desc (in $mode mode)");
}
