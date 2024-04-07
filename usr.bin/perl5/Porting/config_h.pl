#!/usr/bin/perl

# This script reorders config_h.SH after metaconfig
# Changing metaconfig is too complicated
#
# This script is run just after metaconfig, and it
# is run ONLY ONCE. Not to be used afterwards
#
# Copyright (C) 2005-2020 by H.Merijn Brand (m)'20 [23-08-2020]
#
# You may distribute under the terms of either the GNU General Public
# License or the Artistic License, as specified in the README file.

use strict;
use warnings;

my ($cSH, $ch, @ch, %ch) = ("config_h.SH");
open $ch, "<", $cSH or die "Cannot open $cSH: $!\n";
{   local $/ = "\n\n";
    @ch = <$ch>;
    close  $ch;
    }

sub ch_index {
    %ch = ();
    foreach my $ch (0 .. $#ch) {
	while ($ch[$ch] =~ m{^/\* ([A-Z]\w+)}gm) {
	    $ch{$1} = $ch;
	    }
	}
    } # ch_index

my %dep = (
    # This symbol must be defined BEFORE ...
    BYTEORDER		=> [ qw( UVSIZE				) ],
    LONGSIZE		=> [ qw( BYTEORDER			) ],
    MULTIARCH		=> [ qw( BYTEORDER MEM_ALIGNBYTES	) ],
    HAS_QUAD		=> [ qw( I64TYPE			) ],
    HAS_GETGROUPS	=> [ qw( Groups_t			) ],
    HAS_SETGROUPS	=> [ qw( Groups_t			) ],
    );

my $changed;
do {
    $changed = 0;
    foreach my $sym (keys %dep) {
	ch_index ();
	foreach my $dep (@{$dep{$sym}}) {
	    print STDERR "Check if $sym\t($ch{$sym}) precedes $dep\t($ch{$dep})\n";
	    $ch{$sym} < $ch{$dep} and next;
	    my $ch = splice @ch, $ch{$sym}, 1;
	    splice @ch, $ch{$dep}, 0, $ch;
	    $changed++;
	    ch_index ();
	    }
	}
    } while ($changed);

# 30327
for (grep m{echo .Extracting \$CONFIG_H} => @ch) {
    my $case = join "\n",
	qq{case "\$CONFIG_H" in},
	qq{already-done) echo "Not re-extracting config.h" ;;},
	qq{*)}, "";
    s{^(?=echo .Extracting)}{$case}m;
    }

unless ($ch[0] =~ m/THIS IS A GENERATED FILE/) {
    unshift @ch, join "\n" =>
	"#!/bin/sh",
	"#",
	"# THIS IS A GENERATED FILE",
	"# DO NOT HAND-EDIT",
	"#",
	"# See Porting/config_h.pl",
	"",
	"";
    push @ch, ";;\nesac\n";
    }

s/^(\s*)#(\s*)define\t\s*/${1}#${2}define /gm for @ch;

open  $ch, ">", $cSH or die "Cannot write $cSH: $!\n";
print $ch @ch;
close $ch;
