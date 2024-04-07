#!./perl

BEGIN {
    unless (-d 'blib') {
        chdir 't' if -d 't';
    }
    require q(./test.pl);
    set_up_inc('../lib');
}

use strict;
use warnings;
plan(tests => 54);

{
    package New;
    use strict;
    use warnings;

    package Old;
    use strict;
    use warnings;

    {
      no strict 'refs';
      *{'Old::'} = *{'New::'};
    }
}

ok (Old->isa (New::), 'Old inherits from New');
ok (New->isa (Old::), 'New inherits from Old');

object_ok (bless ({}, Old::), New::, 'Old object');
object_ok (bless ({}, New::), Old::, 'New object');


# Test that replacing a package by assigning to an existing glob
# invalidates the isa caches
for(
 {
   name => 'assigning a glob to a glob',
   code => '$life_raft = $::{"Left::"}; *Left:: = $::{"Right::"}',
 },
 {
   name => 'assigning a string to a glob',
   code => '$life_raft = $::{"Left::"}; *Left:: = "Right::"',
 },
 {
   name => 'assigning a stashref to a glob',
   code => '$life_raft = \%Left::; *Left:: = \%Right::',
 },
) {
 fresh_perl_is
   q~
     @Subclass::ISA = "Left";
     @Left::ISA = "TopLeft";

     sub TopLeft::speak { "Woof!" }
     sub TopRight::speak { "Bow-wow!" }

     my $thing = bless [], "Subclass";

     # mro_package_moved needs to know to skip non-globs
     $Right::{"gleck::"} = 3;

     @Right::ISA = 'TopRight';
     my $life_raft;
    __code__;

     print $thing->speak, "\n";

     undef $life_raft;
     print $thing->speak, "\n";
   ~ =~ s\__code__\$$_{code}\r,
  "Bow-wow!\nBow-wow!\n",
   {},
  "replacing packages by $$_{name} updates isa caches";
}

# Similar test, but with nested packages
#
#  TopLeft (Woof)    TopRight (Bow-wow)
#      |                 |
#  Left::Side   <-   Right::Side
#      |
#   Subclass
#
# This test assigns Right:: to Left::, indirectly making Left::Side an
# alias to Right::Side (following the arrow in the diagram).
for(
 {
   name => 'assigning a glob to a glob',
   code => '$life_raft = $::{"Left::"}; *Left:: = $::{"Right::"}',
 },
 {
   name => 'assigning a string to a glob',
   code => '$life_raft = $::{"Left::"}; *Left:: = "Right::"',
 },
 {
   name => 'assigning a stashref to a glob',
   code => '$life_raft = \%Left::; *Left:: = \%Right::',
 },
) {
 fresh_perl_is
   q~
     @Subclass::ISA = "Left::Side";
     @Left::Side::ISA = "TopLeft";

     sub TopLeft::speak { "Woof!" }
     sub TopRight::speak { "Bow-wow!" }

     my $thing = bless [], "Subclass";

     @Right::Side::ISA = 'TopRight';
     my $life_raft;
    __code__;

     print $thing->speak, "\n";

     undef $life_raft;
     print $thing->speak, "\n";
   ~ =~ s\__code__\$$_{code}\r,
  "Bow-wow!\nBow-wow!\n",
   {},
  "replacing nested packages by $$_{name} updates isa caches";
}

