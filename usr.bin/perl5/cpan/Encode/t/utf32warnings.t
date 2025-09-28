BEGIN {
    if ( $] < 5.009 ) {
        print "1..0 # Skip: Perl <= 5.9 or later required\n";
        exit 0;
    }
}
use strict;
use warnings;

my $script = quotemeta $0;

use Encode;
use Test::More tests => 38;

my $valid   = "\x61\x00\x00\x00";
my $invalid = "\x78\x56\x34\x12";

our $warn;
$SIG{__WARN__} = sub { $warn = $_[0] };

my $enc = find_encoding("UTF32-LE");

{
    local $warn;
    my $ret = $enc->encode( "a", Encode::WARN_ON_ERR | Encode::ONLY_PRAGMA_WARNINGS | Encode::LEAVE_SRC );
    is($warn, undef, "Calling encode on UTF32-LE encode object with valid string produces no warnings");
    is($ret, $valid, "Calling encode on UTF32-LE encode object with valid string returns correct output");
}


{
    local $warn;
    $enc->encode( "\x{D800}", Encode::WARN_ON_ERR | Encode::ONLY_PRAGMA_WARNINGS | Encode::LEAVE_SRC );
    like($warn, qr/UTF-16 surrogate.* at $script line /, "Calling encode on UTF32-LE encode object with invalid string warns");
}

{
    local $warn;
    no warnings 'utf8';
    $enc->encode( "\x{D800}", Encode::WARN_ON_ERR | Encode::ONLY_PRAGMA_WARNINGS | Encode::LEAVE_SRC );
    is($warn, undef, "Warning from encode method of UTF32-LE encode object can be silenced via no warnings 'utf8'");
}

{
    local $warn;
    no warnings;
    $enc->encode( "\x{D800}", Encode::WARN_ON_ERR | Encode::ONLY_PRAGMA_WARNINGS | Encode::LEAVE_SRC );
    is($warn, undef, "Warning from encode method of UTF32-LE encode object can be silenced via no warnings");
}

{
    local $warn;
    no warnings 'utf8';
    $enc->encode( "\x{D800}", Encode::WARN_ON_ERR | Encode::LEAVE_SRC );
    like($warn, qr/UTF-16 surrogate.* at $script line /, "Warning from encode method of UTF32-LE encode object cannot be silenced via no warnings 'utf8' when ONLY_PRAGMA_WARNINGS is not used");
}

{
    local $warn;
    no warnings;
    $enc->encode( "\x{D800}", Encode::WARN_ON_ERR | Encode::LEAVE_SRC );
    like($warn, qr/UTF-16 surrogate.* at $script line /, "Warning from encode method of UTF32-LE encode object cannot be silenced via no warnings when ONLY_PRAGMA_WARNINGS is not used");
}


{
    local $warn;
    my $ret = Encode::encode( $enc, "a", Encode::WARN_ON_ERR | Encode::ONLY_PRAGMA_WARNINGS | Encode::LEAVE_SRC );
    is($warn, undef, "Calling Encode::encode for UTF32-LE with valid string produces no warnings");
    is($ret, $valid, "Calling Encode::encode for UTF32-LE with valid string returns correct output");
}


{
    local $warn;
    Encode::encode( $enc, "\x{D800}", Encode::WARN_ON_ERR | Encode::ONLY_PRAGMA_WARNINGS | Encode::LEAVE_SRC );
    like($warn, qr/UTF-16 surrogate.* at $script line /, "Calling Encode::encode for UTF32-LE with invalid string warns");
}


{
    local $warn;
    no warnings 'utf8';
    Encode::encode( $enc, "\x{D800}", Encode::WARN_ON_ERR | Encode::ONLY_PRAGMA_WARNINGS | Encode::LEAVE_SRC );
    is($warn, undef, "Warning from Encode::encode for UTF32-LE can be silenced via no warnings 'utf8'");
}

