# Run the appropriate tests from https://github.com/ingydotnet/yaml-spec-tml
use strict;
use warnings;
use lib 't/lib';
use Test::More 0.88;
use SubtestCompat;
use TestBridge;
use TestUtils;

my $JSON = json_class()
    or Test::More::plan skip_all => 'no JSON backends available!?';

diag 'using JSON backend: ' . $JSON . ' ' . $JSON->VERSION
    if not $ENV{PERL_CORE};

# Each spec test will need a different bridge and arguments:
my @spec_tests = (
    ['t/tml-spec/basic-data.tml', 'test_yaml_json', $JSON],
    # This test is currently failing massively. We use LAST to only run what is
    # covered so far.
    ['t/tml-spec/unicode.tml', 'test_code_point'],  # uses JSON::PP
);

for my $test (@spec_tests) {
    my ($file, $bridge, @args) = @$test;
    my $code = sub {
        my ($file, $blocks) = @_;
        subtest "YAML Spec Test; file: $file" => sub {
            my $func = \&{$bridge};
            $func->($_) for @$blocks;
        };
    };
    run_testml_file($file, $code, @args);
}

done_testing;
