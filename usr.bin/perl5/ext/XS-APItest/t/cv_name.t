use XS::APItest;
use Test::More tests => 30;
use feature "lexical_subs", "state";
no warnings "experimental::lexical_subs";

is (cv_name(\&foo), 'main::foo', 'cv_name with package sub');
is (cv_name(*{"foo"}{CODE}), 'main::foo',
   'cv_name with package sub via glob');
is (cv_name(\*{"foo"}), 'main::foo', 'cv_name with typeglob');
is (cv_name(\"foo"), 'foo', 'cv_name with string');
state sub lex1;
is (cv_name(\&lex1), 'lex1', 'cv_name with lexical sub');

$ret = \cv_name(\&bar, $name);
is $ret, \$name, 'cv_name with package sub returns 2nd argument';
is ($name, 'main::bar', 'retval of cv_name with package sub & 2nd arg');
$ret = \cv_name(*{"bar"}{CODE}, $name);
is $ret, \$name, 'cv_name with package sub via glob returns 2nd argument';
is ($name, 'main::bar', 'retval of cv_name w/pkg sub via glob & 2nd arg');
$ret = \cv_name(\*{"bar"}, $name);
is $ret, \$name, 'cv_name with typeglob returns 2nd argument';
is ($name, 'main::bar', 'retval of cv_name with typeglob & 2nd arg');
$ret = \cv_name(\"bar", $name);
is $ret, \$name, 'cv_name with string returns 2nd argument';
is ($name, 'bar', 'retval of cv_name with string & 2nd arg');
state sub lex2;
$ret = \cv_name(\&lex2, $name);
is $ret, \$name, 'cv_name with lexical sub returns 2nd argument';
is ($name, 'lex2', 'retval of cv_name with lexical sub & 2nd arg');

# nq in test names means CV_NAME_NOTQUAL
is (cv_name(\&foo, undef, 1), 'foo', 'cv_name with package sub (nq)');
is (cv_name(*{"foo"}{CODE}, undef, 1), 'foo',
   'cv_name with package sub via glob (nq)');
is (cv_name(\*{"foo"}, undef, 1), 'foo', 'cv_name with typeglob (nq)');
is (cv_name(\"foo", undef, 1), 'foo', 'cv_name with string (nq)');
is (cv_name(\&lex1, undef, 1), 'lex1', 'cv_name with lexical sub (nq)');

$ret = \cv_name(\&bar, $name, 1);
is $ret, \$name, 'cv_name with package sub returns 2nd argument (nq)';
is ($name, 'bar', 'retval of cv_name with package sub & 2nd arg (nq)');
$ret = \cv_name(*{"bar"}{CODE}, $name, 1);
is $ret, \$name, 'cv_name with package sub via glob returns 2nd arg (nq)';
is ($name, 'bar', 'retval of cv_name w/pkg sub via glob & 2nd arg (nq)');
$ret = \cv_name(\*{"bar"}, $name, 1);
is $ret, \$name, 'cv_name with typeglob returns 2nd argument (nq)';
is ($name, 'bar', 'retval of cv_name with typeglob & 2nd arg (nq)');
$ret = \cv_name(\"bar", $name, 1);
is $ret, \$name, 'cv_name with string returns 2nd argument (nq)';
is ($name, 'bar', 'retval of cv_name with string & 2nd arg (nq)');
$ret = \cv_name(\&lex2, $name, 1);
is $ret, \$name, 'cv_name with lexical sub returns 2nd argument (nq)';
is ($name, 'lex2', 'retval of cv_name with lexical sub & 2nd arg (nq)');
