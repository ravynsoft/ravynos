# Convert POD data to ASCII text with format escapes.
#
# This is a simple subclass of Pod::Text that overrides a few key methods to
# output the right termcap escape sequences for formatted text on the current
# terminal type.
#
# SPDX-License-Identifier: GPL-1.0-or-later OR Artistic-1.0-Perl

##############################################################################
# Modules and declarations
##############################################################################

package Pod::Text::Termcap;

use 5.010;
use strict;
use warnings;

use Pod::Text ();
use POSIX ();
use Term::Cap;

our @ISA = qw(Pod::Text);
our $VERSION = '5.01';

##############################################################################
# Overrides
##############################################################################

# In the initialization method, grab our terminal characteristics as well as
# do all the stuff we normally do.
sub new {
    my ($self, %args) = @_;
    my ($ospeed, $term, $termios);

    # Fall back on a hard-coded terminal speed if POSIX::Termios isn't
    # available (such as on VMS).
    eval { $termios = POSIX::Termios->new };
    if ($@) {
        $ospeed = 9600;
    } else {
        $termios->getattr;
        $ospeed = $termios->getospeed || 9600;
    }

    # Get data from Term::Cap if possible.
    my ($bold, $undl, $norm, $width);
    eval {
        my $term = Tgetent Term::Cap { TERM => undef, OSPEED => $ospeed };
        $bold = $term->Tputs('md');
        $undl = $term->Tputs('us');
        $norm = $term->Tputs('me');
        if (defined $$term{_co}) {
            $width = $$term{_co};
            $width =~ s/^\#//;
        }
    };

    # Figure out the terminal width before calling the Pod::Text constructor,
    # since it will otherwise force 76 characters.  Pod::Text::Termcap has
    # historically used 2 characters less than the width of the screen, while
    # the other Pod::Text classes have used 76.  This is weirdly inconsistent,
    # but there's probably no good reason to change it now.
    unless (defined $args{width}) {
        $args{width} = $ENV{COLUMNS} || $width || 80;
        $args{width} -= 2;
    }

    # Initialize Pod::Text.
    $self = $self->SUPER::new (%args);

    # If we were unable to get any of the formatting sequences, don't attempt
    # that type of formatting.  This will do weird things if bold or underline
    # were available but normal wasn't, but hopefully that will never happen.
    $$self{BOLD} = $bold || q{};
    $$self{UNDL} = $undl || q{};
    $$self{NORM} = $norm || q{};

    return $self;
}

# Make level one headings bold.
sub cmd_head1 {
    my ($self, $attrs, $text) = @_;
    $text =~ s/\s+$//;
    $self->SUPER::cmd_head1 ($attrs, "$$self{BOLD}$text$$self{NORM}");
}

# Make level two headings bold.
sub cmd_head2 {
    my ($self, $attrs, $text) = @_;
    $text =~ s/\s+$//;
    $self->SUPER::cmd_head2 ($attrs, "$$self{BOLD}$text$$self{NORM}");
}

# Fix up B<> and I<>.  Note that we intentionally don't do F<>.
sub cmd_b { my $self = shift; return "$$self{BOLD}$_[1]$$self{NORM}" }
sub cmd_i { my $self = shift; return "$$self{UNDL}$_[1]$$self{NORM}" }

# Return a regex that matches a formatting sequence.  This will only be valid
# if we were able to get at least some termcap information.
sub format_regex {
    my ($self) = @_;
    my @codes = ($self->{BOLD}, $self->{UNDL}, $self->{NORM});
    return join(q{|}, map { $_ eq q{} ? () : "\Q$_\E" } @codes);
}

# Analyze a single line and return any formatting codes in effect at the end
# of that line.
sub end_format {
    my ($self, $line) = @_;
    my $pattern = "(" . $self->format_regex() . ")";
    my $current;
    while ($line =~ /$pattern/g) {
        my $code = $1;
        if ($code eq $$self{NORM}) {
            undef $current;
        } else {
            $current .= $code;
        }
    }
    return $current;
}

# Output any included code in bold.
sub output_code {
    my ($self, $code) = @_;
    $self->output ($$self{BOLD} . $code . $$self{NORM});
}

