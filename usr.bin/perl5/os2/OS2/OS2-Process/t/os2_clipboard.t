#! /usr/bin/perl -w

use strict;
use Test::More tests => 87;
BEGIN {use_ok 'OS2::Process', qw(:DEFAULT CFI_POINTER CF_TEXT)}

# Initialize
my $raw = "Just a random\nselection";
(my $cr = $raw) =~ s/\n/\r\n/g;
ok(ClipbrdText_set($raw), 'ClipbrdText_set');

my ($v, $p, @f);
is(ClipbrdText, $cr, "ClipbrdText it back");
is(ClipbrdOwner, 0, "ClipbrdOwner is not defined");
$v = ClipbrdViewer;
ok((!$v || IsWindow $v), "ClipbrdViewer is not defined or a valid window");

{
  my $h = OS2::localClipbrd->new;
  $p = ClipbrdData;

  @f = MemoryRegionSize($p, 0x4000);		# 4 pages, 16K, limit
  is(scalar @f, 2, 'MemoryRegionSize(16K) returns 2 values');
  # diag(sprintf '%#x, %#x, %#x, %#x', @f, $f[0]+$p, $p);
  is($f[0], 4096, 'MemoryRegionSize claims 1 page is available');
  ok($f[1] & 0x1, 'MemoryRegionSize claims page readable');# PAG_READ=1 0x12013

  my @f1 = MemoryRegionSize($p, 0x100000);		# 16 blocks, 1M, limit
  is(scalar @f1, 2, 'MemoryRegionSize(1M) returns 2 values');
  is($f1[0], $f[0], 'MemoryRegionSize returns same length');
  is($f1[1], $f[1], 'MemoryRegionSize returns same flags');

  @f1 = MemoryRegionSize($p);
  is(scalar @f1, 2, 'MemoryRegionSize(no-limit) returns 2 values');
  is($f1[0], $f[0], 'MemoryRegionSize returns same length');
  is($f1[1], $f[1], 'MemoryRegionSize returns same flags');
}

ok($p, 'ClipbrdData');

is(ClipbrdFmtInfo, CFI_POINTER, 'ClipbrdFmtInfo is CFI_POINTER');

# CF_TEXT is 1
ok(!defined eval {ClipbrdText(1+CF_TEXT); 1}, "ClipbrdText(not CF_TEXT) croaks");
like($@, qr/\bPMERR_INVALID_HWND\b/, 'with expected (lousy) error message');

@f = ClipbrdFmtAtoms;
is(scalar @f, 1, "Only one format available");
is($f[0], CF_TEXT, "format is CF_TEXT");

@f = ClipbrdFmtNames;
is(scalar @f, 1, "Only one format available");
is($f[0], '#1', "format is CF_TEXT='#1'");

{
  my $h = OS2::localClipbrd->new;
  ok(EmptyClipbrd, 'EmptyClipbrd');
}

@f = ClipbrdFmtNames;
is(scalar @f, 0, "No format available");

undef $p; undef $v;
eval {
  my $h = OS2::localClipbrd->new;
  $p = ClipbrdData;
  $v = 1;
};

ok(! defined $p, 'ClipbrdData croaked');
like($@, qr/\bPMERR_INVALID_HWND\b/, 'with expected (lousy) error message');

ok(! defined eval {ClipbrdText}, "ClipbrdText croaks");
like($@, qr/\bPMERR_INVALID_HWND\b/, 'with expected (lousy) error message');

# CF_TEXT is 1
ok(!defined eval {ClipbrdText(1+CF_TEXT); 1}, "ClipbrdText(not CF_TEXT) croaks");
like($@, qr/\bPMERR_INVALID_HWND\b/, 'with expected (lousy) error message');

is(ClipbrdOwner, 0, "ClipbrdOwner is not defined");

$v = ClipbrdViewer;
ok((!$v || IsWindow $v), "ClipbrdViewer is not defined or a valid window");

is(ClipbrdFmtInfo, 0, 'ClipbrdFmtInfo is 0');

@f = ClipbrdFmtAtoms;
is(scalar @f, 0, "No formats available");

{
  my $h = OS2::localClipbrd->new;
  ok(EmptyClipbrd, 'EmptyClipbrd when clipboard is empty succeeds');
}

ok(ClipbrdText_set($raw, 1), 'ClipbrdText_set() raw');
is(ClipbrdText, $raw, "ClipbrdText it back");

{
  my $h = OS2::localClipbrd->new;
  ok(EmptyClipbrd, 'EmptyClipbrd again');
}

my $ar = AddAtom 'perltest/unknown_raw';
ok($ar, 'Atom added');
my $ar1 = AddAtom 'perltest/unknown_raw1';
ok($ar1, 'Atom added');
my $a = AddAtom 'perltest/unknown';
ok($a, 'Atom added');
my $a1 = AddAtom 'perltest/unknown1';
ok($a1, 'Atom added');

