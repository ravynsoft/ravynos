#
# $Id: Encode.pm,v 3.19 2022/08/04 04:42:30 dankogai Exp $
#
package Encode;
use strict;
use warnings;
use constant DEBUG => !!$ENV{PERL_ENCODE_DEBUG};
our $VERSION;
BEGIN {
    $VERSION = sprintf "%d.%02d", q$Revision: 3.19 $ =~ /(\d+)/g;
    require XSLoader;
    XSLoader::load( __PACKAGE__, $VERSION );
}

use Exporter 5.57 'import';

use Carp ();
our @CARP_NOT = qw(Encode::Encoder);

# Public, encouraged API is exported by default

our @EXPORT = qw(
  decode  decode_utf8  encode  encode_utf8 str2bytes bytes2str
  encodings  find_encoding find_mime_encoding clone_encoding
);
our @FB_FLAGS = qw(
  DIE_ON_ERR WARN_ON_ERR RETURN_ON_ERR LEAVE_SRC
  PERLQQ HTMLCREF XMLCREF STOP_AT_PARTIAL
);
our @FB_CONSTS = qw(
  FB_DEFAULT FB_CROAK FB_QUIET FB_WARN
  FB_PERLQQ FB_HTMLCREF FB_XMLCREF
);
our @EXPORT_OK = (
    qw(
      _utf8_off _utf8_on define_encoding from_to is_16bit is_8bit
      is_utf8 perlio_ok resolve_alias utf8_downgrade utf8_upgrade
      ),
    @FB_FLAGS, @FB_CONSTS,
);

our %EXPORT_TAGS = (
    all          => [ @EXPORT,    @EXPORT_OK ],
    default      => [ @EXPORT ],
    fallbacks    => [ @FB_CONSTS ],
    fallback_all => [ @FB_CONSTS, @FB_FLAGS ],
);

# Documentation moved after __END__ for speed - NI-S

our $ON_EBCDIC = ( ord("A") == 193 );

use Encode::Alias ();
use Encode::MIME::Name;

use Storable;

# Make a %Encoding package variable to allow a certain amount of cheating
our %Encoding;
our %ExtModule;
require Encode::Config;
#  See
#  https://bugzilla.redhat.com/show_bug.cgi?id=435505#c2
#  to find why sig handlers inside eval{} are disabled.
eval {
    local $SIG{__DIE__};
    local $SIG{__WARN__};
    local @INC = @INC;
    pop @INC if @INC && $INC[-1] eq '.';
    require Encode::ConfigLocal;
};

sub encodings {
    my %enc;
    my $arg  = $_[1] || '';
    if ( $arg eq ":all" ) {
        %enc = ( %Encoding, %ExtModule );
    }
    else {
        %enc = %Encoding;
        for my $mod ( map { m/::/ ? $_ : "Encode::$_" } @_ ) {
            DEBUG and warn $mod;
            for my $enc ( keys %ExtModule ) {
                $ExtModule{$enc} eq $mod and $enc{$enc} = $mod;
            }
        }
    }
    return sort { lc $a cmp lc $b }
      grep      { !/^(?:Internal|Unicode|Guess)$/o } keys %enc;
}

sub perlio_ok {
    my $obj = ref( $_[0] ) ? $_[0] : find_encoding( $_[0] );
    $obj->can("perlio_ok") and return $obj->perlio_ok();
    return 0;    # safety net
}

sub define_encoding {
    my $obj  = shift;
    my $name = shift;
    $Encoding{$name} = $obj;
    my $lc = lc($name);
    define_alias( $lc => $obj ) unless $lc eq $name;
    while (@_) {
        my $alias = shift;
        define_alias( $alias, $obj );
    }
    my $class = ref($obj);
    push @Encode::CARP_NOT, $class unless grep { $_ eq $class } @Encode::CARP_NOT;
    push @Encode::Encoding::CARP_NOT, $class unless grep { $_ eq $class } @Encode::Encoding::CARP_NOT;
    return $obj;
}

sub getEncoding {
    my ( $class, $name, $skip_external ) = @_;

    defined($name) or return;

    $name =~ s/\s+//g; # https://rt.cpan.org/Ticket/Display.html?id=65796

    ref($name) && $name->can('renew') and return $name;
    exists $Encoding{$name} and return $Encoding{$name};
    my $lc = lc $name;
    exists $Encoding{$lc} and return $Encoding{$lc};

    my $oc = $class->find_alias($name);
    defined($oc) and return $oc;
    $lc ne $name and $oc = $class->find_alias($lc);
    defined($oc) and return $oc;

    unless ($skip_external) {
        if ( my $mod = $ExtModule{$name} || $ExtModule{$lc} ) {
            $mod =~ s,::,/,g;
            $mod .= '.pm';
            eval { require $mod; };
            exists $Encoding{$name} and return $Encoding{$name};
        }
    }
    return;
}

# HACK: These two functions must be defined in Encode and because of
# cyclic dependency between Encode and Encode::Alias, Exporter does not work
sub find_alias {
    goto &Encode::Alias::find_alias;
}
sub define_alias {
    goto &Encode::Alias::define_alias;
}

sub find_encoding($;$) {
    my ( $name, $skip_external ) = @_;
    return __PACKAGE__->getEncoding( $name, $skip_external );
}

sub find_mime_encoding($;$) {
    my ( $mime_name, $skip_external ) = @_;
    my $name = Encode::MIME::Name::get_encode_name( $mime_name );
    return find_encoding( $name, $skip_external );
}

sub resolve_alias($) {
    my $obj = find_encoding(shift);
    defined $obj and return $obj->name;
    return;
}

sub clone_encoding($) {
    my $obj = find_encoding(shift);
    ref $obj or return;
    return Storable::dclone($obj);
}

onBOOT;

if ($ON_EBCDIC) {
    package Encode::UTF_EBCDIC;
    use parent 'Encode::Encoding';
    my $obj = bless { Name => "UTF_EBCDIC" } => "Encode::UTF_EBCDIC";
    Encode::define_encoding($obj, 'Unicode');
    sub decode {
        my ( undef, $str, $chk ) = @_;
        my $res = '';
        for ( my $i = 0 ; $i < length($str) ; $i++ ) {
            $res .=
              chr(
                utf8::unicode_to_native( ord( substr( $str, $i, 1 ) ) )
              );
        }
        $_[1] = '' if $chk;
        return $res;
    }
    sub encode {
        my ( undef, $str, $chk ) = @_;
        my $res = '';
        for ( my $i = 0 ; $i < length($str) ; $i++ ) {
            $res .=
              chr(
                utf8::native_to_unicode( ord( substr( $str, $i, 1 ) ) )
              );
        }
        $_[1] = '' if $chk;
        return $res;
    }
}

{
    # https://rt.cpan.org/Public/Bug/Display.html?id=103253
    package Encode::XS;
    use parent 'Encode::Encoding';
}

{
    package Encode::utf8;
    use parent 'Encode::Encoding';
    my %obj = (
        'utf8'         => { Name => 'utf8' },
        'utf-8-strict' => { Name => 'utf-8-strict', strict_utf8 => 1 }
    );
    for ( keys %obj ) {
        bless $obj{$_} => __PACKAGE__;
        Encode::define_encoding( $obj{$_} => $_ );
    }
    sub cat_decode {
        # ($obj, $dst, $src, $pos, $trm, $chk)
        # currently ignores $chk
        my ( undef, undef, undef, $pos, $trm ) = @_;
        my ( $rdst, $rsrc, $rpos ) = \@_[ 1, 2, 3 ];
        use bytes;
        if ( ( my $npos = index( $$rsrc, $trm, $pos ) ) >= 0 ) {
            $$rdst .=
              substr( $$rsrc, $pos, $npos - $pos + length($trm) );
            $$rpos = $npos + length($trm);
            return 1;
        }
        $$rdst .= substr( $$rsrc, $pos );
        $$rpos = length($$rsrc);
        return '';
    }
}

1;

__END__

=head1 NAME

Encode - character encodings in Perl

=head1 SYNOPSIS

    use Encode qw(decode encode);
    $characters = decode('UTF-8', $octets,     Encode::FB_CROAK);
    $octets     = encode('UTF-8', $characters, Encode::FB_CROAK);

=head2 Table of Contents

Encode consists of a collection of modules whose details are too extensive
to fit in one document.  This one itself explains the top-level APIs
and general topics at a glance.  For other topics and more details,
see the documentation for these modules:

=over 2

=item L<Encode::Alias> - Alias definitions to encodings

=item L<Encode::Encoding> - Encode Implementation Base Class

=item L<Encode::Supported> - List of Supported Encodings

=item L<Encode::CN> - Simplified Chinese Encodings

