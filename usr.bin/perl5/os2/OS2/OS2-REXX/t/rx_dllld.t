BEGIN {
    chdir 't' if -d 't/lib';
    @INC = '../lib' if -d 'lib';
    require Config; import Config;
    if (-d 'lib' and $Config{'extensions'} !~ /\bOS2(::|\/)REXX\b/) {
	print "1..0\n";
	exit 0;
    }
}

use OS2::REXX;

$path = $ENV{LIBPATH} || $ENV{PATH} or die;
foreach $dir (split(';', $path)) {
  next unless -f "$dir/RXU.DLL";
  $found = "$dir/RXU.DLL";
  last;
}
$found or print "1..0 # skipped: cannot find RXU.DLL\n" and exit;

print "1..5\n";

$module = DynaLoader::dl_load_file($found) or die "not ok 1\n# load\n";
print "ok 1\n";

$address = DynaLoader::dl_find_symbol($module, "RXPROCID") 
  or die "not ok 2\n# find\n";
print "ok 2\n";

$result = OS2::REXX::_call("RxProcId", $address) or die "not ok 3\n# REXX";
print "ok 3\n";

($pid, $ppid, $ssid) = split(/\s+/, $result);
$pid == $$ ? print "ok 4\n" : print "not ok 4\n# pid\n";
$ssid == 1 ? print "ok 5\n" : print "not ok 5\n# pid\n";
print "# pid=$pid, ppid=$ppid, ssid=$ssid\n";
