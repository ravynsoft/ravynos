package MIME::Base64;

use strict;
use warnings;

require Exporter;
our @ISA = qw(Exporter);
our @EXPORT = qw(encode_base64 decode_base64);
our @EXPORT_OK = qw(encode_base64url decode_base64url encoded_base64_length decoded_base64_length);

our $VERSION = '3.16_01';

require XSLoader;
XSLoader::load('MIME::Base64', $VERSION);

*encode = \&encode_base64;
*decode = \&decode_base64;

sub encode_base64url {
    my $e = encode_base64(shift, "");
    $e =~ s/=+\z//;
    $e =~ tr[+/][-_];
    return $e;
}

sub decode_base64url {
    my $s = shift;
    $s =~ tr[-_][+/];
    $s .= '=' while length($s) % 4;
    return decode_base64($s);
}

1;

__END__

=head1 NAME

MIME::Base64 - Encoding and decoding of base64 strings

=head1 SYNOPSIS

 use MIME::Base64;

 $encoded = encode_base64('Aladdin:open sesame');
 $decoded = decode_base64($encoded);

=head1 DESCRIPTION

This module provides functions to encode and decode strings into and from the
base64 encoding specified in RFC 2045 - I<MIME (Multipurpose Internet
Mail Extensions)>. The base64 encoding is designed to represent
arbitrary sequences of octets in a form that need not be humanly
readable. A 65-character subset ([A-Za-z0-9+/=]) of US-ASCII is used,
enabling 6 bits to be represented per printable character.

The following primary functions are provided:

=over 4

=item encode_base64( $bytes )

=item encode_base64( $bytes, $eol );

Encode data by calling the encode_base64() function.  The first
argument is the byte string to encode.  The second argument is the
line-ending sequence to use.  It is optional and defaults to "\n".  The
returned encoded string is broken into lines of no more than 76
characters each and it will end with $eol unless it is empty.  Pass an
empty string as second argument if you do not want the encoded string
to be broken into lines.

The function will croak with "Wide character in subroutine entry" if $bytes
contains characters with code above 255.  The base64 encoding is only defined
for single-byte characters.  Use the Encode module to select the byte encoding
you want.

=item decode_base64( $str )

Decode a base64 string by calling the decode_base64() function.  This
function takes a single argument which is the string to decode and
returns the decoded data.

Any character not part of the 65-character base64 subset is
silently ignored.  Characters occurring after a '=' padding character
are never decoded.

=back

If you prefer not to import these routines into your namespace, you can
call them as:

    use MIME::Base64 ();
    $encoded = MIME::Base64::encode($decoded);
    $decoded = MIME::Base64::decode($encoded);

Additional functions not exported by default:

=over 4

=item encode_base64url( $bytes )

=item decode_base64url( $str )

Encode and decode according to the base64 scheme for "URL applications" [1].
This is a variant of the base64 encoding which does not use padding, does not
break the string into multiple lines and use the characters "-" and "_" instead
of "+" and "/" to avoid using reserved URL characters.

=item encoded_base64_length( $bytes )

=item encoded_base64_length( $bytes, $eol )

Returns the length that the encoded string would have without actually
encoding it.  This will return the same value as C<< length(encode_base64($bytes)) >>,
but should be more efficient.

=item decoded_base64_length( $str )

Returns the length that the decoded string would have without actually
decoding it.  This will return the same value as C<< length(decode_base64($str)) >>,
but should be more efficient.

=back

=head1 EXAMPLES

If you want to encode a large file, you should encode it in chunks
that are a multiple of 57 bytes.  This ensures that the base64 lines
line up and that you do not end up with padding in the middle. 57
bytes of data fills one complete base64 line (76 == 57*4/3):

   use MIME::Base64 qw(encode_base64);

   open(FILE, "/var/log/wtmp") or die "$!";
   while (read(FILE, $buf, 60*57)) {
       print encode_base64($buf);
   }

or if you know you have enough memory

   use MIME::Base64 qw(encode_base64);
   local($/) = undef;  # slurp
   print encode_base64(<STDIN>);

The same approach as a command line:

   perl -MMIME::Base64 -0777 -ne 'print encode_base64($_)' <file

Decoding does not need slurp mode if every line contains a multiple
of four base64 chars:

   perl -MMIME::Base64 -ne 'print decode_base64($_)' <file

Perl v5.8 and better allow extended Unicode characters in strings.
Such strings cannot be encoded directly, as the base64
encoding is only defined for single-byte characters.  The solution is
to use the Encode module to select the byte encoding you want.  For
example:

    use MIME::Base64 qw(encode_base64);
    use Encode qw(encode);

    $encoded = encode_base64(encode("UTF-8", "\x{FFFF}\n"));
    print $encoded;

=head1 COPYRIGHT

Copyright 1995-1999, 2001-2004, 2010 Gisle Aas.

This library is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

Distantly based on LWP::Base64 written by Martijn Koster
<m.koster@nexor.co.uk> and Joerg Reichelt <j.reichelt@nexor.co.uk> and
code posted to comp.lang.perl <3pd2lp$6gf@wsinti07.win.tue.nl> by Hans
Mulder <hansm@wsinti07.win.tue.nl>

The XS implementation uses code from metamail.  Copyright 1991 Bell
Communications Research, Inc. (Bellcore)

=head1 SEE ALSO

L<MIME::QuotedPrint>

[1] L<http://en.wikipedia.org/wiki/Base64#URL_applications>

=cut
