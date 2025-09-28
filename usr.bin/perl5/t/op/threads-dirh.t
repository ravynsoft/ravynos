#!perl

# Test interaction of threads and directory handles.

BEGIN {
     chdir 't' if -d 't';
     @INC = '../lib';
     require './test.pl';
     $| = 1;

     require Config;
     skip_all_without_config('useithreads');
     skip_all_if_miniperl("no dynamic loading on miniperl, no threads");
     skip_all("runs out of memory on some EBCDIC") if $ENV{PERL_SKIP_BIG_MEM_TESTS};

     plan(6);
}

use strict;
use warnings;
use threads;
use threads::shared;
use File::Path;
use File::Spec::Functions qw 'updir catdir';
use Cwd 'getcwd';

# Basic sanity check: make sure this does not crash
fresh_perl_is <<'# this is no comment', 'ok', {}, 'crash when duping dirh';
   use threads;
   opendir dir, 'op';
   async{}->join for 1..2;
   print "ok";
# this is no comment

my $dir;
SKIP: {
 skip "telldir or seekdir not defined on this platform", 5
    if !$Config::Config{d_telldir} || !$Config::Config{d_seekdir};
 my $skip = sub {
   chdir($dir);
   chdir updir;
   skip $_[0], 5
 };

 if(!$Config::Config{d_fchdir} && $^O ne "MSWin32") {
  $::TODO = 'dir handle cloning currently requires fchdir on non-Windows platforms';
 }

 my @w :shared; # warnings accumulator
 local $SIG{__WARN__} = sub { push @w, $_[0] };

 $dir = catdir getcwd(), "thrext$$" . int rand() * 100000;

 rmtree($dir) if -d $dir;
 mkdir($dir);

 # Create a dir structure like this:
 #   $dir
 #     |
 #     `- toberead
 #            |
 #            +---- thrit
 #            |
 #            +---- rile
 #            |
 #            `---- zor

 chdir($dir);
 mkdir 'toberead';
 chdir 'toberead';
 {open my $fh, ">thrit" or &$skip("Cannot create file thrit")}
 {open my $fh, ">rile" or &$skip("Cannot create file rile")}
 {open my $fh, ">zor" or &$skip("Cannot create file zor")}
 chdir updir;

 # Then test that dir iterators are cloned correctly.

 opendir my $toberead, 'toberead';
 my $start_pos = telldir $toberead;
 my @first_2 = (scalar readdir $toberead, scalar readdir $toberead);
 my @from_thread = @{; async { [readdir $toberead ] } ->join };
 my @from_main = readdir $toberead;
 is join('-', sort @from_thread), join('-', sort @from_main),
     'dir iterator is copied from one thread to another';
 like
   join('-', "", sort(@first_2, @from_thread), ""),
   qr/(?<!-rile)-rile-thrit-zor-(?!zor-)/i,
  'cloned iterator iterates exactly once over everything not already seen';

 seekdir $toberead, $start_pos;
 readdir $toberead for 1 .. @first_2+@from_thread;
 {
  local $::TODO; # This always passes when dir handles are not cloned.
  is
    async { readdir $toberead // 'undef' } ->join, 'undef',
   'cloned dir iterator that points to the end of the directory'
  ;
 }

 # Make sure the cloning code can handle file names longer than 255 chars
 SKIP: {
  chdir 'toberead';
  open my $fh,
    ">floccipaucinihilopilification-"
   . "pneumonoultramicroscopicsilicovolcanoconiosis-"
   . "lopadotemachoselachogaleokranioleipsanodrimypotrimmatosilphiokarabo"
   . "melitokatakechymenokichlepikossyphophattoperisteralektryonoptokephal"
   . "liokinklopeleiolagoiosiraiobaphetraganopterygon"
    or
     chdir updir,
     skip("OS does not support long file names (and I mean *long*)", 1);
  chdir updir;
  opendir my $dirh, "toberead";
  my $test_name
    = "dir iterators can be cloned when the next fn > 255 chars";
  while() {
   my $pos = telldir $dirh;
   my $fn = readdir($dirh);
   if(!defined $fn) { fail($test_name); last SKIP; }
   if($fn =~ 'lagoio') { 
    seekdir $dirh, $pos;
    last;
   }
  }
  is length async { scalar readdir $dirh } ->join, 258, $test_name;
 }

 is scalar @w, 0, 'no warnings during all that' or diag @w;
 chdir updir;
}
rmtree($dir);
