#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}
use strict;
use warnings;
no warnings 'uninitialized';
no warnings 'deprecated';    # smartmatch is deprecated and will be removed in 5.042

++$|;

use Tie::Array;
use Tie::Hash;

# Predeclare vars used in the tests:
my @empty;
my %empty;
my @sparse; $sparse[2] = 2;

my $deep1 = []; push @$deep1, $deep1;
my $deep2 = []; push @$deep2, $deep2;

my @nums = (1..10);
tie my @tied_nums, 'Tie::StdArray';
@tied_nums =  (1..10);

my %hash = (foo => 17, bar => 23);
tie my %tied_hash, 'Tie::StdHash';
%tied_hash = %hash;

{
    package Test::Object::NoOverload;
    sub new { bless { key => 1 } }
}

{
    package Test::Object::StringOverload;
    use overload '""' => sub { "object" }, fallback => 1;
    sub new { bless { key => 1 } }
}

{
    package Test::Object::WithOverload;
    sub new { bless { key => ($_[1] // 'magic') } }
    use overload '~~' => sub {
	my %hash = %{ $_[0] };
	if ($_[2]) { # arguments reversed ?
	    return $_[1] eq reverse $hash{key};
	}
	else {
	    return $_[1] eq $hash{key};
	}
    };
    use overload '""' => sub { "stringified" };
    use overload 'eq' => sub {"$_[0]" eq "$_[1]"};
}

our $ov_obj = Test::Object::WithOverload->new;
our $ov_obj_2 = Test::Object::WithOverload->new("object");
our $obj = Test::Object::NoOverload->new;
our $str_obj = Test::Object::StringOverload->new;

my %refh;
unless (is_miniperl()) {
    require Tie::RefHash;
    tie %refh, 'Tie::RefHash';
    $refh{$ov_obj} = 1;
}

my @keyandmore = qw(key and more);
my @fooormore = qw(foo or more);
my %keyandmore = map { $_ => 0 } @keyandmore;
my %fooormore = map { $_ => 0 } @fooormore;

# Load and run the tests
plan tests => 349+4;

while (<DATA>) {
  SKIP: {
    next if /^#/ || !/\S/;
    chomp;
    my ($yn, $left, $right, $note) = split /\t+/;

    local $::TODO = $note =~ /TODO/;

    die "Bad test spec: ($yn, $left, $right)" if $yn =~ /[^!@=]/;

    my $tstr = "$left ~~ $right";

    test_again:
    my $res;
    if ($note =~ /NOWARNINGS/) {
	$res = eval "no warnings; $tstr";
    }
    else {
	skip_if_miniperl("Doesn't work with miniperl", $yn =~ /=/ ? 2 : 1)
	    if $note =~ /MINISKIP/;
	$res = eval $tstr;
    }

    chomp $@;

    if ( $yn =~ /@/ ) {
	ok( $@ ne '', "$tstr dies" )
	    and print "# \$\@ was: $@\n";
    } else {
	my $test_name = $tstr . ($yn =~ /!/ ? " does not match" : " matches");
	if ( $@ ne '' ) {
	    fail($test_name);
	    print "# \$\@ was: $@\n";
	} else {
	    ok( ($yn =~ /!/ xor $res), $test_name );
	}
    }

    if ( $yn =~ s/=// ) {
	$tstr = "$right ~~ $left";
	goto test_again;
    }
  }
}

sub foo {}
sub bar {42}
sub gorch {42}
sub fatal {die "fatal sub\n"}

# to test constant folding
sub FALSE() { 0 }
sub TRUE() { 1 }
sub NOT_DEF() { undef }

{
  # [perl #123860]
  # this can but might not crash
  # This can but might not crash
  #
  # The second smartmatch would leave a &PL_sv_no on the stack for
  # each key it checked in %!, this could then cause various types of
  # crash or assertion failure.
  #
  # This isn't guaranteed to crash, but if the stack issue is
  # re-introduced it will probably crash in one of the many smoke
  # builds.
  fresh_perl_is('print (q(x) ~~ q(x)) | (/x/ ~~ %!)', "1",
		{ switches => [ "-MErrno", "-M-warnings=deprecated" ] },
		 "don't fill the stack with rubbish");
}

{
    # [perl #123860] continued;
    # smartmatch was failing to SPAGAIN after pushing an SV and calling
    # pp_match, which may have resulted in the stack being realloced
    # in the meantime. Test this by filling the stack with pregressively
    # larger amounts of data. At some point the stack will get realloced.
    my @a = qw(x);
    my %h = qw(x 1);
    my @args;
    my $x = 1;
    my $bad = -1;
    for (1..1000)  {
        push @args, $_;
        my $exp_n  = join '-',  (@args, $x == 0);
        my $exp_y  = join '-',  (@args, $x == 1);

        my $got_an = join '-',  (@args, (/X/ ~~ @a));
        my $got_ay = join '-',  (@args, (/x/ ~~ @a));
        my $got_hn = join '-',  (@args, (/X/ ~~ %h));
        my $got_hy = join '-',  (@args, (/x/ ~~ %h));

        if (   $exp_n ne $got_an || $exp_n ne $got_hn
            || $exp_y ne $got_ay || $exp_y ne $got_hy
        ) {
            $bad = $_;
            last;
        }
    }
    is($bad, -1, "RT 123860: stack realloc");
}


{
    # [perl #130705]
    # Perl_ck_smartmatch would turn the match in:
    # 0 =~ qr/1/ ~~ 0  # parsed as (0 =~ qr/1/) ~~ 0
    # into a qr, leaving the initial 0 on the stack after execution
    #
    # Similarly for: 0 ~~ (0 =~ qr/1/)
    #
    # Either caused an assertion failure in the context of warn (or print)
    # if there was some other operator's arguments left on the stack, as with
    # the test cases.
    fresh_perl_is('print(0->[0 =~ qr/1/ ~~ 0])', '',
                  { switches => [ "-M-warnings=deprecated" ] },
                  "don't qr-ify left-side match against a stacked argument");
    fresh_perl_is('print(0->[0 ~~ (0 =~ qr/1/)])', '',
                  { switches => [ "-M-warnings=deprecated" ] },
                  "don't qr-ify right-side match against a stacked argument");
}

# Prefix character :
#   - expected to match
# ! - expected to not match
# @ - expected to be a compilation failure
# = - expected to match symmetrically (runs test twice)
# Data types to test :
#   undef
#   Object-overloaded
#   Object
#   Coderef
#   Hash
#   Hashref
#   Array
#   Arrayref
#   Tied arrays and hashes
#   Arrays that reference themselves
#   Regex (// and qr//)
#   Range
#   Num
#   Str
# Other syntactic items of interest:
#   Constants
#   Values returned by a sub call
__DATA__
# Any ~~ undef
!	$ov_obj		undef
!	$obj		undef
!	sub {}		undef
!	%hash		undef
!	\%hash		undef
!	{}		undef
!	@nums		undef
!	\@nums		undef
!	[]		undef
!	%tied_hash	undef
!	@tied_nums	undef
!	$deep1		undef
!	/foo/		undef
!	qr/foo/		undef
!	21..30		undef
!	189		undef
!	"foo"		undef
!	""		undef
!	!1		undef
	undef		undef
	(my $u)		undef
	NOT_DEF		undef
	&NOT_DEF	undef

# Any ~~ object overloaded
!	\&fatal		$ov_obj
	'cigam'		$ov_obj
!	'cigam on'	$ov_obj
!	['cigam']	$ov_obj
!	['stringified']	$ov_obj
!	{ cigam => 1 }	$ov_obj
!	{ stringified => 1 }	$ov_obj
!	$obj		$ov_obj
!	undef		$ov_obj

# regular object
@	$obj		$obj
@	$ov_obj		$obj
=@	\&fatal		$obj
@	\&FALSE		$obj
@	\&foo		$obj
@	sub { 1 }	$obj
@	sub { 0 }	$obj
@	%keyandmore	$obj
@	{"key" => 1}	$obj
@	@fooormore	$obj
@	["key" => 1]	$obj
@	/key/		$obj
@	qr/key/		$obj
@	"key"		$obj
@	FALSE		$obj

# regular object with "" overload
@	$obj		$str_obj
=@	\&fatal		$str_obj
@	\&FALSE		$str_obj
@	\&foo		$str_obj
@	sub { 1 }	$str_obj
@	sub { 0 }	$str_obj
@	%keyandmore	$str_obj
@	{"object" => 1}	$str_obj
@	@fooormore	$str_obj
@	["object" => 1]	$str_obj
@	/object/	$str_obj
@	qr/object/	$str_obj
@	"object"	$str_obj
@	FALSE		$str_obj
# Those will treat the $str_obj as a string because of fallback:

# object (overloaded or not) ~~ Any
	$obj		qr/NoOverload/
	$ov_obj		qr/^stringified$/
=	"$ov_obj"	"stringified"
=	"$str_obj"	"object"
!=	$ov_obj		"stringified"
	$str_obj	"object"
	$ov_obj		'magic'
!	$ov_obj		'not magic'

# ~~ Coderef
	sub{0}		sub { ref $_[0] eq "CODE" }
	%fooormore	sub { $_[0] =~ /^(foo|or|more)$/ }
!	%fooormore	sub { $_[0] =~ /^(foo|or|less)$/ }
	\%fooormore	sub { $_[0] =~ /^(foo|or|more)$/ }
!	\%fooormore	sub { $_[0] =~ /^(foo|or|less)$/ }
	+{%fooormore}	sub { $_[0] =~ /^(foo|or|more)$/ }
!	+{%fooormore}	sub { $_[0] =~ /^(foo|or|less)$/ }
	@fooormore	sub { $_[0] =~ /^(foo|or|more)$/ }
!	@fooormore	sub { $_[0] =~ /^(foo|or|less)$/ }
	\@fooormore	sub { $_[0] =~ /^(foo|or|more)$/ }
!	\@fooormore	sub { $_[0] =~ /^(foo|or|less)$/ }
	[@fooormore]	sub { $_[0] =~ /^(foo|or|more)$/ }
!	[@fooormore]	sub { $_[0] =~ /^(foo|or|less)$/ }
	%fooormore	sub{@_==1}
	@fooormore	sub{@_==1}
	"foo"		sub { $_[0] =~ /^(foo|or|more)$/ }
!	"more"		sub { $_[0] =~ /^(foo|or|less)$/ }
	/fooormore/	sub{ref $_[0] eq 'Regexp'}
	qr/fooormore/	sub{ref $_[0] eq 'Regexp'}
	1		sub{shift}
!	0		sub{shift}
!	undef		sub{shift}
	undef		sub{not shift}
	NOT_DEF		sub{not shift}
	&NOT_DEF	sub{not shift}
	FALSE		sub{not shift}
	[1]		\&bar
	{a=>1}		\&bar
	qr//		\&bar
!	[1]		\&foo
!	{a=>1}		\&foo
	$obj		sub { ref($_[0]) =~ /NoOverload/ }
	$ov_obj		sub { ref($_[0]) =~ /WithOverload/ }
# empty stuff matches, because the sub is never called:
	[]		\&foo
	{}		\&foo
	@empty		\&foo
	%empty		\&foo
!	qr//		\&foo
!	undef		\&foo
	undef		\&bar
@	undef		\&fatal
@	1		\&fatal
@	[1]		\&fatal
@	{a=>1}		\&fatal
@	"foo"		\&fatal
@	qr//		\&fatal
# sub is not called on empty hashes / arrays
	[]		\&fatal
	+{}		\&fatal
	@empty		\&fatal
	%empty		\&fatal
# sub is not special on the left
	sub {0}		qr/^CODE/
	sub {0}		sub { ref shift eq "CODE" }

# HASH ref against:
#   - another hash ref
	{}		{}
=!	{}		{1 => 2}
	{1 => 2}	{1 => 2}
	{1 => 2}	{1 => 3}
=!	{1 => 2}	{2 => 3}
=	\%main::	{map {$_ => 'x'} keys %main::}

#  - tied hash ref
=	\%hash		\%tied_hash
	\%tied_hash	\%tied_hash
!=	{"a"=>"b"}	\%tied_hash
=	%hash		%tied_hash
	%tied_hash	%tied_hash
!=	{"a"=>"b"}	%tied_hash
	$ov_obj		%refh		MINISKIP
!	"$ov_obj"	%refh		MINISKIP
	[$ov_obj]	%refh		MINISKIP
!	["$ov_obj"]	%refh		MINISKIP
	%refh		%refh		MINISKIP

#  - an array ref
#  (since this is symmetrical, tests as well hash~~array)
=	[keys %main::]	\%::
=	[qw[STDIN STDOUT]]	\%::
=!	[]		\%::
=!	[""]		{}
=!	[]		{}
=!	@empty		{}
=	[undef]		{"" => 1}
=	[""]		{"" => 1}
=	["foo"]		{ foo => 1 }
=	["foo", "bar"]	{ foo => 1 }
=	["foo", "bar"]	\%hash
=	["foo"]		\%hash
=!	["quux"]	\%hash
=	[qw(foo quux)]	\%hash
=	@fooormore	{ foo => 1, or => 2, more => 3 }
=	@fooormore	%fooormore
=	@fooormore	\%fooormore
=	\@fooormore	%fooormore

#  - a regex
=	qr/^(fo[ox])$/		{foo => 1}
=	/^(fo[ox])$/		%fooormore
=!	qr/[13579]$/		+{0..99}
=!	qr/a*/			{}
=	qr/a*/			{b=>2}
=	qr/B/i			{b=>2}
=	/B/i			{b=>2}
=!	qr/a+/			{b=>2}
=	qr/^à/			{"à"=>2}

#  - a scalar
	"foo"		+{foo => 1, bar => 2}
	"foo"		%fooormore
!	"baz"		+{foo => 1, bar => 2}
!	"boz"		%fooormore
!	1		+{foo => 1, bar => 2}
!	1		%fooormore
	1		{ 1 => 3 }
	1.0		{ 1 => 3 }
!	"1.0"		{ 1 => 3 }
!	"1.0"		{ 1.0 => 3 }
	"1.0"		{ "1.0" => 3 }
	"à"		{ "à" => "À" }

#  - undef
!	undef		{ hop => 'zouu' }
!	undef		%hash
!	undef		+{"" => "empty key"}
!	undef		{}

# ARRAY ref against:
#  - another array ref
	[]			[]
=!	[]			[1]
	[["foo"], ["bar"]]	[qr/o/, qr/a/]
!	[["foo"], ["bar"]]	[qr/ARRAY/, qr/ARRAY/]
	["foo", "bar"]		[qr/o/, qr/a/]
!	[qr/o/, qr/a/]		["foo", "bar"]
	["foo", "bar"]		[["foo"], ["bar"]]
!	["foo", "bar"]		[qr/o/, "foo"]
	["foo", undef, "bar"]	[qr/o/, undef, "bar"]
!	["foo", undef, "bar"]	[qr/o/, "",    "bar"]
!	["foo", "", "bar"]	[qr/o/, undef, "bar"]
	$deep1			$deep1
	@$deep1			@$deep1
!	$deep1			$deep2

=	\@nums			\@tied_nums
=	@nums			\@tied_nums
=	\@nums			@tied_nums
=	@nums			@tied_nums

#  - an object
!	$obj		@fooormore
	$obj		[sub{ref shift}]

#  - a regex
=	qr/x/		[qw(foo bar baz quux)]
=!	qr/y/		[qw(foo bar baz quux)]
=	/x/		[qw(foo bar baz quux)]
=!	/y/		[qw(foo bar baz quux)]
=	/FOO/i		@fooormore
=!	/bar/		@fooormore

# - a number
	2		[qw(1.00 2.00)]
	2		[qw(foo 2)]
	2.0_0e+0	[qw(foo 2)]
!	2		[qw(1foo bar2)]

# - a string
!	"2"		[qw(1foo 2bar)]
	"2bar"		[qw(1foo 2bar)]

# - undef
	undef		[1, 2, undef, 4]
!	undef		[1, 2, [undef], 4]
!	undef		@fooormore
	undef		@sparse
	undef		[undef]
!	0		[undef]
!	""		[undef]
!	undef		[0]
!	undef		[""]

# - nested arrays and ~~ distributivity
	11		[[11]]
!	11		[[12]]
	"foo"		[{foo => "bar"}]
!	"bar"		[{foo => "bar"}]

# Number against number
	2		2
	20		2_0
!	2		3
	0		FALSE
	3-2		TRUE
!	undef		0
!	(my $u)		0

# Number against string
=	2		"2"
=	2		"2.0"
!	2		"2bananas"
!=	2_3		"2_3"		NOWARNINGS
	FALSE		"0"
!	undef		"0"
!	undef		""

# Regex against string
	"x"		qr/x/
!	"x"		qr/y/

# Regex against number
	12345		qr/3/
!	12345		qr/7/

# array/hash against string
	@fooormore	"".\@fooormore
!	@keyandmore	"".\@fooormore
	%fooormore	"".\%fooormore
!	%keyandmore	"".\%fooormore

# Test the implicit referencing
	7		@nums
	@nums		\@nums
!	@nums		\\@nums
	@nums		[1..10]
!	@nums		[0..9]

	"foo"		%hash
	/bar/		%hash
	[qw(bar)]	%hash
!	[qw(a b c)]	%hash
	%hash		%hash
	%hash		+{%hash}
	%hash		\%hash
	%hash		%tied_hash
	%tied_hash	%tied_hash
	%hash		{ foo => 5, bar => 10 }
!	%hash		{ foo => 5, bar => 10, quux => 15 }

	@nums		{  1, '',  2, '' }
	@nums		{  1, '', 12, '' }
!	@nums		{ 11, '', 12, '' }

# array slices
	@nums[0..-1]	[]
	@nums[0..0]	[1]
!	@nums[0..1]	[0..2]
	@nums[0..4]	[1..5]

!	undef		@nums[0..-1]
	1		@nums[0..0]
	2		@nums[0..1]
!	@nums[0..1]	2

	@nums[0..1]	@nums[0..1]

# hash slices
	@keyandmore{qw(not)}		[undef]
	@keyandmore{qw(key)}		[0]

	undef				@keyandmore{qw(not)}
	0				@keyandmore{qw(key and more)}
!	2				@keyandmore{qw(key and)}

	@fooormore{qw(foo)}		@keyandmore{qw(key)}
	@fooormore{qw(foo or more)}	@keyandmore{qw(key and more)}

# UNDEF
!	3		undef
!	1		undef
!	[]		undef
!	{}		undef
!	\%::main	undef
!	[1,2]		undef
!	%hash		undef
!	@nums		undef
!	"foo"		undef
!	""		undef
!	!1		undef
!	\&foo		undef
!	sub { }		undef
