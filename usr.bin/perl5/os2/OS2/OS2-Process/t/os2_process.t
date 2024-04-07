#! /usr/bin/perl -w

#END {
#  sleep 10;
#}

sub propagate_INC {
  my $inc = $ENV{PERL5LIB};
  $inc = $ENV{PERLLIB} unless defined $inc;
  $inc = '' unless defined $inc;
  $ENV{PERL5LIB} = join ';', @INC, split /;/, $inc;
}

my $separate_session;
BEGIN {			# Remap I/O to the parent's window
 $separate_session = $ENV{OS2_PROCESS_TEST_SEPARATE_SESSION};
 propagate_INC, return unless $separate_session; # done by the parent
 my @fn = split " ", $ENV{NEW_FD};
 my @fh = (*STDOUT, *STDERR);
 my @how = qw( > > );
 # warn $_ for @fn;
 open $fh[$_], "$how[$_]&=$fn[$_]"
   or warn "Cannot reopen $fh[$_], $how[$_]&=$fn[$_]: $!" for 0..1;
}

use strict;
use Test::More tests => 235;
use OS2::Process;

sub SWP_flags ($) {
  my @nkpos = WindowPos shift;
  $nkpos[2];
}

my $interactive_wait = @ARGV && $ARGV[0] eq 'wait';

my @l = OS2::Process::process_entry();
ok(@l == 11, 'all the fields of the process_entry() are there');

# 1: FS 2: Window-VIO 
ok( ($l[9] == 1 or $l[9] == 2), 'we are FS or Windowed-VIO');

#print "# $_\n" for @l;

eval <<'EOE' or die;
#use OS2::Process qw(WM_SYSCOMMAND WM_DBCSLAST FID_CLIENT HWND_DESKTOP);
use OS2::Process qw(WM_SYSCOMMAND WM_DBCSLAST HWND_DESKTOP);

ok( WM_SYSCOMMAND == 0x0021, 'correct WM_SYSCOMMAND' );
ok( WM_DBCSLAST   == 0x00cf,  'correct WM_DBCSLAST' );
#ok( FID_CLIENT    == 0x8008 );
ok( HWND_DESKTOP  == 0x0001,  'correct HWND_DESKTOP' );
1;
EOE

my $t = Title;
my $wint = winTitle;

ok($t, 'got session title');
ok($wint, 'got titlebar text');

my $newt = "test OS2::Process $$";
ok(Title_set($newt), 'successfully set Title');
is(Title, $newt, 'correctly set Title');
my $wt = winTitle or warn "winTitle: $!, $^E";
is(winTitle, $newt, 'winTitle changed its value too');
ok(Title_set $t, 'successfully set Title back');
is(Title, $t, 'correctly set Title back');
is(winTitle, $wint, 'winTitle restored its value too');

$newt = "test OS2::Process both-$$";
ok(bothTitle_set($newt), 'successfully set both titles via Win* API');
is(Title, $newt, 'session title correctly set');
is(winTitle, $newt, 'winTitle correctly set');
ok(bothTitle_set($t), 'successfully reset both titles via Win* API');
is(Title, $t, 'session title correctly reset');
is(winTitle, $wint, 'winTitle correctly reset');

$newt = "test OS2::Process win-$$";
ok(winTitle_set($newt), 'successfully set titlebar title via Win* API');
is(Title, $t, 'session title remained the same');
is(winTitle, $newt, 'winTitle changed value');
ok(winTitle_set($wint), 'successfully reset titlebar title via Win* API');
is(Title, $t, 'session title remained the same');
is(winTitle, $wint, 'winTitle restored value');

$newt = "test OS2::Process sw-$$";
ok(swTitle_set($newt), 'successfully set session title via Win* API');
is(Title, $newt, 'session title correctly set');
is(winTitle, $wint, 'winTitle has unchanged value');
ok(swTitle_set($t), 'successfully reset session title via Win* API');
is(Title, $t, 'session title correctly set');
is(winTitle, $wint, 'winTitle has unchanged value');

$newt = "test OS2::Process again-$$";
ok(Title_set($newt), 'successfully set Title again');
is(Title, $newt, 'correctly set Title again');
is(winTitle, $newt, 'winTitle changed its value too again');
ok(Title_set($t), 'successfully set Title back');
is(Title, $t, 'correctly set Title back');
is(winTitle, $wint, 'winTitle restored its value too again');