{
  my $h = OS2::localClipbrd->new;
  ok(ClipbrdData_set($raw), 	     'ClipbrdData_set()');
  ok(ClipbrdData_set($raw, 0, $ar1), 'ClipbrdData_set(perltest/unknown_raw1)');
  ok(ClipbrdData_set($cr,  0, $ar),  'ClipbrdData_set(perltest/unknown_raw)');
  ok(ClipbrdData_set($raw, 1, $a1),  'ClipbrdData_set(perltest/unknown1)');
  ok(ClipbrdData_set($cr,  1, $a),   'ClipbrdData_set(perltest/unknown)');
  # Results should be the same, except ($raw, 0) one...
}

is(ClipbrdText, $cr,	    "ClipbrdText CF_TEXT back");
is(ClipbrdText($ar1), $raw, "ClipbrdText perltest/unknown_raw1 back");
is(ClipbrdText($ar), $cr,   "ClipbrdText perltest/unknown_raw back");
is(ClipbrdText($a1), $cr,   "ClipbrdText perltest/unknown1 back");
is(ClipbrdText($a), $cr,    "ClipbrdText perltest/unknown back");

is(ClipbrdFmtInfo,	 CFI_POINTER, 'ClipbrdFmtInfo is CFI_POINTER');
is(ClipbrdFmtInfo($ar1), CFI_POINTER, 'ClipbrdFmtInfo is CFI_POINTER');
is(ClipbrdFmtInfo($ar),  CFI_POINTER, 'ClipbrdFmtInfo is CFI_POINTER');
is(ClipbrdFmtInfo($a1),  CFI_POINTER, 'ClipbrdFmtInfo is CFI_POINTER');
is(ClipbrdFmtInfo($a),   CFI_POINTER, 'ClipbrdFmtInfo is CFI_POINTER');

# CF_TEXT is 1
ok(!defined eval {ClipbrdText(1+CF_TEXT); 1}, "ClipbrdText(1+CF_TEXT) croaks");
like($@, qr/\bPMERR_INVALID_HWND\b/, 'with expected (lousy) error message');

my $names = join ',', sort '#1', qw(perltest/unknown perltest/unknown1
				    perltest/unknown_raw perltest/unknown_raw1);
@f = ClipbrdFmtAtoms;
is(scalar @f, 5, "5 formats available");
is((join ',', sort map AtomName($_), @f), $names, "formats are $names");

@f = ClipbrdFmtNames;
is(scalar @f, 5, "Only one format available");
is((join ',', sort @f), $names, "formats are $names");

{
  my $h = OS2::localClipbrd->new;
  ok(EmptyClipbrd, 'EmptyClipbrd');
}

@f = ClipbrdFmtNames;
is(scalar @f, 0, "No formats available");

{
  my $h = OS2::localClipbrd->new;
  ok(ClipbrdText_set($cr,  1, $ar),  'ClipbrdText_set(perltest/unknown_raw)');
};

#diag(join ' ', ClipbrdFmtNames);

is(ClipbrdText($ar), $cr,   "ClipbrdText perltest/unknown_raw back");
is(ClipbrdFmtInfo($ar),  CFI_POINTER, 'ClipbrdFmtInfo is CFI_POINTER');

ok(!defined eval {ClipbrdText(CF_TEXT); 1}, "ClipbrdText(CF_TEXT) croaks");
like($@, qr/\bPMERR_INVALID_HWND\b/, 'with expected (lousy) error message');
# CF_TEXT is 1
ok(!defined eval {ClipbrdText(1+CF_TEXT); 1}, "ClipbrdText(1+CF_TEXT) croaks");
like($@, qr/\bPMERR_INVALID_HWND\b/, 'with expected (lousy) error message');

@f = ClipbrdFmtNames;
is(scalar @f, 1, "1 format available");
is($f[0], 'perltest/unknown_raw', "format is perltest/unknown_raw");

@f = ClipbrdFmtAtoms;
is(scalar @f, 1, "1 format available");
is($f[0], $ar, "format is perltest/unknown_raw");

{
  my $h = OS2::localClipbrd->new;
  ok(EmptyClipbrd, 'EmptyClipbrd');
}

undef $p; undef $v;
eval {
  my $h = OS2::localClipbrd->new;
  $p = ClipbrdData;
  $v = 1;
};

ok(! defined $p, 'ClipbrdData croaked');
like($@, qr/\bPMERR_INVALID_HWND\b/, 'with expected (lousy) error message');

ok(! defined eval {ClipbrdText}, "ClipbrdText croaks");
like($@, qr/\bPMERR_INVALID_HWND\b/, 'with expected (lousy) error message');

# CF_TEXT is 1
ok(!defined eval {ClipbrdText(1+CF_TEXT); 1}, "ClipbrdText(not CF_TEXT) croaks");
like($@, qr/\bPMERR_INVALID_HWND\b/, 'with expected (lousy) error message');

is(ClipbrdOwner, 0, "ClipbrdOwner is not defined");

$v = ClipbrdViewer;
ok((!$v || IsWindow $v), "ClipbrdViewer is not defined or a valid window");

is(ClipbrdFmtInfo, 0, 'ClipbrdFmtInfo is 0');

@f = ClipbrdFmtAtoms;
is(scalar @f, 0, "No formats available");

