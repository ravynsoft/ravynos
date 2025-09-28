#!perl

# Test the various op trees that turn sub () { ... } into a constant, and
# some variants that don’t.

BEGIN {
    chdir 't';
    require './test.pl';
    set_up_inc('../lib');
}
plan 148;

# @tests is an array of hash refs, each of which can have various keys:
#
#   nickname    - name of the sub to use in test names
#   generator   - a sub returning a code ref to test
#   finally     - sub to run after the tests
#
# Each of the following gives expected test results.  If the key is
# omitted, the test is skipped:
#
#   retval      - the returned code ref’s return value
#   same_retval - whether the same scalar is returned each time
#   inlinable   - whether the sub is inlinable
#   deprecated  - whether the sub returning a code ref will emit a depreca-
#                 tion warning when called
#   method      - whether the sub has the :method attribute
#   exception   - sub now throws an exception (previously threw
#                 deprecation warning)

my $exception_134138 = 'Constants from lexical variables potentially modified '
    . 'elsewhere are no longer permitted';

# [perl #63540] Don’t treat sub { if(){.....}; "constant" } as a constant
sub blonk { ++$blonk_was_called }
push @tests, {
  nickname    => 'sub with null+kids (if-block), then constant',
  generator   => sub {
    # This used to turn into a constant with the value of $x
    my $x = 7;
    sub() { if($x){ () = "tralala"; blonk() }; 0 }
  },
  retval      => 0,
  same_retval => 0,
  inlinable   => 0,
  deprecated  => 0,
  method      => 0,
  finally     => sub { ok($blonk_was_called, 'RT #63540'); },
};

# [perl #79908]
push @tests, {
  nickname    => 'sub with simple lexical modified elsewhere',
  generator   => sub { my $x = 5; my $ret = sub(){$x}; $x = 7; $ret },
  exception   => $exception_134138,
};

push @tests, {
  nickname    => 'sub with simple lexical unmodified elsewhere',
  generator   => sub { my $x = 5; sub(){$x} },
  retval      => 5,
  same_retval => 0,
  inlinable   => 1,
  deprecated  => 0,
  method      => 0,
};

push @tests, {
  nickname    => 'return $variable modified elsewhere',
  generator   => sub { my $x=5; my $ret = sub(){return $x}; $x = 7; $ret },
  retval      => 7,
  same_retval => 0,
  inlinable   => 0,
  deprecated  => 0,
  method      => 0,
};

push @tests, {
  nickname    => 'return $variable unmodified elsewhere',
  generator   => sub { my $x = 5; sub(){return $x} },
  retval      => 5,
  same_retval => 0,
  inlinable   => 0,
  deprecated  => 0,
  method      => 0,
};

push @tests, {
  nickname    => 'sub () { 0; $x } with $x modified elsewhere',
  generator   => sub { my $x = 5; my $ret = sub(){0;$x}; $x = 8; $ret },
  retval      => 8,
  same_retval => 0,
  inlinable   => 0,
  deprecated  => 0,
  method      => 0,
};

push @tests, {
  nickname    => 'sub () { 0; $x } with $x unmodified elsewhere',
  generator   => sub { my $x = 5; my $y = $x; sub(){0;$x} },
  retval      => 5,
  same_retval => 0,
  inlinable   => 1,
  deprecated  => 0,
  method      => 0,
};

# Explicit return after optimised statement, not at end of sub
push @tests, {
  nickname    => 'sub () { 0; return $x; ... }',
  generator   => sub { my $x = 5; sub () { 0; return $x; ... } },
  retval      => 5,
  same_retval => 0,
  inlinable   => 0,
  deprecated  => 0,
  method      => 0,
};

# Explicit return after optimised statement, at end of sub [perl #123092]
push @tests, {
  nickname    => 'sub () { 0; return $x }',
  generator   => sub { my $x = 5; sub () { 0; return $x } },
  retval      => 5,
  same_retval => 0,
  inlinable   => 0,
  deprecated  => 0,
  method      => 0,
};