{
    local $warn;
    no warnings;
    Encode::encode( $enc, "\x{D800}", Encode::WARN_ON_ERR | Encode::ONLY_PRAGMA_WARNINGS | Encode::LEAVE_SRC );
    is($warn, undef, "Warning from Encode::encode for UTF32-LE can be silenced via no warnings");
}

{
    local $warn;
    no warnings 'utf8';
    Encode::encode( $enc, "\x{D800}", Encode::WARN_ON_ERR | Encode::LEAVE_SRC );
    like($warn, qr/UTF-16 surrogate.* at $script line /, "Warning from Encode::encode for UTF32-LE cannot be silenced via no warnings 'utf8' when ONLY_PRAGMA_WARNINGS is not used");
}

{
    local $warn;
    no warnings;
    Encode::encode( $enc, "\x{D800}", Encode::WARN_ON_ERR | Encode::LEAVE_SRC );
    like($warn, qr/UTF-16 surrogate.* at $script line /, "Warning from Encode::encode for UTF32-LE cannot be silenced via no warnings when ONLY_PRAGMA_WARNINGS is not used");
}


{
    local $warn;
    my $ret = $enc->decode( $valid, Encode::WARN_ON_ERR | Encode::ONLY_PRAGMA_WARNINGS | Encode::LEAVE_SRC );
    is($warn, undef, "Calling decode on UTF32-LE encode object with valid string produces no warnings");
    is($ret, "a", "Calling decode on UTF32-LE encode object with valid string returns correct output");
}


{
    local $warn;
    $enc->decode( $invalid, Encode::WARN_ON_ERR | Encode::ONLY_PRAGMA_WARNINGS | Encode::LEAVE_SRC );
    like($warn, qr/may not be portable.* at $script line /, "Calling decode on UTF32-LE encode object with invalid string warns");
}

{
    local $warn;
    no warnings 'utf8';
    $enc->decode( $invalid, Encode::WARN_ON_ERR | Encode::ONLY_PRAGMA_WARNINGS | Encode::LEAVE_SRC );
    is($warn, undef, "Warning from decode method of UTF32-LE encode object can be silenced via no warnings 'utf8'");
}

{
    local $warn;
    no warnings;
    $enc->decode( $invalid, Encode::WARN_ON_ERR | Encode::ONLY_PRAGMA_WARNINGS | Encode::LEAVE_SRC );
    is($warn, undef, "Warning from decode method of UTF32-LE encode object can be silenced via no warnings");
}

{
    local $warn;
    no warnings 'utf8';
    $enc->decode( $invalid, Encode::WARN_ON_ERR | Encode::LEAVE_SRC );
    like($warn, qr/may not be portable.* at $script line /, "Warning from decode method of UTF32-LE encode object cannot be silenced via no warnings 'utf8' when ONLY_PRAGMA_WARNINGS is not used");
}

{
    local $warn;
    no warnings;
    $enc->decode( $invalid, Encode::WARN_ON_ERR | Encode::LEAVE_SRC );
    like($warn, qr/may not be portable.* at $script line /, "Warning from decode method of UTF32-LE encode object cannot be silenced via no warnings when ONLY_PRAGMA_WARNINGS is not used");
}


{
    local $warn;
    my $ret = Encode::decode( $enc, $valid, Encode::WARN_ON_ERR | Encode::ONLY_PRAGMA_WARNINGS | Encode::LEAVE_SRC );
    is($warn, undef, "Calling Encode::decode for UTF32-LE with valid string produces no warnings");
    is($ret, "a", "Calling Encode::decode for UTF32-LE with valid string returns correct output");
}


{
    local $warn;
    Encode::decode( $enc, $invalid, Encode::WARN_ON_ERR | Encode::ONLY_PRAGMA_WARNINGS | Encode::LEAVE_SRC );
    like($warn, qr/may not be portable.* at $script line /, "Calling Encode::decode for UTF32-LE with invalid string warns");
}

