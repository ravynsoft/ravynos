# $Id: encoding.pm,v 3.00 2020/04/19 10:56:28 dankogai Exp $
package encoding;
our $VERSION = sprintf "%d.%02d", q$Revision: 3.00 $ =~ /(\d+)/g;

use Encode;
use strict;
use warnings;
use Config;

use constant {
    DEBUG => !!$ENV{PERL_ENCODE_DEBUG},
    HAS_PERLIO => eval { require PerlIO::encoding; PerlIO::encoding->VERSION(0.02) },
    PERL_5_21_7 => $^V && $^V ge v5.21.7, # lexically scoped
};

sub _exception {
    my $name = shift;
    $] > 5.008 and return 0;    # 5.8.1 or higher then no
    my %utfs = map { $_ => 1 }
      qw(utf8 UCS-2BE UCS-2LE UTF-16 UTF-16BE UTF-16LE
      UTF-32 UTF-32BE UTF-32LE);
    $utfs{$name} or return 0;    # UTFs or no
    require Config;
    Config->import();
    our %Config;
    return $Config{perl_patchlevel} ? 0 : 1    # maintperl then no
}

sub in_locale { $^H & ( $locale::hint_bits || 0 ) }

sub _get_locale_encoding {
    my $locale_encoding;

    if ($^O eq 'MSWin32') {
        my @tries = (
            # First try to get the OutputCP. This will work only if we
            # are attached to a console
            'Win32.pm' => 'Win32::GetConsoleOutputCP',
            'Win32/Console.pm' => 'Win32::Console::OutputCP',
            # If above failed, this means that we are a GUI app
            # Let's assume that the ANSI codepage is what matters
            'Win32.pm' => 'Win32::GetACP',
        );
        while (@tries) {
            my $cp = eval {
                require $tries[0];
                no strict 'refs';
                &{$tries[1]}()
            };
            if ($cp) {
                if ($cp == 65001) { # Code page for UTF-8
                    $locale_encoding = 'UTF-8';
                } else {
                    $locale_encoding = 'cp' . $cp;
                }
                return $locale_encoding;
            }
            splice(@tries, 0, 2)
        }
    }

    # I18N::Langinfo isn't available everywhere
    $locale_encoding = eval {
        require I18N::Langinfo;
        find_encoding(
            I18N::Langinfo::langinfo( I18N::Langinfo::CODESET() )
        )->name
    };
    return $locale_encoding if defined $locale_encoding;

    eval {
        require POSIX;
        # Get the current locale
        # Remember that MSVCRT impl is quite different from Unixes
        my $locale = POSIX::setlocale(POSIX::LC_CTYPE());
        if ( $locale =~ /^([^.]+)\.([^.@]+)(?:@.*)?$/ ) {
            my $country_language;
            ( $country_language, $locale_encoding ) = ( $1, $2 );

            # Could do more heuristics based on the country and language
            # since we have Locale::Country and Locale::Language available.
            # TODO: get a database of Language -> Encoding mappings
            # (the Estonian database at http://www.eki.ee/letter/
            # would be excellent!) --jhi
            if (lc($locale_encoding) eq 'euc') {
                if ( $country_language =~ /^ja_JP|japan(?:ese)?$/i ) {
                    $locale_encoding = 'euc-jp';
                }
                elsif ( $country_language =~ /^ko_KR|korean?$/i ) {
                    $locale_encoding = 'euc-kr';
                }
                elsif ( $country_language =~ /^zh_CN|chin(?:a|ese)$/i ) {
                    $locale_encoding = 'euc-cn';
                }
                elsif ( $country_language =~ /^zh_TW|taiwan(?:ese)?$/i ) {
                    $locale_encoding = 'euc-tw';
                }
                else {
                    require Carp;
                    Carp::croak(
                        "encoding: Locale encoding '$locale_encoding' too ambiguous"
                    );
                }
            }
        }
    };

    return $locale_encoding;
}

