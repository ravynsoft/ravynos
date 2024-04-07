use strict;
use warnings;
use lib 't/lib/';
use Test::More 0.88;
use TestBridge;

run_all_testml_files(
    "Real-world examples", 't/tml-world', \&test_yaml_roundtrip
);

done_testing;
