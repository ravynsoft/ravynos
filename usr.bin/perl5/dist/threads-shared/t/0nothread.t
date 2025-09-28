use strict;
use warnings;

use Test::More (tests => 53);

### Start of Testing ###

my @array;
my %hash;

sub hash
{
    my @val = @_;
    is(keys %hash, 0, "hash empty");
    $hash{0} = $val[0];
    is(keys %hash,1, "Assign grows hash");
    is($hash{0},$val[0],"Value correct");
    $hash{2} = $val[2];
    is(keys %hash,2, "Assign grows hash");
    is($hash{0},$val[0],"Value correct");
    is($hash{2},$val[2],"Value correct");
    $hash{1} = $val[1];
    is(keys %hash,3,"Size correct");
    my @keys = keys %hash;
    is(join(',',sort @keys),'0,1,2',"Keys correct");
    my @hval = @hash{0,1,2};
    is(join(',',@hval),join(',',@val),"Values correct");
    my $val = delete $hash{1};
    is($val,$val[1],"Delete value correct");
    is(keys %hash,2,"Size correct");
    while (my ($k,$v) = each %hash) {
        is($v,$val[$k],"each works");
    }
    %hash = ();
    is(keys %hash,0,"Clear hash");
}

sub array
{
    my @val = @_;
    is(@array, 0, "array empty");
    $array[0] = $val[0];
    is(@array,1, "Assign grows array");
    is($array[0],$val[0],"Value correct");
    unshift(@array,$val[2]);
    is($array[0],$val[2],"Unshift worked");
    is($array[-1],$val[0],"-ve index");
    push(@array,$val[1]);
    is($array[-1],$val[1],"Push worked");
    is(@array,3,"Size correct");
    is(shift(@array),$val[2],"Shift worked");
    is(@array,2,"Size correct");
    is(pop(@array),$val[1],"Pop worked");
    is(@array,1,"Size correct");
    @array = ();
    is(@array,0,"Clear array");
}

ok((require threads::shared),"Require module");

if ($threads::shared::VERSION && ! $ENV{'PERL_CORE'}) {
    diag('Testing threads::shared ' . $threads::shared::VERSION);
}

array(24, [], 'Thing');
hash(24, [], 'Thing');

threads::shared->import();

share(\@array);
array(24, 42, 'Thing');

share(\%hash);
hash(24, 42, 'Thing');

exit(0);

# EOF
