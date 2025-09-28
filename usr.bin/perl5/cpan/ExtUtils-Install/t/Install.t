#!/usr/bin/perl -w
use strict;

# Test ExtUtils::Install.

BEGIN {
    unshift @INC, 't/lib';
}

use TieOut;
use File::Path;
use File::Spec;
use File::Temp qw[tempdir];

use Test::More tests => 62;

use MakeMaker::Test::Setup::BFD;

BEGIN {
  local $ENV{PERL_INSTALL_QUIET};
  use_ok('ExtUtils::Install');
}
# ensure the env doesn't pollute our tests
local $ENV{EU_INSTALL_ALWAYS_COPY};
local $ENV{EU_ALWAYS_COPY};

# Check exports.
foreach my $func (qw(install uninstall pm_to_blib install_default)) {
    can_ok(__PACKAGE__, $func);
}

my $tmpdir = tempdir( DIR => 't', CLEANUP => 1 );
chdir $tmpdir;

ok( setup_recurs(), 'setup' );
END {
    ok( chdir File::Spec->updir, 'chdir ..');
    ok( teardown_recurs(), 'teardown' );
}

chdir 'Big-Dummy';

my $stdout = tie *STDOUT, 'TieOut';
pm_to_blib( { 'lib/Big/Dummy.pm' => 'blib/lib/Big/Dummy.pm' },
            'blib/lib/auto'
          );
END { rmtree 'blib' }

ok( -d 'blib/lib',              'pm_to_blib created blib dir' );
ok( -r 'blib/lib/Big/Dummy.pm', '  copied .pm file' );
ok( -r 'blib/lib/auto',         '  created autosplit dir' );
is( $stdout->read, "cp lib/Big/Dummy.pm blib/lib/Big/Dummy.pm\n" );


pm_to_blib( { 'lib/Big/Dummy.pm' => 'blib/lib/Big/Dummy.pm' },
            'blib/lib/auto'
          );
ok( -d 'blib/lib',              'second run, blib dir still there' );
ok( -r 'blib/lib/Big/Dummy.pm', '  .pm file still there' );
ok( -r 'blib/lib/auto',         '  autosplit still there' );
is( $stdout->read, "Skip blib/lib/Big/Dummy.pm (unchanged)\n" );


install( { 'blib/lib' => 'install-test/lib/perl',
           read   => 'install-test/packlist',
           write  => 'install-test/packlist'
         },
       0, 1);
ok( ! -d 'install-test/lib/perl',        'install made dir (dry run)');
ok( ! -r 'install-test/lib/perl/Big/Dummy.pm',
                                         '  .pm file installed (dry run)');
ok( ! -r 'install-test/packlist',        '  packlist exists (dry run)');

install( { 'blib/lib' => 'install-test/lib/perl',
           read   => 'install-test/packlist',
           write  => 'install-test/packlist'
         } );
ok( -d 'install-test/lib/perl',                 'install made dir' );
ok( -r 'install-test/lib/perl/Big/Dummy.pm',    '  .pm file installed' );
ok(!-r 'install-test/lib/perl/Big/Dummy.SKIP',  '  ignored .SKIP file' );
ok( -r 'install-test/packlist',                 '  packlist exists' );

open(PACKLIST, 'install-test/packlist' );
my %packlist = map { chomp;  ($_ => 1) } <PACKLIST>;
close PACKLIST;

# On case-insensitive filesystems (ie. VMS), the keys of the packlist might
# be lowercase. :(
my $native_dummy = File::Spec->catfile(qw(install-test lib perl Big Dummy.pm));
is( keys %packlist, 1 );
is( lc((keys %packlist)[0]), lc $native_dummy, 'packlist written' );


# Test UNINST=1 preserving same versions in other dirs.
install( { 'blib/lib' => 'install-test/other_lib/perl',
           read   => 'install-test/packlist',
           write  => 'install-test/packlist'
         },
       0, 0, 1);
ok( -d 'install-test/other_lib/perl',        'install made other dir' );
ok( -r 'install-test/other_lib/perl/Big/Dummy.pm', '  .pm file installed' );
ok( -r 'install-test/packlist',              '  packlist exists' );
ok( -r 'install-test/lib/perl/Big/Dummy.pm', '  UNINST=1 preserved same' );


chmod 0644, 'blib/lib/Big/Dummy.pm' or die $!;
open(DUMMY, ">>blib/lib/Big/Dummy.pm") or die $!;
print DUMMY "Extra stuff\n";
close DUMMY;


# Test UNINST=0 does not remove other versions in other dirs.
{
  ok( -r 'install-test/lib/perl/Big/Dummy.pm', 'different install exists' );

  local @INC = ('install-test/lib/perl');
  local $ENV{PERL5LIB} = '';
  install( { 'blib/lib' => 'install-test/other_lib/perl',
           read   => 'install-test/packlist',
           write  => 'install-test/packlist'
         },
       0, 0, 0);
  ok( -d 'install-test/other_lib/perl',        'install made other dir' );
  ok( -r 'install-test/other_lib/perl/Big/Dummy.pm', '  .pm file installed' );
  ok( -r 'install-test/packlist',              '  packlist exists' );
  ok( -r 'install-test/lib/perl/Big/Dummy.pm',
                                             '  UNINST=0 left different' );
}

