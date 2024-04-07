#!perl -w
$|=1;
BEGIN {
    require Config; import Config;
    if ($Config{'extensions'} !~ /\bOpcode\b/ && $Config{'osname'} ne 'VMS') {
        print "1..0\n";
        exit 0;
    }
}

use Safe 1.00;
use Test::More tests => 10;

my $safe = Safe->new('PLPerl');
$safe->permit_only(qw(:default sort));

# check basic argument passing and context for anon-subs
my $func = $safe->reval(q{ sub { @_ } });
is_deeply [ $func->() ], [ ];
is_deeply [ $func->("foo") ], [ "foo" ];

my $func1 = $safe->reval(<<'EOS');

    # uses quotes in { "$a" <=> $b } to avoid the optimizer replacing the block
    # with a hardwired comparison
    { package Pkg; sub p_sort { return sort { "$a" <=> $b } @_; } }
                   sub l_sort { return sort { "$a" <=> $b } @_; }

    return sub { return join(",",l_sort(@_)), join(",",Pkg::p_sort(@_)) }

EOS

is $@, '', 'reval should not fail';
is ref $func, 'CODE', 'reval should return a CODE ref';

my ($l_sorted, $p_sorted) = $func1->(3,1,2);
is $l_sorted, "1,2,3";
is $p_sorted, "1,2,3";

# check other aspects of closures created inside Safe

my $die_func = $safe->reval(q{ sub { die @_ if @_; 1 } });

# check $@ not affected by successful call
$@ = 42;
$die_func->();
is $@, 42, 'successful closure call should not alter $@';

{
    my $warns = 0;
    local $SIG{__WARN__} = sub { $warns++ };
    local $TODO = $] >= 5.013 ? "Doesn't die in 5.13" : undef;
    ok !eval { $die_func->("died\n"); 1 }, 'should die';
    is $@, "died\n", '$@ should be set correctly';
    local $TODO = "Shouldn't warn";
    is $warns, 0;
}
