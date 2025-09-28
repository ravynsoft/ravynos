#!./perl

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require Config; import Config;
    if ($Config{'extensions'} !~ /\bFile\/Glob\b/i) {
        print "1..0\n";
        exit 0;
    }
}

use Test::More tests => 7;

BEGIN {
    use_ok('File::Glob', qw(:glob csh_glob));
}

my $pat = "op/G*.t";

import File::Glob ':nocase';
@a = csh_glob($pat);
cmp_ok(scalar @a, '>=', 8, 'use of the case sensitivity tags, via csh_glob()');

# This may fail on systems which are not case-PRESERVING
import File::Glob ':case';
@a = csh_glob($pat);
is(scalar @a, 0, 'None should be uppercase');

@a = bsd_glob($pat, GLOB_NOCASE);
cmp_ok(scalar @a, '>=', 3, 'explicit use of the GLOB_NOCASE flag');

# Test Win32 backslash nastiness...
SKIP: {
    skip 'Not Win32', 3 unless $^O eq 'MSWin32';

    @a = File::Glob::bsd_glob("op\\g*.t");
    cmp_ok(scalar @a, '>=', 8);
    mkdir "[]", 0;
    @a = File::Glob::bsd_glob("\\[\\]", GLOB_QUOTE);
    rmdir "[]";
    is(scalar @a, 1);
    @a = bsd_glob("op\\*", GLOB_QUOTE);
    isnt(scalar @a, 0);
}
