package I18N::Langinfo;

use 5.006;
use strict;
use warnings;
use Carp;

use Exporter 'import';
require XSLoader;

our @EXPORT = qw(langinfo);

our @EXPORT_OK = qw(
	ABDAY_1
	ABDAY_2
	ABDAY_3
	ABDAY_4
	ABDAY_5
	ABDAY_6
	ABDAY_7
	ABMON_1
	ABMON_10
	ABMON_11
	ABMON_12
	ABMON_2
	ABMON_3
	ABMON_4
	ABMON_5
	ABMON_6
	ABMON_7
	ABMON_8
	ABMON_9
	ALT_DIGITS
	AM_STR
	CODESET
	CRNCYSTR
	DAY_1
	DAY_2
	DAY_3
	DAY_4
	DAY_5
	DAY_6
	DAY_7
	D_FMT
	D_T_FMT
	ERA
	ERA_D_FMT
	ERA_D_T_FMT
	ERA_T_FMT
	MON_1
	MON_10
	MON_11
	MON_12
	MON_2
	MON_3
	MON_4
	MON_5
	MON_6
	MON_7
	MON_8
	MON_9
	NOEXPR
	NOSTR
	PM_STR
	RADIXCHAR
	THOUSEP
	T_FMT
	T_FMT_AMPM
	YESEXPR
	YESSTR
);

our $VERSION = '0.22';

XSLoader::load();

1;
__END__

=head1 NAME

I18N::Langinfo - query locale information

=head1 SYNOPSIS

  use I18N::Langinfo;

=head1 DESCRIPTION

The langinfo() function queries various locale information that can be
used to localize output and user interfaces.  It uses the current underlying
locale, regardless of whether or not it was called from within the scope of
S<C<use locale>>.  The langinfo() function requires
one numeric argument that identifies the locale constant to query:
if no argument is supplied, C<$_> is used.  The numeric constants
appropriate to be used as arguments are exportable from I18N::Langinfo.

The following example will import the langinfo() function itself and
three constants to be used as arguments to langinfo(): a constant for
the abbreviated first day of the week (the numbering starts from
Sunday = 1) and two more constants for the affirmative and negative
answers for a yes/no question in the current locale.

    use I18N::Langinfo qw(langinfo ABDAY_1 YESSTR NOSTR);

    my ($abday_1, $yesstr, $nostr) =
        map { langinfo($_) } (ABDAY_1, YESSTR, NOSTR);

    print "$abday_1? [$yesstr/$nostr] ";

In other words, in the "C" (or English) locale the above will probably
print something like:

    Sun? [yes/no]

but under a French locale

    dim? [oui/non]

The usually available constants are as follows.

=over 4

=item *

For abbreviated and full length days of the week and months of the year:

    ABDAY_1 ABDAY_2 ABDAY_3 ABDAY_4 ABDAY_5 ABDAY_6 ABDAY_7
    ABMON_1 ABMON_2 ABMON_3 ABMON_4 ABMON_5 ABMON_6
    ABMON_7 ABMON_8 ABMON_9 ABMON_10 ABMON_11 ABMON_12
    DAY_1 DAY_2 DAY_3 DAY_4 DAY_5 DAY_6 DAY_7
    MON_1 MON_2 MON_3 MON_4 MON_5 MON_6
    MON_7 MON_8 MON_9 MON_10 MON_11 MON_12

=item *

For the date-time, date, and time formats used by the strftime() function
(see L<POSIX>):

    D_T_FMT D_FMT T_FMT

=item *

For the locales for which it makes sense to have ante meridiem and post
meridiem time formats:

    AM_STR PM_STR T_FMT_AMPM

=item *

For the character code set being used (such as "ISO8859-1", "cp850",
"koi8-r", "sjis", "utf8", etc.), and for the currency string:

    CODESET CRNCYSTR

=item *

