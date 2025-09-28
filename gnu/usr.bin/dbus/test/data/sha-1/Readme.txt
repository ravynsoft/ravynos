Test suite from http://csrc.nist.gov/cryptval/shs.html

				Sample Vectors for SHA-1 Testing

	This file describes tests and vectors that can be used in verifying the correctness of 
an SHA-1 implementation.  However, use of these vectors does not take the place of validation 
obtained through the Cryptographic Module Validation Program.

	There are three areas of the Secure Hash Standard for which test vectors are supplied:
short messages of varying length, selected long messages, and pseudorandomly generated messages.
Since it is possible for an implementation to correctly handle the hashing of byte-oriented
messages (and not messages of a non-byte length), the SHS tests each come in two flavors.  For
both byte oriented and bit oriented messages, the message lengths are given in bits.

Type I Test: Messages of Varying Length

	An implementation of the SHS must be able to correctly generate message digests for
messages of arbitrary length.  This functionality can be tested by supplying the implementation
with 1025 pseudorandomly generated messages with lengths from 0 to 1024 bits (for an implementation
that only hashes byte-oriented data correctly, 129 messages of length 0, 8, 16, 24,...,1024 bits
will be supplied).

Type II Test: Selected Long Messages

	Additional testing of an implementation can be performed by testing that the implementation
can correctly generate digests for longer messages.  A list of 100 messages, each of length > 1024,
is supplied.  These can be used to verify the hashing of longer message lengths.  For bit oriented
testing the messages are from 1025 to 103425 bits long (length=1025+i*1024, where 0<=i<100).  For
byte oriented testing the messages are from 1032 to 103432 (length=1032+i*1024, where 0<=i<100).

Type III Test: Pseudorandomly Generated Messages

	This test determines whether the implementation can compute message digests for messages
that are generated using a given seed.  A sequence of 100 message digests is generated using this
seed.  The digests are generated according to the following pseudocode:

procedure MonteCarlo(string SEED)
{
	integer i, j, a;
	string	M;

	M := SEED;
	for j = 0 to 99 do {
		for i = 1 to 50000 do {
			for a = 1 to (j/4*8 + 24) do M := M || ’0’;	/*‘0' is the binary zero bit. */
			M := M || i; 	/* Here, the value for ‘i’ is expressed as a 32-bit word
					   and concatenated with ‘M’. The first bit
					   concatenated with ‘M’ is the most significant bit of
					   this 32-bit word. */
			M := SHA(M);
			}
		print(M);
		}
	}

NOTE: In the above procedure, || denotes concatenation. Also, M || i denotes appending the 32-bit
word representing the value ‘i’, as defined in section 2 of the SHS.  Within the procedure, M is a string
of variable length. The initial length of 416 bits ensures that the length of M never exceeds 512 bits
during execution of the above procedure, and it ensures that messages will be of a byte length.  Each
element printed should be 160 bits in length.


File formats:

There are two files included for each test type (bit-oriented and byte-oriented).  One file contains
the messages and the other file contains the hashes.

The message files provided use "compact strings" to store the message values.  Compact strings are 
used to represented the messages in a compact form.  A compact string has the form
	z || b || n(1) || n(2) || ... || n(z)
where z>=0 that represents the number of n, b is either 0 or 1, and each n(i) is a decimal integer
representing a positive number.  The length of the compact string is given by the summation of the n(i).

The compact string is interpreted as the representation of the bit string consisting of b repeated n(1) times,
followed by 1-b repeated n(2) times, followed by b repeated n(3) times, and so on.

Example:
	M = 5 1 7 13 5 1 2
	where z = 5 and b = 1.  Then the compact string M represents the bit string
	1111111000000000000011111011
	where 1 is repeated 7 times, 0 is repeated 13 times, 1 is repeated 5 times,
	0 is repeated 1 time, and 1 is repeated 2 times.

