#!/usr/bin/perl -w
# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

######################### We start with some black magic to print on failure.

BEGIN {
    $|= 1;

    # when building perl, skip this test if Win32API::File isn't being built
    if ( $ENV{PERL_CORE} ) {
	require Config;
	if ( $Config::Config{extensions} !~ m:(?<!\S)Win32API/File(?!\S): ) {
	    print "1..0 # Skip Win32API::File extension not built\n";
	    exit();
	}
    }

    print "1..270\n";
}
END {print "not ok 1\n" unless $loaded;}

# Win32API::File does an implicit "require Win32", but
# the ../lib directory in @INC will no longer work once
# we chdir() into the TEMP directory.

use Win32;
use File::Spec;
use Carp;
use Carp::Heavy;

use Win32API::File qw(:ALL);
$loaded = 1;
print "ok 1\n";

######################### End of black magic.

$test= 1;

use strict qw(subs);

$temp= File::Spec->tmpdir();
$dir= "W32ApiF.tmp";

$ENV{WINDIR} = $ENV{SYSTEMROOT} if not exists $ENV{WINDIR};

chdir( $temp )
  or  die "# Can't cd to temp directory, $temp: $!\n";
$tempdir = File::Spec->catdir($temp,$dir);
if(  -d $dir  ) {
    print "# deleting ",File::Spec->catdir($temp,$dir,'*'),"\n" if glob "$dir/*";

    for (glob "$dir/*") {
	chmod 0777, $_;
	unlink $_;
    }
    rmdir $dir or die "Could not rmdir $dir: $!";
}
mkdir( $dir, 0777 )
  or  die "# Can't create temp dir, $tempdir: $!\n";
print "# chdir $tempdir\n";
chdir( $dir )
  or  die "# Can't cd to my dir, $tempdir: $!\n";
$h1= createFile( "ReadOnly.txt", "r", { Attributes=>"r" } );
$ok=  ! $h1  &&  Win32API::File::_fileLastError() == 2; # could not find the file
$ok or print "# ","".fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 2
if(  ! $ok  ) {   CloseHandle($h1);   unlink("ReadOnly.txt");   }

$ok= $h1= createFile( "ReadOnly.txt", "wcn", { Attributes=>"r" } );
$ok or print "# ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 3

$ok= WriteFile( $h1, "Original text\n", 0, [], [] );
$ok or print "# ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 4

$h2= createFile( "ReadOnly.txt", "rcn" );
$ok= ! $h2  &&  Win32API::File::_fileLastError() == 80; # file exists
$ok or print "# ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 5
if(  ! $ok  ) {   CloseHandle($h2);   }

$h2= createFile( "ReadOnly.txt", "rwke" );
$ok= ! $h2  &&  Win32API::File::_fileLastError() == 5; # access is denied
$ok or print "# ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 6
if(  ! $ok  ) {   CloseHandle($h2);   }

$ok= $h2= createFile( "ReadOnly.txt", "r" );
$ok or print "# ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 7

$ok= SetFilePointer( $h1, length("Original"), [], FILE_BEGIN );
$ok or print "# ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 8

$ok= WriteFile( $h1, "ly was other text\n", 0, $len, [] )
  &&  $len == length("ly was other text\n");
$ok or print "# <$len> should be <",
  length("ly was other text\n"),">: ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 9

$ok= ReadFile( $h2, $text, 80, $len, [] )
 &&  $len == length($text);
$ok or print "# <$len> should be <",length($text),
  ">: ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 10

$ok= $text eq "Originally was other text\n";
if( !$ok ) {
    $text =~ s/\r/\\r/g;   $text =~ s/\n/\\n/g;
    print "# <$text> should be <Originally was other text\\n>.\n";
}
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 11

$ok= CloseHandle($h2);
$ok or print "# ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 12

$ok= ! ReadFile( $h2, $text, 80, $len, [] )
 &&  Win32API::File::_fileLastError() == 6; # handle is invalid
$ok or print "# ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 13

CloseHandle($h1);

$ok= $h1= createFile( "CanWrite.txt", "rw", FILE_SHARE_WRITE,
	      { Create=>CREATE_ALWAYS } );
$ok or print "# ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 14

$ok= WriteFile( $h1, "Just this and not this", 10, [], [] );
$ok or print "# ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 15

$ok= $h2= createFile( "CanWrite.txt", "wk", { Share=>"rw" } );
$ok or print "# ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 16

$ok= OsFHandleOpen( "APP", $h2, "wat" );
$ok or print "# ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 17

$ok=  $h2 == GetOsFHandle( "APP" );
$ok or print "# $h2 != ",GetOsFHandle("APP"),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 18

{   my $save= select(APP);   $|= 1;  select($save);   }
$ok= print APP "is enough\n";
$ok or print "# ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 19

SetFilePointer($h1, 0, [], FILE_BEGIN) if $^O eq 'cygwin';

$ok= ReadFile( $h1, $text, 0, [], [] );
$ok or print "# ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 20

