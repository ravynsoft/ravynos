#!./perl

BEGIN {
    chdir 't' if -d 't';
    require "./test.pl";
    set_up_inc( qw(../lib) );
}

plan( tests => 55 );

# Used to segfault (bug #15479)
fresh_perl_like(
    'delete $::{STDERR}; my %a = ""',
    qr/Odd number of elements in hash assignment at - line 1\./,
    { switches => [ '-w' ] },
    'delete $::{STDERR} and print a warning',
);

# Used to segfault
fresh_perl_is(
    'BEGIN { $::{"X::"} = 2 }',
    '',
    { switches => [ '-w' ] },
    q(Insert a non-GV in a stash, under warnings 'once'),
);

# Used to segfault, too
SKIP: {
 skip_if_miniperl('requires XS');
  fresh_perl_like(
    'sub foo::bar{}; $mro::{get_mro}=*foo::bar; undef %foo::; require mro',
     qr/^Subroutine mro::get_mro redefined at /,
    { switches => [ '-w' ] },
    q(Defining an XSUB over an existing sub with no stash under warnings),
  );
}

# Used to warn
# Unbalanced string table refcount: (1) for "A::" during global destruction.
# for ithreads.
{
    local $ENV{PERL_DESTRUCT_LEVEL} = 2;
    fresh_perl_is(
		  'package A::B; sub a { // }; %A::=""',
		  '',
		  {},
		  );
    # Variant of the above which creates an object that persists until global
    # destruction, and triggers an assertion failure prior to change
    # a420522db95b7762
    fresh_perl_is(
		  'use Exporter; package A; sub a { // }; delete $::{$_} for keys %::',
		  '',
		  {},
		  );
}

# now tests with strictures

