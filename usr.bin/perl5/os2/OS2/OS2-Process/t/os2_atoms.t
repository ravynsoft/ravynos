#! /usr/bin/perl -w

use strict;
use Test::More tests => 48;
BEGIN {use_ok 'OS2::Process'}

ok(SystemAtomTable(), 'SystemAtomTable succeeds');
my $tbl = CreateAtomTable;

ok($tbl, 'CreateAtomTable succeeds');

is(AtomLength(133, $tbl), 6, 'AtomLength of unknown atom is 6');
is(AtomLength(1, $tbl),   6, 'AtomLength of unknown atom is 6');
ok(!defined eval {AtomLength(100000, $tbl); 1}, 'AtomLength of invalid atom croaks');
# diag($@);

is(AtomUsage(134, $tbl), 65535, 'AtomUsage of unknown atom is 65535');
is(AtomUsage(1, $tbl),   65535, 'AtomUsage of unknown atom is 65535');
ok(!defined eval {AtomUsage(100000, $tbl); 1}, 'AtomUsage of invalid atom croaks');
# diag($@);

is(AtomName(134, $tbl), '#134', 'AtomName of unknown atom is #number');
is(AtomName(2, $tbl),     '#2', 'AtomName of unknown atom is #number');
ok(!defined eval {AtomName(100000, $tbl); 1}, 'AtomName of invalid atom croaks');
# diag($@);

is(FindAtom('#134', $tbl), 134, 'Name of unknown atom per #number');
is(FindAtom('#2', $tbl),     2, 'Name of unknown atom per #number');
ok(!defined eval {FindAtom('#90000', $tbl); 1}, 'Finding invalid numeric atom croaks');
# diag($@);
ok(!defined eval {FindAtom('2#', $tbl); 1}, 'Finding invalid atom croaks');
# diag($@);
ok(!defined eval {FindAtom('texxt/unnknnown', $tbl); 1}, 'Finding invalid atom croaks');
# diag($@);

is(DeleteAtom(125000, $tbl), '', 'Deleting invalid atom returns FALSE');
is(DeleteAtom(10000,  $tbl), 1, 'Deleting unknown atom returns 1');
ok(!defined eval {DeleteAtom(0, $tbl); 1}, 'Deleting zero atom croaks');
# diag($@);

is(AddAtom('#134', $tbl), 134, 'Add unknown atom per #number');
is(AddAtom('#2', $tbl),     2, 'Add unknown atom per #number');
ok(!defined eval {AddAtom('#80000', $tbl); 1}, 'Add invalid numeric atom croaks');
# diag($@);

my $a1 = AddAtom("perltest//pp$$", $tbl);
ok($a1, 'Add unknown atom per string');
my $a2 = AddAtom("perltest//p$$", $tbl);
ok($a2, 'Add another unknown atom per string');
is(AddAtom("perltest//p$$", $tbl), $a2, 'Add same unknown atom per string');
isnt($a1, $a2, 'Different strings result in different atoms');
ok($a1 > 0, 'Atom positive');
ok($a2 > 0, 'Another atom positive');
ok($a1 < 0x10000, 'Atom small');
ok($a2 < 0x10000, 'Another atom small');

is(AtomLength($a1, $tbl), length "perltest//pp$$", 'AtomLength of known atom');
is(AtomLength($a2, $tbl), length "perltest//p$$", 'AtomLength of another known atom');

is(AtomUsage($a1, $tbl), 1, 'AtomUsage of known atom');
is(AtomUsage($a2, $tbl), 2, 'AtomUsage of another known atom');

is(AtomName($a1, $tbl), "perltest//pp$$", 'AtomName of known atom');
is(AtomName($a2, $tbl), "perltest//p$$", 'AtomName of another known atom');

is(FindAtom("perltest//pp$$", $tbl), $a1, 'Name of known atom');
is(FindAtom("perltest//p$$", $tbl),  $a2, 'Name of known atom');

#$^E = 0;
ok(DeleteAtom($a1, $tbl), 'DeleteAtom of known atom');
#diag("err=$^E");
#$^E = 0;
ok(DeleteAtom($a2, $tbl), 'DeleteAtom of another known atom');
#diag("err=$^E");

ok(!defined eval {AtomUsage($a1, $tbl); 1}, 'AtomUsage of deleted known atom croaks');
# diag($@);
is(AtomUsage($a2, $tbl), 1, 'AtomUsage of another known atom');

ok(!defined eval {AtomName($a1, $tbl); 1}, 'AtomName of deleted known atom croaks');
# diag($@);
is(AtomName($a2, $tbl), "perltest//p$$", 'AtomName of undeleted another known atom');

ok(!defined eval {FindAtom("perltest//pp$$", $tbl); 1}, 'Finding known deleted atom croaks');
# diag($@);
is(FindAtom("perltest//p$$", $tbl),  $a2, 'Finding known undeleted atom');

ok(DestroyAtomTable($tbl), 'DestroyAtomTable succeeds');
