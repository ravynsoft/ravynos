#!./perl -w

BEGIN {
    chdir 't' if -d 't';
    @INC = qw(../lib lib);
    require "./test.pl";
}

# This test depends on t/lib/Devel/switchd*.pm.

plan(tests => 21);

my $r;

my $filename = tempfile();
SKIP: {
	open my $f, ">$filename"
	    or skip( "Can't write temp file $filename: $!" );
	print $f <<'__SWDTEST__';
package Bar;
sub bar { $_[0] * $_[0] }
package Foo;
sub foo {
  my $s;
  $s += Bar::bar($_) for 1..$_[0];
}
package main;
Foo::foo(3);
__SWDTEST__
    close $f;
    $| = 1; # Unbufferize.
    $r = runperl(
		 switches => [ '-Ilib', '-f', '-d:switchd' ],
		 progfile => $filename,
		 args => ['3'],
		);
    like($r,
qr/^sub<Devel::switchd::import>;import<Devel::switchd>;DB<main,$::tempfile_regexp,9>;sub<Foo::foo>;DB<Foo,$::tempfile_regexp,5>;DB<Foo,$::tempfile_regexp,6>;sub<Bar::bar>;DB<Bar,$::tempfile_regexp,2>;sub<Bar::bar>;DB<Bar,$::tempfile_regexp,2>;sub<Bar::bar>;DB<Bar,$::tempfile_regexp,2>;$/,
    'Got debugging output: 1');
    $r = runperl(
		 switches => [ '-Ilib', '-f', '-d:switchd=a,42' ],
		 progfile => $filename,
		 args => ['4'],
		);
    like($r,
qr/^sub<Devel::switchd::import>;import<Devel::switchd a 42>;DB<main,$::tempfile_regexp,9>;sub<Foo::foo>;DB<Foo,$::tempfile_regexp,5>;DB<Foo,$::tempfile_regexp,6>;sub<Bar::bar>;DB<Bar,$::tempfile_regexp,2>;sub<Bar::bar>;DB<Bar,$::tempfile_regexp,2>;sub<Bar::bar>;DB<Bar,$::tempfile_regexp,2>;$/,
    'Got debugging output: 2');
    $r = runperl(
		 switches => [ '-Ilib', '-f', '-d:-switchd=a,42' ],
		 progfile => $filename,
		 args => ['4'],
		);
    like($r,
qr/^sub<Devel::switchd::unimport>;unimport<Devel::switchd a 42>;DB<main,$::tempfile_regexp,9>;sub<Foo::foo>;DB<Foo,$::tempfile_regexp,5>;DB<Foo,$::tempfile_regexp,6>;sub<Bar::bar>;DB<Bar,$::tempfile_regexp,2>;sub<Bar::bar>;DB<Bar,$::tempfile_regexp,2>;sub<Bar::bar>;DB<Bar,$::tempfile_regexp,2>;$/,
    'Got debugging output: 3');
}

# [perl #71806]
cmp_ok(
  runperl(       # less is useful for something :-)
   switches => [ '"-Mless ++INC->{q-Devel/_.pm-}"' ],
   progs    => [
    '#!perl -d:_',
    'sub DB::DB{} print scalar @{q/_</.__FILE__}',
   ],
  ),
 '>',
  0,
 'The debugger can see the lines of the main program under #!perl -d',
);

like
  runperl(
   switches => [ '"-Mless ++INC->{q-Devel/_.pm-}"' ],
   progs    => [
    '#!perl -d:_',
    'sub DB::DB{} print line=>__LINE__',
   ],
  ),
  qr/line2/,
 '#!perl -d:whatever does not throw line numbers off';

# [perl #48332]
like(
  runperl(
   switches => [ '-Ilib', '-d:switchd_empty' ],
   progs    => [
    'sub foo { print qq _1\n_ }',
    '*old_foo = \&foo;',
    '*foo = sub { print qq _2\n_ };',
    'old_foo(); foo();',
   ],
  ),
  qr "1\r?\n2\r?\n",
 'Subroutine redefinition works in the debugger [perl #48332]',
);

# [rt.cpan.org #69862]
like(
  runperl(
   switches => [ '-Ilib', '-d:switchd_empty' ],
   progs    => [
    'sub DB::sub { goto &$DB::sub }',
    'sub foo { print qq _1\n_ }',
    'sub bar { print qq _2\n_ }',
    'delete $::{foo}; eval { foo() };',
    'my $bar = *bar; undef *bar; eval { &$bar };',
   ],
  ),
  qr "1\r?\n2\r?\n",
 'Subroutines no longer found under their names can be called',
);

# [rt.cpan.org #69862]
like(
  runperl(
   switches => [ '-Ilib', '-d:switchd_empty' ],
   progs    => [
    'sub DB::sub { goto &$DB::sub }',
    'sub foo { goto &bar::baz; }',
    'sub bar::baz { print qq _ok\n_ }',
    'delete $::{bar::::};',
    'foo();',
   ],
  ),
  qr "ok\r?\n",
 'No crash when calling orphaned subroutine via goto &',
);

