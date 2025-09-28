use strict;
use warnings;

package MyTest;

use Test::Builder;

my $Test = Test::Builder->new;

sub ok
{
	$Test->ok(@_);
}

1;