sub import {

    if ( ord("A") == 193 ) {
        require Carp;
        Carp::croak("encoding: pragma does not support EBCDIC platforms");
    }

    my $deprecate =
        ($] >= 5.017 and !$Config{usecperl})
        ? "Use of the encoding pragma is deprecated" : 0;

    my $class = shift;
    my $name  = shift;
    if (!$name){
	require Carp;
        Carp::croak("encoding: no encoding specified.");
    }
    if ( $name eq ':_get_locale_encoding' ) {    # used by lib/open.pm
        my $caller = caller();
        {
            no strict 'refs';
            *{"${caller}::_get_locale_encoding"} = \&_get_locale_encoding;
        }
        return;
    }
    $name = _get_locale_encoding() if $name eq ':locale';
    BEGIN { strict->unimport('hashpairs') if $] >= 5.027 and $^V =~ /c$/; }
    my %arg = @_;
    $name = $ENV{PERL_ENCODING} unless defined $name;
    my $enc = find_encoding($name);
    unless ( defined $enc ) {
        require Carp;
        Carp::croak("encoding: Unknown encoding '$name'");
    }
    $name = $enc->name;    # canonize
    unless ( $arg{Filter} ) {
        if ($] >= 5.025003 and !$Config{usecperl}) {
            require Carp;
            Carp::croak("The encoding pragma is no longer supported. Check cperl");
        }
        warnings::warnif("deprecated",$deprecate) if $deprecate;

        DEBUG and warn "_exception($name) = ", _exception($name);
        if (! _exception($name)) {
            if (!PERL_5_21_7) {
                ${^ENCODING} = $enc;
            }
            else {
                # Starting with 5.21.7, this pragma uses a shadow variable
                # designed explicitly for it, ${^E_NCODING}, to enforce
                # lexical scope; instead of ${^ENCODING}.
                $^H{'encoding'} = 1;
                ${^E_NCODING} = $enc;
            }
        }
        if (! HAS_PERLIO ) {
            return 1;
        }
    }
    else {
        warnings::warnif("deprecated",$deprecate) if $deprecate;

        defined( ${^ENCODING} ) and undef ${^ENCODING};
        undef ${^E_NCODING} if PERL_5_21_7;

        # implicitly 'use utf8'
        require utf8;      # to fetch $utf8::hint_bits;
        $^H |= $utf8::hint_bits;

            require Filter::Util::Call;
            Filter::Util::Call->import;
            filter_add(
                sub {
                    my $status = filter_read();
                    if ( $status > 0 ) {
                        $_ = $enc->decode( $_, 1 );
                        DEBUG and warn $_;
                    }
                    $status;
                }
            );
    }
    defined ${^UNICODE} and ${^UNICODE} != 0 and return 1;
    for my $h (qw(STDIN STDOUT)) {
        if ( $arg{$h} ) {
            unless ( defined find_encoding( $arg{$h} ) ) {
                require Carp;
                Carp::croak(
                    "encoding: Unknown encoding for $h, '$arg{$h}'");
            }
            binmode( $h, ":raw :encoding($arg{$h})" );
        }
        else {
            unless ( exists $arg{$h} ) {
                    no warnings 'uninitialized';
                    binmode( $h, ":raw :encoding($name)" );
            }
        }
    }
    return 1;    # I doubt if we need it, though
}

sub unimport {
    no warnings;
    undef ${^ENCODING};
    undef ${^E_NCODING} if PERL_5_21_7;
    if (HAS_PERLIO) {
        binmode( STDIN,  ":raw" );
        binmode( STDOUT, ":raw" );
    }
    else {
        binmode(STDIN);
        binmode(STDOUT);
    }
    if ( $INC{"Filter/Util/Call.pm"} ) {
        eval { filter_del() };
    }
}

1;
__END__

=pod

=head1 NAME

encoding - allows you to write your script in non-ASCII and non-UTF-8

=head1 WARNING

This module has been deprecated since perl v5.18.  See L</DESCRIPTION> and
L</BUGS>.