# test when DB::DB is seen but not defined [perl #114990]
like(
  runperl(
    switches => [ '-Ilib', '-d:nodb' ],
    prog     => [ '1' ],
    stderr   => 1,
  ),
  qr/^No DB::DB routine defined/,
  "No crash when *DB::DB exists but not &DB::DB",
);
like(
  runperl(
    switches => [ '-Ilib' ],
    prog     => 'sub DB::DB; BEGIN { $^P = 0x22; } for(0..9){ warn }',
    stderr   => 1,
  ),
  qr/^No DB::DB routine defined/,
  "No crash when &DB::DB exists but isn't actually defined",
);
# or seen and defined later
is(
  runperl(
    switches => [ '-Ilib', '-d:nodb' ], # nodb.pm contains *DB::DB...if 0
    prog     => 'warn; sub DB::DB { print qq-ok\n-; exit }',
    stderr   => 1,
  ),
  "ok\n",
  "DB::DB works after '*DB::DB if 0'",
);

# [perl #115742] Recursive DB::DB clobbering its own pad
like(
  runperl(
    switches => [ '-Ilib' ],
    progs    => [ split "\n", <<'='
     BEGIN {
      $^P = 0x22;
     }
     package DB;
     sub DB {
      my $x = 42;
      return if $__++;
      $^D |= 1 << 30; # allow recursive calls
      main::foo();
      print $x//q-u-, qq-\n-;
     }
     package main;
     chop;
     sub foo { chop; }
=
    ],
    stderr   => 1,
  ),
  qr/42/,
  "Recursive DB::DB does not clobber its own pad",
);

# [perl #118627]
like(
  runperl(
   switches => [ '-Ilib', '-d:switchd_empty' ],
   prog     => 'print @{q|_<-e|}',
  ),
  qr "use Devel::switchd_empty;(?:BEGIN|\r?\nprint)",
                         # miniperl tacks a BEGIN block on to the same line
 'Copy on write does not mangle ${"_<-e"}[0] [perl #118627]',
);

# PERL5DB with embedded newlines
{
    local $ENV{PERL5DB} = "sub DB::DB{}\nwarn";
    is(
      runperl(
       switches => [ '-Ilib', '-ld' ],
       prog     => 'warn',
       stderr   => 1
      ),
      "Warning: something's wrong.\n"
     ."Warning: something's wrong at -e line 1.\n",
     'PERL5DB with embedded newlines',
    );
}

# test that DB::goto works
is(
  runperl(
   switches => [ '-Ilib', '-d:switchd_goto' ],
   prog => 'sub baz { print qq|hello;\n| } sub foo { goto &baz } foo()',
   stderr => 1,
  ),
  "goto<main::baz>;hello;\n",
  "DB::goto"
);

# Test that %DB::lsub is not vivified
is(
  runperl(
   switches => [ '-Ilib', '-d:switchd_empty' ],
   progs => ['sub DB::sub {} sub foo : lvalue {} foo();',
             'print qq-ok\n- unless defined *DB::lsub{HASH}'],
  ),
  "ok\n",
  "%DB::lsub is not vivified"
);

# Test setting of breakpoints without *DB::dbline aliased
is(
  runperl(
   switches => [ '-Ilib', '-d:nodb' ],
   progs => [ split "\n",
    'sub DB::DB {
      $DB::single = 0, return if $DB::single; print qq[ok\n]; exit
     }
     ${q(_<).__FILE__}{6} = 1; # set a breakpoint
     sub foo {
         die; # line 6
     }
     foo();
    '
   ],
   stderr => 1
  ),
  "ok\n",
  "setting breakpoints without *DB::dbline aliased"
);

# [perl #121255]
# Check that utf8 caches are flushed when $DB::sub is set
is(
  runperl(
   switches => [ '-Ilib', '-d:switchd_empty' ],
   progs => [ split "\n",
    'sub DB::sub{length($DB::sub); goto &$DB::sub}
     ${^UTF8CACHE}=-1;
     print
       eval qq|sub oo\x{25f} { 42 }
               sub ooooo\x{25f} { oo\x{25f}() }
               ooooo\x{25f}()| 
        || $@,
       qq|\n|;
    '
   ],
   stderr => 1
  ),
  "42\n",
  'UTF8 length caches on $DB::sub are flushed'
);

# [perl #122771] -d conflicting with sort optimisations
is(
  runperl(
   switches => [ '-Ilib', '-d:switchd_empty' ],
   prog => 'BEGIN { $^P &= ~0x4 } sort { $$b <=> $$a } (); print qq-42\n-',
  ),
  "42\n",
  '-d does not conflict with sort optimisations'
);

SKIP: {
  skip_if_miniperl("under miniperl", 1);
is(
  runperl(
   switches => [ '-Ilib', '-d:switchd_empty' ],
   progs => [ split "\n",
    'use bignum;
     $DB::single=2;
     print qq/debugged\n/;
    '
   ],
   stderr => 1
  ),
  "debugged\n",
  "\$DB::single set to overload"
);
}

# [perl #123748]
#
# On some platforms, it's possible that calls to getenv() will
# return a pointer to statically allocated data that may be
# overwritten by subsequent calls to getenv/putenv/setenv/unsetenv.
#
# In perl.c, s = PerlEnv_GetEnv("PERL5OPT") is called, and
# then moreswitches(s), which, if -d:switchd_empty is given,
# will call my_setenv("PERL5DB", "use Devel::switchd_empty"),
# and then return to continue parsing s.
#
# This may need -Accflags="-DPERL_USE_SAFE_PUTENV" to fail on
# affected systems.
{
local $ENV{PERL5OPT} = '-d:switchd_empty';

like(
  runperl(
   switches => [ '-Ilib' ], prog => 'print q(hi)',
  ),
  qr/hi/,
 'putenv does not interfere with PERL5OPT parsing',
);
}
