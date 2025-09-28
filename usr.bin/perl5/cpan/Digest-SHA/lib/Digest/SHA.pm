package Digest::SHA;

require 5.003000;

use strict;
use warnings;
use vars qw($VERSION @ISA @EXPORT_OK $errmsg);
use Fcntl qw(O_RDONLY O_RDWR);
use Cwd qw(getcwd);
use integer;

$VERSION = '6.04';

require Exporter;
@ISA = qw(Exporter);
@EXPORT_OK = qw(
	$errmsg
	hmac_sha1	hmac_sha1_base64	hmac_sha1_hex
	hmac_sha224	hmac_sha224_base64	hmac_sha224_hex
	hmac_sha256	hmac_sha256_base64	hmac_sha256_hex
	hmac_sha384	hmac_sha384_base64	hmac_sha384_hex
	hmac_sha512	hmac_sha512_base64	hmac_sha512_hex
	hmac_sha512224	hmac_sha512224_base64	hmac_sha512224_hex
	hmac_sha512256	hmac_sha512256_base64	hmac_sha512256_hex
	sha1		sha1_base64		sha1_hex
	sha224		sha224_base64		sha224_hex
	sha256		sha256_base64		sha256_hex
	sha384		sha384_base64		sha384_hex
	sha512		sha512_base64		sha512_hex
	sha512224	sha512224_base64	sha512224_hex
	sha512256	sha512256_base64	sha512256_hex);

# Inherit from Digest::base if possible

eval {
	require Digest::base;
	push(@ISA, 'Digest::base');
};

# The following routines aren't time-critical, so they can be left in Perl

sub new {
	my($class, $alg) = @_;
	$alg =~ s/\D+//g if defined $alg;
	if (ref($class)) {	# instance method
		if (!defined($alg) || ($alg == $class->algorithm)) {
			sharewind($class);
			return($class);
		}
		return shainit($class, $alg) ? $class : undef;
	}
	$alg = 1 unless defined $alg;
	return $class->newSHA($alg);
}

BEGIN { *reset = \&new }

sub add_bits {
	my($self, $data, $nbits) = @_;
	unless (defined $nbits) {
		$nbits = length($data);
		$data = pack("B*", $data);
	}
	$nbits = length($data) * 8 if $nbits > length($data) * 8;
	shawrite($data, $nbits, $self);
	return($self);
}

sub _bail {
	my $msg = shift;

	$errmsg = $!;
	$msg .= ": $!";
	require Carp;
	Carp::croak($msg);
}

{
	my $_can_T_filehandle;

	sub _istext {
		local *FH = shift;
		my $file = shift;

		if (! defined $_can_T_filehandle) {
			local $^W = 0;
			my $istext = eval { -T FH };
			$_can_T_filehandle = $@ ? 0 : 1;
			return $_can_T_filehandle ? $istext : -T $file;
		}
		return $_can_T_filehandle ? -T FH : -T $file;
	}
}

sub _addfile {
	my ($self, $handle) = @_;

	my $n;
	my $buf = "";

	while (($n = read($handle, $buf, 4096))) {
		$self->add($buf);
	}
	_bail("Read failed") unless defined $n;

	$self;
}

sub addfile {
	my ($self, $file, $mode) = @_;

	return(_addfile($self, $file)) unless ref(\$file) eq 'SCALAR';

	$mode = defined($mode) ? $mode : "";
	my ($binary, $UNIVERSAL, $BITS) =
		map { $_ eq $mode } ("b", "U", "0");

		## Always interpret "-" to mean STDIN; otherwise use
		##	sysopen to handle full range of POSIX file names.
		## If $file is a directory, force an EISDIR error
		##	by attempting to open with mode O_RDWR

	local *FH;
	if ($file eq '-') {
		if (-d STDIN) {
			sysopen(FH, getcwd(), O_RDWR)
				or _bail('Open failed');
		}
		open(FH, '< -')
			or _bail('Open failed');
	}
	else {
		sysopen(FH, $file, -d $file ? O_RDWR : O_RDONLY)
			or _bail('Open failed');
	}

	if ($BITS) {
		my ($n, $buf) = (0, "");
		while (($n = read(FH, $buf, 4096))) {
			$buf =~ tr/01//cd;
			$self->add_bits($buf);
		}
		_bail("Read failed") unless defined $n;
		close(FH);
		return($self);
	}

	binmode(FH) if $binary || $UNIVERSAL;
	if ($UNIVERSAL && _istext(*FH, $file)) {
		$self->_addfileuniv(*FH);
	}
	else { $self->_addfilebin(*FH) }
	close(FH);

	$self;
}

