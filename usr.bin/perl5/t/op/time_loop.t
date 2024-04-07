#!perl -w

# d95a2ea538e6c332f36c34ca45b78d6ad93c3a1f allowed times greater than
# 2**63 to be handed to gm/localtime() which caused an internal overflow
# and an excessively long loop.  Test this does not happen.

use strict;

BEGIN { require './test.pl'; }

plan tests => 2;
watchdog(2);

local $SIG{__WARN__} = sub {};
is gmtime(2**69),    undef;
is localtime(2**69), undef;
