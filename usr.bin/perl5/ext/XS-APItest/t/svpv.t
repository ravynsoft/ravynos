#!perl -w

BEGIN { require 'charset_tools.pl'; }

use Test::More tests => 43;

use XS::APItest;

for my $func ('SvPVbyte_nolen', 'SvPVutf8_nolen') {
 $g = *glob;
 $r = \1;
 is &$func($g), '*main::glob', "$func(\$glob_copy)";
 is ref\$g, 'GLOB', "$func(\$glob_copy) does not flatten the glob";
 is &$func($r), "$r", "$func(\$ref)";
 is ref\$r, 'REF', "$func(\$ref) does not flatten the ref";

 is &$func(*glob), '*main::glob', "$func(*glob)";
 is ref\$::{glob}, 'GLOB', "$func(*glob) does not flatten the glob";
 is &$func($^V), "$^V", "$func(\$ro_ref)";
 is ref\$^V, 'REF', "$func(\$ro_ref) does not flatten the ref";
}

my $B6 = byte_utf8a_to_utf8n("\xC2\xB6");
my $individual_B6_utf8_bytes = ($::IS_ASCII)
                               ? "\xC3\x82\xC2\xB6"
                               : I8_to_native("\xC6\xB8\xC6\xA1");
my $data_bin = $B6;
utf8::downgrade($data_bin);
tie my $scalar_bin, 'TieScalarCounter', $data_bin;
do { my $fetch = $scalar_bin };
is tied($scalar_bin)->{fetch}, 1;
is tied($scalar_bin)->{store}, 0;
my $len;
is SvPVutf8_nomg($scalar_bin, $len), $individual_B6_utf8_bytes;
is $len, length($individual_B6_utf8_bytes), "check len set by SvPVutf8_nomg";
is tied($scalar_bin)->{fetch}, 1;
is tied($scalar_bin)->{store}, 0;
undef $len;
is SvPVbyte_nomg($scalar_bin, $len), $B6;
is $len, length($B6), "check len set by SvPVbyte_nomg";
is tied($scalar_bin)->{fetch}, 1;
is tied($scalar_bin)->{store}, 0;

my $data_uni = $B6;
utf8::upgrade($data_uni);
tie my $scalar_uni, 'TieScalarCounter', $data_uni;
do { my $fetch = $scalar_uni };
is tied($scalar_uni)->{fetch}, 1;
is tied($scalar_uni)->{store}, 0;
undef $len;
is SvPVbyte_nomg($scalar_uni, $len), $B6;
is $len, length($B6), "cheeck len set by SvPVbyte_nomg";
is tied($scalar_uni)->{fetch}, 1;
is tied($scalar_uni)->{store}, 0;
undef $len;
is SvPVutf8_nomg($scalar_uni, $len), $individual_B6_utf8_bytes;
is $len, length($individual_B6_utf8_bytes), "check len set by SvPVutf8_nomg";
is tied($scalar_uni)->{fetch}, 1;
is tied($scalar_uni)->{store}, 0;

undef $len;
is SvPVutf8($scalar_bin, $len), $individual_B6_utf8_bytes;
is $len, length $individual_B6_utf8_bytes;
undef $len;
is SvPVutf8($scalar_uni, $len), $individual_B6_utf8_bytes;
is $len, length $individual_B6_utf8_bytes, "check len set by SvPVutf8";

eval 'SvPVbyte_nolen(*{chr 256})';
like $@, qr/^Wide character/, 'SvPVbyte_nolen fails on Unicode glob';
package r { use overload '""' => sub { substr "\x{100}\xff", -1 } }
is SvPVbyte_nolen(bless [], r::), "\xff",
  'SvPVbyte_nolen on ref returning downgradable utf8 string';

sub TIESCALAR { bless \(my $thing = pop), shift }
sub FETCH { ${ +shift } }
tie $tyre, main => bless [], r::;
is SvPVbyte_nolen($tyre), "\xff",
  'SvPVbyte on tie returning ref that returns downgradable utf8 string';

package TieScalarCounter;

sub TIESCALAR {
    my ($class, $value) = @_;
    return bless { fetch => 0, store => 0, value => $value }, $class;
}

sub FETCH {
    my ($self) = @_;
    $self->{fetch}++;
    return $self->{value};
}

sub STORE {
    my ($self, $value) = @_;
    $self->{store}++;
    $self->{value} = $value;
}
