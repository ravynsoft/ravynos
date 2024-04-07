#!./perl -w
use strict;

use Test::More;
use Config;
use File::Temp 'tempdir';
use File::Spec;

BEGIN {
    plan(skip_all => "GDBM_File was not built")
	unless $Config{extensions} =~ /\bGDBM_File\b/;

    # https://rt.perl.org/Public/Bug/Display.html?id=117967
    plan(skip_all => "GDBM_File is flaky in $^O")
        if $^O =~ /darwin/;

    plan(tests => 9);
    use_ok('GDBM_File');
}

my $wd = tempdir(CLEANUP => 1);
my $dbname = File::Spec->catfile($wd, 'Op_dbmx');
my %h;
my $db = tie(%h, 'GDBM_File', $dbname, GDBM_WRCREAT, 0640);
isa_ok($db, 'GDBM_File');
SKIP: {
     my $name = eval { $db->dbname } or do {
         skip "gdbm_setopt GET calls not implemented", 7
             if $@ =~ /GDBM_File::dbname not implemented/;
     };
     is($db->dbname, $dbname, 'get dbname');
     is(eval { $db->dbname("a"); }, undef, 'dbname - bad usage');
     is($db->flags, GDBM_WRCREAT, 'get flags');
     is($db->sync_mode, 0, 'get sync_mode');
     is($db->sync_mode(1), 1, 'set sync_mode');
     is($db->sync_mode, 1, 'get sync_mode');
   SKIP: {
         my ($maj, $min) = GDBM_File->GDBM_version;
         skip "gdbm too old", 1 if $maj != 1 || $maj == 1 && $min < 9;
         isnt($db->mmapsize, 0, "get mmapsize");
     }
}
