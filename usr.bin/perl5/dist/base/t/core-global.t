#!/usr/bin/perl -w

use strict;
use Test::More tests => 1;

BEGIN { *CORE::GLOBAL::require = sub { require $_[0] }; }

{
    # [perl #121196]
    {
        package RequireOverride;
        sub zzz {}
    }
    ok(eval <<'EOS', "handle require overrides")
package RequireOverrideB;
use base 'RequireOverride';
1
EOS
        or diag $@;
}
