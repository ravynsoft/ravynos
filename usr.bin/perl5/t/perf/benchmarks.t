#!./perl
#
# Execute the various code snippets in t/perf/benchmarks
# to ensure that they are all syntactically correct

BEGIN {
    chdir 't';
    require './test.pl';
    @INC = ('.', '../lib');
}

use warnings;
use strict;


my $file = 'perf/benchmarks';
my $benchmark_array = do $file;
unless ($benchmark_array) {
    die "Error while parsing '$file': $@\n" if $@;
    die "Error while trying to read '$file': $!"
        unless defined $benchmark_array;
    die "Unknown error running '$file'\n";
}

die "'$file' did not return an array ref\n"
        unless ref $benchmark_array eq 'ARRAY';

die "Not an even number of key value pairs in '$file'\n"
        if @$benchmark_array % 2;

my %benchmarks;
while (@$benchmark_array) {
    my $key  = shift @$benchmark_array;
    my $hash = shift @$benchmark_array;
    die "Duplicate key '$key' in '$file'\n" if exists $benchmarks{$key};
    $benchmarks{$key} = $hash;
}

plan keys(%benchmarks) * 4;

# check the hash of hashes is minimally consistent in format

my %valid_keys = map { $_=> 1 } qw(desc setup code pre post compile);
my @required_keys = qw(code);

for my $token (sort keys %benchmarks) {
    like($token, qr/^[a-zA-Z](\w|::)+$/a, "$token: legal token");

    my @keys    = sort keys %{$benchmarks{$token}};
    my @invalid = grep !exists $valid_keys{$_}, @keys;
    ok(!@invalid, "$token: only valid keys present")
        or diag("saw these invalid keys: (@invalid)");

    my @missing = grep !exists $benchmarks{$token}{$_}, @required_keys;
    ok(!@missing, "$token: all required keys present")
        or diag("these keys are missing: (@missing)");
}

# check that each bit of code compiles and runs

for my $token (sort keys %benchmarks) {
    my $b = $benchmarks{$token};
    my $setup = $b->{setup} // '';
    my $pre   = $b->{pre}   // '';
    my $post  = $b->{post}  // '';
    my $code = "package $token; $setup; for (1..1) { $pre; $b->{code}; $post; } 1;";
    no warnings;
    no strict;
    ok(eval $code, "running $token")
        or do {
            diag("code:");
            diag($code);
            diag("gave:");
            diag($@);
        }
}


