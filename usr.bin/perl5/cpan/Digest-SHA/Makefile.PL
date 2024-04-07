require 5.003000;

use strict;
use ExtUtils::MakeMaker;
use Getopt::Std;
use Config qw(%Config);

use vars qw($opt_t $opt_w $opt_x);

my $PM = 'lib/Digest/SHA.pm';
my $SHASUM = 'shasum';

	# 'use warnings' if possible, but stay portable

my($use_warnings, @EDITs);
my $warn_1 = 'BEGIN { $^W = 1 }';
my $warn_0 = 'BEGIN { $^W = 0 }';
{
	eval "require warnings; import warnings";
	$use_warnings = $@ ? 0 : 1;

	local(@ARGV) = ($PM, $SHASUM);
	while (<>) {
		if (
		(!$use_warnings && /^(use|no) warnings\b/) ||
		( $use_warnings && /^\Q$warn_1\E # use warnings\b/) ||
		( $use_warnings && /^\Q$warn_0\E # no warnings\b/)) {
			push @EDITs, $ARGV;
			close ARGV;
		}
	}
}

if (@EDITs) {
	local($^I, @ARGV) = ('', @EDITs);
	while (<>) {
		if ($use_warnings) {
			s/^\Q$warn_1\E # (.*)$/$1/;
			s/^\Q$warn_0\E # (.*)$/$1/;
		}
		else {
			s/^(use warnings\b.*)$/$warn_1 # $1/;
			s/^(no warnings\b.*)$/$warn_0 # $1/;
		}
		print;
	}
}

getopts('twx');	# -t is no longer used, but allow it anyway

my @defines;
push(@defines, '-DNO_SHA_384_512')  if $opt_x;
my $define = join(' ', @defines);

	# Workaround for DEC compiler bug, adapted from Digest::MD5

my @extra = ();
if ($^O eq 'VMS') {
	if (defined($Config{ccname})) {
		if (grep(/VMS_VAX/, @INC) && ($Config{ccname} eq 'DEC')) {
			# VAX optimizer, even up to v6.4, gets stuck
			push(@extra, OPTIMIZE => "/Optimize=(NODISJOINT)");
		}
	}
}

	# Enhance performance on Intel when using gcc

if ($Config{archname} =~ /^i[3456]86/ && $Config{ccname} eq 'gcc') {
	push(@extra, OPTIMIZE => '-O1 -fomit-frame-pointer');
}

my $fussy = '-Wall -Wextra -Wconversion -Wcast-align -Wpointer-arith ';
push(@extra, CCFLAGS => $fussy . $Config{ccflags}) if $opt_w;

my %attr = (
	'NAME'		=> 'Digest::SHA',
	'VERSION_FROM'	=> $PM,
	'LIBS'		=> [''],
	'DEFINE'	=> $define,
	'INC'		=> '-I.',
	'EXE_FILES'	=> [ $SHASUM ],
	'INSTALLDIRS'	=> ($] >= 5.010 and $] < 5.011) ? 'perl' : 'site',
	@extra,
);

my $MMversion = $ExtUtils::MakeMaker::VERSION || '0.00_00';
$attr{NO_META} = 1 if $MMversion ge '6.10_03';

WriteMakefile(%attr);
