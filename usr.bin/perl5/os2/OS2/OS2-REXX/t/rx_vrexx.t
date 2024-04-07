BEGIN {
    chdir 't' if -d 't/lib';
    @INC = '../lib' if -d 'lib';
    require Config; import Config;
    if (-d 'lib' and $Config{'extensions'} !~ /\bOS2(::|\/)REXX\b/) {
	print "1..0 # skipped: OS2::REXX not built\n";
	exit 0;
    }
    if (defined $ENV{PERL_TEST_NOVREXX}) {
	print "1..0 # skipped: request via PERL_TEST_NOVREXX\n";
	exit 0;
    }
}

use OS2::REXX;

$name = "VREXX";
$path = $ENV{LIBPATH} || $ENV{PATH} or die;
foreach $dir (split(';', $path)) {
  next unless -f "$dir/$name.DLL";
  $found = "$dir/$name.DLL";
  print "# found at `$found'\n";
  last;
}
$found or print "1..0 # skipped: cannot find $name.DLL\n" and exit;

print "1..10\n";

REXX_call {
  $vrexx = DynaLoader::dl_load_file($found) or die "not ok 1\n# load\n";
  print "ok 1\n";
  $vinit   = DynaLoader::dl_find_symbol($vrexx, "VINIT")   or die "find vinit";
  print "ok 2\n";
  $vexit   = DynaLoader::dl_find_symbol($vrexx, "VEXIT")   or die "find vexit";
  print "ok 3\n";
  $vmsgbox = DynaLoader::dl_find_symbol($vrexx, "VMSGBOX") or die "find vmsgbox";
  print "ok 4\n";
  $vversion= DynaLoader::dl_find_symbol($vrexx, "VGETVERSION") or die "find vgetversion";
  print "ok 5\n";
  
  $result = OS2::REXX::_call("VInit", $vinit) or die "VInit";
  print "ok 6\n";
  print "# VInit: $result\n";
  
  OS2::REXX::_set("MBOX.0" => 4,
  	        "MBOX.1" => "Perl VREXX Access Test",
  	        "MBOX.2" => "",
  	        "MBOX.3" => "(C) Andreas Kaiser",
  	        "MBOX.4" => "December 1994")
  	or die "set var";
  print "ok 7\n";
  
  $result = OS2::REXX::_call("VGetVersion", $vversion) or die "VMsgBox";
  print "ok 8\n";
  print "# VGetVersion: $result\n";
  
  $result = OS2::REXX::_call("VMsgBox", $vmsgbox, "", "Perl", "MBOX", 1) or die "VMsgBox";
  print "ok 9\n";
  print "# VMsgBox: $result\n";
  
  OS2::REXX::_call("VExit", $vexit);
  print "ok 10\n";
};