=item L<Encode::JP> - Japanese Encodings

=item L<Encode::KR> - Korean Encodings

=item L<Encode::TW> - Traditional Chinese Encodings

=back

=head1 DESCRIPTION

The C<Encode> module provides the interface between Perl strings
and the rest of the system.  Perl strings are sequences of
I<characters>.

The repertoire of characters that Perl can represent is a superset of those
defined by the Unicode Consortium. On most platforms the ordinal
values of a character as returned by C<ord(I<S>)> is the I<Unicode
codepoint> for that character. The exceptions are platforms where
the legacy encoding is some variant of EBCDIC rather than a superset
of ASCII; see L<perlebcdic>.

During recent history, data is moved around a computer in 8-bit chunks,
often called "bytes" but also known as "octets" in standards documents.
Perl is widely used to manipulate data of many types: not only strings of
characters representing human or computer languages, but also "binary"
data, being the machine's representation of numbers, pixels in an image, or
just about anything.

When Perl is processing "binary data", the programmer wants Perl to
process "sequences of bytes". This is not a problem for Perl: because a
byte has 256 possible values, it easily fits in Perl's much larger
"logical character".

This document mostly explains the I<how>. L<perlunitut> and L<perlunifaq>
explain the I<why>.

=head2 TERMINOLOGY

=head3 character

A character in the range 0 .. 2**32-1 (or more);
what Perl's strings are made of.

=head3 byte

A character in the range 0..255;
a special case of a Perl character.

=head3 octet

8 bits of data, with ordinal values 0..255;
term for bytes passed to or from a non-Perl context, such as a disk file,
standard I/O stream, database, command-line argument, environment variable,
socket etc.

=head1 THE PERL ENCODING API

=head2 Basic methods

=head3 encode

  $octets  = encode(ENCODING, STRING[, CHECK])

Encodes the scalar value I<STRING> from Perl's internal form into
I<ENCODING> and returns a sequence of octets.  I<ENCODING> can be either a
canonical name or an alias.  For encoding names and aliases, see
L</"Defining Aliases">.  For CHECK, see L</"Handling Malformed Data">.

B<CAVEAT>: the input scalar I<STRING> might be modified in-place depending
on what is set in CHECK. See L</LEAVE_SRC> if you want your inputs to be
left unchanged.

For example, to convert a string from Perl's internal format into
ISO-8859-1, also known as Latin1:

  $octets = encode("iso-8859-1", $string);

B<CAVEAT>: When you run C<$octets = encode("UTF-8", $string)>, then
$octets I<might not be equal to> $string.  Though both contain the
same data, the UTF8 flag for $octets is I<always> off.  When you
encode anything, the UTF8 flag on the result is always off, even when it
contains a completely valid UTF-8 string. See L</"The UTF8 flag"> below.

If the $string is C<undef>, then C<undef> is returned.

C<str2bytes> may be used as an alias for C<encode>.

=head3 decode

  $string = decode(ENCODING, OCTETS[, CHECK])

This function returns the string that results from decoding the scalar
value I<OCTETS>, assumed to be a sequence of octets in I<ENCODING>, into
Perl's internal form.  As with encode(),
I<ENCODING> can be either a canonical name or an alias. For encoding names
and aliases, see L</"Defining Aliases">; for I<CHECK>, see L</"Handling
Malformed Data">.

B<CAVEAT>: the input scalar I<OCTETS> might be modified in-place depending
on what is set in CHECK. See L</LEAVE_SRC> if you want your inputs to be
left unchanged.

For example, to convert ISO-8859-1 data into a string in Perl's
internal format:

  $string = decode("iso-8859-1", $octets);

B<CAVEAT>: When you run C<$string = decode("UTF-8", $octets)>, then $string
I<might not be equal to> $octets.  Though both contain the same data, the
UTF8 flag for $string is on.  See L</"The UTF8 flag">
below.

If the $string is C<undef>, then C<undef> is returned.

C<bytes2str> may be used as an alias for C<decode>.

=head3 find_encoding

  [$obj =] find_encoding(ENCODING)

Returns the I<encoding object> corresponding to I<ENCODING>.  Returns
C<undef> if no matching I<ENCODING> is find.  The returned object is
what does the actual encoding or decoding.

  $string = decode($name, $bytes);

is in fact

    $string = do {
        $obj = find_encoding($name);
        croak qq(encoding "$name" not found) unless ref $obj;
        $obj->decode($bytes);
    };