=head1 SYNOPSIS

  use encoding "greek";  # Perl like Greek to you?
  use encoding "euc-jp"; # Jperl!

  # or you can even do this if your shell supports your native encoding

  perl -Mencoding=latin2 -e'...' # Feeling centrally European?
  perl -Mencoding=euc-kr -e'...' # Or Korean?

  # more control

  # A simple euc-cn => utf-8 converter
  use encoding "euc-cn", STDOUT => "utf8";  while(<>){print};

  # "no encoding;" supported
  no encoding;

  # an alternate way, Filter
  use encoding "euc-jp", Filter=>1;
  # now you can use kanji identifiers -- in euc-jp!

  # encode based on the current locale - specialized purposes only;
  # fraught with danger!!
  use encoding ':locale';

=head1 DESCRIPTION

This pragma is used to enable a Perl script to be written in encodings that
aren't strictly ASCII nor UTF-8.  It translates all or portions of the Perl
program script from a given encoding into UTF-8, and changes the PerlIO layers
of C<STDIN> and C<STDOUT> to the encoding specified.

This pragma dates from the days when UTF-8-enabled editors were uncommon.  But
that was long ago, and the need for it is greatly diminished.  That, coupled
with the fact that it doesn't work with threads, along with other problems,
(see L</BUGS>) have led to its being deprecated.  It is planned to remove this
pragma in a future Perl version.  New code should be written in UTF-8, and the
C<use utf8> pragma used instead (see L<perluniintro> and L<utf8> for details).
Old code should be converted to UTF-8, via something like the recipe in the
L</SYNOPSIS> (though this simple approach may require manual adjustments
afterwards).

If UTF-8 is not an option, it is recommended that one use a simple source
filter, such as that provided by L<Filter::Encoding> on CPAN or this
pragma's own C<Filter> option (see below).

The only legitimate use of this pragma is almost certainly just one per file,
near the top, with file scope, as the file is likely going to only be written
in one encoding.  Further restrictions apply in Perls before v5.22 (see
L</Prior to Perl v5.22>).

There are two basic modes of operation (plus turning if off):

=over 4

=item C<use encoding ['I<ENCNAME>'] ;>

Please note: This mode of operation is no longer supported as of Perl
v5.26.

This is the normal operation.  It translates various literals encountered in
the Perl source file from the encoding I<ENCNAME> into UTF-8, and similarly
converts character code points.  This is used when the script is a combination
of ASCII (for the variable names and punctuation, I<etc>), but the literal
data is in the specified encoding.

I<ENCNAME> is optional.  If omitted, the encoding specified in the environment
variable L<C<PERL_ENCODING>|perlrun/PERL_ENCODING> is used.  If this isn't
set, or the resolved-to encoding is not known to C<L<Encode>>, the error
C<Unknown encoding 'I<ENCNAME>'> will be thrown.

Starting in Perl v5.8.6 (C<Encode> version 2.0.1), I<ENCNAME> may be the
name C<:locale>.  This is for very specialized applications, and is documented
in L</The C<:locale> sub-pragma> below.

The literals that are converted are C<q//, qq//, qr//, qw///, qx//>, and
starting in v5.8.1, C<tr///>.  Operations that do conversions include C<chr>,
C<ord>, C<utf8::upgrade> (but not C<utf8::downgrade>), and C<chomp>.

Also starting in v5.8.1, the C<DATA> pseudo-filehandle is translated from the
encoding into UTF-8.

For example, you can write code in EUC-JP as follows:

  my $Rakuda = "\xF1\xD1\xF1\xCC"; # Camel in Kanji
               #<-char-><-char->   # 4 octets
  s/\bCamel\b/$Rakuda/;

And with C<use encoding "euc-jp"> in effect, it is the same thing as
that code in UTF-8:

  my $Rakuda = "\x{99F1}\x{99DD}"; # two Unicode Characters
  s/\bCamel\b/$Rakuda/;

See L</EXAMPLE> below for a more complete example.

Unless C<${^UNICODE}> (available starting in v5.8.2) exists and is non-zero, the
PerlIO layers of C<STDIN> and C<STDOUT> are set to "C<:encoding(I<ENCNAME>)>".
Therefore,

  use encoding "euc-jp";
  my $message = "Camel is the symbol of perl.\n";
  my $Rakuda = "\xF1\xD1\xF1\xCC"; # Camel in Kanji
  $message =~ s/\bCamel\b/$Rakuda/;
  print $message;

