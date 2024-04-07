# HP C-ANSI-C has problems in the optimizer for 5.8.x (not for 5.11.x)
# So drop to -O1 for Storable

use Config;

unless ($Config{gccversion}) {
    my $optimize = $Config{optimize};
    $optimize =~ s/(^| )[-+]O[2-9]( |$)/$1+O1$2/ and
	$self->{OPTIMIZE} = $optimize;
    }