my $hwnd = process_hwnd;
ok($hwnd, 'found session owner hwnd');
my $c_subhwnd = WindowFromId $hwnd, 0x8008;	# FID_CLIENT;
ok($c_subhwnd, 'found client hwnd');
my $a_subhwnd = ActiveWindow $hwnd;	# or $^E and warn $^E;
ok((not $a_subhwnd and not $^E), 'No active subwindow in a VIO frame');

my $ahwnd = ActiveWindow;
ok($ahwnd, 'found active window');
my $fhwnd = FocusWindow;
ok($fhwnd, 'found focus window');

# This call without morphing results in VIO window with active highlight, but
# no keyboard focus (even after Alt-Tabbing to it; you cannot Alt-Tab off it!)

# Interestingly, Desktop is active on the switch list, but the
# switch list is not acting on keyboard events.

# Give up focus
{ my $force_PM = OS2::localMorphPM->new(0);
  ok $force_PM, 'morphed to PM locally';
  ok FocusWindow_set(1), 'set focus to DESKTOP';	# HWND_DESKTOP
}
my $dtop = DesktopWindow;
ok($dtop, 'found the desktop window');

#OS2::Process::ResetWinError;			# XXXX Should not be needed!
$ahwnd = ActiveWindow or $^E and warn $^E;
ok( (not $ahwnd and not $^E), 'desktop is not active');
$fhwnd = FocusWindow;
ok($fhwnd, 'there is a focus window');
is($fhwnd, $dtop, 'which is the desktop');

# XXXX Well, no need to skip it now...
SKIP: {
  skip 'We already have focus', 4 if $hwnd == $ahwnd;
  my $force_PM = OS2::localMorphPM->new(0);
  ok $force_PM, 'morphed to PM locally again';
  ok FocusWindow_set($c_subhwnd), 'set focus to the client of the session owner';
  # If we do not morph, then when the focus is in another VIO frame,
  # we get two VIO frames with activated titlebars.
  # The only (?) way to take the activated state from another frame
  # is to switch to it via the switch list
  $ahwnd = ActiveWindow;
  ok($ahwnd, 'there is an active window');
  $fhwnd = FocusWindow;
  ok($fhwnd, 'there is a focus window');
  is($hwnd, $ahwnd, 'the active window is the session owner');
  is($fhwnd, $c_subhwnd, 'the focus window is the client of the session owner');
}

# Give up focus again
{ my $force_PM = OS2::localMorphPM->new(0);
  ok $force_PM, 'morphed to PM locally again';
  ok FocusWindow_set(1), 'set focus to DESKTOP again';	# HWND_DESKTOP
}

$ahwnd = ActiveWindow or $^E and warn $^E;
ok( (not $ahwnd and not $^E), 'desktop is not active again');
$fhwnd = FocusWindow;
ok($fhwnd, 'there is a focus window');
is($fhwnd, $dtop, 'which is the desktop');

# XXXX Well, no need to skip it now...
SKIP: {
  skip 'We already have focus', 4 if $hwnd == $ahwnd;
  my $force_PM = OS2::localMorphPM->new(0);
  ok $force_PM, 'morphed to PM locally again';
  ok ActiveWindow_set($hwnd), 'activate the session owner';
  $ahwnd = ActiveWindow;
  ok($ahwnd, 'there is an active window');
  $fhwnd = FocusWindow;
  ok($fhwnd, 'there is a focus window');
  is($hwnd, $ahwnd, 'the active window is the session owner');
}

# XXXX Well, no need to skip it now...
SKIP: {
  skip 'Tests assume we have focus', 1 unless $hwnd == $ahwnd;
  # We have focus
  # is($fhwnd, $ahwnd);
  # is($a_subhwnd, $c_subhwnd);
  is($fhwnd, $c_subhwnd, 'the focus window is the client of the session owner');
}

