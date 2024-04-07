use strict;
use warnings;
BEGIN { 'warnings'->unimport('utf8') if $] < 5.014 }; # turn off 'UTF-16 surrogate 0xd800' warnings

use Test::More;
use Encode qw(encode decode FB_CROAK LEAVE_SRC);

my $script = quotemeta $0;

plan tests => 12;

my @invalid;

ok ! defined eval { encode('UTF-8', "\x{D800}", FB_CROAK | LEAVE_SRC) }, 'Surrogate codepoint \x{D800} is not encoded to strict UTF-8';
like $@, qr/^"\\x\{d800\}" does not map to UTF-8 at $script line /, 'Error message contains strict UTF-8 name';
@invalid = ();
encode('UTF-8', "\x{D800}", sub { @invalid = @_; return ""; });
is_deeply \@invalid, [ 0xD800 ], 'Fallback coderef contains invalid codepoint 0xD800';

ok ! defined eval { decode('UTF-8', "\xed\xa0\x80", FB_CROAK | LEAVE_SRC) }, 'Surrogate UTF-8 byte sequence \xED\xA0\x80 is decoded with strict UTF-8 decoder';
like $@, qr/^UTF-8 "\\xED\\xA0\\x80" does not map to Unicode at $script line /, 'Error message contains strict UTF-8 name and original (not decoded) invalid sequence';
@invalid = ();
decode('UTF-8', "\xed\xa0\x80", sub { @invalid = @_; return ""; });
is_deeply \@invalid, [ 0xED, 0xA0, 0x80 ], 'Fallback coderef contains invalid byte sequence 0xED, 0xA0, 0x80';

ok ! defined eval { decode('UTF-8', "\xed\xa0", FB_CROAK | LEAVE_SRC) }, 'Invalid byte sequence \xED\xA0 is not decoded with strict UTF-8 decoder';
like $@, qr/^UTF-8 "\\xED\\xA0" does not map to Unicode at $script line /, 'Error message contains strict UTF-8 name and original (not decoded) invalid sequence';
@invalid = ();
decode('UTF-8', "\xed\xa0", sub { @invalid = @_; return ""; });
is_deeply \@invalid, [ 0xED, 0xA0 ], 'Fallback coderef contains invalid byte sequence 0xED, 0xA0';

ok ! defined eval { decode('utf8', "\xed\xa0", FB_CROAK | LEAVE_SRC) }, 'Invalid byte sequence \xED\xA0 is not decoded with non-strict utf8 decoder';
like $@, qr/^utf8 "\\xED\\xA0" does not map to Unicode at $script line /, 'Error message contains non-strict utf8 name and original (not decoded) invalid sequence';
decode('utf8', "\xed\xa0", sub { @invalid = @_; return ""; });
is_deeply \@invalid, [ 0xED, 0xA0 ], 'Fallback coderef contains invalid byte sequence 0xED, 0xA0';
