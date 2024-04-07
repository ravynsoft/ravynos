#!./perl

BEGIN {
    require Config; import Config;
    if ($Config{'extensions'} !~ /\bOpcode\b/ && $Config{'osname'} ne 'VMS') {
        print "1..0\n";
        exit 0;
    }
}

use Test::More tests => 2;

eval <<'EOP';
	no ops 'fileno';
	$a = fileno STDIN;
EOP

like($@, qr/trapped/, 'equiv to "perl -M-ops=fileno"');

eval <<'EOP';
	use ops ':default';
	eval 1;
EOP

like($@, qr/trapped/,  'equiv to "perl -Mops=:default"');