# Check enumeration of switch entries:
my $skid_title = "temporary s-kid ppid=$$";
my $spid = system P_SESSION, $^X, '-wle', "END {sleep 25} use OS2::Process; eval {Title_set '$skid_title'} or warn \$@; \$SIG{TERM} = sub {exit 0}";
ok ($spid, 'start the new VIO session with unique title');
sleep 1;
my @sw = grep $_->{title} eq $skid_title, process_hentries;
sleep 1000 unless @sw;
is(scalar @sw, 1, 'exactly one session with this title');
my $sw = $sw[0];
ok $sw, 'have the data about the session';
is($sw->{owner_pid}, $spid, 'session has a correct pid');
my $k_hwnd = $sw->{owner_hwnd};
ok $k_hwnd, 'found the session window handle';
is sidOf($spid), $sw->{owner_sid}, 'we know sid of the session';

# Give up focus again
{ my $force_PM = OS2::localMorphPM->new(0);
  ok $force_PM, 'morphed to PM locally again';
  ok FocusWindow_set($k_hwnd), 'set focus to kid session window';
}

$ahwnd = ActiveWindow;
ok $ahwnd, 'there is an active window';
is $ahwnd, $k_hwnd, 'after focusing the active window is the owner_hwnd';
$fhwnd = FocusWindow;
ok $fhwnd, 'there is a focus window';
my $c_sub_ahwnd = WindowFromId $ahwnd, 0x8008;	# FID_CLIENT;
ok $c_sub_ahwnd, 'the active window has a FID_CLIENT';
is($fhwnd, $ahwnd, 'the focus window = the active window');

ok hWindowPos_set({behind => 3}, $k_hwnd),	# HWND_TOP
  'put kid to the front';

# After Alt-Tab a WS_TOPMOST, WS_DISABLED window of class 'AltTabWindow' exists
my $top = (hWindowPos $k_hwnd)->{behind};
ok(($top == 3 or WindowStyle($top) & 0x200000),	# HWND_TOP, WS_TOPMOST
   'kid is at front');
# is((hWindowPos $k_hwnd)->{behind}, 3, 'kid is at front');

my ($enum_handle, $first_zorder, $first_non_TOPMOST);
{ my $force_PM = OS2::localMorphPM->new(0);
  ok $force_PM, 'morphed to PM locally again';
  $enum_handle = BeginEnumWindows 1;		# HWND_DESKTOP
  ok $enum_handle, 'start enumeration';
  $first_non_TOPMOST = $first_zorder = GetNextWindow $enum_handle;
  ok $first_zorder, 'GetNextWindow works';
  my $f = WindowStyle $first_non_TOPMOST;
  ok $f, 'WindowStyle works';
  $f = WindowStyle($first_non_TOPMOST = GetNextWindow $enum_handle)
    while $f & 0x200000;				# WS_TOPMOST
  ok($first_non_TOPMOST, 'There is non-TOPMOST window');
  ok(!(WindowStyle($first_non_TOPMOST) & 0x200000), 'Indeed non-TOPMOST');
  ok EndEnumWindows($enum_handle), 'end enumeration';
}
is ($first_non_TOPMOST, $k_hwnd, 'kid is the first in z-order enumeration');

ok hWindowPos_set({behind => 4}, $k_hwnd),	# HWND_BOTTOM
  'put kid to the back';

# This does not work, the result is the handle of "Window List"
# is((hWindowPos $k_hwnd)->{behind}, 4, 'kis is at back');

my (@list, $next, @list1);
{ my $force_PM = OS2::localMorphPM->new(0);
  ok $force_PM, 'morphed to PM locally again';
  $enum_handle = BeginEnumWindows 1;		# HWND_DESKTOP
  ok $enum_handle, 'start enumeration';
  push @list, $next while $next = GetNextWindow $enum_handle;
  @list1 = ChildWindows;
  ok 1, 'embedded ChildWindows()';
  ok EndEnumWindows($enum_handle), 'end enumeration';

  is_deeply \@list, \@list1, 'Manual list same as by ChildWindows()';
  # Apparently, the 'Desktop' window is still behind us;
  # Note that this window is *not* what is returned by DesktopWindow
  pop @list if WindowText($list[-1]) eq 'Desktop';
}
is ($list[-1], $k_hwnd, 'kid is the last in z-order enumeration');
# print "# kid=$k_hwnd in @list\n";
@list = ChildWindows;
is_deeply \@list, \@list1, 'Other ChildWindows(), same result';
ok scalar @list, 'ChildWindows works';
is $list[-2], $k_hwnd, 'kid is the last but one in ChildWindows';