with more error checking.

You can therefore save time by reusing this object as follows;

    my $enc = find_encoding("iso-8859-1");
    while(<>) {
        my $string = $enc->decode($_);
        ... # now do something with $string;
    }

Besides L</decode> and L</encode>, other methods are
available as well.  For instance, C<name()> returns the canonical
name of the encoding object.

  find_encoding("latin1")->name; # iso-8859-1

See L<Encode::Encoding> for details.

=head3 find_mime_encoding

  [$obj =] find_mime_encoding(MIME_ENCODING)

Returns the I<encoding object> corresponding to I<MIME_ENCODING>.  Acts
same as C<find_encoding()> but C<mime_name()> of returned object must
match to I<MIME_ENCODING>.  So as opposite of C<find_encoding()>
canonical names and aliases are not used when searching for object.

    find_mime_encoding("utf8"); # returns undef because "utf8" is not valid I<MIME_ENCODING>
    find_mime_encoding("utf-8"); # returns encode object "utf-8-strict"
    find_mime_encoding("UTF-8"); # same as "utf-8" because I<MIME_ENCODING> is case insensitive
    find_mime_encoding("utf-8-strict"); returns undef because "utf-8-strict" is not valid I<MIME_ENCODING>

=head3 from_to

  [$length =] from_to($octets, FROM_ENC, TO_ENC [, CHECK])

Converts I<in-place> data between two encodings. The data in $octets
must be encoded as octets and I<not> as characters in Perl's internal
format. For example, to convert ISO-8859-1 data into Microsoft's CP1250
encoding:

  from_to($octets, "iso-8859-1", "cp1250");

and to convert it back:

  from_to($octets, "cp1250", "iso-8859-1");

Because the conversion happens in place, the data to be
converted cannot be a string constant: it must be a scalar variable.

C<from_to()> returns the length of the converted string in octets on success,
and C<undef> on error.

B<CAVEAT>: The following operations may look the same, but are not:

  from_to($data, "iso-8859-1", "UTF-8"); #1
  $data = decode("iso-8859-1", $data);  #2

Both #1 and #2 make $data consist of a completely valid UTF-8 string,
but only #2 turns the UTF8 flag on.  #1 is equivalent to:

  $data = encode("UTF-8", decode("iso-8859-1", $data));

See L</"The UTF8 flag"> below.

Also note that:

  from_to($octets, $from, $to, $check);

is equivalent to:

  $octets = encode($to, decode($from, $octets), $check);

Yes, it does I<not> respect the $check during decoding.  It is
deliberately done that way.  If you need minute control, use C<decode>
followed by C<encode> as follows:

  $octets = encode($to, decode($from, $octets, $check_from), $check_to);

=head3 encode_utf8

  $octets = encode_utf8($string);

B<WARNING>: L<This function can produce invalid UTF-8!|/UTF-8 vs. utf8 vs. UTF8>
Do not use it for data exchange.
Unless you want Perl's older "lax" mode, prefer
C<$octets = encode("UTF-8", $string)>.

Equivalent to C<$octets = encode("utf8", $string)>.  The characters in
$string are encoded in Perl's internal format, and the result is returned
as a sequence of octets.  Because all possible characters in Perl have a
(loose, not strict) utf8 representation, this function cannot fail.

=head3 decode_utf8

  $string = decode_utf8($octets [, CHECK]);

B<WARNING>: L<This function accepts invalid UTF-8!|/UTF-8 vs. utf8 vs. UTF8>
Do not use it for data exchange.
Unless you want Perl's older "lax" mode, prefer
C<$string = decode("UTF-8", $octets [, CHECK])>.

Equivalent to C<$string = decode("utf8", $octets [, CHECK])>.
The sequence of octets represented by $octets is decoded
from (loose, not strict) utf8 into a sequence of logical characters.
Because not all sequences of octets are valid not strict utf8,
it is quite possible for this function to fail.
For CHECK, see L</"Handling Malformed Data">.

B<CAVEAT>: the input I<$octets> might be modified in-place depending on
what is set in CHECK. See L</LEAVE_SRC> if you want your inputs to be
left unchanged.

=head2 Listing available encodings

  use Encode;
  @list = Encode->encodings();

Returns a list of canonical names of available encodings that have already
been loaded.  To get a list of all available encodings including those that
have not yet been loaded, say:

  @all_encodings = Encode->encodings(":all");

