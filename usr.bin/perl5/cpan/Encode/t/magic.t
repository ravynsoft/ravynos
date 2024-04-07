BEGIN {
    if ($ENV{'PERL_CORE'}) {
        chdir 't';
        unshift @INC, '../lib';
    }
    require Config; import Config;
    if ($Config{'extensions'} !~ /\bEncode\b/) {
      print "1..0 # Skip: Encode was not built\n";
      exit 0;
    }
    if (ord("A") == 193) {
      print "1..0 # Skip: EBCDIC\n";
      exit 0;
    }
    $| = 1;
}

use strict;
use warnings;

use Encode qw(find_encoding encode decode encode_utf8 decode_utf8 is_utf8 _utf8_on _utf8_off FB_CROAK);

use Test::More tests => 3*(2*(4*(4*4)+4)+4+3*3);

my $ascii = find_encoding('ASCII');
my $latin1 = find_encoding('Latin1');
my $utf8 = find_encoding('UTF-8');
my $utf16 = find_encoding('UTF-16LE');

my $undef = undef;
my $ascii_str = 'ascii_str';
my $utf8_str = 'utf8_str';
_utf8_on($utf8_str);

{
    foreach my $str ($undef, $ascii_str, $utf8_str) {
        foreach my $croak (0, 1) {
            foreach my $enc ('ASCII', 'Latin1', 'UTF-8', 'UTF-16LE') {
                my $mod = defined $str && $croak;
                my $func = "encode('" . $enc . "', " . (!defined $str ? 'undef' : is_utf8($str) ? '$utf8_str' : '$ascii_str') . ($croak ? ', FB_CROAK' : '') . ')';
                tie my $input, 'TieScalarCounter', $str;
                my $output = encode($enc, $input, $croak ? FB_CROAK : 0);
                is(tied($input)->{fetch}, 1, "$func processes get magic only once");
                is(tied($input)->{store}, $mod ? 1 : 0, "$func " . ($mod ? 'processes set magic only once' : 'does not process set magic'));
                is($input, $mod ? '' : $str, "$func " . ($mod ? 'modifies' : 'does not modify') . ' $input string');
                is($output, ((defined $str and $enc eq 'UTF-16LE') ? encode("UTF-16LE", $str) : $str), "$func returns correct \$output string");
            }
            foreach my $enc ('ASCII', 'Latin1', 'UTF-8', 'UTF-16LE') {
                my $mod = defined $str && $croak;
                my $func = "decode('" . $enc . "', " . (!defined $str ? 'undef' : is_utf8($str) ? '$utf8_str' : '$ascii_str') . ($croak ? ', FB_CROAK' : '') . ')';
                my $input_str = ((defined $str and $enc eq 'UTF-16LE') ? encode("UTF-16LE", $str) : $str);
                tie my $input, 'TieScalarCounter', $input_str;
                my $output = decode($enc, $input, $croak ? FB_CROAK : 0);
                is(tied($input)->{fetch}, 1, "$func processes get magic only once");
                is(tied($input)->{store}, $mod ? 1 : 0, "$func " . ($mod ? 'processes set magic only once' : 'does not process set magic'));
                is($input, $mod ? '' : $input_str, "$func " . ($mod ? 'modifies' : 'does not modify') . ' $input string');
                is($output, $str, "$func returns correct \$output string");
            }
            foreach my $obj ($ascii, $latin1, $utf8, $utf16) {
                my $mod = defined $str && $croak;
                my $func = '$' . $obj->name() . '->encode(' . (!defined $str ? 'undef' : is_utf8($str) ? '$utf8_str' : '$ascii_str') . ($croak ? ', FB_CROAK' : '') . ')';
                tie my $input, 'TieScalarCounter', $str;
                my $output = $obj->encode($input, $croak ? FB_CROAK : 0);
                is(tied($input)->{fetch}, 1, "$func processes get magic only once");
                is(tied($input)->{store}, $mod ? 1 : 0, "$func " . ($mod ? 'processes set magic only once' : 'does not process set magic'));
                is($input, $mod ? '' : $str, "$func " . ($mod ? 'modifies' : 'does not modify') . ' $input string');
                is($output, ((defined $str and $obj == $utf16) ? encode("UTF-16LE", $str) : $str), "$func returns correct \$output string");
            }
            foreach my $obj ($ascii, $latin1, $utf8, $utf16) {
                my $mod = defined $str && $croak;
                my $func = '$' . $obj->name() . '->decode(' . (!defined $str ? 'undef' : is_utf8($str) ? '$utf8_str' : '$ascii_str') . ($croak ? ', FB_CROAK' : '') . ')';
                my $input_str = ((defined $str and $obj == $utf16) ? encode("UTF-16LE", $str) : $str);
                tie my $input, 'TieScalarCounter', $input_str;
                my $output = $obj->decode($input, $croak ? FB_CROAK : 0);
                is(tied($input)->{fetch}, 1, "$func processes get magic only once");
                is(tied($input)->{store}, $mod ? 1 : 0, "$func " . ($mod ? 'processes set magic only once' : 'does not process set magic'));
                is($input, $mod ? '' : $input_str, "$func " . ($mod ? 'modifies' : 'does not modify') . ' $input string');
                is($output, $str, "$func returns correct \$output string");
            }
            {
                my $mod = defined $str && $croak;
                my $func = 'decode_utf8(' . (!defined $str ? 'undef' : is_utf8($str) ? '$utf8_str' : '$ascii_str') . ($croak ? ', FB_CROAK' : '') . ')';
                tie my $input, 'TieScalarCounter', $str;
                my $output = decode_utf8($input, $croak ? FB_CROAK : 0);
                is(tied($input)->{fetch}, 1, "$func processes get magic only once");
                is(tied($input)->{store}, $mod ? 1 : 0, "$func " . ($mod ? 'processes set magic only once' : 'does not process set magic'));
                is($input, $mod ? '' : $str, "$func " . ($mod ? 'modifies' : 'does not modify') . ' $input string');
                is($output, $str, "$func returns correct \$output string");
            }
        }
        {
            my $func = 'encode_utf8(' . (!defined $str ? 'undef' : is_utf8($str) ? '$utf8_str' : '$ascii_str') . ')';
            tie my $input, 'TieScalarCounter', $str;
            my $output = encode_utf8($input);
            is(tied($input)->{fetch}, 1, "$func processes get magic only once");
            is(tied($input)->{store}, 0, "$func does not process set magic");
            is($input, $str, "$func does not modify \$input string");
            is($output, $str, "$func returns correct \$output string");
        }
        {
            my $func = '_utf8_on(' . (!defined $str ? 'undef' : is_utf8($str) ? '$utf8_str' : '$ascii_str') . ')';
            tie my $input, 'TieScalarCounter', $str;
            _utf8_on($input);
            is(tied($input)->{fetch}, 1, "$func processes get magic only once");
            is(tied($input)->{store}, defined $str ? 1 : 0, "$func " . (defined $str ? 'processes set magic only once' : 'does not process set magic'));
            defined $str ? ok(is_utf8($input), "$func sets UTF8 status flag") : ok(!is_utf8($input), "$func does not set UTF8 status flag");
        }
        {
            my $func = '_utf8_off(' . (!defined $str ? 'undef' : is_utf8($str) ? '$utf8_str' : '$ascii_str') . ')';
            tie my $input, 'TieScalarCounter', $str;
            _utf8_off($input);
            is(tied($input)->{fetch}, 1, "$func processes get magic only once");
            is(tied($input)->{store}, defined $str ? 1 : 0, "$func " . (defined $str ? 'processes set magic only once' : 'does not process set magic'));
            ok(!is_utf8($input), "$func unsets UTF8 status flag");
        }
        {
            my $func = 'is_utf8(' . (!defined $str ? 'undef' : is_utf8($str) ? '$utf8_str' : '$ascii_str') . ')';
            tie my $input, 'TieScalarCounter', $str;
            my $utf8 = is_utf8($input);
            is(tied($input)->{fetch}, 1, "$func processes get magic only once");
            is(tied($input)->{store}, 0, "$func does not process set magic");
            is($utf8, is_utf8($str), "$func returned correct state");
        }
    }
}

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