will print

 "\xF1\xD1\xF1\xCC is the symbol of perl.\n"

not

 "\x{99F1}\x{99DD} is the symbol of perl.\n"

You can override this by giving extra arguments; see below.

Note that C<STDERR> WILL NOT be changed, regardless.

Also note that non-STD file handles remain unaffected.  Use C<use
open> or C<binmode> to change the layers of those.

=item C<use encoding I<ENCNAME>, Filter=E<gt>1;>

This operates as above, but the C<Filter> argument with a non-zero
value causes the entire script, and not just literals, to be translated from
the encoding into UTF-8.  This allows identifiers in the source to be in that
encoding as well.  (Problems may occur if the encoding is not a superset of
ASCII; imagine all your semi-colons being translated into something
different.)  One can use this form to make

 ${"\x{4eba}"}++

work.  (This is equivalent to C<$I<human>++>, where I<human> is a single Han
ideograph).

This effectively means that your source code behaves as if it were written in
UTF-8 with C<'use utf8>' in effect.  So even if your editor only supports
Shift_JIS, for example, you can still try examples in Chapter 15 of
C<Programming Perl, 3rd Ed.>.

This option is significantly slower than the other one.

=item C<no encoding;>

Unsets the script encoding. The layers of C<STDIN>, C<STDOUT> are
reset to "C<:raw>" (the default unprocessed raw stream of bytes).

=back

=head1 OPTIONS

=head2 Setting C<STDIN> and/or C<STDOUT> individually

The encodings of C<STDIN> and C<STDOUT> are individually settable by parameters to
the pragma:

 use encoding 'euc-tw', STDIN => 'greek'  ...;

In this case, you cannot omit the first I<ENCNAME>.  C<< STDIN => undef >>
turns the I/O transcoding completely off for that filehandle.

When C<${^UNICODE}> (available starting in v5.8.2) exists and is non-zero,
these options will be completely ignored.  See L<perlvar/C<${^UNICODE}>> and
L<"C<-C>" in perlrun|perlrun/-C [numberE<sol>list]> for details.

=head2 The C<:locale> sub-pragma

Starting in v5.8.6, the encoding name may be C<:locale>.  This means that the
encoding is taken from the current locale, and not hard-coded by the pragma.
Since a script really can only be encoded in exactly one encoding, this option
is dangerous.  It makes sense only if the script itself is written in ASCII,
and all the possible locales that will be in use when the script is executed
are supersets of ASCII.  That means that the script itself doesn't get
changed, but the I/O handles have the specified encoding added, and the
operations like C<chr> and C<ord> use that encoding.

The logic of finding which locale C<:locale> uses is as follows:

=over 4

=item 1.

If the platform supports the C<langinfo(CODESET)> interface, the codeset
returned is used as the default encoding for the open pragma.

=item 2.

If 1. didn't work but we are under the locale pragma, the environment
variables C<LC_ALL> and C<LANG> (in that order) are matched for encodings
(the part after "C<.>", if any), and if any found, that is used
as the default encoding for the open pragma.

=item 3.

If 1. and 2. didn't work, the environment variables C<LC_ALL> and C<LANG>
(in that order) are matched for anything looking like UTF-8, and if
any found, C<:utf8> is used as the default encoding for the open
pragma.

=back

If your locale environment variables (C<LC_ALL>, C<LC_CTYPE>, C<LANG>)
contain the strings 'UTF-8' or 'UTF8' (case-insensitive matching),
the default encoding of your C<STDIN>, C<STDOUT>, and C<STDERR>, and of
B<any subsequent file open>, is UTF-8.

=head1 CAVEATS

=head2 SIDE EFFECTS

=over

=item *

If the C<encoding> pragma is in scope then the lengths returned are
calculated from the length of C<$/> in Unicode characters, which is not
always the same as the length of C<$/> in the native encoding.

=item *