# Test UNINST=1 only warning when failing to remove an irrelevant shadow file
{
  my $tfile='install-test/lib/perl/Big/Dummy.pm';
  local $ExtUtils::Install::Testing = $tfile; 
  local @INC = ('install-test/other_lib/perl','install-test/lib/perl');
  local $ENV{PERL5LIB} = '';
  ok( -r $tfile, 'different install exists' );
  my @warn;
  local $SIG{__WARN__}=sub { push @warn, @_; return };
  my $ok=eval {
    install( { 'blib/lib' => 'install-test/other_lib/perl',
           read   => 'install-test/packlist',
           write  => 'install-test/packlist'
         },
       0, 0, 1);
    1
  };
  ok($ok,'  we didnt die');
  ok(0+@warn,"  we did warn");
  ok( -d 'install-test/other_lib/perl',        'install made other dir' );
  ok( -r 'install-test/other_lib/perl/Big/Dummy.pm', '  .pm file installed' );
  ok( -r 'install-test/packlist',              '  packlist exists' );
  ok( -r $tfile, '  UNINST=1 failed to remove different' );
  
}

# Test UNINST=1 dieing when failing to remove an relevant shadow file
{
  my $tfile='install-test/lib/perl/Big/Dummy.pm';
  local $ExtUtils::Install::Testing = $tfile;
  local @INC = ('install-test/lib/perl','install-test/other_lib/perl');
  local $ENV{PERL5LIB} = '';
  ok( -r $tfile, 'different install exists' );
  my @warn;
  local $SIG{__WARN__}=sub { push @warn,@_; return };
  my $ok=eval {
    install( { 'blib/lib' => 'install-test/other_lib/perl',
           read   => 'install-test/packlist',
           write  => 'install-test/packlist'
         },
       0, 0, 1);
    1
  };
  ok(!$ok,'  we did die');
  ok(!@warn,"  we didnt warn");
  ok( -d 'install-test/other_lib/perl',        'install made other dir' );
  ok( -r 'install-test/other_lib/perl/Big/Dummy.pm', '  .pm file installed' );
  ok( -r 'install-test/packlist',              '  packlist exists' );
  ok( -r $tfile,'  UNINST=1 failed to remove different' );
}

# Test UNINST=1 removing other versions in other dirs.
{
  local @INC = ('install-test/lib/perl');
  local $ENV{PERL5LIB} = '';
  install( { 'blib/lib' => 'install-test/other_lib/perl',
           read   => 'install-test/packlist',
           write  => 'install-test/packlist'
         },
       0, 0, 1);
  ok( -d 'install-test/other_lib/perl',        'install made other dir' );
  ok( -r 'install-test/other_lib/perl/Big/Dummy.pm', '  .pm file installed' );
  ok( -r 'install-test/packlist',              '  packlist exists' );
  ok( !-r 'install-test/lib/perl/Big/Dummy.pm',
                                             '  UNINST=1 removed different' );
}


# do a -w style test, but based on just on file perms rather than UID
# (on UNIX, root sees everything as writeable)

sub writeable {
    my ($file) = @_;
    my @stat = stat $file;
    return 0 unless defined $stat[2]; # mode
    return $stat[2] & 0200;
}


# really this test should be run on any platform that supports
# symbolic and hard links, but this representative sample should do for
# now


# check hard and symbolic links

SKIP: {
    my $has_links =
        $^O =~ /^(aix|bsdos|darwin|freebsd|hpux|irix|linux|openbsd|solaris)$/;
    skip "(sym)links not supported", 8 unless $has_links;

    install([ from_to => { 'blib/lib/' => 'install-links',
                           read   => 'install-links/packlist',
                           write  => 'install-links/packlist'
                         },
    ]);

    # make orig file a hard link and check that it doesn't get messed up

    my $bigdir = 'install-links/Big';
    ok link("$bigdir/Dummy.pm", "$bigdir/DummyHard.pm"),
        'link DummyHard.pm';

    open(my $fh, ">>", "blib/lib/Big/Dummy.pm") or die $!;
    print $fh "Extra stuff 2\n";
    close $fh;

    install([ from_to => { 'blib/lib/' => 'install-links',
                           read   => 'install-links/packlist',
                           write  => 'install-links/packlist'
                         },
    ]);

    ok( !writeable("$bigdir/DummyHard.pm"), 'DummyHard.pm not writeable' );

    use File::Compare;
    ok(compare("$bigdir/Dummy.pm", "$bigdir/DummyHard.pm"),
        "hard-linked file should be different");

    # make orig file a symlink and check that it doesn't get messed up

    ok rename("$bigdir/Dummy.pm", "$bigdir/DummyOrig.pm"),
        'rename DummyOrig.pm';
    ok symlink('DummyOrig.pm', "$bigdir/Dummy.pm"),
        'symlink Dummy.pm';


    open($fh, ">>", "blib/lib/Big/Dummy.pm") or die $!;
    print $fh "Extra stuff 3\n";
    close $fh;

    install([ from_to => { 'blib/lib/' => 'install-links',
                           read   => 'install-links/packlist',
                           write  => 'install-links/packlist'
                         },
    ]);

    ok( !writeable("$bigdir/DummyOrig.pm"), 'DummyOrig.pm not writeable' );
    ok( !-l "$bigdir/Dummy.pm", 'Dummy.pm not a link' );
    ok(compare("$bigdir/Dummy.pm", "$bigdir/DummyOrig.pm"),
        "orig file should be different");
}

pm_to_blib( { 'lib/Dummy/Split.pm' => 'blib/lib/Dummy/Split.pm' },
            'blib/lib/auto'
          );

ok( -r 'blib/lib/auto/Dummy/Split/split.al',
  'pm_to_blib does autosplit on appropriate files',
);
eval {
  pm_to_blib( { 'lib/Dummy/Split.pm' => 'blib/lib/Dummy/Split.pm' } );
};
is $@, '', 'pm_to_blib with no autodir works';