# Multiple closure tests
push @tests, {
  nickname    => 'simple lexical after another closure and no lvalue',
  generator   => sub {
    my $x = 5;
    # This closure prevents inlining, though theoretically it shouldn’t
    # have to.  If you change the behaviour, just change the test.  This
    # fails the refcount check in op.c:op_const_sv, which is necessary for
    # the sake of \(my $x = 1) (tested below).
    my $sub1 = sub () { () = $x };
    sub () { $x };
  },
  retval      => 5,
  same_retval => 0,
  inlinable   => 0,
  deprecated  => 0,
  method      => 0,
};
push @tests, {
  nickname    => 'simple lexical before another closure and no lvalue',
  generator   => sub {
    my $x = 5;
    my $ret = sub () { $x };
    # This does not prevent inlining and never has.
    my $sub1 = sub () { () = $x };
    $ret;
  },
  retval      => 5,
  same_retval => 0,
  inlinable   => 1,
  deprecated  => 0,
  method      => 0,
};
push @tests, {
  nickname    => 'simple lexical after an lvalue closure',
  generator   => sub {
    my $x = 5;
    # This has always prevented inlining
    my $sub1 = sub () { $x++ };
    sub () { $x };
  },
  retval      => 5,
  same_retval => 0,
  inlinable   => 0,
  deprecated  => 0,
  method      => 0,
};
push @tests, {
  nickname    => 'simple lexical before an lvalue closure',
  generator   => sub {
    my $x = 5;
    my $ret = sub () { $x };  # <-- simple lexical op tree
    # Traditionally this has not prevented inlining, though it should.  But
    # since $ret has a simple lexical op tree, we preserve backward-compat-
    # ibility, but deprecate it.
    my $sub1 = sub () { $x++ };
    $ret;
  },
  exception   => $exception_134138,
};
push @tests, {
  nickname    => 'complex lexical op tree before an lvalue closure',
  generator   => sub {
    my $x = 5;
    my $ret = sub () { 0; $x };  # <-- more than just a lexical
    # This used not to prevent inlining, though it should, and now does.
    my $sub1 = sub () { $x++ };
    $ret;
  },
  retval      => 5,
  same_retval => 0,
  inlinable   => 0,
  deprecated  => 0,
  method      => 0,
};
push @tests, {
  nickname    => 'complex lexical op tree before a nested lvalue closure',
  generator   => sub {
    my $x = 5;
    my $ret = sub () { 0; $x };  # <-- more than just a lexical
    # This used not to prevent inlining, though it should, and now does.
    my $sub1 = sub () { sub () { $x++ } }; # nested
    $ret;
  },
  retval      => 5,
  same_retval => 0,
  inlinable   => 0,
  deprecated  => 0,
  method      => 0,
};

use feature 'state', 'lexical_subs';
no warnings 'experimental::lexical_subs';

# Constant constants
push @tests, {
  nickname    => 'sub with constant',
  generator   => sub { sub () { 8 } },
  retval      => 8,
  same_retval => 0,
  inlinable   => 1,
  deprecated  => 0,
  method      => 0,
};
push @tests, {
  nickname    => 'sub with constant and return',
  generator   => sub { sub () { return 8 } },
  retval      => 8,
  same_retval => 0,
  inlinable   => 0,
  deprecated  => 0,
  method      => 0,
};
push @tests, {
  nickname    => 'sub with optimised statement and constant',
  generator   => sub { sub () { 0; 8 } },
  retval      => 8,
  same_retval => 0,
  inlinable   => 1,
  deprecated  => 0,
  method      => 0,
};
push @tests, {
  nickname    => 'sub with optimised statement, constant and return',
  generator   => sub { sub () { 0; return 8 } },
  retval      => 8,
  same_retval => 0,
  inlinable   => 0,
  deprecated  => 0,
  method      => 0,
};
push @tests, {
  nickname    => 'my sub with constant',
  generator   => sub { my sub x () { 8 } \&x },
  retval      => 8,
  same_retval => 0,
  inlinable   => 1,
  deprecated  => 0,
  method      => 0,
};
push @tests, {
  nickname    => 'my sub with constant and return',
  generator   => sub { my sub x () { return 8 } \&x },
  retval      => 8,
  same_retval => 0,
  inlinable   => 0,
  deprecated  => 0,
  method      => 0,
};
push @tests, {
  nickname    => 'my sub with optimised statement and constant',
  generator   => sub { my sub x () { 0; 8 } \&x },
  retval      => 8,
  same_retval => 0,
  inlinable   => 1,
  deprecated  => 0,
  method      => 0,
};
push @tests, {
  nickname    => 'my sub with optimised statement, constant and return',
  generator   => sub { my sub x () { 0; return 8 } \&x },
  retval      => 8,
  same_retval => 0,
  inlinable   => 0,
  deprecated  => 0,
  method      => 0,
};

# String eval
push @tests, {
  nickname    => 'sub () { $x } with eval in scope',
  generator   => sub {
    my $outer = 43;
    my $ret = sub () { $outer };
    eval '$outer++';
    $ret;
  },
  exception   => $exception_134138,
};
push @tests, {
  nickname    => 'sub () { $x } with s///ee in scope',
  generator   => sub {
    my $outer = 43;
    my $dummy = '$outer++';
    my $ret = sub () { $outer };
    $dummy =~ s//$dummy/ee;
    $ret;
  },
  exception   => $exception_134138,
};
push @tests, {
  nickname    => 'sub () { $x } with eval not in scope',
  generator   => sub {
    my $ret;
    {
      my $outer = 43;
      $ret = sub () { $outer };
    }
    eval '';
    $ret;
  },
  retval      => 43,
  same_retval => 0,
  inlinable   => 1,
  deprecated  => 0,
  method      => 0,
};