Without this pragma, if strings operating under byte semantics and strings
with Unicode character data are concatenated, the new string will
be created by decoding the byte strings as I<ISO 8859-1 (Latin-1)>.

The B<encoding> pragma changes this to use the specified encoding
instead.  For example:

    use encoding 'utf8';
    my $string = chr(20000); # a Unicode string
    utf8::encode($string);   # now it's a UTF-8 encoded byte string
    # concatenate with another Unicode string
    print length($string . chr(20000));

Will print C<2>, because C<$string> is upgraded as UTF-8.  Without
C<use encoding 'utf8';>, it will print C<4> instead, since C<$string>
is three octets when interpreted as Latin-1.

=back

=head2 DO NOT MIX MULTIPLE ENCODINGS

Notice that only literals (string or regular expression) having only
legacy code points are affected: if you mix data like this

    \x{100}\xDF
    \xDF\x{100}

the data is assumed to be in (Latin 1 and) Unicode, not in your native
encoding.  In other words, this will match in "greek":

    "\xDF" =~ /\x{3af}/

but this will not

    "\xDF\x{100}" =~ /\x{3af}\x{100}/

since the C<\xDF> (ISO 8859-7 GREEK SMALL LETTER IOTA WITH TONOS) on
the left will B<not> be upgraded to C<\x{3af}> (Unicode GREEK SMALL
LETTER IOTA WITH TONOS) because of the C<\x{100}> on the left.  You
should not be mixing your legacy data and Unicode in the same string.

This pragma also affects encoding of the 0x80..0xFF code point range:
normally characters in that range are left as eight-bit bytes (unless
they are combined with characters with code points 0x100 or larger,
in which case all characters need to become UTF-8 encoded), but if
the C<encoding> pragma is present, even the 0x80..0xFF range always
gets UTF-8 encoded.

After all, the best thing about this pragma is that you don't have to
resort to \x{....} just to spell your name in a native encoding.
So feel free to put your strings in your encoding in quotes and
regexes.

=head2 Prior to Perl v5.22

The pragma was a per script, not a per block lexical.  Only the last
C<use encoding> or C<no encoding> mattered, and it affected
B<the whole script>.  However, the C<no encoding> pragma was supported and
C<use encoding> could appear as many times as you want in a given script
(though only the last was effective).

Since the scope wasn't lexical, other modules' use of C<chr>, C<ord>, I<etc.>
were affected.  This leads to spooky, incorrect action at a distance that is
hard to debug.

This means you would have to be very careful of the load order:

  # called module
  package Module_IN_BAR;
  use encoding "bar";
  # stuff in "bar" encoding here
  1;

  # caller script
  use encoding "foo"
  use Module_IN_BAR;
  # surprise! use encoding "bar" is in effect.

The best way to avoid this oddity is to use this pragma RIGHT AFTER
other modules are loaded.  i.e.

  use Module_IN_BAR;
  use encoding "foo";

=head2 Prior to Encode version 1.87

=over

=item *

C<STDIN> and C<STDOUT> were not set under the filter option.
And C<< STDIN=>I<ENCODING> >> and C<< STDOUT=>I<ENCODING> >> didn't work like
non-filter version.

=item *

C<use utf8> wasn't implicitly declared so you have to C<use utf8> to do

 ${"\x{4eba}"}++

=back

=head2 Prior to Perl v5.8.1

=over

=item "NON-EUC" doublebyte encodings

Because perl needs to parse the script before applying this pragma, such
encodings as Shift_JIS and Big-5 that may contain C<'\'> (BACKSLASH;
C<\x5c>) in the second byte fail because the second byte may
accidentally escape the quoting character that follows.

=item C<tr///>

The B<encoding> pragma works by decoding string literals in
C<q//,qq//,qr//,qw///, qx//> and so forth.  In perl v5.8.0, this
does not apply to C<tr///>.  Therefore,

  use encoding 'euc-jp';
  #....
  $kana =~ tr/\xA4\xA1-\xA4\xF3/\xA5\xA1-\xA5\xF3/;
  #           -------- -------- -------- --------

Does not work as

  $kana =~ tr/\x{3041}-\x{3093}/\x{30a1}-\x{30f3}/;

