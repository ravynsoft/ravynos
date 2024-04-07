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

    plan(tests => 3);
    use_ok('GDBM_File');
 }

my $wd = tempdir(CLEANUP => 1);

my %h;
my $db = tie(%h, 'GDBM_File', File::Spec->catfile($wd, 'Op_dbmx'),
             GDBM_WRCREAT, 0640);

isa_ok($db, 'GDBM_File');
SKIP: {
     skip 'GDBM_File::count not available', 1
        unless $db->can('count'); 

     $h{one} = '1';
     $h{two} = '2';
     $h{three} = '3';
     is($db->count, 3, 'count');
}

