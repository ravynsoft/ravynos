#!./perl

BEGIN {
    chdir 't' if -d 't';
    require "./test.pl";
}

plan(3);

fresh_perl_is('$_ = qq{OK\n}; print;', "OK\n", {},
              'print without arguments outputs $_');
fresh_perl_is('$_ = qq{OK\n}; print STDOUT;', "OK\n", {},
              'print with only a filehandle outputs $_');
SKIP: {
    skip_if_miniperl('no dynamic loading of PerlIO::scalar in miniperl');
    if ($::IS_ASCII) {
        fresh_perl_is(<<'EOF', "\xC1\xAF\xC1\xAF\xC1\xB0\xC1\xB3", {}, "print doesn't launder utf8 overlongs");
use strict;
use warnings;

no warnings 'utf8';

# These form overlong "oops"
open my $fh, "<:utf8", \"\xC1\xAF\xC1\xAF\xC1\xB0\xC1\xB3"
    or die "Could not open\n";
read($fh, my $s, 10) or die "Could not read\n";
print $s;
EOF
    }
    else {
        fresh_perl_is(<<'EOF', "\x76\x41\x76\x41\x77\x42\x77\x43", {}, "print doesn't launder utf8 overlongs");
use strict;
use warnings;

no warnings 'utf8';

# These form overlong "oops"
open my $fh, "<:utf8", \"\x76\x41\x76\x41\x77\x42\x77\x43"
    or die "Could not open\n";
read($fh, my $s, 10) or die "Could not read\n";
print $s;
EOF
    }
}
