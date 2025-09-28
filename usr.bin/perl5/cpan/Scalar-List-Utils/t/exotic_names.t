use strict;
use warnings;

use Test::More;
use B 'svref_2object';
BEGIN { $^P |= 0x210 }

# This is a mess. The stash can supposedly handle Unicode but the behavior
# is literally undefined before 5.16 (with crashes beyond the basic plane),
# and remains unclear past 5.16 with evalbytes and feature unicode_eval
# In any case - Sub::Name needs to *somehow* work with this, so we will do
# a heuristic with ambiguous eval and looking for octets in the stash
use if $] >= 5.016, feature => 'unicode_eval';

if ($] >= 5.008) {
    my $builder = Test::More->builder;
    binmode $builder->output,         ":encoding(utf8)";
    binmode $builder->failure_output, ":encoding(utf8)";
    binmode $builder->todo_output,    ":encoding(utf8)";
}

sub compile_named_sub {
    my ( $fullname, $body ) = @_;
    my $sub = eval "sub $fullname { $body }" . '\\&{$fullname}';
    return $sub if $sub;
    my $e = $@;
    require Carp;
    Carp::croak $e;
}

sub caller3_ok {
    my ( $sub, $expected, $type, $ord ) = @_;

    local $Test::Builder::Level = $Test::Builder::Level + 1;

    my $for_what = sprintf "when it contains \\x%s ( %s )", (
        ( ($ord > 255)
            ? sprintf "{%X}", $ord
            : sprintf "%02X", $ord
        ),
        (
            $ord > 255                    ? unpack('H*', pack 'C0U', $ord )
            : (chr $ord =~ /[[:print:]]/) ? sprintf "%c", $ord
            :                               sprintf '\%o', $ord
        ),
    );

    $expected =~ s/'/::/g;

    # this is apparently how things worked before 5.16
    utf8::encode($expected) if $] < 5.016 and $ord > 255;

    my $stash_name = join '::', map { $_->STASH->NAME, $_->NAME } svref_2object($sub)->GV;

    is $stash_name, $expected, "stash name for $type is correct $for_what";
    is $sub->(), $expected, "caller() in $type returns correct name $for_what";
    SKIP: {
      skip '%DB::sub not populated when enabled at runtime', 1
        unless keys %DB::sub;
      my ($prefix) = $expected =~ /^(.*?test::[^:]+::)/;
      my ($db_found) = grep /^$prefix/, keys %DB::sub;
      is $db_found, $expected, "%DB::sub entry for $type is correct $for_what";
    }
}

#######################################################################

use Sub::Util 'set_subname';

my @ordinal = ( 1 .. 255 );

# 5.14 is the first perl to start properly handling \0 in identifiers
unshift @ordinal, 0
    unless $] < 5.014;

# Unicode in 5.6 is not sane (crashes etc)
push @ordinal,
    0x100,    # LATIN CAPITAL LETTER A WITH MACRON
    0x498,    # CYRILLIC CAPITAL LETTER ZE WITH DESCENDER
    0x2122,   # TRADE MARK SIGN
    0x1f4a9,  # PILE OF POO
    unless $] < 5.008;

plan tests => @ordinal * 2 * 3;

my $legal_ident_char = "A-Z_a-z0-9'";
$legal_ident_char .= join '', map chr, 0x100, 0x498
    unless $] < 5.008;

my $uniq = 'A000';
for my $ord (@ordinal) {
    my $sub;
    $uniq++;
    my $pkg      = sprintf 'test::%s::SOME_%c_STASH', $uniq, $ord;
    my $subname  = sprintf 'SOME_%s_%c_NAME', $uniq, $ord;
    my $fullname = join '::', $pkg, $subname;

    $sub = set_subname $fullname => sub { (caller(0))[3] };
    caller3_ok $sub, $fullname, 'renamed closure', $ord;

    # test that we can *always* compile at least within the correct package
    my $expected;
    if ( chr($ord) =~ m/^[$legal_ident_char]$/o ) { # compile directly
        $expected = "native::$fullname";
        $sub = compile_named_sub $expected => '(caller(0))[3]';
    }
    else { # not a legal identifier but at least test the package name by aliasing
        $expected = "aliased::native::$fullname";
        {
          no strict 'refs';
          *palatable:: = *{"aliased::native::${pkg}::"};
          # now palatable:: literally means aliased::native::${pkg}::
          my $encoded_sub = $subname;
          utf8::encode($encoded_sub) if "$]" < 5.016 and $ord > 255;
          ${"palatable::$encoded_sub"} = 1;
          ${"palatable::"}{"sub"} = ${"palatable::"}{$encoded_sub};
          # and palatable::sub means aliased::native::${pkg}::${subname}
        }
        $sub = compile_named_sub 'palatable::sub' => '(caller(0))[3]';
    }
    caller3_ok $sub, $expected, 'natively compiled sub', $ord;
}