# Another nested package test, in which the isa cache needs to be reset on
# the subclass of a package that does not exist.
#
# Parenthesized packages do not exist.
#
#  outer::inner    ( clone::inner )
#       |                 |
#     left              right
#
#        outer  ->  clone
#
# This test assigns outer:: to clone::, making clone::inner an alias to
# outer::inner.
#
# Then we also run the test again, but without outer::inner
for(
 {
   name => 'assigning a glob to a glob',
   code => '*clone:: = *outer::',
 },
 {
   name => 'assigning a string to a glob',
   code => '*clone:: = "outer::"',
 },
 {
   name => 'assigning a stashref to a glob',
   code => '*clone:: = \%outer::',
 },
) {
 for my $tail ('inner', 'inner::', 'inner:::', 'inner::::') {
  fresh_perl_is
    q~
      my $tail = shift;
      @left::ISA = "outer::$tail";
      @right::ISA = "clone::$tail";
      bless [], "outer::$tail"; # autovivify the stash

     __code__;

      print "ok 1", "\n" if left->isa("clone::$tail");
      print "ok 2", "\n" if right->isa("outer::$tail");
      print "ok 3", "\n" if right->isa("clone::$tail");
      print "ok 4", "\n" if left->isa("outer::$tail");
    ~ =~ s\__code__\$$_{code}\r,
   "ok 1\nok 2\nok 3\nok 4\n",
    { args => [$tail] },
   "replacing nonexistent nested packages by $$_{name} updates isa caches"
     ." ($tail)";

  # Same test but with the subpackage autovivified after the assignment
  fresh_perl_is
    q~
      my $tail = shift;
      @left::ISA = "outer::$tail";
      @right::ISA = "clone::$tail";

     __code__;

      bless [], "outer::$tail";

      print "ok 1", "\n" if left->isa("clone::$tail");
      print "ok 2", "\n" if right->isa("outer::$tail");
      print "ok 3", "\n" if right->isa("clone::$tail");
      print "ok 4", "\n" if left->isa("outer::$tail");
    ~ =~ s\__code__\$$_{code}\r,
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
 @Pet::ISA = ("Cur", "Hound");
 @Cur::ISA = "Hylactete";

 sub Hylactete::speak { "Arff!" }
 sub Hound::speak { "Woof!" }

 my $pet = bless [], "Pet";

 my $life_raft = delete $::{'Cur::'};

 is $pet->speak, 'Woof!',
  'deleting a stash from its parent stash invalidates the isa caches';

 undef $life_raft;
 is $pet->speak, 'Woof!',
  'the deleted stash is gone completely when freed';
}
# Same thing, but with nested packages
{
 @Pett::ISA = ("Curr::Curr::Curr", "Hownd");
 @Curr::Curr::Curr::ISA = "Latrator";

 sub Latrator::speak { "Arff!" }
 sub Hownd::speak { "Woof!" }

 my $pet = bless [], "Pett";

 my $life_raft = delete $::{'Curr::'};

 is $pet->speak, 'Woof!',
  'deleting a stash from its parent stash resets caches of substashes';

 undef $life_raft;
 is $pet->speak, 'Woof!',
  'the deleted substash is gone completely when freed';
}

# [perl #77358]
fresh_perl_is
   q~#!perl -w
     @Pet::ISA = "Tike";
     @Tike::ISA = "Barker";
     
     sub Barker::speak { print "Woof!\n" }
     sub Latrator::speak { print "Bow-wow!\n" }
     
     my $pet = bless [], "Pet";
     
     $pet->speak;
     
     sub Dog::speak { print "Hello.\n" } # strange dog!
     @Dog::ISA = 'Latrator';
     *Tike:: = delete $::{'Dog::'};
     
     $pet->speak;
   ~,
  "Woof!\nHello.\n",
   { stderr => 1 },
  "Assigning a nameless package over one w/subclasses updates isa caches";

# mro_package_moved needs to make a distinction between replaced and
# assigned stashes when keeping track of what it has seen so far.
no warnings; {
    no strict 'refs';

    sub bar::blonk::blonk::phoo { "bbb" }
    sub veclum::phoo { "lasrevinu" }
    @feedlebomp::ISA = qw 'phoo::blonk::blonk veclum';
    *phoo::baz:: = *bar::blonk::;   # now bar::blonk:: is on both sides
    *phoo:: = *bar::;         # here bar::blonk:: is both deleted and added
    *bar:: = *boo::;          # now it is only known as phoo::blonk::

    # At this point, before the bug was fixed, %phoo::blonk::blonk:: ended
    # up with no effective name, allowing it to be deleted without updating
    # its subclasses’ caches.

    my $accum = '';

    $accum .= 'feedlebomp'->phoo;          # bbb
    delete ${"phoo::blonk::"}{"blonk::"};
    $accum .= 'feedlebomp'->phoo;          # bbb (Oops!)
    @feedlebomp::ISA = @feedlebomp::ISA;
    $accum .= 'feedlebomp'->phoo;          # lasrevinu

    is $accum, 'bbblasrevinulasrevinu',
      'nested classes deleted & added simultaneously';
}
use warnings;

# mro_package_moved needs to check for self-referential packages.
# This broke Text::Template [perl #78362].
watchdog 3;
*foo:: = \%::;
*Acme::META::Acme:: = \*Acme::; # indirect self-reference
pass("mro_package_moved and self-referential packages");

# Deleting a glob whose name does not indicate its location in the symbol
# table but which nonetheless *is* in the symbol table.
{
    no strict refs=>;
    no warnings;
    @one::more::ISA = "four";
    sub four::womp { "aoeaa" }
    *two:: = *one::;
    delete $::{"one::"};
    @Childclass::ISA = 'two::more';
    my $accum = 'Childclass'->womp . '-';
    my $life_raft = delete ${"two::"}{"more::"};
    $accum .= eval { 'Childclass'->womp } // '<undef>';
    is $accum, 'aoeaa-<undef>',
     'Deleting globs whose loc in the symtab differs from gv_fullname'
}

# Pathological test for undeffing a stash that has an alias.
*Ghelp:: = *Neen::;
@Subclass::ISA = 'Ghelp';
undef %Ghelp::;
sub Frelp::womp { "clumpren" }
eval '
  $Neen::whatever++;
  @Neen::ISA = "Frelp";
';
is eval { 'Subclass'->womp }, 'clumpren',
 'Changes to @ISA after undef via original name';
undef %Ghelp::;
eval '
  $Ghelp::whatever++;
  @Ghelp::ISA = "Frelp";
';
is eval { 'Subclass'->womp }, 'clumpren',
 'Changes to @ISA after undef via alias';


# Packages whose containing stashes have aliases must lose all names cor-
# responding to that container when detached.
{
 {package smare::baz} # autovivify
 *phring:: = *smare::;  # smare::baz now also named phring::baz
 *bonk:: = delete $smare::{"baz::"};
 # In 5.13.7, it has now lost its smare::baz name (reverting to phring::baz
 # as the effective name), and gained bonk as an alias.
 # In 5.13.8, both smare::baz *and* phring::baz names are deleted.

 # Make some methods
 no strict 'refs';
 *{"phring::baz::frump"} = sub { "hello" };
 sub frumper::frump { "good bye" };

 @brumkin::ISA = qw "bonk frumper"; # now wrongly inherits from phring::baz

 is frump brumkin, "good bye",
  'detached stashes lose all names corresponding to the containing stash';
}

# Crazy edge cases involving packages ending with a single :
@Colon::ISA = 'Organ:'; # pun intended!
bless [], "Organ:"; # autovivify the stash
ok "Colon"->isa("Organ:"), 'class isa "class:"';
{ no strict 'refs'; *{"Organ:::"} = *Organ:: }
ok "Colon"->isa("Organ"),
 'isa(foo) when inheriting from "class:" which is an alias for foo';
{
 no warnings;
 # The next line of code is *not* normative. If the structure changes,
 # this line needs to change, too.
 my $foo = delete $Organ::{":"};
 ok !Colon->isa("Organ"),
  'class that isa "class:" no longer isa foo if "class:" has been deleted';
}
@Colon::ISA = ':';
bless [], ":";
ok "Colon"->isa(":"), 'class isa ":"';
{ no strict 'refs'; *{":::"} = *Punctuation:: }
ok "Colon"->isa("Punctuation"),
 'isa(foo) when inheriting from ":" which is an alias for foo';
@Colon::ISA = 'Organ:';
bless [], "Organ:";
{
 no strict 'refs';
 my $life_raft = \%{"Organ:::"};
 *{"Organ:::"} = \%Organ::;
 ok "Colon"->isa("Organ"),
  'isa(foo) when inheriting from "class:" after hash-to-glob assignment';
}
@Colon::ISA = 'O:';
bless [], "O:";
{
 no strict 'refs';
 my $life_raft = \%{"O:::"};
 *{"O:::"} = "Organ::";
 ok "Colon"->isa("Organ"),
  'isa(foo) when inheriting from "class:" after string-to-glob assignment';
}

@Bazo::ISA = "Fooo::bar";
sub Fooo::bar::ber { 'baz' }
sub UNIVERSAL::ber { "black sheep" }
Bazo->ber;
local *Fooo:: = \%Baro::;
{
    no warnings;
    is 'Bazo'->ber, 'black sheep', 'localised *glob=$stashref assignment';
}

# $Stash::{"entries::"} that are not globs.
# These used to crash.
$NotGlob::{"NotGlob::"} = 0; () = $NewNotGlob::NotGlob::;
*NewNotGlob:: = *NotGlob::;
pass(
   "no crash when clobbering sub-'stash' whose parent stash entry is no GV"
);
