#!perl -w

# We assume that TestInit has been used.

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    skip_all_if_miniperl();
}

use strict;
use warnings;

use Config;
use Data::Dumper;
$Data::Dumper::Sortkeys = 1;

# Windows doesn't seem to be able to test signals.
skip_all("Signals lock up tests on $^O") if $^O =~ /MSWin32/;

$| = 1;

# Extract the signals from %Config.
my @SIGNAMES = split /\s+/, $Config{sig_name};
my @SIGNUMS  = split /\s+/, $Config{sig_num};

my %SIG_MAP;
foreach my $i ( 0 .. ( scalar @SIGNAMES - 1 ) ) {
    $SIG_MAP{ $SIGNAMES[$i] } = $SIGNUMS[$i];
}

# Find the canonical (first) signal names.
my %CANONICAL_SIG;
my @duplicate_signals;
foreach my $sig (@SIGNAMES) {
    my $signum = $SIG_MAP{$sig};
    $CANONICAL_SIG{$signum} //= $sig;
    push @duplicate_signals, $sig if $CANONICAL_SIG{$signum} ne $sig;
}

plan tests => scalar @duplicate_signals * 5;
watchdog(25);

# Define the duplicate signal handlers.
my $sent = '';

sub handler_is {
    my $signame = shift;
    my $signum  = $SIG_MAP{$signame};

    my $canonical = $CANONICAL_SIG{$signum};

    is( $signame, $canonical, "Signal name for $sent is recieved as the canonical '$canonical' name." );

    return;
}

foreach my $dupe (@duplicate_signals) {
    my $canonical_name = $CANONICAL_SIG{ $SIG_MAP{$dupe} };
    note "Testing $dupe / $canonical_name signal pair";
    {
        local $SIG{$dupe} = \&handler_is;
        is( $SIG{$dupe}, $SIG{$canonical_name}, "Both handlers for $canonical_name/$dupe are set" );

        $sent = $dupe;
        kill $dupe, $$;

        $sent = $canonical_name;
        kill $canonical_name, $$;
    }

    is( $SIG{$dupe},           undef, "The signal $dupe is cleared after local goes out of scope." );
    is( $SIG{$canonical_name}, undef, "The signal $canonical_name is cleared after local goes out of scope." );
}

