#!./perl

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib'; 
}

my $perl = $^X;
$perl = VMS::Filespec::vmsify($perl) if $^O eq 'VMS';

my $Invoke_Perl = qq(MCR $perl "-I[-.lib]");

use Test::More;

SKIP: {
    skip("tests for non-VMS only", 1) if $^O eq 'VMS';

    no utf8;

    BEGIN { $Orig_Bits = $^H }

    # make sure that all those 'use vmsish' calls didn't do anything.
    is( $Orig_Bits, $^H,    'use vmsish a no-op' );
}

SKIP: {
    skip("tests for VMS only", 28) unless $^O eq 'VMS';

#========== vmsish status ==========
`$Invoke_Perl -e 1`;  # Avoid system() from a pipe from harness.  Mutter.
is($?,0,"simple Perl invocation: POSIX success status");
{
  use vmsish qw(status);
  is(($? & 1),1, "importing vmsish [vmsish status]");
  {
    no vmsish qw(status); # check unimport function
    is($?,0, "unimport vmsish [POSIX STATUS]");
  }
  # and lexical scoping
  is(($? & 1),1,"lex scope of vmsish [vmsish status]");
}
is($?,0,"outer lex scope of vmsish [POSIX status]");

{
  use vmsish qw(exit);  # check import function
  is($?,0,"importing vmsish exit [POSIX status]");
}

#========== vmsish exit, messages ==========
{
  use vmsish qw(status);

  $msg = do_a_perl('-e "exit 1"');
    $msg =~ s/\n/\\n/g; # keep output on one line
  like($msg, qr/ABORT/, "POSIX ERR exit, DCL error message check");
  is($?&1,0,"vmsish status check, POSIX ERR exit");

  $msg = do_a_perl('-e "use vmsish qw(exit); exit 1"');
    $msg =~ s/\n/\\n/g; # keep output on one line
  ok(length($msg)==0,"vmsish OK exit, DCL error message check");
  is($?&1,1, "vmsish status check, vmsish OK exit");

  $msg = do_a_perl('-e "\&CORE::exit;use vmsish qw(exit);&CORE::exit(1)"');
    $msg =~ s/\n/\\n/g; # keep output on one line
  ok(length($msg)==0,"vmsish OK exit (via &CORE::), DCL err msg check");
  is($?&1,1, "vmsish status check, vmsish OK exit (&CORE::exit)");

  $msg = do_a_perl('-e "use vmsish qw(exit); exit 44"');
    $msg =~ s/\n/\\n/g; # keep output on one line
  like($msg, qr/ABORT/, "vmsish ERR exit, DCL error message check");
  is($?&1,0,"vmsish ERR exit, vmsish status check");

  $msg = do_a_perl('-e "use vmsish qw(hushed); exit 1"');
  $msg =~ s/\n/\\n/g; # keep output on one line
  ok(($msg !~ /ABORT/),"POSIX ERR exit, vmsish hushed, DCL error message check");

  $msg = do_a_perl('-e "\&CORE::exit; use vmsish qw(hushed); '
                  .'vmsish::hushed(0); &CORE::exit 1"');
  $msg =~ s/\n/\\n/g; # keep output on one line
  ok(($msg !~ /ABORT/),
   "POSIX ERR exit, vmsish hushed, DCL error message check (&CORE::exit)");

  $msg = do_a_perl('-e "use vmsish qw(exit hushed); exit 44"');
    $msg =~ s/\n/\\n/g; # keep output on one line
  ok(($msg !~ /ABORT/),"vmsish ERR exit, vmsish hushed, DCL error message check");

  $msg = do_a_perl('-e "use vmsish qw(exit hushed); no vmsish qw(hushed); exit 44"');
  $msg =~ s/\n/\\n/g; # keep output on one line
  like($msg, qr/ABORT/,"vmsish ERR exit, no vmsish hushed, DCL error message check");

  $msg = do_a_perl('-e "use vmsish qw(hushed); die(qw(blah));"');
  $msg =~ s/\n/\\n/g; # keep output on one line
  ok(($msg !~ /ABORT/),"die, vmsish hushed, DCL error message check");

  $msg = do_a_perl('-e "\&CORE::die; use vmsish qw(hushed); '
                  .'vmsish::hushed(0); &CORE::die(qw(blah));"');
  $msg =~ s/\n/\\n/g; # keep output on one line
  ok(($msg !~ /ABORT/),"&CORE::die, vmsish hushed, DCL error msg check");

  $msg = do_a_perl('-e "use vmsish qw(hushed); use Carp; croak(qw(blah));"');
  $msg =~ s/\n/\\n/g; # keep output on one line
  ok(($msg !~ /ABORT/),"croak, vmsish hushed, DCL error message check");

  $msg = do_a_perl('-e "use vmsish qw(exit); vmsish::hushed(1); exit 44;"');
  $msg =~ s/\n/\\n/g; # keep output on one line
  ok(($msg !~ /ABORT/),"vmsish ERR exit, vmsish hushed at runtime, DCL error message check");

  local *TEST;
  open(TEST,'>','vmsish_test.pl') || die('not ok ?? : unable to open "vmsish_test.pl" for writing');  
  print TEST "#! perl\n";
  print TEST "use vmsish qw(hushed);\n";
  print TEST "\$obvious = (\$compile(\$error;\n";
  close TEST;
  $msg = do_a_perl('vmsish_test.pl');
  $msg =~ s/\n/\\n/g; # keep output on one line
  ok(($msg !~ /ABORT/),"compile ERR exit, vmsish hushed, DCL error message check");
  unlink 'vmsish_test.pl';
}


#========== vmsish time ==========
{
  my($utctime, @utclocal, @utcgmtime, $utcmtime,
     $vmstime, @vmslocal, @vmsgmtime, $vmsmtime,
     $utcval,  $vmaval, $offset);
  # Make sure apparent local time isn't GMT
  if (not $ENV{'SYS$TIMEZONE_DIFFERENTIAL'}) {
    $oldtz = $ENV{'SYS$TIMEZONE_DIFFERENTIAL'};
    $ENV{'SYS$TIMEZONE_DIFFERENTIAL'} = 3600;
    eval "END { \$ENV{'SYS\$TIMEZONE_DIFFERENTIAL'} = $oldtz; }";
    gmtime(0); # Force reset of tz offset
  }

  # Unless we are prepared to parse the timezone rules here and figure out
  # what the correct offset was when the file was last revised, we need to 
  # use a file for which the current offset is known to be valid.  That's why
  # we create a file rather than using an existing one for the stat() test.

  my $file = 'sys$scratch:vmsish_t_flirble.tmp';
  open TMP, '>', $file or die "Couldn't open file $file";
  close TMP;
  END { 1 while unlink $file; }

  {
     use_ok('vmsish', 'time');

     # but that didn't get it in our current scope
     use vmsish qw(time);

     $vmstime   = time;
     @vmslocal  = localtime($vmstime);
     @vmsgmtime = gmtime($vmstime);
     $vmsmtime  = (stat $file)[9];
  }
  $utctime   = time;
  @utclocal  = localtime($vmstime);
  @utcgmtime = gmtime($vmstime);
  $utcmtime  = (stat $file)[9];
  
  $offset = $ENV{'SYS$TIMEZONE_DIFFERENTIAL'};

  # We allow lots of leeway (10 sec) difference for these tests,
  # since it's unlikely local time will differ from UTC by so small
  # an amount, and it renders the test resistant to delays from
  # things like stat() on a file mounted over a slow network link.
  ok(abs($utctime - $vmstime + $offset) <= 10,"(time) UTC: $utctime VMS: $vmstime");

  $utcval = $utclocal[5] * 31536000 + $utclocal[7] * 86400 +
            $utclocal[2] * 3600     + $utclocal[1] * 60 + $utclocal[0];
  $vmsval = $vmslocal[5] * 31536000 + $vmslocal[7] * 86400 +
            $vmslocal[2] * 3600     + $vmslocal[1] * 60 + $vmslocal[0];
  ok(abs($vmsval - $utcval + $offset) <= 10, "(localtime) UTC: $utcval  VMS: $vmsval");
  print "# UTC: @utclocal\n# VMS: @vmslocal\n";

  $utcval = $utcgmtime[5] * 31536000 + $utcgmtime[7] * 86400 +
            $utcgmtime[2] * 3600     + $utcgmtime[1] * 60 + $utcgmtime[0];
  $vmsval = $vmsgmtime[5] * 31536000 + $vmsgmtime[7] * 86400 +
            $vmsgmtime[2] * 3600     + $vmsgmtime[1] * 60 + $vmsgmtime[0];
  ok(abs($vmsval - $utcval + $offset) <= 10, "(gmtime) UTC: $utcval  VMS: $vmsval");
  print "# UTC: @utcgmtime\n# VMS: @vmsgmtime\n";

  ok(abs($utcmtime - $vmsmtime + $offset) <= 10,"(stat) UTC: $utcmtime  VMS: $vmsmtime");
}
}

done_testing();

#====== need this to make sure error messages come out, even if
#       they were turned off in invoking procedure
sub do_a_perl {
    local *P;
    open(P,'>','vmsish_test.com') || die('not ok ?? : unable to open "vmsish_test.com" for writing');
    print P "\$ set message/facil/sever/ident/text\n";
    print P "\$ define/nolog/user sys\$error _nla0:\n";
    print P "\$ $Invoke_Perl @_\n";
    close P;
    my $x = `\@vmsish_test.com`;
    unlink 'vmsish_test.com';
    return $x;
}