$ok=  $text eq "is enough\r\n";
if( !$ok ) {
    $text =~ s/\r/\\r/g;
    $text =~ s/\n/\\n/g;
    print "# <$text> should be <is enough\\r\\n>\n";
}
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 21

$skip = "";
if ($^O eq 'cygwin') {
    $ok = 1;
    $skip = " # skip cygwin can delete open files";
}
else {
    unlink("CanWrite.txt");
    $ok = -e "CanWrite.txt" &&  $! =~ /permission denied/i;
    $ok or print "# $!\n";
}
print $ok ? "" : "not ", "ok ", ++$test, "$skip\n"; # ok 22

close(APP);		# Also does C<CloseHandle($h2)>
## CloseHandle( $h2 );
CloseHandle( $h1 );

$ok= ! DeleteFile( "ReadOnly.txt" )
 &&  Win32API::File::_fileLastError() == 5; # access is denied
$ok or print "# ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 23

$ok= ! CopyFile( "ReadOnly.txt", "CanWrite.txt", 1 )
 &&  Win32API::File::_fileLastError() == 80; # file exists
$ok or print "# ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 24

$ok= ! CopyFile( "CanWrite.txt", "ReadOnly.txt", 0 )
 &&  Win32API::File::_fileLastError() == 5; # access is denied
$ok or print "# ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 25

$ok= ! MoveFile( "NoSuchFile", "NoSuchDest" )
 &&  Win32API::File::_fileLastError() == 2; # not find the file
$ok or print "# ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 26

$ok= ! MoveFileEx( "NoSuchFile", "NoSuchDest", 0 )
 &&  Win32API::File::_fileLastError() == 2; # not find the file
$ok or print "# ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 27

$ok= ! MoveFile( "ReadOnly.txt", "CanWrite.txt" )
 &&  Win32API::File::_fileLastError() == 183; # file already exists
$ok or print "# ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 28

$ok= ! MoveFileEx( "ReadOnly.txt", "CanWrite.txt", 0 )
 &&  Win32API::File::_fileLastError() == 183; # file already exists
$ok or print "# ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 29

$ok= CopyFile( "ReadOnly.txt", "ReadOnly.cp", 1 )
 &&  CopyFile( "CanWrite.txt", "CanWrite.cp", 1 );
$ok or print "# ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 30

$ok= ! MoveFileEx( "CanWrite.txt", "ReadOnly.cp", MOVEFILE_REPLACE_EXISTING )
 &&  (Win32API::File::_fileLastError() == 5     # access is denied
 ||   Win32API::File::_fileLastError() == 183); # already exists
$ok or print "# ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 31

$ok= MoveFileEx( "ReadOnly.cp", "CanWrite.cp", MOVEFILE_REPLACE_EXISTING );
$ok or print "# ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 32

$ok= MoveFile( "CanWrite.cp", "Moved.cp" );
$ok or print "# ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 33

$ok= ! unlink( "ReadOnly.cp" )
 &&  $! =~ /no such file/i
 &&  ! unlink( "CanWrite.cp" )
 &&  $! =~ /no such file/i;
$ok or print "# $!\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 34

$ok= ! DeleteFile( "Moved.cp" )
 &&  Win32API::File::_fileLastError() == 5; # access is denied
$ok or print "# ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 35

if ($^O eq 'cygwin') {
    chmod( 0200 | 07777 & (stat("Moved.cp"))[2], "Moved.cp" );
}
else {
    system( "attrib -r Moved.cp" );
}

$ok= DeleteFile( "Moved.cp" );
$ok or print "# ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 36

$new= SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX;
$old= SetErrorMode( $new );
$renew= SetErrorMode( $old );
$reold= SetErrorMode( $old );

$ok= $old == $reold;
$ok or print "# $old != $reold: ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 37

$ok= ($renew&$new) == $new;
$ok or print "# $new != $renew: ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 38

$ok= @drives= getLogicalDrives();
$ok && print "# @drives\n";
$ok or print "# ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 39

$ok=  $drives[0] !~ /^[ab]/  ||  DRIVE_REMOVABLE == GetDriveType($drives[0]);
$ok or print "# ",DRIVE_REMOVABLE," != ",GetDriveType($drives[0]),
  ": ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 40

$drive= substr( $ENV{WINDIR}, 0, 3 );

$ok= 1 == grep /^\Q$drive\E/i, @drives;
$ok or print "# No $drive found in list of drives.\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 41

$ok= DRIVE_FIXED == GetDriveType( $drive );
$ok or print
  "# ",DRIVE_FIXED," != ",GetDriveType($drive),": ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 42

$ok=  GetVolumeInformation( $drive, $vol, 64, $ser, $max, $flag, $fs, 16 );
$ok or print "# ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 43
$vol= $ser= $max= $flag= $fs= "";	# Prevent warnings.