ok hWindowPos_set({behind => 3}, $k_hwnd),	# HWND_TOP
  'put kid to the front again';

$top = (hWindowPos $k_hwnd)->{behind};
ok(($top == 3 or WindowStyle($top) & 0x200000),	# WS_TOPMOST
   'kid is at front again');
sleep 5 if $interactive_wait;

ok IsWindow($k_hwnd), 'IsWindow works';
#print "# win=$k_hwnd => err=$^E\n";
my $c_sub_khwnd = WindowFromId $k_hwnd, 0x8008;	# FID_CLIENT
ok $c_sub_khwnd, 'have kids client window';
ok IsWindow($c_sub_khwnd), 'IsWindow works on the client';
#print "# win=$c_sub_khwnd => IsWindow err=$^E\n";
my ($pkid,$tkid) = WindowProcess $c_sub_khwnd;
my ($pkid1,$tkid1) = WindowProcess $hwnd;
ok($pkid1 > 0, 'our window has a governing process');
ok($tkid1 > 0, 'our window has a governing thread');
is($pkid, $pkid1, 'kid\'s window is governed by the same process as our (PMSHELL:1)');
is($tkid, $tkid1, 'likewise for threads');
is $pkid, ppidOf($spid), 'the governor is the parent of the kid session';

my $my_pos = hWindowPos($hwnd);
ok $my_pos, 'got my position';
{ my $force_PM = OS2::localMorphPM->new(0);
  ok $force_PM, 'morphed to PM locally again';
  my @pos = WindowPos $hwnd;
  my @ppos = WindowPos $k_hwnd;
  # ok hWindowPos_set({%$my_pos, behind => $hwnd}, $k_hwnd), 'hide the kid behind us';
  # Hide it completely behind our window
  ok hWindowPos_set({x => $my_pos->{x}, y => $my_pos->{y}, behind => $hwnd,
		     width => $my_pos->{width}, height => $my_pos->{height}},
		    $k_hwnd), 'hide the kid behind us';
  # ok WindowPos_set($k_hwnd, $pos[0], $pos[1]), 'hide the kid behind us';
  my @kpos = WindowPos $k_hwnd;
  # print "# kidpos=@ppos\n";
  # print "#  mypos=@pos\n";
  # print "# kidpos=@kpos\n";
# kidpos=252 630 4111 808 478 3 66518088 502482793
#  mypos=276 78 4111 491 149 2147484137 66518060 502532977
# kidpos=276 78 4111 491 149 2147484255 1392374582 213000
  print "# Before window position\n" if $interactive_wait;
  sleep 5 if $interactive_wait;

  my $w_at = WindowFromPoint($kpos[0] + 5, $kpos[0] + 5, 1, 0); # HWND_DESKTOP, no grandchildren
  ok $w_at, 'got window near LL corner of the kid';
  print "# we=$hwnd, our client=$c_subhwnd, kid=$k_hwnd, kid's client=$c_sub_khwnd\n";
  #is $w_at, $c_sub_khwnd, 'it is the kids client';
  #is $w_at, $k_hwnd, 'it is the kids frame';
  # Apparently, this result is accidental only...
#  is $w_at, $hwnd, 'it is our frame - is on top, but no focus';
  #is $w_at, $c_subhwnd, 'it is our client';
  print "# text: `", WindowText $w_at, "'.\n";
  $w_at = WindowFromPoint($kpos[0] + 5, $kpos[0] + 5); # HWND_DESKTOP, grandchildren too
  ok $w_at, 'got grandkid window near LL corner of the kid';
  # Apparently, this result is accidental only...
#  is $w_at, $c_subhwnd, 'it is our client';
  print "# text: `", WindowText $w_at, "'.\n";
  ok IsWindowVisible $k_hwnd, 'kid is flaged as visible';
  ok IsWindowEnabled $k_hwnd, 'kid is flaged as enabled';
  ok IsWindowShowing $hwnd, 'we are showing';
  ok ((not IsWindowShowing $k_hwnd), 'kid is not showing');
  ok ((not eval { IsWindowShowing 12; 1 }), 'wrong kid causes errors');
  is $^E+0, 0x1001, 'error is 0x1001';
  like $@, qr/\Q[Win]IsWindowShowing/, 'error message shows function';
  like $@, qr/SYS4097\b/, 'error message shows error number';
  like $@, qr/\b0x1001\b/, 'error message shows error number in hex';

  ok WindowPos_set($k_hwnd, @ppos[0..5]), 'restore the kid position';
  my @nkpos = WindowPos $k_hwnd;
  my $fl = $nkpos[2];
  is_deeply([@ppos[0..5]], [@nkpos[0..5]], 'position restored');
  ok IsWindowShowing $k_hwnd, 'kid is showing';
  ok IsWindowVisible $k_hwnd, 'kid is flaged as visible';
  ok IsWindowEnabled $k_hwnd, 'kid is flaged as enabled';
  sleep 5 if $interactive_wait;
  ok EnableWindow($k_hwnd, 0), 'disable the kid';
  ok IsWindowShowing $k_hwnd, 'kid is showing';
  ok IsWindowVisible $k_hwnd, 'kid is flaged as visible';
  ok !IsWindowEnabled $k_hwnd, 'kid is flaged as not enabled';
  ok EnableWindow($k_hwnd), 'enable the kid';
  ok IsWindowShowing $k_hwnd, 'kid is showing';
  ok IsWindowVisible $k_hwnd, 'kid is flaged as visible';
  ok IsWindowEnabled $k_hwnd, 'kid is flaged as enabled';
  ok ShowWindow($k_hwnd, 0), 'hide the kid';
  ok !IsWindowShowing $k_hwnd, 'kid is not showing';
  ok !IsWindowVisible $k_hwnd, 'kid is flaged as not visible';
  ok IsWindowEnabled $k_hwnd, 'kid is flaged as enabled';
  ok ShowWindow($k_hwnd), 'show the kid';
  ok IsWindowShowing $k_hwnd, 'kid is showing';
  ok IsWindowVisible $k_hwnd, 'kid is flaged as visible';
  ok IsWindowEnabled $k_hwnd, 'kid is flaged as enabled';
  ok( ($fl & 0x1800), 'window is maximized or restored'); # SWP_MAXIMIZE SWP_RESTORE
  ok( ($fl & 0x1800) != 0x1800, 'window is not maximized AND restored'); # SWP_MAXIMIZE SWP_RESTORE

  ok PostMsg( $k_hwnd, 0x21,	# WM_SYSCOMMAND, SC_MINIMIZE
	      OS2::Process::MPFROMSHORT 0x8002), 'post minimize message';
  sleep 1;
  ok !IsWindowShowing $k_hwnd, 'kid is not showing';
  ok IsWindowVisible $k_hwnd, 'kid is flaged as visible';
  ok IsWindowEnabled $k_hwnd, 'kid is flaged as enabled';
  is 0x1c00 & SWP_flags $k_hwnd, 0x400, 'kid is minimized'; # SWP_MINIMIZE

  ok PostMsg($k_hwnd, 0x21,	# WM_SYSCOMMAND, SC_RESTORE
	     OS2::Process::MPFROMSHORT 0x8008), 'post restore message';
  sleep 1;
  ok IsWindowShowing $k_hwnd, 'kid is showing';
  ok IsWindowVisible $k_hwnd, 'kid is flaged as visible';
  ok IsWindowEnabled $k_hwnd, 'kid is flaged as enabled';
  is 0x1c00 & SWP_flags $k_hwnd, 0x1000, 'kid is restored'; # SWP_RESTORE

  ok PostMsg($k_hwnd, 0x21,	# WM_SYSCOMMAND, SC_MAXIMIZE
	    OS2::Process::MPFROMSHORT 0x8003), 'post maximize message';
  sleep 1;
  ok IsWindowShowing $k_hwnd, 'kid is showing';
  ok IsWindowVisible $k_hwnd, 'kid is flaged as visible';
  ok IsWindowEnabled $k_hwnd, 'kid is flaged as enabled';
  is 0x1c00 & SWP_flags $k_hwnd, 0x800, 'kid is maximized'; # SWP_MAXIMIZE

  ok PostMsg( $k_hwnd, 0x21,	# WM_SYSCOMMAND, SC_MINIMIZE
	      OS2::Process::MPFROMSHORT 0x8002), 'post minimize message again';
  sleep 1;
  ok !IsWindowShowing $k_hwnd, 'kid is not showing';
  ok IsWindowVisible $k_hwnd, 'kid is flaged as visible';
  ok IsWindowEnabled $k_hwnd, 'kid is flaged as enabled';
  is 0x1c00 & SWP_flags $k_hwnd, 0x400, 'kid is minimized'; # SWP_MINIMIZE

  ok PostMsg($k_hwnd, 0x21,	# WM_SYSCOMMAND, SC_RESTORE
	     OS2::Process::MPFROMSHORT 0x8008), 'post restore message again';
  sleep 1;
  ok IsWindowShowing $k_hwnd, 'kid is showing';
  ok IsWindowVisible $k_hwnd, 'kid is flaged as visible';
  ok IsWindowEnabled $k_hwnd, 'kid is flaged as enabled';
  is 0x1c00 & SWP_flags $k_hwnd, 0x1000, 'kid is restored'; # SWP_RESTORE

  ok PostMsg( $k_hwnd, 0x21,	# WM_SYSCOMMAND, SC_MINIMIZE
	      OS2::Process::MPFROMSHORT 0x8002), 'post minimize message again';
  sleep 1;
  ok !IsWindowShowing $k_hwnd, 'kid is not showing';
  ok IsWindowVisible $k_hwnd, 'kid is flaged as visible';
  ok IsWindowEnabled $k_hwnd, 'kid is flaged as enabled';
  is 0x1c00 & SWP_flags $k_hwnd, 0x400, 'kid is minimized'; # SWP_MINIMIZE

  ok PostMsg($k_hwnd, 0x21,	# WM_SYSCOMMAND, SC_RESTORE
	     OS2::Process::MPFROMSHORT (($fl & 0x800) ? 0x8003 : 0x8008)), # SWP_MAXIMIZE
	'return back to the initial MAXIMIZE/RESTORE state';
  sleep 1;
  ok IsWindowShowing $k_hwnd, 'kid is showing';
  ok IsWindowVisible $k_hwnd, 'kid is flaged as visible';
  ok IsWindowEnabled $k_hwnd, 'kid is flaged as enabled';
  SKIP: {
    skip 'if defaultVIO=MAXIMIZED, new windows are shifted, but maximize to UL corner', 1 unless $fl & 0x800;
    ok hWindowPos_set({x => $ppos[0], y => $ppos[1]}, $k_hwnd), 'x,y-restore for de-minimization of MAXIMIZED';
  }
  @nkpos = WindowPos $k_hwnd;
  is_deeply([@ppos[0..5]], [@nkpos[0..5]], 'position restored');


  # Now the other way
  ok hWindowPos_set( {flags => 0x400}, $k_hwnd), 'set to minimized';
  ok !IsWindowShowing $k_hwnd, 'kid is not showing';
  ok IsWindowVisible $k_hwnd, 'kid is flaged as visible';
  ok IsWindowEnabled $k_hwnd, 'kid is flaged as enabled';
  is 0x1c00 & SWP_flags $k_hwnd, 0x400, 'kid is minimized'; # SWP_MINIMIZE

  ok hWindowPos_set( {flags => 0x1000}, $k_hwnd), 'set to restore';
  ok IsWindowShowing $k_hwnd, 'kid is showing';
  ok IsWindowVisible $k_hwnd, 'kid is flaged as visible';
  ok IsWindowEnabled $k_hwnd, 'kid is flaged as enabled';
  is 0x1c00 & SWP_flags $k_hwnd, 0x1000, 'kid is restored'; # SWP_RESTORE

  ok hWindowPos_set( {flags => 0x800}, $k_hwnd), 'set to maximized';
  ok IsWindowShowing $k_hwnd, 'kid is showing';
  ok IsWindowVisible $k_hwnd, 'kid is flaged as visible';
  ok IsWindowEnabled $k_hwnd, 'kid is flaged as enabled';
  is 0x1c00 & SWP_flags $k_hwnd, 0x800, 'kid is maximized'; # SWP_MAXIMIZE

  ok hWindowPos_set( {flags => 0x400}, $k_hwnd), 'set to minimized again';
  ok !IsWindowShowing $k_hwnd, 'kid is not showing';
  ok IsWindowVisible $k_hwnd, 'kid is flaged as visible';
  ok IsWindowEnabled $k_hwnd, 'kid is flaged as enabled';
  is 0x1c00 & SWP_flags $k_hwnd, 0x400, 'kid is minimized'; # SWP_MINIMIZE

  ok hWindowPos_set( {flags => 0x1000}, $k_hwnd), 'set to restore again';
  ok IsWindowShowing $k_hwnd, 'kid is showing';
  ok IsWindowVisible $k_hwnd, 'kid is flaged as visible';
  ok IsWindowEnabled $k_hwnd, 'kid is flaged as enabled';
  is 0x1c00 & SWP_flags $k_hwnd, 0x1000, 'kid is restored'; # SWP_RESTORE

  ok hWindowPos_set( {flags => 0x400}, $k_hwnd), 'set to minimized again';
  ok !IsWindowShowing $k_hwnd, 'kid is not showing';
  ok IsWindowVisible $k_hwnd, 'kid is flaged as visible';
  ok IsWindowEnabled $k_hwnd, 'kid is flaged as enabled';
  is 0x1c00 & SWP_flags $k_hwnd, 0x400, 'kid is minimized'; # SWP_MINIMIZE

  ok hWindowPos_set( {flags => ($fl & 0x1800)}, $k_hwnd),
	'set back to the initial MAXIMIZE/RESTORE state';
  ok IsWindowShowing $k_hwnd, 'kid is showing';
  ok IsWindowVisible $k_hwnd, 'kid is flaged as visible';
  ok IsWindowEnabled $k_hwnd, 'kid is flaged as enabled';
  SKIP: {
    skip 'if defaultVIO=MAXIMIZED, new windows are shifted, but maximize to UL corner', 1 unless $fl & 0x800;
    ok hWindowPos_set({x => $ppos[0], y => $ppos[1]}, $k_hwnd), 'x,y-restore for de-minimization of MAXIMIZED';
  }
  @nkpos = WindowPos $k_hwnd;
  is_deeply([@ppos[0..5]], [@nkpos[0..5]], 'position restored');

}

