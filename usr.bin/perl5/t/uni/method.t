#!./perl -w

#
# test method calls and autoloading.
#

BEGIN {
    chdir 't' if -d 't';
    require "./test.pl";
    set_up_inc( qw(. ../lib ../cpan/parent/lib) );
	require './charset_tools.pl';
}

use strict;
use utf8;
use open qw( :utf8 :std );
no warnings 'once';

plan(tests => 62);

#Can't use bless yet, as it might not be clean

sub F::ｂ { ::is shift, "F";  "UTF8 meth"       }
sub Ｆ::b { ::is shift, "Ｆ";  "UTF8 Stash"     }
sub Ｆ::ｂ { ::is shift, "Ｆ"; "UTF8 Stash&meth" }

is(F->ｂ, "UTF8 meth", "If the method is in UTF-8, lookup works through explicitly named methods");
is(F->${\"ｂ"}, "UTF8 meth", '..as does for ->${\""}');
eval { F->${\"ｂ\0nul"} };
ok $@, "If the method is in UTF-8, lookup is nul-clean";

is(Ｆ->b, "UTF8 Stash", "If the stash is in UTF-8, lookup works through explicitly named methods");
is(Ｆ->${\"b"}, "UTF8 Stash", '..as does for ->${\""}');
eval { Ｆ->${\"b\0nul"} };
ok $@, "If the stash is in UTF-8, lookup is nul-clean";

is(Ｆ->ｂ, "UTF8 Stash&meth", "If both stash and method are in UTF-8, lookup works through explicitly named methods");
is(Ｆ->${\"ｂ"}, "UTF8 Stash&meth", '..as does for ->${\""}');
eval { Ｆ->${\"ｂ\0nul"} };
ok $@, "Even if both stash and method are in UTF-8, lookup is nul-clean";

eval { my $ref = \my $var; $ref->ｍｅｔｈｏｄ };
like $@, qr/Can't call method "ｍｅｔｈｏｄ" on unblessed reference /u;

{
    use utf8;
    use open qw( :utf8 :std );

    my $e;
    
    eval '$e = bless {}, "Ｅ::Ａ"; Ｅ::Ａ->ｆｏｏ()';
    like ($@, qr/^\QCan't locate object method "ｆｏｏ" via package "Ｅ::Ａ" at/u);
    eval '$e = bless {}, "Ｅ::Ｂ"; $e->ｆｏｏ()';  
    like ($@, qr/^\QCan't locate object method "ｆｏｏ" via package "Ｅ::Ｂ" at/u);
    eval 'Ｅ::Ｃ->ｆｏｏ()';
    like ($@, qr/^\QCan't locate object method "ｆｏｏ" via package "Ｅ::Ｃ" (perhaps /u);
    
    eval 'UNIVERSAL->Ｅ::Ｄ::ｆｏｏ()';
    like ($@, qr/^\QCan't locate object method "ｆｏｏ" via package "Ｅ::Ｄ" (perhaps /u);
    eval 'my $e = bless {}, "UNIVERSAL"; $e->Ｅ::Ｅ::ｆｏｏ()';
    like ($@, qr/^\QCan't locate object method "ｆｏｏ" via package "Ｅ::Ｅ" (perhaps /u);
    
    $e = bless {}, "Ｅ::Ｆ";  # force package to exist
    eval 'UNIVERSAL->Ｅ::Ｆ::ｆｏｏ()';
    like ($@, qr/^\QCan't locate object method "ｆｏｏ" via package "Ｅ::Ｆ" at/u);
    eval '$e = bless {}, "UNIVERSAL"; $e->Ｅ::Ｆ::ｆｏｏ()';
    like ($@, qr/^\QCan't locate object method "ｆｏｏ" via package "Ｅ::Ｆ" at/u);
}

is(do { use utf8; use open qw( :utf8 :std ); eval 'Ｆｏｏ->ｂｏｏｇｉｅ()';
	  $@ =~ /^\QCan't locate object method "ｂｏｏｇｉｅ" via package "Ｆｏｏ" (perhaps /u ? 1 : $@}, 1);

#This reimplements a bit of _fresh_perl() from test.pl, as we want to decode
#the output of that program before using it.
SKIP: {
    skip_if_miniperl('no dynamic loading on miniperl, no Encode');

    my $prog = q!use utf8; use open qw( :utf8 :std ); sub Ｔ::DESTROY { $x = $_[0]; } bless [], "Ｔ";!;
    utf8::decode($prog);

    my $tmpfile = tempfile();
    my $runperl_args = {};
    $runperl_args->{progfile} = $tmpfile;
    $runperl_args->{stderr} = 1;

    open TEST, '>', $tmpfile or die "Cannot open $tmpfile: $!";

    print TEST $prog;
    close TEST or die "Cannot close $tmpfile: $!";

    my $results = runperl(%$runperl_args);

    require Encode;
    $results = Encode::decode("UTF-8", $results);

    like($results,
            qr/DESTROY created new reference to dead object 'Ｔ' during global destruction./u,
            "DESTROY creating a new reference to the object generates a warning in UTF-8.");
}

package Føø::Bær {
    sub new { bless {}, shift }
    sub nèw { bless {}, shift }
}

like( Føø::Bær::new("Føø::Bær"), qr/Føø::Bær=HASH/u, 'Can access new directly through a UTF-8 package.' );
like( Føø::Bær->new, qr/Føø::Bær=HASH/u, 'Can access new as a method through a UTF-8 package.' );
like( Føø::Bær::nèw("Føø::Bær"), qr/Føø::Bær=HASH/u, 'Can access nèw directly through a UTF-8 package.' );
like( Føø::Bær->nèw, qr/Føø::Bær=HASH/u, 'Can access nèw as a method through a UTF-8 package.' );

is( ref Føø::Bær->new, 'Føø::Bær');

my $new_ascii = "new";
my $new_latin = "nèw";
my $e_with_grave = byte_utf8a_to_utf8n("\303\250");
my $e_with_grave_escaped= $e_with_grave=~s/\x{a8}/\\\\x\\{a8\\}/r;
my $new_utf8  = "n${e_with_grave}w";
my $newoct    = "n${e_with_grave}w";
utf8::decode($new_utf8);

like( Føø::Bær->$new_ascii, qr/Føø::Bær=HASH/u, "Can access \$new_ascii, [$new_ascii], stored in a scalar, as a method, through a UTF-8 package." );
like( Føø::Bær->$new_latin, qr/Føø::Bær=HASH/u, "Can access \$new_latin, [$new_latin], stored in a scalar, as a method, through a UTF-8 package." );
like( Føø::Bær->$new_utf8, qr/Føø::Bær=HASH/u, "Can access \$new_utf8, [$new_utf8], stored in a scalar, as a method, through a UTF-8 package." );
{
    local $@;
    eval { Føø::Bær->$newoct };
    like($@, qr/Can't locate object method "n${e_with_grave_escaped}w" via package "Føø::Bær"/u,
        "Can't access [$newoct], stored in a scalar, as a method through a UTF-8 package." );
}


like( nèw Føø::Bær, qr/Føø::Bær=HASH/u, "Can access [nèw] as a method through a UTF-8 indirect object package.");

my $pkg_latin_1 = 'Føø::Bær';

like( $pkg_latin_1->new, qr/Føø::Bær=HASH/u, 'Can access new as a method when the UTF-8 package name is in a scalar.');
like( $pkg_latin_1->nèw, qr/Føø::Bær=HASH/u, 'Can access nèw as a method when the UTF-8 package name is in a scalar.');

like( $pkg_latin_1->$new_ascii, qr/Føø::Bær=HASH/u, "Can access \$new_ascii, [$new_ascii], stored in a scalar, as a method, when the UTF-8 package name is also in a scalar.");
like( $pkg_latin_1->$new_latin, qr/Føø::Bær=HASH/u, "Can access \$new_latin, [$new_latin], stored in a scalar, as a method, when the UTF-8 package name is also in a scalar.");
like( $pkg_latin_1->$new_utf8, qr/Føø::Bær=HASH/u, "Can access \$new_utf8, [$new_utf8], stored in a scalar, as a method, when the UTF-8 package name is also in a scalar." );
{
    local $@;

    eval { $pkg_latin_1->$newoct };
    like($@, qr/Can't locate object method "n${e_with_grave_escaped}w" via package "Føø::Bær"/u,
        "Can't access [$newoct], stored in a scalar, as a method, when the UTF-8 package name is also in a scalar.");
}

ok !!Føø::Bær->can($new_ascii), "->can works for [$new_ascii]";
ok !!Føø::Bær->can($new_latin), "->can works for [$new_latin]";
ok((not !!Føø::Bær->can($newoct)), "->can doesn't work for [$newoct]");

package クラス {
    sub new { bless {}, shift }
    sub ニュー { bless {}, shift }
}

like( クラス::new("クラス"), qr/クラス=HASH/u);
like( クラス->new, qr/クラス=HASH/u);

like( クラス::ニュー("クラス"), qr/クラス=HASH/u);
like( クラス->ニュー, qr/クラス=HASH/u);

like( ニュー クラス, qr/クラス=HASH/u, "Indirect object is UTF-8, as is the class.");

is( ref クラス->new, 'クラス');
is( ref クラス->ニュー, 'クラス');

package Foo::Bar {
    our @ISA = qw( Føø::Bær );
}

package Foo::Bàz {
    use parent qw( -norequire Føø::Bær );
}

package ฟọ::バッズ {
    use parent qw( -norequire Føø::Bær クラス );
}

ok(Foo::Bar->new, 'Simple inheritance works by pushing into @ISA,');
ok(Foo::Bar->nèw, 'Even with UTF-8 methods');

ok(Foo::Bàz->new, 'Simple inheritance works with parent using -norequire,');
ok(Foo::Bàz->nèw, 'Even with UTF-8 methods');

ok(ฟọ::バッズ->new, 'parent using -norequire, in a UTF-8 package.');
ok(ฟọ::バッズ->nèw, 'Also works with UTF-8 methods');
ok(ฟọ::バッズ->ニュー, 'Even methods from an UTF-8 parent');

BEGIN {no strict 'refs';
       ++${"\xff::foo"} if $::IS_ASCII;
       ++${"\xdf::foo"} if $::IS_EBCDIC;
       } # autovivify the package
package ÿ {                                 # without UTF8
 sub AUTOLOAD {
  if ($::IS_ASCII) {
    ::is our $AUTOLOAD,
      "\xff::\x{100}", '$AUTOLOAD made from Latin1 package + UTF8 sub';
  }
  else {
    ::is our $AUTOLOAD,
      "\xdf::\x{100}", '$AUTOLOAD made from Latin1 package + UTF8 sub';
    }
  }
}
ÿ->${\"\x{100}"};

#This test should go somewhere else.
#DATA was being generated in the wrong package.
package ʑ;
no strict 'refs';

::ok( *{"ʑ::DATA"}{IO}, "DATA is generated in the right glob");
::ok !defined(*{"main::DATA"}{IO});
::is scalar <DATA>, "Some data\n";

__DATA__
Some data
