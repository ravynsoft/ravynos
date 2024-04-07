#!./perl

#
# various stash tests
#

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

use utf8;
use open qw( :utf8 :std );

plan( tests => 49 );

#These come from op/my_stash.t
{
    use constant Myクラス => 'ꕽ::Ʉ::ꔬz::ꢨᙇ';
    
    {
        package ꕽ::Ʉ::ꔬz::ꢨᙇ;
        1;
    }
    
    for (qw(ꕽ ꕽ:: Myクラス __PACKAGE__)) {
        eval "sub { my $_ \$obj = shift; }";
        ok ! $@, "op/my_stash.t test, $_";
    }
    
    use constant NòClàss => '노pӬ::ꕽ::Ꜻ::BӢz::ʙࡆ';
    
    for (qw(노pӬ 노pӬ:: NòClàss)) {
        eval "sub { my $_ \$obj = shift; }";
        ok $@, "op/my_stash.t test";
    }
}

#op/stash.t
{
    package ᛐⲞɲe::Šꇇᚽṙᆂṗ;
    $본go::ଶfʦbᚒƴ::scalar = 1;
    
    package main;
        
    # now tests with strictures
    
    {
        use strict;
        ok( !exists $piƓ::{bodine}, q(referencing a non-existent stash element doesn't produce stricture errors) );
    }

    SKIP: {
        eval { require B; 1 } or skip "no B", 28;
    
        *b = \&B::svref_2object;
        my $CVf_ANON = B::CVf_ANON();
    
        my $sub = do {
            package 온ꪵ;
            \&{"온ꪵ"};
        };
        delete $온ꪵ::{온ꪵ};
        my $gv = b($sub)->GV;
    
        object_ok( $gv, "B::GV", "deleted stash entry leaves CV with valid GV");
        is( b($sub)->CvFLAGS & $CVf_ANON, $CVf_ANON, "...and CVf_ANON set");
        is( eval { $gv->NAME }, "__ANON__", "...and an __ANON__ name");
        is( eval { $gv->STASH->NAME }, "온ꪵ", "...but leaves stash intact");
    
        $sub = do {
            package tꖿ;
            \&{"tꖿ"};
        };
        %tꖿ:: = ();
        $gv = b($sub)->GV;
    
        object_ok( $gv, "B::GV", "cleared stash leaves CV with valid GV");
        is( b($sub)->CvFLAGS & $CVf_ANON, $CVf_ANON, "...and CVf_ANON set");
        is( eval { $gv->NAME }, "__ANON__", "...and an __ANON__ name");
        is( eval { $gv->STASH->NAME }, "tꖿ", "...but leaves stash intact");
    
        $sub = do {
            package ᖟ레ￇ;
            \&{"ᖟ레ￇ"};
        };
        undef %ᖟ레ￇ::;
        $gv = b($sub)->GV;
    
        object_ok( $gv, "B::GV", "undefed stash leaves CV with valid GV");
        is( b($sub)->CvFLAGS & $CVf_ANON, $CVf_ANON, "...and CVf_ANON set");
        is( eval { $gv->NAME }, "__ANON__", "...and an __ANON__ name");
        is( eval { $gv->STASH->NAME }, "__ANON__", "...and an __ANON__ stash");
    
        my $sub = do {
            package ꃖᚢ;
            sub { 1 };
        };
        %ꃖᚢ:: = ();
    
        my $gv = B::svref_2object($sub)->GV;
        ok($gv->isa(q/B::GV/), "cleared stash leaves anon CV with valid GV");
    
        my $st = eval { $gv->STASH->NAME };
        is($st, q/ꃖᚢ/, "...but leaves the stash intact");
    
        $sub = do {
            package fꢄᶹᵌ;
            sub { 1 };
        };
        undef %fꢄᶹᵌ::;
    
        $gv = B::svref_2object($sub)->GV;
        ok($gv->isa(q/B::GV/), "undefed stash leaves anon CV with valid GV");
    
        $st = eval { $gv->STASH->NAME };

        { local $TODO = 'STASHES not anonymized';
            is($st, q/__ANON__/, "...and an __ANON__ stash");
        }

        $sub = do {
            package sӥㄒ;
            \&{"sӥㄒ"}
        };
        my $stash_glob = delete $::{"sӥㄒ::"};
        # Now free the GV while the stash still exists (though detached)
        delete $$stash_glob{"sӥㄒ"};
        $gv = B::svref_2object($sub)->GV;
        ok($gv->isa(q/B::GV/),
        'anonymised CV whose stash is detached still has a GV');
        #fails because mro_gather_and_rename isn't clean
        is $gv->STASH->NAME, '__ANON__',
        'CV anonymised when its stash is detached becomes __ANON__::__ANON__';

        # CvSTASH should be null on a named sub if the stash has been deleted
        {
            package ＦŌŌ;
            sub Ƒಓ {}
            my $rfoo = \&Ƒಓ;
            package main;
            delete $::{'ＦŌŌ::'};
            my $cv = B::svref_2object($rfoo);
            # (is there a better way of testing for NULL ?)
            my $stash = $cv->STASH;
            like($stash, qr/B::SPECIAL/, "NULL CvSTASH on named sub");
        }
    
        # on glob reassignment, orphaned CV should have anon CvGV
    
        {
            my $r;
            eval q[
                package ＦŌŌ௨;
                sub Ƒ{};
                $r = \&Ƒ;
                *Ƒ = sub {};
            ];
            delete $ＦŌŌ௨::{Ƒ};
            my $cv = B::svref_2object($r);
            my $gv = $cv->GV;
            ok($gv->isa(q/B::GV/), "orphaned CV has valid GV");
            is($gv->NAME, '__ANON__', "orphaned CV has anon GV");
        }
    
        # deleting __ANON__ glob shouldn't break things
    
        {
            package ＦŌŌ３;
            sub 남えㄉ {};
            my $anon = sub {};
            my $남えㄉ = eval q[*남えㄉ{CODE}]; # not \&남えㄉ; need a real GV
            package main;
            delete $ＦŌŌ３::{남えㄉ}; # make named anonymous
    
            delete $ＦŌŌ３::{__ANON__}; # whoops!
            my ($cv,$gv);
            $cv = B::svref_2object($남えㄉ);
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
                package bᓙṗ;
    
                BEGIN {
                    $r = \&main::Ẃⱒcᴷ;
                }
            }
    
            my $br = B::svref_2object($r);
            is ($br->STASH->NAME, 'bᓙṗ',
                'stub records the package it was compiled in');
    
            # We need to take this reference "late", after the subroutine is
            # defined.
            $br = B::svref_2object(eval 'sub Ẃⱒcᴷ {}; \&Ẃⱒcᴷ');
            die $@ if $@;
    
            is ($br->STASH->NAME, 'main',
                'definition overrides the package it was compiled in');
            like ($br->FILE, qr/eval/,
                'definition overrides the file it was compiled in');
        }
    }
    
    # make sure having a sub called __ANON__ doesn't confuse perl.
    
    {
        package クラス;
        my $c;
        sub __ANON__ { $c = (caller(0))[3]; }
        {
            local $@;
            eval { ok(1); };
            ::like($@, qr/^Undefined subroutine &クラス::ok/);
        }
        __ANON__();
        ::is ($c, 'クラス::__ANON__', '__ANON__ sub called ok');
    }

    # Stashes that are effectively renamed
    {
        package rìle;
    
        use Config;
    
        my $obj  = bless [];
        my $globref = \*tàt;
    
        # effectively rename a stash
        *slìn:: = *rìle::; *rìle:: = *zòr::;
        
        ::is *$globref, "*rìle::tàt",
        'globs stringify the same way when stashes are moved';
        ::is ref $obj, "rìle",
        'ref() returns the same thing when an object’s stash is moved';
        ::like "$obj", qr "^rìle=ARRAY\(0x[\da-f]+\)\z",
        'objects stringify the same way when their stashes are moved';
        ::is eval '__PACKAGE__', 'rìle',
            '__PACKAGE__ returns the same when the current stash is moved';
    
        # Now detach it completely from the symtab, making it effect-
        # ively anonymous
        my $life_raft = \%slìn::;
        *slìn:: = *zòr::;
    
        ::is *$globref, "*rìle::tàt",
        'globs stringify the same way when stashes are detached';
        ::is ref $obj, "rìle",
        'ref() returns the same thing when an object’s stash is detached';
        ::like "$obj", qr "^rìle=ARRAY\(0x[\da-f]+\)\z",
        'objects stringify the same way when their stashes are detached';
        ::is eval '__PACKAGE__', 'rìle',
            '__PACKAGE__ returns the same when the current stash is detached';
    }
    
    # Setting the name during undef %stash:: should have no effect.
    {
        my $glob = \*Phòò::glòb;
        sub ò::DESTROY { eval '++$Phòò::bòr' }
        no strict 'refs';
        ${"Phòò::thòng1"} = bless [], "ò";
        undef %Phòò::;
        is "$$glob", "*__ANON__::glòb",
        "setting stash name during undef has no effect";
    }
    
    # [perl #88134] incorrect package structure
    {
        package Bèàr::;
        sub bàz{1}
        package main;
        ok eval { Bèàr::::bàz() },
        'packages ending with :: are self-consistent';
    }
    
    # [perl #88138] ' not equivalent to :: before a null
    ${"à'\0b"} = "c";
    is ${"à::\0b"}, "c", "' is equivalent to :: before a null";
}
