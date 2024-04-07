use strict;
use warnings;
use Test::More tests => 2;
use Module::Metadata;

# This test case tests about parsing pod after `__END__` token.

my $pm_info = Module::Metadata->new_from_file('t/lib/ENDPOD.pm', collect_pod => 1,);
is( $pm_info->name, 'ENDPOD', 'found default package' );
is(join(',', $pm_info->pod_inside), 'NAME');
