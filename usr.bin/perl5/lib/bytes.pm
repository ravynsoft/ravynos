package bytes;

use strict;
use warnings;

our $VERSION = '1.08';

$bytes::hint_bits = 0x00000008;

sub import {
    $^H |= $bytes::hint_bits;
}

sub unimport {
    $^H &= ~$bytes::hint_bits;
}

our $AUTOLOAD;
sub AUTOLOAD {
    require "bytes_heavy.pl";
    goto &$AUTOLOAD if defined &$AUTOLOAD;
    require Carp;
    Carp::croak("Undefined subroutine $AUTOLOAD called");
}

sub length (_);
sub chr (_);
sub ord (_);
sub substr ($$;$$);
sub index ($$;$);
sub rindex ($$;$);

1;
__END__

=head1 NAME

bytes - Perl pragma to expose the individual bytes of characters

=head1 NOTICE

Because the bytes pragma breaks encapsulation (i.e. it exposes the innards of
how the perl executable currently happens to store a string), the byte values
that result are in an unspecified encoding.

B<Use of this module for anything other than debugging purposes is
strongly discouraged.>  If you feel that the functions here within
might be useful for your application, this possibly indicates a
mismatch between your mental model of Perl Unicode and the current
reality. In that case, you may wish to read some of the perl Unicode
documentation: L<perluniintro>, L<perlunitut>, L<perlunifaq> and
L<perlunicode>.

=head1 SYNOPSIS

    use bytes;
    ... chr(...);       # or bytes::chr
    ... index(...);     # or bytes::index
    ... length(...);    # or bytes::length
    ... ord(...);       # or bytes::ord
    ... rindex(...);    # or bytes::rindex
    ... substr(...);    # or bytes::substr
    no bytes;


=head1 DESCRIPTION

Perl's characters are stored internally as sequences of one or more bytes.
This pragma allows for the examination of the individual bytes that together
comprise a character.

Originally the pragma was designed for the loftier goal of helping incorporate
Unicode into Perl, but the approach that used it was found to be defective,
and the one remaining legitimate use is for debugging when you need to
non-destructively examine characters' individual bytes.  Just insert this
pragma temporarily, and remove it after the debugging is finished.

The original usage can be accomplished by explicit (rather than this pragma's
implicit) encoding using the L<Encode> module:

    use Encode qw/encode/;

    my $utf8_byte_string   = encode "UTF8",   $string;
    my $latin1_byte_string = encode "Latin1", $string;

Or, if performance is needed and you are only interested in the UTF-8
representation:

    utf8::encode(my $utf8_byte_string = $string);

C<no bytes> can be used to reverse the effect of C<use bytes> within the
current lexical scope.

As an example, when Perl sees C<$x = chr(400)>, it encodes the character
in UTF-8 and stores it in C<$x>. Then it is marked as character data, so,
for instance, C<length $x> returns C<1>. However, in the scope of the
C<bytes> pragma, C<$x> is treated as a series of bytes - the bytes that make
up the UTF8 encoding - and C<length $x> returns C<2>:

 $x = chr(400);
 print "Length is ", length $x, "\n";     # "Length is 1"
 printf "Contents are %vd\n", $x;         # "Contents are 400"
 {
     use bytes; # or "require bytes; bytes::length()"
     print "Length is ", length $x, "\n"; # "Length is 2"
     printf "Contents are %vd\n", $x;     # "Contents are 198.144 (on
                                          # ASCII platforms)"
 }

C<chr()>, C<ord()>, C<substr()>, C<index()> and C<rindex()> behave similarly.

For more on the implications, see L<perluniintro> and L<perlunicode>.

C<bytes::length()> is admittedly handy if you need to know the
B<byte length> of a Perl scalar.  But a more modern way is:

   use Encode 'encode';
   length(encode('UTF-8', $scalar))

=head1 LIMITATIONS

C<bytes::substr()> does not work as an I<lvalue()>.

=head1 SEE ALSO

L<perluniintro>, L<perlunicode>, L<utf8>, L<Encode>

=cut
