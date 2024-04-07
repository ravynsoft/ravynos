#!/usr/bin/perl -w
BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
}

use Test::More tests => 19;
use strict;
use Config;

my $cwd = Cwd::sys_cwd();
ok -d $cwd;

my $lpb = Cwd::extLibpath;
$lpb .= ';' unless $lpb and $lpb =~ /;$/;

my $lpe = Cwd::extLibpath(1);
$lpe .= ';' unless $lpe and $lpe =~ /;$/;

ok Cwd::extLibpath_set("$lpb$cwd");

$lpb = Cwd::extLibpath;
$lpb =~ s#\\#/#g;
(my $s_cwd = $cwd) =~ s#\\#/#g;

like($lpb, qr/\Q$s_cwd/);

ok Cwd::extLibpath_set("$lpe$cwd", 1);

$lpe = Cwd::extLibpath(1);
$lpe =~ s#\\#/#g;

like($lpe, qr/\Q$s_cwd/);

if (uc OS2::DLLname() eq uc $^X) {	# Static build
  my ($short) = ($^X =~ m,.*[/\\]([^.]+),);
  is(uc OS2::DLLname(1), uc $short);
  is(uc OS2::DLLname, uc $^X );		# automatically
  is(1,1);				# automatically...
} else {
  is(uc OS2::DLLname(1), uc $Config{dll_name});
  like(OS2::DLLname, qr#\Q/$Config{dll_name}\E\.dll$#i );
  (my $root_cwd = $s_cwd) =~ s,/t$,,;
  like(OS2::DLLname, qr#^\Q$root_cwd\E(/t)?\Q/$Config{dll_name}\E\.dll#i );
}
is(OS2::DLLname, OS2::DLLname(2));
like(OS2::DLLname(0), qr#^(\d+)$# );


is(OS2::DLLname($_), OS2::DLLname($_, \&Cwd::extLibpath) ) for 0..2;
ok(not defined eval { OS2::DLLname $_, \&Cwd::cwd; 1 } ) for 0..2;
ok(not defined eval { OS2::DLLname $_, \&xxx; 1 } ) for 0..2;
