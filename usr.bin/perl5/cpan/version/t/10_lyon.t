#! perl

use Test::More qw/no_plan/;

use version;

# These values are from the Lyon consensus, as taken from
# https://gist.github.com/dagolden/9559280

ok('version'->new(1.0203) == 'version'->new('1.0203'));
ok('version'->new(1.02_03) == 'version'->new('1.02_03'));
ok('version'->new(v1.2.3) == 'version'->new('v1.2.3'));
if ($] >= 5.008_001) {
    ok('version'->new(v1.2.3_0) == 'version'->new('v1.2.3_0'));
}

cmp_ok('version'->new(1.0203), '==', 'version'->new('1.0203'));
cmp_ok('version'->new(1.02_03), '==', 'version'->new('1.02_03'));
cmp_ok('version'->new(v1.2.3), '==', 'version'->new('v1.2.3'));
if ($] >= 5.008_001) {
    cmp_ok('version'->new(v1.2.3_0), '==', 'version'->new('v1.2.3_0'));
}

cmp_ok('version'->new('1.0203')->numify, '==', '1.0203');
is('version'->new('1.0203')->normal, 'v1.20.300');

cmp_ok('version'->new('1.02_03')->numify, '==', '1.0203');
is('version'->new('1.02_03')->normal, 'v1.20.300');

cmp_ok('version'->new('v1.2.30')->numify, '==', '1.002030');
is('version'->new('v1.2.30')->normal, 'v1.2.30');
cmp_ok('version'->new('v1.2.3_0')->numify, '==', '1.002030');
is('version'->new('v1.2.3_0')->normal, 'v1.2.30');

is('version'->new("1.0203")->stringify, "1.0203");
is('version'->new("1.02_03")->stringify, "1.02_03");
is('version'->new("v1.2.30")->stringify, "v1.2.30");
is('version'->new("v1.2.3_0")->stringify, "v1.2.3_0");
is('version'->new(1.0203)->stringify, "1.0203");
is('version'->new(1.02_03)->stringify, "1.0203");
is('version'->new(v1.2.30)->stringify, "v1.2.30");
if ($] >= 5.008_001) {
    is('version'->new(v1.2.3_0)->stringify, "v1.2.30");
}
