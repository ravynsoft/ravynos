package locale;

use strict;
use warnings;

our $VERSION = '1.10';
use Config;

$Carp::Internal{ (__PACKAGE__) } = 1;

=head1 NAME

locale - Perl pragma to use or avoid POSIX locales for built-in operations

=head1 WARNING

DO NOT USE this pragma in scripts that have multiple
L<threads|threads> active.  The locale is not local to a single thread.
Another thread may change the locale at any time, which could cause at a
minimum that a given thread is operating in a locale it isn't expecting
to be in.  On some platforms, segfaults can also occur.  The locale
change need not be explicit; some operations cause perl to change the
locale itself.  You are vulnerable simply by having done a C<"use
locale">.

=head1 SYNOPSIS

    @x = sort @y;      # Native-platform/Unicode code point sort order
    {
        use locale;
        @x = sort @y;  # Locale-defined sort order
    }
    @x = sort @y;      # Native-platform/Unicode code point sort order
                       # again

=head1 DESCRIPTION

This pragma tells the compiler to enable (or disable) the use of POSIX
locales for built-in operations (for example, LC_CTYPE for regular
expressions, LC_COLLATE for string comparison, and LC_NUMERIC for number
formatting).  Each "use locale" or "no locale"
affects statements to the end of the enclosing BLOCK.

See L<perllocale> for more detailed information on how Perl supports
locales.

On systems that don't have locales, this pragma will cause your operations
to behave as if in the "C" locale; attempts to change the locale will fail.

=cut

# A separate bit is used for each of the two forms of the pragma, to save
# having to look at %^H for the normal case of a plain 'use locale' without an
# argument.

$locale::hint_bits = 0x4;
$locale::partial_hint_bits = 0x10;  # If pragma has an argument

# The pseudo-category :characters consists of 2 real ones; but it also is
# given its own number, -1, because in the complement form it also has the
# side effect of "use feature 'unicode_strings'"

sub import {
    shift;  # should be 'locale'; not checked

    $^H{locale} = 0 unless defined $^H{locale};
    if (! @_) { # If no parameter, use the plain form that changes all categories
        $^H |= $locale::hint_bits;

    }
    else {
        my @categories = ( qw(:ctype :collate :messages
                              :numeric :monetary :time) );
        for (my $i = 0; $i < @_; $i++) {
            my $arg = $_[$i];
            my $complement = $arg =~ s/ : ( ! | not_ ) /:/x;
            if (! grep { $arg eq $_ } @categories, ":characters") {
                require Carp;
                Carp::croak("Unknown parameter '$_[$i]' to 'use locale'");
            }

            if ($complement) {
                if ($i != 0 || $i < @_ - 1)  {
                    require Carp;
                    Carp::croak("Only one argument to 'use locale' allowed"
                                . "if is $complement");
                }

                if ($arg eq ':characters') {
                    push @_, grep { $_ ne ':ctype' && $_ ne ':collate' }
                                  @categories;
                    # We add 1 to the category number;  This category number
                    # is -1
                    $^H{locale} |= (1 << 0);
                }
                else {
                    push @_, grep { $_ ne $arg } @categories;
                }
                next;
            }
            elsif ($arg eq ':characters') {
                push @_, ':ctype', ':collate';
                next;
            }

            $^H |= $locale::partial_hint_bits;

            # This form of the pragma overrides the other
            $^H &= ~$locale::hint_bits;

            $arg =~ s/^://;

            eval { require POSIX; import POSIX 'locale_h'; };

            # Map our names to the ones defined by POSIX
            my $LC = "LC_" . uc($arg);

            my $bit = eval "&POSIX::$LC";
            if (defined $bit) { # XXX Should we warn that this category isn't
                                # supported on this platform, or make it
                                # always be the C locale?

                # Verify our assumption.
                if (! ($bit >= 0 && $bit < 31)) {
                    require Carp;
                    Carp::croak("Cannot have ':$arg' parameter to 'use locale'"
                              . " on this platform.  Use the 'perlbug' utility"
                              . " to report this problem, or send email to"
                              . " 'perlbug\@perl.org'.  $LC=$bit");
                }

                # 1 is added so that the pseudo-category :characters, which is
                # -1, comes out 0.
                $^H{locale} |= 1 << ($bit + 1);
            }
        }
    }

}

sub unimport {
    $^H &= ~($locale::hint_bits|$locale::partial_hint_bits);
    $^H{locale} = 0;
}

1;