sub getstate {
	my $self = shift;

	my $alg = $self->algorithm or return;
	my $state = $self->_getstate or return;
	my $nD = $alg <= 256 ?  8 :  16;
	my $nH = $alg <= 256 ? 32 :  64;
	my $nB = $alg <= 256 ? 64 : 128;
	my($H, $block, $blockcnt, $lenhh, $lenhl, $lenlh, $lenll) =
		$state =~ /^(.{$nH})(.{$nB})(.{4})(.{4})(.{4})(.{4})(.{4})$/s;
	for ($alg, $H, $block, $blockcnt, $lenhh, $lenhl, $lenlh, $lenll) {
		return unless defined $_;
	}

	my @s = ();
	push(@s, "alg:" . $alg);
	push(@s, "H:" . join(":", unpack("H*", $H) =~ /.{$nD}/g));
	push(@s, "block:" . join(":", unpack("H*", $block) =~ /.{2}/g));
	push(@s, "blockcnt:" . unpack("N", $blockcnt));
	push(@s, "lenhh:" . unpack("N", $lenhh));
	push(@s, "lenhl:" . unpack("N", $lenhl));
	push(@s, "lenlh:" . unpack("N", $lenlh));
	push(@s, "lenll:" . unpack("N", $lenll));
	join("\n", @s) . "\n";
}

