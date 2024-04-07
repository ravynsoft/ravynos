#!./perl

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require Config; import Config;
    require Test::More; import Test::More;
    plan(tests, 12);
}

require AnyDBM_File;
use Fcntl;


$Is_Dosish = ($^O eq 'amigaos' || $^O eq 'MSWin32' ||
	      $^O eq 'os2' ||
	      $^O eq 'cygwin');

my $filename = "Any_dbmx$$";
unlink <"$filename*">;

umask(0);

ok( tie(%h,AnyDBM_File,"$filename", O_RDWR|O_CREAT, 0640), "Tie");

$Dfile = "$filename.pag";
if (! -e $Dfile) {
	($Dfile) = <$filename*>;
}

SKIP:
{
    skip( "different file permission semantics",1)
                      if $Is_Dosish;
    ($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,$atime,$mtime,$ctime,
     $blksize,$blocks) = stat($Dfile);
    ok(($mode & 0777) == 0640 , "File permissions");
}

while (($key,$value) = each(%h)) {
    $i++;
}

ok(!$i,"Hash created empty");

$h{'goner1'} = 'snork';

$h{'abc'} = 'ABC';
$h{'def'} = 'DEF';
$h{'jkl','mno'} = "JKL\034MNO";
$h{'a',2,3,4,5} = join("\034",'A',2,3,4,5);
$h{'a'} = 'A';
$h{'b'} = 'B';
$h{'c'} = 'C';
$h{'d'} = 'D';
$h{'e'} = 'E';
$h{'f'} = 'F';
$h{'g'} = 'G';
$h{'h'} = 'H';
$h{'i'} = 'I';

$h{'goner2'} = 'snork';
delete $h{'goner2'};

untie(%h);
ok(tie(%h,AnyDBM_File,"$filename", O_RDWR, 0640),"Re-tie hash");

$h{'j'} = 'J';
$h{'k'} = 'K';
$h{'l'} = 'L';
$h{'m'} = 'M';
$h{'n'} = 'N';
$h{'o'} = 'O';
$h{'p'} = 'P';
$h{'q'} = 'Q';
$h{'r'} = 'R';
$h{'s'} = 'S';
$h{'t'} = 'T';
$h{'u'} = 'U';
$h{'v'} = 'V';
$h{'w'} = 'W';
$h{'x'} = 'X';
$h{'y'} = 'Y';
$h{'z'} = 'Z';

$h{'goner3'} = 'snork';

delete $h{'goner1'};
delete $h{'goner3'};

@keys = keys(%h);
@values = values(%h);

ok( ($#keys == 29 && $#values == 29),'$#keys == $#values');

while (($key,$value) = each(%h)) {
    if ($key eq $keys[$i] && $value eq $values[$i] && $key eq lc($value)) {
	$key =~ y/a-z/A-Z/;
	$i++ if $key eq $value;
    }
}

ok($i == 30,"keys and values match");

@keys = ('blurfl', keys(%h), 'dyick');
ok($#keys == 31,"Correct number of keys");

$h{'foo'} = '';
$h{''} = 'bar';

# check cache overflow and numeric keys and contents
$ok = 1;
for ($i = 1; $i < 200; $i++) { $h{$i + 0} = $i + 0; }
for ($i = 1; $i < 200; $i++) { $ok = 0 unless $h{$i} == $i; }
ok($ok, "cache overflow and numeric keys and contents");

($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,$atime,$mtime,$ctime,
   $blksize,$blocks) = stat($Dfile);
ok($size > 0, "check file size");

@h{0..200} = 200..400;
@foo = @h{0..200};
ok( join(':',200..400) eq join(':',@foo), "hash slice");

ok($h{'foo'} eq '', "empty value");

my $compact = '';

if ($AnyDBM_File::ISA[0] eq 'DB_File' && ($DB_File::db_ver >= 2.004010 && $DB_File::db_ver < 3.001)) {
     ($major, $minor, $patch) = ($DB_File::db_ver =~ /^(\d+)\.(\d\d\d)(\d\d\d)/) ;
     $major =~ s/^0+// ;
     $minor =~ s/^0+// ;
     $patch =~ s/^0+// ;
     $compact = "$major.$minor.$patch" ;
     #
     # anydbm.t test 12 will fail when AnyDBM_File uses the combination of
     # DB_File and Berkeley DB 2.4.10 (or greater). 
     # You are using DB_File $DB_File::VERSION and Berkeley DB $compact
     #
     # Berkeley DB 2 from version 2.4.10 onwards does not allow null keys.
     # This feature returned with version 3.1
     #
}

SKIP:
{
  skip( "db v$compact, no null key support", 1) if $compact;
  ok($h{''} eq 'bar','null key');
}

untie %h;

if ($^O eq 'VMS') {
  unlink "$filename.sdbm_dir", $Dfile;
} else {
  unlink "$filename.dir", $Dfile;
}
