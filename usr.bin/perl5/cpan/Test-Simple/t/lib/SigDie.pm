package SigDie;

use strict;

our $DIE;
$SIG{__DIE__} = sub { $DIE = $@ };

1;
