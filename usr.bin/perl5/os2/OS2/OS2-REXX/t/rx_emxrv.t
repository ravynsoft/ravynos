BEGIN {
    chdir 't' if -d 't/lib';
    @INC = '../lib' if -d 'lib';
    require Config; import Config;
    if (-d 'lib' and $Config{'extensions'} !~ /\bOS2(::|\/)REXX\b/) {
	print "1..0\n";
	exit 0;
    }
}

print "1..20\n";

require OS2::DLL;
print "ok 1\n";
$emx_dll = OS2::DLL->load('emx');
print "ok 2\n";
$emx_version = $emx_dll->emx_revision();
print "ok 3\n";
$emx_version >= 40 or print "not ";	# We cannot work with old EMXs
print "ok 4\n";

$reason = '';
$emx_version >= 99 and $reason = ' # skipped: version of EMX 100 or more';	# Be safe
print "ok 5$reason\n";

$emx_fullname = OS2::DLLname 0x202, $emx_dll->{Handle};	# Handle ==> fullname
print "ok 6\n";
$emx_dll1 = OS2::DLL->module($emx_fullname);
print "ok 7\n";
$emx_dll->{Handle} == $emx_dll1->{Handle} or print "not ";
print "ok 8\n";

$emx_version1 = $emx_dll1->emx_revision();
print "ok 9\n";
$emx_version1 eq $emx_version or print "not ";
print "ok 10\n";

$emx_revision = $emx_dll->wrapper_REXX('emx_revision');
print "ok 11\n";
$emx_version2 = $emx_revision->();
print "ok 12\n";
$emx_version2 eq $emx_version or print "not ";
print "ok 13\n";

$emx_revision1 = $emx_dll1->wrapper_REXX('#128');
print "ok 14\n";
$emx_version3 = $emx_revision1->();
print "ok 15\n";
$emx_version3 eq $emx_version or print "not ";
print "ok 16\n";

($emx_fullname1 = $emx_fullname) =~ s,/,\\,g;
$emx_dll2 = OS2::DLL->new($emx_fullname1);
print "ok 17\n";
$emx_dll->{Handle} == $emx_dll2->{Handle} or print "not ";
print "ok 18\n";

$emx_version4 = $emx_dll2->emx_revision();
print "ok 19\n";
$emx_version4 eq $emx_version or print "not ";
print "ok 20\n";