For an alternate representation of digits, for the
radix character used between the integer and the fractional part
of decimal numbers, the group separator string for large-ish floating point
numbers (yes, the final two are redundant with
L<POSIX::localeconv()|POSIX/localeconv>):

    ALT_DIGITS RADIXCHAR THOUSEP

=item *

For the affirmative and negative responses and expressions:

    YESSTR YESEXPR NOSTR NOEXPR

=item *

For the eras based on typically some ruler, such as the Japanese Emperor
(naturally only defined in the appropriate locales):

    ERA ERA_D_FMT ERA_D_T_FMT ERA_T_FMT

=back

=head2 For systems without C<nl_langinfo>

This module originally was just a wrapper for the libc C<nl_langinfo>
function, and did not work on systems lacking it, such as Windows.

Starting in Perl 5.28, this module works on all platforms.  When
C<nl_langinfo> is not available, it uses various methods to construct
what that function, if present, would return.  But there are potential
glitches.  These are the items that could be different:

=over

=item C<ERA>

Unimplemented, so returns C<"">.

=item C<CODESET>

This should work properly for Windows platforms.  On almost all other modern
platforms, it will reliably return "UTF-8" if that is the code set.
Otherwise, it depends on the locale's name.  If that is of the form
C<foo.bar>, it will assume C<bar> is the code set; and it also knows about the
two locales "C" and "POSIX".  If none of those apply it returns C<"">.

=item C<YESEXPR>

=item C<YESSTR>

=item C<NOEXPR>

=item C<NOSTR>

Only the values for English are returned.  C<YESSTR> and C<NOSTR> have been
removed from POSIX 2008, and are retained here for backwards compatibility.
Your platform's C<nl_langinfo> may not support them.

=item C<D_FMT>

Always evaluates to C<%x>, the locale's appropriate date representation.

=item C<T_FMT>

Always evaluates to C<%X>, the locale's appropriate time representation.

=item C<D_T_FMT>

Always evaluates to C<%c>, the locale's appropriate date and time
representation.

=item C<CRNCYSTR>

The return may be incorrect for those rare locales where the currency symbol
replaces the radix character.  If you have examples of it needing to work
differently, please file a report at L<https://github.com/Perl/perl5/issues>.

=item C<ALT_DIGITS>

Currently this gives the same results as Linux does.  If you have examples of
it needing to work differently, please file a report at
L<https://github.com/Perl/perl5/issues>.

=item C<ERA_D_FMT>

=item C<ERA_T_FMT>

=item C<ERA_D_T_FMT>

=item C<T_FMT_AMPM>

These are derived by using C<strftime()>, and not all versions of that function
know about them.  C<""> is returned for these on such systems.

=back

See your L<nl_langinfo(3)> for more information about the available
constants.  (Often this means having to look directly at the
F<langinfo.h> C header file.)

=head2 EXPORT

By default only the C<langinfo()> function is exported.

=head1 BUGS

Before Perl 5.28, the returned values are unreliable for the C<RADIXCHAR> and
C<THOUSEP> locale constants.

Starting in 5.28, changing locales on threaded builds is supported on systems
that offer thread-safe locale functions.  These include POSIX 2008 systems and
Windows starting with Visual Studio 2005, and this module will work properly
in such situations.  However, on threaded builds on Windows prior to Visual
Studio 2015, retrieving the items C<CRNCYSTR> and C<THOUSEP> can result in a
race with a thread that has converted to use the global locale.  It is quite
uncommon for a thread to have done this.  It would be possible to construct a
workaround for this; patches welcome: see L<perlapi/switch_to_global_locale>.

=head1 SEE ALSO

L<perllocale>, L<POSIX/localeconv>, L<POSIX/setlocale>, L<nl_langinfo(3)>.

=head1 AUTHOR

Jarkko Hietaniemi, E<lt>jhi@hut.fiE<gt>.  Now maintained by Perl 5 porters.

=head1 COPYRIGHT AND LICENSE

Copyright 2001 by Jarkko Hietaniemi

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself.

=cut