# XXXX Well, no need to skip it now...
SKIP: {
  skip 'We already have focus', 4 if $hwnd == $ahwnd;
  my $force_PM = OS2::localMorphPM->new(0);
  ok($force_PM, 'morphed to catch focus again');
  ok FocusWindow_set($c_subhwnd), 'set focus to the client of the session owner';
  # If we do not morph, then when the focus is in another VIO frame,
  # we get two VIO frames with activated titlebars.
  # The only (?) way to take the activated state from another frame
  # is to switch to it via the switch list
  $ahwnd = ActiveWindow;
  ok($ahwnd, 'there is an active window');
  $fhwnd = FocusWindow;
  ok($fhwnd, 'there is a focus window');
  is($hwnd, $ahwnd, 'the active window is the session owner');
  is($fhwnd, $c_subhwnd, 'the focus window is the client of the session owner');
}

SKIP: {
  skip 'Potentially destructive session modifications, done in a separate session only',
    12, unless $separate_session;
  # Manipulate process' hentry
  my $he = process_hentry;
  ok($he, 'got process hentry');
  ok($he->{visible}, 'session switch is visible');# 4? Assume nobody manipulated it...

  ok change_entryh($he), 'can change it (without modifications)';
  my $nhe = process_hentry;
  ok $nhe, 'could refetch the process hentry';
  is_deeply($nhe, $he, 'it did not change');

  sleep 5 if $interactive_wait;
  # Try removing the process entry from the switch list
  $nhe->{visible} = 0;
  ok change_entryh($nhe), 'can change it to be invisible';
  my $nnhe = process_hentry;
  ok($nnhe, 'could refetch the process hentry');
  is_deeply($nnhe, $nhe, 'it is modified as expected');
  is($nnhe->{visible}, 0, 'it is not visible');

  sleep 5 if $interactive_wait;

  $nhe->{visible} = 1;
  ok change_entryh ($nhe), 'can change it to be visible';
  $nnhe = process_hentry;
  ok($nnhe, 'could refetch the process hentry');
  ok($nnhe->{visible}, 'it is visible');
  sleep 5 if $interactive_wait;
}