sub putstate {
	my($class, $state) = @_;

	my %s = ();
	for (split(/\n/, $state)) {
		s/^\s+//;
		s/\s+$//;
		next if (/^(#|$)/);
		my @f = split(/[:\s]+/);
		my $tag = shift(@f);
		$s{$tag} = join('', @f);
	}

	# H and block may contain arbitrary values, but check everything else
	grep { $_ == $s{'alg'} } (1,224,256,384,512,512224,512256) or return;
	length($s{'H'}) == ($s{'alg'} <= 256 ? 64 : 128) or return;
	length($s{'block'}) == ($s{'alg'} <= 256 ? 128 : 256) or return;
	{
		no integer;
		for (qw(blockcnt lenhh lenhl lenlh lenll)) {
			0 <= $s{$_} or return;
			$s{$_} <= 4294967295 or return;
		}
		$s{'blockcnt'} < ($s{'alg'} <= 256 ? 512 : 1024) or return;
	}

	my $packed_state = (
		pack("H*", $s{'H'}) .
		pack("H*", $s{'block'}) .
		pack("N",  $s{'blockcnt'}) .
		pack("N",  $s{'lenhh'}) .
		pack("N",  $s{'lenhl'}) .
		pack("N",  $s{'lenlh'}) .
		pack("N",  $s{'lenll'})
	);

	return $class->new($s{'alg'})->_putstate($packed_state);
}

sub dump {
	my $self = shift;
	my $file = shift;

	my $state = $self->getstate or return;
	$file = "-" if (!defined($file) || $file eq "");

	local *FH;
	open(FH, "> $file") or return;
	print FH $state;
	close(FH);

	return($self);
}

sub load {
	my $class = shift;
	my $file = shift;

	$file = "-" if (!defined($file) || $file eq "");

	local *FH;
	open(FH, "< $file") or return;
	my $str = join('', <FH>);
	close(FH);

	$class->putstate($str);
}

eval {
	require XSLoader;
	XSLoader::load('Digest::SHA', $VERSION);
	1;
} or do {
	require DynaLoader;
	push @ISA, 'DynaLoader';
	Digest::SHA->bootstrap($VERSION);
};

1;
__END__

=head1 NAME

Digest::SHA - Perl extension for SHA-1/224/256/384/512

=head1 SYNOPSIS

In programs:

		# Functional interface

	use Digest::SHA qw(sha1 sha1_hex sha1_base64 ...);

	$digest = sha1($data);
	$digest = sha1_hex($data);
	$digest = sha1_base64($data);

	$digest = sha256($data);
	$digest = sha384_hex($data);
	$digest = sha512_base64($data);

		# Object-oriented

	use Digest::SHA;

	$sha = Digest::SHA->new($alg);

	$sha->add($data);		# feed data into stream

	$sha->addfile(*F);
	$sha->addfile($filename);

	$sha->add_bits($bits);
	$sha->add_bits($data, $nbits);

	$sha_copy = $sha->clone;	# make copy of digest object
	$state = $sha->getstate;	# save current state to string
	$sha->putstate($state);		# restore previous $state

	$digest = $sha->digest;		# compute digest
	$digest = $sha->hexdigest;
	$digest = $sha->b64digest;

From the command line:

	$ shasum files

	$ shasum --help

=head1 SYNOPSIS (HMAC-SHA)

		# Functional interface only

	use Digest::SHA qw(hmac_sha1 hmac_sha1_hex ...);

	$digest = hmac_sha1($data, $key);
	$digest = hmac_sha224_hex($data, $key);
	$digest = hmac_sha256_base64($data, $key);

=head1 ABSTRACT

Digest::SHA is a complete implementation of the NIST Secure Hash Standard.
It gives Perl programmers a convenient way to calculate SHA-1, SHA-224,
SHA-256, SHA-384, SHA-512, SHA-512/224, and SHA-512/256 message digests.
The module can handle all types of input, including partial-byte data.

=head1 DESCRIPTION

Digest::SHA is written in C for speed.  If your platform lacks a
C compiler, you can install the functionally equivalent (but much
slower) L<Digest::SHA::PurePerl> module.

The programming interface is easy to use: it's the same one found
in CPAN's L<Digest> module.  So, if your applications currently
use L<Digest::MD5> and you'd prefer the stronger security of SHA,
it's a simple matter to convert them.

The interface provides two ways to calculate digests:  all-at-once,
or in stages.  To illustrate, the following short program computes
the SHA-256 digest of "hello world" using each approach:

	use Digest::SHA qw(sha256_hex);

	$data = "hello world";
	@frags = split(//, $data);

	# all-at-once (Functional style)
	$digest1 = sha256_hex($data);

	# in-stages (OOP style)
	$state = Digest::SHA->new(256);
	for (@frags) { $state->add($_) }
	$digest2 = $state->hexdigest;

	print $digest1 eq $digest2 ?
		"whew!\n" : "oops!\n";

To calculate the digest of an n-bit message where I<n> is not a
multiple of 8, use the I<add_bits()> method.  For example, consider
the 446-bit message consisting of the bit-string "110" repeated
148 times, followed by "11".  Here's how to display its SHA-1
digest:

	use Digest::SHA;
	$bits = "110" x 148 . "11";
	$sha = Digest::SHA->new(1)->add_bits($bits);
	print $sha->hexdigest, "\n";

Note that for larger bit-strings, it's more efficient to use the
two-argument version I<add_bits($data, $nbits)>, where I<$data> is
in the customary packed binary format used for Perl strings.

The module also lets you save intermediate SHA states to a string.  The
I<getstate()> method generates portable, human-readable text describing
the current state of computation.  You can subsequently restore that
state with I<putstate()> to resume where the calculation left off.

To see what a state description looks like, just run the following:

	use Digest::SHA;
	print Digest::SHA->new->add("Shaw" x 1962)->getstate;

As an added convenience, the Digest::SHA module offers routines to
calculate keyed hashes using the HMAC-SHA-1/224/256/384/512
algorithms.  These services exist in functional form only, and
mimic the style and behavior of the I<sha()>, I<sha_hex()>, and
I<sha_base64()> functions.

	# Test vector from draft-ietf-ipsec-ciph-sha-256-01.txt

	use Digest::SHA qw(hmac_sha256_hex);
	print hmac_sha256_hex("Hi There", chr(0x0b) x 32), "\n";

=head1 UNICODE AND SIDE EFFECTS

Perl supports Unicode strings as of version 5.6.  Such strings may
contain wide characters, namely, characters whose ordinal values are
greater than 255.  This can cause problems for digest algorithms such
as SHA that are specified to operate on sequences of bytes.

The rule by which Digest::SHA handles a Unicode string is easy
to state, but potentially confusing to grasp: the string is interpreted
as a sequence of byte values, where each byte value is equal to the
ordinal value (viz. code point) of its corresponding Unicode character.
That way, the Unicode string 'abc' has exactly the same digest value as
the ordinary string 'abc'.

Since a wide character does not fit into a byte, the Digest::SHA
routines croak if they encounter one.  Whereas if a Unicode string
contains no wide characters, the module accepts it quite happily.
The following code illustrates the two cases:

	$str1 = pack('U*', (0..255));
	print sha1_hex($str1);		# ok

	$str2 = pack('U*', (0..256));
	print sha1_hex($str2);		# croaks

Be aware that the digest routines silently convert UTF-8 input into its
equivalent byte sequence in the native encoding (cf. utf8::downgrade).
This side effect influences only the way Perl stores the data internally,
but otherwise leaves the actual value of the data intact.

=head1 NIST STATEMENT ON SHA-1

NIST acknowledges that the work of Prof. Xiaoyun Wang constitutes a
practical collision attack on SHA-1.  Therefore, NIST encourages the
rapid adoption of the SHA-2 hash functions (e.g. SHA-256) for applications
requiring strong collision resistance, such as digital signatures.

ref. L<http://csrc.nist.gov/groups/ST/hash/statement.html>

=head1 PADDING OF BASE64 DIGESTS

By convention, CPAN Digest modules do B<not> pad their Base64 output.
Problems can occur when feeding such digests to other software that
expects properly padded Base64 encodings.

For the time being, any necessary padding must be done by the user.
Fortunately, this is a simple operation: if the length of a Base64-encoded
digest isn't a multiple of 4, simply append "=" characters to the end
of the digest until it is:

	while (length($b64_digest) % 4) {
		$b64_digest .= '=';
	}

To illustrate, I<sha256_base64("abc")> is computed to be

	ungWv48Bz+pBQUDeXa4iI7ADYaOWF3qctBD/YfIAFa0

which has a length of 43.  So, the properly padded version is

	ungWv48Bz+pBQUDeXa4iI7ADYaOWF3qctBD/YfIAFa0=

=head1 EXPORT

None by default.

=head1 EXPORTABLE FUNCTIONS

Provided your C compiler supports a 64-bit type (e.g. the I<long
long> of C99, or I<__int64> used by Microsoft C/C++), all of these
functions will be available for use.  Otherwise, you won't be able
to perform the SHA-384 and SHA-512 transforms, both of which require
64-bit operations.

I<Functional style>

=over 4

=item B<sha1($data, ...)>

=item B<sha224($data, ...)>

=item B<sha256($data, ...)>

=item B<sha384($data, ...)>

=item B<sha512($data, ...)>

=item B<sha512224($data, ...)>

=item B<sha512256($data, ...)>

Logically joins the arguments into a single string, and returns
its SHA-1/224/256/384/512 digest encoded as a binary string.

=item B<sha1_hex($data, ...)>

=item B<sha224_hex($data, ...)>

=item B<sha256_hex($data, ...)>

=item B<sha384_hex($data, ...)>

=item B<sha512_hex($data, ...)>

=item B<sha512224_hex($data, ...)>

=item B<sha512256_hex($data, ...)>

Logically joins the arguments into a single string, and returns
its SHA-1/224/256/384/512 digest encoded as a hexadecimal string.

=item B<sha1_base64($data, ...)>

=item B<sha224_base64($data, ...)>

=item B<sha256_base64($data, ...)>

=item B<sha384_base64($data, ...)>

=item B<sha512_base64($data, ...)>

=item B<sha512224_base64($data, ...)>

=item B<sha512256_base64($data, ...)>

Logically joins the arguments into a single string, and returns
its SHA-1/224/256/384/512 digest encoded as a Base64 string.

It's important to note that the resulting string does B<not> contain
the padding characters typical of Base64 encodings.  This omission is
deliberate, and is done to maintain compatibility with the family of
CPAN Digest modules.  See L</"PADDING OF BASE64 DIGESTS"> for details.

=back

I<OOP style>

=over 4

=item B<new($alg)>

Returns a new Digest::SHA object.  Allowed values for I<$alg> are 1,
224, 256, 384, 512, 512224, or 512256.  It's also possible to use
common string representations of the algorithm (e.g. "sha256",
"SHA-384").  If the argument is missing, SHA-1 will be used by
default.

Invoking I<new> as an instance method will reset the object to the
initial state associated with I<$alg>.  If the argument is missing,
the object will continue using the same algorithm that was selected
at creation.

=item B<reset($alg)>

This method has exactly the same effect as I<new($alg)>.  In fact,
I<reset> is just an alias for I<new>.

=item B<hashsize>

Returns the number of digest bits for this object.  The values are
160, 224, 256, 384, 512, 224, and 256 for SHA-1, SHA-224, SHA-256,
SHA-384, SHA-512, SHA-512/224 and SHA-512/256, respectively.

=item B<algorithm>

Returns the digest algorithm for this object.  The values are 1,
224, 256, 384, 512, 512224, and 512256 for SHA-1, SHA-224, SHA-256,
SHA-384, SHA-512, SHA-512/224, and SHA-512/256, respectively.

=item B<clone>

Returns a duplicate copy of the object.

=item B<add($data, ...)>

Logically joins the arguments into a single string, and uses it to
update the current digest state.  In other words, the following
statements have the same effect:

	$sha->add("a"); $sha->add("b"); $sha->add("c");
	$sha->add("a")->add("b")->add("c");
	$sha->add("a", "b", "c");
	$sha->add("abc");

The return value is the updated object itself.

=item B<add_bits($data, $nbits)>

=item B<add_bits($bits)>

Updates the current digest state by appending bits to it.  The
return value is the updated object itself.

The first form causes the most-significant I<$nbits> of I<$data>
to be appended to the stream.  The I<$data> argument is in the
customary binary format used for Perl strings.

The second form takes an ASCII string of "0" and "1" characters as
its argument.  It's equivalent to

	$sha->add_bits(pack("B*", $bits), length($bits));

So, the following two statements do the same thing:

	$sha->add_bits("111100001010");
	$sha->add_bits("\xF0\xA0", 12);

Note that SHA-1 and SHA-2 use I<most-significant-bit ordering>
for their internal state.  This means that

	$sha3->add_bits("110");

is equivalent to

	$sha3->add_bits("1")->add_bits("1")->add_bits("0");

=item B<addfile(*FILE)>

Reads from I<FILE> until EOF, and appends that data to the current
state.  The return value is the updated object itself.

=item B<addfile($filename [, $mode])>

Reads the contents of I<$filename>, and appends that data to the current
state.  The return value is the updated object itself.

By default, I<$filename> is simply opened and read; no special modes
or I/O disciplines are used.  To change this, set the optional I<$mode>
argument to one of the following values:

	"b"	read file in binary mode

	"U"	use universal newlines

	"0"	use BITS mode

The "U" mode is modeled on Python's "Universal Newlines" concept, whereby
DOS and Mac OS line terminators are converted internally to UNIX newlines
before processing.  This ensures consistent digest values when working
simultaneously across multiple file systems.  B<The "U" mode influences
only text files>, namely those passing Perl's I<-T> test; binary files
are processed with no translation whatsoever.

The BITS mode ("0") interprets the contents of I<$filename> as a logical
stream of bits, where each ASCII '0' or '1' character represents a 0 or
1 bit, respectively.  All other characters are ignored.  This provides
a convenient way to calculate the digest values of partial-byte data
by using files, rather than having to write separate programs employing
the I<add_bits> method.

=item B<getstate>

Returns a string containing a portable, human-readable representation
of the current SHA state.

=item B<putstate($str)>

Returns a Digest::SHA object representing the SHA state contained
in I<$str>.  The format of I<$str> matches the format of the output
produced by method I<getstate>.  If called as a class method, a new
object is created; if called as an instance method, the object is reset
to the state contained in I<$str>.

=item B<dump($filename)>

Writes the output of I<getstate> to I<$filename>.  If the argument is
missing, or equal to the empty string, the state information will be
written to STDOUT.

=item B<load($filename)>

Returns a Digest::SHA object that results from calling I<putstate> on
the contents of I<$filename>.  If the argument is missing, or equal to
the empty string, the state information will be read from STDIN.

=item B<digest>

Returns the digest encoded as a binary string.

Note that the I<digest> method is a read-once operation. Once it
has been performed, the Digest::SHA object is automatically reset
in preparation for calculating another digest value.  Call
I<$sha-E<gt>clone-E<gt>digest> if it's necessary to preserve the
original digest state.

=item B<hexdigest>

Returns the digest encoded as a hexadecimal string.

Like I<digest>, this method is a read-once operation.  Call
I<$sha-E<gt>clone-E<gt>hexdigest> if it's necessary to preserve
the original digest state.

=item B<b64digest>

Returns the digest encoded as a Base64 string.

Like I<digest>, this method is a read-once operation.  Call
I<$sha-E<gt>clone-E<gt>b64digest> if it's necessary to preserve
the original digest state.

It's important to note that the resulting string does B<not> contain
the padding characters typical of Base64 encodings.  This omission is
deliberate, and is done to maintain compatibility with the family of
CPAN Digest modules.  See L</"PADDING OF BASE64 DIGESTS"> for details.

=back

I<HMAC-SHA-1/224/256/384/512>

=over 4

=item B<hmac_sha1($data, $key)>

=item B<hmac_sha224($data, $key)>

=item B<hmac_sha256($data, $key)>

=item B<hmac_sha384($data, $key)>

=item B<hmac_sha512($data, $key)>

=item B<hmac_sha512224($data, $key)>

=item B<hmac_sha512256($data, $key)>

Returns the HMAC-SHA-1/224/256/384/512 digest of I<$data>/I<$key>,
with the result encoded as a binary string.  Multiple I<$data>
arguments are allowed, provided that I<$key> is the last argument
in the list.

=item B<hmac_sha1_hex($data, $key)>

=item B<hmac_sha224_hex($data, $key)>

=item B<hmac_sha256_hex($data, $key)>

=item B<hmac_sha384_hex($data, $key)>

=item B<hmac_sha512_hex($data, $key)>

=item B<hmac_sha512224_hex($data, $key)>

=item B<hmac_sha512256_hex($data, $key)>

Returns the HMAC-SHA-1/224/256/384/512 digest of I<$data>/I<$key>,
with the result encoded as a hexadecimal string.  Multiple I<$data>
arguments are allowed, provided that I<$key> is the last argument
in the list.

=item B<hmac_sha1_base64($data, $key)>

=item B<hmac_sha224_base64($data, $key)>

=item B<hmac_sha256_base64($data, $key)>

=item B<hmac_sha384_base64($data, $key)>

=item B<hmac_sha512_base64($data, $key)>

=item B<hmac_sha512224_base64($data, $key)>

=item B<hmac_sha512256_base64($data, $key)>

Returns the HMAC-SHA-1/224/256/384/512 digest of I<$data>/I<$key>,
with the result encoded as a Base64 string.  Multiple I<$data>
arguments are allowed, provided that I<$key> is the last argument
in the list.

It's important to note that the resulting string does B<not> contain
the padding characters typical of Base64 encodings.  This omission is
deliberate, and is done to maintain compatibility with the family of
CPAN Digest modules.  See L</"PADDING OF BASE64 DIGESTS"> for details.

=back

=head1 SEE ALSO

L<Digest>, L<Digest::SHA::PurePerl>

The Secure Hash Standard (Draft FIPS PUB 180-4) can be found at:

L<http://csrc.nist.gov/publications/drafts/fips180-4/Draft-FIPS180-4_Feb2011.pdf>

The Keyed-Hash Message Authentication Code (HMAC):

L<http://csrc.nist.gov/publications/fips/fips198/fips-198a.pdf>

=head1 AUTHOR

	Mark Shelor	<mshelor@cpan.org>

=head1 ACKNOWLEDGMENTS

The author is particularly grateful to

	Gisle Aas
	H. Merijn Brand
	Sean Burke
	Chris Carey
	Alexandr Ciornii
	Chris David
	Jim Doble
	Thomas Drugeon
	Julius Duque
	Jeffrey Friedl
	Robert Gilmour
	Brian Gladman
	Jarkko Hietaniemi
	Adam Kennedy
	Mark Lawrence
	Andy Lester
	Alex Muntada
	Steve Peters
	Chris Skiscim
	Martin Thurn
	Gunnar Wolf
	Adam Woodbury

"who by trained skill rescued life from such great billows and such thick
darkness and moored it in so perfect a calm and in so brilliant a light"
- Lucretius

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2003-2022 Mark Shelor

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself.

L<perlartistic>

=cut