Or you can give the name of a specific module:

  @with_jp = Encode->encodings("Encode::JP");

When "C<::>" is not in the name, "C<Encode::>" is assumed.

  @ebcdic = Encode->encodings("EBCDIC");

To find out in detail which encodings are supported by this package,
see L<Encode::Supported>.

=head2 Defining Aliases

To add a new alias to a given encoding, use:

  use Encode;
  use Encode::Alias;
  define_alias(NEWNAME => ENCODING);

After that, I<NEWNAME> can be used as an alias for I<ENCODING>.
I<ENCODING> may be either the name of an encoding or an
I<encoding object>.

Before you do that, first make sure the alias is nonexistent using
C<resolve_alias()>, which returns the canonical name thereof.
For example:

  Encode::resolve_alias("latin1") eq "iso-8859-1" # true
  Encode::resolve_alias("iso-8859-12")   # false; nonexistent
  Encode::resolve_alias($name) eq $name  # true if $name is canonical

C<resolve_alias()> does not need C<use Encode::Alias>; it can be
imported via C<use Encode qw(resolve_alias)>.

See L<Encode::Alias> for details.

=head2 Finding IANA Character Set Registry names

The canonical name of a given encoding does not necessarily agree with
IANA Character Set Registry, commonly seen as C<< Content-Type:
text/plain; charset=I<WHATEVER> >>.  For most cases, the canonical name
works, but sometimes it does not, most notably with "utf-8-strict".

As of C<Encode> version 2.21, a new method C<mime_name()> is therefore added.

  use Encode;
  my $enc = find_encoding("UTF-8");
  warn $enc->name;      # utf-8-strict
  warn $enc->mime_name; # UTF-8

See also:  L<Encode::Encoding>

=head1 Encoding via PerlIO

If your perl supports C<PerlIO> (which is the default), you can use a
C<PerlIO> layer to decode and encode directly via a filehandle.  The
following two examples are fully identical in functionality:

  ### Version 1 via PerlIO
    open(INPUT,  "< :encoding(shiftjis)", $infile)
        || die "Can't open < $infile for reading: $!";
    open(OUTPUT, "> :encoding(euc-jp)",  $outfile)
        || die "Can't open > $output for writing: $!";
    while (<INPUT>) {   # auto decodes $_
        print OUTPUT;   # auto encodes $_
    }
    close(INPUT)   || die "can't close $infile: $!";
    close(OUTPUT)  || die "can't close $outfile: $!";

  ### Version 2 via from_to()
    open(INPUT,  "< :raw", $infile)
        || die "Can't open < $infile for reading: $!";
    open(OUTPUT, "> :raw",  $outfile)
        || die "Can't open > $output for writing: $!";

    while (<INPUT>) {
        from_to($_, "shiftjis", "euc-jp", 1);  # switch encoding
        print OUTPUT;   # emit raw (but properly encoded) data
    }
    close(INPUT)   || die "can't close $infile: $!";
    close(OUTPUT)  || die "can't close $outfile: $!";

In the first version above, you let the appropriate encoding layer
handle the conversion.  In the second, you explicitly translate
from one encoding to the other.

Unfortunately, it may be that encodings are not C<PerlIO>-savvy.  You can check
to see whether your encoding is supported by C<PerlIO> by invoking the
C<perlio_ok> method on it:

  Encode::perlio_ok("hz");             # false
  find_encoding("euc-cn")->perlio_ok;  # true wherever PerlIO is available

  use Encode qw(perlio_ok);            # imported upon request
  perlio_ok("euc-jp")

Fortunately, all encodings that come with C<Encode> core are C<PerlIO>-savvy
except for C<hz> and C<ISO-2022-kr>.  For the gory details, see
L<Encode::Encoding> and L<Encode::PerlIO>.

=head1 Handling Malformed Data

The optional I<CHECK> argument tells C<Encode> what to do when
encountering malformed data.  Without I<CHECK>, C<Encode::FB_DEFAULT>
(== 0) is assumed.

As of version 2.12, C<Encode> supports coderef values for C<CHECK>;
see below.

B<NOTE:> Not all encodings support this feature.
Some encodings ignore the I<CHECK> argument.  For example,
L<Encode::Unicode> ignores I<CHECK> and it always croaks on error.

=head2 List of I<CHECK> values

