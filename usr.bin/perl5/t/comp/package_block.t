#!./perl

print "1..7\n";

$main::result = "";
eval q{
    $main::result .= "a(".__PACKAGE__."/".eval("__PACKAGE__").")";
    package Foo {
	$main::result .= "b(".__PACKAGE__."/".eval("__PACKAGE__").")";
	package Bar::Baz {
	    $main::result .= "c(".__PACKAGE__."/".eval("__PACKAGE__").")";
	}
	$main::result .= "d(".__PACKAGE__."/".eval("__PACKAGE__").")";
    }
    $main::result .= "e(".__PACKAGE__."/".eval("__PACKAGE__").")";
};
print $main::result eq
	"a(main/main)b(Foo/Foo)c(Bar::Baz/Bar::Baz)d(Foo/Foo)e(main/main)" ?
    "ok 1\n" : "not ok 1\n";

$main::result = "";
eval q{
    $main::result .= "a($Foo::VERSION)";
    $main::result .= "b($Bar::VERSION)";
    package Foo 11 { ; }
    package Bar 22 {
	$main::result .= "c(".__PACKAGE__."/".eval("__PACKAGE__").")";
    }
};
print $main::result eq "a(11)b(22)c(Bar/Bar)" ? "ok 2\n" : "not ok 2\n";

$main::result = "";
eval q{
    $main::result .= "a(".__PACKAGE__."/".eval("__PACKAGE__").")";
    package Foo { }
    $main::result .= "b(".__PACKAGE__."/".eval("__PACKAGE__").")";
};
print $main::result eq "a(main/main)b(main/main)" ? "ok 3\n" : "not ok 3\n";

eval q[package Foo {];
print $@ =~ /\AMissing right curly / ? "ok 4\n" : "not ok 4\n";

$main::result = "";
eval q{
    $main::result .= "a(".__LINE__.")";
    package Foo {
	$main::result .= "b(".__LINE__.")";
	package Bar::Baz {
	    $main::result .= "c(".__LINE__.")";
	}
	$main::result .= "d(".__LINE__.")";
    }
    $main::result .= "e(".__LINE__.")";
    package Quux { }
    $main::result .= "f(".__LINE__.")";
};
print $main::result eq "a(2)b(4)c(6)d(8)e(10)f(12)" ? "ok 5\n" : "not ok 5\n";

$main::result = "";
$main::warning = "";
$SIG{__WARN__} = sub { $main::warning .= $_[0]; };
eval q{
    $main::result .= "a(".__PACKAGE__."/".eval("__PACKAGE__").")";
    goto l0;
    $main::result .= "b(".__PACKAGE__."/".eval("__PACKAGE__").")";
    package Foo {
	$main::result .= "c(".__PACKAGE__."/".eval("__PACKAGE__").")";
	l0:
	$main::result .= "d(".__PACKAGE__."/".eval("__PACKAGE__").")";
	goto l1;
	$main::result .= "e(".__PACKAGE__."/".eval("__PACKAGE__").")";
    }
    $main::result .= "f(".__PACKAGE__."/".eval("__PACKAGE__").")";
    l1:
    $main::result .= "g(".__PACKAGE__."/".eval("__PACKAGE__").")";
    goto l2;
    $main::result .= "h(".__PACKAGE__."/".eval("__PACKAGE__").")";
    package Bar {
	l2:
	$main::result .= "i(".__PACKAGE__."/".eval("__PACKAGE__").")";
    }
    $main::result .= "j(".__PACKAGE__."/".eval("__PACKAGE__").")";
};
print $main::result eq
	"a(main/main)d(Foo/Foo)g(main/main)i(Bar/Bar)j(main/main)" ?
    "ok 6\n" : "not ok 6\n";
print $main::warning =~ /\A
	Use\ of\ "goto"\ [^\n]*\ line\ 3\.\n
	Use\ of\ "goto"\ [^\n]*\ line\ 15\.\n
    \z/x ? "ok 7\n" : "not ok 7\n";

1;
