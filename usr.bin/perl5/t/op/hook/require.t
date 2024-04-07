#!perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc( qw(../lib) );
}

use strict;
use warnings;

plan(tests => 14);

{
    fresh_perl_like(
        '${^HOOK}{require__before} = "x";',
        qr!\$\{\^HOOK\}\{require__before\} may only be a CODE reference or undef!,
        { },
        '%{^HOOK} forbids non code refs (string)');
}
{
    fresh_perl_like(
        '${^HOOK}{require__before} = [];',
        qr!\$\{\^HOOK\}\{require__before\} may only be a CODE reference or undef!,
        { },
        '%{^HOOK} forbids non code refs (array)');
}
{
    fresh_perl_like(
        '${^HOOK}{require__before} = sub { die "Not allowed to load $_[0]" }; require Frobnitz;',
        qr!Not allowed to load Frobnitz\.pm!,
        { },
        '${^HOOK}{require__before} exceptions stop require');
}
{
    fresh_perl_is(
        'use lib "./lib/caller"; '.
        '${^HOOK}{require__before} = '.
        '  sub { my ($name) = @_; warn "before $name"; ' .
        '       return sub { warn "after $name" } }; ' .
        'require Apack;',
        <<'EOF_WANT',
before Apack.pm at - line 1.
before Bpack.pm at - line 1.
before Cpack.pm at - line 1.
after Cpack.pm at - line 1.
after Bpack.pm at - line 1.
after Apack.pm at - line 1.
EOF_WANT
        { },
        '${^HOOK}{require__before} with post action works as expected with t/lib/caller/Apack');
}
{
    fresh_perl_is(
        'use lib "./lib/caller"; '.
        '${^HOOK}{require__before} = '.
        '  sub { $_[0] = "Apack.pm" if $_[0] eq "Cycle.pm";'.
        '        my ($name) = @_; warn "before $name"; ' .
        '        return sub { warn "after $name" } }; ' .
        'require Cycle;',
        <<'EOF_WANT',
before Apack.pm at - line 1.
before Bpack.pm at - line 1.
before Cpack.pm at - line 1.
after Cpack.pm at - line 1.
after Bpack.pm at - line 1.
after Apack.pm at - line 1.
EOF_WANT
        { },
        '${^HOOK}{require__before} with filename rewrite works as expected (Cycle.pm -> Apack.pm)');
}
{
    fresh_perl_is(
        'use lib "./lib/caller"; '.
        '${^HOOK}{require__before} = '.
        '  sub { my ($name) = @_; my $n = ++$::counter; warn "before $name ($n)"; ' .
        '       return sub { warn "after $name ($n)" } }; ' .
        'require Cycle;',
        <<'EOF_WANT',
before Cycle.pm (1) at - line 1.
before Bicycle.pm (2) at - line 1.
before Tricycle.pm (3) at - line 1.
before Cycle.pm (4) at - line 1.
after Cycle.pm (4) at - line 1.
after Tricycle.pm (3) at - line 1.
after Bicycle.pm (2) at - line 1.
after Cycle.pm (1) at - line 1.
EOF_WANT
        { },
        '${^HOOK}{require__before} with post action with state work as expected with t/lib/caller/Cycle');
}
{
    fresh_perl_is(
        'use lib "./lib/caller"; my @seen;'.
        '${^HOOK}{require__before} = '.
        '  sub { die "Cycle detected: @seen $_[0]\n" if grep $_ eq $_[0], @seen; push @seen,$_[0]; ' .
        '       return sub { pop @seen } }; ' .
        'require Cycle;',
        <<'EOF_WANT',
Cycle detected: Cycle.pm Bicycle.pm Tricycle.pm Cycle.pm
Compilation failed in require at lib/caller/Bicycle.pm line 1.
Compilation failed in require at lib/caller/Cycle.pm line 1.
Compilation failed in require at - line 1.
EOF_WANT
        { },
        '${^HOOK}{require__before} with post action with state work as expected with t/lib/caller/Cycle');
}
{
    fresh_perl_is(
        'use lib "./lib/caller"; '.
        '${^HOOK}{require__before} = '.
        '  sub { my ($before_name) = @_; warn "before $before_name"; ' .
        '       return sub { my ($after_name) = @_; warn "after $after_name" } }; ' .
        'require Apack;',
        <<'EOF_WANT',
before Apack.pm at - line 1.
before Bpack.pm at - line 1.
before Cpack.pm at - line 1.
after Cpack.pm at - line 1.
after Bpack.pm at - line 1.
after Apack.pm at - line 1.
EOF_WANT
        { },
        '${^HOOK}{require__before} with post action and name arg works as expected');
}
{
    fresh_perl_is(
        'use lib "./lib/caller"; '.
        '${^HOOK}{require__before} = '.
        '  sub { my ($name) = @_; warn "before $name" };' .
        'require Apack;',
        <<'EOF_WANT',
before Apack.pm at - line 1.
before Bpack.pm at - line 1.
before Cpack.pm at - line 1.
EOF_WANT
        { },
        '${^HOOK}{require__before} with no post action works as expected with t/lib/caller/Apack');
}
{
    fresh_perl_is(
        'use lib "./lib/caller"; '.
        '${^HOOK}{require__after} = '.
        '  sub { my ($name) = @_; warn "after $name" };' .
        'require Apack;',
        <<'EOF_WANT',
after Cpack.pm at - line 1.
after Bpack.pm at - line 1.
after Apack.pm at - line 1.
EOF_WANT
        { },
        '${^HOOK}{require__after} works as expected with t/lib/caller/Apack');
}
{
    fresh_perl_is(
        'use lib "./lib/caller"; '.
        '%{^HOOK} = ( require__before => sub { print "before: $_[0]\n" },
                      require__after => sub { print "after: $_[0]\n" } );
         { local %{^HOOK}; require Apack; }
         print "done\n";',
         "done\n",
         { },
         'local %{^HOOK} works to clear hooks.'
    );
}
{
    fresh_perl_is(
        'use lib "./lib/caller"; '.
        '%{^HOOK} = ( require__before => sub { print "before: $_[0]\n" },
                      require__after => sub { print "after: $_[0]\n" } );
         { local %{^HOOK}; require Cycle; }
         require Apack;',
        <<'EOF_WANT',
before: Apack.pm
before: Bpack.pm
before: Cpack.pm
after: Cpack.pm
after: Bpack.pm
after: Apack.pm
EOF_WANT
         { },
         'local %{^HOOK} works to clear and restore hooks.'
    );
}
{
    fresh_perl_is(
        'use lib "./lib/caller"; '.
        '%{^HOOK} = ( require__before => sub { print "before: $_[0]\n" } );
         %{^HOOK} = ( require__after  => sub { print "after: $_[0]\n" } );
         require Apack;',
        <<'EOF_WANT',
after: Cpack.pm
after: Bpack.pm
after: Apack.pm
EOF_WANT
         { },
         '%{^HOOK} = (...); works as expected (part 1)'
    );
}

{
    fresh_perl_is(
        'use lib "./lib/caller"; '.
        '%{^HOOK} = ( require__after  => sub { print "after: $_[0]\n" } );
         %{^HOOK} = ( require__before => sub { print "before: $_[0]\n" } );
         require Apack;',
        <<'EOF_WANT',
before: Apack.pm
before: Bpack.pm
before: Cpack.pm
EOF_WANT
         { },
         '%{^HOOK} = (...); works as expected (part 2)'
    );
}