{
    use strict;
    ok( !exists $pig::{bodine}, q(referencing a non-existent stash element doesn't produce stricture errors) );
}

SKIP: {
    eval { require B; 1 } or skip "no B", 29;

    *b = \&B::svref_2object;
    my $CVf_ANON = B::CVf_ANON();

    my $sub = do {
        package one;
        \&{"one"};
    };
    delete $one::{one};
    my $gv = b($sub)->GV;

    object_ok( $gv, "B::GV", "deleted stash entry leaves CV with valid GV");
    is( b($sub)->CvFLAGS & $CVf_ANON, $CVf_ANON, "...and CVf_ANON set");
    is( eval { $gv->NAME }, "__ANON__", "...and an __ANON__ name");
    is( eval { $gv->STASH->NAME }, "one", "...but leaves stash intact");

    $sub = do {
        package two;
        \&{"two"};
    };
    %two:: = ();
    $gv = b($sub)->GV;

    object_ok( $gv, "B::GV", "cleared stash leaves CV with valid GV");
    is( b($sub)->CvFLAGS & $CVf_ANON, $CVf_ANON, "...and CVf_ANON set");
    is( eval { $gv->NAME }, "__ANON__", "...and an __ANON__ name");
    is( eval { $gv->STASH->NAME }, "two", "...but leaves stash intact");

    $sub = do {
        package three;
        \&{"three"};
    };
    undef %three::;
    $gv = b($sub)->GV;

    object_ok( $gv, "B::GV", "undefed stash leaves CV with valid GV");
    is( b($sub)->CvFLAGS & $CVf_ANON, $CVf_ANON, "...and CVf_ANON set");
    is( eval { $gv->NAME }, "__ANON__", "...and an __ANON__ name");
    is( eval { $gv->STASH->NAME }, "__ANON__", "...and an __ANON__ stash");

    my $sub = do {
	package four;
	sub { 1 };
    };
    %four:: = ();

    my $gv = B::svref_2object($sub)->GV;
    ok($gv->isa(q/B::GV/), "cleared stash leaves anon CV with valid GV");

    my $st = eval { $gv->STASH->NAME };
    is($st, q/four/, "...but leaves the stash intact");

    my $sub = do {
	package five;
	sub { 1 };
    };
    undef %five::;

    $gv = B::svref_2object($sub)->GV;
    ok($gv->isa(q/B::GV/), "undefed stash leaves anon CV with valid GV");

    $st = eval { $gv->STASH->NAME };
    { local $TODO = 'STASHES not anonymized';
	is($st, q/__ANON__/, "...and an __ANON__ stash");
    }

    my $sub = do {
	package six;
	\&{"six"}
    };
    my $stash_glob = delete $::{"six::"};
    # Now free the GV while the stash still exists (though detached)
    delete $$stash_glob{"six"};
    $gv = B::svref_2object($sub)->GV;
    ok($gv->isa(q/B::GV/),
       'anonymised CV whose stash is detached still has a GV');
    is $gv->STASH->NAME, '__ANON__',
     'CV anonymised when its stash is detached becomes __ANON__::__ANON__';

    # CvSTASH should be null on a named sub if the stash has been deleted
    {
	package FOO;
	sub foo {}
	my $rfoo = \&foo;
	package main;
	delete $::{'FOO::'};
	my $cv = B::svref_2object($rfoo);
	# (is there a better way of testing for NULL ?)
	my $stash = $cv->STASH;
	like($stash, qr/B::SPECIAL/, "NULL CvSTASH on named sub");
    }

    # on glob reassignment, orphaned CV should have anon CvGV

    {
	my $r;
	eval q[
	    package FOO2;
	    sub f{};
	    $r = \&f;
	    *f = sub {};
	];
	delete $FOO2::{f};
	my $cv = B::svref_2object($r);
	my $gv = $cv->GV;
	ok($gv->isa(q/B::GV/), "orphaned CV has valid GV");
	is($gv->NAME, '__ANON__', "orphaned CV has anon GV");
    }

    # deleting __ANON__ glob shouldn't break things

    {
	package FOO3;
	sub named {};
	my $anon = sub {};
	my $named = eval q[*named{CODE}]; # not \&named; we want a real GV
	package main;
	delete $FOO3::{named}; # make named anonymous

	delete $FOO3::{__ANON__}; # whoops!
	my ($cv,$gv);
	$cv = B::svref_2object($named);
	$gv = $cv->GV;
	ok($gv->isa(q/B::GV/), "ex-named CV has valid GV");
	is($gv->NAME, '__ANON__', "ex-named CV has anon GV");

	$cv = B::svref_2object($anon);
	$gv = $cv->GV;
	ok($gv->isa(q/B::GV/), "anon CV has valid GV");
	is($gv->NAME, '__ANON__', "anon CV has anon GV");
    }

    {
	my $r;
	{
	    package bloop;

	    BEGIN {
		$r = \&main::whack;
	    }
	}

	my $br = B::svref_2object($r);
	is ($br->STASH->NAME, 'bloop',
	    'stub records the package it was compiled in');
	# Arguably this shouldn't quite be here, but it's easy to add it
	# here, and tricky to figure out a different good place for it.
	like ($br->FILE, qr/stash/i,
	      'stub records the file it was compiled in');

	# We need to take this reference "late", after the subroutine is
	# defined.
	$br = B::svref_2object(eval 'sub whack {}; \&whack');
	die $@ if $@;

	is ($br->STASH->NAME, 'main',
	    'definition overrides the package it was compiled in');
	like ($br->FILE, qr/eval/,
	      'definition overrides the file it was compiled in');
    }
}

# [perl #58530]
fresh_perl_is(
    'sub foo { 1 }; use overload q/""/ => \&foo;' .
        'delete $main::{foo}; bless []',
    "",
    {},
    "no segfault with overload/deleted stash entry [#58530]",
);

# make sure having a sub called __ANON__ doesn't confuse perl.

{
    my $c;
    sub __ANON__ { $c = (caller(0))[3]; }
    __ANON__();
    is ($c, 'main::__ANON__', '__ANON__ sub called ok');
}


# Stashes that are effectively renamed
{
    package rile;

    use Config;

    my $obj  = bless [];
    my $globref = \*tat;

    # effectively rename a stash
    *slin:: = *rile::; *rile:: = *zor::;
    
    ::is *$globref, "*rile::tat",
     'globs stringify the same way when stashes are moved';
    ::is ref $obj, "rile",
     'ref() returns the same thing when an object\'s stash is moved';
    ::like "$obj", qr "^rile=ARRAY\(0x[\da-f]+\)\z",
     'objects stringify the same way when their stashes are moved';
    ::is eval '__PACKAGE__', 'rile',
	 '__PACKAGE__ returns the same when the current stash is moved';

    # Now detach it completely from the symtab, making it effect-
    # ively anonymous
    my $life_raft = \%slin::;
    *slin:: = *zor::;

    ::is *$globref, "*rile::tat",
     'globs stringify the same way when stashes are detached';
    ::is ref $obj, "rile",
     'ref() returns the same thing when an object\'s stash is detached';
    ::like "$obj", qr "^rile=ARRAY\(0x[\da-f]+\)\z",
     'objects stringify the same way when their stashes are detached';
    ::is eval '__PACKAGE__', 'rile',
	 '__PACKAGE__ returns the same when the current stash is detached';
}

# Setting the name during undef %stash:: should have no effect.
{
    my $glob = \*Phoo::glob;
    sub o::DESTROY { eval '++$Phoo::bar' }
    no strict 'refs';
    ${"Phoo::thing1"} = bless [], "o";
    undef %Phoo::;
    is "$$glob", "*__ANON__::glob",
      "setting stash name during undef has no effect";
}

# [perl #88134] incorrect package structure
{
    package Bear::;
    sub baz{1}
    package main;
    ok eval { Bear::::baz() },
     'packages ending with :: are self-consistent';
}

# [perl #88138] ' not equivalent to :: before a null
${"a'\0b"} = "c";
is ${"a::\0b"}, "c", "' is equivalent to :: before a null";

# [perl #101486] Clobbering the current package
ok eval '
     package Do;
     BEGIN { *Do:: = *Re:: }
     sub foo{};
     1
  ', 'no crashing or errors when clobbering the current package';

# Bareword lookup should not vivify stashes
is runperl(
    prog =>
      'sub foo { print shift, qq-\n- } SUPER::foo bar if 0; foo SUPER',
    stderr => 1,
   ),
   "SUPER\n",
   'bareword lookup does not vivify stashes';

is runperl(
    prog => '%0; *bar::=*foo::=0; print qq|ok\n|',
    stderr => 1,
   ),
   "ok\n",
   '[perl #123847] no crash from *foo::=*bar::=*glob_with_hash';

is runperl(
    prog => '%h; *::::::=*h; delete $::{q|::|}; print qq|ok\n|',
    stderr => 1,
   ),
   "ok\n",
   '[perl #128086] no crash from assigning hash to *:::::: & deleting it';

is runperl(
    prog => 'BEGIN { %: = 0; $^W=1}; print qq|ok\n|',
    stderr => 1,
   ),
   "ok\n",
   "[perl #128238] don't treat %: as a stash (needs 2 colons)";

is runperl(
    prog => 'BEGIN { $::{q|foo::|}=*ENV; $^W=1}; print qq|ok\n|',
    stderr => 1,
   ),
   "ok\n",
   "[perl #128238] non-stashes in stashes";

is runperl(
    prog => '%:: = (); print *{q|::|}, qq|\n|',
    stderr => 1,
   ),
   "*main::main::\n",
   "[perl #129869] lookup %:: by name after clearing %::";
