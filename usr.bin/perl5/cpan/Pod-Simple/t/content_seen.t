BEGIN {
    if($ENV{PERL_CORE}) {
        chdir 't';
        @INC = '../lib';
    }
}

use strict;
use warnings;
use Test;
BEGIN { plan tests => 2 };

use Pod::Simple::Text;

my $p = Pod::Simple::Text->new();
$p->parse_string_document('dm+aSxLl7V3VUJFIe6CFDU13zhZ3yvjIuVkp6l//ZHcDcX014vnnh3FoElI92kFB
JGFU23Vga5Tfz0Epybwio9dq1gzrZ/PIcil2MnEcUWSrIStriv4hAbf0MXcNRHOM
oOV7xKU=
=y6KV
-----END PGP PUBLIC KEY BLOCK-----};

print $key;
exit;
');

# The =y6KV should not make this appear to be pod
ok ! $p->content_seen;

my $q = Pod::Simple::Text->new();
$q->parse_string_document('=head1 yes this is pod

And this fills it in
');

ok $q->content_seen;
