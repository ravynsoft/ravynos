package Optionally::Deprecated;
use strict;

use if $] >=  5.011, 'deprecate';

q(Mostly harmless);

