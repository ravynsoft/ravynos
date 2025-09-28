#!./perl -w
use strict;

use Test::More;
use Config;
use File::Temp 'tempdir';
use File::Spec;
use Fcntl qw( :mode );

BEGIN {
    plan(skip_all => "GDBM_File was not built")
        unless $Config{extensions} =~ /\bGDBM_File\b/;

    plan(tests => 18);
    use_ok('GDBM_File');
}

use constant {
    DUMP_ASCII => 0,
    DUMP_BIN => 1,
    DUMP_UNKNOWN => -1
};

sub dump_format {
    my $file = shift;
    if (open(my $fd, '<', $file)) {
        $_ = <$fd>;
        if (/^# GDBM dump file created by GDBM version/) {
            return DUMP_ASCII;
        }
        if (/^!\r$/) {
            $_ = <$fd>;
            if (/^! GDBM FLAT FILE DUMP -- THIS IS NOT A TEXT FILE/) {
                return DUMP_BIN;
            }
        }
    }
    return DUMP_UNKNOWN;
}

my $wd = tempdir(CLEANUP => 1);
my $dbname = File::Spec->catfile($wd, 'Op_dbmx');
my %h;
my $db = tie(%h, 'GDBM_File', $dbname, GDBM_WRCREAT, 0640);
isa_ok($db, 'GDBM_File');
SKIP: {
     skip 'GDBM_File::dump not available', 16
        unless $db->can('dump');

     $h{one} = '1';
     $h{two} = '2';
     $h{three} = '3';

     my $dumpname = "$dbname.dump";
     is(eval { $db->dump($dumpname); 1 }, 1, "Create ASCII dump");
     is(dump_format($dumpname), DUMP_ASCII, "ASCII dump created");
     is(eval { $db->dump($dumpname); 1 }, undef, "Refuse to overwrite existing dump");
     is(eval { $db->dump($dumpname, overwrite => 1); 1 }, 1, "Working overwrite option");

     my $binname = "$dbname.bin";
     is(eval { $db->dump($binname, binary => 1); 1 }, 1, "Create binary dump");
     is(dump_format($binname), DUMP_BIN, "Binary dump created");
     is(eval { $db->dump($binname, binary => 1); 1 }, undef, "Refuse to overwrite existing binary dump");
     is(eval { $db->dump($binname, binary => 1, overwrite => 1); 1 }, 1, "Working overwrite option (binary format)");

     untie %h;
     $db->close;

     #
     # Test loading the database
     #

     $db = tie(%h, 'GDBM_File', $dbname, GDBM_NEWDB, 0640);
     isa_ok($db, 'GDBM_File');

     is(eval { $db->load($dumpname); 1 }, 1, "Loading from ascii dump");
     is_deeply({map { $_ => $h{$_} } sort keys %h},
        { one => 1, two => 2, three => 3 },
        "Restored database content");

     is(eval { $db->load($dumpname); 1 }, undef, "Refuse to replace existing keys");

     is(eval { $db->load($dumpname, replace => 1); 1 }, 1, "Replace existing keys");

     untie %h;
     $db->close;

     #
     # Test loading the database from binary dump
     #
     $db = tie(%h, 'GDBM_File', $dbname, GDBM_NEWDB, 0640);
     isa_ok($db, 'GDBM_File');

     is(eval { $db->load($binname); 1 }, 1, "Loading from binary dump");
     is_deeply({map { $_ => $h{$_} } sort keys %h},
        { one => 1, two => 2, three => 3 },
        "Restored database content");

}


