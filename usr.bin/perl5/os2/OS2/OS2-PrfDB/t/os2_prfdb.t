BEGIN {
    chdir 't' if -d 't/lib';
    @INC = '../lib' if -d 'lib';
    require Config; import Config;
    if (-d 'lib' and $Config{'extensions'} !~ /\bOS2(::|\/)PrfDB\b/) {
	print "1..0\n";
	exit 0;
    }
}

# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

######################### We start with some black magic to print on failure.

# Change 1..1 below to 1..last_test_to_print .
# (It may become useful if the test is moved to ./t subdirectory.)

BEGIN { $| = 1; print "1..48\n"; }
END {print "not ok 1\n" unless $loaded;}
use OS2::PrfDB;
$loaded = 1;
use strict;

print "ok 1\n";

######################### End of black magic.

# Insert your test code below (better if it prints "ok 13"
# (correspondingly "not ok 13") depending on the success of chunk 13
# of the test code):

my $inifile = "my.ini";

unlink $inifile if -w $inifile;

my $ini = OS2::Prf::Open($inifile);
print( ($ini ? "": "not "), "ok 2\n# HINI=`$ini'\n");

print( (OS2::Prf::GetLength($ini,'aaa', 'bbb') != -1) ? 
    "not ok 3\n# err: `$^E'\n" : "ok 3\n");


print( OS2::Prf::Set($ini,'aaa', 'bbb','xyz') ? "ok 4\n" :
    "not ok 4\n# err: `$^E'\n");

my $len = OS2::Prf::GetLength($ini,'aaa', 'bbb');
print( $len == 3 ? "ok 5\n" : "not ok 5# len: `$len' err: `$^E'\n");

my $val = OS2::Prf::Get($ini,'aaa', 'bbb');
print( $val eq 'xyz' ? "ok 6\n" : "not ok 6# val: `$val' err: `$^E'\n");

$val = OS2::Prf::Get($ini,'aaa', undef);
print( $val eq "bbb\0" ? "ok 7\n" : "not ok 7# val: `$val' err: `$^E'\n");

$val = OS2::Prf::Get($ini, undef, undef);
print( $val eq "aaa\0" ? "ok 8\n" : "not ok 8# val: `$val' err: `$^E'\n");

my $res = OS2::Prf::Set($ini,'aaa', 'bbb',undef);
print( $res ? "ok 9\n" : "not ok 9# err: `$^E'\n");

$val = OS2::Prf::Get($ini, undef, undef);
print( (! defined $val) ? "ok 10\n" : "not ok 10# val: `$val' err: `$^E'\n");

$val = OS2::Prf::Get($ini,'aaa', undef);
print( (! defined $val) ? "ok 11\n" : "not ok 11# val: `$val' err: `$^E'\n");

print((OS2::Prf::Close($ini) ? "" : "not ") . "ok 12\n");

my $files = OS2::Prf::Profiles();
print( (defined $files) ? "ok 13\n" : "not ok 13# err: `$^E'\n");
print( (@$files == 2) ? "ok 14\n" : "not ok 14# `@$files' err: `$^E'\n");
print "# `@$files'\n";

$ini = OS2::Prf::Open($inifile);
print( ($ini ? "": "not "), "ok 15\n# HINI=`$ini'\n");


print( OS2::Prf::Set($ini,'aaa', 'ccc','xyz') ? "ok 16\n" :
    "not ok 16\n# err: `$^E'\n");

print( OS2::Prf::Set($ini,'aaa', 'ddd','123') ? "ok 17\n" :
    "not ok 17\n# err: `$^E'\n");

print( OS2::Prf::Set($ini,'bbb', 'xxx','abc') ? "ok 18\n" :
    "not ok 18\n# err: `$^E'\n");

print( OS2::Prf::Set($ini,'bbb', 'yyy','456') ? "ok 19\n" :
    "not ok 19\n# err: `$^E'\n");

OS2::Prf::Close($ini);

my %hash1;

tie %hash1, 'OS2::PrfDB::Sub', $inifile, 'aaa';
$OS2::PrfDB::Sub::debug = 1;
print "ok 20\n";

my @a1 = keys %hash1;
print (@a1 == 2 ? "ok 21\n" : "not ok 21\n# `@a1'\n");

my @a2 = sort @a1;
print ("@a2" eq "ccc ddd" ? "ok 22\n" : "not ok 22\n# `@a2'\n");

$val = $hash1{ccc};
print ($val eq "xyz" ? "ok 23\n" : "not ok 23\n# `$val'\n");

$val = $hash1{ddd};
print ($val eq "123" ? "ok 24\n" : "not ok 24\n# `$val'\n");

print (exists $hash1{ccc} ? "ok 25\n" : "not ok 25\n# `$val'\n");

print (!exists $hash1{hhh} ? "ok 26\n" : "not ok 26\n# `$val'\n");

$hash1{hhh} = 12;
print (exists $hash1{hhh} ? "ok 27\n" : "not ok 27\n# `$val'\n");

$val = $hash1{hhh};
print ($val eq "12" ? "ok 28\n" : "not ok 28\n# `$val'\n");

delete $hash1{ccc};

untie %hash1;
print "ok 29\n";

tie %hash1, 'OS2::PrfDB::Sub', $inifile, 'aaa';
print "ok 30\n";

@a1 = keys %hash1;
print (@a1 == 2 ? "ok 31\n" : "not ok 31\n# `@a1'\n");

@a2 = sort @a1;
print ("@a2" eq "ddd hhh" ? "ok 32\n" : "not ok 32\n# `@a2'\n");

print (exists $hash1{hhh} ? "ok 33\n" : "not ok 33\n# `$val'\n");

$val = $hash1{hhh};
print ($val eq "12" ? "ok 34\n" : "not ok 34\n# `$val'\n");

%hash1 = ();
print "ok 35\n";

%hash1 = ( hhh => 12, ddd => 5);

untie %hash1;

my %hash;

tie %hash, 'OS2::PrfDB', $inifile;
print "ok 36\n";

@a1 = keys %hash;
print (@a1 == 2 ? "ok 37\n" : "not ok 37\n# `@a1'\n");

@a2 = sort @a1;
print ("@a2" eq "aaa bbb" ? "ok 38\n" : "not ok 38\n# `@a2'\n");

print (exists $hash{aaa} ? "ok 39\n" : "not ok 39\n# `$val'\n");

$val = $hash{aaa};
print (ref $val eq "HASH" ? "ok 40\n" : "not ok 40\n# `$val'\n");

%hash1 = %$val;
print "ok 41\n";

@a1 = keys %hash1;
print (@a1 == 2 ? "ok 42\n" : "not ok 31\n# `@a1'\n");

@a2 = sort @a1;
print ("@a2" eq "ddd hhh" ? "ok 43\n" : "not ok 43\n# `@a2'\n");

print (exists $hash1{hhh} ? "ok 44\n" : "not ok 44\n# `$val'\n");

$val = $hash1{hhh};
print ($val eq "12" ? "ok 45\n" : "not ok 45\n# `$val'\n");

$hash{nnn}{mmm} = 67;
print "ok 46\n";

untie %hash;

my %hash2;

tie %hash2, 'OS2::PrfDB', $inifile;
print "ok 47\n";

print ($hash2{nnn}->{mmm} eq "67" ? "ok 48\n" : "not ok 48\n# `$val'\n");

untie %hash2;
unlink $inifile;
