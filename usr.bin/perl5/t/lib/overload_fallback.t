use warnings;
use strict;

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require './test.pl';
    plan(tests => 4);
}

use overload '""' => sub { 'stringvalue' }, fallback => 1;

BEGIN {
my $x = bless {}, 'main';
ok ($x eq 'stringvalue', 'fallback worked');
}


# NOTE: delete the next line and this test script will pass
use overload '+' => sub { die "unused"; };

my $x = bless {}, 'main';
ok (eval {$x eq 'stringvalue'}, 'fallback worked again');

TODO: {
  local $::TODO = 'RT #43356: Autogeneration of ++ is incorrect';
  fresh_perl_is(<<'EOC', '2', {}, 'RT #43356: Autogeneration of ++');
use overload
    "0+"     => sub { ${$_[0]} },
    "="      => sub { ${$_[0]} },
    fallback => 1;
my $value = bless \(my $dummy = 1), __PACKAGE__;
print ++$value;
EOC
}

{
    my $warned = 0;
    local $SIG{__WARN__} = sub { $warned++; };

    eval q{
        use overload '${}', 'fallback';
        no overload '${}', 'fallback';
    };

    ok($warned == 0, 'no overload should not warn');
}

