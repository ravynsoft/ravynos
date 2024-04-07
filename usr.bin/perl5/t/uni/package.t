#!./perl

# Checks if 'package' work as intended.

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
}

plan (tests => 18);

use utf8;
use open qw( :utf8 :std );

package Føø::Bær { }

package クラス { }

package ฟọ::バッズ { }

ok 1, "sanity check. If we got this far, UTF-8 in package names is legal.";

#The next few come from comp/package.t
{

    $ㄅĽuṞfⳐ = 123;
    
    package ꑭʑ;

    sub ニュー {bless [];}
    $bar = 4;
    {
        package 압Ƈ;
        $ㄅĽuṞfⳐ = 5;
    }
    
    {
        no warnings qw(syntax deprecated);
        $압Ƈ'd읯ⱪ = 6;        #'
    }
    
    $ꑭʑ = 2;
    
    $ꑭʑ = join(':', sort(keys %ꑭʑ::));
    $압Ƈ = join(':', sort(keys %압Ƈ::));
    
    ::is $ꑭʑ, 'BEGIN:bar:ニュー:ꑭʑ:압Ƈ', "comp/stash.t test 1";
    ::is $압Ƈ, "d읯ⱪ:ㄅĽuṞfⳐ", "comp/stash.t test 2";

    {
        no warnings qw(syntax deprecated);
        ::is $main'ㄅĽuṞfⳐ, 123, "comp/stash.t test 3";
    }

    package 압Ƈ;

    ::is $ㄅĽuṞfⳐ, 5, "comp/stash.t test 4";
    eval '::is $ㄅĽuṞfⳐ, 5, "comp/stash.t test 5";';
    eval 'package main; is $ㄅĽuṞfⳐ, 123, "comp/stash.t test 6";';
    ::is $ㄅĽuṞfⳐ, 5, "comp/stash.t test 7";

    #This is actually pretty bad, as caller() wasn't clean to begin with.
    package main;
    sub ㄘ { caller(0) }
    
    sub ƒஓ {
    my $s = shift;
    if ($s) {
            package ᛔQR;
            main::ㄘ();
    }
    }
    
    is((ƒஓ(1))[0], 'ᛔQR', "comp/stash.t test 8");
    
    my $Q = ꑭʑ->ニュー();
    undef %ꑭʑ::;
    eval { $a = *ꑭʑ::ニュー{PACKAGE}; };
    is $a, "__ANON__", "comp/stash.t test 9";

    {
        local $@;
        eval { $Q->param; };
        like $@, qr/^Can't use anonymous symbol table for method lookup/, "comp/stash.t test 10";
    }
    
    like "$Q", qr/^__ANON__=/, "comp/stash.t test 11";

    is ref $Q, "__ANON__", "comp/stash.t test 12";

    package bugⅲⅱⅴⅵⅱ { #not really latin, but bear with me, I'm not Damian.
        ::is( __PACKAGE__,   'bugⅲⅱⅴⅵⅱ', "comp/stash.t test 13");
        ::is( eval('__PACKAGE__'), 'bugⅲⅱⅴⅵⅱ', "comp/stash.t test 14");
    }
}

#This comes from comp/package_block.t
{
    local $@;
    eval q[package ᕘ {];
    like $@, qr/\AMissing right curly /, "comp/package_block.t test";
}

# perl #105922

{
   my $latin_1 = "þackage";
   my $utf8    = "þackage";
   utf8::downgrade($latin_1);
   utf8::upgrade($utf8);

   local $@;
   eval { $latin_1->can("yadda") };
   ok(!$@, "latin1->meth works");

   local $@;
   eval { $utf8->can("yadda") };
   ok(!$@, "utf8->meth works");
}
