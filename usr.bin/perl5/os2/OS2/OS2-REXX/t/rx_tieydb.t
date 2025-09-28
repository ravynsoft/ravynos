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
$rx = load OS2::REXX "RXU"     # from RXU1a.ZIP
  or print "1..0 # skipped: cannot find RXU.DLL\n" and exit;

print "1..7\n", "ok 1\n";

$rx->prefix("Rx");                         # implicit function prefix
print "ok 2\n";

REXX_call {
  tie @pib, OS2::REXX, "IB.P";       # bind array to REXX stem variable
  print "ok 3\n";
  tie %tib, OS2::REXX, "IB.T.";      # bind associative array to REXX stem var
  print "ok 4\n";

  $rx->GetInfoBlocks("IB.");    # call REXX function
  print "ok 5\n";
  defined $pib[6] ? print "ok 6\n" : print "not ok 6\n# pib\n";
  defined $tib{7} && $tib{7} =~ /^\d+$/ ? print "ok 7\n"
    : print "not ok 7\n# tib\n";
  print "# Process status is ", unpack("I", $pib[6]),
        ", thread ordinal is $tib{7}\n";
};
