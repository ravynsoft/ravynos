#!perl

# Complicated enough to get its own test file.

# When a subroutine is called recursively, it gets a new pad indexed by its
# recursion depth (CvDEPTH).  If the sub is called at the same recursion
# depth again, the pad is reused.  Pad entries are localised on the
# savestack when ‘my’ is encountered.
#
# When a die/last/goto/exit unwinds the stack, it can trigger a DESTROY
# that recursively calls a subroutine that is in the middle of being
# popped.  Before this bug was fixed, the context stack was popped first,
# including CvDEPTH--, and then the savestack would be popped afterwards.
# Popping the savestack could trigger DESTROY and cause a sub to be called
# after its CvDEPTH was lowered but while its pad entries were still live
# and waiting to be cleared.  Decrementing CvDEPTH marks the pad as being
# available for the next call, which is wrong if the pad entries have not
# been cleared.
#
# Below we test two main variations of the bug that results.  First, we
# test an inner sub’s lexical holding an object whose DESTROY calls the
# outer sub.  Then we test a lexical directly inside the sub that DESTROY
# calls.  Then we repeat with formats.

BEGIN { chdir 't' if -d 't'; require './test.pl' }
plan 22;

sub foo {
    my ($block) = @_;

    my $got;
    $_ = $got ? "this is clearly a bug" : "ok";

    $got = 1;

    $block->();
}
sub Foo::DESTROY {
    foo(sub { });
    return;
}

eval { foo(sub { my $o = bless {}, 'Foo'; die }) };
is $_, "ok", 'die triggering DESTROY that calls outer sub';

undef $_;
{ foo(sub { my $o = bless {}, 'Foo'; last }) }
is $_, "ok", 'last triggering DESTROY that calls outer sub';

undef $_;
{ foo(sub { my $o = bless {}, 'Foo'; next }) }
is $_, "ok", 'next triggering DESTROY that calls outer sub';

undef $_;
{ if (!$count++) { foo(sub { my $o = bless {}, 'Foo'; redo }) } }
is $_, "ok", 'redo triggering DESTROY that calls outer sub';

undef $_;
foo(sub { my $o = bless {}, 'Foo'; goto test });
test:
is $_, "ok", 'goto triggering DESTROY that calls outer sub';

# END blocks trigger in reverse
sub END { is $_, "ok", 'exit triggering DESTROY that calls outer sub' }
sub END { undef $_; foo(sub { my $o = bless {}, 'Foo'; exit }); }


sub bar {
    my ($block) = @_;

    my $got;
    $_ = $got ? "this is clearly a bug" : "ok";

    $got = 1;

    my $o;
    if ($block) {
        $o = bless {}, "Bar";
        $block->();
    }
}
sub Bar::DESTROY {
    bar();
    return;
}

eval { bar(sub { die }) };
is $_, "ok", 'die triggering DESTROY that calls current sub';

undef $_;
{ bar(sub { last }) }
is $_, "ok", 'last triggering DESTROY that calls current sub';

undef $_;
{ bar(sub { next }) }
is $_, "ok", 'next triggering DESTROY that calls current sub';

undef $_;
undef $count;
{ if (!$count++) { bar(sub { redo }) } }
is $_, "ok", 'redo triggering DESTROY that calls current sub';

undef $_;
bar(sub { goto test2 });
test2:
is $_, "ok", 'goto triggering DESTROY that calls current sub';

sub END { is $_, "ok", 'exit triggering DESTROY that calls current sub' }
sub END { undef $_; bar(sub { exit }) }


format foo =
@
{
    my $got;
    $_ = $got ? "this is clearly a bug" : "ok";

    $got = 1;

    if ($inner_format) {
        local $~ = $inner_format;
        write;
    }
    "#"
}
.
sub Foomat::DESTROY {
    local $inner_format;
    local $~ = "foo";
    write;
    return;
}

$~ = "foo";

format inner_die =
@
{ my $o = bless {}, 'Foomat'; die }
.
undef $_;
study;
eval { local $inner_format = 'inner_die'; write };
is $_, "ok", 'die triggering DESTROY that calls outer format';

format inner_last =
@
{ my $o = bless {}, 'Foomat'; last LAST }
.
undef $_;
LAST: { local $inner_format = 'inner_last'; write }
is $_, "ok", 'last triggering DESTROY that calls outer format';

format inner_next =
@
{ my $o = bless {}, 'Foomat'; next NEXT }
.
undef $_;
NEXT: { local $inner_format = 'inner_next'; write }
is $_, "ok", 'next triggering DESTROY that calls outer format';

format inner_redo =
@
{ my $o = bless {}, 'Foomat'; redo REDO }
.
undef $_;
undef $_;
undef $count;
REDO: { if (!$count++) { local $inner_format = 'inner_redo'; write } }
is $_, "ok", 'redo triggering DESTROY that calls outer format';

# Can't "goto" out of a pseudo block.... (another bug?)
#format inner_goto =
#@
#{ my $o = bless {}, 'Foomat'; goto test3 }
#.
#undef $_;
#{ local $inner_format = 'inner_goto'; write }
#test3:
#is $_, "ok", 'goto triggering DESTROY that calls outer format';

format inner_exit =
@
{ my $o = bless {}, 'Foomat'; exit }
.
# END blocks trigger in reverse
END { is $_, "ok", 'exit triggering DESTROY that calls outer format' }
END { local $inner_format = 'inner_exit'; write }


format bar =
@
{
    my $got;
    $_ = $got ? "this is clearly a bug" : "ok";

    $got = 1;

    my $o;
    if ($block) {
        $o = bless {}, "Barmat";
        $block->();
    }
    "#"
}
.
sub Barmat::DESTROY {
    local $block;
    write;
    return;
}

$~ = "bar";

undef $_;
eval { local $block = sub { die }; write };
is $_, "ok", 'die triggering DESTROY directly inside format';

undef $_;
LAST: { local $block = sub { last LAST }; write }
is $_, "ok", 'last triggering DESTROY directly inside format';

undef $_;
NEXT: { local $block = sub { next NEXT }; write }
is $_, "ok", 'next triggering DESTROY directly inside format';

undef $_;
undef $count;
REDO: { if (!$count++) { local $block = sub { redo REDO }; write } }
is $_, "ok", 'redo triggering DESTROY directly inside format';

#undef $_;
#{ local $block = sub { goto test4 }; write }
#test4:
#is $_, "ok", 'goto triggering DESTROY directly inside format';

sub END { is $_, "ok", 'exit triggering DESTROY directly inside format' }
sub END { undef $_; local $block = sub { exit }; write }
