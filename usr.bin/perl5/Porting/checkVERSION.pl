#!/usr/bin/perl -w

#
# Check the tree against missing VERSIONs.
#
# Originally by Larry Shatzer
#

use strict;
use File::Find;

find(
     sub {
	 return unless -f;
	 if (/\.pm$/ && $File::Find::name !~ m:/t/:) { # pm but not in a test
	     unless (parse_file($_)) {
		 print "$File::Find::name\n";
	     }
	 }
     }, @ARGV ? shift : ".");

sub parse_file {
    my $parsefile = shift;

    my $result;

    open(FH,'<',$parsefile) or warn "Could not open '$parsefile': $!";

    my $inpod = 0;
    while (<FH>) {
	$inpod = /^=(?!cut)/ ? 1 : /^=cut/ ? 0 : $inpod;
	next if $inpod || /^\s*\#/;
	chomp;
	next unless /([\$*])(([\w\:\']*)\bVERSION)\b.*\=/;
	my $eval = qq{
	    package ExtUtils::MakeMaker::_version;
	    no strict;
	    local $1$2;
	    \$$2=undef; do {
		$_
	    }; \$$2
	};
	no warnings;
	$result = eval($eval);
	warn "Could not eval '$eval' in $parsefile: $@" if $@;
	$result = "undef" unless defined $result;
	last;
    }
    close FH;
    return $result;
}

