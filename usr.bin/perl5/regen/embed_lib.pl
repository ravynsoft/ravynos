#!/usr/bin/perl -w
BEGIN {
    for $n (qw(lib regen)) {
        if (-e "../$n") {
            push @INC, "../$n";
        } elsif (-e "./$n") {
            push @INC, "./$n";
        }
    }
}
use strict;
use warnings;
use HeaderParser;

# read embed.fnc and regen/opcodes, needed by regen/embed.pl, makedef.pl,
# autodoc.pl and t/porting/diag.t

require 5.004;	# keep this compatible, an old perl is all we may have before
                # we build the new one

sub setup_embed {
    my $prefix = shift || '';
    my $parser= HeaderParser->new(
        pre_process_content => sub {
            my ($self,$line_data)= @_;
            # HeaderParser knows how to parse and normalize embed_fnc.
            # calling this here ensures sets up the embed subpacket.
            $self->tidy_embed_fnc_entry($line_data);
            my $embed= $line_data->{embed}
                or return;
        },
        post_process_grouped_content => sub {
            # sort the group content by name.
            @{$_[1]}=
                sort {
                    $a->{embed}{name} cmp $b->{embed}{name}
                } @{$_[1]};
        },
    )->read_file($prefix . 'embed.fnc');
    my $lines= $parser->lines();

    # add the opcode checker functions automatically.
    open my $in_fh, '<', $prefix . 'regen/opcodes' or die $!;
    {
        my %syms;

        my $line_num = 0;
        while (my $line= <$in_fh>) {
            $line_num++;
            chomp($line);
            next unless $line;
            next if $line=~/^#/;
            my $check = (split /\t+/, $line)[2];
            next if $syms{$check}++;

            # These are all indirectly referenced by globals.c.
            my $new= HeaderLine->new(
                cond => [["defined(PERL_IN_GLOBALS_C) || defined(PERL_IN_OP_C) || defined(PERL_IN_PEEP_C)"]],
                raw => "pR|OP *|$check|NN OP *o",
                line => "pR|OP *|$check|NN OP *o",
                type => "content",
                level => 1,
                source => 'regen/opcodes',
                start_line_num => $line_num,
            );
            $parser->tidy_embed_fnc_entry($new);
            push @$lines, $new;
        }
    }
    close $in_fh
        or die "Problem reading regen/opcodes: $!";

    # Cluster entries in embed.fnc that have the same #ifdef guards.
    # Also, split out at the top level the three classes of functions.
    # The result for each group_content() calls is an arrayref containing
    # HeaderLine objects, with the embed.fnc data prenormalized, and each
    # conditional clause containing a sorted list of functions, with
    # any further conditional clauses following.
    # Note this is a normalized and relatively smart grouping, and we can
    # handle if/elif and etc properly. At the cost of being a touch slow.

    return (
        $parser->group_content($lines,
                sub { $_[1]->{embed} }),                # everything
        $parser->group_content($lines,
                sub { $_[1]->{embed} &&
                      $_[1]->{embed}{flags}=~/[AC]/ }), # only API and private API
        $parser->group_content($lines,
                sub { $_[1]->{embed} &&
                      $_[1]->{embed}{flags}!~/[AC]/ &&  # otherwise Extensions
                      $_[1]->{embed}{flags}=~/[E]/ }),
        $parser->group_content($lines,
                sub { $_[1]->{embed} &&
                      $_[1]->{embed}{flags}!~/[AC]/ &&  # everything else.
                      $_[1]->{embed}{flags}!~/[E]/ }),
    );
}

1;