=head3 FB_DEFAULT

  I<CHECK> = Encode::FB_DEFAULT ( == 0)

If I<CHECK> is 0, encoding and decoding replace any malformed character
with a I<substitution character>.  When you encode, I<SUBCHAR> is used.
When you decode, the Unicode REPLACEMENT CHARACTER, code point U+FFFD, is
used.  If the data is supposed to be UTF-8, an optional lexical warning of
warning category C<"utf8"> is given.

=head3 FB_CROAK

  I<CHECK> = Encode::FB_CROAK ( == 1)

If I<CHECK> is 1, methods immediately die with an error
message.  Therefore, when I<CHECK> is 1, you should trap
exceptions with C<eval{}>, unless you really want to let it C<die>.

=head3 FB_QUIET

  I<CHECK> = Encode::FB_QUIET

If I<CHECK> is set to C<Encode::FB_QUIET>, encoding and decoding immediately
return the portion of the data that has been processed so far when an
error occurs. The data argument is overwritten with everything
after that point; that is, the unprocessed portion of the data.  This is
handy when you have to call C<decode> repeatedly in the case where your
source data may contain partial multi-byte character sequences,
(that is, you are reading with a fixed-width buffer). Here's some sample
code to do exactly that:

    my($buffer, $string) = ("", "");
    while (read($fh, $buffer, 256, length($buffer))) {
        $string .= decode($encoding, $buffer, Encode::FB_QUIET);
        # $buffer now contains the unprocessed partial character
    }

=head3 FB_WARN

  I<CHECK> = Encode::FB_WARN

This is the same as C<FB_QUIET> above, except that instead of being silent
on errors, it issues a warning.  This is handy for when you are debugging.

B<CAVEAT>: All warnings from Encode module are reported, independently of
L<pragma warnings|warnings> settings. If you want to follow settings of
lexical warnings configured by L<pragma warnings|warnings> then append
also check value C<ENCODE::ONLY_PRAGMA_WARNINGS>. This value is available
since Encode version 2.99.

=head3 FB_PERLQQ FB_HTMLCREF FB_XMLCREF

=over 2

=item perlqq mode (I<CHECK> = Encode::FB_PERLQQ)

=item HTML charref mode (I<CHECK> = Encode::FB_HTMLCREF)

=item XML charref mode (I<CHECK> = Encode::FB_XMLCREF)

=back

For encodings that are implemented by the C<Encode::XS> module, C<CHECK> C<==>
C<Encode::FB_PERLQQ> puts C<encode> and C<decode> into C<perlqq> fallback mode.

When you decode, C<\xI<HH>> is inserted for a malformed character, where
I<HH> is the hex representation of the octet that could not be decoded to
utf8.  When you encode, C<\x{I<HHHH>}> will be inserted, where I<HHHH> is
the Unicode code point (in any number of hex digits) of the character that
cannot be found in the character repertoire of the encoding.

The HTML/XML character reference modes are about the same. In place of
C<\x{I<HHHH>}>, HTML uses C<&#I<NNN>;> where I<NNN> is a decimal number, and
XML uses C<&#xI<HHHH>;> where I<HHHH> is the hexadecimal number.

In C<Encode> 2.10 or later, C<LEAVE_SRC> is also implied.

=head3 The bitmask

These modes are all actually set via a bitmask.  Here is how the C<FB_I<XXX>>
constants are laid out.  You can import the C<FB_I<XXX>> constants via
C<use Encode qw(:fallbacks)>, and you can import the generic bitmask
constants via C<use Encode qw(:fallback_all)>.

                     FB_DEFAULT FB_CROAK FB_QUIET FB_WARN  FB_PERLQQ
 DIE_ON_ERR    0x0001             X
 WARN_ON_ERR   0x0002                               X
 RETURN_ON_ERR 0x0004                      X        X
 LEAVE_SRC     0x0008                                        X
 PERLQQ        0x0100                                        X
 HTMLCREF      0x0200
 XMLCREF       0x0400

=head3 LEAVE_SRC

  Encode::LEAVE_SRC

If the C<Encode::LEAVE_SRC> bit is I<not> set but I<CHECK> is set, then the
source string to encode() or decode() will be overwritten in place.
If you're not interested in this, then bitwise-OR it with the bitmask.

=head2 coderef for CHECK

As of C<Encode> 2.12, C<CHECK> can also be a code reference which takes the
ordinal value of the unmapped character as an argument and returns
octets that represent the fallback character.  For instance:

  $ascii = encode("ascii", $utf8, sub{ sprintf "<U+%04X>", shift });