chop($drive);
$ok= QueryDosDevice( $drive, $dev, 80 );
$ok or print "# $drive: ",fileLastError(),"\n";
if( $ok ) {
    ( $text= $dev ) =~ s/\0/\\0/g;
    print "# $drive => $text\n";
}
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 44

$bits= GetLogicalDrives();
$let= 25;
$bit= 1<<$let;
while(  $bit & $bits  ) {
    $let--;
    $bit >>= 1;
}
$let= pack( "C", $let + unpack("C","A") ) . ":";
print "# Querying undefined $let.\n";

$ok= DefineDosDevice( 0, $let, $ENV{WINDIR} );
$ok or print "# $let,$ENV{WINDIR}: ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 45

$ok=  -s $let."/Win.ini"  ==  -s $ENV{WINDIR}."/Win.ini";
$ok or print "# ", -s $let."/Win.ini", " vs. ",
  -s $ENV{WINDIR}."/Win.ini", ": ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 46

$ok= DefineDosDevice( DDD_REMOVE_DEFINITION|DDD_EXACT_MATCH_ON_REMOVE,
		      $let, $ENV{WINDIR} );
$ok or print "# $let,$ENV{WINDIR}: ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 47

$ok= ! -f $let."/Win.ini"
  &&  $! =~ /no such file/i;
$ok or print "# $!\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 48

$ok= DefineDosDevice( DDD_RAW_TARGET_PATH, $let, $dev );
if( !$ok  ) {
    ( $text= $dev ) =~ s/\0/\\0/g;
    print "# $let,$text: ",fileLastError(),"\n";
}
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 49

my $path = $ENV{WINDIR};
$ok= -f $let.substr($path,$^O eq 'cygwin'?2:3)."/win.ini";
$ok or print "# ",$let.substr($path,3)."/win.ini ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 50

$ok= DefineDosDevice( DDD_REMOVE_DEFINITION|DDD_EXACT_MATCH_ON_REMOVE
		     |DDD_RAW_TARGET_PATH, $let, $dev );
$ok or print "# $let,$dev: ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 51

my $attrs = GetFileAttributes( $path );
$ok= $attrs != INVALID_FILE_ATTRIBUTES;
$ok or print "# $path gave invalid attribute value, attrs=$attrs: ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 52

$ok= ($attrs & FILE_ATTRIBUTE_DIRECTORY);
$ok or print "# $path not a directory, attrs=$attrs: ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 53

$path .= "/win.ini";
$attrs = GetFileAttributes( $path );
$ok= $attrs != INVALID_FILE_ATTRIBUTES;
$ok or print "# $path gave invalid attribute value, attrs=$attrs: ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 54

$ok= !($attrs & FILE_ATTRIBUTE_DIRECTORY);
$ok or print "# $path is a directory, attrs=$attrs: ",fileLastError(),"\n";
print $ok ? "" : "not ", "ok ", ++$test, "\n";	# ok 55

#	DefineDosDevice
#	GetFileType
#	GetVolumeInformation
#	QueryDosDevice
#Add a drive letter that points to our temp directory
#Add a drive letter that points to the drive our directory is in

#winnt.t:
# get first drive letters and use to test disk and storage IOCTLs
# "//./PhysicalDrive0"
#	DeviceIoControl

my %consts;
my @consts= @Win32API::File::EXPORT_OK;
@consts{@consts}= @consts;

my( @noargs, %noargs )= qw(
  attrLetsToBits fileLastError getLogicalDrives GetLogicalDrives );
@noargs{@noargs}= @noargs;

foreach $func ( @{$Win32API::File::EXPORT_TAGS{Func}} ) {
    delete $consts{$func};
    if(  defined( $noargs{$func} )  ) {
	$ok=  ! eval("$func(0,0)")  &&  $@ =~ /(::|\s)_?${func}A?[(:\s]/;
    } else {
	$ok=  ! eval("$func()")  &&  $@ =~ /(::|\s)_?${func}A?[(:\s]/;
    }
    $ok or print "# $func: $@\n";
    print $ok ? "" : "not ", "ok ", ++$test, "\n";
}

foreach $func ( @{$Win32API::File::EXPORT_TAGS{FuncA}},
                @{$Win32API::File::EXPORT_TAGS{FuncW}} ) {
    $ok=  ! eval("$func()")  &&  $@ =~ /::_?${func}\(/;
    delete $consts{$func};
    $ok or print "# $func: $@\n";
    print $ok ? "" : "not ", "ok ", ++$test, "\n";
}

foreach $const ( keys(%consts) ) {
    $ok= eval("my \$x= $const(); 1");
    $ok or print "# Constant $const: $@\n";
    print $ok ? "" : "not ", "ok ", ++$test, "\n";
}

chdir( $temp );
if (-e "$dir/ReadOnly.txt") {
    chmod 0777, "$dir/ReadOnly.txt";
    unlink "$dir/ReadOnly.txt";
}
unlink "$dir/CanWrite.txt" if -e "$dir/CanWrite.txt";
rmdir $dir;

__END__