push @tests, {
  nickname    => 'sub () { my $x; state sub z { $x } $outer }',
  generator   => sub {
    my $outer = 43;
    sub () { my $x; state sub z { $x } $outer }
  },
  retval      => 43,
  same_retval => 0,
  inlinable   => 0,
  deprecated  => 0,
  method      => 0,
};

push @tests, {
  nickname    => 'closure after \(my $x=1)',
  generator   => sub {
    $y = \(my $x = 1);
    my $ret = sub () { $x };
    $$y += 7;
    $ret;
  },
  retval      => 8,
  same_retval => 0,
  inlinable   => 0,
  deprecated  => 0,
  method      => 0,
};

push @tests, {
  nickname    => 'sub:method with simple lexical',
  generator   => sub { my $y; sub():method{$y} },
  retval      => undef,
  same_retval => 0,
  inlinable   => 1,
  deprecated  => 0,
  method      => 1,
};
push @tests, {
  nickname    => 'sub:method with constant',
  generator   => sub { sub():method{3} },
  retval      => 3,
  same_retval => 0,
  inlinable   => 1,
  deprecated  => 0,
  method      => 1,
};
push @tests, {
  nickname    => 'my sub:method with constant',
  generator   => sub { my sub x ():method{3} \&x },
  retval      => 3,
  same_retval => 0,
  inlinable   => 1,
  deprecated  => 0,
  method      => 1,
};

push @tests, {
  nickname    => 'sub closing over state var',
  generator   => sub { state $x = 3; sub () {$x} },
  retval      => 3,
  same_retval => 0,
  inlinable   => 1,
  deprecated  => 0,
  method      => 0,
};
push @tests, {
  nickname    => 'sub closing over state var++',
  generator   => sub { state $x++; sub () { $x } },
  exception   => $exception_134138,
};


use feature 'refaliasing';
no warnings 'experimental::refaliasing';
for \%_ (@tests) {
    my $nickname = $_{nickname};
    if (exists $_{exception} and $_{exception}) {
        local $@;
        eval { my $sub = &{$_{generator}}; };
        like($@, qr/$_{exception}/, "$nickname: now throws exception (RT 134138)");
        next;
    }
    my $w;
    local $SIG{__WARN__} = sub { $w = shift };
    my $sub = &{$_{generator}};
    if (exists $_{deprecated}) {
        if ($_{deprecated}) {
            like $w, qr/^Constants from lexical variables potentially (?x:
                       )modified elsewhere are deprecated\. This will (?x:
                       )not be allowed in Perl 5\.32 at /,
                "$nickname is deprecated";
        }
        else {
            is $w, undef, "$nickname is not deprecated";
        }
    }
    if (exists $_{retval}) {
        is &$sub, $_{retval}, "retval of $nickname";
    }
    if (exists $_{same_retval}) {
        my $same = $_{same_retval} ? "same" : "different";
        &{$_{same_retval} ? \&is : \&isnt}(
            \scalar &$sub(), \scalar &$sub(),
            "$nickname gives $same retval each call"
        );
    }
    if (exists $_{inlinable}) {
        local *temp_inlinability_test = $sub;
        $w = undef;
        use warnings 'redefine';
        *temp_inlinability_test = sub (){};
	my $S = $_{inlinable} ? "Constant s" : "S";
        my $not = " not" x! $_{inlinable};
        like $w, qr/^${S}ubroutine .* redefined at /,
                "$nickname is$not inlinable";
    }
    if (exists $_{method}) {
        local *time = $sub;
        $w = undef;
        use warnings 'ambiguous';
        eval "()=time";
        if ($_{method}) {
            is $w, undef, "$nickname has :method attribute";
        }
        else {
            like $w, qr/^Ambiguous call resolved as CORE::time\(\), (?x:
                        )qualify as such or use & at /,
                "$nickname has no :method attribute";
        }
    }

    &{$_{finally} or next}
}

# This used to fail an assertion in leave_scope.  For some reason, it did
# not fail within the framework above.
sub  { my $x = "x"; my $sub = sub () { $x }; undef $sub; } ->();
pass("No assertion failure when turning on PADSTALE on lexical shared by"
    ." erstwhile constant");

{
    my $sub = sub {
        my $x = "x"x2000; sub () {$x};
    }->();
    $y = &$sub;
    $z = &$sub;
    is $z, $y, 'inlinable sub ret vals are not swipable';
}

