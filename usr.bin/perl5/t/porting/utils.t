#!./perl -w

# What does this test?
# This checks that all the perl "utils" don't have embarrassing syntax errors
#
# Why do we test this?
# Right now, without this, it's possible to pass the all the regression tests
# even if one has introduced syntax errors into scripts such as installperl
# or installman. No tests fail, so it's fair game to push the commit.
# Obviously this breaks installing perl, but we won't spot this.
# Whilst we can't easily test that the various scripts *work*, we can at least
# check that we've not made any trivial screw ups.
#
# It's broken - how do I fix it?
# Presumably it's failed because some (other) code that you changed was (also)
# used by one of the utility scripts. So you'll have to manually test that
# script.

BEGIN {
    @INC = '..' if -f '../TestInit.pm';
}
use TestInit qw(T); # T is chdir to the top level
use strict;

require './t/test.pl';

# It turns out that, since the default @INC will include your old 5.x libs, if
# you have them, the Porting utils might load a library that no longer compiles
# clean.  This actually happened, with Local::Maketext::Lexicon from a 5.10.0
# preventing 5.16.0-RC0 from testing successfully.  This test is really only
# needed for porters, anyway.  -- rjbs, 2012-05-10
find_git_or_skip('all');

my @maybe;

open my $fh, '<', 'MANIFEST' or die "Can't open MANIFEST: $!";
while (<$fh>) {
    push @maybe, $1 if m!^(Porting/\S+)!;
}
close $fh or die $!;

open $fh, '<', 'utils.lst' or die "Can't open utils.lst: $!";
while (<$fh>) {
    die unless  m!^(\S+)!;
    push @maybe, $1;
    $maybe[$#maybe] .= '.com' if $^O eq 'VMS';
}
close $fh or die $!;

# regen/uconfig_h.pl is here because it's not possible to test it by running
# it on non-*nix platforms, as it requires a Bourne shell. As it's the only file
# in regen/ which we can syntax check but can't run, it's simpler to add it to
# the list here, than copy-paste the entire syntax-checking logic to
# t/porting/regen.t
my @victims = (qw(installman installperl regen_perly.pl regen/uconfig_h.pl));
my %excuses = (
               'Porting/git-deltatool' => 'Git::Wrapper',
               'Porting/podtidy' => 'Pod::Tidy',
               'Porting/leakfinder.pl' => 'XS::APItest',
              );

foreach (@maybe) {
    if (/\.p[lm]$/) {
        push @victims, $_;
    } else {
        open $fh, '<', $_ or die "Can't open '$_': $!";
        my $line = <$fh>;
        if ($line =~ m{^#!(?:\S*|/usr/bin/env\s+)perl}
	    || $^O eq 'VMS' && $line =~ m{^\$ perl}) {
            push @victims, $_;
        } else {
            print "# $_ isn't a Perl script\n";
        }
    }
}

printf "1..%d\n", scalar @victims;

# Does this perl have 64 bit integers?
my $has_64bit_ints = eval { pack "Q", 1 };

foreach my $victim (@victims) {
 SKIP: {
        skip ("$victim uses $excuses{$victim}, so can't test with just core modules")
            if $excuses{$victim};

        my $got = runperl(switches => ['-c'], progfile => $victim, stderr => 1, nolib => 1);

        # check to see if this script needs 64 bit integers.
        if (!$has_64bit_ints and $got =~ /requires 64 bit integers/) {
            skip("$victim requires 64 bit integers and this is a 32 bit Perl", 1);
        }

        is($got, "$victim syntax OK\n", "$victim compiles")
            or diag("when executing perl with '-c $victim'");
    }
}

# ex: set ts=8 sts=4 sw=4 et:
