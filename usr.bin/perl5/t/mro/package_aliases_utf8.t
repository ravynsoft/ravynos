#!./perl

BEGIN {
    $ENV{PERL_UNICODE} = 0;
    unless (-d 'blib') {
        chdir 't' if -d 't';
    }
    require q(./test.pl);
    set_up_inc('../lib');
}

use strict;
use warnings;
use utf8;
use open qw( :utf8 :std );

plan(tests => 52);

{
    package Ｎeẁ;
    use strict;
    use warnings;

    package ऑlㄉ;
    use strict;
    use warnings;

    {
      no strict 'refs';
      *{'ऑlㄉ::'} = *{'Ｎeẁ::'};
    }
}

ok (ऑlㄉ->isa(Ｎeẁ::), 'ऑlㄉ inherits from Ｎeẁ');
ok (Ｎeẁ->isa(ऑlㄉ::), 'Ｎeẁ inherits from ऑlㄉ');

object_ok (bless ({}, ऑlㄉ::), Ｎeẁ::, 'ऑlㄉ object');
object_ok (bless ({}, Ｎeẁ::), ऑlㄉ::, 'Ｎeẁ object');


# Test that replacing a package by assigning to an existing glob
# invalidates the isa caches
for(
 {
   name => 'assigning a glob to a glob',
   code => '$life_raft = $::{"ｌㅔf::"}; *ｌㅔf:: = $::{"릭Ⱶᵀ::"}',
 },
 {
   name => 'assigning a string to a glob',
   code => '$life_raft = $::{"ｌㅔf::"}; *ｌㅔf:: = "릭Ⱶᵀ::"',
 },
 {
   name => 'assigning a stashref to a glob',
   code => '$life_raft = \%ｌㅔf::; *ｌㅔf:: = \%릭Ⱶᵀ::',
 },
) {
my $prog =    q~
     BEGIN {
         unless (-d 'blib') {
             chdir 't' if -d 't';
             @INC = '../lib';
         }
     }
     use utf8;
     use open qw( :utf8 :std );

     @숩cਲꩋ::ISA = "ｌㅔf";
     @ｌㅔf::ISA = "톺ĺФț";

     sub 톺ĺФț::Ｓᑊeಅḱ { "Woof!" }
     sub ᴖ릭ᚽʇ::Ｓᑊeಅḱ { "Bow-wow!" }

     my $thing = bless [], "숩cਲꩋ";

     # mro_package_moved needs to know to skip non-globs
     $릭Ⱶᵀ::{"ᚷꝆエcƙ::"} = 3;

     @릭Ⱶᵀ::ISA = 'ᴖ릭ᚽʇ';
     my $life_raft;
    __code__;

     print $thing->Ｓᑊeಅḱ, "\n";

     undef $life_raft;
     print $thing->Ｓᑊeಅḱ, "\n";
   ~ =~ s\__code__\$$_{code}\r; #\
utf8::encode($prog);
 fresh_perl_is
  $prog, 
  "Bow-wow!\nBow-wow!\n",
   {},
  "replacing packages by $$_{name} updates isa caches";
}

# Similar test, but with nested packages
#
#  톺ĺФț (Woof)    ᴖ릭ᚽʇ (Bow-wow)
#      |                 |
#  ｌㅔf::Side   <-   릭Ⱶᵀ::Side
#      |
#   숩cਲꩋ
#
# This test assigns 릭Ⱶᵀ:: to ｌㅔf::, indirectly making ｌㅔf::Side an
# alias to 릭Ⱶᵀ::Side (following the arrow in the diagram).
for(
 {
   name => 'assigning a glob to a glob',
   code => '$life_raft = $::{"ｌㅔf::"}; *ｌㅔf:: = $::{"릭Ⱶᵀ::"}',
 },
 {
   name => 'assigning a string to a glob',
   code => '$life_raft = $::{"ｌㅔf::"}; *ｌㅔf:: = "릭Ⱶᵀ::"',
 },
 {
   name => 'assigning a stashref to a glob',
   code => '$life_raft = \%ｌㅔf::; *ｌㅔf:: = \%릭Ⱶᵀ::',
 },
) {
 my $prog = q~
     BEGIN {
         unless (-d 'blib') {
             chdir 't' if -d 't';
             @INC = '../lib';
         }
     }
     use utf8;
     use open qw( :utf8 :std );
     @숩cਲꩋ::ISA = "ｌㅔf::Side";
     @ｌㅔf::Side::ISA = "톺ĺФț";

     sub 톺ĺФț::Ｓᑊeಅḱ { "Woof!" }
     sub ᴖ릭ᚽʇ::Ｓᑊeಅḱ { "Bow-wow!" }

     my $thing = bless [], "숩cਲꩋ";

     @릭Ⱶᵀ::Side::ISA = 'ᴖ릭ᚽʇ';
     my $life_raft;
    __code__;

     print $thing->Ｓᑊeಅḱ, "\n";

     undef $life_raft;
     print $thing->Ｓᑊeಅḱ, "\n";
   ~ =~ s\__code__\$$_{code}\r;
 utf8::encode($prog);

 fresh_perl_is
  $prog,
  "Bow-wow!\nBow-wow!\n",
   {},
  "replacing nested packages by $$_{name} updates isa caches";
}

# Another nested package test, in which the isa cache needs to be reset on
# the subclass of a package that does not exist.
#
# Parenthesized packages do not exist.
#
#  ɵűʇㄦ::인ንʵ    ( cฬnए::인ንʵ )
#       |                 |
#     Ｌфť              R익hȚ
#
#        ɵűʇㄦ  ->  cฬnए
#
# This test assigns ɵűʇㄦ:: to cฬnए::, making cฬnए::인ንʵ an alias to
# ɵűʇㄦ::인ንʵ.
#
# Then we also run the test again, but without ɵűʇㄦ::인ንʵ
for(
 {
   name => 'assigning a glob to a glob',
   code => '*cฬnए:: = *ɵűʇㄦ::',
 },
 {
   name => 'assigning a string to a glob',
   code => '*cฬnए:: = "ɵűʇㄦ::"',
 },
 {
   name => 'assigning a stashref to a glob',
   code => '*cฬnए:: = \%ɵűʇㄦ::',
 },
) {
 for my $tail ('인ንʵ', '인ንʵ::', '인ንʵ:::', '인ንʵ::::') {
  my $prog =     q~
     BEGIN {
         unless (-d 'blib') {
             chdir 't' if -d 't';
             @INC = '../lib';
         }
     }
      use utf8;
      use open qw( :utf8 :std );
      use Encode ();

      if (grep /\P{ASCII}/, @ARGV) {
        @ARGV = map { Encode::decode("UTF-8", $_) } @ARGV;
      }

      my $tail = shift;
      @Ｌфť::ISA = "ɵűʇㄦ::$tail";
      @R익hȚ::ISA = "cฬnए::$tail";
      bless [], "ɵűʇㄦ::$tail"; # autovivify the stash

     __code__;

      print "ok 1", "\n" if Ｌфť->isa("cฬnए::$tail");
      print "ok 2", "\n" if R익hȚ->isa("ɵűʇㄦ::$tail");
      print "ok 3", "\n" if R익hȚ->isa("cฬnए::$tail");
      print "ok 4", "\n" if Ｌфť->isa("ɵűʇㄦ::$tail");
    ~ =~ s\__code__\$$_{code}\r;
  utf8::encode($prog);
  fresh_perl_is
   $prog,
   "ok 1\nok 2\nok 3\nok 4\n",
    { args => [$tail] },
   "replacing nonexistent nested packages by $$_{name} updates isa caches"
     ." ($tail)";

  # Same test but with the subpackage autovivified after the assignment
  $prog =     q~
      BEGIN {
         unless (-d 'blib') {
             chdir 't' if -d 't';
             @INC = '../lib';
         }
      }
      use utf8;
      use open qw( :utf8 :std );
      use Encode ();

      if (grep /\P{ASCII}/, @ARGV) {
        @ARGV = map { Encode::decode("UTF-8", $_) } @ARGV;
      }

      my $tail = shift;
      @Ｌфť::ISA = "ɵűʇㄦ::$tail";
      @R익hȚ::ISA = "cฬnए::$tail";

     __code__;

      bless [], "ɵűʇㄦ::$tail";

      print "ok 1", "\n" if Ｌфť->isa("cฬnए::$tail");
      print "ok 2", "\n" if R익hȚ->isa("ɵűʇㄦ::$tail");
      print "ok 3", "\n" if R익hȚ->isa("cฬnए::$tail");
      print "ok 4", "\n" if Ｌфť->isa("ɵűʇㄦ::$tail");
    ~ =~ s\__code__\$$_{code}\r;
  utf8::encode($prog);
  fresh_perl_is
   $prog,
   "ok 1\nok 2\nok 3\nok 4\n",
    { args => [$tail] },
   "Giving nonexistent packages multiple effective names by $$_{name}"
     . " ($tail)";
 }
}

no warnings; # temporary; there seems to be a scoping bug, as this does not
             # work when placed in the blocks below

# Test that deleting stash elements containing
# subpackages also invalidates the isa cache.
# Maybe this does not belong in package_aliases.t, but it is closely
# related to the tests immediately preceding.
{
 @ቹऋ::ISA = ("Cuȓ", "ฮﾝᛞ");
 @Cuȓ::ISA = "Hyḹ앛Ҭテ";

 sub Hyḹ앛Ҭテ::Ｓᑊeಅḱ { "Arff!" }
 sub ฮﾝᛞ::Ｓᑊeಅḱ { "Woof!" }

 my $pet = bless [], "ቹऋ";

 my $life_raft = delete $::{'Cuȓ::'};

 is $pet->Ｓᑊeಅḱ, 'Woof!',
  'deleting a stash from its parent stash invalidates the isa caches';

 undef $life_raft;
 is $pet->Ｓᑊeಅḱ, 'Woof!',
  'the deleted stash is gone completely when freed';
}
# Same thing, but with nested packages
{
 @펱ᑦ::ISA = ("Cuȓȓ::Cuȓȓ::Cuȓȓ", "ɥwn");
 @Cuȓȓ::Cuȓȓ::Cuȓȓ::ISA = "lȺt랕ᚖ";

 sub lȺt랕ᚖ::Ｓᑊeಅḱ { "Arff!" }
 sub ɥwn::Ｓᑊeಅḱ { "Woof!" }

 my $pet = bless [], "펱ᑦ";

 my $life_raft = delete $::{'Cuȓȓ::'};

 is $pet->Ｓᑊeಅḱ, 'Woof!',
  'deleting a stash from its parent stash resets caches of substashes';

 undef $life_raft;
 is $pet->Ｓᑊeಅḱ, 'Woof!',
  'the deleted substash is gone completely when freed';
}

# [perl #77358]
my $prog =    q~#!perl -w
     BEGIN {
         unless (-d 'blib') {
             chdir 't' if -d 't';
             @INC = '../lib';
         }
     }
     use utf8;
     use open qw( :utf8 :std );
     @펱ᑦ::ISA = "T잌ዕ";
     @T잌ዕ::ISA = "Bᛆヶṝ";
     
     sub Bᛆヶṝ::Ｓᑊeಅḱ { print "Woof!\n" }
     sub lȺt랕ᚖ::Ｓᑊeಅḱ { print "Bow-wow!\n" }
     
     my $pet = bless [], "펱ᑦ";
     
     $pet->Ｓᑊeಅḱ;
     
     sub ດƓ::Ｓᑊeಅḱ { print "Hello.\n" } # strange ດƓ!
     @ດƓ::ISA = 'lȺt랕ᚖ';
     *T잌ዕ:: = delete $::{'ດƓ::'};
     
     $pet->Ｓᑊeಅḱ;
   ~;
utf8::encode($prog);
fresh_perl_is
  $prog,
  "Woof!\nHello.\n",
   { stderr => 1 },
  "Assigning a nameless package over one w/subclasses updates isa caches";

# mro_package_moved needs to make a distinction between replaced and
# assigned stashes when keeping track of what it has seen so far.
no warnings; {
    no strict 'refs';

    sub ʉ::bᓗnǩ::bᓗnǩ::ພo { "bbb" }
    sub ᵛeↄl움::ພo { "lasrevinu" }
    @ݏ엗Ƚeᵬૐᵖ::ISA = qw 'ພo::bᓗnǩ::bᓗnǩ ᵛeↄl움';
    *ພo::ବㄗ:: = *ʉ::bᓗnǩ::;   # now ʉ::bᓗnǩ:: is on both sides
    *ພo:: = *ʉ::;         # here ʉ::bᓗnǩ:: is both deleted and added
    *ʉ:: = *ቦᵕ::;          # now it is only known as ພo::bᓗnǩ::

    # At this point, before the bug was fixed, %ພo::bᓗnǩ::bᓗnǩ:: ended
    # up with no effective name, allowing it to be deleted without updating
    # its subclassesâ caches.

    my $accum = '';

    $accum .= 'ݏ엗Ƚeᵬૐᵖ'->ພo;          # bbb
    delete ${"ພo::bᓗnǩ::"}{"bᓗnǩ::"};
    $accum .= 'ݏ엗Ƚeᵬૐᵖ'->ພo;          # bbb (Oops!)
    @ݏ엗Ƚeᵬૐᵖ::ISA = @ݏ엗Ƚeᵬૐᵖ::ISA;
    $accum .= 'ݏ엗Ƚeᵬૐᵖ'->ພo;          # lasrevinu

    is $accum, 'bbblasrevinulasrevinu',
      'nested classes deleted & added simultaneously';
}
use warnings;

# mro_package_moved needs to check for self-referential packages.
# This broke Text::Template [perl #78362].
watchdog 3;
*ᕘ:: = \%::;
*Aᶜme::Mῌ::Aᶜme:: = \*Aᶜme::; # indirect self-reference
pass("mro_package_moved and self-referential packages");

