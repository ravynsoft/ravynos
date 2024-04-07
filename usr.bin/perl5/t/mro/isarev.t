#!./perl

BEGIN {
    unless (-d 'blib') {
        chdir 't' if -d 't';
    }
    require q(./test.pl);
    set_up_inc('../lib') unless -d 'blib';
}

use strict;
use warnings;
plan(tests => 24);

use mro;

sub i {
 my @args = @_;
 @_
  = (
     join(" ", sort @{mro::get_isarev $args[0]}),
     join(" ", sort @args[1..$#args-1]),
     pop @args
    );
 goto &is;
}

# Basic isarev updating, when @ISA changes
@Pastern::ISA = "BodyPart::Ungulate";
@Scur::ISA    = "BodyPart::Ungulate";
@BodyPart::Ungulate::ISA = "BodyPart";
i BodyPart => qw [ BodyPart::Ungulate Pastern Scur ],
 'subclasses and subsubclasses are added to isarev';
@Pastern::ISA = ();
i BodyPart => qw [ BodyPart::Ungulate Scur ],
 'single deletion from isarev';
@BodyPart::Ungulate::ISA = ();
i BodyPart => qw [ ], 'recursive deletion from isarev';
                      # except underneath it is not actually recursive


# More complicated tests that move packages around

@Huskey::ISA = "Dog";
@Dog::ISA = "Canid";
@Wolf::ISA = "Canid";
@Some::Brand::Name::ISA = "Dog::Bone";
@Dog::Bone::ISA = "Treat";
@Free::Time::ISA = "Treat";
@MyCollar::ISA = "Dog::Collar::Leather";
@Dog::Collar::Leather::ISA = "Collar";
@Another::Collar::ISA = "Collar";
*Tike:: = *Dog::;
delete $::{"Dog::"};
i Canid=>qw[ Wolf Tike ],
 "deleting a stash elem updates isarev entries";
i Treat=>qw[ Free::Time Tike::Bone ],
 "deleting a nested stash elem updates isarev entries";
i Collar=>qw[ Another::Collar Tike::Collar::Leather ],
 "deleting a doubly nested stash elem updates isarev entries";

@Goat::ISA = "Ungulate";
@Goat::Dairy::ISA = "Goat";
@Goat::Dairy::Toggenburg::ISA = "Goat::Dairy";
@Weird::Thing::ISA = "g";
*g:: = *Goat::;
i Goat => qw[ Goat::Dairy Goat::Dairy::Toggenburg Weird::Thing ],
 "isarev includes subclasses of aliases";
delete $::{"g::"};
i Ungulate => qw[ Goat Goat::Dairy Goat::Dairy::Toggenburg ],
 "deleting an alias to a package updates isarev entries";
i"Goat" => qw[ Goat::Dairy Goat::Dairy::Toggenburg ],
 "deleting an alias to a package updates isarev entries of nested stashes";
i"Goat::Dairy" => qw[ Goat::Dairy::Toggenburg ],
 "deleting an stash alias updates isarev entries of doubly nested stashes";
i g => qw [ Weird::Thing ],
 "subclasses of the deleted alias become part of its isarev";

@Caprine::ISA = "Hoofed::Mammal";
@Caprine::Dairy::ISA = "Caprine";
@Caprine::Dairy::Oberhasli::ISA = "Caprine::Dairy";
@Whatever::ISA = "Caprine";
*Caprid:: = *Caprine::;
*Caprine:: = *Chevre::;
i"Hoofed::Mammal" => qw[ Caprid ],
 "replacing a stash updates isarev entries";
i Chevre => qw[ Caprid::Dairy Whatever ],
 "replacing nested stashes updates isarev entries";

@Disease::Eye::ISA = "Disease";
@Disease::Eye::Infectious::ISA = "Disease::Eye";
@Keratoconjunctivitis::ISA = "Disease::Ophthalmic::Infectious";
*Disease::Ophthalmic:: = *Disease::Eye::;
{package some_random_new_symbol::Infectious} # autovivify
*Disease::Ophthalmic:: = *some_random_new_symbol::;
i Disease => qw[ Disease::Eye Disease::Eye::Infectious ],
 "replacing an alias of a stash updates isarev entries";
i"Disease::Eye" => qw[ Disease::Eye::Infectious ],
 "replacing an alias of a stash containing another updates isarev entries";
i"some_random_new_symbol::Infectious" => qw[ Keratoconjunctivitis ],
 "replacing an alias updates isarev of stashes nested in the replacement";

# Globs ending with :: have autovivified stashes in them by default. We
# want one without a stash.
undef *Empty::;
@Null::ISA = "Empty";
@Null::Null::ISA = "Empty::Empty";
{package Zilch::Empty} # autovivify it
*Empty:: = *Zilch::;
i Zilch => qw[ Null ], "assigning to an empty spot updates isarev";
i"Zilch::Empty" => qw[ Null::Null ],
 "assigning to an empty spot updates isarev of nested packages";

# Classes inheriting from multiple classes that get moved in a single
# assignment.
@foo::ISA = ("B", "B::B");
{package A::B}
my $A = \%A::;     # keep a ref
*A:: = 'whatever'; # clobber it
*B:: = $A;         # assign to two superclasses of foo at the same time
# There should be no A::B isarev entry.
i"A::B" => qw [], 'assigning to two superclasses at the same time';
ok !foo->isa("A::B"),
 "A class must not inherit from its superclass's former name";

# undeffing globs
@alpha::ISA = 'beta';
$_ = \*alpha::ISA;    # hang on to the glob
undef *alpha::ISA;
i beta => qw [], "undeffing an ISA glob deletes isarev entries";
@az::ISA = 'buki';
$_ = \*az::ISA;
undef *az::;
i buki => qw [], "undeffing a package glob deletes isarev entries";

# Package aliasing/clobbering when the clobbered package has grandchildren
# by inheritance.
@bar::ISA = 'phoo';
@subclassA::ISA = "subclassB";
@subclassB::ISA = "bar";
*bar:: = *baz::;
i phoo => qw [],
 'clobbering a class w/multiple layers of subclasses updates its parent';

@Thrat::ISA = 'Smin';
%Thrat:: = ();
i Smin => qw [], '%Package:: list assignment';
