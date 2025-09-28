package Dev::Null;

use strict;

sub TIEHANDLE { bless {}, shift }
sub PRINT { 1 }

1;