# Deleting a glob whose name does not indicate its location in the symbol
# table but which nonetheless *is* in the symbol table.
{
    no strict refs=>;
    no warnings;
    @ოƐ::mഒrェ::ISA = "foᚒ";
    sub foᚒ::ວmᑊ { "aoeaa" }
    *ťວ:: = *ოƐ::;
    delete $::{"ოƐ::"};
    @C힐dᒡl았::ISA = 'ťວ::mഒrェ';
    my $accum = 'C힐dᒡl았'->ວmᑊ . '-';
    my $life_raft = delete ${"ťວ::"}{"mഒrェ::"};
    $accum .= eval { 'C힐dᒡl았'->ວmᑊ } // '<undef>';
    is $accum, 'aoeaa-<undef>',
     'Deleting globs whose loc in the symtab differs from gv_fullname'
}

# Pathological test for undeffing a stash that has an alias.
*ᵍh엞:: = *ኔƞ::;
@숩cਲꩋ::ISA = 'ᵍh엞';
undef %ᵍh엞::;
sub F렐ᛔ::ວmᑊ { "clumpren" }
eval '
  $ኔƞ::whatever++;
  @ኔƞ::ISA = "F렐ᛔ";
';
is eval { '숩cਲꩋ'->ວmᑊ }, 'clumpren',
 'Changes to @ISA after undef via original name';
undef %ᵍh엞::;
eval '
  $ᵍh엞::whatever++;
  @ᵍh엞::ISA = "F렐ᛔ";
';
is eval { '숩cਲꩋ'->ວmᑊ }, 'clumpren',
 'Changes to @ISA after undef via alias';


# Packages whose containing stashes have aliases must lose all names cor-
# responding to that container when detached.
{
 {package śmᛅḙ::በɀ} # autovivify
 *pḢ린ᚷ:: = *śmᛅḙ::;  # śmᛅḙ::በɀ now also named pḢ린ᚷ::በɀ
 *본:: = delete $śmᛅḙ::{"በɀ::"};
 # In 5.13.7, it has now lost its śmᛅḙ::በɀ name (reverting to pḢ린ᚷ::በɀ
 # as the effective name), and gained 본 as an alias.
 # In 5.13.8, both śmᛅḙ::በɀ *and* pḢ린ᚷ::በɀ names are deleted.

 # Make some methods
 no strict 'refs';
 *{"pḢ린ᚷ::በɀ::fฤmᛈ"} = sub { "hello" };
 sub Ｆルmፕṟ::fฤmᛈ { "good bye" };

 @ᵇるᣘ킨::ISA = qw "본 Ｆルmፕṟ"; # now wrongly inherits from pḢ린ᚷ::በɀ

 is fฤmᛈ ᵇるᣘ킨, "good bye",
  'detached stashes lose all names corresponding to the containing stash';
}

# Crazy edge cases involving packages ending with a single :
@촐oン::ISA = 'ᚖგ:'; # pun intended!
bless [], "ᚖგ:"; # autovivify the stash
ok "촐oン"->isa("ᚖგ:"), 'class isa "class:"';
{ no strict 'refs'; *{"ᚖგ:::"} = *ᚖგ:: }
ok "촐oン"->isa("ᚖგ"),
 'isa(ᕘ) when inheriting from "class:" which is an alias for ᕘ';
{
 no warnings;
 # The next line of code is *not* normative. If the structure changes,
 # this line needs to change, too.
 my $ᕘ = delete $ᚖგ::{":"};
 ok !촐oン->isa("ᚖგ"),
  'class that isa "class:" no longer isa ᕘ if "class:" has been deleted';
}
@촐oン::ISA = ':';
bless [], ":";
ok "촐oン"->isa(":"), 'class isa ":"';
{ no strict 'refs'; *{":::"} = *ፑňṪu앝ȋ온:: }
ok "촐oン"->isa("ፑňṪu앝ȋ온"),
 'isa(ᕘ) when inheriting from ":" which is an alias for ᕘ';
@촐oン::ISA = 'ᚖგ:';
bless [], "ᚖგ:";
{
 no strict 'refs';
 my $life_raft = \%{"ᚖგ:::"};
 *{"ᚖგ:::"} = \%ᚖგ::;
 ok "촐oン"->isa("ᚖგ"),
  'isa(ᕘ) when inheriting from "class:" after hash-to-glob assignment';
}
@촐oン::ISA = 'ŏ:';
bless [], "ŏ:";
{
 no strict 'refs';
 my $life_raft = \%{"ŏ:::"};
 *{"ŏ:::"} = "ᚖგ::";
 ok "촐oン"->isa("ᚖგ"),
  'isa(ᕘ) when inheriting from "class:" after string-to-glob assignment';
}
=cut