{
    local $warn;
    no warnings 'utf8';
    Encode::decode( $enc, $invalid, Encode::WARN_ON_ERR | Encode::ONLY_PRAGMA_WARNINGS | Encode::LEAVE_SRC );
    is($warn, undef, "Warning from Encode::decode for UTF32-LE can be silenced via no warnings 'utf8'");
}

{
    local $warn;
    no warnings;
    Encode::decode( $enc, $invalid, Encode::WARN_ON_ERR | Encode::ONLY_PRAGMA_WARNINGS | Encode::LEAVE_SRC );
    is($warn, undef, "Warning from Encode::decode for UTF32-LE can be silenced via no warnings");
}

{
    local $warn;
    no warnings 'utf8';
    Encode::decode( $enc, $invalid, Encode::WARN_ON_ERR | Encode::LEAVE_SRC );
    like($warn, qr/may not be portable.* at $script line /, "Warning from Encode::decode for UTF32-LE cannot be silenced via no warnings 'utf8' when ONLY_PRAGMA_WARNINGS is not used");
}

{
    local $warn;
    no warnings;
    Encode::decode( $enc, $invalid, Encode::WARN_ON_ERR | Encode::LEAVE_SRC );
    like($warn, qr/may not be portable.* at $script line /, "Warning from Encode::decode for UTF32-LE cannot be silenced via no warnings when ONLY_PRAGMA_WARNINGS is not used");
}


use PerlIO::encoding;
$PerlIO::encoding::fallback |= Encode::ONLY_PRAGMA_WARNINGS;

{
    local $warn;
    my $tmp = $valid;
    $tmp .= ''; # de-COW
    open my $fh, '<:encoding(UTF32-LE)', \$tmp or die;
    my $str = <$fh>;
    close $fh;
    is($warn, undef, "Calling PerlIO :encoding on valid string produces no warnings");
    is($str, "a", "PerlIO decodes string correctly");
}


{
    local $warn;
    my $tmp = $invalid;
    use Devel::Peek;
    $tmp .= ''; # de-COW
    open my $fh, '<:encoding(UTF32-LE)', \$tmp or die;
    my $str = <$fh>;
    close $fh;
    like($warn, qr/may not be portable.* at $script line /, "Calling PerlIO :encoding on invalid string warns");
}

{
    local $warn;
    my $tmp = $invalid;
    $tmp .= ''; # de-COW
    no warnings 'utf8';
    open my $fh, '<:encoding(UTF32-LE)', \$tmp or die;
    my $str = <$fh>;
    close $fh;
    is($warn, undef, "Warning from PerlIO :encoding can be silenced via no warnings 'utf8'");
}

{
    local $warn;
    my $tmp = $invalid;
    $tmp .= ''; # de-COW
    no warnings;
    open my $fh, '<:encoding(UTF32-LE)', \$tmp or die;
    my $str = <$fh>;
    close $fh;
    is($warn, undef, "Warning from PerlIO :encoding can be silenced via no warnings");
}


{
    local $warn;
    my $str;
    open my $fh, '>:encoding(UTF32-LE)', \$str or die;
    print $fh "a";
    close $fh;
    is($warn, undef, "Calling PerlIO :encoding on valid string produces no warnings");
    is($str, $valid, "PerlIO encodes string correctly");
}


{
    local $warn;
    my $str;
    open my $fh, '>:encoding(UTF32-LE)', \$str or die;
    print $fh "\x{D800}";
    close $fh;
    like($warn, qr/UTF-16 surrogate.* at $script line /, "Calling PerlIO :encoding on invalid string warns");
}

{
    local $warn;
    my $str;
    no warnings 'utf8';
    open my $fh, '>:encoding(UTF32-LE)', \$str or die;
    print $fh "\x{D800}";
    close $fh;
    is($warn, undef, "Warning from PerlIO :encoding can be silenced via no warnings 'utf8'");
}

{
    local $warn;
    my $str;
    no warnings;
    open my $fh, '>:encoding(UTF32-LE)', \$str or die;
    print $fh "\x{D800}";
    close $fh;
    is($warn, undef, "Warning from PerlIO :encoding can be silenced via no warnings");
}
