#!perl -w

use strict;
use Test::More;

BEGIN {use_ok('XS::APItest')};
my (%sigils);
BEGIN {
    %sigils = (
	       '$' => 'sv',
	       '@' => 'av',
	       '%' => 'hv',
	       '&' => 'cv',
	       '*' => 'gv'
	      );
}
my %types = map {$_, eval "&to_${_}_amg()"} values %sigils;

{
    package None;
}

{
    package Other;
    use overload 'eq' => sub {no overloading; $_[0] == $_[1]},
	'""' =>  sub {no overloading; "$_[0]"},
	'~' => sub {return "Perl rules"};
}

{
    package Same;
    use overload 'eq' => sub {no overloading; $_[0] == $_[1]},
	'""' =>  sub {no overloading; "$_[0]"},
	map {$_ . '{}', sub {return $_[0]}} keys %sigils;
}

{
    package Chain;
    use overload 'eq' => sub {no overloading; $_[0] == $_[1]},
	'""' =>  sub {no overloading; "$_[0]"},
	map {$_ . '{}', sub {no overloading; return $_[0][0]}} keys %sigils;
}

my @non_ref = (['undef', undef],
		 ['number', 42],
		 ['string', 'Pie'],
		);

my @ref = (['unblessed SV', do {\my $whap}],
	   ['unblessed AV', []],
	   ['unblessed HV', {}],
	   ['unblessed CV', sub {}],
	   ['unblessed GV', \*STDOUT],
	   ['no overloading', bless {}, 'None'],
	   ['other overloading', bless {}, 'Other'],
	   ['same overloading', bless {}, 'Same'],
	  );

while (my ($type, $enum) = each %types) {
    foreach ([amagic_deref_call => \&amagic_deref_call],
	     [tryAMAGICunDEREF_var => \&tryAMAGICunDEREF_var],
	    ) {
	my ($name, $func) = @$_;
	foreach (@non_ref, @ref,
		) {
	    my ($desc, $input) = @$_;
	    my $got = &$func($input, $enum);
	    is($got, $input, "$name: expect no change for to_$type $desc");
	}
	foreach (@non_ref) {
	    my ($desc, $sucker) = @$_;
	    my $input = bless [$sucker], 'Chain';
	    is(eval {&$func($input, $enum)}, undef,
	       "$name: chain to $desc for to_$type");
	    like($@, qr/Overloaded dereference did not return a reference/,
		 'expected error');
	}
	foreach (@ref,
		) {
	    my ($desc, $sucker) = @$_;
	    my $input = bless [$sucker], 'Chain';
	    my $got = &$func($input, $enum);
	    is($got, $sucker, "$name: chain to $desc for to_$type");
	    $input = bless [bless [$sucker], 'Chain'], 'Chain';
	    $got = &$func($input, $enum);
	    is($got, $sucker, "$name: chain to chain to $desc for to_$type");
	}
    }
}

{
    package String;
    use overload q("")=>sub { return $_[0]->val };
    sub is_string_amg { 1 }
    sub val { "string" }
}
{
    package Num;
    sub is_string_amg { 1 }
    use overload q(0+) => sub { return $_[0]->val };
    sub val { 12345 };
}
{
    package NumNoFallback;
    sub is_string_amg { undef }
    use overload q(0+) => sub { return $_[0]->val }, fallback=>0;
    sub val { 1234 };
}
{
    package NumWithFallback;
    sub is_string_amg { 1 }
    use overload q(0+)=>sub { return $_[0]->val }, fallback=>1;
    sub val { 123456 };
}
{
    package NoMethod;
    use overload q(nomethod)=> sub { $_[0]->val };
    sub is_string_amg { 1 }
    sub val { return(ref($_[0])||$_[0]); };
}
{
    package NoOverload;
    sub is_string_amg { 0 }
}


{
    # these should be false

    my $string_amg = 0x0a;
    my $unary= 8;

    foreach my $class (
        "String",
        "Num",
        "NumNoFallback",
        "NumWithFallback",
        "NoMethod",
        "NoOverload",
    ) {
        my $item= bless {}, $class;
        my $str= eval { "$item" };
        my $std_str= overload::StrVal($item);
        my $ok= does_amagic_apply($item, $string_amg, $unary);
        my $want = $class->is_string_amg;
        is(0+$ok, $want//0, "amagic_applies($class,string_amg,AMGf_unary) works as expected");
        is($str, $want ? $class->val : defined ($want) ? $std_str : undef,
            "Stringified var matches amagic_applies()");
    }
}

done_testing;