# Strip all of the formatting from a provided string, returning the stripped
# version.
sub strip_format {
    my ($self, $text) = @_;
    $text =~ s/\Q$$self{BOLD}//g;
    $text =~ s/\Q$$self{UNDL}//g;
    $text =~ s/\Q$$self{NORM}//g;
    return $text;
}

# Override the wrapping code to ignore the special sequences.
sub wrap {
    my $self = shift;
    local $_ = shift;
    my $output = '';
    my $spaces = ' ' x $$self{MARGIN};
    my $width = $$self{opt_width} - $$self{MARGIN};

    # If we were unable to find any termcap sequences, use Pod::Text wrapping.
    if ($self->{BOLD} eq q{} && $self->{UNDL} eq q{} && $self->{NORM} eq q{}) {
        return $self->SUPER::wrap($_);
    }

    # $code matches a single special sequence.  $char matches any number of
    # special sequences preceding a single character other than a newline.
    # $shortchar matches some sequence of $char ending in codes followed by
    # whitespace or the end of the string.  $longchar matches exactly $width
    # $chars, used when we have to truncate and hard wrap.
    my $code = "(?:" . $self->format_regex() . ")";
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
                $line .= $$self{NORM};
            }
        }
        $output = join ("\n", @lines);
    }

    # Fix up trailing whitespace and return the results.
    $output =~ s/\s+$/\n\n/;
    return $output;
}

##############################################################################
# Module return value and documentation
##############################################################################

1;
__END__

=for stopwords
ECMA-48 VT100 Allbery Solaris TERMPATH unformatted

=head1 NAME

Pod::Text::Termcap - Convert POD data to ASCII text with format escapes

=head1 SYNOPSIS

    use Pod::Text::Termcap;
    my $parser = Pod::Text::Termcap->new (sentence => 0, width => 78);

    # Read POD from STDIN and write to STDOUT.
    $parser->parse_from_filehandle;

    # Read POD from file.pod and write to file.txt.
    $parser->parse_from_file ('file.pod', 'file.txt');

=head1 DESCRIPTION

Pod::Text::Termcap is a simple subclass of Pod::Text that highlights output
text using the correct termcap escape sequences for the current terminal.
Apart from the format codes, it in all ways functions like Pod::Text.  See
L<Pod::Text> for details and available options.

This module uses L<Term::Cap> to find the correct terminal settings.  See the
documentation of that module for how it finds terminal database information
and how to override that behavior if necessary.  If unable to find control
strings for bold and underscore formatting, that formatting is skipped,
resulting in the same output as Pod::Text.

=head1 COMPATIBILITY

Pod::Text::Termcap 0.04 (based on L<Pod::Parser>) was the first version of
this module included with Perl, in Perl 5.6.0.

The current API based on L<Pod::Simple> was added in Pod::Text::Termcap 2.00.
Pod::Text::Termcap 2.01 was included in Perl 5.9.3, the first version of Perl
to incorporate those changes.

Several problems with wrapping and line length were fixed as recently as
Pod::Text::Termcap 4.11, included in Perl 5.29.1.

Pod::Text::Termcap 4.13 stopped setting the TERMPATH environment variable
during module load.  It also stopped falling back on VT100 escape sequences if
Term::Cap was not able to find usable escape sequences, instead producing
unformatted output for better results on dumb terminals.  The next version to
be incorporated into Perl, 4.14, was included in Perl 5.31.8.

This module inherits its API and most behavior from Pod::Text, so the details
in L<Pod::Text/COMPATIBILITY> also apply.  Pod::Text and Pod::Text::Termcap
have had the same module version since 4.00, included in Perl 5.23.7.  (They
unfortunately diverge in confusing ways prior to that.)

=head1 AUTHOR

Russ Allbery <rra@cpan.org>

=head1 COPYRIGHT AND LICENSE

Copyright 1999, 2001-2002, 2004, 2006, 2008-2009, 2014-2015, 2018-2019, 2022
Russ Allbery <rra@cpan.org>

This program is free software; you may redistribute it and/or modify it
under the same terms as Perl itself.

=head1 SEE ALSO

L<Pod::Text>, L<Pod::Simple>, L<Term::Cap>

The current version of this module is always available from its web site at
L<https://www.eyrie.org/~eagle/software/podlators/>.  It is also part of the
Perl core distribution as of 5.6.0.

=cut

# Local Variables:
# copyright-at-end-flag: t
# End:
