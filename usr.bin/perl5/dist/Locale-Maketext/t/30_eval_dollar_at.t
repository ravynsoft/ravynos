use strict;
use warnings;

{
    package TEST;
    use parent qw(Locale::Maketext);
}

{
    package TEST::en;
    use parent -norequire, qw(TEST);
    our %Lexicon = (
        _AUTO => 1,
    );
}

package main;
use strict;
use warnings;
use Test::More tests => 12;

my $lh = TEST->get_handle('en');
$@ = "foo";
is($lh->maketext("This works fine"), "This works fine", "straight forward _AUTO string test");
is($@, "foo", q{$@ isn't altered during calls to maketext});

my $err = eval {
   $lh->maketext('this is ] an error');
};
is($err, undef, "no return from eval");
like("$@", qr/Unbalanced\s'\]',\sin/ms, '$@ shows that ] was unbalanced'); 

# _try_use doesn't pollute $@
$@ = 'foo2';
is(Locale::Maketext::_try_use("This::module::does::not::exist"), 0, "0 return if module is missing when _try_use is called");
is($@, 'foo2', '$@ is unmodified by a failed _try_use');

# _try_use doesn't pollute $@ for valid call
$@ = '';
is(Locale::Maketext::_try_use("Locale::Maketext::Guts"), 1, "1 return using valid module Locale::Maketext::Guts");
is($@, '', '$@ is clean after failed _try_use');

# failure_handler_auto handles $@ locally.
{
    $@ = '';
    my $err = '';
    $lh->{failure_lex}->{"foo_fail"} = sub {die("fail message");};
    $err = eval {$lh->failure_handler_auto("foo_fail")};
    is($err, undef, "die event calling failure_handler on bad code");
    like($@, qr/^Error in maketexting "foo_fail":/ms, "\$@ is re-written as expected.");
}

$@ = 'foo';
is($lh->maketext('Eval error: [_1]', $@), 'Eval error: foo', "Make sure \$@ is localized when passed to maketext");
is($@, 'foo', "\$@ wasn't modified during call");
