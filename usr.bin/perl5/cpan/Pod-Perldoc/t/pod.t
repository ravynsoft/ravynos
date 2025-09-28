use strict;
use warnings;
use Test::More;

unless ( $ENV{RELEASE_TESTING} ) {
    plan skip_all => 'Pod tests not required for installation.';
}

eval "use Test::Pod 1.22";
plan skip_all => 'Test::Pod 1.22 or higher not installed.' if $@;
all_pod_files_ok();