=over

=item Legend of characters above

  utf8     euc-jp   charnames::viacode()
  -----------------------------------------
  \x{3041} \xA4\xA1 HIRAGANA LETTER SMALL A
  \x{3093} \xA4\xF3 HIRAGANA LETTER N
  \x{30a1} \xA5\xA1 KATAKANA LETTER SMALL A
  \x{30f3} \xA5\xF3 KATAKANA LETTER N

=back

This counterintuitive behavior has been fixed in perl v5.8.1.

In perl v5.8.0, you can work around this as follows;

  use encoding 'euc-jp';
  #  ....
  eval qq{ \$kana =~ tr/\xA4\xA1-\xA4\xF3/\xA5\xA1-\xA5\xF3/ };

Note the C<tr//> expression is surrounded by C<qq{}>.  The idea behind
this is the same as the classic idiom that makes C<tr///> 'interpolate':

   tr/$from/$to/;            # wrong!
   eval qq{ tr/$from/$to/ }; # workaround.

=back

=head1 EXAMPLE - Greekperl

    use encoding "iso 8859-7";

    # \xDF in ISO 8859-7 (Greek) is \x{3af} in Unicode.

    $a = "\xDF";
    $b = "\x{100}";

    printf "%#x\n", ord($a); # will print 0x3af, not 0xdf

    $c = $a . $b;

    # $c will be "\x{3af}\x{100}", not "\x{df}\x{100}".

    # chr() is affected, and ...

    print "mega\n"  if ord(chr(0xdf)) == 0x3af;

    # ... ord() is affected by the encoding pragma ...

    print "tera\n" if ord(pack("C", 0xdf)) == 0x3af;

    # ... as are eq and cmp ...

    print "peta\n" if "\x{3af}" eq  pack("C", 0xdf);
    print "exa\n"  if "\x{3af}" cmp pack("C", 0xdf) == 0;

    # ... but pack/unpack C are not affected, in case you still
    # want to go back to your native encoding

    print "zetta\n" if unpack("C", (pack("C", 0xdf))) == 0xdf;

=head1 BUGS

=over

=item Thread safety

C<use encoding ...> is not thread-safe (i.e., do not use in threaded
applications).

=item Can't be used by more than one module in a single program.

Only one encoding is allowed.  If you combine modules in a program that have
different encodings, only one will be actually used.

=item Other modules using C<STDIN> and C<STDOUT> get the encoded stream

They may be expecting something completely different.

=item literals in regex that are longer than 127 bytes

For native multibyte encodings (either fixed or variable length),
the current implementation of the regular expressions may introduce
recoding errors for regular expression literals longer than 127 bytes.

=item EBCDIC

The encoding pragma is not supported on EBCDIC platforms.

=item C<format>

This pragma doesn't work well with C<format> because PerlIO does not
get along very well with it.  When C<format> contains non-ASCII
characters it prints funny or gets "wide character warnings".
To understand it, try the code below.

  # Save this one in utf8
  # replace *non-ascii* with a non-ascii string
  my $camel;
  format STDOUT =
  *non-ascii*@>>>>>>>
  $camel
  .
  $camel = "*non-ascii*";
  binmode(STDOUT=>':encoding(utf8)'); # bang!
  write;              # funny
  print $camel, "\n"; # fine

Without binmode this happens to work but without binmode, print()
fails instead of write().

At any rate, the very use of C<format> is questionable when it comes to
unicode characters since you have to consider such things as character
width (i.e. double-width for ideographs) and directions (i.e. BIDI for
Arabic and Hebrew).

=item See also L</CAVEATS>

=back

=head1 HISTORY

This pragma first appeared in Perl v5.8.0.  It has been enhanced in later
releases as specified above.

=head1 SEE ALSO

L<perlunicode>, L<Encode>, L<open>, L<Filter::Util::Call>,

Ch. 15 of C<Programming Perl (3rd Edition)>
by Larry Wall, Tom Christiansen, Jon Orwant;
O'Reilly & Associates; ISBN 0-596-00027-8

=cut