Acts like C<FB_PERLQQ> but U+I<XXXX> is used instead of C<\x{I<XXXX>}>.

Fallback for C<decode> must return decoded string (sequence of characters)
and takes a list of ordinal values as its arguments. So for
example if you wish to decode octets as UTF-8, and use ISO-8859-15 as
a fallback for bytes that are not valid UTF-8, you could write

    $str = decode 'UTF-8', $octets, sub {
        my $tmp = join '', map chr, @_;
        return decode 'ISO-8859-15', $tmp;
    };

=head1 Defining Encodings

To define a new encoding, use:

    use Encode qw(define_encoding);
    define_encoding($object, CANONICAL_NAME [, alias...]);

I<CANONICAL_NAME> will be associated with I<$object>.  The object
should provide the interface described in L<Encode::Encoding>.
If more than two arguments are provided, additional
arguments are considered aliases for I<$object>.

See L<Encode::Encoding> for details.

=head1 The UTF8 flag

Before the introduction of Unicode support in Perl, The C<eq> operator
just compared the strings represented by two scalars. Beginning with
Perl 5.8, C<eq> compares two strings with simultaneous consideration of
I<the UTF8 flag>. To explain why we made it so, I quote from page 402 of
I<Programming Perl, 3rd ed.>

=over 2

=item Goal #1:

Old byte-oriented programs should not spontaneously break on the old
byte-oriented data they used to work on.

=item Goal #2:

Old byte-oriented programs should magically start working on the new
character-oriented data when appropriate.

=item Goal #3:

Programs should run just as fast in the new character-oriented mode
as in the old byte-oriented mode.

=item Goal #4:

Perl should remain one language, rather than forking into a
byte-oriented Perl and a character-oriented Perl.

=back

When I<Programming Perl, 3rd ed.> was written, not even Perl 5.6.0 had been
born yet, many features documented in the book remained unimplemented for a
long time.  Perl 5.8 corrected much of this, and the introduction of the
UTF8 flag is one of them.  You can think of there being two fundamentally
different kinds of strings and string-operations in Perl: one a
byte-oriented mode  for when the internal UTF8 flag is off, and the other a
character-oriented mode for when the internal UTF8 flag is on.

This UTF8 flag is not visible in Perl scripts, exactly for the same reason
you cannot (or rather, you I<don't have to>) see whether a scalar contains
a string, an integer, or a floating-point number.   But you can still peek
and poke these if you will.  See the next section.

=head2 Messing with Perl's Internals

The following API uses parts of Perl's internals in the current
implementation.  As such, they are efficient but may change in a future
release.

=head3 is_utf8

  is_utf8(STRING [, CHECK])

[INTERNAL] Tests whether the UTF8 flag is turned on in the I<STRING>.
If I<CHECK> is true, also checks whether I<STRING> contains well-formed
UTF-8.  Returns true if successful, false otherwise.

Typically only necessary for debugging and testing.  Don't use this flag as
a marker to distinguish character and binary data, that should be decided
for each variable when you write your code.

B<CAVEAT>: If I<STRING> has UTF8 flag set, it does B<NOT> mean that
I<STRING> is UTF-8 encoded and vice-versa.

As of Perl 5.8.1, L<utf8> also has the C<utf8::is_utf8> function.

=head3 _utf8_on

  _utf8_on(STRING)

[INTERNAL] Turns the I<STRING>'s internal UTF8 flag B<on>.  The I<STRING>
is I<not> checked for containing only well-formed UTF-8.  Do not use this
unless you I<know with absolute certainty> that the STRING holds only
well-formed UTF-8.  Returns the previous state of the UTF8 flag (so please
don't treat the return value as indicating success or failure), or C<undef>
if I<STRING> is not a string.

B<NOTE>: For security reasons, this function does not work on tainted values.

=head3 _utf8_off

  _utf8_off(STRING)

[INTERNAL] Turns the I<STRING>'s internal UTF8 flag B<off>.  Do not use
frivolously.  Returns the previous state of the UTF8 flag, or C<undef> if
I<STRING> is not a string.  Do not treat the return value as indicative of
success or failure, because that isn't what it means: it is only the
previous setting.

B<NOTE>: For security reasons, this function does not work on tainted values.

=head1 UTF-8 vs. utf8 vs. UTF8

  ....We now view strings not as sequences of bytes, but as sequences
  of numbers in the range 0 .. 2**32-1 (or in the case of 64-bit
  computers, 0 .. 2**64-1) -- Programming Perl, 3rd ed.

That has historically been Perl's notion of UTF-8, as that is how UTF-8 was
first conceived by Ken Thompson when he invented it. However, thanks to
later revisions to the applicable standards, official UTF-8 is now rather
stricter than that. For example, its range is much narrower (0 .. 0x10_FFFF
to cover only 21 bits instead of 32 or 64 bits) and some sequences
are not allowed, like those used in surrogate pairs, the 31 non-character
code points 0xFDD0 .. 0xFDEF, the last two code points in I<any> plane
(0xI<XX>_FFFE and 0xI<XX>_FFFF), all non-shortest encodings, etc.

The former default in which Perl would always use a loose interpretation of
UTF-8 has now been overruled:

  From: Larry Wall <larry@wall.org>
  Date: December 04, 2004 11:51:58 JST
  To: perl-unicode@perl.org
  Subject: Re: Make Encode.pm support the real UTF-8
  Message-Id: <20041204025158.GA28754@wall.org>

  On Fri, Dec 03, 2004 at 10:12:12PM +0000, Tim Bunce wrote:
  : I've no problem with 'utf8' being perl's unrestricted uft8 encoding,
  : but "UTF-8" is the name of the standard and should give the
  : corresponding behaviour.

  For what it's worth, that's how I've always kept them straight in my
  head.

  Also for what it's worth, Perl 6 will mostly default to strict but
  make it easy to switch back to lax.

  Larry

Got that?  As of Perl 5.8.7, B<"UTF-8"> means UTF-8 in its current
sense, which is conservative and strict and security-conscious, whereas
B<"utf8"> means UTF-8 in its former sense, which was liberal and loose and
lax.  C<Encode> version 2.10 or later thus groks this subtle but critically
important distinction between C<"UTF-8"> and C<"utf8">.

  encode("utf8",  "\x{FFFF_FFFF}", 1); # okay
  encode("UTF-8", "\x{FFFF_FFFF}", 1); # croaks

This distinction is also important for decoding. In the following,
C<$s> stores character U+200000, which exceeds UTF-8's allowed range.
C<$s> thus stores an invalid Unicode code point:

  $s = decode("utf8", "\xf8\x88\x80\x80\x80");

C<"UTF-8">, by contrast, will either coerce the input to something valid:

    $s = decode("UTF-8", "\xf8\x88\x80\x80\x80"); # U+FFFD

.. or croak:

    decode("UTF-8", "\xf8\x88\x80\x80\x80", FB_CROAK|LEAVE_SRC);

In the C<Encode> module, C<"UTF-8"> is actually a canonical name for
C<"utf-8-strict">.  That hyphen between the C<"UTF"> and the C<"8"> is
critical; without it, C<Encode> goes "liberal" and (perhaps overly-)permissive:

  find_encoding("UTF-8")->name # is 'utf-8-strict'
  find_encoding("utf-8")->name # ditto. names are case insensitive
  find_encoding("utf_8")->name # ditto. "_" are treated as "-"
  find_encoding("UTF8")->name  # is 'utf8'.

Perl's internal UTF8 flag is called "UTF8", without a hyphen. It indicates
whether a string is internally encoded as "utf8", also without a hyphen.

=head1 SEE ALSO

L<Encode::Encoding>,
L<Encode::Supported>,
L<Encode::PerlIO>,
L<encoding>,
L<perlebcdic>,
L<perlfunc/open>,
L<perlunicode>, L<perluniintro>, L<perlunifaq>, L<perlunitut>
L<utf8>,
the Perl Unicode Mailing List L<http://lists.perl.org/list/perl-unicode.html>

=head1 MAINTAINER

This project was originated by the late Nick Ing-Simmons and later
maintained by Dan Kogai I<< <dankogai@cpan.org> >>.  See AUTHORS
for a full list of people involved.  For any questions, send mail to
I<< <perl-unicode@perl.org> >> so that we can all share.

While Dan Kogai retains the copyright as a maintainer, credit
should go to all those involved.  See AUTHORS for a list of those
who submitted code to the project.

=head1 COPYRIGHT

Copyright 2002-2014 Dan Kogai I<< <dankogai@cpan.org> >>.

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself.

=cut
