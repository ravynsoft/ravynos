#!./perl -w
# t/quotekeys.t - Test Quotekeys()

use strict;
use warnings;

use Data::Dumper;
use Test::More tests => 18;
use lib qw( ./t/lib );
use Testing qw( _dumptostr );

my %d = (
    delta   => 'd',
    beta    => 'b',
    gamma   => 'c',
    alpha   => 'a',
);

my $is_ascii = ord("A") == 65;

run_tests_for_quotekeys();
SKIP: {
    skip "XS version was unavailable, so we already ran with pure Perl", 5
        if $Data::Dumper::Useperl;
    local $Data::Dumper::Useperl = 1;
    run_tests_for_quotekeys();
}

sub run_tests_for_quotekeys {
    note("\$Data::Dumper::Useperl = $Data::Dumper::Useperl");

    my ($obj, %dumps, $quotekeys, $starting);

    note("\$Data::Dumper::Quotekeys and Quotekeys() set to true value");

    $obj = Data::Dumper->new( [ \%d ] );
    $dumps{'ddqkdefault'} = _dumptostr($obj);

    $starting = $Data::Dumper::Quotekeys;
    $quotekeys = 1;
    local $Data::Dumper::Quotekeys = $quotekeys;
    $obj = Data::Dumper->new( [ \%d ] );
    $dumps{'ddqkone'} = _dumptostr($obj);
    local $Data::Dumper::Quotekeys = $starting;

    $obj = Data::Dumper->new( [ \%d ] );
    $obj->Quotekeys($quotekeys);
    $dumps{'objqkone'} = _dumptostr($obj);

    is($dumps{'ddqkdefault'}, $dumps{'ddqkone'},
        "\$Data::Dumper::Quotekeys = 1 is default");
    is($dumps{'ddqkone'}, $dumps{'objqkone'},
        "\$Data::Dumper::Quotekeys = 1 and Quotekeys(1) are equivalent");
    %dumps = ();

    $quotekeys = 0;
    local $Data::Dumper::Quotekeys = $quotekeys;
    $obj = Data::Dumper->new( [ \%d ] );
    $dumps{'ddqkzero'} = _dumptostr($obj);
    local $Data::Dumper::Quotekeys = $starting;

    $obj = Data::Dumper->new( [ \%d ] );
    $obj->Quotekeys($quotekeys);
    $dumps{'objqkzero'} = _dumptostr($obj);

    is($dumps{'ddqkzero'}, $dumps{'objqkzero'},
        "\$Data::Dumper::Quotekeys = 0 and Quotekeys(0) are equivalent");

    $quotekeys = undef;
    local $Data::Dumper::Quotekeys = $quotekeys;
    $obj = Data::Dumper->new( [ \%d ] );
    $dumps{'ddqkundef'} = _dumptostr($obj);
    local $Data::Dumper::Quotekeys = $starting;

    $obj = Data::Dumper->new( [ \%d ] );
    $obj->Quotekeys($quotekeys);
    $dumps{'objqkundef'} = _dumptostr($obj);

    is($dumps{'ddqkundef'}, $dumps{'objqkundef'},
        "\$Data::Dumper::Quotekeys = undef and Quotekeys(undef) are equivalent");
    is($dumps{'ddqkzero'}, $dumps{'objqkundef'},
        "\$Data::Dumper::Quotekeys = undef and = 0 are equivalent");
    %dumps = ();

    local $Data::Dumper::Quotekeys = 1;
    local $Data::Dumper::Sortkeys = 1;
    local $Data::Dumper::Indent = 0;
    local $Data::Dumper::Useqq = 0;

    my %qkdata =
      (
       0 => 1,
       '012345' => 1,
       12 => 1,
       123456789 => 1,
       1234567890 => 1,
       '::de::fg' => 1,
       ab => 1,
       'hi::12' => 1,
       "1\x{660}" => 1,
      );

    is(Dumper(\%qkdata),
       (($is_ascii) # Sort order is different on EBCDIC platforms
        ? q($VAR1 = {'0' => 1,'012345' => 1,'12' => 1,'123456789' => 1,'1234567890' => 1,"1\x{660}" => 1,'::de::fg' => 1,'ab' => 1,'hi::12' => 1};)
        : q($VAR1 = {'::de::fg' => 1,'ab' => 1,'hi::12' => 1,'0' => 1,'012345' => 1,'12' => 1,'123456789' => 1,'1234567890' => 1,"1\x{660}" => 1};)),
       "always quote when quotekeys true");

    {
        local $Data::Dumper::Useqq = 1;
        is(Dumper(\%qkdata),
           (($is_ascii)
	    ? q($VAR1 = {"0" => 1,"012345" => 1,"12" => 1,"123456789" => 1,"1234567890" => 1,"1\x{660}" => 1,"::de::fg" => 1,"ab" => 1,"hi::12" => 1};)
            : q($VAR1 = {"::de::fg" => 1,"ab" => 1,"hi::12" => 1,"0" => 1,"012345" => 1,"12" => 1,"123456789" => 1,"1234567890" => 1,"1\x{660}" => 1};)),
	   "always quote when quotekeys true (useqq)");
    }

    local $Data::Dumper::Quotekeys = 0;

    is(Dumper(\%qkdata),
        (($is_ascii)
         ? q($VAR1 = {0 => 1,'012345' => 1,12 => 1,123456789 => 1,'1234567890' => 1,"1\x{660}" => 1,'::de::fg' => 1,ab => 1,'hi::12' => 1};)
         : q($VAR1 = {'::de::fg' => 1,ab => 1,'hi::12' => 1,0 => 1,'012345' => 1,12 => 1,123456789 => 1,'1234567890' => 1,"1\x{660}" => 1};)),
	      "avoid quotes when quotekeys false");
    {
        local $Data::Dumper::Useqq = 1;
	is(Dumper(\%qkdata),
            (($is_ascii)
	     ? q($VAR1 = {0 => 1,"012345" => 1,12 => 1,123456789 => 1,"1234567890" => 1,"1\x{660}" => 1,"::de::fg" => 1,ab => 1,"hi::12" => 1};)
             : q($VAR1 = {"::de::fg" => 1,ab => 1,"hi::12" => 1,0 => 1,"012345" => 1,12 => 1,123456789 => 1,"1234567890" => 1,"1\x{660}" => 1};)),
	      "avoid quotes when quotekeys false (useqq)");
    }
}

