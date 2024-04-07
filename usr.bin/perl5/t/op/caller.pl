# tests shared between t/op/caller.t and ext/XS-APItest/t/op.t

use strict;
use warnings;

sub dooot {
    is(hint_fetch('dooot'), undef);
    is(hint_fetch('thikoosh'), undef);
    ok(!hint_exists('dooot'));
    ok(!hint_exists('thikoosh'));
    if ($::testing_caller) {
	is(hint_fetch('dooot', 1), 54);
    }
    BEGIN {
	$^H{dooot} = 42;
    }
    is(hint_fetch('dooot'), 6 * 7);
    if ($::testing_caller) {
	is(hint_fetch('dooot', 1), 54);
    }

    BEGIN {
	$^H{dooot} = undef;
    }
    is(hint_fetch('dooot'), undef);
    ok(hint_exists('dooot'));

    BEGIN {
	delete $^H{dooot};
    }
    is(hint_fetch('dooot'), undef);
    ok(!hint_exists('dooot'));
    if ($::testing_caller) {
	is(hint_fetch('dooot', 1), 54);
    }
}
{
    is(hint_fetch('dooot'), undef);
    is(hint_fetch('thikoosh'), undef);
    BEGIN {
	$^H{dooot} = 1;
	$^H{thikoosh} = "SKREECH";
    }
    if ($::testing_caller) {
	is(hint_fetch('dooot'), 1);
    }
    is(hint_fetch('thikoosh'), "SKREECH");

    BEGIN {
	$^H{dooot} = 42;
    }
    {
	{
	    BEGIN {
		$^H{dooot} = 6 * 9;
	    }
	    is(hint_fetch('dooot'), 54);
	    is(hint_fetch('thikoosh'), "SKREECH");
	    {
		BEGIN {
		    delete $^H{dooot};
		}
		is(hint_fetch('dooot'), undef);
		ok(!hint_exists('dooot'));
		is(hint_fetch('thikoosh'), "SKREECH");
	    }
	    dooot();
	}
	is(hint_fetch('dooot'), 6 * 7);
	is(hint_fetch('thikoosh'), "SKREECH");
    }
    is(hint_fetch('dooot'), 6 * 7);
    is(hint_fetch('thikoosh'), "SKREECH");
}

print "# which now works inside evals\n";

{
    BEGIN {
	$^H{dooot} = 42;
    }
    is(hint_fetch('dooot'), 6 * 7);

    eval "is(hint_fetch('dooot'), 6 * 7); 1" or die $@;

    eval <<'EOE' or die $@;
    is(hint_fetch('dooot'), 6 * 7);
    eval "is(hint_fetch('dooot'), 6 * 7); 1" or die $@;
    BEGIN {
	$^H{dooot} = 54;
    }
    is(hint_fetch('dooot'), 54);
    eval "is(hint_fetch('dooot'), 54); 1" or die $@;
    eval 'BEGIN { $^H{dooot} = -1; }; 1' or die $@;
    is(hint_fetch('dooot'), 54);
    eval "is(hint_fetch('dooot'), 54); 1" or die $@;
EOE
}

{
    BEGIN {
	$^H{dooot} = "FIP\0FOP\0FIDDIT\0FAP";
    }
    is(hint_fetch('dooot'), "FIP\0FOP\0FIDDIT\0FAP", "Can do embedded 0 bytes");

    BEGIN {
	$^H{dooot} = chr 256;
    }
    is(hint_fetch('dooot'), chr 256, "Can do Unicode");

    BEGIN {
	$^H{dooot} = -42;
    }
    is(hint_fetch('dooot'), -42, "Can do IVs");

    BEGIN {
	$^H{dooot} = ~0;
    }
    cmp_ok(hint_fetch('dooot'), '>', 42, "Can do UVs");
}

{
    my ($k1, $k2, $k3, $k4);
    BEGIN {
	$k1 = chr 163;
	$k2 = $k1;
	$k3 = chr 256;
	$k4 = $k3;
	utf8::upgrade $k2;
	utf8::encode $k4;

	$^H{$k1} = 1;
	$^H{$k2} = 2;
	$^H{$k3} = 3;
	$^H{$k4} = 4;
    }

	
    is(hint_fetch($k1), 2, "UTF-8 or not, it's the same");
    if ($::testing_caller) {
	# Perl_refcounted_he_fetch() insists that you have the key correctly
	# normalised for the way hashes store them. As this one isn't
	# normalised down to bytes, it won't work with
	# Perl_refcounted_he_fetch()
	is(hint_fetch($k2), 2, "UTF-8 or not, it's the same");
    }
    is(hint_fetch($k3), 3, "Octect sequences and UTF-8 are distinct");
    is(hint_fetch($k4), 4, "Octect sequences and UTF-8 are distinct");
}

{
    my ($k1, $k2, $k3);
    BEGIN {
	($k1, $k2, $k3) = ("\0", "\0\0", "\0\0\0");
	$^H{$k1} = 1;
	$^H{$k2} = 2;
	$^H{$k3} = 3;
    }

    is(hint_fetch($k1), 1, "Keys with the same hash value don't clash");
    is(hint_fetch($k2), 2, "Keys with the same hash value don't clash");
    is(hint_fetch($k3), 3, "Keys with the same hash value don't clash");

    BEGIN {
	$^H{$k1} = "a";
	$^H{$k2} = "b";
	$^H{$k3} = "c";
    }

    is(hint_fetch($k1), "a", "Keys with the same hash value don't clash");
    is(hint_fetch($k2), "b", "Keys with the same hash value don't clash");
    is(hint_fetch($k3), "c", "Keys with the same hash value don't clash");
}

1;
