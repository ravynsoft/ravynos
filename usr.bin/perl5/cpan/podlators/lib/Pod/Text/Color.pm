# Convert POD data to formatted color ASCII text
#
# This is just a basic proof of concept.  It should later be modified to make
# better use of color, take options changing what colors are used for what
# text, and the like.
#
# SPDX-License-Identifier: GPL-1.0-or-later OR Artistic-1.0-Perl

##############################################################################
# Modules and declarations
##############################################################################

package Pod::Text::Color;

use 5.010;
use strict;
use warnings;

use Pod::Text ();
use Term::ANSIColor qw(color colored);

our @ISA = qw(Pod::Text);
our $VERSION = '5.01';

##############################################################################
# Overrides
##############################################################################

# Make level one headings bold.
sub cmd_head1 {
    my ($self, $attrs, $text) = @_;
    $text =~ s/\s+$//;
    local $Term::ANSIColor::EACHLINE = "\n";
    $self->SUPER::cmd_head1 ($attrs, colored ($text, 'bold'));
}

# Make level two headings bold.
sub cmd_head2 {
    my ($self, $attrs, $text) = @_;
    $text =~ s/\s+$//;
    $self->SUPER::cmd_head2 ($attrs, colored ($text, 'bold'));
}

# Fix the various formatting codes.
sub cmd_b { return colored ($_[2], 'bold')   }
sub cmd_f { return colored ($_[2], 'cyan')   }
sub cmd_i { return colored ($_[2], 'yellow') }

# Analyze a single line and return any formatting codes in effect at the end
# of that line.
sub end_format {
    my ($self, $line) = @_;
    my $reset = color ('reset');
    my $current;
    while ($line =~ /(\e\[[\d;]+m)/g) {
        my $code = $1;
        if ($code eq $reset) {
            undef $current;
        } else {
            $current .= $code;
        }
    }
    return $current;
}

# Output any included code in green.
sub output_code {
    my ($self, $code) = @_;
    local $Term::ANSIColor::EACHLINE = "\n";
    $code = colored ($code, 'green');
    $self->output ($code);
}

# Strip all of the formatting from a provided string, returning the stripped
# version.  We will eventually want to use colorstrip() from Term::ANSIColor,
# but it's fairly new so avoid the tight dependency.
sub strip_format {
    my ($self, $text) = @_;
    $text =~ s/\e\[[\d;]*m//g;
    return $text;
}

# We unfortunately have to override the wrapping code here, since the normal
# wrapping code gets really confused by all the escape sequences.
sub wrap {
    my $self = shift;
    local $_ = shift;
    my $output = '';
    my $spaces = ' ' x $$self{MARGIN};
    my $width = $$self{opt_width} - $$self{MARGIN};

    # $codes matches a single special sequence.  $char matches any number of
    # special sequences preceding a single character other than a newline.
    # $shortchar matches some sequence of $char ending in codes followed by
    # whitespace or the end of the string.  $longchar matches exactly $width
    # $chars, used when we have to truncate and hard wrap.
    my $code = '(?:\e\[[\d;]+m)';
    my $char = "(?>$code*[^\\n])";
    my $shortchar = '^(' . $char . "{0,$width}(?>$code*)" . ')(?:\s+|\z)';
    my $longchar = '^(' . $char . "{$width})";
    while (length > $width) {
        if (s/$shortchar// || s/$longchar//) {
            $output .= $spaces . $1 . "\n";
        } else {
            last;
        }
    }
    $output .= $spaces . $_;

    # less -R always resets terminal attributes at the end of each line, so we
    # need to clear attributes at the end of lines and then set them again at
    # the start of the next line.  This requires a second pass through the
    # wrapped string, accumulating any attributes we see, remembering them,
    # and then inserting the appropriate sequences at the newline.
    if ($output =~ /\n/) {
        my @lines = split (/\n/, $output);
        my $start_format;
        for my $line (@lines) {
            if ($start_format && $line =~ /\S/) {
                $line =~ s/^(\s*)(\S)/$1$start_format$2/;
            }
            $start_format = $self->end_format ($line);
            if ($start_format) {
                $line .= color ('reset');
            }
        }
        $output = join ("\n", @lines);
    }

    # Fix up trailing whitespace and return the results.
    $output =~ s/\s+$/\n\n/;
    $output;
}

##############################################################################
# Module return value and documentation
##############################################################################

1;
__END__

=for stopwords
Allbery

=head1 NAME

Pod::Text::Color - Convert POD data to formatted color ASCII text

=head1 SYNOPSIS

    use Pod::Text::Color;
    my $parser = Pod::Text::Color->new (sentence => 0, width => 78);

    # Read POD from STDIN and write to STDOUT.
    $parser->parse_from_filehandle;

    # Read POD from file.pod and write to file.txt.
    $parser->parse_from_file ('file.pod', 'file.txt');

=head1 DESCRIPTION

Pod::Text::Color is a simple subclass of Pod::Text that highlights output
text using ANSI color escape sequences.  Apart from the color, it in all
ways functions like Pod::Text.  See L<Pod::Text> for details and available
options.

Term::ANSIColor is used to get colors and therefore must be installed to use
this module.

=head1 COMPATIBILITY

Pod::Text::Color 0.05 (based on L<Pod::Parser>) was the first version of this
module included with Perl, in Perl 5.6.0.

The current API based on L<Pod::Simple> was added in Pod::Text::Color 2.00.
Pod::Text::Color 2.01 was included in Perl 5.9.3, the first version of Perl to
incorporate those changes.

Several problems with wrapping and line length were fixed as recently as
Pod::Text::Color 4.11, included in Perl 5.29.1.

This module inherits its API and most behavior from Pod::Text, so the details
in L<Pod::Text/COMPATIBILITY> also apply.  Pod::Text and Pod::Text::Color have
had the same module version since 4.00, included in Perl 5.23.7.  (They
unfortunately diverge in confusing ways prior to that.)

=head1 AUTHOR

Russ Allbery <rra@cpan.org>.

=head1 COPYRIGHT AND LICENSE

Copyright 1999, 2001, 2004, 2006, 2008, 2009, 2018-2019, 2022 Russ Allbery
<rra@cpan.org>

This program is free software; you may redistribute it and/or modify it
under the same terms as Perl itself.

=head1 SEE ALSO

L<Pod::Text>, L<Pod::Simple>

The current version of this module is always available from its web site at
L<https://www.eyrie.org/~eagle/software/podlators/>.  It is also part of the
Perl core distribution as of 5.6.0.

=cut

# Local Variables:
# copyright-at-end-flag: t
# End:
