package Encode::MIME::Header;
use strict;
use warnings;

our $VERSION = do { my @r = ( q$Revision: 2.29 $ =~ /\d+/g ); sprintf "%d." . "%02d" x $#r, @r };

use Carp ();
use Encode ();
use MIME::Base64 ();

my %seed = (
    decode_b => 1,       # decodes 'B' encoding ?
    decode_q => 1,       # decodes 'Q' encoding ?
    encode   => 'B',     # encode with 'B' or 'Q' ?
    charset  => 'UTF-8', # encode charset
    bpl      => 75,      # bytes per line
);

my @objs;

push @objs, bless {
    %seed,
    Name     => 'MIME-Header',
} => __PACKAGE__;

push @objs, bless {
    %seed,
    decode_q => 0,
    Name     => 'MIME-B',
} => __PACKAGE__;

push @objs, bless {
    %seed,
    decode_b => 0,
    encode   => 'Q',
    Name     => 'MIME-Q',
} => __PACKAGE__;

Encode::define_encoding($_, $_->{Name}) foreach @objs;

use parent qw(Encode::Encoding);

sub needs_lines { 1 }
sub perlio_ok   { 0 }

# RFC 2047 and RFC 2231 grammar
my $re_charset = qr/[!"#\$%&'+\-0-9A-Z\\\^_`a-z\{\|\}~]+/;
my $re_language = qr/[A-Za-z]{1,8}(?:-[0-9A-Za-z]{1,8})*/;
my $re_encoding = qr/[QqBb]/;
my $re_encoded_text = qr/[^\?]*/;
my $re_encoded_word = qr/=\?$re_charset(?:\*$re_language)?\?$re_encoding\?$re_encoded_text\?=/;
my $re_capture_encoded_word = qr/=\?($re_charset)((?:\*$re_language)?)\?($re_encoding\?$re_encoded_text)\?=/;
my $re_capture_encoded_word_split = qr/=\?($re_charset)((?:\*$re_language)?)\?($re_encoding)\?($re_encoded_text)\?=/;

# in strict mode check also for valid base64 characters and also for valid quoted printable codes
my $re_encoding_strict_b = qr/[Bb]/;
my $re_encoding_strict_q = qr/[Qq]/;
my $re_encoded_text_strict_b = qr/(?:[0-9A-Za-z\+\/]{4})*(?:[0-9A-Za-z\+\/]{2}==|[0-9A-Za-z\+\/]{3}=|)/;
my $re_encoded_text_strict_q = qr/(?:[\x21-\x3C\x3E\x40-\x7E]|=[0-9A-Fa-f]{2})*/; # NOTE: first part are printable US-ASCII except ?, =, SPACE and TAB
my $re_encoded_word_strict = qr/=\?$re_charset(?:\*$re_language)?\?(?:$re_encoding_strict_b\?$re_encoded_text_strict_b|$re_encoding_strict_q\?$re_encoded_text_strict_q)\?=/;
my $re_capture_encoded_word_strict = qr/=\?($re_charset)((?:\*$re_language)?)\?($re_encoding_strict_b\?$re_encoded_text_strict_b|$re_encoding_strict_q\?$re_encoded_text_strict_q)\?=/;

my $re_newline = qr/(?:\r\n|[\r\n])/;

# in strict mode encoded words must be always separated by spaces or tabs (or folded newline)
# except in comments when separator between words and comment round brackets can be omitted
my $re_word_begin_strict = qr/(?:(?:[ \t]|\A)\(?|(?:[^\\]|\A)\)\()/;
my $re_word_sep_strict = qr/(?:$re_newline?[ \t])+/;
my $re_word_end_strict = qr/(?:\)\(|\)?(?:$re_newline?[ \t]|\z))/;

my $re_match = qr/()((?:$re_encoded_word\s*)*$re_encoded_word)()/;
my $re_match_strict = qr/($re_word_begin_strict)((?:$re_encoded_word_strict$re_word_sep_strict)*$re_encoded_word_strict)(?=$re_word_end_strict)/;

my $re_capture = qr/$re_capture_encoded_word(?:\s*)?/;
my $re_capture_strict = qr/$re_capture_encoded_word_strict$re_word_sep_strict?/;

our $STRICT_DECODE = 0;

sub decode($$;$) {
    my ($obj, $str, $chk) = @_;
    return undef unless defined $str;

    my $re_match_decode = $STRICT_DECODE ? $re_match_strict : $re_match;
    my $re_capture_decode = $STRICT_DECODE ? $re_capture_strict : $re_capture;

    my $stop = 0;
    my $output = substr($str, 0, 0); # to propagate taintedness

    # decode each line separately, match whole continuous folded line at one call
    1 while not $stop and $str =~ s{^((?:[^\r\n]*(?:$re_newline[ \t])?)*)($re_newline)?}{

        my $line = $1;
        my $sep = defined $2 ? $2 : '';

        $stop = 1 unless length($line) or length($sep);

        # in non strict mode append missing '=' padding characters for b words
        # fixes below concatenation of consecutive encoded mime words
        1 while not $STRICT_DECODE and $line =~ s/(=\?$re_charset(?:\*$re_language)?\?[Bb]\?)((?:[^\?]{4})*[^\?]{1,3})(\?=)/$1.$2.('='x(4-length($2)%4)).$3/se;

        # NOTE: this code partially could break $chk support
        # in non strict mode concat consecutive encoded mime words with same charset, language and encoding
        # fixes breaking inside multi-byte characters
        1 while not $STRICT_DECODE and $line =~ s/$re_capture_encoded_word_split\s*=\?\1\2\?\3\?($re_encoded_text)\?=/=\?$1$2\?$3\?$4$5\?=/so;

        # process sequence of encoded MIME words at once
        1 while not $stop and $line =~ s{^(.*?)$re_match_decode}{

            my $begin = $1 . $2;
            my $words = $3;

            $begin =~ tr/\r\n//d;
            $output .= $begin;

            # decode one MIME word
            1 while not $stop and $words =~ s{^(.*?)($re_capture_decode)}{

                $output .= $1;
                my $orig = $2;
                my $charset = $3;
                my ($mime_enc, $text) = split /\?/, $5;

                $text =~ tr/\r\n//d;

                my $enc = Encode::find_mime_encoding($charset);

                # in non strict mode allow also perl encoding aliases
                if ( not defined $enc and not $STRICT_DECODE ) {
                    # make sure that decoded string will be always strict UTF-8
                    $charset = 'UTF-8' if lc($charset) eq 'utf8';
                    $enc = Encode::find_encoding($charset);
                }

                if ( not defined $enc ) {
                    Carp::croak qq(Unknown charset "$charset") if not ref $chk and $chk and $chk & Encode::DIE_ON_ERR;
                    Carp::carp qq(Unknown charset "$charset") if not ref $chk and $chk and $chk & Encode::WARN_ON_ERR;
                    $stop = 1 if not ref $chk and $chk and $chk & Encode::RETURN_ON_ERR;
                    $output .= ($output =~ /(?:\A|[ \t])$/ ? '' : ' ') . $orig unless $stop; # $orig mime word is separated by whitespace
                    $stop ? $orig : '';
                } else {
                    if ( uc($mime_enc) eq 'B' and $obj->{decode_b} ) {
                        my $decoded = _decode_b($enc, $text, $chk);
                        $stop = 1 if not defined $decoded and not ref $chk and $chk and $chk & Encode::RETURN_ON_ERR;
                        $output .= (defined $decoded ? $decoded : $text) unless $stop;
                        $stop ? $orig : '';
                    } elsif ( uc($mime_enc) eq 'Q' and $obj->{decode_q} ) {
                        my $decoded = _decode_q($enc, $text, $chk);
                        $stop = 1 if not defined $decoded and not ref $chk and $chk and $chk & Encode::RETURN_ON_ERR;
                        $output .= (defined $decoded ? $decoded : $text) unless $stop;
                        $stop ? $orig : '';
                    } else {
                        Carp::croak qq(MIME "$mime_enc" unsupported) if not ref $chk and $chk and $chk & Encode::DIE_ON_ERR;
                        Carp::carp qq(MIME "$mime_enc" unsupported) if not ref $chk and $chk and $chk & Encode::WARN_ON_ERR;
                        $stop = 1 if not ref $chk and $chk and $chk & Encode::RETURN_ON_ERR;
                        $output .= ($output =~ /(?:\A|[ \t])$/ ? '' : ' ') . $orig unless $stop; # $orig mime word is separated by whitespace
                        $stop ? $orig : '';
                    }
                }

            }se;

            if ( not $stop ) {
                $output .= $words;
                $words = '';
            }

            $words;

        }se;

        if ( not $stop ) {
            $line =~ tr/\r\n//d;
            $output .= $line . $sep;
            $line = '';
            $sep = '';
        }

        $line . $sep;

    }se;

    $_[1] = $str if not ref $chk and $chk and !($chk & Encode::LEAVE_SRC);
    return $output;
}

sub _decode_b {
    my ($enc, $text, $chk) = @_;
    # MIME::Base64::decode ignores everything after a '=' padding character
    # in non strict mode split string after each sequence of padding characters and decode each substring
    my $octets = $STRICT_DECODE ?
        MIME::Base64::decode($text) :
        join('', map { MIME::Base64::decode($_) } split /(?<==)(?=[^=])/, $text);
    return _decode_octets($enc, $octets, $chk);
}

sub _decode_q {
    my ($enc, $text, $chk) = @_;
    $text =~ s/_/ /go;
    $text =~ s/=([0-9A-Fa-f]{2})/pack('C', hex($1))/ego;
    return _decode_octets($enc, $text, $chk);
}

sub _decode_octets {
    my ($enc, $octets, $chk) = @_;
    $chk = 0 unless defined $chk;
    $chk &= ~Encode::LEAVE_SRC if not ref $chk and $chk;
    my $output = $enc->decode($octets, $chk);
    return undef if not ref $chk and $chk and $octets ne '';
    return $output;
}

sub encode($$;$) {
    my ($obj, $str, $chk) = @_;
    return undef unless defined $str;
    my $output = $obj->_fold_line($obj->_encode_string($str, $chk));
    $_[1] = $str if not ref $chk and $chk and !($chk & Encode::LEAVE_SRC);
    return $output . substr($str, 0, 0); # to propagate taintedness
}

sub _fold_line {
    my ($obj, $line) = @_;
    my $bpl = $obj->{bpl};
    my $output = '';

    while ( length($line) ) {
        if ( $line =~ s/^(.{0,$bpl})(\s|\z)// ) {
            $output .= $1;
            $output .= "\r\n" . $2 if length($line);
        } elsif ( $line =~ s/(\s)(.*)$// ) {
            $output .= $line;
            $line = $2;
            $output .= "\r\n" . $1 if length($line);
        } else {
            $output .= $line;
            last;
        }
    }

    return $output;
}

sub _encode_string {
    my ($obj, $str, $chk) = @_;
    my $wordlen = $obj->{bpl} > 76 ? 76 : $obj->{bpl};
    my $enc = Encode::find_mime_encoding($obj->{charset});
    my $enc_chk = $chk;
    $enc_chk = 0 unless defined $enc_chk;
    $enc_chk |= Encode::LEAVE_SRC if not ref $enc_chk and $enc_chk;
    my @result = ();
    my $octets = '';
    while ( length( my $chr = substr($str, 0, 1, '') ) ) {
        my $seq = $enc->encode($chr, $enc_chk);
        if ( not length($seq) ) {
            substr($str, 0, 0, $chr);
            last;
        }
        if ( $obj->_encoded_word_len($octets . $seq) > $wordlen ) {
            push @result, $obj->_encode_word($octets);
            $octets = '';
        }
        $octets .= $seq;
    }
    length($octets) and push @result, $obj->_encode_word($octets);
    $_[1] = $str if not ref $chk and $chk and !($chk & Encode::LEAVE_SRC);
    return join(' ', @result);
}

sub _encode_word {
    my ($obj, $octets) = @_;
    my $charset = $obj->{charset};
    my $encode = $obj->{encode};
    my $text = $encode eq 'B' ? _encode_b($octets) : _encode_q($octets);
    return "=?$charset?$encode?$text?=";
}

sub _encoded_word_len {
    my ($obj, $octets) = @_;
    my $charset = $obj->{charset};
    my $encode = $obj->{encode};
    my $text_len = $encode eq 'B' ? _encoded_b_len($octets) : _encoded_q_len($octets);
    return length("=?$charset?$encode??=") + $text_len;
}

sub _encode_b {
    my ($octets) = @_;
    return MIME::Base64::encode($octets, '');
}

sub _encoded_b_len {
    my ($octets) = @_;
    return ( length($octets) + 2 ) / 3 * 4;
}

my $re_invalid_q_char = qr/[^0-9A-Za-z !*+\-\/]/;

sub _encode_q {
    my ($octets) = @_;
    $octets =~ s{($re_invalid_q_char)}{
        join('', map { sprintf('=%02X', $_) } unpack('C*', $1))
    }egox;
    $octets =~ s/ /_/go;
    return $octets;
}

sub _encoded_q_len {
    my ($octets) = @_;
    my $invalid_count = () = $octets =~ /$re_invalid_q_char/sgo;
    return ( $invalid_count * 3 ) + ( length($octets) - $invalid_count );
}

1;
__END__

=head1 NAME

Encode::MIME::Header -- MIME encoding for an unstructured email header

=head1 SYNOPSIS

    use Encode qw(encode decode);

    my $mime_str = encode("MIME-Header", "Sample:Text \N{U+263A}");
    # $mime_str is "=?UTF-8?B?U2FtcGxlOlRleHQg4pi6?="

    my $mime_q_str = encode("MIME-Q", "Sample:Text \N{U+263A}");
    # $mime_q_str is "=?UTF-8?Q?Sample=3AText_=E2=98=BA?="

    my $str = decode("MIME-Header",
        "=?ISO-8859-1?B?SWYgeW91IGNhbiByZWFkIHRoaXMgeW8=?=\r\n " .
        "=?ISO-8859-2?B?dSB1bmRlcnN0YW5kIHRoZSBleGFtcGxlLg==?="
    );
    # $str is "If you can read this you understand the example."

    use Encode qw(decode :fallbacks);
    use Encode::MIME::Header;
    local $Encode::MIME::Header::STRICT_DECODE = 1;
    my $strict_string = decode("MIME-Header", $mime_string, FB_CROAK);
    # use strict decoding and croak on errors

=head1 ABSTRACT

This module implements L<RFC 2047|https://tools.ietf.org/html/rfc2047> MIME
encoding for an unstructured field body of the email header.  It can also be
used for L<RFC 822|https://tools.ietf.org/html/rfc822> 'text' token.  However,
it cannot be used directly for the whole header with the field name or for the
structured header fields like From, To, Cc, Message-Id, etc...  There are 3
encoding names supported by this module: C<MIME-Header>, C<MIME-B> and
C<MIME-Q>.

=head1 DESCRIPTION

Decode method takes an unstructured field body of the email header (or
L<RFC 822|https://tools.ietf.org/html/rfc822> 'text' token) as its input and
decodes each MIME encoded-word from input string to a sequence of bytes
according to L<RFC 2047|https://tools.ietf.org/html/rfc2047> and
L<RFC 2231|https://tools.ietf.org/html/rfc2231>.  Subsequently, each sequence
of bytes with the corresponding MIME charset is decoded with
L<the Encode module|Encode> and finally, one output string is returned.  Text
parts of the input string which do not contain MIME encoded-word stay
unmodified in the output string.  Folded newlines between two consecutive MIME
encoded-words are discarded, others are preserved in the output string.
C<MIME-B> can decode Base64 variant, C<MIME-Q> can decode Quoted-Printable
variant and C<MIME-Header> can decode both of them.  If L<Encode module|Encode>
does not support particular MIME charset or chosen variant then an action based
on L<CHECK flags|Encode/Handling Malformed Data> is performed (by default, the
MIME encoded-word is not decoded).

Encode method takes a scalar string as its input and uses
L<strict UTF-8|Encode/UTF-8 vs. utf8 vs. UTF8> encoder for encoding it to UTF-8
bytes.  Then a sequence of UTF-8 bytes is encoded into MIME encoded-words
(C<MIME-Header> and C<MIME-B> use a Base64 variant while C<MIME-Q> uses a
Quoted-Printable variant) where each MIME encoded-word is limited to 75
characters.  MIME encoded-words are separated by C<CRLF SPACE> and joined to
one output string.  Output string is suitable for unstructured field body of
the email header.

Both encode and decode methods propagate
L<CHECK flags|Encode/Handling Malformed Data> when encoding and decoding the
MIME charset.

=head1 BUGS

Versions prior to 2.22 (part of Encode 2.83) have a malfunctioning decoder
and encoder.  The MIME encoder infamously inserted additional spaces or
discarded white spaces between consecutive MIME encoded-words, which led to
invalid MIME headers produced by this module.  The MIME decoder had a tendency
to discard white spaces, incorrectly interpret data or attempt to decode Base64
MIME encoded-words as Quoted-Printable.  These problems were fixed in version
2.22.  It is highly recommended not to use any version prior 2.22!

Versions prior to 2.24 (part of Encode 2.87) ignored
L<CHECK flags|Encode/Handling Malformed Data>.  The MIME encoder used
L<not strict utf8|Encode/UTF-8 vs. utf8 vs. UTF8> encoder for input Unicode
strings which could lead to invalid UTF-8 sequences.  MIME decoder used also
L<not strict utf8|Encode/UTF-8 vs. utf8 vs. UTF8> decoder and additionally
called the decode method with a C<Encode::FB_PERLQQ> flag (thus user-specified
L<CHECK flags|Encode/Handling Malformed Data> were ignored).  Moreover, it
automatically croaked when a MIME encoded-word contained unknown encoding.
Since version 2.24, this module uses
L<strict UTF-8|Encode/UTF-8 vs. utf8 vs. UTF8> encoder and decoder.  And
L<CHECK flags|Encode/Handling Malformed Data> are correctly propagated.

Since version 2.22 (part of Encode 2.83), the MIME encoder should be fully
compliant to L<RFC 2047|https://tools.ietf.org/html/rfc2047> and
L<RFC 2231|https://tools.ietf.org/html/rfc2231>.  Due to the aforementioned
bugs in previous versions of the MIME encoder, there is a I<less strict>
compatible mode for the MIME decoder which is used by default.  It should be
able to decode MIME encoded-words encoded by pre 2.22 versions of this module.
However, note that this is not correct according to
L<RFC 2047|https://tools.ietf.org/html/rfc2047>.

In default I<not strict> mode the MIME decoder attempts to decode every substring
which looks like a MIME encoded-word.  Therefore, the MIME encoded-words do not
need to be separated by white space.  To enforce a correct I<strict> mode, set
variable C<$Encode::MIME::Header::STRICT_DECODE> to 1 e.g. by localizing:

  use Encode::MIME::Header;
  local $Encode::MIME::Header::STRICT_DECODE = 1;

=head1 AUTHORS

Pali E<lt>pali@cpan.orgE<gt>

=head1 SEE ALSO

L<Encode>,
L<RFC 822|https://tools.ietf.org/html/rfc822>,
L<RFC 2047|https://tools.ietf.org/html/rfc2047>,
L<RFC 2231|https://tools.ietf.org/html/rfc2231>

=cut
