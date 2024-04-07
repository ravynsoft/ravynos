#!/usr/bin/perl

use strict;
use warnings;
use Text::Tabs qw(expand unexpand);

=head1 NAME

Porting/manifest_lib.pl - functions for managing manifests

=head1 SYNOPSIS

    require './Porting/manifest_lib.pl';

=head1 DESCRIPTION

This file makes available one function, C<sort_manifest()>.

=head2 C<sort_manifest>

Treats its arguments as (chomped) lines from a MANIFEST file, and returns that
listed sorted appropriately.

=cut

# Try to get a sane sort. case insensitive, more or less
# sorted such that path components are compared independently,
# and so that lib/Foo/Bar sorts before lib/Foo-Alpha/Baz
# and so that lib/Foo/Bar.pm sorts before lib/Foo/Bar/Alpha.pm
# and so that configure and Configure sort together.
sub sort_manifest {
    my @lines = @_;

    # first we ensure that the descriptions for the files
    # are lined up reasonably.
    my %pfx_len;
    my @line_tuples;
    foreach my $idx (0 .. $#lines) {
        my $line = $lines[$idx];
        # clean up tab/space issues
        $line =~ s/\t[ ]+/\t/;
        if ($line =~ s/^(\S+)([ ]\s+)(\S+.*)/$1\t/) {
            my $descr = $2;
            $descr =~ s/\t+/ /g;
            $line .= $descr;
        }
        $line =~ s/\s+\z//;
        $line =~ /^(\S+)(?:\t+([^\t]*))?\z/
            or do {
                $line =~ s/\t/\\t/g;
                die "Malformed content in MANIFEST at line $idx: '$line'\n",
                    "Note: tabs have been encoded as \\t in this message.\n";
            };
        my ($file, $descr) = ($1, $2);
        my $pfx;
        if ($file =~ m!^((?:[^/]+/){1,2})!) {
            $pfx = $1;
        } else {
            $pfx = "";
        }
        #print "'$pfx': $file\n";
        push @line_tuples, [$pfx, $file, $descr];
        $pfx_len{$pfx} //= 40;

        # ensure we have at least one "space" (really tab)
        my $flen = 1 + length $file;
        $pfx_len{$pfx} = $flen
            if $pfx_len{$pfx} < $flen;
    }

    # round up to the next tab stop
    $_ % 8 and $_ += (8 - ($_ % 8)) for values %pfx_len;

    my @pretty_lines;
    foreach my $tuple (@line_tuples) {
        my ($pfx, $file, $descr) = @$tuple;
        my $str = sprintf "%*s", -$pfx_len{$pfx}, $file;
        ($str) = unexpand($str);
        # I do not understand why this is necessary. Bug in unexpand()?
        # See https://github.com/ap/Text-Tabs/issues/5
        $str =~ s/[ ]+/\t/;
        if ($descr) {
            $str =~ s/\t?\z/\t/;
            $str .= $descr;
        }
        $str =~ s/\s+\z//;
        push @pretty_lines, $str;
    }

    @pretty_lines =
    # case insensitive sorting of directory components independently.
    map { $_->[0] } # extract the full line
    sort {
        $a->[2] cmp $b->[2] || # sort by the first directory
        $a->[1] cmp $b->[1] || # sort in order of munged filename
        $a->[0] cmp $b->[0]    # then by the exact text in full line
    }
    map {
        # split out the filename and the description
        my ($f) = split /\s+/, $_, 2;
        # extract out the first directory
        my $d = $f=~m!^(\w+/)! ? lc $1 : "";
        # lc the filename so Configure and configure sort together in the list
        my $m= lc $f; # $m for munged
        # replace slashes by nulls, this makes short directory names sort before
        # longer ones, such as "foo/" sorting before "foo-bar/"
        $m =~ s!/!\0!g;
        # replace the extension (only one) by null null extension.
        # this puts any foo/blah.ext before any files in foo/blah/
        $m =~ s{(?<!\A)(\.[^.]+\z)}{\0\0$1};

        # return the original string, and the munged filename, and root dir
        [ $_, $m, $d ];
    } @pretty_lines;

    return @pretty_lines;
}

1;

# ex: set ts=8 sts=4 sw=4 et:
