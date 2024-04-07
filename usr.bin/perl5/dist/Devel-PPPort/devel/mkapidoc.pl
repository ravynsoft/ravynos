#!/usr/bin/perl

################################################################################
#
#  mkapidoc.pl -- generate apidoc.fnc from scanning the Perl source
#
# Should be called from the base directory for Devel::PPPort.
# If that happens to be in the /dist directory of a perl build structure, and
# you're doing the standard thing, no parameters are required.  Otherwise
# (again with the standard things, its single parameter is the base directory
# of the perl source tree to be used.
#
################################################################################
#
#  Version 3.x, Copyright (C) 2004-2013, Marcus Holland-Moritz.
#  Version 2.x, Copyright (C) 2001, Paul Marquess.
#  Version 1.x, Copyright (C) 1999, Kenneth Albanowski.
#
#  This program is free software; you can redistribute it and/or
#  modify it under the same terms as Perl itself.
#
################################################################################

use warnings;
use strict;
use File::Find;
use re '/aa';

my $PERLROOT = $ARGV[0];
unless ($PERLROOT) {
    $PERLROOT = '../..';
    print STDERR "$0: perl directory root argument not specified. Assuming '$PERLROOT'\n";
}

die "'$PERLROOT' is invalid, or you haven't successfully run 'make' in it"
                                                unless -e "$PERLROOT/warnings.h";
my $maindir = '.';
require "$maindir/parts/ppptools.pl";

my %seen;

# Find the files in MANIFEST that are core, but not embed.fnc, nor .t's
my @files;
open(my $m, '<', "$PERLROOT/MANIFEST") || die "MANIFEST:$!";
while (<$m>) {                      # In embed.fnc,
    chomp;
    next if m! ^ embed \. fnc \t !x;
    next if m! ^ ( cpan | dist | t) / !x;
    next if m! [^\t]* \.t \t !x;
    s/\t.*//;
    push @files, "$PERLROOT/$_";
}
close $m or die "Can't close $m: $!";

# Here, we have the lists of doc files and root First, get the known macros
# and functions from embed.fnc, converting from an array into a hash (for
# convenience)
my %embeds;
my %apidoc;

foreach my $entry (parse_embed("$maindir/parts/embed.fnc")) {
    my $name = $entry->{'name'};
    my $cond = $entry->{'cond'};

    my $flags = join "", sort { lc $a cmp lc $b or $a cmp $b }
                                                    keys $entry->{flags}->%*;
    my @arg_pairs;
    foreach my $pair ($entry->{args}->@*) {
        push @arg_pairs, join " ", $pair->@*;
    }
    my $args = join "|", @arg_pairs;

    die "Multiple entries for $embeds{$name}{$cond}"
                                                if defined $embeds{$name}{$cond};

    # Save the embed.fnc entry
    $embeds{$name}{$cond} = "$flags|$entry->{'ret'}|$name|$args";
}


# Examine the SEE ALSO section of perlapi which should contain links to all
# the pods with apidoc entries in them.  Add them to the MANIFEST list.
my $file;

sub callback {
    return unless $_ eq $file;
    return if $_ eq 'config.h';   # We don't examine this one
    return if $_ eq 'perlintern.pod';   # We don't examine this one
    return if $File::Find::dir =~ / \/ ( cpan | dist | t ) \b /x;
    push @files, $File::Find::name;
}

open my $a, '<', "$PERLROOT/pod/perlapi.pod"
        or die "Can't open perlapi.pod ($PERLROOT needs to have been built): $!";
while (<$a>) {
    next unless / ^ =head1\ SEE\ ALSO /x;
    while (<$a>) {
        # The lines look like:
        # F<config.h>, L<perlintern>, L<perlapio>, L<perlcall>, L<perlclib>,
        last if /^=/;

        my @tags = split /, \s* | \s+ /x;  # Allow comma- or just space-separated

        foreach my $tag (@tags) {
            if ($tag =~ / ^ F< (.*) > $ /x) {
                $file = $1;
            }
            elsif ($tag =~ / ^ L< (.*) > $ /x) {
                $file = "$1.pod";
            }
            else {
                die "Unknown tag '$tag'";
            }

            find(\&callback, $PERLROOT);
        }
    }
}

my ($controlling_flags, $controlling_ret_type, $controlling_args);

# Look through all the files that potentially have apidoc entries
# These may be associated with embed.fnc, in which case we do nothing;
# otherwise, we output them to apidoc.fnc, potentially modified.
for my $file (@files) {

    $file =~ s/ \t .* //x;      # Trim all but first column
    open my $f, '<', "$file" or die "Can't open $file: $!";

    my $line;
    while (defined ($line = <$f>)) {
        chomp $line;
        next unless $line =~ /  ^ =for \s+ apidoc ( _item )? \s+
                               (?:
                                  (   [^|]*? )  # flags, backoff trailing
                                                # white space
                                  \s* \| \s*

                                  (   [^|]*? )  # return type

                                  \s* \| \s*

                               )?               # flags and ret type are all
                                                # or nothing

                               ( [^|]+? )       # name

                               \s*

                               (?:  \| \s* ( .* ) \s* )?    # optional args

                               $
                             /x;
        my $item = $1 // 0;
        my $flags = $2 // "";
        my $ret_type = $3 // "";
        my $name = $4;
        my $args = $5 // "";

        next unless $name;  # Not an apidoc line

        # If embed.fnc already contains this name, this better be an empty
        # entry, unless it has the M flag, meaning there is another macro
        # defined for it.
        if (defined $embeds{$name}) {
            my @conds = keys $embeds{$name}->%*;

            # If this is just the anchor for where the pod is in the source,
            # the entry is already fully in embed.fnc.
            if ("$flags$ret_type$args" eq "") {
                if (! $item) {
                    foreach my $cond (@conds) {
                        # For a plain apidoc entry, save the inputs, so as to apply them
                        # to any following apidoc_item lines.
                        ($controlling_flags, $controlling_ret_type, $controlling_args)
                            = $embeds{$name}{$cond} =~ / ( [^|]* ) \| ( [^|]* ) \| (?: [^|]* ) \| (.*) /x;
                        $controlling_flags =~ s/[iMpb]//g;
                        $controlling_flags .= 'm' unless $controlling_flags =~ /m/;
                        last;
                    }
                }
                next;
            }

            # And the only reason we should have something with other
            # information than what's in embed.fnc is if it is an M flag,
            # meaning there is an extra macro for this function, and this is
            # documenting that.
            my $msg;
            foreach my $cond (@conds) {
                if ($embeds{$name}{$cond} !~ / ^ [^|]* M /x) {
                    $msg = "Specify only name when main entry is in embed.fnc";
                    last;
                }
            }

            if (! defined $msg) {
                if ($flags !~ /m/) {
                    $msg = "Must have 'm' flag for overriding 'M' embed.fnc entry";
                }
                elsif ($flags =~ /p/) {
                    $msg = "Must not have 'p' flag for overriding 'M' embed.fnc entry";
                }
            }

            die "$msg: $file: $.: \n'$line'\n" if defined $msg;
        }

        # Here, we have an entry for apidoc.fnc, one that isn't in embed.fnc.

        # If this is an apidoc_item line, there was a plain apidoc line
        # earlier, and we saved the values from that to use here (if here is
        # empty).
        if ($item) {
            $flags = $controlling_flags unless $flags ne "";
            $ret_type = $controlling_ret_type unless $ret_type ne "";
            $args = $controlling_args unless $args ne "";
        }
        else {
            # For a plain apidoc entry, save the inputs, so as to apply them
            # to any following apidoc_item lines.
            $controlling_flags = $flags;
            $controlling_ret_type = $ret_type;
            $controlling_args = $args;
        }

        # Many of the entries omit the "d" flag to indicate they are
        # documented, but we got here because of an apidoc line, which
        # indicates it is documented in the source
        $flags .= 'd' unless $flags =~ /d/;

        # We currently don't handle typedefs, nor this special case
        next if $flags =~ /y/;
        next if $name eq 'JMPENV_PUSH';

        my $entry = "$flags|$ret_type|$name";
        $entry .= "|$args" if $args ne "";
        $apidoc{$name}{entry} = $entry;
    }
}

my $outfile = "$maindir/parts/apidoc.fnc";
open my $out, ">", $outfile
                        or die "Can't open '$outfile' for writing: $!";
require "$maindir/parts/inc/inctools";
print $out <<EOF;
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:
:  !!!! Do NOT edit this file directly! -- Edit devel/mkapidoc.sh instead. !!!!
:
:  This file was automatically generated from the API documentation scattered
:  all over the Perl source code. To learn more about how all this works,
:  please read the F<HACKERS> file that came with this distribution.
:
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:
: This file lists all API functions/macros that are documented in the Perl
: source code, but are not contained in F<embed.fnc>.
:
EOF
print $out join "\n", sort sort_api_lines map { $apidoc{$_}{entry} } keys %apidoc;
close $out or die "Close failed: $!";
print "$outfile regenerated\n";
