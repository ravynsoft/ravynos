#!perl -w

use TestInit qw(T);
use strict;
use Config;

require './t/test.pl';

skip_all("Code to read symbols not ported to $^O")
    if $^O eq 'VMS' or $^O eq 'MSWin32';

# Not investigated *why* we don't export these, but we don't, and we've not
# received any bug reports about it causing problems:
my %skip = map { ("PL_$_", 1) }
    qw(
	  DBcv bitcount cshname generation lastgotoprobe
	  mod_latin1_uc modcount no_symref_sv uudmap
	  watchaddr watchok warn_uninit_sv hash_chars
     );

$skip{PL_hash_rand_bits}= $skip{PL_hash_rand_bits_enabled}= 1; # we can be compiled without these, so skip testing them
$skip{PL_warn_locale}= 1; # we can be compiled without locales, so skip testing them


my $trial = "$Config{nm} globals$Config{_o} 2>&1";
my $yes = `$trial`;

skip_all("Could not run `$trial`") if $?;

my $defined = $^O eq "hpux" ? qr/\|/ : qr/^[0-9a-fA-F]{8,16}\s+[^Uu]\s+_?/m;

skip_all("Could not spot definition of PL_Yes in output of `$trial`")
    unless $yes =~ /${defined}PL_Yes/m;

my %exported;
open my $fh, '-|', $^X, '-Ilib', './makedef.pl', 'PLATFORM=test'
    or die "Can't run makedef.pl";

while (<$fh>) {
    next unless /^PL_/;
    chomp;
    ++$exported{$_};
}

close $fh or die "Problem running makedef.pl";

my %unexported;

foreach my $file (map {$_ . $Config{_o}} qw(globals regcomp)) {
    open $fh, '-|', $Config{nm}, $file
	or die "Can't run nm $file";

    while (<$fh>) {
	next unless /$defined(PL_\S+)/;
	if (delete $exported{$1}) {
	    note("Seen definition of $1");
	    next;
	}
	++$unexported{$1};
    }
    close $fh or die "Problem running nm $file";
}

unless ($Config{d_double_has_inf}) {
    $skip{PL_inf}++;
}
unless ($Config{d_double_has_nan}) {
    $skip{PL_nan}++;
}

foreach (sort keys %exported) {
 SKIP: {
    skip("We dont't export '$_' (Perl not built with this enabled?)",1) if $skip{$_};
    fail("Attempting to export '$_' which is never defined");
 }
}

$::TODO = $::TODO; # silence uninitialized warnings
foreach (sort keys %unexported) {
 SKIP: {
        skip("We don't export '$_'", 1) if $skip{$_};
        TODO: {
            local $::TODO = "HPUX exports everything" if $^O eq "hpux";
            fail("'$_' is defined, but we do not export it");
        }
    }
}

done_testing();
