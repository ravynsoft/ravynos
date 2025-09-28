#!./perl

# Testing the : prototype(..) attribute


BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    skip_all_if_miniperl("miniperl can't load attributes");
}
use warnings;

plan tests => 48;

my @warnings;
my ($attrs, $ret) = ("", "");
sub Q::MODIFY_CODE_ATTRIBUTES { my ($name, $ref, @attrs) = @_; $attrs = "@attrs";return;}
$SIG{__WARN__} = sub { push @warnings, shift;};

$ret = eval 'package Q; sub A(bar) : prototype(bad) : dummy1 {} prototype \&A;';
is $ret, "bad", "Prototype is set to \"bad\"";
is $attrs, "dummy1", "MODIFY_CODE_ATTRIBUTES called, but not for prototype(..)";
like shift @warnings, qr/Illegal character in prototype for Q::A : bar/,
    "First warning is bad prototype - bar";
like shift @warnings, qr/Illegal character in prototype for Q::A : bad/,
    "Second warning is bad prototype - bad";
like shift @warnings, qr/Prototype \'bar\' overridden by attribute \'prototype\(bad\)\' in Q::A/,
    "Third warning is Prototype overridden";
is @warnings, 0, "No more warnings";

# The override warning should not be hidden by no warnings (similar to prototype changed warnings)
{
    no warnings 'illegalproto';
    $ret = eval 'package Q; sub B(bar) : prototype(bad) dummy2 {4} prototype \&B;';
    is $ret, "bad", "Prototype is set to \"bad\"";
    is $attrs, "dummy2", "MODIFY_CODE_ATTRIBUTES called, but not for prototype(..)";
    like shift @warnings, qr/Prototype \'bar\' overridden by attribute \'prototype\(bad\)\' in Q::B/,
        "First warning is Prototype overridden";
    is @warnings, 0, "No more warnings";
}

# Redeclaring a sub with a prototype attribute ignores it
$ret = eval 'package Q; sub B(ignored) : prototype(baz) : dummy3; prototype \&B;';
is $ret, "bad", "Declaring with prototype(..) after definition doesn't change the prototype";
is $attrs, "dummy3", "MODIFY_CODE_ATTRIBUTES called, but not for prototype(..)";
like shift @warnings, qr/Illegal character in prototype for Q::B : ignored/,
    "Shifting off warning for the 'ignored' prototype";
like shift @warnings, qr/Illegal character in prototype for Q::B : baz/,
    "Attempting to redeclare triggers Illegal character warning";
like shift @warnings, qr/Prototype \'ignored\' overridden by attribute \'prototype\(baz\)\' in Q::B/,
    "Shifting off Prototype overridden warning";
like shift @warnings, qr/Prototype mismatch: sub Q::B \(bad\) vs \(baz\)/,
    "Attempting to redeclare triggers prototype mismatch warning against first prototype";
is @warnings, 0, "No more warnings";

# Confirm redifining with a prototype attribute takes it
$ret = eval 'package Q; sub B(ignored) : prototype(baz) dummy4 {5}; prototype \&B;';
is $ret, "baz", "Redefining with prototype(..) changes the prototype";
is $attrs, "dummy4", "MODIFY_CODE_ATTRIBUTES called, but not for prototype(..)";
is &Q::B, 5, "Function successfully redefined";
like shift @warnings, qr/Illegal character in prototype for Q::B : ignored/,
    "Attempting to redeclare triggers Illegal character warning";
like shift @warnings, qr/Illegal character in prototype for Q::B : baz/,
    "Attempting to redeclare triggers Illegal character warning";
like shift @warnings, qr/Prototype \'ignored\' overridden by attribute \'prototype\(baz\)\' in Q::B/,
    "Shifting off Prototype overridden warning";
like shift @warnings, qr/Prototype mismatch: sub Q::B \(bad\) vs \(baz\)/,
    "Attempting to redeclare triggers prototype mismatch warning";
like shift @warnings, qr/Subroutine B redefined/,
    "Only other warning is subroutine redefinition";
is @warnings, 0, "No more warnings";

# Multiple prototype declarations only takes the last one
$ret = eval 'package Q; sub dummy6 : prototype($$) : prototype($$$) {}; prototype \&dummy6;';
is $ret, "\$\$\$", "Last prototype declared wins";
like shift @warnings, qr/Attribute prototype\(\$\$\$\) discards earlier prototype attribute in same sub/,
    "Multiple prototype declarations warns";
is @warnings, 0, "No more warnings";

# Use attributes
eval 'package Q; use attributes __PACKAGE__, \&B, "prototype(new)";';
$ret = prototype \&Q::B;
is $ret, "new", "use attributes also sets the prototype";
like shift @warnings, qr/Prototype mismatch: sub Q::B \(baz\) vs \(new\)/,
    "Prototype mismatch warning triggered";
is @warnings, 0, "No more warnings";

eval 'package Q; use attributes __PACKAGE__, \&B, "prototype(\$\$~";';
$ret = prototype \&Q::B;
is $ret, "new", "A malformed prototype doesn't reset it";
like $@, qr/Unterminated attribute parameter in attribute list/, "Malformed prototype croaked";
is @warnings, 0, "Malformed prototype isn't just a warning";

eval 'use attributes __PACKAGE__, \&foo, "prototype($$\x{100}";';
$ret = prototype \&Q::B;
is $ret, "new", "A malformed prototype doesn't reset it";
like $@, qr/Unterminated attribute parameter in attribute list/, "Malformed prototype croaked";
is @warnings, 0, "Malformed prototype isn't just a warning";

# Anonymous subs (really just making sure they don't crash, since the prototypes
# themselves aren't much use)
{
    is eval 'package Q; my $a = sub(bar) : prototype(baz) {}; 1;', 1,
        "Sanity checking that eval of anonymous sub didn't croak";
    # The fact that the name is '?' in the first case
    # and __ANON__ in the second is due to toke.c temporarily setting
    # the name to '?' before calling the proto check, despite setting
    # it to the real name very shortly after.
    # In short - if this test breaks, just change the test.
    like shift @warnings, qr/Illegal character in prototype for \? : bar/,
        "(anon) bar triggers illegal proto warnings";
    like shift @warnings, qr/Illegal character in prototype for Q::__ANON__ : baz/,
        "(anon) baz triggers illegal proto warnings";
    like shift @warnings, qr/Prototype \'bar\' overridden by attribute \'prototype\(baz\)\' in Q::__ANON__/,
        "(anon) overridden warning triggered in anonymous sub";
    is @warnings, 0, "No more warnings";
}

# Testing lexical subs
{
    use feature "lexical_subs";
    no warnings "experimental::lexical_subs";
    $ret = eval 'my sub foo(bar) : prototype(baz) {}; prototype \&foo;';
    is $ret, "baz", "my sub foo honors the prototype attribute";
    like shift @warnings, qr/Illegal character in prototype for foo : bar/,
        "(lexical) bar triggers illegal proto warnings";
    like shift @warnings, qr/Illegal character in prototype for foo : baz/,
        "(lexical) baz triggers illegal proto warnings";
    like shift @warnings, qr/Prototype \'bar\' overridden by attribute \'prototype\(baz\)\' in foo/,
        "(lexical) overridden warning triggered in anonymous sub";
    is @warnings, 0, "No more warnings";
}

# ex: set ts=8 sts=4 sw=4 et:
